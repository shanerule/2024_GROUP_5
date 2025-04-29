#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QModelIndex>
#include "ModelPart.h"
#include "ui_mainwindow.h"
#include "Option_Dialog.h"
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
#include <QFileDialog>
#include <vtkJPEGReader.h>
#include <vtkPNGReader.h>
#include <vtkTexture.h>
#include <qtimer.h>s
#include "skyboxutils.h"
#include <QDir>
#include <qmessagebox.h>





MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect button signal to slot
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::handleButton);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(ui->actionOpen_File, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->treeView, &QWidget::customContextMenuRequested, this, &MainWindow::showTreeContextMenu);

    connect(ui->loadBackgroundButton, &QPushButton::clicked, this, &MainWindow::onLoadBackgroundClicked);

    //skybox connect 
    connect(ui->loadSkyboxButton, &QPushButton::clicked, this, &MainWindow::onLoadSkyboxClicked);

    // Connect status bar signal to status bar slot
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);

    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);



    // Create and initialize the ModelPartList
    this->partList = new ModelPartList("PartsList");

    // Link it to the TreeView in the GUI
    ui->treeView->setModel(this->partList);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    /* This needs adding to MainWindow constructor */
    /* Link a render window with the Qt widget */
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->vtkWidget->setRenderWindow(renderWindow);

    /* Add a renderer */
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    /* Create an object and add to renderer (this will change later to display a CAD model) */

    /* Will just copy and paste cylinder example from before */
    // This creates a polygonal cylinder model with eight circumferential facets
    // (i.e, in practice an octagonal prism).
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(8);

    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes are
    // defined.
    vtkNew<vtkPolyDataMapper> cylinderMapper;
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    // The actor is a grouping mechanism: besides the geometry (mapper), it
    // also has a property, transformation matrix, and/or texture map.
    // Here we set its color and rotate it around the X and Y axes.
    vtkNew<vtkActor> cylinderActor;
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1., 0., 0.35);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);

    renderer->AddActor(cylinderActor);

    /* Reset Camera (probably needs to go in its own function that is called whenever model is changed) */
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();

    // Create a root item
    ModelPart* rootItem = this->partList->getRootItem();

    // Add 1 top level items
    QString name = QString("Model ").arg(1);
    QString visible("true");

    // Create child item
    ModelPart* childItem = new ModelPart({ name, visible });
    rootItem->appendChild(childItem);
    //--------------------------------------
    // Create the rotation timer
    rotationTimer = new QTimer(this);
    connect(rotationTimer, &QTimer::timeout, this, &MainWindow::onAutoRotate);

    // Connect slider signal
    connect(ui->rotationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onRotationSpeedChanged);

    // Start with stopped rotation
    rotationSpeed = 0.0;
    rotationTimer->start(16); // ~60 FPS (16 ms)
    //--------------------------------------
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleButton() {
    QMessageBox msgBox;
    msgBox.setText("Add button was clicked");
    msgBox.exec();

    // Emit signal to update status bar
    emit statusUpdateMessage(QString("Add button was clicked"), 0);
}

void MainWindow::handleTreeClicked() {
    QModelIndex index = ui->treeView->currentIndex();

    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());

    if (selectedPart) {
        QString text = selectedPart->data(0).toString();
        emit statusUpdateMessage(QString("The selected item is: ") + text, 0);
    }
}


void MainWindow::on_actionOpen_File_triggered() {
    emit statusUpdateMessage(QString("Open File action triggered"), 0);
}

// Implement only this for select multiple
void MainWindow::openFile() {
    // Open a file dialog to select multiple files
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open File"),
        QDir::homePath(),
        tr("STL Files (*.stl);;All Files (*)")
    );

    // If files were selected, add them to the tree view
    if (!fileNames.isEmpty()) {
        QModelIndex index = ui->treeView->currentIndex();
        ModelPart* selectedPart = nullptr;

        // If no item is selected, add the new parts as children of the root item
        if (!index.isValid()) {
            selectedPart = partList->getRootItem();
        }
        else {
            selectedPart = static_cast<ModelPart*>(index.internalPointer());
        }

        // Loop through each selected file and add them to the tree
        for (const QString& fileName : fileNames) {
            // Create a new ModelPart for the STL file
            QList<QVariant> data = { QFileInfo(fileName).fileName(), "true" };
            QModelIndex newIndex = partList->appendChild(index, data);

            // Load the STL file into the new ModelPart
            ModelPart* newPart = static_cast<ModelPart*>(newIndex.internalPointer());
            newPart->loadSTL(fileName);

            // Update the renderer to show the new STL file
            updateRender();
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    openOptionDialog();
}

void MainWindow::showTreeContextMenu(const QPoint& pos) {
    QMenu contextMenu(this);

    QAction* itemOptions = new QAction("Item Options", this);
    connect(itemOptions, &QAction::triggered, this, &MainWindow::on_actionItemOptions_triggered);

    contextMenu.addAction(itemOptions);
    contextMenu.exec(ui->treeView->mapToGlobal(pos));
}

void MainWindow::on_actionItemOptions_triggered() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    Option_Dialog dialog(this);
    dialog.setModelPart(selectedPart);  // Pass the data into dialog

    if (dialog.exec() == QDialog::Accepted) {
        // Get the updated data from the dialog
        QString name;
        int r, g, b;
        bool visible;
        dialog.getModelPartData(name, r, g, b, visible);

        // Update the tree item
        selectedPart->setName(name);
        selectedPart->setColor(QColor(r, g, b));
        selectedPart->setVisible(visible);

        //renderWindow->Render();
        //selectedPart->set(1, visible ? "true" : "false"); // Store visibility text

        // Refresh the UI
        //QModelIndex parentIndex = index.parent();
        //ui->treeView->model()->dataChanged(index, index);
        //ui->treeView->model()->dataChanged(index, index, {Qt::DisplayRole, Qt::BackgroundRole});
        //ui->treeView->update();

        // **FORCE UI REFRESH**
        //QAbstractItemModel *model = ui->treeView->model();
        //model->dataChanged(index, index, {Qt::DisplayRole, Qt::ForegroundRole});

        //ui->treeView->model()->dataChanged(index, index, {Qt::DisplayRole, Qt::BackgroundRole});

        qDebug() << "Updating tree view for item:" << name << "Color:" << r << g << b;

        QAbstractItemModel* model = ui->treeView->model();
        emit model->dataChanged(index, index, { Qt::DisplayRole, Qt::BackgroundRole });


        ui->treeView->update();
        ui->treeView->viewport()->update();

        renderWindow->Render();

        //emit statusUpdateMessage("Item Options Selected", 0);

        //QModelIndex index = ui->treeView->currentIndex();
        //emit ui->treeView->model()->dataChanged(index, index);

    }

}


void MainWindow::openOptionDialog() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        emit statusUpdateMessage("No item selected.", 0);
        return;
    }

    // Get the selected item
    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    // Open dialog and pass selected item
    Option_Dialog dialog(this);
    dialog.setModelPart(selectedPart);

    if (dialog.exec() == QDialog::Accepted) {  // If user clicks OK
        emit statusUpdateMessage("Updated: " + selectedPart->data(0).toString(), 0);

        // Notify TreeView model to refresh
        ui->treeView->model()->dataChanged(index, index);
        renderWindow->Render();
    }
}

void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();
    updateRenderFromTree(partList->index(0, 0, QModelIndex()));

    renderer->ResetCamera();
    renderer->ResetCameraClippingRange();
    renderer->Render();
}

void MainWindow::updateRenderFromTree(const QModelIndex& index) {
    if (index.isValid()) {
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());

        // Retrieve the actor from the selected part and add it to the renderer
        vtkSmartPointer<vtkActor> actor = selectedPart->getActor();
        if (actor) {
            renderer->AddActor(actor);
        }

        // Check if this part has any children
        if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
            return;
        }

        // Loop through children and add their actors
        int rows = partList->rowCount(index);
        for (int i = 0; i < rows; i++) {
            updateRenderFromTree(partList->index(i, 0, index));
        }
    }
}

void MainWindow::deleteSelectedItem() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        emit statusUpdateMessage("No item selected to delete.", 0);
        return;
    }

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    QString partName = selectedPart->data(0).toString();

    // Optional: Confirm delete
    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete " + partName + "?") != QMessageBox::Yes)
        return;

    QModelIndex parentIndex = index.parent();
    partList->removeRow(index.row(), parentIndex);

    emit statusUpdateMessage("Deleted: " + partName, 0);

    updateRender(); // Refresh the 3D view
}

void MainWindow::onLoadBackgroundClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Background Image"), "",
        tr("Images (*.png *.jpg *.jpeg)"));

    if (fileName.isEmpty())
        return;

    // Detect file type
    std::string fileStd = fileName.toStdString();
    vtkSmartPointer<vtkImageReader2> reader;

    if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
        reader = vtkSmartPointer<vtkJPEGReader>::New();
    }
    else if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
        reader = vtkSmartPointer<vtkPNGReader>::New();
    }
    else {
        // Unsupported format
        return;
    }

    reader->SetFileName(fileStd.c_str());
    reader->Update();

    vtkSmartPointer<vtkTexture> backgroundTexture = vtkSmartPointer<vtkTexture>::New();
    backgroundTexture->SetInputConnection(reader->GetOutputPort());

    this->renderer->TexturedBackgroundOn();
    this->renderer->SetBackgroundTexture(backgroundTexture);

    // Refresh render window
    this->renderWindow->Render();
}
//----------------Rotate-----------------------------//
void MainWindow::onRotationSpeedChanged(int value)
{
    // Map slider value (0 to 100) to a rotation speed (degrees per timer tick)
    rotationSpeed = static_cast<double>(value) * 0.1; // adjust 0.1 if needed

    // No need to change timer interval — speed is controlled by rotation amount
}

void MainWindow::onAutoRotate()
{
    if (rotationSpeed == 0.0)
        return;

    // Rotate the actor, not the camera!
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid())
        return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart)
        return;

    vtkSmartPointer<vtkActor> actor = selectedPart->getActor();
    if (actor)
    {
        actor->RotateY(rotationSpeed);  // You can also use RotateX or RotateZ depending on desired axis
    }

    this->renderWindow->Render();
}

//---------------------------------------------------//

//-----------------------sky slot function-----------------------//
void MainWindow::onLoadSkyboxClicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Skybox Folder");
    if (dirPath.isEmpty())
        return;

    std::vector<std::string> faceFilenames = {
        (dirPath + "/px.png").toStdString(),   // +X
        (dirPath + "/nx.png").toStdString(),    // -X
        (dirPath + "/py.png").toStdString(),     // +Y
        (dirPath + "/ny.png").toStdString(),  // -Y
        (dirPath + "/pz.png").toStdString(),   // +Z
        (dirPath + "/nz.png").toStdString()     // -Z
    };

    auto cubemapTexture = LoadCubemapTexture(faceFilenames);
    AddSkyboxToRenderer(renderer, cubemapTexture);
    renderWindow->Render();
}




//void MainWindow::openFile() {
// Open a file dialog to select a file
// QString fileName = QFileDialog::getOpenFileName(
//   this,
//   tr("Open File"),
//   QDir::homePath(),
//  tr("STL Files (*.stl);;Text Files (*.txt);;All Files (*)")
//   );

/// If a file was selected, update the selected tree item name
//  if (!fileName.isEmpty()) {
//    QModelIndex index = ui->treeView->currentIndex();
//    if (!index.isValid()) return;

//    ModelPart *selectedPart = static_cast<ModelPart*>(index.internalPointer());
//      if (!selectedPart) return;

//     QString newName = QFileInfo(fileName).fileName();  // Extract filename only
//     selectedPart->setName(newName);

// Notify the model that data has changed
//    ui->treeView->model()->dataChanged(index, index, {Qt::DisplayRole});
//   emit statusUpdateMessage("Updated item name to " + newName, 0);
//  }
//}
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ModelPart.h"
#include "Option_Dialog.h"
#include "skyboxutils.h"
#include "VRRenderThread.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QTimer>
#include <QSlider>
#include <QCheckBox>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>
#include <vtkJPEGReader.h>
#include <vtkPNGReader.h>
#include <vtkLight.h>
#include <vtkImageReader2.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rotationTimer(new QTimer(this))
    , rotationSpeed(0.0)
    , sceneLight(nullptr)
{
    ui->setupUi(this);

    ui->horizontalSlider->setRange(0, 100);  
    ui->horizontalSlider->setValue(50);      


    // Connect button signal to slot
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::handleButton);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(ui->actionOpen_File, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->treeView, &QWidget::customContextMenuRequested, this, &MainWindow::showTreeContextMenu);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);
    connect(ui->toggleVR, &QPushButton::released, this, &MainWindow::handleStartVR);
    // Load background image / skybox
    connect(ui->loadBackgroundButton, &QPushButton::clicked, this, &MainWindow::onLoadBackgroundClicked);
    connect(ui->loadSkyboxButton, &QPushButton::clicked, this, &MainWindow::onLoadSkyboxClicked);

    // Connect status bar signal to status bar slot
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);

    // Rotation system
    connect(rotationTimer, &QTimer::timeout, this, &MainWindow::onAutoRotate);
    connect(ui->rotationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onRotationSpeedChanged);
    rotationSpeed = 0.0;
    rotationTimer->start(16); // ~60 FPS (16 ms)

    // Light intensity slider
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::onLightIntensityChanged);

    // Clip/Shrink filter checkboxes
    connect(ui->checkBox_Clip, &QCheckBox::toggled, this, &MainWindow::on_checkBox_Clip_toggled);
    connect(ui->checkBox_Shrink, &QCheckBox::toggled, this, &MainWindow::on_checkBox_Shrink_toggled);

    // Create and initialize the ModelPartList
    this->partList = new ModelPartList("PartsList");

    // Link it to the TreeView in the GUI
    ui->treeView->setModel(this->partList);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Link a render window with the Qt widget
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->vtkWidget->setRenderWindow(renderWindow);

    // Add a renderer
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Create an object and add to renderer (this will change later to display a CAD model)
    // Will just copy and paste cylinder example from before
    // This creates a polygonal cylinder model with eight circumferential facets
    // (i.e, in practice an octagonal prism).
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(8);

    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes are defined.
    vtkNew<vtkPolyDataMapper> cylinderMapper;
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    // The actor is a grouping mechanism: besides the geometry (mapper), it
    // also has a property, transformation matrix, and/or texture map.
    // Here we set its color and rotate it around the X and Y axes.
    vtkNew<vtkActor> cylinderActor;
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1.0, 0.0, 0.35);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);

    renderer->AddActor(cylinderActor);

    // Reset Camera (probably needs to go in its own function that is called whenever model is changed)
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();

    // Create a root item in tree (example child added)
    ModelPart* rootItem = this->partList->getRootItem();
    //QString name = QString("Model ").arg(1);
    //QString visible("true");
    //ModelPart* childItem = new ModelPart({ name, visible });
    //rootItem->appendChild(childItem);

    // Setup scene light
    /*sceneLight = vtkSmartPointer<vtkLight>::New();
    sceneLight->SetLightTypeToSceneLight();
    sceneLight->SetPosition(5, 5, 15);
    sceneLight->SetPositional(true);
    sceneLight->SetConeAngle(15);
    sceneLight->SetFocalPoint(0, 0, 0);
    sceneLight->SetDiffuseColor(1, 1, 1);
    sceneLight->SetAmbientColor(1, 1, 1);
    sceneLight->SetSpecularColor(1, 1, 1);
    sceneLight->SetIntensity(0.5);  // Initial intensity
    renderer->AddLight(sceneLight);  // Add light after adding actors
    */

    sceneLight = vtkSmartPointer<vtkLight>::New();
    sceneLight->SetLightTypeToSceneLight();
    sceneLight->SetPosition(5, 5, 15);   // Place light above scene
    sceneLight->SetFocalPoint(0, 0, 0);
    sceneLight->SetDiffuseColor(1, 1, 1);
    sceneLight->SetAmbientColor(1, 1, 1);
    sceneLight->SetSpecularColor(1, 1, 1);
    sceneLight->SetIntensity(0.5);       // Initial brightness
    renderer->AddLight(sceneLight);      // Add light to renderer

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleButton()
{
    QMessageBox msgBox;
    msgBox.setText("Add button was clicked");
    msgBox.exec();

    // Emit signal to update status bar
    emit statusUpdateMessage(QString("Add button was clicked"), 0);
}

void MainWindow::handleTreeClicked()
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (selectedPart) {
        QString text = selectedPart->data(0).toString();
        emit statusUpdateMessage(QString("The selected item is: ") + text, 0);
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    emit statusUpdateMessage(QString("Open File action triggered"), 0);
}

void MainWindow::openFile()
{
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
            QString shortName = QFileInfo(fileName).fileName();

            // Check for duplicates
            bool alreadyExists = false;
            int rows = partList->rowCount(QModelIndex());
            for (int i = 0; i < rows; ++i) {
                QModelIndex existingIndex = partList->index(i, 0, QModelIndex());
                ModelPart* existingPart = static_cast<ModelPart*>(existingIndex.internalPointer());
                if (existingPart && existingPart->data(0).toString() == shortName) {
                    alreadyExists = true;
                    break;
                }
            }

            if (alreadyExists) {
                QMessageBox::information(this, "Duplicate File", "The file \"" + shortName + "\" is already loaded.");
                continue; // Skip adding this file
            }

            // Add new ModelPart
            QList<QVariant> data = { shortName, "true" };
            QModelIndex newIndex = partList->appendChild(data);

            ModelPart* newPart = static_cast<ModelPart*>(newIndex.internalPointer());
            newPart->loadSTL(fileName);

            updateRender();
        }
    }

    ui->treeView->expandAll();
}

void MainWindow::on_pushButton_2_clicked()
{
    openOptionDialog();
}

void MainWindow::showTreeContextMenu(const QPoint& pos)
{
    QMenu contextMenu(this);

    QAction* itemOptions = new QAction("Item Options", this);
    connect(itemOptions, &QAction::triggered, this, &MainWindow::on_actionItemOptions_triggered);

    contextMenu.addAction(itemOptions);
    contextMenu.exec(ui->treeView->mapToGlobal(pos));
}

void MainWindow::on_actionItemOptions_triggered()
{
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

        // FORCE UI REFRESH
        QAbstractItemModel* model = ui->treeView->model();
        emit model->dataChanged(index, index, { Qt::DisplayRole, Qt::BackgroundRole });

        ui->treeView->update();
        ui->treeView->viewport()->update();

        renderWindow->Render();

        emit statusUpdateMessage("Updated: " + name, 0);
    }
}

void MainWindow::openOptionDialog()
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        emit statusUpdateMessage("No item selected.", 0);
        return;
    }

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    Option_Dialog dialog(this);
    dialog.setModelPart(selectedPart);

    if (dialog.exec() == QDialog::Accepted) {
        emit statusUpdateMessage("Updated: " + selectedPart->data(0).toString(), 0);
        ui->treeView->model()->dataChanged(index, index);
        renderWindow->Render();
    }
}

void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();

    int rows = partList->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = partList->index(i, 0, QModelIndex());
        updateRenderFromTree(index);
    }

    renderer->ResetCamera();
    renderer->Render();
}

void MainWindow::updateRenderFromTree(const QModelIndex& index)
{
    if (!index.isValid()) return;

    // 1. Standard on-screen rendering
    ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
    vtkSmartPointer<vtkActor> onscreen = part->getActor();
    if (onscreen && part->visible()) {
        renderer->AddActor(onscreen);
    }

    // 2. Build a fresh actor for VR and queue it
    if (vrThread && part->getSource() && part->visible()) {
        // Create a new mapper linked to the original data source
        vtkSmartPointer<vtkPolyDataMapper> vrMapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        vrMapper->SetInputConnection(part->getSource()->GetOutputPort());

        // Create a brand-new actor for VR
        vtkSmartPointer<vtkActor> vrActor =
            vtkSmartPointer<vtkActor>::New();
        vrActor->SetMapper(vrMapper);

        // Copy over any property changes (colour, opacity, etc.)
        vrActor->SetProperty(onscreen->GetProperty());

        // Queue up for the VR render thread
        vrThread->addActorOffline(vrActor);
    }

    // Recurse
    if (!partList->hasChildren(index) ||
        (index.flags() & Qt::ItemNeverHasChildren)) {
        return;
    }
    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; ++i) {
        updateRenderFromTree(partList->index(i, 0, index));
    }
}

void MainWindow::deleteSelectedItem()
{
    QItemSelectionModel* selectionModel = ui->treeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedRows();

    if (selectedIndexes.isEmpty()) {
        emit statusUpdateMessage("No items selected to delete.", 0);
        return;
    }

    // Collect names for confirmation message
    QStringList partNames;
    for (const QModelIndex& index : selectedIndexes) {
        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
        if (part)
            partNames << part->data(0).toString();
    }

    QString message = "Are you sure you want to delete the following " +
        QString::number(partNames.size()) + " items?\n\n" +
        partNames.join("\n");

    if (QMessageBox::question(this, "Confirm Delete", message) != QMessageBox::Yes)
        return;

    // Sort indexes in descending order before deletion
    std::sort(selectedIndexes.begin(), selectedIndexes.end(),
        [](const QModelIndex& a, const QModelIndex& b) {
            return a.row() > b.row();
        });

    for (const QModelIndex& index : selectedIndexes) {
        partList->removeRow(index.row(), index.parent());
    }

    emit statusUpdateMessage("Deleted: " + partNames.join(", "), 0);
    updateRender();
}

void MainWindow::onLoadBackgroundClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Background Image"), "",
        tr("Images (*.png *.jpg *.jpeg)"));

    if (fileName.isEmpty())
        return;

    // Detect file type and read
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

    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    vtkSmartPointer<vtkTexture> backgroundTexture = vtkSmartPointer<vtkTexture>::New();
    backgroundTexture->SetInputConnection(reader->GetOutputPort());

    this->renderer->TexturedBackgroundOn();
    this->renderer->SetBackgroundTexture(backgroundTexture);

    // Refresh render window
    this->renderWindow->Render();
}

void MainWindow::onLoadSkyboxClicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Skybox Folder");
    if (dirPath.isEmpty())
        return;

    std::vector<std::string> faceFilenames = {
        (dirPath + "/px.png").toStdString(),   // +X
        (dirPath + "/nx.png").toStdString(),   // -X
        (dirPath + "/py.png").toStdString(),   // +Y
        (dirPath + "/ny.png").toStdString(),   // -Y
        (dirPath + "/pz.png").toStdString(),   // +Z
        (dirPath + "/nz.png").toStdString()    // -Z
    };

    auto cubemapTexture = LoadCubemapTexture(faceFilenames);
    AddSkyboxToRenderer(renderer, cubemapTexture);
    renderWindow->Render();
}

void MainWindow::onRotationSpeedChanged(int value)
{
    // Map slider value (0 to 100) to a rotation speed (degrees per timer tick)
    rotationSpeed = static_cast<double>(value) * 0.1; // adjust if needed
}

/*void MainWindow::onAutoRotate()
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
    if (actor) {
        actor->RotateY(rotationSpeed);
    }

    this->renderWindow->Render();
}*/

void MainWindow::onAutoRotate()
{
    if (rotationSpeed == 0.0)
        return;

    // Get all selected rows
    QItemSelectionModel* selectionModel = ui->treeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedRows();

    for (const QModelIndex& index : selectedIndexes) {
        if (!index.isValid()) continue;

        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
        if (!part) continue;

        vtkSmartPointer<vtkActor> actor = part->getActor();
        if (actor) {
            actor->RotateY(rotationSpeed);
        }
    }

    renderWindow->Render();
}


void MainWindow::onLightIntensityChanged(int value)
{
    if (!sceneLight) return;

    double intensity = static_cast<double>(value) / 100.0;
    sceneLight->SetIntensity(intensity);
    renderWindow->Render();
}

void MainWindow::on_checkBox_Clip_toggled(bool checked)
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    if (checked) {
        selectedPart->applyClipFilter();
    }
    else {
        selectedPart->removeClipFilter();
    }

    refreshSelectedActor();
}

void MainWindow::on_checkBox_Shrink_toggled(bool checked)
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    if (checked) {
        selectedPart->applyShrinkFilter();
    }
    else {
        selectedPart->removeShrinkFilter();
    }

    refreshSelectedActor();
}

void MainWindow::refreshSelectedActor()
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    vtkSmartPointer<vtkActor> actor = selectedPart->getActor();
    if (!actor) return;

    renderer->RemoveActor(actor);
    renderer->AddActor(actor);

    renderWindow->Render();
}

void MainWindow::handleStartVR() {
    // Create and start the off-thread VR renderer
    vrThread = new VRRenderThread(this);
    // Populate it with all currently visible parts
    updateRenderFromTree(partList->index(0, 0, QModelIndex()));
    vrThread->start();
    emit statusUpdateMessage(QString("VR LOADING.."), 0);
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



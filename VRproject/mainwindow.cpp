// @file MainWindow.cpp
//
// EEEE2076 - Software Engineering & VR Project
//
// Main application window: handles STL loading, tree interaction, rendering, filters, skybox, and VR.

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ModelPart.h"
#include "Option_Dialog.h"
#include "skyboxutils.h"
#include "VRRenderThread.h"

// --------------------------------------- Qt Includes ---------------------------------------

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
#include <QtConcurrent>

// --------------------------------------- VTK Includes ---------------------------------------

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkJPEGReader.h>
#include <vtkPNGReader.h>
#include <vtkLight.h>
#include <vtkImageReader2.h>
#include <vtkClipDataSet.h>
#include <vtkShrinkFilter.h>
#include <vtkPlane.h>
#include <vtkGeometryFilter.h>

// --------------------------------------- Constructor & Setup ---------------------------------------

// Constructs the MainWindow and initializes all UI, rendering, and signals
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rotationTimer(new QTimer(this))
    , rotationSpeed(0.0)
    , sceneLight(nullptr)
{
    ui->setupUi(this);

    // Setup sliders
    ui->horizontalSlider->setRange(0, 100);
    ui->horizontalSlider->setValue(50);

    // Connect UI buttons to actions
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::handleButton);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(ui->actionOpen_File, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->treeView, &QWidget::customContextMenuRequested, this, &MainWindow::showTreeContextMenu);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);
    connect(ui->toggleVR, &QPushButton::released, this, &MainWindow::handleStartVR);
    connect(ui->loadBackgroundButton, &QPushButton::clicked, this, &MainWindow::onLoadBackgroundClicked);
    connect(ui->loadSkyboxButton, &QPushButton::clicked, this, &MainWindow::onLoadSkyboxClicked);
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);

    // Rotation timer and slider
    connect(rotationTimer, &QTimer::timeout, this, &MainWindow::onAutoRotate);
    connect(ui->rotationSpeedSlider, &QSlider::valueChanged, this, &MainWindow::onRotationSpeedChanged);
    rotationTimer->start(16); // ~60 FPS

    // Lighting intensity
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::onLightIntensityChanged);

    // Filter toggles
    connect(ui->checkBox_Clip, &QCheckBox::toggled, this, &MainWindow::on_checkBox_Clip_toggled);
    connect(ui->checkBox_Shrink, &QCheckBox::toggled, this, &MainWindow::on_checkBox_Shrink_toggled);
    connect(ui->exitVRButton, &QPushButton::clicked, this, &MainWindow::onExitVRClicked);

    // Create model part list and link to tree view
    this->partList = new ModelPartList("PartsList");
    ui->treeView->setModel(this->partList);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Setup VTK rendering
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->vtkWidget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Add a sample cylinder model
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(8);

    vtkNew<vtkPolyDataMapper> cylinderMapper;
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    vtkNew<vtkActor> cylinderActor;
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1.0, 0.0, 0.35);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);
    renderer->AddActor(cylinderActor);

    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(30);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->ResetCameraClippingRange();

    // Setup default lighting
    sceneLight = vtkSmartPointer<vtkLight>::New();
    sceneLight->SetLightTypeToSceneLight();
    sceneLight->SetPosition(5, 5, 15);
    sceneLight->SetFocalPoint(0, 0, 0);
    sceneLight->SetDiffuseColor(1, 1, 1);
    sceneLight->SetAmbientColor(1, 1, 1);
    sceneLight->SetSpecularColor(1, 1, 1);
    sceneLight->SetIntensity(0.5);
    renderer->AddLight(sceneLight);

    // Initialize VR thread
    vrThread = new VRRenderThread(this);
}

// Destructor: cleans up UI
MainWindow::~MainWindow()
{
    delete ui;
}

// --------------------------------------- UI Button Handlers ---------------------------------------

// Handles generic button click (test)
void MainWindow::handleButton()
{
    QMessageBox msgBox;
    msgBox.setText("Add button was clicked");
    msgBox.exec();
    emit statusUpdateMessage(QString("Add button was clicked"), 0);
}

// Updates status bar when tree item clicked
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

// --------------------------------------- File Loading ---------------------------------------

// Triggered from menu to open STL files
void MainWindow::on_actionOpen_File_triggered()
{
    emit statusUpdateMessage(QString("Open File action triggered"), 0);
}

// Opens STL files and adds them to the tree
void MainWindow::openFile()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this, tr("Open File"), QDir::homePath(), tr("STL Files (*.stl);;All Files (*)"));

    if (!fileNames.isEmpty()) {
        QModelIndex index = ui->treeView->currentIndex();
        ModelPart* selectedPart = index.isValid()
            ? static_cast<ModelPart*>(index.internalPointer())
            : partList->getRootItem();

        for (const QString& fileName : fileNames) {
            QString shortName = QFileInfo(fileName).fileName();

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
                continue;
            }

            QList<QVariant> data = { shortName, "true" };
            QModelIndex newIndex = partList->appendChild(data);
            ModelPart* newPart = static_cast<ModelPart*>(newIndex.internalPointer());
            newPart->loadSTL(fileName);
        }

        updateRender();
        ui->treeView->expandAll();
    }
}

// --------------------------------------- Dialogs & Tree Context ---------------------------------------

// Opens Option_Dialog from a secondary button
void MainWindow::on_pushButton_2_clicked()
{
    openOptionDialog();
}

// Opens context menu for tree item
void MainWindow::showTreeContextMenu(const QPoint& pos)
{
    QMenu contextMenu(this);
    QAction* itemOptions = new QAction("Item Options", this);
    connect(itemOptions, &QAction::triggered, this, &MainWindow::on_actionItemOptions_triggered);
    contextMenu.addAction(itemOptions);
    contextMenu.exec(ui->treeView->mapToGlobal(pos));
}

// Handles the Option_Dialog for item editing
void MainWindow::on_actionItemOptions_triggered()
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    Option_Dialog dialog(this);
    dialog.setModelPart(selectedPart);
    connect(&dialog, &Option_Dialog::visibilityChanged, this, &MainWindow::onVisibilityChanged);

    if (dialog.exec() == QDialog::Accepted) {
        QString name;
        int r, g, b;
        bool visible;
        dialog.getModelPartData(name, r, g, b, visible);
        selectedPart->setName(name);
        selectedPart->setColor(QColor(r, g, b));
        selectedPart->setVisible(visible);

        emit ui->treeView->model()->dataChanged(index, index, { Qt::DisplayRole, Qt::BackgroundRole });
        ui->treeView->update();
        renderWindow->Render();
        emit statusUpdateMessage("Updated: " + name, 0);
    }
}

// Shows the options dialog directly
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

// --------------------------------------- Rendering ---------------------------------------

// Updates the scene with current model parts
void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();

    // If VR is running, clear old actors
    if (vrThread && vrThread->isRunning())
        vrThread->clearAllActors();  // Must be implemented in VRRenderThread

    // Loop through each top-level item and recursively render
    int rows = partList->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = partList->index(i, 0, QModelIndex());
        updateRenderFromTree(index);
    }

    renderer->ResetCamera();
    renderer->Render();
}

// Recursively updates render and VR actors from the tree
void MainWindow::updateRenderFromTree(const QModelIndex& index)
{
    if (!index.isValid()) return;

    ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
    if (!part) return;

    // Add on-screen actor if visible
    vtkSmartPointer<vtkActor> onscreen = part->getActor();
    if (onscreen && part->visible()) {
        renderer->AddActor(onscreen);
    }

    // Add VR actor if running
    if (vrThread && part->visible()) {
        vtkSmartPointer<vtkActor> vrActor = part->getVRActor();
        if (vrActor)
            vrThread->addActorOffline(vrActor);
    }

    // Recurse into children
    if (!partList->hasChildren(index) ||
        (index.flags() & Qt::ItemNeverHasChildren)) {
        return;
    }

    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; ++i) {
        updateRenderFromTree(partList->index(i, 0, index));
    }
}

// --------------------------------------- Tree Actions ---------------------------------------

// Deletes selected items from the tree and renderer
void MainWindow::deleteSelectedItem()
{
    QItemSelectionModel* selectionModel = ui->treeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedRows();

    if (selectedIndexes.isEmpty()) {
        emit statusUpdateMessage("No items selected to delete.", 0);
        return;
    }

    QStringList partNames;
    for (const QModelIndex& index : selectedIndexes) {
        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
        if (part)
            partNames << part->data(0).toString();
    }

    QString message = "Are you sure you want to delete the following " +
        QString::number(partNames.size()) + " items?\n\n" + partNames.join("\n");

    if (QMessageBox::question(this, "Confirm Delete", message) != QMessageBox::Yes)
        return;

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

// --------------------------------------- Background & Skybox ---------------------------------------

// Loads a static background image and applies it to the renderer
void MainWindow::onLoadBackgroundClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Background Image"), "", tr("Images (*.png *.jpg *.jpeg)"));
    if (fileName.isEmpty()) return;

    vtkSmartPointer<vtkImageReader2> reader;
    if (fileName.endsWith(".jpg", Qt::CaseInsensitive) || fileName.endsWith(".jpeg", Qt::CaseInsensitive)) {
        reader = vtkSmartPointer<vtkJPEGReader>::New();
    }
    else if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
        reader = vtkSmartPointer<vtkPNGReader>::New();
    }
    else {
        return;
    }

    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    vtkSmartPointer<vtkTexture> backgroundTexture = vtkSmartPointer<vtkTexture>::New();
    backgroundTexture->SetInputConnection(reader->GetOutputPort());

    renderer->TexturedBackgroundOn();
    renderer->SetBackgroundTexture(backgroundTexture);
    renderWindow->Render();
}

// Loads a skybox cubemap texture from a folder and applies it
void MainWindow::onLoadSkyboxClicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Skybox Folder");
    if (dirPath.isEmpty()) return;

    std::vector<std::string> faceFilenames = {
        (dirPath + "/px.png").toStdString(),
        (dirPath + "/nx.png").toStdString(),
        (dirPath + "/py.png").toStdString(),
        (dirPath + "/ny.png").toStdString(),
        (dirPath + "/pz.png").toStdString(),
        (dirPath + "/nz.png").toStdString()
    };

    auto cubemapTexture = LoadCubemapTexture(faceFilenames);
    AddSkyboxToRenderer(renderer, cubemapTexture);
    renderWindow->Render();
}

// --------------------------------------- Lighting & Rotation ---------------------------------------

// Updates light intensity using slider value
void MainWindow::onLightIntensityChanged(int value)
{
    if (!sceneLight) return;

    double intensity = static_cast<double>(value) / 100.0;
    sceneLight->SetIntensity(intensity);
    renderWindow->Render();
}

// Updates rotation speed and optionally starts VR thread
void MainWindow::onRotationSpeedChanged(int value)
{
    rotationSpeed = static_cast<double>(value) * 0.1;

    if (vrThread) {
        if (rotationSpeed == 0.0) {
            vrThread->setRotation(0.0, 0.0, 0.0);
        }
        else {
            vrThread->setRotation(0.0, rotationSpeed, 0.0);
            if (vrThread && vrThread->isRunning()) {
                vrThread->setRotation(0.0, rotationSpeed, 0.0);
            }

        }
    }
}

// Automatically rotates selected actors in the scene
void MainWindow::onAutoRotate()
{
    if (rotationSpeed == 0.0)
        return;

    QItemSelectionModel* selectionModel = ui->treeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedRows();

    for (const QModelIndex& index : selectedIndexes) {
        if (!index.isValid()) continue;

        ModelPart* part = static_cast<ModelPart*>(index.internalPointer());
        if (!part) continue;

        vtkSmartPointer<vtkActor> actor = part->getActor();
        if (actor) {
            actor->RotateY(rotationSpeed);  // Apply rotation
        }

        // Only call VR update if the thread exists and is running
        if (vrThread && vrThread->isRunning()) {
            updateRenderFromTree(index);
        }
    }

    renderWindow->Render();
}


// --------------------------------------- Filter Toggles ---------------------------------------

// Applies/removes clip filter from selected model
void MainWindow::on_checkBox_Clip_toggled(bool checked)
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    double origin[3] = { 0.0, 0.0, 0.0 };
    double normal[3] = { 0.0, -1.0, 0.0 };
    selectedPart->applyClipFilter(checked, origin, normal);

    renderWindow->Render();
    if (vrThread && vrThread->isRunning())
        updateRenderFromTree(index);
}

// Applies/removes shrink filter from selected model
void MainWindow::on_checkBox_Shrink_toggled(bool checked)
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) return;

    ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
    if (!selectedPart) return;

    selectedPart->applyShrinkFilter(checked, 0.8);
    renderWindow->Render();

    if (vrThread && vrThread->isRunning())
        updateRenderFromTree(index);
}

// --------------------------------------- VR Thread Management ---------------------------------------

// Starts the VR rendering thread
void MainWindow::handleStartVR()
{
    if (!vrThread->isRunning()) {
        updateRenderFromTree(partList->index(0, 0, QModelIndex()));
        vrThread->start();
        emit statusUpdateMessage(QString("VR LOADING.."), 0);
    }
    else {
        emit statusUpdateMessage(QString("VR already running"), 0);
    }
}

// Gracefully stops the VR thread
void MainWindow::onExitVRClicked()
{
    if (vrThread && vrThread->isRunning()) {
        vrThread->issueCommand(VRRenderThread::END_RENDER, 0);
        vrThread->wait();
        delete vrThread;
        vrThread = nullptr;
    }
}

// Called when visibility checkbox in dialog is changed
void MainWindow::onVisibilityChanged(bool visible)
{
    double visibilityValue = visible ? 1.0 : 0.0;
    if (vrThread && vrThread->isRunning()) {
        vrThread->issueCommand(VRRenderThread::TOGGLE_VISIBILITY, visibilityValue);
    }
}

// --------------------------------------- Actor Refresh ---------------------------------------

// Removes and re-adds selected actor to force rendering refresh
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


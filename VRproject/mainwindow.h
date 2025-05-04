// @file MainWindow.h
//
// EEEE2076 - Software Engineering & VR Project
//
// Main GUI window for STL loading, tree interaction, filtering, rendering, and VR support.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// --------------------------------------- Project Includes ---------------------------------------

#include "ModelPartList.h"      // Custom tree model for parts
#include "ModelPart.h"          // Individual STL part container
#include "VRRenderThread.h"     // Background thread for VR rendering

// --------------------------------------- Qt Includes ---------------------------------------

#include <QMainWindow>          // Base class for main application window
#include <QTreeView>            // For model hierarchy view
#include <QTimer>               // Used for auto-rotation
#include <QSlider>              // For light and rotation controls
#include <QCheckBox>            // For filter toggles
#include <QVTKOpenGLNativeWidget.h> // VTK-Qt render widget
#include <QVTKInteractor.h>     // VTK event interactor

// --------------------------------------- VTK Includes ---------------------------------------

#include <vtkSmartPointer.h>                 // Smart pointer for VTK memory management
#include <vtkRenderer.h>                     // Scene renderer
#include <vtkGenericOpenGLRenderWindow.h>    // Render window
#include <vtkLight.h>                        // Lighting

#include <vtkClipDataSet.h>                  // For clipping filter
#include <vtkShrinkFilter.h>                 // For shrinking geometry
#include <vtkPlane.h>                        // For defining clip planes
#include <vtkGeometryFilter.h>               // Converts datasets to polygonal data

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

// --------------------------------------- MainWindow Class ---------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor: sets up UI, rendering, tree model, and signals
    MainWindow(QWidget* parent = nullptr);

    // Destructor: cleans up UI
    ~MainWindow();

public slots:

    // Starts the VR thread if not already running
    void handleStartVR();

    // Handles general UI test button
    void handleButton();

    // Updates status when tree item is clicked
    void handleTreeClicked();

    // Triggered from file menu (Open File)
    void on_actionOpen_File_triggered();

    // Opens one or more STL files and adds to tree
    void openFile();

    // Opens the option dialog for selected item
    void openOptionDialog();

    // Shows right-click menu in tree view
    void showTreeContextMenu(const QPoint& pos);

    // Opens item options from context menu
    void on_actionItemOptions_triggered();

    // Rebuilds the 3D scene from tree structure
    void updateRender();

    // Recursively adds actors from a given tree index
    void updateRenderFromTree(const QModelIndex&);

    // --------------------------------------- Background & Skybox ---------------------------------------

    // Loads a static background image
    void onLoadBackgroundClicked();

    // Loads a cubemap skybox from 6 images
    void onLoadSkyboxClicked();

    // --------------------------------------- Rotation ---------------------------------------

    // Sets the auto-rotation speed from UI
    void onRotationSpeedChanged(int value);

    // Rotates all selected parts at regular intervals
    void onAutoRotate();

    // --------------------------------------- Lighting ---------------------------------------

    // Updates global light intensity
    void onLightIntensityChanged(int value);

    // --------------------------------------- Render Utility ---------------------------------------

    // Forces re-adding the selected actor
    void refreshSelectedActor();

    // Called when visibility toggled in Option_Dialog
    void onVisibilityChanged(bool visible);

signals:

    // Emits messages to show in the status bar
    void statusUpdateMessage(const QString& message, int timeout);

private slots:

    // Handles Option_Dialog via button 2
    void on_pushButton_2_clicked();

    // Deletes selected items from tree
    void deleteSelectedItem();

    // Toggles clip filter on selected part
    void on_checkBox_Clip_toggled(bool checked);

    // Toggles shrink filter on selected part
    void on_checkBox_Shrink_toggled(bool checked);

    // Shuts down the VR thread
    void onExitVRClicked();

private:

    // --------------------------------------- UI & Tree Model ---------------------------------------

    Ui::MainWindow* ui;           // Pointer to Qt-generated UI class
    ModelPartList* partList;      // Custom tree model managing ModelPart items

    // --------------------------------------- VTK Rendering ---------------------------------------

    vtkSmartPointer<vtkRenderer> renderer;                        // Scene renderer
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;  // Render window for 3D view
    vtkSmartPointer<vtkLight> sceneLight;                        // Global lighting object

    // --------------------------------------- Rotation ---------------------------------------

    QTimer* rotationTimer;    // Timer used for rotation animation
    double rotationSpeed;     // Speed of auto-rotation

    // --------------------------------------- VR Support ---------------------------------------

    VRRenderThread* vrThread; // Background thread for VR rendering
};

#endif // MAINWINDOW_H

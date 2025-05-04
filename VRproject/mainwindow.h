// @file MainWindow.h
//
// EEEE2076 - Software Engineering & VR Project
//
// Main GUI window for STL loading, tree interaction, filtering, rendering, and VR support.

/**
 * @file mainwindow.h
 * @brief Main GUI window for STL loading, tree interaction, filtering, rendering, and VR support.
 *
 * EEEE2076 – Software Engineering & VR Project
 */

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

/**
 * @class MainWindow
 * @brief Main application window, inherits QMainWindow.
 *
 * Sets up the UI, the VTK rendering context, the model-tree view, and provides slots for:
 *  - Opening STL files
 *  - Managing the parts tree
 *  - Applying filters (clip, shrink)
 *  - Controlling lighting and auto-rotation
 *  - Starting and stopping VR rendering
 */
// --------------------------------------- MainWindow Class ---------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the MainWindow and initializes UI components and VTK rendering.
     * @param parent  Parent widget for Qt ownership (defaults to nullptr).
     */

    // Constructor: sets up UI, rendering, tree model, and signals
    MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor: cleans up UI and allocated resources.
     */

    // Destructor: cleans up UI
    ~MainWindow();

public slots:
    /**
     * @brief Starts the VR rendering thread if it is not already running.
     */

    // Starts the VR thread if not already running
    void handleStartVR();

    /**
     * @brief Generic handler for the test button in the UI.
     *
     * Pops up a message box and emits a status bar message.
     */

    // Handles general UI test button
    void handleButton();

    /**
     * @brief Updates the status bar when a tree item is clicked.
     */

    // Updates status when tree item is clicked
    void handleTreeClicked();

    /**
     * @brief Slot connected to the “Open File” action in the File menu.
     */

    // Triggered from file menu (Open File)
    void on_actionOpen_File_triggered();

    /**
    * @brief Opens one or more STL files and adds them to the model tree.
    */

    // Opens one or more STL files and adds to tree
    void openFile();

    /**
    * @brief Opens the options dialog for the currently selected tree item.
    */

    // Opens the option dialog for selected item
    void openOptionDialog();

    /**
    * @brief Shows a custom context menu at the given position in the tree view.
    * @param pos  Position (in tree-view coordinates) where the menu appears.
    */

    // Shows right-click menu in tree view
    void showTreeContextMenu(const QPoint& pos);

    /**
     * @brief Handler for the “Item Options” action from the context menu.
     */

    // Opens item options from context menu

    void on_actionItemOptions_triggered();
    /**
     * @brief Clears and rebuilds the 3D scene from the tree model.
     */

    // Rebuilds the 3D scene from tree structure
    void updateRender();

    /**
    * @brief Recursively adds actors from the model tree into the renderer (and VR thread).
    * @param index  QModelIndex identifying the node to process.
    */

    // Recursively adds actors from a given tree index
    void updateRenderFromTree(const QModelIndex&);

    // --------------------------------------- Background & Skybox ---------------------------------------
    /**
     * @brief Opens a file dialog to select and load a static background image.
     */

    // Loads a static background image
    void onLoadBackgroundClicked();

    /**
    * @brief Opens a directory dialog to select a cubemap folder and applies it as a skybox.
    */

    // Loads a cubemap skybox from 6 images
    void onLoadSkyboxClicked();

    // --------------------------------------- Rotation ---------------------------------------
     /**
     * @brief Sets the auto-rotation speed from the UI slider.
     * @param value  Slider value (mapped internally to degrees per frame).
     */

    // Sets the auto-rotation speed from UI
    void onRotationSpeedChanged(int value);

    /**
     * @brief Called periodically to rotate all selected parts by the current speed.
     */

    // Rotates all selected parts at regular intervals
    void onAutoRotate();

    // --------------------------------------- Lighting ---------------------------------------
    /**
     * @brief Updates the global scene light intensity based on the slider.
     * @param value  Slider value in [0–100].
     */

    // Updates global light intensity
    void onLightIntensityChanged(int value);

    // --------------------------------------- Render Utility ---------------------------------------
    /**
     * @brief Forces re-adding the currently selected actor to the renderer.
     */

    // Forces re-adding the selected actor
    void refreshSelectedActor();

    /**
     * @brief Reacts to visibility changes from the Option_Dialog for VR.
     * @param visible  True to show the part in VR, false to hide.
     */

    // Called when visibility toggled in Option_Dialog
    void onVisibilityChanged(bool visible);

signals:
    /**
    * @brief Emitted to display a message in the status bar.
    * @param message  Text to display.
    * @param timeout  Duration in milliseconds before the message disappears (0 = stay until replaced).
    */

    // Emits messages to show in the status bar
    void statusUpdateMessage(const QString& message, int timeout);

private slots:
    /**
     * @brief Handler for the secondary push button to open Option_Dialog.
     */

    // Handles Option_Dialog via button 2
    void on_pushButton_2_clicked();

    /**
     * @brief Deletes the currently selected items from the model tree and renderer.
     */

    // Deletes selected items from tree
    void deleteSelectedItem();

    /**
     * @brief Toggles a clipping filter on the selected model part.
     * @param checked  True to apply clipping, false to remove it.
     */

    // Toggles clip filter on selected part
    void on_checkBox_Clip_toggled(bool checked);

    /**
    * @brief Toggles a shrink filter on the selected model part.
    * @param checked  True to apply shrinking, false to remove it.
    */
    // Toggles shrink filter on selected part

    void on_checkBox_Shrink_toggled(bool checked);

    /**
     * @brief Stops and cleans up the VR rendering thread.
     */

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

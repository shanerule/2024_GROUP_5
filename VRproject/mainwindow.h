#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ModelPartList.h"
#include "ModelPart.h"
#include "VRRenderThread.h"

#include <QMainWindow>
#include <QTreeView>
#include <QTimer>
#include <QSlider>
#include <QCheckBox>
#include <QVTKOpenGLNativeWidget.h>
#include <QVTKInteractor.h>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkLight.h>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void handleStartVR();
    void handleButton();
    void handleTreeClicked();
    void on_actionOpen_File_triggered();
    void openFile();
    void openOptionDialog();
    void showTreeContextMenu(const QPoint& pos);
    void on_actionItemOptions_triggered();
    void updateRender();
    void updateRenderFromTree(const QModelIndex&);

    // Background & Skybox
    void onLoadBackgroundClicked();
    void onLoadSkyboxClicked();

    // Rotation
    void onRotationSpeedChanged(int value);
    void onAutoRotate();

    // Light intensity
    void onLightIntensityChanged(int value);

    // Clip/Shrink filters
    void on_checkBox_Clip_toggled(bool checked);
    void on_checkBox_Shrink_toggled(bool checked);
    void refreshSelectedActor();


signals:
    void statusUpdateMessage(const QString& message, int timeout);

private slots:
    void on_pushButton_2_clicked();
    void deleteSelectedItem();

private:
    Ui::MainWindow* ui;
    ModelPartList* partList;

    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkLight> sceneLight;

    // Rotation timer and speed
    QTimer* rotationTimer;
    double rotationSpeed;
    VRRenderThread* vrThread;
};

#endif // MAINWINDOW_H

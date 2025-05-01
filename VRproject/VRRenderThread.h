/** @file VRRenderThread.h
  * @brief EEEE2046 - Software Engineering & VR Project
  * Template to add VR rendering to your application.
  * P Evans 2022
  */

#ifndef VR_RENDER_THREAD_H
#define VR_RENDER_THREAD_H

/* Project headers */

/* Qt headers */
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/* Vtk headers */
#include <vtkActor.h>
#include <vtkOpenVRRenderWindow.h>				
#include <vtkOpenVRRenderWindowInteractor.h>	
#include <vtkOpenVRRenderer.h>					
#include <vtkOpenVRCamera.h>	
#include <vtkActorCollection.h>
#include <vtkCommand.h>

/**
 * @class VRRenderThread
 * @brief The VRRenderThread class inherits from the Qt class QThread which allows it to be a parallel thread to the main() thread, and also from vtkCommand which allows it to act as a "callback" for the vtkRenderWindowInteractor. This callback functionality means that once the renderWindowInteractor takes control of this thread to enable VR, it can callback to a function in the class to check to see if the user has requested any changes.
 */
class VRRenderThread : public QThread {
    Q_OBJECT

public:
    /**
     * @enum Command
     * @brief List of command names.
     */
    enum {
        END_RENDER,
        ROTATE_X,
        ROTATE_Y,
        ROTATE_Z
    } Command;

    /**
     * @brief Constructor for the VRRenderThread class.
     * @param parent is a pointer to the parent QObject.
     */
    VRRenderThread(QObject* parent = nullptr);

    /**
     * @brief Destructor for the VRRenderThread class.
     */
    ~VRRenderThread();

    /**
     * @brief This function allows actors to be added to the VR renderer BEFORE the VR interactor has been started.
     * @param actor is a pointer to the vtkActor to be added.
     */
    void addActorOffline(vtkActor* actor);

    /**
     * @brief This function allows commands to be issued to the VR thread in a thread safe way. Function will set variables within the class to indicate the type of action / animation / etc to perform. The rendering thread will then implement this.
     * @param cmd is the command to be issued.
     * @param value is the value associated with the command.
     */
    void issueCommand( int cmd, double value );

protected:
    /**
     * @brief This function is a re-implementation of a QThread function.
     */
    void run() override;

private:
    /* Standard VTK VR Classes */
    vtkSmartPointer<vtkOpenVRRenderWindow>              window; /**< A smart pointer to the VR render window. */
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor>    interactor; /**< A smart pointer to the VR render window interactor. */
    vtkSmartPointer<vtkOpenVRRenderer>                  renderer; /**< A smart pointer to the VR renderer. */
    vtkSmartPointer<vtkOpenVRCamera>                    camera; /**< A smart pointer to the VR camera. */

    /* Use to synchronise passing of data to VR thread */
    QMutex                                              mutex; /**< A mutex for synchronising passing of data to VR thread. */
    QWaitCondition                                      condition; /**< A wait condition for synchronising passing of data to VR thread. */

    /** List of actors that will need to be added to the VR scene */
    vtkSmartPointer<vtkActorCollection>                 actors; /**< A smart pointer to the list of actors that will need to be added to the VR scene. */

    /** A timer to help implement animations and visual effects */
    std::chrono::time_point<std::chrono::steady_clock>  t_last; /**< A timer to help implement animations and visual effects. */

    /** This will be set to false by the constructor, if it is set to true by the GUI then the rendering will end. */
    bool                                                endRender; /**< A boolean value that indicates whether the rendering will end. */

    /* Some variables to indicate animation actions to apply. */
    double rotateX; /**< Degrees to rotate around X axis (per time-step). */
    double rotateY; /**< Degrees to rotate around Y axis (per time-step). */
    double rotateZ; /**< Degrees to rotate around Z axis (per time-step). */
};

#endif

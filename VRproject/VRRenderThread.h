/**
 * @file VRRenderThread.cpp
 * @brief EEEE2046 - Software Engineering & VR Project
 * Adds OpenVR-based rendering support in a separate thread using Qt + VTK
 * Paul Evans 2022
 */

#ifndef VR_RENDER_THREAD_H
#define VR_RENDER_THREAD_H

// --------------------------------------- Qt Includes ---------------------------------------

#include <QThread>           // For running VR in a separate thread
#include <QVector>           // Container for queued actors
#include <QMutex>            // Thread synchronization
#include <QWaitCondition>    // Thread wait signaling

// --------------------------------------- VTK Includes ---------------------------------------

#include <vtkActor.h>                        // Basic renderable entity
#include <vtkActorCollection.h>              // Collection of actors
#include <vtkCommand.h>                      // For callback support
#include <vtkSmartPointer.h>                 // Smart pointer management
#include <vtkOpenVRRenderWindow.h>           // OpenVR-compatible render window
#include <vtkOpenVRRenderWindowInteractor.h> // VR interactor (event loop)
#include <vtkOpenVRRenderer.h>               // VR renderer
#include <vtkOpenVRCamera.h>                 // VR camera

#include <chrono>                            // Used for animation timing

// --------------------------------------- VRRenderThread Class ---------------------------------------
/**
 * @class VRRenderThread
 * @brief Runs a dedicated thread for rendering VTK scenes in VR using OpenVR.
 */
class VRRenderThread : public QThread {
    Q_OBJECT

public:
    /**
    * @brief Enumeration of commands that can be issued to the VR thread.
    */
    // Commands that can be sent to the VR thread
    enum {
        END_RENDER,         // Stop rendering
        ROTATE_X,           // Rotate actor(s) around X axis
        ROTATE_Y,           // Rotate actor(s) around Y axis
        ROTATE_Z,           // Rotate actor(s) around Z axis
        TOGGLE_VISIBILITY   // Toggle visibility on/off
    } Command;

    /**
     * @brief Constructor: initializes member variables.
     * @param parent Optional QObject parent.
     */
    // Constructor: initializes thread and renderer
    VRRenderThread(QObject* parent = nullptr);

    /**
     * @brief Destructor: cleans up resources.
     */
    // Destructor: cleans up VTK resources
    ~VRRenderThread();

    /**
     * @brief Adds an actor to the scene before the VR thread starts.
     * @param actor Pointer to the vtkActor to add.
     */
    // Adds an actor before the VR interactor starts
    void addActorOffline(vtkActor* actor);

    /**
     * @brief Issues a command to the VR rendering thread.
     * @param cmd Command enum (e.g., ROTATE_X, TOGGLE_VISIBILITY).
     * @param value Value associated with the command.
     */
    // Issues a command to the VR thread (thread-safe)
    void issueCommand(int cmd, double value);

    /**
     * @brief Sets the rotation speed per update on each axis.
     * @param x Degrees per update around X.
     * @param y Degrees per update around Y.
     * @param z Degrees per update around Z.
     */
    // Sets rotation speed (degrees per update) on each axis
    void setRotation(double x, double y, double z);

    
public slots:
    /**
     * @brief Clears all queued actors (thread-safe).
     */
    // Removes all actors from the VR renderer
    void clearAllActors();

protected:
    /**
     * @brief Main entry point for the VR rendering thread.
     *        Initializes and runs the OpenVR event loop.
     */
    // Entry point for the VR rendering thread
    void run() override;

private:
    // --------------------------------------- VTK VR Components ---------------------------------------

    vtkSmartPointer<vtkOpenVRRenderWindow> window;            // OpenVR-compatible render window
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> interactor; // VR event loop handler
    vtkSmartPointer<vtkOpenVRRenderer> renderer;              // Scene renderer for VR
    vtkSmartPointer<vtkOpenVRCamera> camera;                  // VR camera

    // --------------------------------------- Thread Synchronization ---------------------------------------

    QMutex mutex;                    // Protects shared resources from race conditions
    QWaitCondition condition;        // Used to wake/sleep the thread safely

    // --------------------------------------- Actor Management ---------------------------------------

    QVector<vtkSmartPointer<vtkActor>> queuedActors;  // Actors added before interactor starts
    vtkSmartPointer<vtkActorCollection> actors;       // All actors currently in the VR scene

    // --------------------------------------- State & Animation ---------------------------------------

    std::chrono::time_point<std::chrono::steady_clock> t_last;  // Used for animation timing
    bool endRender;     // True when rendering should stop

    double rotateX;     // Degrees per step around X axis
    double rotateY;     // Degrees per step around Y axis
    double rotateZ;     // Degrees per step around Z axis
};

#endif // VR_RENDER_THREAD_H

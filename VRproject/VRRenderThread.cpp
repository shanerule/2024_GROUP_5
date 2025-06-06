/**
 * @file VRRenderThread.cpp
 * @brief EEEE2046 - Software Engineering & VR Project
 * Template to add VR rendering to your application
 * Paul Evans 2022
 */

#include "VRRenderThread.h"

// --------------------------------------- VTK Includes ---------------------------------------

#include <vtkActor.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRCamera.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkNamedColors.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSTLReader.h>
#include <vtkDataSetMapper.h>
#include <vtkCallbackCommand.h>

#include <QMutexLocker>

// --------------------------------------- Constructor ---------------------------------------

/**
 * @brief Constructor: Initializes the actor collection and rotation values.
 * @param parent The QObject parent.
 */
// Initializes the actor collection and rotation values
VRRenderThread::VRRenderThread(QObject* parent) {
    actors = vtkActorCollection::New();
    rotateX = 0.;
    rotateY = 0.;
    rotateZ = 0.;
}

// --------------------------------------- Destructor ---------------------------------------

/**
 * @brief Destructor: Smart pointers handle cleanup.
 */
VRRenderThread::~VRRenderThread() {
    // Empty � smart pointers handle cleanup
}

// --------------------------------------- Add Actor Before Render Starts ---------------------------------------

/**
 * @brief Adds a VTK actor to the scene before the VR thread starts.
 * @param actor The actor to add.
 */
// Adds an actor before the VR thread begins
void VRRenderThread::addActorOffline(vtkActor* actor) {
    if (!this->isRunning()) {
        double* ac = actor->GetOrigin();

        // Apply transform to place model in viewable position
        actor->RotateX(-90);
        actor->AddPosition(-ac[0]+0, -ac[1]+0, -ac[2]+0);

        actors->AddItem(actor);
    }
}

// --------------------------------------- Issue Command ---------------------------------------

/**
 * @brief Issues a command to the VR thread (e.g., rotate, toggle visibility).
 * @param cmd The command ID.
 * @param value Command value (angle or visibility flag).
 */
// Allows GUI to send commands to the VR thread (e.g., rotate or quit)
void VRRenderThread::issueCommand(int cmd, double value) {
    switch (cmd) {
    case END_RENDER:
        this->endRender = true;
        break;
    case ROTATE_X:
        this->rotateX = value;
        break;
    case ROTATE_Y:
        this->rotateY = value;
        break;
    case ROTATE_Z:
        this->rotateZ = value;
        break;
    case TOGGLE_VISIBILITY:
        // Toggle visibility for all actors
        actors->InitTraversal();
        vtkActor* actor = nullptr;
        while ((actor = actors->GetNextActor())) {
            bool visible = value > 0.5;
            actor->SetVisibility(visible ? 1 : 0);
        }
        break;
    }
}

// --------------------------------------- VR Render Thread Entry ---------------------------------------
/**
 * @brief Main entry point for the VR rendering thread. Runs the render loop.
 */
// Starts the VR render loop in a separate thread
void VRRenderThread::run() {
    vtkNew<vtkNamedColors> colors;

    std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
    colors->SetColor("BkgColor", bkg.data());

    // Create renderer and apply background
    renderer = vtkOpenVRRenderer::New();
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    // Add all actors to the renderer
    vtkActor* a;
    actors->InitTraversal();
    while ((a = actors->GetNextActor())) {
        renderer->AddActor(a);
    }

    // Setup render window
    window = vtkOpenVRRenderWindow::New();
    window->Initialize();
    window->AddRenderer(renderer);

    // Setup VR camera
    camera = vtkOpenVRCamera::New();
    renderer->SetActiveCamera(camera);

    // Setup interactor
    interactor = vtkOpenVRRenderWindowInteractor::New();
    interactor->SetRenderWindow(window);
    interactor->Initialize();
    window->Render();

    // Start VR render loop
    endRender = false;
    t_last = std::chrono::steady_clock::now();

    while (!interactor->GetDone() && !this->endRender) {
        interactor->DoOneEvent(window, renderer);

        // Check if 20ms have passed since last frame
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t_last).count() > 20) {

            // Rotate all actors on X axis
            vtkActorCollection* actorList = renderer->GetActors();
            actorList->InitTraversal();
            while ((a = actorList->GetNextActor())) {
                a->RotateX(rotateX);
            }

            // Rotate all actors on Y axis
            actorList->InitTraversal();
            while ((a = actorList->GetNextActor())) {
                a->RotateY(rotateY);
            }

            // Rotate all actors on Z axis
            actorList->InitTraversal();
            while ((a = actorList->GetNextActor())) {
                a->RotateZ(rotateZ);
            }

            // Reset last update time
            t_last = std::chrono::steady_clock::now();
        }
    }
}

// --------------------------------------- Clear All Actors ---------------------------------------
/**
 * @brief Clears all queued actors (thread-safe).
 */
// Clears queued actors (thread-safe)
void VRRenderThread::clearAllActors() {
    QMutexLocker locker(&mutex);
    queuedActors.clear();
}

// --------------------------------------- Set Rotation ---------------------------------------
/**
 * @brief Sets the per-frame rotation speeds along each axis.
 * @param x Rotation speed along X.
 * @param y Rotation speed along Y.
 * @param z Rotation speed along Z.
 */
// Sets new rotation values to apply to all actors
void VRRenderThread::setRotation(double x, double y, double z) {
    this->rotateX = x;
    this->rotateY = y;
    this->rotateZ = z;
}

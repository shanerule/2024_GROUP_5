/**
 * @file skyboxutils.cpp
 * @brief Implementation of skybox utility functions for VTK using cubemap textures.
 *
 * This file defines functions to load six images into a cubemap texture and
 * render it as a skybox using VTK. These functions enable realistic background
 * environments in a 3D scene using OpenGL-based textures and VTK rendering pipeline.
 */

#include "skyboxutils.h"

// --------------------------------------- VTK Includes ---------------------------------------

#include <vtkSkybox.h>                  // Actor representing the skybox
#include <vtkImageReader2Factory.h>     // Factory to detect image format
#include <vtkImageReader2.h>            // Base class for image readers
#include <vtkImageFlip.h>               // Used to vertically flip textures
#include <vtkOpenGLTexture.h>           // For cubemap texture support
#include <vtkRenderer.h>                // VTK scene renderer
#include <iostream>                     // For error logging

// --------------------------------------- LoadCubemapTexture ---------------------------------------
/**
 * @brief Loads six images into a cubemap texture used for skybox rendering.
 *
 * The images must be ordered as: +X, -X, +Y, -Y, +Z, -Z (i.e., px, nx, py, ny, pz, nz).
 * Each image is read, flipped along the Y-axis if needed, and assigned to the appropriate face.
 *
 * @param faceFilenames A vector of six image file paths.
 * @return vtkSmartPointer<vtkOpenGLTexture> A cubemap texture with all six faces assigned.
 */
// Loads six images as cubemap texture faces (px, nx, py, ny, pz, nz)
vtkSmartPointer<vtkOpenGLTexture> LoadCubemapTexture(const std::vector<std::string>& faceFilenames) {

    auto texture = vtkSmartPointer<vtkOpenGLTexture>::New();
    texture->CubeMapOn();                     // Enable cubemap mode
    texture->SetUseSRGBColorSpace(true);      // Use sRGB color for correct brightness
    texture->InterpolateOn();                 // Smooth texture sampling
    texture->RepeatOff();                     // Prevent edge wrapping artifacts
    texture->MipmapOff();                     // Disable mipmaps (not needed for skybox)

    // Load and assign all six faces
    for (int i = 0; i < 6; ++i) {
        vtkSmartPointer<vtkImageReader2Factory> readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
        vtkImageReader2* reader = readerFactory->CreateImageReader2(faceFilenames[i].c_str());

        if (!reader) {
            std::cerr << "Failed to load cubemap face: " << faceFilenames[i] << std::endl;
            continue;
        }

        reader->SetFileName(faceFilenames[i].c_str());
        reader->Update();

        // Flip image vertically if needed (some formats are upside down)
        auto flipY = vtkSmartPointer<vtkImageFlip>::New();
        flipY->SetInputConnection(reader->GetOutputPort());
        flipY->SetFilteredAxis(1);  // 1 = Y-axis
        flipY->Update();

        texture->SetInputConnection(i, flipY->GetOutputPort());
        reader->Delete();  // Manually delete because CreateImageReader2 returns raw pointer
    }

    return texture;
}

// --------------------------------------- AddSkyboxToRenderer ---------------------------------------
/**
 * @brief Adds a skybox actor to a renderer using the provided cubemap texture.
 *
 * The skybox is added as a background element with cube projection and gamma correction enabled.
 *
 * @param renderer Pointer to the VTK renderer.
 * @param cubemapTexture The cubemap texture returned by LoadCubemapTexture().
 */
// Adds a skybox actor to the renderer using the given cubemap texture
void AddSkyboxToRenderer(vtkRenderer* renderer, vtkTexture* cubemapTexture) {
    auto skybox = vtkSmartPointer<vtkSkybox>::New();
    skybox->SetTexture(cubemapTexture);      // Assign cubemap texture
    skybox->SetProjectionToCube();           // Use cube projection mode
    skybox->GammaCorrectOn();                // Enable gamma correction

    renderer->AddActor(skybox);              // Add skybox to the scene
}

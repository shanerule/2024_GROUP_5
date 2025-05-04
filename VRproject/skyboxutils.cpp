// @file skyboxutils.cpp
// EEEE2076 - Software Engineering & VR Project
// Implements utility functions for applying a cubemap-based skybox in VTK

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
// Adds a skybox actor to the renderer using the given cubemap texture
void AddSkyboxToRenderer(vtkRenderer* renderer, vtkTexture* cubemapTexture) {
    auto skybox = vtkSmartPointer<vtkSkybox>::New();
    skybox->SetTexture(cubemapTexture);      // Assign cubemap texture
    skybox->SetProjectionToCube();           // Use cube projection mode
    skybox->GammaCorrectOn();                // Enable gamma correction

    renderer->AddActor(skybox);              // Add skybox to the scene
}

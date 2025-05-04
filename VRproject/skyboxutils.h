// @file skyboxutils.h
//
// EEEE2076 - Software Engineering & VR Project
//
// Utility functions for loading and rendering a cubemap skybox in VTK.

#pragma once

// --------------------------------------- Standard Includes ---------------------------------------

#include <vector>     // Used for passing six image paths (px, nx, py, ny, pz, nz)
#include <string>     // For image file path strings

// --------------------------------------- VTK Includes ---------------------------------------

#include <vtkSmartPointer.h>        // Smart pointer management for VTK
#include <vtkRenderer.h>            // For rendering scene
#include <vtkOpenGLTexture.h>       // OpenGL-compatible texture (for cubemap)

// --------------------------------------- Function Declarations ---------------------------------------

/**
 * Loads six images into a cubemap texture.
 * @param faceFilenames Vector of 6 file paths in order: px, nx, py, ny, pz, nz
 * @return vtkOpenGLTexture* representing the cubemap texture
 */
vtkSmartPointer<vtkOpenGLTexture> LoadCubemapTexture(const std::vector<std::string>& faceFilenames);

/**
 * Applies a skybox to the given renderer using a cubemap texture.
 * @param renderer VTK renderer to apply skybox to
 * @param cubemapTexture Cubemap texture created by LoadCubemapTexture()
 */
void AddSkyboxToRenderer(vtkRenderer* renderer, vtkTexture* cubemapTexture);

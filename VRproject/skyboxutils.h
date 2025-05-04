/**
 * @file skyboxutils.h
 * @brief Utility functions for loading and applying a cubemap-based skybox in VTK.
 *
 * This header provides functions to load six images as a cubemap texture and apply
 * it as a skybox to a VTK renderer. It supports `.png`, `.jpg`, and other formats
 * recognized by VTK's image reader factory.
 *
 */

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
 * @brief Loads six images and creates a cubemap texture for skybox rendering.
 *
 * The filenames should be passed in this order:
 * - Positive X (px)
 * - Negative X (nx)
 * - Positive Y (py)
 * - Negative Y (ny)
 * - Positive Z (pz)
 * - Negative Z (nz)
 *
 * @param faceFilenames A vector of 6 file paths representing cubemap faces.
 * @return vtkSmartPointer to the generated cubemap texture.
 */
vtkSmartPointer<vtkOpenGLTexture> LoadCubemapTexture(const std::vector<std::string>& faceFilenames);

/**
 * @brief Adds a cubemap-based skybox to the given VTK renderer.
 *
 * The cubemap texture is rendered as the background of the 3D scene.
 *
 * @param renderer The VTK renderer to which the skybox will be added.
 * @param cubemapTexture The texture returned from LoadCubemapTexture().
 */
void AddSkyboxToRenderer(vtkRenderer* renderer, vtkTexture* cubemapTexture);

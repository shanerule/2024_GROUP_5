// @file ModelPart.h
//
// EEEE2076 - Software Engineering & VR Project
//
// Template for model parts that will be added as treeview items
//
// P Evans 2022

#ifndef VIEWER_MODELPART_H  
#define VIEWER_MODELPART_H  

#pragma once  

// --------------------------------------- Qt Headers ---------------------------------------

#include <QString>        // For part names and file paths
#include <QList>          // For storing child items
#include <QVariant>       // For storing heterogeneous tree data
#include <QColor>         // For storing/displaying part color
#include <QBrush>         // For use with Qt display roles (e.g., tree view background)

// --------------------------------------- VTK Headers ---------------------------------------

#include <vtkSmartPointer.h>      // Smart pointer wrapper for VTK objects
#include <vtkSTLReader.h>         // Reads .stl mesh files
#include <vtkPolyDataMapper.h>    // Maps polygonal data to graphics primitives
#include <vtkActor.h>             // Represents an object in the scene
#include <vtkAlgorithm.h>         // Base class for all VTK pipeline components
#include <vtkClipDataSet.h>       // Clips mesh geometry with a plane
#include <vtkShrinkFilter.h>      // Shrinks cells in the mesh
#include <vtkPlane.h>             // Defines clipping planes
#include <vtkPolyData.h>          // Stores polygonal mesh data
#include <vtkGeometryFilter.h>    // Converts non-poly data to polygonal form
#include <vtkPolyDataNormals.h>   // Computes surface normals for shading

class ModelPart {
public:

    // --------------------------------------- Constructor & Destructor ---------------------------------------

        // Constructs a ModelPart with given data and optional parent
    ModelPart(const QList<QVariant>& data, ModelPart* parent = nullptr);

    // Destructor that cleans up child items and VTK resources
    ~ModelPart();

    // --------------------------------------- Tree Structure Methods ---------------------------------------

        // Adds a child part to this part
    void appendChild(ModelPart* item);

    // Returns the child part at the given row
    ModelPart* child(int row);

    // Returns the number of children
    int childCount() const;

    // Returns the number of columns in the data
    int columnCount() const;

    // Returns the parent of this item
    ModelPart* parentItem();

    // Returns this item's index under its parent
    int row() const;

    // Removes the child at the specified row
    void removeChild(int row);

    // --------------------------------------- Data Handling ---------------------------------------

        // Returns the data stored at a column (used in Qt views)
    QVariant data(int column, int role = Qt::DisplayRole) const;

    // Sets the value for a specific column
    void set(int column, const QVariant& value);

    // Sets the display name of this part (column 0)
    void setName(const QString& newName);

    // --------------------------------------- Visual Properties ---------------------------------------

        // Returns the current background color of the item
    QColor getColor() const;

    // Sets the background color of the item
    void setColor(const QColor& color);

    // Sets the visibility of the part in the scene
    void setVisible(bool visible);

    // Returns the visibility status
    bool visible() const;

    // --------------------------------------- VTK Source & Actor Access ---------------------------------------

        // Returns the VTK actor used for rendering
    vtkSmartPointer<vtkActor> getActor() const;

    // Returns the VTK data source (e.g., STL reader)
    vtkAlgorithm* getSource() const;

    // Returns the output port from the current pipeline stage
    vtkAlgorithmOutput* getOutputPort() const;

    // Returns a separate actor for VR rendering
    vtkSmartPointer<vtkActor> getVRActor();

    // --------------------------------------- STL Loading ---------------------------------------

        // Loads an STL file and sets up the rendering pipeline
    void loadSTL(QString fileName);

    // --------------------------------------- Filter Handling ---------------------------------------

        // Enables/disables clip filter with given origin and normal
    void applyClipFilter(bool enable, double origin[3], double normal[3]);

    // Enables/disables shrink filter with specified factor
    void applyShrinkFilter(bool enable, double factor);

    // Returns true if clip filter is active
    bool isClipFilterEnabled() const;

    // Returns true if shrink filter is active
    bool isShrinkFilterEnabled() const;

    // Updates the filter chain based on active filters
    void updateFilters();

    // --------------------------------------- Original Data Backup ---------------------------------------

        // Checks if original (unfiltered) polydata exists
    bool hasOriginalData() const;

    // Stores the original polydata before any filtering
    void storeOriginalData();

    // Returns the original polydata
    vtkSmartPointer<vtkPolyData> getOriginalPolyData() const;

private:

    // --------------------------------------- Member Variables ---------------------------------------

    QList<ModelPart*>              m_childItems;     // List of children
    QList<QVariant>                m_itemData;       // Column data (e.g., name, visibility)
    ModelPart* m_parentItem;     // Parent in tree hierarchy
    QColor                         partColor;        // Tree background color
    bool                           isVisible;        // Visibility state

    // VTK pipeline components
    vtkSmartPointer<vtkSTLReader>         file;             // Reads STL file
    vtkSmartPointer<vtkPolyDataMapper>    mapper;           // Maps geometry to graphics primitives
    vtkSmartPointer<vtkActor>             actor;            // Represents object in scene
    vtkSmartPointer<vtkAlgorithm>         currentFilter;    // Most recent filter in pipeline

    vtkSmartPointer<vtkPolyData>          originalData;     // Cached original mesh
    vtkSmartPointer<vtkPolyDataNormals>   originalNormalsFilter; // Computes normals

    vtkSmartPointer<vtkClipDataSet>       clipFilter;       // Clip filter
    vtkSmartPointer<vtkShrinkFilter>      shrinkFilter;     // Shrink filter

    bool  clipEnabled;                                      // True if clip is active
    bool  shrinkEnabled;                                    // True if shrink is active
    double shrinkFactor;                                    // Shrink intensity
    double clipOrigin[3];                                   // Plane origin for clip
    double clipNormal[3];                                   // Plane normal for clip
};

#endif // VIEWER_MODELPART_H

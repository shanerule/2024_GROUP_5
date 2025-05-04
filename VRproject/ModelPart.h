/** @file ModelPart.h
  * @brief EEEE2076 - Software Engineering & VR Project
  * Template for model parts that will be added as treeview items
  * P Evans 2022
  */

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

/**
 * @class ModelPart
 * @brief Encapsulates a single part in the model tree, including its hierarchy, display properties, and VTK pipeline.
 *
 * Manages child items, display data (name, visibility, colour), STL file loading,
 * filter application (clip, shrink), and provides actors for on‐screen and VR rendering. :contentReference[oaicite:0]{index=0}:contentReference[oaicite:1]{index=1}
 */

class ModelPart {
public:
    /**
     * @brief Constructs a ModelPart with given column data and optional parent.
     * @param data   A list of QVariant items representing the columns in the tree view.
     * @param parent Pointer to the parent ModelPart; nullptr for the root.
     */

    // --------------------------------------- Constructor & Destructor ---------------------------------------

        // Constructs a ModelPart with given data and optional parent
    ModelPart(const QList<QVariant>& data, ModelPart* parent = nullptr);

    /**
     * @brief Destructor: cleans up child items and VTK resources.
     */

    // Destructor that cleans up child items and VTK resources
    ~ModelPart();

    // --------------------------------------- Tree Structure Methods ---------------------------------------
     /// @name Tree Structure Methods
    ///@{
    /**
     * @brief Appends a child ModelPart to this node.
     * @param item Pointer to the child ModelPart to add.
     */

        // Adds a child part to this part
    void appendChild(ModelPart* item);

    /**
     * @brief Retrieves the child at the specified row.
     * @param row Zero-based index of the child.
     * @return Pointer to the ModelPart at that row, or nullptr if out-of-range.
     */

    // Returns the child part at the given row
    ModelPart* child(int row);

    /**
     * @brief Returns the number of child parts.
     * @return The count of children under this node.
     */

    // Returns the number of children
    int childCount() const;

    /**
     * @brief Returns the number of data columns for this part.
     * @return Number of QVariant columns stored.
     */

    // Returns the number of columns in the data
    int columnCount() const;

    /**
     * @brief Returns the parent of this ModelPart.
     * @return Pointer to the parent ModelPart, or nullptr if this is the root.
     */

    // Returns the parent of this item
    ModelPart* parentItem();

    /**
     * @brief Determines this part’s row index within its parent.
     * @return Zero-based row index, or 0 if no parent.
     */

    // Returns this item's index under its parent
    int row() const;

    /**
     * @brief Removes the child at the given row.
     * @param row Zero-based index of the child to remove.
     */

    // Removes the child at the specified row
    void removeChild(int row);

    ///@}

    /// @name Data Handling
    ///@{
    /**
     * @brief Retrieves data for a given column and role (used by Qt views).
     * @param column Zero-based column index.
     * @param role   Qt::ItemDataRole specifying display/background/foreground.
     * @return QVariant containing the data or styling brush, or an invalid QVariant if out-of-range.
     */

    // --------------------------------------- Data Handling ---------------------------------------

        // Returns the data stored at a column (used in Qt views)
    QVariant data(int column, int role = Qt::DisplayRole) const;

    /**
     * @brief Sets the data in the specified column.
     * @param column Zero-based index of the column to modify.
     * @param value  The new QVariant value to set.
     */

    // Sets the value for a specific column
    void set(int column, const QVariant& value);

    /**
     * @brief Sets the display name of this part (column 0).
     * @param newName New name string.
     */

    // Sets the display name of this part (column 0)
    void setName(const QString& newName);

    // --------------------------------------- Visual Properties ---------------------------------------
     
    ///@}

    /// @name Visual Properties
    ///@{
    /**
     * @brief Retrieves the current background color of the item.
     * @return QColor representing the part’s colour.
     */

    // Returns the current background color of the item
    QColor getColor() const;

    /**
     * @brief Sets the background color of the item.
     * @param color QColor to apply.
     */

    // Sets the background color of the item
    void setColor(const QColor& color);

    /**
    * @brief Sets the on-screen visibility of this part.
    * @param visible True to show; false to hide.
    */

    // Sets the visibility of the part in the scene
    void setVisible(bool visible);

    /**
     * @brief Queries whether this part is currently visible.
     * @return True if visible; false otherwise.
     */

    // Returns the visibility status

    bool visible() const;

    ///@}

    /// @name VTK Source & Actor Access
    ///@{
    /**
     * @brief Returns the VTK actor for on-screen rendering.
     * @return Smart pointer to the vtkActor instance.
     */

    // --------------------------------------- VTK Source & Actor Access ---------------------------------------

        // Returns the VTK actor used for rendering
    vtkSmartPointer<vtkActor> getActor() const;

    /**
     * @brief Returns the VTK data source (e.g., STL reader or filter).
     * @return Pointer to vtkAlgorithm representing the source/filter.
     */

    // Returns the VTK data source (e.g., STL reader)
    vtkAlgorithm* getSource() const;

    /**
     * @brief Returns the output port of the active VTK filter pipeline.
     * @return Pointer to vtkAlgorithmOutput for downstream connection.
     */

    // Returns the output port from the current pipeline stage
    vtkAlgorithmOutput* getOutputPort() const;

    /**
     * @brief Creates and returns a separate actor for VR rendering.
     * @return Smart pointer to a new vtkActor configured like the on-screen actor.
     */

    // Returns a separate actor for VR rendering
    vtkSmartPointer<vtkActor> getVRActor();


    // --------------------------------------- STL Loading ---------------------------------------
    ///@}

    /// @name STL Loading
    ///@{
    /**
     * @brief Loads an STL file into this part’s VTK pipeline.
     * @param fileName Path to the STL file.
     *
     * Uses vtkSTLReader to read geometry, then constructs a vtkPolyDataMapper
     * and vtkActor for rendering.
     */

        // Loads an STL file and sets up the rendering pipeline
    void loadSTL(QString fileName);

    // --------------------------------------- Filter Handling ---------------------------------------
   ///@}

    /// @name Filter Handling
    ///@{
    /**
     * @brief Enables or disables the clip filter and updates the pipeline.
     * @param enable True to enable clipping; false to disable.
     * @param origin  Array of three doubles specifying the clipping plane origin.
     * @param normal  Array of three doubles specifying the clipping plane normal.
     */
      
    // Enables/disables clip filter with given origin and normal
    void applyClipFilter(bool enable, double origin[3], double normal[3]);
    
    /**
     * @brief Enables or disables the shrink filter and sets its factor.
     * @param enable True to enable shrinking; false to disable.
     * @param factor Shrink factor to apply when enabled.
     */

    // Enables/disables shrink filter with specified factor
    void applyShrinkFilter(bool enable, double factor);

    /**
    * @brief Checks if the clip filter is currently enabled.
    * @return True if clipping is enabled; false otherwise.
    */

    // Returns true if clip filter is active
    bool isClipFilterEnabled() const;

    /**
     * @brief Checks if the shrink filter is currently enabled.
     * @return True if shrinking is enabled; false otherwise.
     */

    // Returns true if shrink filter is active
    bool isShrinkFilterEnabled() const;

    /**
    * @brief Rebuilds the VTK pipeline based on current filter settings.
    *
    * Applies shrink and clip filters in sequence, updates the mapper,
    * and reassigns it to the actor.
    */

    // Updates the filter chain based on active filters
    void updateFilters();

    // --------------------------------------- Original Data Backup ---------------------------------------
    ///@}

    /// @name Original Data Backup
    ///@{
    /**
     * @brief Checks if original (unfiltered) polydata exists.
     * @return True if original data is stored; false otherwise.
     */

    // Checks if original (unfiltered) polydata exists
    bool hasOriginalData() const;
    
    /**
     * @brief Stores a deep copy of the current dataset as original data.
     */

    // Stores the original polydata before any filtering
    void storeOriginalData();

    /**
     * @brief Retrieves the stored original polydata.
     * @return Smart pointer to the vtkPolyData.
     */

    // Returns the original polydata
    vtkSmartPointer<vtkPolyData> getOriginalPolyData() const;

    ///@}

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

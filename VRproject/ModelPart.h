/**
  *  @file ModelPart.h
  *
  *  EEEE2076 - Software Engineering & VR Project
  *
  *  Template for model parts that will be added as treeview items
  *
  *  P Evans 2022
  */

#ifndef VIEWER_MODELPART_H  
#define VIEWER_MODELPART_H  

#pragma once  

#include <QString>  
#include <QList>  
#include <QVariant>  
#include <QColor>  
#include <QBrush>  

  /* VTK headers - needed once VTK library is installed */
#include <vtkSmartPointer.h>  
#include <vtkSTLReader.h>  
#include <vtkPolyDataMapper.h>  
#include <vtkActor.h>  
#include <vtkAlgorithm.h>  

class ModelPart {
public:
    /**
      * Constructor
      * @param data   List of QVariant for each property (e.g., part name and visibility)
      * @param parent Parent item in the tree (nullptr for root)
      */
    ModelPart(const QList<QVariant>& data, ModelPart* parent = nullptr);

    /**
      * Destructor
      * Frees child items and deletes VTK actor if present
      */
    ~ModelPart();

    /**
      * Add a child item under this part
      * @param item Pointer to already-allocated ModelPart
      */
    void appendChild(ModelPart* item);

    /**
      * Get child at given row
      * @param row Index of child
      * @return Pointer to child or nullptr if out of range
      */
    ModelPart* child(int row);

    /**
      * Number of children under this part
      */
    int childCount() const;

    /**
      * Number of data columns (e.g., name + visibility)
      */
    int columnCount() const;

    /**
      * Data accessor for Qt tree view
      * @param column Column index
      * @param role   Qt display/background/foreground roles
      */
    QVariant data(int column, int role = Qt::DisplayRole) const;

    /**
      * Set data in a given column (used by Qt for editing)
      * @param column Column index
      * @param value  New QVariant value
      */
    void set(int column, const QVariant& value);

    /**
      * Get parent item pointer
      */
    ModelPart* parentItem();

    /**
      * Get this item's row index under its parent
      */
    int row() const;

    /**
      * Get and set the background colour used in the tree view
      */
    QColor getColor() const;
    void setColor(const QColor& color);

    /**
      * Show or hide this part
      * @param Visible True = visible, False = hidden
      */
    void setVisible(bool visible);
    bool visible() const;

    vtkAlgorithm* getSource() const;
    /**
      * Change this part's name (stored in column 0)
      */
    void setName(const QString& newName);

    /**
      * Load an STL file into this part's VTK pipeline
      * @param fileName Path to .stl file
      */
    void loadSTL(QString fileName);

    /**
      * Return the VTK actor for GUI rendering
      */
    vtkSmartPointer<vtkActor> getActor() const;

    /**
      * Remove a single child at given row (does not delete actor)
      */
    void removeChild(int row);

    /**
      * Apply and remove various VTK filters on the mesh:
      */
    void applyClipFilter();      // Clip half by plane  
    void removeClipFilter();     // Restore unclipped mesh  
    void applyShrinkFilter();    // Shrink cells  
    void removeShrinkFilter();   // Restore original mesh  

private:
    QList<ModelPart*>              m_childItems;     /**< Child items */
    QList<QVariant>                m_itemData;       /**< Column data */
    ModelPart* m_parentItem;     /**< Parent pointer */
    QColor                         partColor;        /**< Background color */
    bool                           isVisible;        /**< Visibility flag */
    bool                           clipFilterActive; /**< Clip state */
    bool                           shrinkFilterActive;/**< Shrink state */

    // VTK pipeline objects  
    vtkSmartPointer<vtkSTLReader>  file;             /**< STL reader */
    vtkSmartPointer<vtkPolyDataMapper> mapper;       /**< Mesh mapper */
    vtkSmartPointer<vtkActor>      actor;            /**< Render actor */
    vtkSmartPointer<vtkAlgorithm>  currentFilter;    /**< Last-applied filter */
};

#endif // VIEWER_MODELPART_H  

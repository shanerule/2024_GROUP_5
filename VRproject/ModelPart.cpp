/**
  *  @file ModelPart.cpp
  *
  *  EEEE2076 - Software Engineering & VR Project
  *
  *  Template for model parts that will be added as treeview items
  *
  *  P Evans 2022
  */

#include "ModelPart.h"  
#include <QDebug>  

  /* VTK includes: ensure all necessary filters, normals, mappers and readers are available */
#include <vtkSmartPointer.h>  
#include <vtkSTLReader.h>  
#include <vtkPolyDataMapper.h>  
#include <vtkPolyDataNormals.h>  
#include <vtkPlane.h>  
#include <vtkClipPolyData.h>  
#include <vtkCleanPolyData.h>  
#include <vtkShrinkPolyData.h>  
#include <vtkProperty.h>

ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent)
    : m_itemData(data), m_parentItem(parent), isVisible(true), clipFilterActive(false), shrinkFilterActive(false)
{
    partColor = QColor(255, 255, 255);
    /* You probably want to give the item a default colour */
}

ModelPart::~ModelPart()
{
    // First, delete child items, ensuring no invalid memory access  
    qDeleteAll(m_childItems);
    m_childItems.clear();

    // If the actor is set, delete it safely  
    if (actor) {
        actor->Delete();
        actor = nullptr;  // Avoid dangling pointer  
    }
}

void ModelPart::appendChild(ModelPart* item)
{
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}

ModelPart* ModelPart::child(int row)
{
    /* Return pointer to child item in row below this item. */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const
{
    /* Count number of child items */
    return m_childItems.count();
}

int ModelPart::columnCount() const
{
    /* Count number of columns (properties) that this item has. */
    return m_itemData.count();
}

QVariant ModelPart::data(int column, int role) const
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_itemData.at(column);

    if (role == Qt::BackgroundRole && partColor.isValid()) {
        qDebug() << "Applying background color to row" << row()
            << "Color:" << partColor;
        return QBrush(partColor);  // Return a QBrush for background  
    }

    if (role == Qt::ForegroundRole)
        return QBrush(Qt::black);  // Optional: change text color  

    return QVariant();
}

void ModelPart::set(int column, const QVariant& value)
{
    /* Set the data associated with a column of this item */
    if (column < 0 || column >= m_itemData.size())
        return;
    m_itemData.replace(column, value);
}

ModelPart* ModelPart::parentItem()
{
    return m_parentItem;
}

int ModelPart::row() const
{
    /* Return the row index of this item, relative to its parent. */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

QColor ModelPart::getColor() const
{
    return partColor;
}

void ModelPart::setColor(const QColor& color)
{
    partColor = color;
    // Apply color to the VTK actor if present  
    if (actor)
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

void ModelPart::setName(const QString& newName)
{
    set(0, newName);
}

void ModelPart::setVisible(bool visible)
{
    /* Toggle visibility flag and update both the tree and the actor */
    isVisible = visible;
    set(1, visible ? "true" : "false");

    if (actor) {
        actor->SetVisibility(visible ? 1 : 0);
        qDebug() << "Actor visibility set to" << visible;
    }
    else {
        qDebug() << "Actor is null!";
    }
}

bool ModelPart::visible() const
{
    return isVisible;
}

vtkAlgorithm* ModelPart::getSource() const
{
    // currentFilter points at either the reader or the last‐applied filter
    return currentFilter.Get();
}

/*void ModelPart::loadSTL(QString fileName)
{
    // Create and run STL reader  
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    // Generate surface normals (needed for correct lighting)  
    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputConnection(reader->GetOutputPort());
    normals->ConsistencyOn();
    normals->AutoOrientNormalsOn();
    normals->Update();

    // Create mapper linked to normals output  
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(normals->GetOutputPort());

    // Create actor linked to mapper  
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.9, 0.0, 0.0); // Default: reddish  

    // Store reader as the base filter for later  
    file = reader;
    currentFilter = reader;
}*/

void ModelPart::loadSTL(QString fileName)
{
    // Read STL file
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    // Generate normals
    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputConnection(reader->GetOutputPort());
    normals->AutoOrientNormalsOn();
    normals->ConsistencyOn();
    normals->ComputePointNormalsOn();
    normals->Update();

    // Set mapper
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(normals->GetOutputPort());

    // Set actor
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Balanced material that responds to light changes
    vtkSmartPointer<vtkProperty> prop = actor->GetProperty();
    prop->SetColor(0.7, 0.7, 0.7);        // Light gray (not fully white)
    prop->SetDiffuse(0.8);               // Reflects directional light
    prop->SetAmbient(0.2);               // Base brightness
    prop->SetSpecular(0.3);              // Some shine
    prop->SetSpecularPower(15);          // Soft highlight
    prop->LightingOn();                  // Ensure lighting is enabled

    // Optional: Center object
    double bounds[6];
    normals->GetOutput()->GetBounds(bounds);
    double center[3] = {
        (bounds[0] + bounds[1]) / 2.0,
        (bounds[2] + bounds[3]) / 2.0,
        (bounds[4] + bounds[5]) / 2.0
    };
    //actor->SetPosition(-center[0], -center[1], -center[2]);

    // Store reader and filter for later use
    file = reader;
    currentFilter = reader;
}




vtkSmartPointer<vtkActor> ModelPart::getActor() const
{
    return actor;
}

void ModelPart::applyClipFilter()
{
    if (!file) return;

    // Compute model center for clipping plane  
    double bounds[6];
    file->GetOutput()->GetBounds(bounds);
    double cx = (bounds[0] + bounds[1]) / 2.0;
    double cy = (bounds[2] + bounds[3]) / 2.0;
    double cz = (bounds[4] + bounds[5]) / 2.0;

    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(cx, cy, cz);
    plane->SetNormal(1.0, 0.0, 0.0);  // Clip along X  

    // Clip -> Clean pipeline  
    auto clipFilter = vtkSmartPointer<vtkClipPolyData>::New();
    clipFilter->SetInputConnection(currentFilter->GetOutputPort());
    clipFilter->SetClipFunction(plane);
    clipFilter->Update();

    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(clipFilter->GetOutputPort());
    cleanFilter->Update();

    // Update mapper & actor  
    currentFilter = cleanFilter;
    mapper->SetInputConnection(currentFilter->GetOutputPort());
    actor->SetMapper(mapper);
    clipFilterActive = true;
}

void ModelPart::removeClipFilter()
{
    if (!file) return;
    // Restore original geometry  
    mapper->SetInputConnection(file->GetOutputPort());
    actor->SetMapper(mapper);
    clipFilterActive = false;
}

void ModelPart::applyShrinkFilter()
{
    if (!file) return;

    // Shrink -> Clean pipeline  
    auto shrinkFilter = vtkSmartPointer<vtkShrinkPolyData>::New();
    shrinkFilter->SetInputConnection(currentFilter->GetOutputPort());
    shrinkFilter->SetShrinkFactor(0.8);
    shrinkFilter->Update();

    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(shrinkFilter->GetOutputPort());
    cleanFilter->Update();

    // Update mapper & actor  
    currentFilter = cleanFilter;
    mapper->SetInputConnection(currentFilter->GetOutputPort());
    actor->SetMapper(mapper);
    shrinkFilterActive = true;
}

void ModelPart::removeShrinkFilter()
{
    if (!file) return;
    // Restore original geometry  
    mapper->SetInputConnection(file->GetOutputPort());
    actor->SetMapper(mapper);
    shrinkFilterActive = false;
}

void ModelPart::removeChild(int row)
{
    if (row >= 0 && row < m_childItems.size())
        m_childItems.removeAt(row);
}

//vtkActor* ModelPart::getNewActor() {
/* This is a placeholder function that you will need to modify if you want to use it
     *
     * The default mapper/actor combination can only be used to render the part in
     * the GUI, it CANNOT also be used to render the part in VR. This means you need
     * to create a second mapper/actor combination for use in VR - that is the role
     * of this function. */


     /* 1. Create new mapper */

     /* 2. Create new actor and link to mapper */

     /* 3. Link the vtkProperties of the original actor to the new actor. This means
           *    if you change properties of the original part (colour, position, etc), the
           *    changes will be reflected in the GUI AND VR rendering.
           *
           *    See the vtkActor documentation, particularly the GetProperty() and SetProperty()
           *    functions.
           */


           /* The new vtkActor pointer must be returned here */
           //    return nullptr;

           //}
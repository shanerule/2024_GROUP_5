// @file ModelPart.cpp
//
// EEEE2076 - Software Engineering & VR Project
//
// Template for model parts that will be added as treeview items
//
// P Evans 2022

// --------------------------------------- Includes ---------------------------------------

#include "ModelPart.h"
#include <QDebug>

// VTK headers for rendering, filters, and geometry processing
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkShrinkPolyData.h>
#include <vtkProperty.h>
#include <vtkClipClosedSurface.h>
#include <vtkClipDataSet.h>
#include <vtkShrinkFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkPlaneCollection.h>

// --------------------------------------- Constructor & Destructor ---------------------------------------

// Constructs a new ModelPart with optional parent and default values
ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent)
    : m_itemData(data), m_parentItem(parent), isVisible(true)
{
    partColor = QColor(255, 255, 255); // Default white color
    clipEnabled = false;
    shrinkEnabled = false;
    shrinkFactor = 0.8;
    clipOrigin[0] = clipOrigin[1] = clipOrigin[2] = 0.0;
    clipNormal[0] = -1.0; clipNormal[1] = 0.0; clipNormal[2] = 0.0;
}

// Destroys the ModelPart and all its children, deletes actor if present
ModelPart::~ModelPart()
{
    qDeleteAll(m_childItems);
    m_childItems.clear();

    if (actor) {
        actor->Delete();
        actor = nullptr;
    }
}

// --------------------------------------- Tree Methods ---------------------------------------

// Adds a child ModelPart to this item
void ModelPart::appendChild(ModelPart* item)
{
    item->m_parentItem = this;
    m_childItems.append(item);
}

// Returns the child ModelPart at the given row
ModelPart* ModelPart::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

// Returns the number of children
int ModelPart::childCount() const
{
    return m_childItems.count();
}

// Returns the number of columns in this item's data
int ModelPart::columnCount() const
{
    return m_itemData.count();
}

// Returns the parent ModelPart
ModelPart* ModelPart::parentItem()
{
    return m_parentItem;
}

// Returns the index of this item under its parent
int ModelPart::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

// Removes a child from the given row index
void ModelPart::removeChild(int row)
{
    if (row >= 0 && row < m_childItems.size())
        m_childItems.removeAt(row);
}

// --------------------------------------- Data & Property Access ---------------------------------------

// Returns data for a given column and role (used by Qt tree view)
QVariant ModelPart::data(int column, int role) const
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return m_itemData.at(column);

    if (role == Qt::BackgroundRole && partColor.isValid()) {
        qDebug() << "Applying background color to row" << row() << "Color:" << partColor;
        return QBrush(partColor);
    }

    if (role == Qt::ForegroundRole)
        return QBrush(Qt::black);

    return QVariant();
}

// Sets the data in the specified column
void ModelPart::set(int column, const QVariant& value)
{
    if (column < 0 || column >= m_itemData.size())
        return;
    m_itemData.replace(column, value);
}

// Sets the display name of this part (column 0)
void ModelPart::setName(const QString& newName)
{
    set(0, newName);
}

// Returns the color assigned to this part
QColor ModelPart::getColor() const
{
    return partColor;
}

// Sets the background color and updates the actor's visual color
void ModelPart::setColor(const QColor& color)
{
    partColor = color;
    if (actor)
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

// Sets visibility flag and updates actor visibility
void ModelPart::setVisible(bool visible)
{
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

// Returns whether this part is visible
bool ModelPart::visible() const
{
    return isVisible;
}

// --------------------------------------- STL Loading ---------------------------------------

// Loads an STL file and initializes the VTK pipeline
void ModelPart::loadSTL(QString fileName)
{
    auto reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    auto polyData = reader->GetOutput();
    if (!polyData || polyData->GetNumberOfPoints() == 0 || polyData->GetNumberOfCells() == 0) {
        qWarning() << "STL load failed or empty: " << fileName;
        actor = nullptr;
        return;
    }

    originalData = vtkSmartPointer<vtkPolyData>::New();
    originalData->DeepCopy(polyData);

    file = reader;
    currentFilter = reader;

    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(originalData);

    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(partColor.redF(), partColor.greenF(), partColor.blueF());

    updateFilters();
}

// --------------------------------------- VTK Actor Access ---------------------------------------

// Returns the main rendering actor
vtkSmartPointer<vtkActor> ModelPart::getActor() const
{
    return actor;
}

// Returns the current VTK source or filter
vtkAlgorithm* ModelPart::getSource() const
{
    return currentFilter.Get();
}

// Returns the output port of the last-applied VTK filter
vtkAlgorithmOutput* ModelPart::getOutputPort() const
{
    if (clipEnabled && clipFilter)
        return clipFilter->GetOutputPort();
    if (shrinkEnabled && shrinkFilter)
        return shrinkFilter->GetOutputPort();
    return originalNormalsFilter->GetOutputPort();
}

// Creates and returns a duplicate actor for VR rendering
vtkSmartPointer<vtkActor> ModelPart::getVRActor()
{
    if (!mapper || !actor)
        return nullptr;

    auto vrMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vrMapper->SetInputConnection(mapper->GetInputConnection(0, 0));

    auto vrActor = vtkSmartPointer<vtkActor>::New();
    vrActor->SetMapper(vrMapper);
    vrActor->SetProperty(actor->GetProperty());
    vrActor->SetVisibility(isVisible ? 1 : 0);
    vrActor->SetUserMatrix(actor->GetUserMatrix());

    if (actor->GetTexture()) {
        vrActor->SetTexture(actor->GetTexture());
    }

    return vrActor;
}

// --------------------------------------- Filter Application ---------------------------------------

// Enables/disables the clip filter with given plane
void ModelPart::applyClipFilter(bool enable, double origin[3], double normal[3])
{
    clipEnabled = enable;
    if (enable) {
        for (int i = 0; i < 3; i++) {
            clipOrigin[i] = origin[i];
            clipNormal[i] = normal[i];
        }
    }
    updateFilters();
}

// Enables/disables shrink filter and sets the factor
void ModelPart::applyShrinkFilter(bool enable, double factor)
{
    shrinkEnabled = enable;
    shrinkFactor = factor;
    updateFilters();
}

// Rebuilds the filter pipeline with currently active filters
void ModelPart::updateFilters()
{
    if (!originalData || !mapper) return;

    auto processedData = vtkSmartPointer<vtkPolyData>::New();
    processedData->DeepCopy(originalData);

    if (shrinkEnabled) {
        if (!shrinkFilter) shrinkFilter = vtkSmartPointer<vtkShrinkFilter>::New();
        shrinkFilter->SetInputData(processedData);
        shrinkFilter->SetShrinkFactor(shrinkFactor);
        shrinkFilter->Update();

        auto geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
        geometryFilter->SetInputConnection(shrinkFilter->GetOutputPort());
        geometryFilter->Update();
        processedData->DeepCopy(geometryFilter->GetOutput());
    }

    if (clipEnabled) {
        auto clipper = vtkSmartPointer<vtkClipClosedSurface>::New();
        auto plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(clipOrigin);
        plane->SetNormal(clipNormal);
        auto planes = vtkSmartPointer<vtkPlaneCollection>::New();
        planes->AddItem(plane);

        clipper->SetInputData(processedData);
        clipper->SetClippingPlanes(planes);
        clipper->GenerateFacesOn();
        clipper->Update();

        processedData->DeepCopy(clipper->GetOutput());
    }

    mapper->SetInputData(processedData);
    mapper->Update();
    if (actor) actor->SetMapper(mapper);
}

// --------------------------------------- Filter Status ---------------------------------------

// Returns true if clip filter is enabled
bool ModelPart::isClipFilterEnabled() const {
    return clipEnabled;
}

// Returns true if shrink filter is enabled
bool ModelPart::isShrinkFilterEnabled() const {
    return shrinkEnabled;
}

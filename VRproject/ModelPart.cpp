/** @file ModelPart.cpp
  * @brief EEEE2076 - Software Engineering & VR Project
  * Template for model parts that will be added as treeview items
  * P Evans 2022
  */

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
/**
 * @brief Constructs a new ModelPart with given column data and optional parent.
 * @param data   Column‐wise QVariant list representing the tree view columns.
 * @param parent Pointer to the parent ModelPart; nullptr if this is the root.
 *
 * Initializes default white colour, visibility, and filter parameters.
 */

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

/**
 * @brief Destroys this ModelPart and all its children.
 *
 * Recursively deletes child items and releases the VTK actor if allocated.
 */

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
/**
 * @brief Appends a child ModelPart to this node.
 * @param item Pointer to the child ModelPart to add.
 *
 * Sets this instance as the parent of the appended child.
 */

// Adds a child ModelPart to this item
void ModelPart::appendChild(ModelPart* item)
{
    item->m_parentItem = this;
    m_childItems.append(item);
}

/**
 * @brief Retrieves the child at the specified row.
 * @param row Zero‐based index of the child.
 * @return Pointer to the ModelPart at that row, or nullptr if out‐of‐range.
 */
 
// Returns the child ModelPart at the given row
ModelPart* ModelPart::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

/**
 * @brief Returns the number of child parts.
 * @return Count of children under this node.
 */

// Returns the number of children
int ModelPart::childCount() const
{
    return m_childItems.count();
}

/**
 * @brief Returns the number of data columns stored in this part.
 * @return Number of QVariant columns.
 */

// Returns the number of columns in this item's data
int ModelPart::columnCount() const
{
    return m_itemData.count();
}

/**
 * @brief Returns the parent of this ModelPart.
 * @return Pointer to the parent ModelPart, or nullptr if root.
 */

// Returns the parent ModelPart
ModelPart* ModelPart::parentItem()
{
    return m_parentItem;
}

/**
 * @brief Determines this part’s index within its parent.
 * @return Zero‐based row index, or 0 if no parent.
 */

// Returns the index of this item under its parent
int ModelPart::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

/**
 * @brief Removes the child at the given row.
 * @param row Zero‐based index of the child to remove.
 */

// Removes a child from the given row index
void ModelPart::removeChild(int row)
{
    if (row >= 0 && row < m_childItems.size())
        m_childItems.removeAt(row);
}

// --------------------------------------- Data & Property Access ---------------------------------------
/**
 * @brief Retrieves data for a given column and role (used by Qt views).
 * @param column Zero‐based column index.
 * @param role   Qt::ItemDataRole specifying display/background/foreground.
 * @return QVariant containing the data or styling brush, or an invalid QVariant if out‐of‐range.
 */

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

/**
 * @brief Sets the data in the specified column.
 * @param column Zero‐based index of the column to modify.
 * @param value  New QVariant value to set.
 */

// Sets the data in the specified column
void ModelPart::set(int column, const QVariant& value)
{
    if (column < 0 || column >= m_itemData.size())
        return;
    m_itemData.replace(column, value);
}

/**
 * @brief Sets the display name of this part (column 0).
 * @param newName New name string.
 */

// Sets the display name of this part (column 0)
void ModelPart::setName(const QString& newName)
{
    set(0, newName);
}

/**
 * @brief Returns the current colour of this part.
 * @return QColor representing the part’s colour.
 */

// Returns the color assigned to this part
QColor ModelPart::getColor() const
{
    return partColor;
}

/**
 * @brief Sets the colour of this part and updates the actor’s property.
 * @param color QColor to apply.
 */

// Sets the background color and updates the actor's visual color
void ModelPart::setColor(const QColor& color)
{
    partColor = color;
    if (actor)
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

/**
 * @brief Sets the on‐screen visibility of this part.
 * @param visible True to show; false to hide.
 */

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

/**
 * @brief Queries whether this part is visible.
 * @return True if visible; false otherwise.
 */

// Returns whether this part is visible
bool ModelPart::visible() const
{
    return isVisible;
}

// --------------------------------------- STL Loading ---------------------------------------
/**
 * @brief Loads an STL file and initializes the VTK pipeline.
 * @param fileName Path to the STL file.
 *
 * Uses vtkSTLReader to read geometry, sets up mapper and actor,
 * and applies initial filters.
 */

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
/**
 * @brief Returns the main VTK actor for on‐screen rendering.
 * @return Smart pointer to the vtkActor instance.
 */

// Returns the main rendering actor
vtkSmartPointer<vtkActor> ModelPart::getActor() const
{
    return actor;
}

/**
 * @brief Returns the current VTK source or filter in the pipeline.
 * @return Pointer to vtkAlgorithm representing the source/filter.
 */

// Returns the current VTK source or filter
vtkAlgorithm* ModelPart::getSource() const
{
    return currentFilter.Get();
}

/**
 * @brief Returns the output port of the active VTK filter pipeline.
 * @return Pointer to vtkAlgorithmOutput for downstream connection.
 */

// Returns the output port of the last-applied VTK filter
vtkAlgorithmOutput* ModelPart::getOutputPort() const
{
    if (clipEnabled && clipFilter)
        return clipFilter->GetOutputPort();
    if (shrinkEnabled && shrinkFilter)
        return shrinkFilter->GetOutputPort();
    return originalNormalsFilter->GetOutputPort();
}

/**
 * @brief Creates and returns a copy of the actor for VR or off‐screen rendering.
 * @return Smart pointer to a new vtkActor configured like the on‐screen actor, or nullptr on failure.
 */

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
/**
 * @brief Enables or disables the clip filter and updates the pipeline.
 * @param enable True to enable clipping; false to disable.
 * @param origin Array of three doubles specifying the clipping plane origin.
 * @param normal Array of three doubles specifying the clipping plane normal.
 */

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

/**
 * @brief Enables or disables the shrink filter and sets its factor.
 * @param enable True to enable shrinking; false to disable.
 * @param factor Shrink factor to apply when enabled.
 */

// Enables/disables shrink filter and sets the factor
void ModelPart::applyShrinkFilter(bool enable, double factor)
{
    shrinkEnabled = enable;
    shrinkFactor = factor;
    updateFilters();
}

/**
 * @brief Rebuilds the VTK pipeline based on current filter settings.
 *
 * Applies shrink and clip filters in sequence, updates the mapper,
 * and reassigns it to the actor.
 */

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
/**
 * @brief Checks if the clip filter is currently enabled.
 * @return True if clipping is enabled; false otherwise.
 */

// Returns true if clip filter is enabled
bool ModelPart::isClipFilterEnabled() const {
    return clipEnabled;
}

/**
 * @brief Checks if the shrink filter is currently enabled.
 * @return True if shrinking is enabled; false otherwise.
 */

// Returns true if shrink filter is enabled
bool ModelPart::isShrinkFilterEnabled() const {
    return shrinkEnabled;
}

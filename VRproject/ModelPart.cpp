/**     @file ModelPart.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */

#include "ModelPart.h"
#include <QDebug>


  /* Commented out for now, will be uncommented later when you have
   * installed the VTK library
   */
#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkShrinkPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>

ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent)
    : m_itemData(data), m_parentItem(parent) {

    partColor = QColor(255, 255, 255);

    /* You probably want to give the item a default colour */
}


/*ModelPart::~ModelPart() {
    //qDeleteAll(m_childItems);
}*/

ModelPart::~ModelPart() {
    // First, delete child items, ensuring no invalid memory access
    qDeleteAll(m_childItems);  // This will delete all child items
    m_childItems.clear();       // Clear the list after deletion

    // If the actor is set, delete it safely
    if (actor) {
        actor->Delete();
        actor = nullptr;  // Avoid dangling pointer
    }
}




void ModelPart::appendChild(ModelPart* item) {
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}


ModelPart* ModelPart::child(int row) {
    /* Return pointer to child item in row below this item.
     */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const {
    /* Count number of child items
     */
    return m_childItems.count();
}


int ModelPart::columnCount() const {
    /* Count number of columns (properties) that this item has.
     */
    return m_itemData.count();
}


QVariant ModelPart::data(int column, int role) const {
    if (column < 0 || column >= m_itemData.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return m_itemData.at(column);
    }

    if (role == Qt::BackgroundRole) {
        if (partColor.isValid()) {
            qDebug() << "Applying background color to row" << row() << "Color:" << partColor;
            return QBrush(partColor);  // ✅ Ensure it returns a QBrush
        }
    }

    if (role == Qt::ForegroundRole) {
        return QBrush(Qt::black);  // Optional: Change text color if needed
    }

    return QVariant();
}






void ModelPart::set(int column, const QVariant& value) {
    /* Set the data associated with a column of this item
     */
    if (column < 0 || column >= m_itemData.size())
        return;

    m_itemData.replace(column, value);
}


ModelPart* ModelPart::parentItem() {
    return m_parentItem;
}


int ModelPart::row() const {
    /* Return the row index of this item, relative to it's parent.
     */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

/*void ModelPart::setColor(const QColor& color) {
    partColor = color;
    qDebug() << "setColor() called with RGB: " << partColor.red() << partColor.green() << partColor.blue();
}*/


QColor ModelPart::getColor() const {
    return partColor;
}

void ModelPart::setColor(const QColor& color) {
    partColor = color;
    //qDebug() << "setColor() called with RGB: " << partColor.red() << partColor.green() << partColor.blue();

    //if (actor) {
        //actor->GetProperty()->SetColor(partColor.redF(), partColor.greenF(), partColor.blueF());
    //}
    if (actor) {
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    }
}




//unsigned char ModelPart::getColourR() const { return partColor.red(); }
//unsigned char ModelPart::getColourG() const { return partColor.green(); }
//unsigned char ModelPart::getColourB() const { return partColor.blue(); }

void ModelPart::setName(const QString& newName) {
    set(0, newName);
}




//QColor ModelPart::getcolor() const {
    //return partColor;
//}

void ModelPart::setVisible(bool visible) {
    /* This is a placeholder function that you will need to modify if you want to use it */
    //isVisible = visible;
    //set(1, visible ? "true" : "false");    /* As the name suggests ... */

    //if (actor) {
        //actor->SetVisibility(visible ? 1 : 0);
    //}

    /*isVisible = visible;
    set(1, visible ? "true" : "false");

    if (actor) {
        actor->SetVisibility(visible ? 1 : 0);
    }*/

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

bool ModelPart::Visible() const {
    /* This is a placeholder function that you will need to modify if you want to use it */
    return isVisible;
    //return data(1).toString() == "true";
    /* As the name suggests ... */

}

void ModelPart::loadSTL(QString fileName) {
    // Create a VTK STL reader
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    // Generate surface normals (needed for lighting)
    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputConnection(reader->GetOutputPort());
    normals->ConsistencyOn();
    normals->AutoOrientNormalsOn();
    normals->Update();

    // Create a mapper and link it to the normals
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(normals->GetOutputPort());

    // Create an actor and link it to the mapper
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.9, 0.0, 0.0); // Default: reddish

    // Store the VTK objects in the ModelPart
    this->file = reader;
    this->mapper = mapper;
    this->actor = actor;
}


vtkSmartPointer<vtkActor> ModelPart::getActor() const {
    return actor;
}

void ModelPart::applyClipFilter()
{
    if (!file) return;

    currentFilter = static_cast<vtkAlgorithm*>(file.GetPointer());

    double bounds[6];
    file->GetOutput()->GetBounds(bounds);

    double centerX = (bounds[0] + bounds[1]) / 2.0;
    double centerY = (bounds[2] + bounds[3]) / 2.0;
    double centerZ = (bounds[4] + bounds[5]) / 2.0;

    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(centerX, centerY, centerZ);   
    plane->SetNormal(1.0, 0.0, 0.0);      

    // --- STEP 1: Clip Filter ---
    auto clipFilter = vtkSmartPointer<vtkClipPolyData>::New();
    clipFilter->SetInputConnection(currentFilter->GetOutputPort());
    clipFilter->SetClipFunction(plane);
    clipFilter->Update();

    // --- STEP 2: Clean Filter ---
    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(clipFilter->GetOutputPort());
    cleanFilter->Update();

    // --- STEP 3: Mapper-Actor ---
    currentFilter = static_cast<vtkAlgorithm*>(cleanFilter.GetPointer());
    mapper->SetInputConnection(currentFilter->GetOutputPort());
    actor->SetMapper(mapper);
}


void ModelPart::removeClipFilter()
{
    if (!file) return;

    mapper->SetInputConnection(file->GetOutputPort());
    actor->SetMapper(mapper);
    clipFilterActive = false;
}

void ModelPart::applyShrinkFilter()
{
    if (!file) return;

    currentFilter = static_cast<vtkAlgorithm*>(file.GetPointer());

    // --- STEP 1: Shrink Filter ---
    auto shrinkFilter = vtkSmartPointer<vtkShrinkPolyData>::New();
    shrinkFilter->SetInputConnection(currentFilter->GetOutputPort());
    shrinkFilter->SetShrinkFactor(0.8);  
    shrinkFilter->Update();

    // --- STEP 2: Clean Filter ---
    auto cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(shrinkFilter->GetOutputPort());
    cleanFilter->Update();

    // --- STEP 3: Mapper-Actor ---
    currentFilter = static_cast<vtkAlgorithm*>(cleanFilter.GetPointer());
    mapper->SetInputConnection(currentFilter->GetOutputPort());
    actor->SetMapper(mapper);
}

void ModelPart::removeShrinkFilter()
{
    if (!file) return;

    mapper->SetInputConnection(file->GetOutputPort());
    actor->SetMapper(mapper);
    shrinkFilterActive = false;
}

void ModelPart::removeChild(int row) {
    if (row >= 0 && row < m_childItems.size()) {
        m_childItems.removeAt(row);
    }
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
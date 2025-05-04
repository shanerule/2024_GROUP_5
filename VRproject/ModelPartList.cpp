// @file ModelPartList.cpp
//
// EEEE2076 - Software Engineering & VR Project
//
// Implementation of the ModelPartList class used for displaying parts in a QTreeView
//
// P Evans 2022

#include "ModelPartList.h"
#include "ModelPart.h"

// --------------------------------------- Constructor & Destructor ---------------------------------------

// Constructs the model with a root item (column headers)
ModelPartList::ModelPartList(const QString& data, QObject* parent)
    : QAbstractItemModel(parent)
{
    // The root item stores headers for the columns ("Part", "Visible")
    rootItem = new ModelPart({ QString("Part"), QString("Visible") });
}

// Destructor: deletes the root item (which owns all children)
ModelPartList::~ModelPartList() {
    delete rootItem;
}

// --------------------------------------- Model Interface ---------------------------------------

// Returns the number of columns (always matches root item)
int ModelPartList::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

// Returns the data to display at a given index and role
QVariant ModelPartList::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ModelPart* item = static_cast<ModelPart*>(index.internalPointer());
    return item->data(index.column());
}

// Returns the item flags for the given index (read-only)
Qt::ItemFlags ModelPartList::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

// Provides header labels for the tree view (e.g., "Part", "Visible")
QVariant ModelPartList::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);
    return QVariant();
}

// Returns a QModelIndex pointing to a given row/column (child of root)
QModelIndex ModelPartList::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ModelPart* parentItem = rootItem; // Flat model: all items are children of root
    ModelPart* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);

    return QModelIndex();
}

// Returns the parent of a given index — unused for flat model
QModelIndex ModelPartList::parent(const QModelIndex& /*index*/) const {
    return QModelIndex();  // Flat model: no parent
}

// Returns the number of children under the given parent
int ModelPartList::rowCount(const QModelIndex& parent) const {
    ModelPart* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ModelPart*>(parent.internalPointer());

    return parentItem->childCount();
}

// --------------------------------------- Custom Functions ---------------------------------------

// Returns the root item (used for access and building the tree)
ModelPart* ModelPartList::getRootItem() {
    return rootItem;
}

// Appends a new child to the root with given data
QModelIndex ModelPartList::appendChild(const QList<QVariant>& data) {
    int newRow = rootItem->childCount();
    beginInsertRows(QModelIndex(), newRow, newRow);

    ModelPart* childPart = new ModelPart(data, rootItem);
    rootItem->appendChild(childPart);

    endInsertRows();
    return createIndex(newRow, 0, childPart);
}

// Removes a single row (convenience function)
bool ModelPartList::removeRow(int row, const QModelIndex& parent) {
    return removeRows(row, 1, parent);
}

// Removes one or more rows starting from `row` under the given parent
bool ModelPartList::removeRows(int row, int count, const QModelIndex& parent) {
    ModelPart* parentItem = getItem(parent);
    if (!parentItem || row < 0 || row + count > parentItem->childCount())
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        parentItem->removeChild(row);  // Always remove at same index
    }
    endRemoveRows();
    return true;
}

// Returns the ModelPart pointer from a given QModelIndex
ModelPart* ModelPartList::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        auto item = static_cast<ModelPart*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

/**     @file ModelPartList.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model part list that will be used to create the treeview.
  *
  *     P Evans 2022
  */

#include "ModelPartList.h"
#include "ModelPart.h"

ModelPartList::ModelPartList(const QString& data, QObject* parent)
    : QAbstractItemModel(parent)
{
    /* Have option to specify number of visible properties for each item in tree - the root item
     * acts as the column headers
     */
     // rootItem = new ModelPart({ tr("Part"), tr("Visible?") });
    rootItem = new ModelPart({ QString("Part"), QString("Visible") });
}

ModelPartList::~ModelPartList() {
    delete rootItem;
}

int ModelPartList::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

QVariant ModelPartList::data(const QModelIndex& index, int role) const {
    /* If the item index isn't valid, return a new, empty QVariant (QVariant is generic datatype
     * that could be any valid Qt class) */
    if (!index.isValid())
        return QVariant();

    /* Role represents what this data will be used for. We only handle DisplayRole here. */
    if (role != Qt::DisplayRole)
        return QVariant();

    /* Get a pointer to the item referred to by the QModelIndex */
    ModelPart* item = static_cast<ModelPart*>(index.internalPointer());

    /* Return the data for the requested column */
    return item->data(index.column());
}

Qt::ItemFlags ModelPartList::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

QVariant ModelPartList::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);
    return QVariant();
}

QModelIndex ModelPartList::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ModelPart* parentItem = rootItem; // Always use rootItem for a flat model
    ModelPart* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);

    return QModelIndex();
}

QModelIndex ModelPartList::parent(const QModelIndex& /*index*/) const {
    // Flat model: no parent for any item
    return QModelIndex();
}

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

ModelPart* ModelPartList::getRootItem() {
    return rootItem;
}

QModelIndex ModelPartList::appendChild(const QList<QVariant>& data) {
    int newRow = rootItem->childCount();
    beginInsertRows(QModelIndex(), newRow, newRow);

    ModelPart* childPart = new ModelPart(data, rootItem);
    rootItem->appendChild(childPart);

    endInsertRows();
    return createIndex(newRow, 0, childPart);
}

bool ModelPartList::removeRow(int row, const QModelIndex& parent) {
    return removeRows(row, 1, parent);
}

bool ModelPartList::removeRows(int row, int count, const QModelIndex& parent) {
    ModelPart* parentItem = getItem(parent);
    if (!parentItem || row < 0 || row + count > parentItem->childCount())
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        parentItem->removeChild(row);
    }
    endRemoveRows();
    return true;
}

ModelPart* ModelPartList::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        auto item = static_cast<ModelPart*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}


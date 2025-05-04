// @file ModelPartList.h
//
// EEEE2076 - Software Engineering & VR Project
//
// Tree model wrapper for ModelPart objects, displayed in a QTreeView.
//
// P Evans 2022

#ifndef VIEWER_MODELPARTLIST_H  
#define VIEWER_MODELPARTLIST_H  

#pragma once  

// --------------------------------------- Includes ---------------------------------------

#include "ModelPart.h"

#include <QAbstractItemModel>  // Base class for custom tree/table models
#include <QModelIndex>         // Used to refer to specific cells
#include <QVariant>            // Flexible container for different data types
#include <QString>
#include <QList>

class ModelPart;  // Forward declaration

// --------------------------------------- ModelPartList Class ---------------------------------------

class ModelPartList : public QAbstractItemModel {
    Q_OBJECT  // Enables signals/slots and Qt meta-object features

public:

    // --------------------------------------- Constructor & Destructor ---------------------------------------

    // Constructor: creates the model and root item
    ModelPartList(const QString& data, QObject* parent = nullptr);

    // Destructor: frees memory allocated to root and children
    ~ModelPartList();

    // --------------------------------------- Overridden Qt Model Functions ---------------------------------------

    // Returns the number of columns (e.g., "Part", "Visible")
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // Returns data to display in a given index and role
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Returns flags (e.g., editable/selectable) for a given index
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Returns header text for each column
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Returns a QModelIndex for a specific child item
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    // Returns the parent index of a child (only used in nested models)
    QModelIndex parent(const QModelIndex& index) const override;

    // Returns number of child items for a given parent
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // --------------------------------------- Custom API ---------------------------------------

    // Returns pointer to the root item
    ModelPart* getRootItem();

    // Appends a child item directly under the root
    QModelIndex appendChild(const QList<QVariant>& data);

    // Appends a child under a specified parent (unused in flat model)
    QModelIndex appendChild(QModelIndex& parent, const QList<QVariant>& data);

    // Removes multiple child rows under a parent
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // Removes a single row under a parent
    bool removeRow(int row, const QModelIndex& parent = QModelIndex());

    // Converts a QModelIndex into its corresponding ModelPart*
    ModelPart* getItem(const QModelIndex& index) const;

private:
    // Root of the tree (not shown in UI)
    ModelPart* rootItem;
};

#endif // VIEWER_MODELPARTLIST_H  

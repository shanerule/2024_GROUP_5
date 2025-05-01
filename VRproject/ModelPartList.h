/**
  *  @file ModelPartList.h
  *
  *  EEEE2076 - Software Engineering & VR Project
  *
  *  Template for model part list that will be used to create the treeview.
  *
  *  P Evans 2022
  */

#ifndef VIEWER_MODELPARTLIST_H  
#define VIEWER_MODELPARTLIST_H  

#pragma once  

#include "ModelPart.h"  

#include <QAbstractItemModel>  
#include <QModelIndex>  
#include <QVariant>  
#include <QString>  
#include <QList>  

class ModelPart;

class ModelPartList : public QAbstractItemModel {
    Q_OBJECT        /**< Qt meta-object tag (enables signals/slots, etc.) */

public:
    /**
      * Constructor
      * @param data   Unused string (could define column headers)
      * @param parent Parent QObject for Qt hierarchy
      */
    ModelPartList(const QString& data, QObject* parent = nullptr);

    /**
      * Destructor
      * Frees the root item allocated in constructor
      */
    ~ModelPartList();

    /**
      * Return number of columns in the tree view
      * @param parent Unused
      * @return number of columns – e.g. "Part" and "Visible" = 2
      */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
      * Return the data for a given index and role
      * @param index Model index (row + column)
      * @param role  Qt role (DisplayRole, BackgroundRole, etc.)
      * @return QVariant containing the data or empty if invalid/unsupported role
      */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
      * Return item flags for a given index
      * @param index Model index
      * @return Qt::ItemFlags (e.g. selectable, editable)
      */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
      * Return header data for columns
      * @param section    Column number
      * @param orientation Horizontal/Vertical
      * @param role       Qt role
      * @return QVariant with header text or empty
      */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
      * Create a QModelIndex for the given row, column, under parent
      * @param row    Row number
      * @param column Column number
      * @param parent Parent index (or root)
      * @return QModelIndex for the requested item or invalid if out of range
      */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    /**
      * Given a child index, return its parent index
      * @param index Child index
      * @return Parent index or invalid for root-level items
      */
    QModelIndex parent(const QModelIndex& index) const override;

    /**
      * Return number of rows (children) under a given parent index
      * @param parent QModelIndex of parent (invalid for root)
      * @return Number of child items
      */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
      * Get pointer to the root item
      * @return ModelPart* pointing to root
      */
    ModelPart* getRootItem();

    /**
      * Append a new child to the root item
      * @param data List of QVariant for new child's columns
      * @return QModelIndex of the newly created item
      */
    QModelIndex appendChild(const QList<QVariant>& data);

    /**
      * Append a new child under a specified parent index
      * @param parent QModelIndex under which to insert
      * @param data   List of QVariant for new child's columns
      * @return QModelIndex of the newly created item
      */
    QModelIndex appendChild(QModelIndex& parent, const QList<QVariant>& data);

    /**
      * Remove rows from the model (override)
      * @param row    First row to remove
      * @param count  Number of rows to remove
      * @param parent Parent index
      * @return true if removal succeeded
      */
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    /**
      * Convenience to remove a single row
      * @param row    Row to remove
      * @param parent Parent index
      * @return true if removal succeeded
      */
    bool removeRow(int row, const QModelIndex& parent = QModelIndex());

    /**
      * Retrieve ModelPart* from a QModelIndex
      * @param index QModelIndex of item
      * @return Pointer to ModelPart or root if index invalid
      */
    ModelPart* getItem(const QModelIndex& index) const;

private:
    ModelPart* rootItem;   /**< The hidden root item at base of tree */
};

#endif // VIEWER_MODELPARTLIST_H  

// @file Option_Dialog.h
//
// EEEE2076 - Software Engineering & VR Project
//
// Header for dialog that allows editing a ModelPart's name, color, and visibility.

#ifndef OPTION_DIALOG_H
#define OPTION_DIALOG_H

// --------------------------------------- Qt Includes ---------------------------------------

#include <QDialog>     // Base class for modal/non-modal dialogs
#include <QColor>      // Color handling for preview and data

// --------------------------------------- Project Includes ---------------------------------------

#include "ModelPart.h" // Custom model class for STL parts

// Forward declaration of UI namespace generated from .ui file
namespace Ui {
    class Option_Dialog;
}

// --------------------------------------- Option_Dialog Class ---------------------------------------

class Option_Dialog : public QDialog
{
    Q_OBJECT

public:

    // --------------------------------------- Public Functions ---------------------------------------

    // Constructor: creates dialog and sets up UI
    explicit Option_Dialog(QWidget* parent = nullptr);

    // Destructor: releases resources
    ~Option_Dialog();

    // Sets dialog fields using individual data values
    void setModelPartData(const QString& name, int r, int g, int b, bool Visible);

    // Gets the ModelPart currently being edited
    ModelPart* getModelPart();

    // Retrieves current dialog field values
    void getModelPartData(QString& name, int& r, int& g, int& b, bool& Visible) const;

    // Sets the ModelPart to edit and updates fields
    void setModelPart(ModelPart* part);

    // Called when user presses OK/accept — applies dialog changes
    void accept() override;

signals:

    // --------------------------------------- Signals ---------------------------------------

    // Emitted when visibility is toggled
    void visibilityChanged(bool visible);

private slots:

    // --------------------------------------- Private Slots ---------------------------------------

    // Updates the preview box with the selected color
    void updateColorPreview();

    // Opens color picker dialog and updates preview
    void on_colorButton_clicked();

private:

    // --------------------------------------- Private Members ---------------------------------------

    Ui::Option_Dialog* ui;       // Pointer to UI elements
    ModelPart* currentPart;      // Currently selected ModelPart
    QColor selectedColor;        // Currently selected color for the part
};

#endif // OPTION_DIALOG_H

/**
 * @file Option_Dialog.h
 * @brief UI dialog for editing a ModelPart's name, color, and visibility in the VTK viewer.
 *
 * This class provides a Qt-based dialog to allow users to update the display properties
 * of a model part, including its label, color (via color picker), and visibility toggle.
 */
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

/**
 * @class Option_Dialog
 * @brief Dialog that enables editing of a ModelPart's properties: name, color, and visibility.
 */
class Option_Dialog : public QDialog
{
    Q_OBJECT

public:

    // --------------------------------------- Public Functions ---------------------------------------

    /**
     * @brief Constructor that initializes the dialog and its widgets.
     * @param parent Parent widget (optional).
     */
    // Constructor: creates dialog and sets up UI
    explicit Option_Dialog(QWidget* parent = nullptr);

    /**
     * @brief Destructor that cleans up UI resources.
     */
    // Destructor: releases resources
    ~Option_Dialog();

    /**
     * @brief Pre-fills dialog fields with explicit values.
     * @param name Name of the model part.
     * @param r Red color value (0–255).
     * @param g Green color value (0–255).
     * @param b Blue color value (0–255).
     * @param Visible Visibility flag.
     */
    // Sets dialog fields using individual data values
    void setModelPartData(const QString& name, int r, int g, int b, bool Visible);

    /**
     * @brief Returns the pointer to the currently selected ModelPart.
     * @return The ModelPart being edited.
     */
    // Gets the ModelPart currently being edited
    ModelPart* getModelPart();

    /**
     * @brief Extracts the user-edited values from the dialog.
     * @param name Output variable for the name.
     * @param r Output red value.
     * @param g Output green value.
     * @param b Output blue value.
     * @param Visible Output visibility flag.
     */
    // Retrieves current dialog field values
    void getModelPartData(QString& name, int& r, int& g, int& b, bool& Visible) const;

    /**
     * @brief Sets the ModelPart and updates dialog UI accordingly.
     * @param part The ModelPart to load into the dialog.
     */
    // Sets the ModelPart to edit and updates fields
    void setModelPart(ModelPart* part);

    /**
     * @brief Applies changes to the part and accepts the dialog.
     */
    // Called when user presses OK/accept — applies dialog changes
    void accept() override;

signals:

    // --------------------------------------- Signals ---------------------------------------

    /**
     * @brief Emitted when the visibility checkbox is toggled.
     * @param visible New visibility state.
     */
    // Emitted when visibility is toggled
    void visibilityChanged(bool visible);

private slots:

    // --------------------------------------- Private Slots ---------------------------------------

    /**
     * @brief Updates the UI color preview box with the selected color.
     */
    // Updates the preview box with the selected color
    void updateColorPreview();

    /**
     * @brief Opens a QColorDialog for the user to pick a color.
     */
    // Opens color picker dialog and updates preview
    void on_colorButton_clicked();

private:

    // --------------------------------------- Private Members ---------------------------------------

    Ui::Option_Dialog* ui;       // Pointer to UI elements
    ModelPart* currentPart;      // Currently selected ModelPart
    QColor selectedColor;        // Currently selected color for the part
};

#endif // OPTION_DIALOG_H

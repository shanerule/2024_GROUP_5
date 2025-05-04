/**
 * @file Option_Dialog.cpp
 * @brief Implementation of Option_Dialog class for editing a ModelPart's name, color, and visibility.
 */

#include "Option_Dialog.h"       // Declaration of Option_Dialog class
#include "ui_Option_Dialog.h"    // Auto-generated UI class for Option_Dialog
#include <QColorDialog>          // For color picking dialog
#include <QPalette>              // For applying color to preview widgets

// --------------------------------------- Constructor & Destructor ---------------------------------------

/**
 * @brief Constructs the dialog and connects signals for UI interaction.
 * @param parent Optional parent widget.
 */
// Constructs the dialog and connects signals for UI interaction
Option_Dialog::Option_Dialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Option_Dialog)
    , currentPart(nullptr)
{
    ui->setupUi(this);

    // Connect color button to handler
    connect(ui->colorButton, &QPushButton::clicked, this, &Option_Dialog::on_colorButton_clicked);

    // Connect visibility checkbox to signal emission
    connect(ui->checkBox, &QCheckBox::toggled, this, &Option_Dialog::visibilityChanged);
}

/**
 * @brief Destructor: cleans up the generated UI.
 */
// Destructor: cleans up the generated UI
Option_Dialog::~Option_Dialog() {
    delete ui;
}

// --------------------------------------- Public Interface ---------------------------------------

/**
 * @brief Sets the current ModelPart and populates the dialog fields.
 * @param part Pointer to the ModelPart to edit.
 */
// Sets the current ModelPart and populates the dialog with its values
void Option_Dialog::setModelPart(ModelPart* part) {
    if (!part) return;

    currentPart = part;

    // Set name in line edit
    ui->lineEdit->setText(part->data(0).toString());

    // Store the selected color from the part
    selectedColor = part->getColor();

    // Update visibility checkbox
    ui->checkBox->setChecked(part->visible());

    // Preview color in UI
    updateColorPreview();
}

/**
 * @brief Returns the ModelPart currently being edited.
 * @return Pointer to the ModelPart.
 */
// Returns the current associated ModelPart
ModelPart* Option_Dialog::getModelPart() {
    return currentPart;
}

// --------------------------------------- Slots ---------------------------------------

/**
 * @brief Opens a color picker dialog and updates the selected color preview.
 */
// Opens a QColorDialog and updates the preview with chosen color
void Option_Dialog::on_colorButton_clicked()
{
    QColor color = QColorDialog::getColor(selectedColor, this, "Choose Color");

    if (color.isValid()) {
        selectedColor = color;
        updateColorPreview();
    }
}

/**
 * @brief Applies the dialog changes to the associated ModelPart and closes the dialog.
 */
// Applies the current form values to the ModelPart and accepts the dialog
void Option_Dialog::accept() {
    if (currentPart) {
        QString name = ui->lineEdit->text();
        bool Visible = ui->checkBox->isChecked();

        currentPart->setName(name);
        currentPart->setColor(selectedColor);
        currentPart->setVisible(Visible);

        emit visibilityChanged(Visible);
    }

    QDialog::accept();
}

// --------------------------------------- Utility ---------------------------------------

/**
 * @brief Updates the color preview widget with the selected color.
 */
// Updates the color preview box with the selected color
void Option_Dialog::updateColorPreview() {
    QPalette palette = ui->colorPreview->palette();
    palette.setColor(QPalette::Window, selectedColor);
    ui->colorPreview->setAutoFillBackground(true);
    ui->colorPreview->setPalette(palette);
    ui->colorPreview->update();
}

/**
 * @brief Extracts current values from the dialog UI into variables.
 * @param name Output name.
 * @param r Output red value (0–255).
 * @param g Output green value (0–255).
 * @param b Output blue value (0–255).
 * @param Visible Output visibility flag.
 */
// Gets current values from the dialog into individual variables
void Option_Dialog::getModelPartData(QString& name, int& r, int& g, int& b, bool& Visible) const {
    name = ui->lineEdit->text();
    QColor color = selectedColor;
    r = color.red();
    g = color.green();
    b = color.blue();
    Visible = ui->checkBox->isChecked();
}

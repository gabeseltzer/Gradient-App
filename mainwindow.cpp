//Written by Gabriel Seltzer
//Released under LGPL licensing

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <string>
#include <QVector>
#include <QVectorIterator>
#include <QColorDialog>
#include <QPalette>
#include "gcodeParser.cpp"

QFile *gcodeFile;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow);
{
    ui->setupUi(this);
}

void MainWindow::openFile() {

    // TODO: Try https://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum instead of C://
    // TODO: Also, you should probably use an object that actually uses paths, not turn this into a string
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "C://", "Gcode Files(*.gcode);;Text Files (*.txt);;All files (*.*)");
    
    if (filename.isNull())
        return;

    ui->filePath->setText(filename);

    gcodeFile = new QFile(filename);
    if(!gcodeFile->open(QIODevice::ReadOnly))
        QMessageBox::information(0,"info",gcodeFile->errorString());

// TODO: Make the following 3 lines into a function, because it is not really part of openFile
    QTextStream in(gcodeFile);
    ui->plainTextEdit->setPlainText(in.readAll());
    in.seek(0);
    
    processGcode();
}


// TODO: This should probably go into the kernel side
int MainWindow::countLayers(){
    QTextStream reader(gcodeFile);
    QString currentLine;
    int layerCount = 0;
    while(!reader.atEnd()){
        currentLine = reader.readLine();
        if (currentLine.contains("; layer"))
        {
            layerCount++;
        }
    }
    reader.seek(0);
    return layerCount;

}

void MainWindow::processGcode(){
    int layers;
    layers = countLayers();
    QString stringLayers = QString::number(layers);
    ui->gradientStartSlider->setRange(1,layers);
    ui->gradientStartSlider->setEnabled(true);
    ui->gradientStartSlider->setValue(1);
    ui->gradientStartTextField->setText("1");
    ui->gradientStartTextField->setEnabled(true);
    ui->gradientStartTextField->setValidator(new QIntValidator(1, layers, this));

    ui->gradientEndSlider->setRange(1,layers);
    ui->gradientEndSlider->setValue(layers);
    ui->gradientEndSlider->setEnabled(true);
    ui->gradientEndTextField->setEnabled(true);
    ui->gradientEndTextField->setValidator(new QIntValidator(1, layers, this));
    ui->gradientEndTextField->setText(stringLayers);

    ui->writeGcodeButton->setEnabled(true);

    ui->gradientStartSpinBox->setEnabled(true);
    ui->gradientEndSpinBox->setEnabled(true);
    ui->gradientEndSpinBox->setValue(100);
    
    ui->gradientEndColorButton->setStyleSheet("background-color: grey");
    ui->gradientStartColorButton->setStyleSheet("background-color: grey");
    
    ui->retractionCheckbox->setEnabled(true);
    ui->plainTextEdit->setEnabled(true);
    ui->printerBox->setEnabled(true);
}

void MainWindow::on_openFileButton_clicked()
{
    openFile();
}

void MainWindow::on_gradientStartSlider_sliderMoved(int position)
{
    ui->gradientStartTextField->setText(QString::number(position));
    if (position > ui->gradientEndSlider->value())
    {
        ui->gradientEndSlider->setValue(position);
        ui->gradientEndTextField->setText(QString::number(position));
    }
}

void MainWindow::on_gradientEndSlider_sliderMoved(int position)
{
    ui->gradientEndTextField->setText(QString::number(position));
    if (position < ui->gradientStartSlider->value())
    {
        ui->gradientStartSlider->setValue(position);
        ui->gradientStartTextField->setText(QString::number(position));
    }
}

void MainWindow::on_gradientStartTextField_returnPressed()
{
    ui->gradientStartSlider->setValue(ui->gradientStartTextField->text().toInt());
    if (ui->gradientStartSlider->value() > ui->gradientEndSlider->value())
    {
        ui->gradientEndSlider->setValue(ui->gradientStartSlider->value());
        ui->gradientEndTextField->setText(ui->gradientStartTextField->text());
    }
}

void MainWindow::on_gradientStartTextField_editingFinished()
{
    ui->gradientStartSlider->setValue(ui->gradientStartTextField->text().toInt());
    if (ui->gradientStartSlider->value() > ui->gradientEndSlider->value())
    {
        ui->gradientEndSlider->setValue(ui->gradientStartSlider->value());
        ui->gradientEndTextField->setText(ui->gradientStartTextField->text());
    }
}

void MainWindow::on_gradientEndTextField_editingFinished()
{
    ui->gradientEndSlider->setValue(ui->gradientEndTextField->text().toInt());
    if (ui->gradientEndSlider->value() < ui->gradientStartSlider->value())
    {
        ui->gradientStartSlider->setValue(ui->gradientEndSlider->value());
        ui->gradientStartTextField->setText(ui->gradientEndTextField->text());
    }
}

void MainWindow::on_gradientEndTextField_returnPressed()
{
    ui->gradientEndSlider->setValue(ui->gradientEndTextField->text().toInt());
    if (ui->gradientEndSlider->value() < ui->gradientStartSlider->value())
    {
        ui->gradientStartSlider->setValue(ui->gradientEndSlider->value());
        ui->gradientStartTextField->setText(ui->gradientEndTextField->text());
    }
}

void MainWindow::on_writeGcodeButton_clicked()
{
    int gradientStartLayer = ui->gradientStartSlider->value();
    int gradientEndLayer = ui->gradientEndSlider->value();
    bool fancyRetraction = ui->retractionCheckbox->isChecked();
    QString defaultFileSaveName = gcodeFile->fileName();
    defaultFileSaveName = defaultFileSaveName.mid(0,defaultFileSaveName.size()-6) + " Gradient";
    QString saveFileName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"), defaultFileSaveName, "Gcode Files(*.gcode);;Text Files (*.txt);;All files (*.*)");
    if (saveFileName.isNull())
        return;
    QTextStream reader(gcodeFile);
    if (reader.atEnd())
        reader.seek(0);
    QFile saveFile(saveFileName);
    saveFile.open(QIODevice::WriteOnly);
    QTextStream writer(&saveFile);
    
    
    if (ui->printerBox->currentText() == "Diamond Hotend") {
        //Get the current Colors of the two gradient buttons
        QPalette startPal = ui->gradientStartColorButton->palette();
        QPalette endPal = ui->gradientEndColorButton->palette();
        writeGcode(gradientStartLayer,gradientEndLayer,
                   fancyRetraction, 
                   startPal,endPal,
                   &reader,&writer);
    }
    else if (ui->printerBox->currentText() == "Builder Dual"){
        int gradientStartPercent = ui->gradientStartSpinBox->value();
        int gradientEndPercent = ui->gradientEndSpinBox->value();
        writeGcode(gradientStartLayer,gradientEndLayer,
                   fancyRetraction,
                   gradientStartPercent,gradientEndPercent,
                   &reader,&writer);
    }
}

void MainWindow::on_gradientStartColorButton_clicked()
{
    QPalette pal = ui->gradientStartColorButton->palette();
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Button));
    if (!newColor.isValid())
        return;
    QString newStyle = "background-color: " + newColor.name();
    ui->gradientStartColorButton->setStyleSheet(newStyle);
    ui->gradientStartColorButton->update();
}

void MainWindow::on_gradientEndColorButton_clicked()
{
    QPalette pal = ui->gradientEndColorButton->palette();
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Button));
    if (!newColor.isValid())
        return;
    QString newStyle = "background-color: " + newColor.name();
    ui->gradientEndColorButton->setStyleSheet(newStyle);
    ui->gradientEndColorButton->update();
}

void MainWindow::on_printerBox_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "Diamond Hotend"){
        ui->gradientEndColorButton->setEnabled(true);
        ui->gradientStartColorButton->setEnabled(true);
        
        ui->gradientEndSpinBox->setEnabled(false);
        ui->gradientStartSpinBox->setEnabled(false);
    }
    else if (arg1 == "Builder Dual"){
        ui->gradientEndSpinBox->setEnabled(true);
        ui->gradientStartSpinBox->setEnabled(true);
        
        ui->gradientStartColorButton->setEnabled(false);
        ui->gradientEndColorButton->setEnabled(false);
    }
}

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
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFile() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "C://", "Gcode Files(*.gcode);;Text Files (*.txt);;All files (*.*)");
    if (filename.isNull())
        return;
    ui->filePath->setText(filename);
    gcodeFile = new QFile(filename);
    if(!gcodeFile->open(QIODevice::ReadOnly))
        QMessageBox::information(0,"info",gcodeFile->errorString());
    QTextStream in(gcodeFile);
    ui->plainTextEdit->setPlainText(in.readAll());
    in.seek(0);
     processGcode();
}

int MainWindow::countLayers(){
    QTextStream reader(gcodeFile);
    QString currentLine;
    int lineCount = 0;
    while(!reader.atEnd()){
        currentLine = reader.readLine();
        //TODO: Put code in the plainTextEdit widget here, so I only have to run through the gcode file once
        if (currentLine.contains("; layer"))
        {
            lineCount++;
        }
    }
    reader.seek(0);
    return lineCount;

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

    ui->gradientStartColorButton->setEnabled(true);
    ui->gradientStartColorButton->setStyleSheet("background-color: black");
    ui->gradientEndColorButton->setEnabled(true);
    ui->gradientEndColorButton->setStyleSheet("background-color: white");

    ui->retractionCheckbox->setEnabled(true);
    ui->plainTextEdit->setEnabled(true);
}

void MainWindow::on_pushButton_clicked()
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

    //Get the current Colors of the two gradient buttons
    QPalette startPal = ui->gradientStartColorButton->palette();
    QPalette endPal = ui->gradientEndColorButton->palette();
    bool fancyRetraction = ui->retractionCheckbox->isChecked();
    writeGcode(gradientStartLayer,gradientEndLayer,fancyRetraction, startPal,endPal,gcodeFile);
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

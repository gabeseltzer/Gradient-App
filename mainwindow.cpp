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
    ui->filePath->setText(filename);
    gcodeFile = new QFile(filename);
    if(!gcodeFile->open(QIODevice::ReadOnly))
        QMessageBox::information(0,"info",gcodeFile->errorString());
    QTextStream in(gcodeFile);
        //TODO: Takes too long to show on screen
//    ui->fileOutput->setText(in.readAll());
//    in.seek(0);
}

int countLayers(){
    QTextStream reader(gcodeFile);
    QString currentLine;
    int lineCount = 0;
    while(!reader.atEnd()){
        currentLine = reader.readLine();
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

    ui->gradeintStartSpinBox->setEnabled(true);
    ui->gradientEndSpinBox->setEnabled(true);
}

void MainWindow::writeGcode(){
    int gradientStartLayer = ui->gradientStartSlider->value();
    int gradientEndLayer = ui->gradientEndSlider->value();
    int gradientStartPercent = ui->gradeintStartSpinBox->value();
    int gradientEndPercent = ui->gradientEndSpinBox->value();
    int delta = 1;
    if (gradientStartPercent > gradientEndPercent)
        delta = -1;
    int nextGLayer = gradientStartLayer;
    int gAmmount = gradientStartPercent;
    int interval = calculateGradientShifts(gradientStartLayer, gradientEndLayer, gradientStartPercent, gradientEndPercent);
    QString currentLine;
    QString defaultFileSaveName = gcodeFile->fileName();
    defaultFileSaveName = defaultFileSaveName.mid(0,defaultFileSaveName.size()-6) + " Gradient";
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileSaveName, "Gcode Files(*.gcode);;Text Files (*.txt);;All files (*.*)");
    QTextStream reader(gcodeFile);
    QFile saveFile(saveFileName);
    saveFile.open(QIODevice::WriteOnly);
    QTextStream writer(&saveFile);
    currentLine = reader.readLine();
    while (gAmmount != (gradientEndPercent + delta))
    {
//        qDebug() << "Current gAmmount: " + QString::number(gAmmount) + "Current nextGLayer: " + QString::number(nextGLayer) + "Current Line: " + currentLine;
        writer << currentLine + "\n";
        if (currentLine.contains("; layer " + QString::number(nextGLayer)))
        {
            writer << "G93 R" + QString::number(gAmmount) + "\n";
            gAmmount += delta;
            nextGLayer += interval;
        }
        currentLine = reader.readLine();
    }
    writer << currentLine + "\n";
    while (!reader.atEnd())
    {
        currentLine = reader.readLine();
        writer << currentLine + "\n";
    }
    QMessageBox::information(0,"done","All Done!");
}

int MainWindow::calculateGradientShifts(int start, int end, int startPercent, int endPercent)
{
    int range = (end - start)/abs(endPercent - startPercent);
//    qDebug() << (QString::number(end) + " - " + QString::number(start) + ") / abs (" + QString::number(endPercent) + " - " + QString::number(startPercent) + " = " + QString::number(range));
    return range;
}

void MainWindow::on_pushButton_clicked()
{
    openFile();
    processGcode();
}

void MainWindow::on_gradientStartSlider_sliderMoved(int position)
{
    ui->gradientStartTextField->setText(QString::number(position));
}

void MainWindow::on_gradientEndSlider_sliderMoved(int position)
{
    ui->gradientEndTextField->setText(QString::number(position));
}

void MainWindow::on_gradientStartTextField_returnPressed()
{
    ui->gradientStartSlider->setValue(ui->gradientStartTextField->text().toInt());
}

void MainWindow::on_gradientStartTextField_editingFinished()
{
    ui->gradientStartSlider->setValue(ui->gradientStartTextField->text().toInt());
}

void MainWindow::on_gradientEndTextField_editingFinished()
{
    ui->gradientEndSlider->setValue(ui->gradientEndTextField->text().toInt());
}

void MainWindow::on_gradientEndTextField_returnPressed()
{
    ui->gradientEndSlider->setValue(ui->gradientEndTextField->text().toInt());
}

void MainWindow::on_writeGcodeButton_clicked()
{
    writeGcode();
}

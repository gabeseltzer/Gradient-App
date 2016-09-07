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
        //TODO: Takes too long to show on screen
//    ui->fileOutput->setText(in.readAll());
//    in.seek(0);
     processGcode();
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

    ui->gradientStartColorButton->setEnabled(true);
    ui->gradientStartColorButton->setStyleSheet("background-color: black");
    ui->gradientEndColorButton->setEnabled(true);
    ui->gradientEndColorButton->setStyleSheet("background-color: white");
}

void MainWindow::writeGcode(){
    int gradientStartLayer = ui->gradientStartSlider->value();
    int gradientEndLayer = ui->gradientEndSlider->value();

    //Get the current Colors of the two gradient buttons
    QPalette startPal = ui->gradientStartColorButton->palette();
    QColor startColor = startPal.color(QPalette::Button);
    QPalette endPal = ui->gradientEndColorButton->palette();
    QColor endColor = endPal.color(QPalette::Button);

    //Now get the CMY componants of the corresponding colors
    int cyanStartWeight = startColor.cyan();
    int magentaStartWeight = startColor.magenta();
    int yellowStartWeight = startColor.yellow();

    int cyanEndWeight = endColor.cyan();
    int magentaEndWeight = endColor.magenta();
    int yellowEndWeight = endColor.yellow();

    QString currentLine;
    QString defaultFileSaveName = gcodeFile->fileName();
    defaultFileSaveName = defaultFileSaveName.mid(0,defaultFileSaveName.size()-6) + " Gradient";
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileSaveName, "Gcode Files(*.gcode);;Text Files (*.txt);;All files (*.*)");
    if (saveFileName.isNull())
        return;
    QTextStream reader(gcodeFile);
    QFile saveFile(saveFileName);
    saveFile.open(QIODevice::WriteOnly);
    QTextStream writer(&saveFile);

    //Default Layer and Weight deltas
    float cyanWeightDelta = 1.0;
    float magentaWeightDelta = 1.0;
    float yellowWeightDelta = 1.0;

////    First, check to see if we're increasing gradient percent for each color
//    int cyanAascending = 1;
//    if (cyanStartWeight > cyanEndWeight)
//        cyanAscending = -1;
//    int magentaAascending = 1;
//    if (magentaStartWeight > magentaEndWeight)
//        magentaAscending = -1;
//    int yellowAascending = 1;
//    if (yellowStartWeight > yellowEndWeight)
//        yellowAscending = -1;

    //Also, lets keep track of how many percents we're changing (absolute value)
    int cyanActiveWeights = abs(cyanEndWeight - cyanStartWeight);
    int magentaActiveWeights = abs(magentaEndWeight - magentaStartWeight);
    int yellowActiveWeights = abs(yellowEndWeight - yellowStartWeight);

    //Next, calculate the number of layers we have to work with
    int activeLayers = gradientEndLayer - gradientStartLayer;

    //How much should we change the weight of each color per layer
    cyanWeightDelta = (float)cyanActiveWeights/(float)activeLayers;
    magentaWeightDelta = (float)magentaActiveWeights/(float)activeLayers;
    yellowWeightDelta = (float)yellowActiveWeights/(float)activeLayers;

    //Now these are the variables we'll increment in the loop through the input file
    int nextActiveLayer = gradientStartLayer;
    float fCyanNextActiveWeight = cyanStartWeight;
    int cyanNextActiveWeight = cyanStartWeight;
    float fMagentaNextActiveWeight = magentaStartWeight;
    int magentaNextActiveWeight = magentaStartWeight;
    float fYellowNextActiveWeight = yellowStartWeight;
    int yellowNextActiveWeight = yellowStartWeight;


//    qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
//    qDebug() << "ascending: " + QString::number(ascending) + ", percentDelta: " + QString::number(percentDelta) + ", layerDelta: " + QString::number(layerDelta) + ", activeLayers: " + QString::number(activeLayers) + ", activePercents: " + QString::number(activePercents);
    while (!reader.atEnd())
    {
        //Read next line, transcribe it
        currentLine = reader.readLine();
        writer << currentLine + "\n";

        //If this is a layer we need to add a gradient command to, add it
        if ((currentLine.contains("; layer " + QString::number(nextActiveLayer)) && (nextActiveLayer < gradientEndLayer)))
        {
            writer << "M163 S0 P" + QString::number(cyanNextActiveWeight) + "\n" +
                      "M163 S1 P" + QString::number(magentaNextActiveWeight) + "\n" +
                      "M163 S2 P" + QString::number(yellowNextActiveWeight) + "\n" +
                      "M164 S0" + "\n";

            //Adjust the searching variables to find the next active layer
            nextActiveLayer += 1;
            fCyanNextActiveWeight += cyanWeightDelta;
            cyanNextActiveWeight = qRound(fCyanNextActiveWeight);
            fMagentaNextActiveWeight += magentaWeightDelta;
            magentaNextActiveWeight = qRound(fMagentaNextActiveWeight);
            fYellowNextActiveWeight += yellowWeightDelta;
            yellowNextActiveWeight = qRound(fYellowNextActiveWeight);

//            qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
        }
    }
    QMessageBox::information(0,"done","All Done!");
}

//int MainWindow::calculateGradientShifts(int start, int end, int startPercent, int endPercent)
//{
//    int range = (end - start)/abs(endPercent - startPercent);
////    qDebug() << (QString::number(end) + " - " + QString::number(start) + ") / abs (" + QString::number(endPercent) + " - " + QString::number(startPercent) + " = " + QString::number(range));
//    return range;
//}

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
    writeGcode();
}

void MainWindow::on_gradientStartColorButton_clicked()
{
    QPalette pal = ui->gradientStartColorButton->palette();
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Button));
    QString newStyle = "background-color: " + newColor.name();
    ui->gradientStartColorButton->setStyleSheet(newStyle);
    ui->gradientStartColorButton->update();
}

void MainWindow::on_gradientEndColorButton_clicked()
{
    QPalette pal = ui->gradientEndColorButton->palette();
    QColor newColor = QColorDialog::getColor(pal.color(QPalette::Button));
    QString newStyle = "background-color: " + newColor.name();
    ui->gradientEndColorButton->setStyleSheet(newStyle);
    ui->gradientEndColorButton->update();
}

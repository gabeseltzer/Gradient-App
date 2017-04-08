#include <QDebug>
#include <QFile>
#include <QPalette>
#include <QFileDialog>
#include <QMessageBox>
#include <gradientcolor.h>


static QVariant findECoordinate(QString currentLine){
    QStringList splitLine = currentLine.split(" ", QString::SkipEmptyParts);
    QStringListIterator itr(splitLine);
    while(itr.hasNext()) {
        QString cur = itr.next();
        //Detect comment
        if (cur.startsWith(";"))
            break;
        if (cur.startsWith('E')){
            qDebug() << "Found an E coordinate: " + (cur.midRef(1,(cur.length()-2))).toString();
            return QVariant(cur.midRef(1, (cur.length()-2)).toFloat()); //This has no error detection
        }
    }
    return QVariant(QString());
}

static int findToolChange(QString currentLine){
    return (currentLine.midRef(1,1).toInt()); //This has no error detection
}


static void writeGcode(int gradientStartLayer, int gradientEndLayer, bool fancyRetraction, QPalette startPal, QPalette endPal, QFile *inputFile){

    //Get the current Colors of the two gradient buttons
    QColor startColor = startPal.color(QPalette::Button);
    QColor endColor = endPal.color(QPalette::Button);

    //Next, calculate the number of layers we have to work with
    int activeLayers = gradientEndLayer - gradientStartLayer;
    
    //Now get the CMY componants of the corresponding colors
    int startWeight = startColor.cyan();
    int endWeight = endColor.cyan();
    gradientColor cyan(startWeight, endWeight, activeLayers, 0);
    
    startWeight = startColor.magenta();
    endWeight = endColor.magenta();
    gradientColor yellow(startWeight, endWeight, activeLayers, 1);
    
    startWeight = startColor.yellow();
    endWeight = endColor.yellow();
    gradientColor magenta(startWeight, endWeight, activeLayers, 2);

    QString currentLine;
    QString defaultFileSaveName = inputFile->fileName();
    defaultFileSaveName = defaultFileSaveName.mid(0,defaultFileSaveName.size()-6) + " Gradient";
    QString saveFileName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"), defaultFileSaveName, "Gcode Files(*.gcode);;Text Files (*.txt);;All files (*.*)");
    if (saveFileName.isNull())
        return;
    QTextStream reader(inputFile);
    if (reader.atEnd())
        reader.seek(0);
    QFile saveFile(saveFileName);
    saveFile.open(QIODevice::WriteOnly);
    QTextStream writer(&saveFile);

    QVariant newECoordinateVariant;
    float prevECoordinate = 0.0;
    bool currentlyRetracting = false;
    int previousTool = 0;

    //Now these are the variables we'll increment in the loop through the input file
    int nextActiveLayer = gradientStartLayer;


//    qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
//    qDebug() << "ascending: " + QString::number(ascending) + ", percentDelta: " + QString::number(percentDelta) + ", layerDelta: " + QString::number(layerDelta) + ", activeLayers: " + QString::number(activeLayers) + ", activePercents: " + QString::number(activePercents);
    while (!reader.atEnd())
    {
        //Read next line, transcribe it
        currentLine = reader.readLine();

        //Handle Fancy Retraction:
        if (fancyRetraction){
            
            QStringList splitLine = currentLine.split(" ", QString::SkipEmptyParts);
            QString command = splitLine[0];
            if (command == "G1" || command == "G0")
                newECoordinateVariant = findECoordinate(currentLine);
            else if (command == "G92")
                prevECoordinate = 0.0;
            else if (command.startsWith('T'))
                previousTool = findToolChange(currentLine); //Not sure about this interaction
            
//            QString commandType = identifyCommand(currentLine); //Todo: this should probably use an enum or something
//            if (commandType == "G1" || commandType == "G0")
//                int newECoordinate = findECoordinate(currentLine);
//            else if (commandType == "G92") {
//                int prevEcoordinate = 0;
//            }
//            else if (commnadType == "T*")
                
            qDebug() << "Command: " + command + ", prevECoordinate: " + QString::number(prevECoordinate) + ", newECoordinate: " + newECoordinateVariant.toString();
            if (!newECoordinateVariant.isNull()){
                float newECoordinate = newECoordinateVariant.toFloat();
                if (currentlyRetracting){
                    if (newECoordinate > prevECoordinate){
                        writer << "//STOP RETRACTION \n";
                        writer << (currentLine + "\n");
                        writer << ("T" + QString::number(previousTool) + "\n");
                        currentlyRetracting = false;
                        prevECoordinate = newECoordinate;
                    }
                    else
                        writer << currentLine + "\n";
                }
                else{
                    if (newECoordinate < prevECoordinate){
                        writer << "//START RETRACTION \n";
                        writer << "T15 \n";
                        writer << (currentLine + "\n");
                        currentlyRetracting = true;
                        prevECoordinate = newECoordinate;
                    }
                    else
                        writer << currentLine + "\n";
                }
            }
            else
                writer << currentLine + "\n";

        }
        else {
            writer << currentLine + "\n";

        }

        //If this is a layer we need to add a gradient command to, add it
        if ((currentLine.contains("; layer " + QString::number(nextActiveLayer)) && (nextActiveLayer < gradientEndLayer)))
        {
//            //Adjust the searching variables to find the next active layer
            nextActiveLayer += 1;

            writer << cyan.printAndIncrement();
            writer << yellow.printAndIncrement();
            writer << magenta.printAndIncrement();
            writer << "M164 S0\n";

//            qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
        }
    }
    QMessageBox::information(0,"done","All Done!");
}

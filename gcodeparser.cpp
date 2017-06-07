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
    return (currentLine.midRef(1,1).toInt()); //TODO: This has no error detection
}

//WRITE GCODE FOR DIAMOND HOTEND
static void writeGcode(int gradientStartLayer, int gradientEndLayer, 
                       bool fancyRetraction, 
                       QPalette startPal, QPalette endPal, 
                       QTextStream *reader, QTextStream *writer,
                       bool fiveColor){

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
    
    startWeight = startColor.black();
    endWeight = endColor.black();
    gradientColor black(startWeight, endWeight, activeLayers, 3);
    
    startWeight = 256 - startWeight;
    endWeight = 256 - endWeight;
    gradientColor white(startWeight, endWeight, activeLayers, 4);
    
    QString currentLine;
    
    QVariant newECoordinateVariant;
    float prevECoordinate = 0.0;
    bool currentlyRetracting = false;
    int previousTool = 0;

    //Now these are the variables we'll increment in the loop through the input file
    int nextActiveLayer = gradientStartLayer;


//    qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
//    qDebug() << "ascending: " + QString::number(ascending) + ", percentDelta: " + QString::number(percentDelta) + ", layerDelta: " + QString::number(layerDelta) + ", activeLayers: " + QString::number(activeLayers) + ", activePercents: " + QString::number(activePercents);
    while (!reader->atEnd())
    {
        //Read next line, transcribe it
        currentLine = reader->readLine();

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
                
//            qDebug() << "Command: " + command + ", prevECoordinate: " + QString::number(prevECoordinate) + ", newECoordinate: " + newECoordinateVariant.toString();
            if (!newECoordinateVariant.isNull()){
                float newECoordinate = newECoordinateVariant.toFloat();
                if (currentlyRetracting){
                    if (newECoordinate > prevECoordinate){
//                        *writer << "//STOP RETRACTION \n";
                        *writer << (currentLine + "\n");
                        *writer << ("T" + QString::number(previousTool) + "\n");
                        currentlyRetracting = false;
                        prevECoordinate = newECoordinate;
                    }
                    else
                        *writer << currentLine + "\n";
                }
                else{
                    if (newECoordinate < prevECoordinate){
//                        *writer << "//START RETRACTION \n";
                        *writer << "T15 \n";
                        *writer << (currentLine + "\n");
                        currentlyRetracting = true;
                        prevECoordinate = newECoordinate;
                    }
                    else
                        *writer << currentLine + "\n";
                }
            }
            else
                *writer << currentLine + "\n";

        }
        else {
            *writer << currentLine + "\n";

        }

        //If this is a layer we need to add a gradient command to, add it
        if ((currentLine.contains("; layer " + QString::number(nextActiveLayer)) && (nextActiveLayer < gradientEndLayer)))
        {
//            //Adjust the searching variables to find the next active layer
            nextActiveLayer += 1;

            *writer << cyan.printAndIncrement();
            *writer << yellow.printAndIncrement();
            *writer << magenta.printAndIncrement();
            if (fiveColor) {
                *writer << black.printAndIncrement();
                *writer << white.printAndIncrement();
            }
            *writer << "M164 S0\n";

//            qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
        }
    }
    QMessageBox::information(0,"done","All Done!");
}

//WRITE GCODE FOR BUILDER DUAL
static void writeGcode(int gradientStartLayer, int gradientEndLayer,
                       bool fancyRetraction,
                       int gradientStartPercent, int gradientEndPercent,
                       QTextStream *reader, QTextStream *writer){
    
    QString currentLine;
    
    //Default Layer and Percent deltas
    float percentDelta = 1.0;
    float layerDelta = 1.0;

    //First, check to see if we're increasing gradient percent
    int ascending = 1;
    if (gradientStartPercent > gradientEndPercent)
        ascending = -1;

    //Also, lets keep track of how many percents we're changing (absolute value)
    int activePercents = abs(gradientEndPercent - gradientStartPercent);
    //Next, calculate the number of layers we have to work with
    int activeLayers = gradientEndLayer - gradientStartLayer;

    //See if we're moving more than one percent at a time, or more than one layer at a time
    if (activeLayers < activePercents)
        percentDelta = (float)activePercents/(float)activeLayers;
    else
        layerDelta = (float)activeLayers/(float)activePercents;

    float fNextActiveLayer = gradientStartLayer;
    int nextActiveLayer = gradientStartLayer;
    float fNextActivePercent = gradientStartPercent;
    int nextActivePercent = gradientStartPercent;

    QVariant newECoordinateVariant;
    float prevECoordinate = 0.0;
    bool currentlyRetracting = false;
    int previousTool = 0;
    
    while (!reader->atEnd())
    {
        //Read next line, transcribe it
        currentLine = reader->readLine();
        
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
                
            qDebug() << "Command: " + command + ", prevECoordinate: " + QString::number(prevECoordinate) + ", newECoordinate: " + newECoordinateVariant.toString();
            if (!newECoordinateVariant.isNull()){
                float newECoordinate = newECoordinateVariant.toFloat();
                if (currentlyRetracting){
                    if (newECoordinate > prevECoordinate){
                        *writer << "//STOP RETRACTION \n";
                        *writer << (currentLine + "\n");
                        *writer << ("G93 R" + QString::number(nextActivePercent-percentDelta) + "\n");
                        currentlyRetracting = false;
                        prevECoordinate = newECoordinate;
                    }
                    else
                        *writer << currentLine + "\n";
                }
                else{
                    if (newECoordinate < prevECoordinate){
                        *writer << "//START RETRACTION \n";
                        *writer << "G93 R50 \n";
                        *writer << (currentLine + "\n");
                        currentlyRetracting = true;
                        prevECoordinate = newECoordinate;
                    }
                    else
                        *writer << currentLine + "\n";
                }
            }
            else
                *writer << currentLine + "\n";

        }
        else {
            *writer << currentLine + "\n";

        }

        //If this is a layer we need to add a gradient command to, add it
        if ((currentLine.contains("; layer " + QString::number(nextActiveLayer)) && (nextActiveLayer < gradientEndLayer)))
        {
            if (!currentlyRetracting)
                *writer << "G93 R" + QString::number(nextActivePercent) + "\n";

            //Adjust the searching variables to find the next active layer
            fNextActiveLayer += layerDelta;
            nextActiveLayer = qRound(fNextActiveLayer);
            fNextActivePercent += (percentDelta * ascending);
            nextActivePercent = qRound(fNextActivePercent);
//            qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
        }
    }
    QMessageBox::information(0,"done","All Done!");
}


    
static int calculateGradientShifts(int start, int end, int startPercent, int endPercent)
    {
        int range = (end - start)/abs(endPercent - startPercent);
    //    qDebug() << (QString::number(end) + " - " + QString::number(start) + ") / abs (" + QString::number(endPercent) + " - " + QString::number(startPercent) + " = " + QString::number(range));
        return range;
}

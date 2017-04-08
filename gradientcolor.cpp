#include "gradientcolor.h"

gradientColor::gradientColor(int startWeight, int endWeight, int activeLayers, int colorExtruder)
{
    gradientColor::startWeight = startWeight;
    gradientColor::endWeight = endWeight;
    gradientColor::activeLayers = activeLayers;
    gradientColor::colorExtruder = colorExtruder;
    
    (startWeight > endWeight) ? gradientColor::ascending = -1 : gradientColor::ascending = 1;
    int activeWeights = abs(endWeight - startWeight);
    gradientColor::weightDelta = (float)activeWeights/(float)activeLayers;
    gradientColor::nextActiveWeight = startWeight;
    gradientColor::floatNextActiveWeight = startWeight;
    
}

QString gradientColor::printAndIncrement(){
    QString toReturn = QString("M163 S%1 P%2\n").arg(QString::number(colorExtruder),QString::number(nextActiveWeight));
    floatNextActiveWeight += weightDelta*ascending;
    nextActiveWeight = qRound(floatNextActiveWeight);
    return toReturn;
}

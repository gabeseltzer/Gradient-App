#ifndef GRADIENTCOLOR_H
#define GRADIENTCOLOR_H
#include <QString>

class gradientColor
{
public:
    gradientColor(int startWeight, int endWeight, int activeLayers, int colorExtruder);
    QString printAndIncrement();
    
private:
    int startWeight;
    int endWeight;
    int colorExtruder;
    int ascending;
    int activeLayers;
    float floatNextActiveWeight;
    int nextActiveWeight;
    float weightDelta;
    
};


#endif // GRADIENTCOLOR_H

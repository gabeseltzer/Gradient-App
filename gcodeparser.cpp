#include <QDebug>
#include <QFile>
#include <QPalette>
#include <QFileDialog>
#include <QMessageBox>

static void writeGcode(int gradientStartLayer, int gradientEndLayer, QPalette startPal, QPalette endPal, QFile *inputFile){

    //Get the current Colors of the two gradient buttons
    QColor startColor = startPal.color(QPalette::Button);
    QColor endColor = endPal.color(QPalette::Button);

    //Now get the CMY componants of the corresponding colors
    int cyanStartWeight = startColor.cyan();
    int magentaStartWeight = startColor.magenta();
    int yellowStartWeight = startColor.yellow();

    int cyanEndWeight = endColor.cyan();
    int magentaEndWeight = endColor.magenta();
    int yellowEndWeight = endColor.yellow();

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

    //Default Layer and Weight deltas
    float cyanWeightDelta = 1.0;
    float magentaWeightDelta = 1.0;
    float yellowWeightDelta = 1.0;

//    First, check to see if we're increasing gradient percent for each color
    int cyanAscending = 1;
    if (cyanStartWeight > cyanEndWeight)
        cyanAscending = -1;
    int magentaAscending = 1;
    if (magentaStartWeight > magentaEndWeight)
        magentaAscending = -1;
    int yellowAscending = 1;
    if (yellowStartWeight > yellowEndWeight)
        yellowAscending = -1;

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
            fCyanNextActiveWeight += cyanWeightDelta*cyanAscending;
            cyanNextActiveWeight = qRound(fCyanNextActiveWeight);
            fMagentaNextActiveWeight += magentaWeightDelta*magentaAscending;
            magentaNextActiveWeight = qRound(fMagentaNextActiveWeight);
            fYellowNextActiveWeight += yellowWeightDelta*yellowAscending;
            yellowNextActiveWeight = qRound(fYellowNextActiveWeight);

//            qDebug() << "fNextActiveLayer:" + QString::number(fNextActiveLayer) + " , nextActiveLayer: " + QString::number(nextActiveLayer) + " , fNextActivePercent: " + QString::number(fNextActivePercent) + " , nextActivePercent: " + QString::number(nextActivePercent);
        }
    }
    QMessageBox::information(0,"done","All Done!");
}


static void retractionProcess(){
    qDebug() << "CALLED RETRACTION PROCESS";
////    std::wcout << argv[2];
////------------------------------------------INITIALIZATION---------------------------------------------------
//   FILE *fp;
//   FILE *output;
//   char a[200];
//   int b = 1; // initialize b
//   char c[200];
//   int n=0;
//   int p=0;
//   int k=0;
//   char line[100]= "";//variable used to store current line
//   char number[10]="";//variable used to store E movement value before converting to number
//   double dnumber = 0;//variable used to store E movement value
//   double lastdnumber = 0;//used to store the last E value so it can be compared to the current E value- separate values depending on current extruder
//   int retract = 0;
//   int previoustool=0;//set the "previous tool" to T0 by default


//   std::string inputPath = argv[2];
//   std::string outputPath = inputPath.substr(0,(inputPath.find_last_of("."))) + "output.gcode";
////   std::cout << outputPath.c_str();
//   fp=fopen(argv[2], "r"); //open the input file
//   output=fopen(outputPath.c_str(), "w"); //create output file
//   fprintf(output,"M163 S0 P1 \nM163 S1 P1 \nM163 S2 P1 \nM164 S15 \n;retraction virtual extruder\n"); //set the virtual extruder used for retracting (ext 15)
////   fprintf(output,"M163 S0 P1 \nM163 S1 P0 \nM163 S2 P0 \nM164 S0 \n");//set the first extruder to be only extuder 1 in case its not set in slicer
////   fprintf(output,"M163 S0 P0 \nM163 S1 P1 \nM163 S2 P0 \nM164 S1 \n");//set the second extruder to be only extuder 2 in case its not set in slicer
////   fprintf(output,"M163 S0 P0 \nM163 S1 P0 \nM163 S2 P1 \nM164 S1 \n");//set the third extruder to be only extuder 3 in case its not set in slicer
//   b=fgetc(fp); // get first character from the file
//   fputs("; Added Fancy Retraction \n", output);


////-------------------------------------START OF MAIN LOOP---------------------------------------------------------
//while(b!=EOF) //loop over all characters in file
//{
////    std::cout << "New Loop" + b;

////***** CASE 1 (Comment)********
//   switch (b) //check type of character
//   {
//          case ';': //start of comment. Ignore all lines until new line is found

//               while(b!='\n')
//               {
//                        fputc(b, output);
//                        b=fgetc(fp);
//               }
//               fputc('\n',output);
//               //fprintf(output, "End of comment \n");
//          break;



// //********************

// //***** CASE 2 (G command - G1, G0, or G92 E0 is what we care about)********
//          case 'G': //we're going to need to load this line into a string variable (line) in order to analyze it

//                for (n=0;n<100;n++)//for up to 100 characters
//                {
//                    line[n]=b;//store into string
//                    line[n+1]='\0';//end string after this character
//                    b=fgetc(fp);//get next character
//                    if (b=='\n'||b==EOF)
//                       break; //exit out if a carriage return or EOF is found
//                    if (b==';')
//                    {
//                       ungetc(b,fp);//moves the position back 1 so it can be read as a comment on the next loop
//                       break;
//                    }
//                }
//                //analyze string for G1 or G0
//                if (line[1]=='0' || line[1]=='1')//analyze string for G1 or G0
//                {
//                   //fprintf(output, "The following is a G1 or G0 command \n");

//                   if (strstr(line,"E")!=NULL)//analyze string for E command
//                   {
//                      //fprintf(output, "The following contains an E move \n");
//                      for (n=0;n<100;n++)
//                      {
//                          if (line[n]=='E')
//                             {
//                                           k = n;//store position of 'E'
//                                           //fprintf(output,"The position of E is %i\n",k);
//                                           break;
//                             }
//                      }
//                      //convert characters after the E to a "float" number (fnumber variable)
//                      for (n=0;n<11;n++)
//                      {
//                          if (isdigit(line[k+n+1])||line[k+n+1]=='.'||line[k+n+1]=='-')
//                          {
//                          }
//                          else
//                              break;
//                          number[n]=line[k+n+1];
//                          number[n+1]='\0';//end string after this character
//                      }

//                      dnumber=strtod(number,NULL);
//                      //fprintf(output, "The string number is %s \n",number);
//                      //fprintf(output, "The float number is %f \n",dnumber);
//                      //fprintf(output, "The previous float number is %f \n",lastdnumber);
//                      if (dnumber<lastdnumber)//detected retract move
//                      {
//                            fprintf(output, "T15\n");//put in the temporary mixing ratio before this move
//                            //fprintf(output, ";This is a retraction\n");
//                            retract = 1;//flag for next loop to see if its un-retraction
//                            fprintf(output,line); //writes current line to file
//                            fputc('\n',output);//ends line
//                            fprintf(output, "T%i\n",previoustool);//switch back to previous tool
//                      }
//                      else if (retract==1)//the e move is right after a retraction move, then its an "unretraction".
//                      {
//                            //Reset the mixing ratio after this move.
//                            //fprintf(output, ";This is an unretraction\n");
//                            retract=0;//reset flag
//                            fprintf(output, "T15\n");//put in the temporary mixing ratio before this move
//                            fprintf(output,line); //writes line to file
//                            fputc('\n',output);//ends line
//                            fprintf(output, "T%i\n",previoustool);//switch back to previous tool
//                      }
//                      else // if neither of these are true then just add the line with no modifications
//                      {
//                           fprintf(output,line); //writes line to file
//                           fputc('\n',output);//ends line
//                      }
//                      lastdnumber=dnumber;//store into last distance so it can be compared on next loop
//                   }
//                   else
//                   {
//                       fprintf(output,line);
//                       fputc('\n',output);//ends line
//                   }
//                }
//                else if (line[1]=='9'&&line[2]=='2'&&line[4]=='E'&&line[5]=='0')//G92 E0 command
//                {
//                     lastdnumber=0;//G92 E0 command resets the E position
//                     fprintf(output,line);
//                     fputc('\n',output);//ends line
//                     //fprintf(output,";reset E position\n");
//                }
//                else
//                {
//                    fprintf(output,line);
//                    fputc('\n',output);//ends line
//                }

//                //fprintf(output, "End of G command \n");

//         break;
////***********************
////********CASE 3 (Tool change TX)****************
//         case 'T':
//              fputc(b,output);//write T into the file.
//              b=fgetc(fp);//get the next character (should be the first digit of the tool number)
//              previoustool = b - '0';//convert to integer number and stores for the retraction code
//              fputc(b,output);//write into file
//              b=fgetc(fp);//get the next character (should be the second digit of the tool number or something else)
//              if (isdigit(b))
//                 previoustool = previoustool*10+(b - '0');//tens place
//              fputc(b,output);//write into file

//         break;

// //***** CASE 3 (DEFAULT)********

//         default: //if anything else, just add it back into the output
//         fputc(b,output);
//         break;
////********************************

//}

//   b=fgetc(fp); // get next character from the file

////---------------------------------------- END OF MAIN LOOP-------------------------------------------
//}

////-----------------------------------------COMPLETE---------------------------------------------------


//   printf("Done");

//   //scanf("%s", c);


//   return 0;


}

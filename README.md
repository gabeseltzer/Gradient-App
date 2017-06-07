# Gradient-App
A QT Gui app for postprocessing Gcode to add a gradient. Works with Simplify3D only for the moment
This app was created by Gabriel Seltzer. Some of the code/ideas came from Colby Lippincott. 
It is licensed under LGPL V3. There's a copy of the license in the repo, as per the requirements of the license.

**Please post any and all issues so I can fix them, and share your results with me as well! I want to know that my code works, so send pictures!**

Check out my first prerelease here: https://github.com/Memnarch1113/Gradient-App/releases

##A few notes:

1. THIS PROGRAM ONLY WORKS WITH SIMPLIFY3D, as it uses Simplify3D's comments to detect layer changes. If you're using another slicer, you must put in the same comments as are in Simplify3D to mark layer changes. This may get updated in the future...
2. To take advantage of fancy retraction on a diamond hotend, make sure that your T15 is set to equal parts of each extruder, that way you retract all filament at once.
3. If printing with a diamond hotend, you must load your filament like so: Extuder 0 -> Cyan, Extruder 1 -> Magenta, Extruder 2 -> Yellow (Extruder 3 -> Black, Extruder 4 -> White if you have the 5-color varient)
4. All gradients will be applied to Tool 0, so plan your slicing accordingly.
5. I have no idea what I'm doing with the 5 color gradients, so your mialage may be terrible. If you know how to convert rgb colors to cmykw, that would be helpful, as I'm pretty sure my current algorithm is wrong...

##Known Bugs:

1. I'm not sure if the retraction code currently works with T numbers greater than 9 (that's a bug for now)

##To implement in the future:

1. Work with other slicers besides Simplify3D
2. Show the changes that will be made in the text window, and allow the user to edit the gcode manually
3. Let user select which tool they'd like the gradient to be mapped to
4. 3D visualization of your slice, with color coding (heh, never gonna happen)
5. 

To build:

- cd libs; ./initLibraries.sh; cd .. 
- qmake Tool.pro
- make

To run examples:

-Go to "test" folder and choose one of the following dynamics benchmark (moon landing with 1 or 2 dimensions or the vehicle dynamics)
-Run the script "classFolderGenerator.py" to generate the folders with different run configurations (coarse, fine and steady with a few different options).
-Inside these run configuration folders, you can modify the "abstraction.conf" file to modify the parameters of the workspace. If you do so, it is very likely you will need to change the "slugsExample.structuredslugs" file, which contains the specification to run the abstraction.
-Run the script "run.sh" to start the generation of the abstraction (this may take a while). Once is finish, the abstraction will be contained in "slugsExample.bdd".
-Run the script "checkRealizable.sh" to see if the abstraction is realisable with the specification defined in "slugsExample.structuredslugs".
-You can run the "runGraphicalSimulator.sh" script to see a graphical simulation of system with the defined specification. If you are running the "Fine" version of the vehicle, please, overwrite the file "slugsExample.png" with the file "slugsExampleFine.png" that you can fine in the "dataFile" folder.

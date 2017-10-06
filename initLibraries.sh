#This script creates the libraries needed fot the abstraction processor
#SCOTSv0.2: https://gitlab.lrz.de/matthias/SCOTSv0.2.git
#slugs: https://github.com/VerifiableRobotics/slugs.git
#picosat: http://fmv.jku.at/picosat/picosat-965.tar.gz
#lingeling: http://fmv.jku.at/lingeling/lingeling-bbc-9230380-160707.tar.gz

cd libs
wget http://fmv.jku.at/picosat/picosat-965.tar.gz
tar -xvzf picosat-965.tar.gz
rm picosat-965.tar.gz
cd picosat-965
./configure && make
cd ..
wget http://fmv.jku.at/lingeling/lingeling-bbc-9230380-160707.tar.gz
tar -xvzf lingeling-bbc-9230380-160707.tar.gz
rm lingeling-bbc-9230380-160707.tar.gz
cd lingeling-bbc-9230380-160707.tar.gz
./configure.sh && make
cd ..
git clone https://gitlab.lrz.de/matthias/SCOTSv0.2.git
git clone https://github.com/VerifiableRobotics/slugs.git
cd slugs/src; make
cd ../../..


#!/usr/bin/env bash
set -e
#This script downloads the libraries needed for the abstraction processor
#SCOTSv0.2: https://gitlab.lrz.de/matthias/SCOTSv0.2.git
#slugs: https://github.com/VerifiableRobotics/slugs.git
#picosat: http://fmv.jku.at/picosat/picosat-965.tar.gz
#lingeling: http://fmv.jku.at/lingeling/lingeling-bbc-9230380-160707.tar.gz

# =====================================================================
# Check if run from the correct directory
# =====================================================================
if [ ! -f initLibraries.sh ]; then
    echo "Error: Please start this script from the directory it is located in."
    exit
fi

# =====================================================================
# Picosat
# =====================================================================
if [ ! -f picosat-965/picosat ]; then
    echo "========================================"
    echo "Downloading and building picosat"
    echo "========================================"    
    wget http://fmv.jku.at/picosat/picosat-965.tar.gz -O picosat-965.tar.gz
    tar -xvzf picosat-965.tar.gz
    # rm picosat-965.tar.gz
    cd picosat-965
    ./configure.sh
    make
    cd ..    
fi    

# =====================================================================
# Lingeling
# =====================================================================    
if [ ! -f lingeling-bbc-9230380-160707/lingeling ]; then
    echo "========================================"
    echo "Downloading and building lingeling"
    echo "========================================"    
    wget http://fmv.jku.at/lingeling/lingeling-bbc-9230380-160707.tar.gz -O lingeling-bbc-9230380-160707.tar.gz
    tar -xvzf lingeling-bbc-9230380-160707.tar.gz
    # rm lingeling-bbc-9230380-160707.tar.gz
    cd lingeling-bbc-9230380-160707
    ./configure.sh
    make
    cd ..
fi

# =====================================================================
# SCOTS
# =====================================================================    
if [ ! -d SCOTSv0.2 ]; then
    echo "========================================"
    echo "Checking out SCOTS"
    echo "========================================"    
    git clone https://gitlab.lrz.de/matthias/SCOTSv0.2.git
fi
cd SCOTSv0.2
git checkout ff25c857d0d7e43cfc4296b7be2600f0d8b04626
cd ..

# =====================================================================
# SLUGS - Checking out
# =====================================================================    
if [ ! -d slugs ]; then
    echo "========================================"
    echo "Checking out SLUGS"
    echo "========================================"    
    git clone https://github.com/VerifiableRobotics/slugs.git
fi

# =====================================================================
# SLUGS - Building
# =====================================================================        
cd slugs; git checkout unstable; touch lib/cudd-3.0.0/aclocal.m4; touch lib/cudd-3.0.0/Makefile.in; cd src; make -j; cd ../..

# =====================================================================
# CUDD - Building will all libraries
# =====================================================================        
cd slugs/lib/cudd-3.0.0; ./configure --enable-dddmp --enable-obj --enable-shared --enable-static; make -j; cd ../../..

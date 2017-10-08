# QMake Build file
QMAKE_CC = gcc
QMAKE_LINK_C = g++
QMAKE_CXX = g++
QMAKE_LINK = g++
CC = gcc

DEFINES += USE_CUDD SCOTS_BDD NLGLYALSAT NLGLDRUPLIG NDEBUG BUILD_SCOTS_ABSTRACTION_FILE # PRINT_SAT_INSTANCE
CFLAGS += -g -fpermissive

QMAKE_LFLAGS_X86_64 = -arch x86_64

QMAKE_CFLAGS_X86_64 = -arch x86_64
QMAKE_CXXFLAGS_X86_64 = -arch x86_64

BDDFLAGS = -Wall -Wextra -std=c++14 -Wno-sign-compare -Wno-unused-parameter

QMAKE_CFLAGS_RELEASE += -g
QMAKE_CXXFLAGS_RELEASE += -g  \
    $$BDDFLAGS
QMAKE_CFLAGS_DEBUG += -g -Wall -Wextra
QMAKE_CXXFLAGS_DEBUG += -g -Wall -Wextra \
    $$BDDFLAGS
    
TEMPLATE = app \
    console
CONFIG += release
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += libs/SCOTSv0.2/utils/RungeKutta4.hh \
           libs/SCOTSv0.2/src/scots.hh \
           include/dynamics.hh \
           include/SCOTSAbstraction.hh \
           include/vehicleDynamics.hh \
           include/satSolvers.hpp \
           include/smartAbstractionProcessor.hh \
           include/bareAbstractionProcessor.hh \
           include/workspaceProperties.hh \
           libs/picosat-965/picosat.h \
           libs/SCOTSv0.2/src/TransitionFunction.hh \
           libs/SCOTSv0.2/src/UniformGrid.hh \
           libs/SCOTSv0.2/src/Abstraction.hh \
           libs/SCOTSv0.2/src/GameSolver.hh \
           libs/SCOTSv0.2/src/WinningDomain.hh \
           libs/SCOTSv0.2/src/StaticController.hh \
           libs/SCOTSv0.2/src/InputOutput.hh \
           libs/BFAbstractionLibrary/BFCudd.h \
           libs/BFAbstractionLibrary/BFCuddManager.h \
           libs/BFAbstractionLibrary/bddDump.h \
           libs/BFAbstractionLibrary/BF.h \
           include/tools.hh \
           include/vehicleDynamicsSimple.hh \
           include/moonLandingDynamics.hh \
           include/moonLanding1DDynamics.hh \
           include/vehicleDynamicsSingleSpeed.hh \
           include/hybridAbstractionProcessor.hh

SOURCES += libs/BFAbstractionLibrary/BFCuddManager.cpp libs/BFAbstractionLibrary/bddDump.cpp \
    libs/picosat-965/picosat.c \
    libs/lingeling-bbc-9230380-160707/lglib.c \
    libs/lingeling-bbc-9230380-160707/lglopts.c \
    mainProject.cc

TARGET = mainProject
INCLUDEPATH = include libs/SCOTSv0.2/src libs/SCOTSv0.2/utils libs/slugs/lib/cudd-3.0.0 libs/slugs/lib/cudd-3.0.0/cudd libs/slugs/lib/cudd-3.0.0/epd libs/slugs/lib/cudd-3.0.0/dddmp libs/slugs/lib/cudd-3.0.0/cplusplus libs/slugs/lib/cudd-3.0.0/mtr libs/slugs/lib/cudd-3.0.0/util libs/slugs/lib/cudd-3.0.0/st libs/picosat-965 libs/lingeling-bbc-9230380-160707 libs/BFAbstractionLibrary

LIBS += -static -L$$_PRO_FILE_PWD_/libs/slugs/lib/cudd-3.0.0/cudd/.libs -lcudd

PKGCONFIG += 
QT -= gui \
    core



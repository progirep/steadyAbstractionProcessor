#================================= Steady Abstraction Processor Makefile =====================================

# Definitions
DEFINES       = -DUSE_CUDD -DSCOTS_BDD -DNLGLYALSAT -DNLGLDRUPLIG -DNDEBUG
CFLAGS        = -pipe -O3 -g -Wall -W -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O3 -g -Wall -Wextra -std=c++14 -Wno-sign-compare -Wno-unused-parameter -Wall -W -fPIC $(DEFINES)
INCPATH       = -I. -Iinclude -Ilibs/SCOTSv0.2/src -Ilibs/SCOTSv0.2/utils -Ilibs/slugs/lib/cudd-3.0.0 -Ilibs/slugs/lib/cudd-3.0.0/cudd -Ilibs/slugs/lib/cudd-3.0.0/epd -Ilibs/slugs/lib/cudd-3.0.0/dddmp -Ilibs/slugs/lib/cudd-3.0.0/cplusplus -Ilibs/slugs/lib/cudd-3.0.0/mtr -Ilibs/slugs/lib/cudd-3.0.0/util -Ilibs/slugs/lib/cudd-3.0.0/st -Ilibs/picosat-965 -Ilibs/lingeling-bbc-9230380-160707 -Ilibs/BFAbstractionLibrary 
LINK          = $(CXX)
LFLAGS        = -Llibs/slugs/lib/cudd-3.0.0/cudd/.libs -lcudd
INCPATH       = -I. -Iinclude -Ilibs/SCOTSv0.2/src -Ilibs/SCOTSv0.2/utils -Ilibs/slugs/lib/cudd-3.0.0 -Ilibs/slugs/lib/cudd-3.0.0/cudd -Ilibs/slugs/lib/cudd-3.0.0/epd -Ilibs/slugs/lib/cudd-3.0.0/dddmp -Ilibs/slugs/lib/cudd-3.0.0/cplusplus -Ilibs/slugs/lib/cudd-3.0.0/mtr -Ilibs/slugs/lib/cudd-3.0.0/util -Ilibs/slugs/lib/cudd-3.0.0/st -Ilibs/picosat-965 -Ilibs/lingeling-bbc-9230380-160707 -Ilibs/BFAbstractionLibrary 


# Object files
OBJECTS       = BFCuddManager.o \
		bddDump.o \
		picosat.o \
		lglib.o \
		lglopts.o \
		mainProject.o


# Executable & Cleaning
mainProject: $(OBJECTS)
	$(LINK) -o mainProject $(OBJECTS) $(LFLAGS)

clean: 
	rm -f $(OBJECTS) mainProject

BFCuddManager.o: libs/BFAbstractionLibrary/BFCuddManager.cpp libs/BFAbstractionLibrary/BF.h \
		libs/BFAbstractionLibrary/BFCuddManager.h \
		libs/slugs/lib/cudd-3.0.0/cudd/cudd.h \
		libs/BFAbstractionLibrary/BFCudd.h \
		libs/BFAbstractionLibrary/BFCuddVarCube.h \
		libs/BFAbstractionLibrary/BFCuddVarVector.h \
		libs/BFAbstractionLibrary/BFCuddComposeVector.h \
		libs/slugs/lib/cudd-3.0.0/cudd/cuddInt.h \
		libs/slugs/lib/cudd-3.0.0/config.h \
		libs/slugs/lib/cudd-3.0.0/st/st.h \
		libs/slugs/lib/cudd-3.0.0/mtr/mtr.h \
		libs/slugs/lib/cudd-3.0.0/epd/epd.h \
		libs/BFAbstractionLibrary/BFCuddInlines.h \
		libs/BFAbstractionLibrary/BFCuddMintermEnumerator.h \
		libs/slugs/lib/cudd-3.0.0/dddmp/dddmp.h \
		libs/slugs/lib/cudd-3.0.0/util/util.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o BFCuddManager.o libs/BFAbstractionLibrary/BFCuddManager.cpp

bddDump.o: libs/BFAbstractionLibrary/bddDump.cpp libs/BFAbstractionLibrary/BF.h \
		libs/BFAbstractionLibrary/BFCuddManager.h \
		libs/slugs/lib/cudd-3.0.0/cudd/cudd.h \
		libs/BFAbstractionLibrary/BFCudd.h \
		libs/BFAbstractionLibrary/BFCuddVarCube.h \
		libs/BFAbstractionLibrary/BFCuddVarVector.h \
		libs/BFAbstractionLibrary/BFCuddComposeVector.h \
		libs/slugs/lib/cudd-3.0.0/cudd/cuddInt.h \
		libs/slugs/lib/cudd-3.0.0/config.h \
		libs/slugs/lib/cudd-3.0.0/st/st.h \
		libs/slugs/lib/cudd-3.0.0/mtr/mtr.h \
		libs/slugs/lib/cudd-3.0.0/epd/epd.h \
		libs/BFAbstractionLibrary/BFCuddInlines.h \
		libs/BFAbstractionLibrary/BFCuddMintermEnumerator.h \
		libs/BFAbstractionLibrary/bddDump.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o bddDump.o libs/BFAbstractionLibrary/bddDump.cpp

picosat.o: libs/picosat-965/picosat.c libs/picosat-965/picosat.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o picosat.o libs/picosat-965/picosat.c

lglib.o: libs/lingeling-bbc-9230380-160707/lglib.c libs/lingeling-bbc-9230380-160707/lglib.h \
		libs/lingeling-bbc-9230380-160707/lglconst.h \
		libs/lingeling-bbc-9230380-160707/lglopts.h \
		libs/lingeling-bbc-9230380-160707/lgloptl.h
	$(CC) -c $(CFLAGS) -Wno-sign-compare -Wno-unused-parameter $(INCPATH) -o lglib.o libs/lingeling-bbc-9230380-160707/lglib.c

lglopts.o: libs/lingeling-bbc-9230380-160707/lglopts.c libs/lingeling-bbc-9230380-160707/lglopts.h \
		libs/lingeling-bbc-9230380-160707/lglconst.h \
		libs/lingeling-bbc-9230380-160707/lgloptl.h
	$(CC) -c $(CFLAGS) -Wno-sign-compare -Wno-unused-parameter $(INCPATH) -o lglopts.o libs/lingeling-bbc-9230380-160707/lglopts.c

mainProject.o: mainProject.cc libs/BFAbstractionLibrary/BF.h \
		libs/BFAbstractionLibrary/BFCuddManager.h \
		libs/slugs/lib/cudd-3.0.0/cudd/cudd.h \
		libs/BFAbstractionLibrary/BFCudd.h \
		libs/BFAbstractionLibrary/BFCuddVarCube.h \
		libs/BFAbstractionLibrary/BFCuddVarVector.h \
		libs/BFAbstractionLibrary/BFCuddComposeVector.h \
		libs/slugs/lib/cudd-3.0.0/cudd/cuddInt.h \
		libs/slugs/lib/cudd-3.0.0/config.h \
		libs/slugs/lib/cudd-3.0.0/st/st.h \
		libs/slugs/lib/cudd-3.0.0/mtr/mtr.h \
		libs/slugs/lib/cudd-3.0.0/epd/epd.h \
		libs/BFAbstractionLibrary/BFCuddInlines.h \
		libs/BFAbstractionLibrary/BFCuddMintermEnumerator.h \
		libs/BFAbstractionLibrary/bddDump.h \
		libs/slugs/lib/cudd-3.0.0/dddmp/dddmp.h \
		libs/slugs/lib/cudd-3.0.0/util/util.h \
		libs/slugs/lib/cudd-3.0.0/cplusplus/cuddObj.hh \
		include/satSolvers.hpp \
		libs/picosat-965/picosat.h \
		libs/lingeling-bbc-9230380-160707/lglib.h \
		include/SCOTSAbstraction.hh \
		include/dynamics.hh \
		libs/SCOTSv0.2/src/scots.hh \
		libs/SCOTSv0.2/src/TransitionFunction.hh \
		libs/SCOTSv0.2/src/UniformGrid.hh \
		libs/SCOTSv0.2/src/Abstraction.hh \
		libs/SCOTSv0.2/src/GameSolver.hh \
		libs/SCOTSv0.2/src/WinningDomain.hh \
		libs/SCOTSv0.2/src/StaticController.hh \
		libs/SCOTSv0.2/src/InputOutput.hh \
		libs/SCOTSv0.2/src/FileHandler.hh \
		libs/SCOTSv0.2/src/SymbolicSet.hh \
		libs/SCOTSv0.2/src/IntegerInterval.hh \
		libs/SCOTSv0.2/src/SymbolicModel.hh \
		libs/SCOTSv0.2/src/EnfPre.hh \
		libs/SCOTSv0.2/utils/RungeKutta4.hh \
		include/vehicleDynamics.hh \
		include/tools.hh \
		include/vehicleDynamicsSimple.hh \
		include/vehicleDynamicsSingleSpeed.hh \
		include/workspaceProperties.hh \
		include/moonLandingDynamics.hh \
		include/moonLanding1DDynamics.hh \
		include/smartAbstractionProcessor.hh \
		include/bareAbstractionProcessor.hh \
		include/hybridAbstractionProcessor.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

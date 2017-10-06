#include <array>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <map>
#include <list>
#include <functional>
#include <algorithm>
#include <cstring>
#include <bitset>
#include <boost/functional/hash.hpp>

#include "BF.h"
#include "bddDump.h"
#include "dddmp.h"
#undef fail

#include "cuddObj.hh"
#include "satSolvers.hpp"
#include "SCOTSAbstraction.hh"
#include "vehicleDynamics.hh"
#include "vehicleDynamicsSimple.hh"
#include "vehicleDynamicsSingleSpeed.hh"
#include "moonLandingDynamics.hh"
#include "moonLanding1DDynamics.hh"
#include "smartAbstractionProcessor.hh"
#include "dumbAbstractionProcessor.hh"
#include "workspaceProperties.hh"
#include "hybridAbstractionProcessor.hh"


template<typename DynamicsClass,class AbstractionProcessorClass> void runConfiguration(std::string parameterFileName) {

    //Load dynamics
    WorkspaceProperties<DynamicsClass> workspaceProperties(parameterFileName.c_str());
    DynamicsClass dynamics;

    //Obtain SCOTS abstraction
    SCOTSAbstraction<DynamicsClass> scotsAbstraction(dynamics,workspaceProperties);

    //Defining the abstraction processor
    AbstractionProcessorClass abstractionProcessor(scotsAbstraction, workspaceProperties);
    abstractionProcessor.computeAbstraction();


}




int main(int argc, char **args) {

    try {
        std::string configurationFile = "";
        std::set<std::string> options;
        for (unsigned int i=1;i<argc;i++) {
            std::string current = args[i];
            if (current.substr(0,1)=="-") {
                // Option
                options.insert(current);
            } else {
                if (configurationFile!="") throw "Error: Multiple file names given.";
                configurationFile = current;
            }
        }

        // Join options
        std::string allOptions = " ";
        for (auto it : options) allOptions = allOptions + it + " ";

        if (allOptions==" --dumb --moon ") {
            runConfiguration<MoonLandingDynamics,DumbAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>>>(configurationFile);
        } else if (allOptions==" --dumb --simple ") {
            runConfiguration<VehicleDynamicsSimple,DumbAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>>>(configurationFile);
        } else if (allOptions==" --dumb --vehicle ") {
            runConfiguration<VehicleDynamics,DumbAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>>>(configurationFile);
        } else if (allOptions==" --moon --steady ") {
            runConfiguration<MoonLandingDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,PicosatSolver,false,false>>(configurationFile);
        } else if (allOptions==" --moon --removeDominant --steady ") {
            runConfiguration<MoonLandingDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,PicosatSolver,false,true>>(configurationFile);
        } else if (allOptions==" --lingeling --moon --steady ") {
            runConfiguration<MoonLandingDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,LingelingSolver,false,false>>(configurationFile);
        } else if (allOptions==" --lingeling --moon --removeDominant --steady ") {
            runConfiguration<MoonLandingDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,LingelingSolver,false,true>>(configurationFile);
        } else if (allOptions==" --moon --restrictSpreading --steady ") {
            runConfiguration<MoonLandingDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,PicosatSolver,true,false>>(configurationFile);
        } else if (allOptions==" --moon --removeDominant --restrictSpreading --steady ") {
            runConfiguration<MoonLandingDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,PicosatSolver,true,true>>(configurationFile);
        } else if (allOptions==" --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,PicosatSolver,false,false>>(configurationFile);
        } else if (allOptions==" --simple -removeDominant --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,PicosatSolver,false,true>>(configurationFile);
        } else if (allOptions==" --lingeling --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,LingelingSolver,false,false>>(configurationFile);
        } else if (allOptions==" --lingeling -removeDominant --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,LingelingSolver,false,true>>(configurationFile);
        } else if (allOptions==" --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,PicosatSolver,false,false>>(configurationFile);
        } else if (allOptions==" --removeDominant --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,PicosatSolver,false,true>>(configurationFile);
        } else if (allOptions==" --lingeling --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,LingelingSolver,false,false>>(configurationFile);
        } else if (allOptions==" --lingeling --removeDominant --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,LingelingSolver,false,true>>(configurationFile);
        } else if (allOptions==" --restrictSpreading --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,PicosatSolver,true,false>>(configurationFile);
        } else if (allOptions==" --removeDominant--restrictSpreading --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,PicosatSolver,true,true>>(configurationFile);
        } else if (allOptions==" --lingeling --restrictSpreading --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,LingelingSolver,true,false>>(configurationFile);
        } else if (allOptions==" --lingeling --removeDominant --restrictSpreading --simple --steady ") {
            runConfiguration<VehicleDynamicsSimple,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,LingelingSolver,true,true>>(configurationFile);
        } else if (allOptions==" --restrictSpreading --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,PicosatSolver,true,false>>(configurationFile);
        } else if (allOptions==" --removeDominant --restrictSpreading --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,PicosatSolver,true,true>>(configurationFile);
        } else if (allOptions==" --lingeling --restrictSpreading --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,LingelingSolver,true,false>>(configurationFile);
        } else if (allOptions==" --lingeling --removeDominant --restrictSpreading --steady --vehicle ") {
            runConfiguration<VehicleDynamics,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,LingelingSolver,true,true>>(configurationFile);
        } else if (allOptions==" --lingeling --removeDominant --restrictSpreading --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,LingelingSolver,true,true>>(configurationFile);
        } else if (allOptions==" --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,PicosatSolver,false,false>>(configurationFile);
        } else if (allOptions==" --removeDominant --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,PicosatSolver,false,true>>(configurationFile);
        } else if (allOptions==" --restrictSpreading --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,PicosatSolver,true,false>>(configurationFile);
        } else if (allOptions==" --removeDominant --restrictSpreading --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,PicosatSolver,true,true>>(configurationFile);
        } else if (allOptions==" --lingeling --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,LingelingSolver,false,false>>(configurationFile);
        } else if (allOptions==" --lingeling --removeDominant --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,LingelingSolver,false,true>>(configurationFile);
        } else if (allOptions==" --lingeling --restrictSpreading --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,LingelingSolver,true,false>>(configurationFile);
        } else if (allOptions==" --removeDominant --restrictSpreading --steady --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,SmartAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,LingelingSolver,true,true>>(configurationFile);
        } else if (allOptions==" --hybrid --moon ") {
            runConfiguration<MoonLandingDynamics,HybridAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,PicosatSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --lingeling --moon ") {
            runConfiguration<MoonLandingDynamics,HybridAbstractionProcessor<SCOTSAbstraction<MoonLandingDynamics>,WorkspaceProperties<MoonLandingDynamics>,LingelingSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --simple ") {
            runConfiguration<VehicleDynamicsSimple,HybridAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,PicosatSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --lingeling --simple ") {
            runConfiguration<VehicleDynamicsSimple,HybridAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSimple>,WorkspaceProperties<VehicleDynamicsSimple>,LingelingSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --vehicle ") {
            runConfiguration<VehicleDynamics,HybridAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,PicosatSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --lingeling --vehicle ") {
            runConfiguration<VehicleDynamics,HybridAbstractionProcessor<SCOTSAbstraction<VehicleDynamics>,WorkspaceProperties<VehicleDynamics>,LingelingSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,HybridAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,PicosatSolver>>(configurationFile);
        } else if (allOptions==" --hybrid --lingeling --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,HybridAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>,LingelingSolver>>(configurationFile);
        } else if (allOptions==" --dumb --moon1D ") {
            runConfiguration<MoonLanding1DDynamics,DumbAbstractionProcessor<SCOTSAbstraction<MoonLanding1DDynamics>,WorkspaceProperties<MoonLanding1DDynamics>>>(configurationFile);
        } else if (allOptions==" --dumb --vehicleSingleSpeed ") {
            runConfiguration<VehicleDynamicsSingleSpeed,DumbAbstractionProcessor<SCOTSAbstraction<VehicleDynamicsSingleSpeed>,WorkspaceProperties<VehicleDynamicsSingleSpeed>>>(configurationFile);
        } else if (allOptions==" --moon1D --steady ") {
            runConfiguration<MoonLanding1DDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLanding1DDynamics>,WorkspaceProperties<MoonLanding1DDynamics>,PicosatSolver,false,false>>(configurationFile);
        } else if (allOptions==" --moon1D --restrictSpreading --steady ") {
            runConfiguration<MoonLanding1DDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLanding1DDynamics>,WorkspaceProperties<MoonLanding1DDynamics>,PicosatSolver,true,false>>(configurationFile);
        } else if (allOptions==" --moon1D --removeDominant --steady ") {
            runConfiguration<MoonLanding1DDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLanding1DDynamics>,WorkspaceProperties<MoonLanding1DDynamics>,PicosatSolver,false,true>>(configurationFile);
        } else if (allOptions==" --moon1D --removeDominant --restrictSpreading --steady ") {
            runConfiguration<MoonLanding1DDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLanding1DDynamics>,WorkspaceProperties<MoonLanding1DDynamics>,PicosatSolver,true,true>>(configurationFile);
        } else if (allOptions==" --hybrid --moon1D ") {
            runConfiguration<MoonLanding1DDynamics,SmartAbstractionProcessor<SCOTSAbstraction<MoonLanding1DDynamics>,WorkspaceProperties<MoonLanding1DDynamics>,PicosatSolver,true,true>>(configurationFile);
        }

        else {
            std::cerr << "Error: Parameter combination '" <<allOptions << "' is not supported.\n";
            return 1;
        }



    } catch (const char *error) {
        std::cerr << "Exception: " << error << std::endl;
        return 1;
    } catch (const std::string error) {
        std::cerr << "Exception: " << error << std::endl;
        return 1;
    }
}

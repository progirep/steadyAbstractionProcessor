#ifndef __SCOTS_ABSTRACTION_HH__
#define __SCOTS_ABSTRACTION_HH__

#include "dynamics.hh"
#include <array>
#include "SCOTSAbstraction.hh"

#include "BF.h"
#include "bddDump.h"
#include "dddmp.h"
#undef fail

#include "cuddObj.hh"

#include "scots.hh"
#undef fail
#include "RungeKutta4.hh"

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <unordered_map>

#include "vehicleDynamics.hh"
#include "vehicleDynamicsSimple.hh"
#include "vehicleDynamicsSingleSpeed.hh"
#include "workspaceProperties.hh"




template<class _ConcreteDynamics> class SCOTSAbstraction {
public:
    typedef _ConcreteDynamics ConcreteDynamics;
    typedef typename ConcreteDynamics::discrete_state_type state_type;
    typedef typename ConcreteDynamics::discrete_input_type input_type;
    typedef typename ConcreteDynamics::discrete_state_type_hasher state_type_hasher;
    typedef typename ConcreteDynamics::discrete_input_type_hasher input_type_hasher;

    SCOTSAbstraction(ConcreteDynamics,WorkspaceProperties<ConcreteDynamics>&);
    void symbolicModelComputation(std::unordered_map<state_type, std::map<input_type,std::list<state_type>>, state_type_hasher> &,std::unordered_map<input_type,unsigned int,input_type_hasher> &);
    std::array<int,ConcreteDynamics::getStateNofDimensions()> &getNofGridPointsState();
    std::array<int,ConcreteDynamics::getInputNofDimensions()> &getNofGridPointsInput();
private:
    ConcreteDynamics systemDynamics;
    WorkspaceProperties<ConcreteDynamics> &workspaceProperties;
    std::array<int,ConcreteDynamics::getStateNofDimensions()> nofGridPointsState = {};
    std::array<int,ConcreteDynamics::getInputNofDimensions()> nofGridPointsInput = {};
};


template <class ConcreteDynamics> SCOTSAbstraction<ConcreteDynamics>::SCOTSAbstraction(ConcreteDynamics sDynamics, WorkspaceProperties<ConcreteDynamics> &_workspaceProperties) :
    systemDynamics(sDynamics), workspaceProperties(_workspaceProperties) {}


template <class ConcreteDynamics> void SCOTSAbstraction<ConcreteDynamics>::symbolicModelComputation(std::unordered_map<state_type, std::map<input_type,std::list<state_type>>, state_type_hasher> &transitionMap, std::unordered_map<input_type, unsigned int, input_type_hasher> &mapInputCombinationToInputActionValue) {

    const unsigned int nofStateDimensions = ConcreteDynamics::getStateNofDimensions();
    const unsigned int nofInputDimensions = ConcreteDynamics::getInputNofDimensions();

    const std::array<double,ConcreteDynamics::getStateNofDimensions()> stateWindowLowerBound = workspaceProperties.getTransitionEnumeratorWindowLowerBound();
    const std::array<double,ConcreteDynamics::getStateNofDimensions()> stateWindowUpperBound = workspaceProperties.getTransitionEnumeratorWindowUpperBound();
    const std::array<double,ConcreteDynamics::getStateNofDimensions()> stateETA = workspaceProperties.getStateETA();

    const std::array<double,ConcreteDynamics::getInputNofDimensions()> inputLowerBound = workspaceProperties.getInputLowerBound();
    const std::array<double,ConcreteDynamics::getInputNofDimensions()> inputUpperBound = workspaceProperties.getInputUpperBound();
    const std::array<double,ConcreteDynamics::getInputNofDimensions()> inputETA = workspaceProperties.getInputETA();
    const std::array<int,ConcreteDynamics::getStateNofDimensions()> splittingFactor = workspaceProperties.getSplittingFactors();
    const std::array<double,ConcreteDynamics::getStateNofDimensions()> wrapAroundValues = ConcreteDynamics::getDimensionWrapAroundLimits();
    const std::array<bool,ConcreteDynamics::getStateNofDimensions()> dimensionIsWrapAround = ConcreteDynamics::getDimensionIsWrapAround();

    assert(mapInputCombinationToInputActionValue.size()==0);
    assert(transitionMap.size()==0);

    /* construct SymbolicSet with the UniformGrid information for the state space and BDD variable IDs for the pre */
    Cudd mgr;
    scots::SymbolicSet ss_pre = scots::SymbolicSet(mgr, nofStateDimensions, stateWindowLowerBound,stateWindowUpperBound,stateETA);
    /* construct SymbolicSet with the UniformGrid information for the state space and BDD variable IDs for the post */
    scots::SymbolicSet ss_post = scots::SymbolicSet(mgr, nofStateDimensions, stateWindowLowerBound,stateWindowUpperBound,stateETA);

    //TODO: Sometimes the value I do calculate and the one from SCOTS are not equal...this could be a problem in the future?
    size_t const *nofGridPointsStateAuto = ss_pre.getNofGridPoints();
    for (int dim = 0; dim < nofStateDimensions; dim++) {
        /*nofGridPointsState[dim] = static_cast<int>((stateUpperBound[dim] - stateLowerBound[dim])/stateETA[dim]);
        if (nofGridPointsStateAuto[dim] != nofGridPointsState[dim]) {
            std::cerr << "Error at state grid points, auto is " << nofGridPointsStateAuto[dim] << " and I got " << nofGridPointsState[dim] << std::endl;
            throw 123;
        }*/
        nofGridPointsState[dim] = static_cast<int>(nofGridPointsStateAuto[dim]);
    }


    std::cout << "Uniform grid details:" << std::endl;

    ss_pre.print_info(1);

    scots::SymbolicSet ss_input = scots::SymbolicSet(mgr, nofInputDimensions,inputLowerBound,inputUpperBound,inputETA);

    ss_input.print_info(1);

    size_t const *nofGridPointsInputAuto = ss_input.getNofGridPoints();

    for (int dim = 0; dim < nofInputDimensions; dim++) {
        /*nofGridPointsInput[dim] = static_cast<int>((inputUpperBound[dim] - inputLowerBound[dim])/inputETA[dim]);
        if (nofGridPointsInputAuto[dim] != nofGridPointsInput[dim]) {
            std::cerr << "Error at input grid points, auto is " << nofGridPointsInputAuto[dim] << " and I got " << nofGridPointsInput[dim] << std::endl;
            throw 123;
        }*/
        nofGridPointsInput[dim] = static_cast<int>(nofGridPointsInputAuto[dim]);
    }


    scots::SymbolicModel<typename ConcreteDynamics::continuous_state_type, typename ConcreteDynamics::continuous_input_type> sym_model(ss_pre,ss_input,ss_post);

    scots::SymbolicSet symbolicSetTransitions = scots::SymbolicSet(scots::SymbolicSet(ss_pre,ss_input),ss_post);

    size_t nof_trans;

    auto postSystemDynamicsFunction = ConcreteDynamics::getSystemDynamicsPost(workspaceProperties.getSamplingTime());
    auto growthBoundFunction = ConcreteDynamics::getGrowthBoundFunction(workspaceProperties.getSamplingTime());
    BDD TF = sym_model.compute_gb(mgr,postSystemDynamicsFunction,growthBoundFunction,nof_trans);

    scots::write_to_file(mgr,symbolicSetTransitions,TF,"auxfiles/SCOTSwindowabstraction");

    //Ask to the SCOTS solver to get all transitions from those states such that changing the value of the transition-invariant dimensions do not change the reachable states
    std::vector<scots::abs_type> lowInterest;
    for (int i = 0; i < nofStateDimensions; i++) lowInterest.push_back(0);

    std::vector<scots::abs_type> upperInterest;
    for (int i = 0; i < nofStateDimensions; i++) upperInterest.push_back(nofGridPointsState[i]);

    std::array<bool,nofStateDimensions> dimensionIsTranslationInvariant = ConcreteDynamics::getDimensionIsTranslationInvariant();

    for (int i = 0; i < nofStateDimensions; i++) {
        if (dimensionIsTranslationInvariant[i]) {
            int cornerDimensionOfInterest = nofGridPointsState[i]/2;
            lowInterest[i] = static_cast<int>(cornerDimensionOfInterest);
            upperInterest[i] = static_cast<int>(lowInterest[i]+splittingFactor[i]-1);

        }
    }


    BDD interestingTransitions = TF & ss_pre.interval_to_bdd(mgr,lowInterest,upperInterest);
    std::vector<BDD> toBeDumped;
    toBeDumped.push_back( ss_pre.interval_to_bdd(mgr,lowInterest,upperInterest));

    //mgr.DumpDot(toBeDumped);

    //We ask SCOTS to provide a list of all the transitions available
    auto allTransitionsAsDouble = symbolicSetTransitions.bdd_to_grid_points(mgr,interestingTransitions);

    unsigned int inputIter = 0;

    //fraparo: Writting every transition in a file
#ifdef BUILD_SCOTS_ABSTRACTION_FILE
    std::ofstream myfile("SCOTSAbstraction.txt");
#endif

    constexpr int nofDimensionsPerTransition = nofStateDimensions * 2 + nofInputDimensions;
    for(unsigned int i=0;i<allTransitionsAsDouble.size();i+=nofDimensionsPerTransition) {

        //Store information in double format from SCOTS into vectors
        std::vector<double> stateOriginal(allTransitionsAsDouble.begin()+i,allTransitionsAsDouble.begin()+i+nofStateDimensions);
        std::vector<double> controlInput(allTransitionsAsDouble.begin()+i+nofStateDimensions,allTransitionsAsDouble.begin()+i+nofStateDimensions+nofInputDimensions);
        std::vector<double> stateDestiny(allTransitionsAsDouble.begin()+i+nofStateDimensions+nofInputDimensions,allTransitionsAsDouble.begin()+i+nofDimensionsPerTransition);

#ifndef NDEBUG
        // Output all transitions
        std::cerr << "Transition SCOTS Abstraction: (" << printDoubleVector(stateOriginal) << ") --[" << printDoubleVector(controlInput) << "] -> (" << printDoubleVector(stateDestiny) << ")\n";
#endif

        //Construct the types for the transition: origState --[cInput]--> destState
        state_type origState;
        input_type cInput;
        state_type destState;

        //Convert from double to int (the int represent the numerical value of the position of the value in an array with all the values)
        for (int j = 0; j < nofStateDimensions; j++) {
            //if (dimensionIsTranslationInvariant[j])
            //    origState[j] = (static_cast<int>(std::floor(((stateOriginal[j] - stateWindowLowerBound[j]) / stateETA[j]) + 0.00001)) - lowInterest[j]);
            //else
            if (dimensionIsWrapAround[j])
                origState[j] = (static_cast<int>(std::floor(((std::fmod(stateOriginal[j]+wrapAroundValues[j],wrapAroundValues[j]) - stateWindowLowerBound[j]) / stateETA[j]) + 0.00001)));
            else
                origState[j] = (static_cast<int>(std::floor(((stateOriginal[j] - stateWindowLowerBound[j]) / stateETA[j]) + 0.00001)));
        }

        for (int j = 0; j < nofInputDimensions; j++) {
            cInput[j] = (static_cast<int>(std::floor(((controlInput[j] - inputLowerBound[j]) / inputETA[j]) + 0.00001)));
        }

        for (int j = 0; j < nofStateDimensions; j++) {
            //if (dimensionIsTranslationInvariant[j])
            //    destState[j] = (static_cast<int>(std::floor(((stateDestiny[j] - stateWindowLowerBound[j]) / stateETA[j]) + 0.00001)) - lowInterest[j]);
            //else
            if (dimensionIsWrapAround[j])
                destState[j] = (static_cast<int>(std::floor(((std::fmod(stateDestiny[j]+wrapAroundValues[j],wrapAroundValues[j]) - stateWindowLowerBound[j]) / stateETA[j]) + 0.00001)));
            else
                destState[j] = (static_cast<int>(std::floor(((stateDestiny[j] - stateWindowLowerBound[j]) / stateETA[j]) + 0.00001)));
        }

#ifdef BUILD_SCOTS_ABSTRACTION_FILE
        myfile << "(" << origState[0];
        for (int dim = 1; dim < nofStateDimensions; dim++) {
            myfile << "," << origState[dim];
        }
        myfile << ")-->["<<cInput[0];
        for (int dimI = 1; dimI < nofInputDimensions; dimI++) {
            myfile << "," << cInput[dimI];
        }
        myfile << "]-->(" << destState[0];
        for (int dim = 1; dim < nofStateDimensions; dim++) {
            myfile << "," << destState[dim];
        }
        myfile << ")" << std::endl;
#endif

        transitionMap[origState][cInput].push_back(destState);
        //If there is not value asigned, assign an integer value to the input
        if (mapInputCombinationToInputActionValue.count(cInput)==0) {
            mapInputCombinationToInputActionValue[cInput] = inputIter;
            inputIter++;
        }
    }

#ifdef BUILD_SCOTS_ABSTRACTION_FILE
    if (myfile.fail()) throw "Error writing to SCOTSAbstraction.txt";
    myfile.close();
#endif

    // Check that every two states that only differ in translation-invariant dimensions,
    // we have the same number of actions available -- otherwise the window is too small.
    // -----> Is overly conservative at the moment!
    /*std::map<std::array<int,nofStateDimensions>,std::set<input_type> > inputsAvailableInBaseStates;
    for (auto it : transitionMap) {
        std::array<int,nofStateDimensions> baseState = it.first;
        for (unsigned int i=0;i<nofStateDimensions;i++) {
            if (dimensionIsTranslationInvariant[i]) baseState[i] = 0;
        }

        // Build set of available actions
        std::set<input_type> availableInputs;
        for (auto it2 : it.second) availableInputs.insert(it2.first);

        if (inputsAvailableInBaseStates.count(baseState)>0) {
            if (inputsAvailableInBaseStates[baseState]!=availableInputs) {
                throw "Error: Window size is too small.";
            }
        } else {
            inputsAvailableInBaseStates[baseState] = availableInputs;
        }
    }*/

}

template <class concreteDynamics> inline std::array<int,concreteDynamics::getStateNofDimensions()>& SCOTSAbstraction<concreteDynamics>::getNofGridPointsState() {
    return nofGridPointsState;
}

template <class concreteDynamics> inline std::array<int,concreteDynamics::getInputNofDimensions()>& SCOTSAbstraction<concreteDynamics>::getNofGridPointsInput() {
    return nofGridPointsInput;
}

#endif

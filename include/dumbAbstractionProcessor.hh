#ifndef __DUMB_ABSTRACTION_PROCESSOR_HH__
#define __DUMB_ABSTRACTION_PROCESSOR_HH__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "SCOTSAbstraction.hh"
#include "tools.hh"


template<class ConcreteSCOTSAbstraction, class ConcreteWorkspaceProperties> class DumbAbstractionProcessor {
private:
    ConcreteSCOTSAbstraction SCOTSAbstraction;
    ConcreteWorkspaceProperties workspaceProperties;

    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_state_type state_type;
    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_input_type input_type;
    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_state_type_hasher state_type_hasher;
    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_input_type_hasher input_type_hasher;

public:
    DumbAbstractionProcessor(ConcreteSCOTSAbstraction, ConcreteWorkspaceProperties);
    void convertNormalTransitionIntoSteadyTransitions();
    void computeAbstraction();
};

template<class ConcreteSCOTSAbstraction, class ConcreteWorkspaceProperties> DumbAbstractionProcessor<ConcreteSCOTSAbstraction,ConcreteWorkspaceProperties>::DumbAbstractionProcessor(ConcreteSCOTSAbstraction _SCOTSAbstraction, ConcreteWorkspaceProperties _workspaceProperties) :
    SCOTSAbstraction(_SCOTSAbstraction), workspaceProperties(_workspaceProperties) {}


template<class ConcreteSCOTSAbstraction, class ConcreteWorkspaceProperties> void DumbAbstractionProcessor<ConcreteSCOTSAbstraction,ConcreteWorkspaceProperties>::computeAbstraction() {

    //This map holds all transitions from the microcells that together form the macrocell "of interest"
    std::unordered_map<state_type, std::map<input_type,std::list<state_type>>, state_type_hasher> transitionMap;

    //This map holds for every possible combination of inputs, an integer number
    std::unordered_map<input_type,unsigned int,input_type_hasher> mapInputCombinationToInputActionValue;

    //First, call SCOTS to compute the "plain" abstraction
    SCOTSAbstraction.symbolicModelComputation(transitionMap,mapInputCombinationToInputActionValue);

    //Obtain all data from the SCOTS abstraction and workspace
    constexpr int nofDimensionsPerState = ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions();
    constexpr int nofDimensionsPerInput = ConcreteSCOTSAbstraction::ConcreteDynamics::getInputNofDimensions();
    std::array<bool,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> dimensionIsTranslationInvariant = ConcreteSCOTSAbstraction::ConcreteDynamics::getDimensionIsTranslationInvariant();


    // Sanity check
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateWindowLowerBound = workspaceProperties.getTransitionEnumeratorWindowLowerBound();
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateWindowUpperBound = workspaceProperties.getTransitionEnumeratorWindowUpperBound();
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateLowerBound = workspaceProperties.getStateLowerBound();
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateUpperBound = workspaceProperties.getStateUpperBound();

    for (int i = 0; i < nofDimensionsPerState; i++) {
        if (dimensionIsTranslationInvariant[i]) {
            // Nothing to do
        } else {
            if (stateWindowLowerBound[i]!=stateLowerBound[i]) throw "Error: Lower bound of non-translation-invariant dimensions must be the same as the window lower bound.";
            if (stateWindowUpperBound[i]!=stateUpperBound[i]) throw "Error: Upper bound of non-translation-invariant dimensions must be the same as the window upper bound.";
        }
    }

    //std::array<int,nofDimensionsPerState> nofGridPointsStateSCOTSAbstraction = SCOTSAbstraction.getNofGridPointsState();
    std::array<int,nofDimensionsPerInput> nofGridPointsInput = SCOTSAbstraction.getNofGridPointsInput();
    std::array<int,nofDimensionsPerState> nofGridPointsStateFineFinalAbstraction = workspaceProperties.getNofDiscreteStateCellsFineAbstraction();
    std::array<int,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> splittingFactor = workspaceProperties.getSplittingFactors();

    std::array<int,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> nofGridPointsStateCoarse;
    for (int dim = 0; dim < nofDimensionsPerState; dim++) {
        nofGridPointsStateCoarse[dim] = static_cast<int>(nofGridPointsStateFineFinalAbstraction[dim]/splittingFactor[dim]);
        if ((nofGridPointsStateFineFinalAbstraction[dim] % splittingFactor[dim])>0) {
            std::ostringstream errorMsg;
            errorMsg << "Illegal size of the workspace in dimension " << dim << " for the splitting factor of " << splittingFactor[dim] << " as the number of grid points in this dimension is " << nofGridPointsStateFineFinalAbstraction[dim];
            throw errorMsg.str();
        }
    }

    std::array<std::string,nofDimensionsPerState> stateComponentBFVariableNamePrefixes = ConcreteSCOTSAbstraction::ConcreteDynamics::getStateVariableNames();

    //Create the BDD and the variables needed to represent the transitions in the processed abstraction
    BFBddManager mgrBDD;
    std::vector<BF> bfVars;
    std::vector<std::string> bfVarNames;

    std::array<std::vector<BF>,nofDimensionsPerState> stateDimensionsEncodingBits;
    std::array<std::vector<BF>,nofDimensionsPerState> nextStateDimensionsEncodingBits;


    //For each dimension of the states, create the amount of variables needed to represent the values in binary
    for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
        std::vector<BF> binaryBitsMacrocellDimension;
        std::vector<BF> binaryBitsNextMacrocellDimension;

        unsigned int numberOfBitsMacrocell = ceil(log2(nofGridPointsStateCoarse[i]));

        for (unsigned int j = 0; j < numberOfBitsMacrocell; j++) {
            BF bddStateVariableValue = mgrBDD.newVariable();
            bfVars.push_back(bddStateVariableValue);
            std::ostringstream oss1;
            oss1 << stateComponentBFVariableNamePrefixes[i] << "@" << j;
            if (j==0) oss1 << ".0." << (nofGridPointsStateCoarse[i]-1);
            bfVarNames.push_back(oss1.str());
            binaryBitsMacrocellDimension.push_back(bddStateVariableValue);
            BF bddNextStateVariableValue = mgrBDD.newVariable();
            bfVars.push_back(bddNextStateVariableValue);
            bfVarNames.push_back(oss1.str()+"'");
            binaryBitsNextMacrocellDimension.push_back(bddNextStateVariableValue);
        }

        stateDimensionsEncodingBits[i] = binaryBitsMacrocellDimension;
        nextStateDimensionsEncodingBits[i] = binaryBitsNextMacrocellDimension;
    }

    // Allocate input vars
    std::vector<BF> inputComponentEncodingBits;
    unsigned int nofInputsCombinations = 1;
    for (unsigned int i = 0; i < nofDimensionsPerInput; i++) {
        nofInputsCombinations *= nofGridPointsInput[i];
    }

    unsigned int numberOfBitsInputCombination = ceil(log2(nofInputsCombinations));
    for (unsigned int i=0; i<numberOfBitsInputCombination; i++) {
        BF bddInputBitVariableValue = mgrBDD.newVariable();
        bfVars.push_back(bddInputBitVariableValue);
        std::ostringstream oss1;
        oss1 << "action@" << i;
        if (i==0) oss1 << ".0." << (nofInputsCombinations-1);
        oss1 << "'";
        bfVarNames.push_back(oss1.str());
        inputComponentEncodingBits.push_back(bddInputBitVariableValue);
    }



    //Ask SCOTS to get all transitions from those states such that changing the value of the transition-invariant dimensions do not change the reachable states
    //std::vector<scots::abs_type> lowInterest;
    //for (int i = 0; i < nofDimensionsPerState; i++) lowInterest.push_back(0);

    //std::vector<scots::abs_type> upperInterest;
    //for (int i = 0; i < nofDimensionsPerState; i++) upperInterest.push_back(nofGridPointsStateFineFinalAbstraction[i]);

    //std::map<std::array<int,nofDimensionsPerState>, int> macrocellsChecked;

    BF allTransitions = mgrBDD.constantFalse();

    //For every transition in the map, we will "copy & paste" every transition for the corresponding invariant dimensions of the states

    // Writing every transition to a file for debugging
#ifdef WRITE_DUMB_ABSTRACTION_FILE
    std::ofstream myfile("DumbAbstraction.txt");
#endif

    std::map<std::vector<int>, int> macrocellsChecked;

    //For every transition in the map, we will "copy & paste" every transition for the corresponding invariant dimensions of the states
    for (typename std::unordered_map<state_type, std::map<input_type,std::list<state_type>>,state_type_hasher>::const_iterator transitionElement = transitionMap.begin(); transitionElement != transitionMap.end(); transitionElement++) {

        //Create initial macrocell
        state_type initialState = transitionElement->first;

        //Check if the macrocell which the microcell form part of already has not been worked with, otherwise, continue to next microcell.
        std::vector<int> startingMacrocell;
        for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
            startingMacrocell.push_back((int)floor(initialState[i]/splittingFactor[i]));
        }
        if (macrocellsChecked.count(startingMacrocell) == 0) {
            macrocellsChecked[startingMacrocell] = 1;
        } else {
            continue;
        }

        BF startingMacrocellEncoding = mgrBDD.constantTrue();

        for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
            startingMacrocellEncoding &= encodeInBFBits(mgrBDD,startingMacrocell[i],stateDimensionsEncodingBits[i]);
        }

        std::list<std::array<int,nofDimensionsPerState>> microcellsOfStartingMacrocell;

        //Recursive function to find the microcells that form the macrocell
        //std::cerr << "The microcell checked is " << printArrayInt(initialState) << std::endl;
        std::function<void(std::list<std::array<int,nofDimensionsPerState> > &, int, std::array<int,nofDimensionsPerState> &)> enumeratorFunction = [&](std::list<std::array<int,nofDimensionsPerState> > &destinationList, int currentDimension, std::array<int,nofDimensionsPerState> &currentTuple) {
            for (uint i = 0; i < splittingFactor[currentDimension]; i++) {
                currentTuple[currentDimension] = startingMacrocell[currentDimension]*splittingFactor[currentDimension]+i;
                if (currentDimension == 0) {
                    destinationList.push_back(currentTuple);
                } else {
                    enumeratorFunction(destinationList,currentDimension-1,currentTuple);
                }
            }
        };

        {
            std::array<int,nofDimensionsPerState> mid;
            enumeratorFunction(microcellsOfStartingMacrocell,nofDimensionsPerState-1,mid);
        }

        std::set<input_type> unavailableInputs;

        for (auto startingMicrocell : microcellsOfStartingMacrocell) {

            //Create initial macrocell
            state_type initialState = startingMicrocell;

            //Check if the macrocell which the microcell form part of already has not been worked with, otherwise, continue to next microcell.
            std::array<int,nofDimensionsPerState> startingMacrocell;
            for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
                startingMacrocell[i] = static_cast<int>(floor(initialState[i]/splittingFactor[i]));
            }

            /*if (macrocellsChecked.count(startingMacrocell) == 0) {
                macrocellsChecked[startingMacrocell] = 1;
            } else {
                continue;
            }*/

            //For every possible action
            for (auto cInput : mapInputCombinationToInputActionValue) {

                BF controlInputEncoded = mgrBDD.constantTrue();
                controlInputEncoded &= encodeInBFBits(mgrBDD,cInput.second,inputComponentEncodingBits);
                const input_type &inp = cInput.first;

                const std::map<input_type,std::list<state_type>> &avaiableTransitions = transitionMap[startingMicrocell];

                //For every possible microcell which conform the initial macrocell

                auto it = avaiableTransitions.find(inp);

                if (it==avaiableTransitions.end()) {
                    unavailableInputs.insert(inp);
                } else {

                    //std::cerr << "Found successor state set.\n";

                    for (auto nextMicrocell : it->second) {

                        /*std::cerr << "Processing edge to ";
                        std::cerr << "(" << nextMicrocell[0];
                        for (int dim = 1; dim < nofDimensionsPerState; dim++) {
                            std::cerr << "," << nextMicrocell[dim];
                        }
                        std::cerr << ")\n";*/

                        BF nextMacrocell = mgrBDD.constantTrue();

    #ifdef WRITE_DUMB_ABSTRACTION_FILE
                        myfile << "(" << startingMacrocell[0];
                        for (int dim = 1; dim < nofDimensionsPerState; dim++) {
                            myfile << "," << startingMacrocell[dim];
                        }
                        myfile << ")-->["<<inp[0];
                        for (int dimI = 1; dimI < nofDimensionsPerInput; dimI++) {
                            myfile << "," << inp[dimI];
                        }
                        myfile << "]-->(" << static_cast<int>(nextMicrocell[0]/splittingFactor[0]);
    #endif

                        for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
    #ifdef WRITE_DUMB_ABSTRACTION_FILE
                            if (i !=0) myfile << "," << static_cast<int>(nextMicrocell[i]/splittingFactor[i]);
    #endif
                            int dimValue = static_cast<int>(nextMicrocell[i]/splittingFactor[i]);
                            nextMacrocell &= encodeInBFBits(mgrBDD,dimValue,nextStateDimensionsEncodingBits[i]);
                        }

    #ifdef WRITE_DUMB_ABSTRACTION_FILE
                        myfile << ")" << std::endl;
    #endif
                        allTransitions |= startingMacrocellEncoding & controlInputEncoded & nextMacrocell;
                    }
                }
            }
        }

        // Get rid of the transitions that are unavailable in some microcells
        for (auto it : unavailableInputs) {
            //std::cerr << "Unavailable transition for action: " << mapInputCombinationToInputActionValue[it] << std::endl;
            allTransitions &= ((!startingMacrocellEncoding) | (!encodeInBFBits(mgrBDD,mapInputCombinationToInputActionValue[it],inputComponentEncodingBits)));
        }

    }

#ifdef WRITE_DUMB_ABSTRACTION_FILE
    if (myfile.fail()) throw "Error writing to DumbAbstraction file.";
    myfile.close();
#endif

    BF_newDumpDot(TrivialVariableInfoContainer(mgrBDD,bfVarNames,bfVars),allTransitions,NULL,"/tmp/beforeReplication.dot");

    //Obtain T' from T (all transitions in all cells from all transitions from one cell)
    BFVarCube varCubeNextState;
    {
        std::vector<BF> allNextStateVars;
        for (auto it : nextStateDimensionsEncodingBits) {
            allNextStateVars.insert(allNextStateVars.end(),it.begin(),it.end());
        }
        varCubeNextState = mgrBDD.computeCube(allNextStateVars);
    }


    for (int dim = 0; dim < nofDimensionsPerState; dim++) {
        if (dimensionIsTranslationInvariant[dim]) {
            BF allTransitionsTranslated = mgrBDD.constantFalse();
            BF disallowedActions = mgrBDD.constantFalse();
            BFVarCube dimensionInState = mgrBDD.computeCube(stateDimensionsEncodingBits[dim]);
            BFVarCube dimensionInNextState = mgrBDD.computeCube(nextStateDimensionsEncodingBits[dim]);

            std::cerr << "Translation invariant dimension: " << dim << std::endl;
            //Because we start from the "most middle value of the dimension", the bigger difference between that value and the boundaries is half of the total amount of different values of the dimension
            for (unsigned int start=0;start<nofGridPointsStateCoarse[dim];start++) {
                BF startingTransitions = allTransitions & encodeInBFBits(mgrBDD,start,stateDimensionsEncodingBits[dim]);
                startingTransitions = startingTransitions.ExistAbstract(dimensionInState);

                for (unsigned int end=0;end<nofGridPointsStateCoarse[dim];end++) {
                    BF finalTransitions = startingTransitions & encodeInBFBits(mgrBDD,end,nextStateDimensionsEncodingBits[dim]);
                    finalTransitions = finalTransitions.ExistAbstract(dimensionInNextState);

                    for (unsigned int shift=0;shift<nofGridPointsStateCoarse[dim];shift++) {

                        if (((end-start+shift)>=0) && (end-start+shift<nofGridPointsStateCoarse[dim])) {
                            if (!(finalTransitions.isFalse()))
                                std::cerr << "values Start End Shift: " << start << ";" << end << ";" << shift << std::endl;
                            allTransitionsTranslated |= finalTransitions & encodeInBFBits(mgrBDD,shift,stateDimensionsEncodingBits[dim]) & encodeInBFBits(mgrBDD,end-start+shift, nextStateDimensionsEncodingBits[dim]);
                        } else {
                            disallowedActions |= finalTransitions.ExistAbstract(varCubeNextState) & encodeInBFBits(mgrBDD,shift,stateDimensionsEncodingBits[dim]);
                        }
                    }
                }
            }
            allTransitions = allTransitionsTranslated & !disallowedActions;
        }
    }

    // Write BDD to file
    mgrBDD.writeBDDToFile(workspaceProperties.getStringConfigurationValue("outFileName").c_str(),"",allTransitions,bfVars,bfVarNames);

    BF_newDumpDot(TrivialVariableInfoContainer(mgrBDD,bfVarNames,bfVars),allTransitions,NULL,"/tmp/afterReplication.dot");

}

#endif

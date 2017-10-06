#ifndef __HYBRID_ABSTRACTION_PROCESSOR_HH__
#define __HYBRID_ABSTRACTION_PROCESSOR_HH__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include "SCOTSAbstraction.hh"
#include "tools.hh"


template<class ConcreteSCOTSAbstraction, class ConcreteWorkspaceProperties, class ConcreteSATSolver> class HybridAbstractionProcessor {
private:
    ConcreteSCOTSAbstraction SCOTSAbstraction;
    ConcreteWorkspaceProperties workspaceProperties;

    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_state_type state_type;
    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_input_type input_type;
    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_state_type_hasher state_type_hasher;
    typedef typename ConcreteSCOTSAbstraction::ConcreteDynamics::discrete_input_type_hasher input_type_hasher;

public:
    HybridAbstractionProcessor(ConcreteSCOTSAbstraction, ConcreteWorkspaceProperties);
    void convertNormalTransitionIntoSteadyTransitions();
    void computeAbstraction();
};

template<class ConcreteSCOTSAbstraction, class ConcreteWorkspaceProperties, class ConcreteSATSolver> HybridAbstractionProcessor<ConcreteSCOTSAbstraction,ConcreteWorkspaceProperties,ConcreteSATSolver>::HybridAbstractionProcessor(ConcreteSCOTSAbstraction _SCOTSAbstraction, ConcreteWorkspaceProperties _workspaceProperties) :
    SCOTSAbstraction(_SCOTSAbstraction), workspaceProperties(_workspaceProperties)
{}


template<class ConcreteSCOTSAbstraction, class ConcreteWorkspaceProperties, class ConcreteSATSolver> void HybridAbstractionProcessor<ConcreteSCOTSAbstraction,ConcreteWorkspaceProperties,ConcreteSATSolver>::computeAbstraction() {

    //This map holds all transitions from the microcells that together form the macrocell "of interest"
    std::unordered_map<state_type, std::map<input_type,std::list<state_type>>, state_type_hasher> transitionMap;

    //This map holds for every possible combination of inputs, an integer number
    std::unordered_map<input_type,unsigned int,input_type_hasher> mapInputCombinationToInputActionValue;

    //First, call SCOTS to compute the "plain" abstraction
    SCOTSAbstraction.symbolicModelComputation(transitionMap,mapInputCombinationToInputActionValue);

    //Obtain all data from the SCOTS abstraction and workspace
    constexpr int nofDimensionsPerState = ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions();
    constexpr int nofDimensionsPerInput = ConcreteSCOTSAbstraction::ConcreteDynamics::getInputNofDimensions();
    (void) nofDimensionsPerInput; // Used in debug messages only, to avoid warnings

    std::array<bool,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> dimensionIsTranslationInvariant = ConcreteSCOTSAbstraction::ConcreteDynamics::getDimensionIsTranslationInvariant();

    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateWindowLowerBound = workspaceProperties.getTransitionEnumeratorWindowLowerBound();
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateWindowUpperBound = workspaceProperties.getTransitionEnumeratorWindowUpperBound();
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateLowerBound = workspaceProperties.getStateLowerBound();
    const std::array<double,ConcreteSCOTSAbstraction::ConcreteDynamics::getStateNofDimensions()> stateUpperBound = workspaceProperties.getStateUpperBound();

    // Sanity check
    for (int i = 0; i < nofDimensionsPerState; i++) {
        if (dimensionIsTranslationInvariant[i]) {
            // Nothing to do
        } else {
            if (stateWindowLowerBound[i]!=stateLowerBound[i]) throw "Error: Lower bound of non-translation-invariant dimensions must be the same as the window lower bound.";
            if (stateWindowUpperBound[i]!=stateUpperBound[i]) throw "Error: Upper bound of non-translation-invariant dimensions must be the same as the window upper bound.";
        }
    }

    //std::array<int,nofDimensionsPerState> nofGridPointsStateSCOTSAbstraction = SCOTSAbstraction.getNofGridPointsState();
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
    std::vector<BF> inputComponentEncodingBits;

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

    BF allTransitions = mgrBDD.constantFalse();

    //For every transition in the map, we will "copy & paste" every transition for the corresponding invariant dimensions of the states

    // Writing every transition to a file for debugging
#ifdef WRITE_SMART_ABSTRACTION_FILE
    std::ofstream myfile("SmartAbstraction.txt");
#endif

    std::map<std::vector<int>, int> macrocellsChecked;
    unsigned int maxActionNumber = 0;

    // Iterate over the starting macrocells
    for (typename std::unordered_map<state_type, std::map<input_type,std::list<state_type>>,state_type_hasher>::const_iterator transitionElement = transitionMap.begin(); transitionElement != transitionMap.end(); transitionElement++) {

        //Create initial macrocell
        const state_type &initialState = transitionElement->first;

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

        std::cerr << "Processing macrocell of microcell: " << intArrayPrinter<nofDimensionsPerState>(initialState) << std::endl;

        BF startingMacrocellEncoding = mgrBDD.constantTrue();

        for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
            startingMacrocellEncoding &= encodeInBFBits(mgrBDD,startingMacrocell[i],stateDimensionsEncodingBits[i]);
        }

        std::list<std::array<int,nofDimensionsPerState>> microcellsOfStartingMacrocell;

        //Recursive function to find the microcells that form the macrocell
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

        // Get rid of dominated actions
        for (auto const &startingMicrocell : microcellsOfStartingMacrocell) {
            transitionMap[startingMicrocell];
            //std::cerr << "Number of available inputs for starting cell " << intArrayPrinter<nofDimensionsPerState>(startingMicrocell) << ": " << transitionMap[startingMicrocell].size() << std::endl;
            std::set<input_type> dominatedInputs;
            for (auto const &cInputMap1 : transitionMap.at(startingMicrocell)) {
                for (auto const &cInputMap2 : transitionMap.at(startingMicrocell)) {
                    if (dominatedInputs.count(cInputMap2.first)==0) {
                        if (std::includes(cInputMap1.second.begin(),cInputMap1.second.end(),cInputMap2.second.begin(),cInputMap2.second.end())) {
                            if (cInputMap1.first!=cInputMap2.first) {
                                // std::cerr << "(b " << cInputMap1.second.size() << "." << cInputMap2.second.size() << ")";
                                dominatedInputs.insert(cInputMap1.first);
                            }
                        }
                    }
                }
                for (auto it : dominatedInputs) {
                    transitionMap.at(startingMicrocell).erase(it);
                }
                std::cerr << "(Removed dominated inputs: " << dominatedInputs.size() << " with " << transitionMap.at(startingMicrocell).size() << " remaining)" << std::endl;
            }
        }


        // Enumerate combinations of successor states
        std::unordered_map<input_type, std::set<state_type >,input_type_hasher> reachableCombinations;

        for (auto const &startingMicrocell : microcellsOfStartingMacrocell) {
            for (auto const &cInputMap1 : transitionMap.at(startingMicrocell)) {
                for (auto it : cInputMap1.second) {
                    state_type macrocellState;
                    for (int dim = 0; dim < nofDimensionsPerState; dim++) {
                        macrocellState[dim] = static_cast<int>(it[dim] / splittingFactor[dim]);
                    }
                    reachableCombinations[cInputMap1.first].insert(macrocellState);
                    //std::cerr << "Reachable via " << intArrayPrinter<nofDimensionsPerInput>(cInputMap1.first) << ":" << intArrayPrinter<nofDimensionsPerState>(macrocellState) << "\n";
                }
            }
        }

        //Use a SAT solver to find those transitions such that produce the least amount of macrocells reached
        ConcreteSATSolver s;
        std::map<state_type,int> successorMacrostateToSATVarMapper;
        std::map<std::pair<state_type,input_type>,int> actionSelectionSATVarsForEachStartingMicrocell;

        int firstFreshVariable = 0;

        //Adding clauses
        //This clauses represent that at least one valid action for the starting macrocell's microcells is taken
        for (auto const &microcellState : microcellsOfStartingMacrocell) {
            try {
                for (auto const &cInputMap : transitionMap.at(microcellState)) {
                    input_type cInput = cInputMap.first;
                    std::pair<state_type, input_type> mapKey (microcellState,cInput);
                    actionSelectionSATVarsForEachStartingMicrocell[mapKey] = ++firstFreshVariable;
                    s.markVariableAsIncremental(firstFreshVariable);
                    s.add(firstFreshVariable);
                }
            } catch (const std::out_of_range& oor) {
                // No suitable elements in the map.
            }
            s.add(0);
        }

        //These clauses represent which macrocells are reached for every action taken in every starting macrocell's microcells
        for (auto const &microcellState: microcellsOfStartingMacrocell) {
            try {
                for (auto const &cInputMap : transitionMap.at(microcellState)) {
                    input_type cInput = cInputMap.first;
                    std::pair<state_type, input_type> mapKey (microcellState,cInput);
                    assert(actionSelectionSATVarsForEachStartingMicrocell.count(mapKey)>0);
                    int actionLiteral = -1*actionSelectionSATVarsForEachStartingMicrocell[mapKey];
                    for (state_type nextState : cInputMap.second) {
                        state_type macrocellState;
                        for (int dim = 0; dim < nofDimensionsPerState; dim++) {
                            macrocellState[dim] = static_cast<int>(nextState[dim] / splittingFactor[dim]);
                        }
                        s.add(actionLiteral);
                        auto it = successorMacrostateToSATVarMapper.find(macrocellState);
                        int successorMacrocellVar;
                        if (it==successorMacrostateToSATVarMapper.end()) {
                            successorMacrostateToSATVarMapper[macrocellState] = ++firstFreshVariable;
                            successorMacrocellVar = firstFreshVariable;
                            s.markVariableAsIncremental(firstFreshVariable);
                        } else {
                            successorMacrocellVar = it->second;
                        }
                        s.add(successorMacrocellVar);
                        s.add(0);
                    }
                }
            } catch (const std::out_of_range& oor) {
                // No suitable elements in the map.
            }
        }

        // Require that the found states are <= some non-steady action that we could also have taken
        {
            std::vector<int> oneOfThem;
            for (auto it : reachableCombinations) {
                int thisSelector = ++firstFreshVariable;
                //std::cerr << "Selector: " << thisSelector << " for input " << intArrayPrinter<nofDimensionsPerInput>(it.first) << std::endl;
                oneOfThem.push_back(thisSelector);
                for (auto &it2 : successorMacrostateToSATVarMapper) {
                    if (it.second.count(it2.first)==0) {
                        //std::cerr << "Not allowing: " << intArrayPrinter<nofDimensionsPerState>(it2.first) << std::endl;
                        s.add(-1*thisSelector);
                        s.add(-1*it2.second);
                        s.add(0);
                    } else {
                        //std::cerr << "Allowing: " << intArrayPrinter<nofDimensionsPerState>(it2.first) << std::endl;
                    }
                }
            }
            for (auto it : oneOfThem) {
                s.add(it);
            }
            s.add(0);
        }

        //Use a SAT solver to minimize the set of possible macrocell reached for every action
        std::list<std::map<state_type,input_type>> steadyActionsComputed;


        //This loop tries to reduce the number of macrocells reached
        while (s.solve()) {

            std::map<state_type,input_type> bestSelection; /* Maps from the microcell to the chosen action */

            //This loop tries to reduce the number of macrocells reached
            while (s.solve()) {
                std::vector<int> assumptions;

                //Add new constraint which minimize the number of macrocells reached (while the SAT instance can be solved)
                std::vector<int> oneLessClause;

                for (auto const &macroCell : successorMacrostateToSATVarMapper) {
                    int macroVar = macroCell.second;
                    if (s.getVariableValue(macroVar)) {
                        assert(macroVar!=0);
                        oneLessClause.push_back(-1*macroVar);
                    } else {
                        assumptions.push_back(-1*macroVar);
                    }
                }

                // Record selection of action
                bestSelection.clear();
                for (auto it : actionSelectionSATVarsForEachStartingMicrocell) {
                    if (s.getVariableValue(it.second)) {
                        bestSelection[it.first.first] = it.first.second;
                    }
                }

                // Feed the "one cless clause" back to the solver.
                for (auto it : oneLessClause) s.add(it);
                s.add(0);

                for (auto it : assumptions) s.assume(it);
            }


            //Once we got a model with a transition which minimize the number of macrocells reached, we construct the smart BDD
            assert(bestSelection.size()>0);
            if (bestSelection.size() > 0) {

                //List with all macrocells than can be reached from the starting macrocell with a particular action
                std::set<state_type> reachedStatesForOneMacrocell;

                for (auto const &bestSelectionElement : bestSelection) {
                    for (auto successorState : transitionMap[bestSelectionElement.first][bestSelectionElement.second]) {
                        state_type macrocellState;
                        for (int dim = 0; dim < nofDimensionsPerState; dim++) {
                            macrocellState[dim] = static_cast<int>(successorState[dim] / splittingFactor[dim]);
                        }
                        reachedStatesForOneMacrocell.insert(macrocellState);
                    }
                }

                steadyActionsComputed.push_back(std::map<state_type,input_type>(bestSelection));

                {
                    for (auto const &macrocellState : reachedStatesForOneMacrocell) {
                        assert(successorMacrostateToSATVarMapper.count(macrocellState)>0);
                        int literal = successorMacrostateToSATVarMapper[macrocellState]*-1;
                        //If the literal is not included yet in the clause, include it
                        s.add(literal);
                    }
                    s.add(0);
                }

                if ((steadyActionsComputed.size() % 100)==0) {
                    std::cerr << "*";
                }

            }
        }


        // Write selected transitions
        unsigned int actionNumber = 0;

        // Iterate over all actions and encode them.
        for (auto it : steadyActionsComputed) {

            std::map<state_type,input_type> const &chosenActions = it;

            // Make sure that there are enough input bits
            unsigned int inputBitsNeeded = ceil(log2(actionNumber+1));
            while (inputBitsNeeded > inputComponentEncodingBits.size()) {
                BF bddInputBitVariableValue = mgrBDD.newVariable();
                bfVars.push_back(bddInputBitVariableValue);
                std::ostringstream oss1;
                oss1 << "action@" << inputComponentEncodingBits.size() << "'";
                bfVarNames.push_back(oss1.str());
                inputComponentEncodingBits.push_back(bddInputBitVariableValue);
                allTransitions &= !bddInputBitVariableValue;
            }

            BF inputValueEncoding = encodeInBFBits(mgrBDD,actionNumber,inputComponentEncodingBits);

            // Encode successors
            BF successorCells = mgrBDD.constantFalse();
            for (auto it2 : chosenActions) {
                const state_type &startingMicrocell = it2.first;
                for (state_type &successor : transitionMap[startingMicrocell][it2.second]) {
                    BF successorCell = mgrBDD.constantTrue();
                    for (unsigned int i = 0; i < nofDimensionsPerState; i++) {
                        int dimValue = static_cast<int>(successor[i]/splittingFactor[i]);
                        successorCell &= encodeInBFBits(mgrBDD,dimValue,nextStateDimensionsEncodingBits[i]);
                    }
                    successorCells |= successorCell;
                }
            }
            allTransitions |= inputValueEncoding & successorCells & startingMacrocellEncoding;

            maxActionNumber = std::max(maxActionNumber,actionNumber);
            actionNumber++;
        }
    }


#ifdef WRITE_SMART_ABSTRACTION_FILE
    if (myfile.fail()) throw "Error writing to SmartAbstraction file.";
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
            for (unsigned int start=0; start<nofGridPointsStateCoarse[dim]; start++) {
                BF startingTransitions = allTransitions & encodeInBFBits(mgrBDD,start,stateDimensionsEncodingBits[dim]);
                startingTransitions = startingTransitions.ExistAbstract(dimensionInState);

                for (unsigned int end=0; end<nofGridPointsStateCoarse[dim]; end++) {
                    BF finalTransitions = startingTransitions & encodeInBFBits(mgrBDD,end,nextStateDimensionsEncodingBits[dim]);
                    finalTransitions = finalTransitions.ExistAbstract(dimensionInNextState);

                    for (unsigned int shift=0; shift<nofGridPointsStateCoarse[dim]; shift++) {

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


    // Paint robot BDD
    std::cerr << "DOTTing Robot BDD\n";

    // Write abstraction. But first, rename the first action bit to carry the number of different actions
    for (unsigned int i=0; i<bfVarNames.size(); i++) {
        if (bfVarNames[i]=="action@0'") {
            std::ostringstream nof;
            nof << "action@0.0." << maxActionNumber << "'";
            bfVarNames[i] = nof.str();
        }
    }

    // Write BDD to file
    mgrBDD.writeBDDToFile(workspaceProperties.getStringConfigurationValue("outFileName").c_str(),"",allTransitions,bfVars,bfVarNames);

    BF_newDumpDot(TrivialVariableInfoContainer(mgrBDD,bfVarNames,bfVars),allTransitions,NULL,"/tmp/afterReplication.dot");

}




#endif

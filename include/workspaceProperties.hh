#ifndef __WORKSPACE_PROPERTIES_HH__
#define __WORKSPACE_PROPERTIES_HH__

#include <string>
#include <map>
#include <set>


template<class _ConcreteDynamics> class WorkspaceProperties {
public:
    typedef _ConcreteDynamics ConcreteDynamics;
private:

    double samplingTime; //tau

    std::array<double,ConcreteDynamics::getStateNofDimensions()> stateLowerBound;
    std::array<double,ConcreteDynamics::getStateNofDimensions()> stateUpperBound;
    std::array<double,ConcreteDynamics::getStateNofDimensions()> stateETA;
    std::array<double,ConcreteDynamics::getInputNofDimensions()> inputLowerBound;
    std::array<double,ConcreteDynamics::getInputNofDimensions()> inputUpperBound;
    std::array<double,ConcreteDynamics::getInputNofDimensions()> inputETA;

    std::array<int,ConcreteDynamics::getStateNofDimensions()> splittingFactors;

    std::array<double,ConcreteDynamics::getStateNofDimensions()> transitionEnumeratorWindowLowerBound;
    std::array<double,ConcreteDynamics::getStateNofDimensions()> transitionEnumeratorWindowUpperBound;

    std::array<double,ConcreteDynamics::getStateNofDimensions()> maxSpreadingFactors;

    std::map<std::string,double> values;
    std::set<std::string> usedValues;
    std::map<std::string,std::string> stringValues;

    // Used to bulk-parse
    template<int dim> void parseStringDoublePairsToDoubleArrays(std::array<double,dim> &destination,std::string varNameSuffix, std::array<std::string,dim> varNamePrefixes) {
        for (unsigned int i=0;i<dim;i++) {
            std::string currentKey = varNamePrefixes[i]+varNameSuffix;
            auto it = values.find(currentKey);
            if (it==values.end()) {
                std::ostringstream errorMsg;
                errorMsg << "Error in configuration file. Cannot find value for key '" << currentKey << "'";
                throw errorMsg.str();
            }
            destination[i] = it->second;
            usedValues.insert(it->first);
        }
    }
    template<int dim> void parseStringDoublePairsToIntArrays(std::array<int,dim> &destination,std::string varNameSuffix, std::array<std::string,dim> varNamePrefixes) {
        for (unsigned int i=0;i<dim;i++) {
            std::string currentKey = varNamePrefixes[i]+varNameSuffix;
            auto it = values.find(currentKey);
            if (it==values.end()) {
                std::ostringstream errorMsg;
                errorMsg << "Error in configuration file. Cannot find value for key '" << currentKey << "'";
                throw errorMsg.str();
            }
            destination[i] = it->second;
            usedValues.insert(it->first);
        }
    }

public:

    WorkspaceProperties(const char* filePath) {
        std::ifstream dynamicsFile(filePath);
        if (dynamicsFile.is_open()) {
            // Everything ok.
        }
        else {
          std::ostringstream os;
          os << "Error opening configuration file '" << filePath << "'. Please check that the file exists.";
          throw os.str();
        }

        std::string currentLine;
        while (std::getline(dynamicsFile,currentLine)) {
            while (currentLine.substr(0,1)==" ") currentLine = currentLine.substr(1,std::string::npos);
            if (currentLine.size()==0) {
                // Empty line
            } else if (currentLine.substr(0,1)=="#") {
                // comment
            } else {
                size_t equalityPos=currentLine.find("=");
                if (equalityPos==std::string::npos) throw "Error in configuration file: Does not find the equality sign.";
                std::string key = currentLine.substr(0,equalityPos);
                std::string valueString = currentLine.substr(equalityPos+1,std::string::npos);

                double epsilon = 0.0;
                if (endsWith(valueString,"epsilon") && valueString.size()>8) {

                    std::map<std::string,double>::const_iterator it = values.find("epsilon");
                    if (it==values.end()) {
                        throw "Error: epsilon used before it definition.";
                    }

                    if (valueString[valueString.size()-8]=='-') {
                        epsilon = it->second * -1.0;
                    } else if (valueString[valueString.size()-8]=='+') {
                        epsilon = it->second;
                    } else {
                        throw "Error: Usage of epsilon must be preceded by '+' or '-'";
                    }

                    valueString = valueString.substr(0,valueString.size()-8);
                }

                std::istringstream valueReader(valueString);

                double value;
                valueReader >> value;
                value += epsilon;
                if (valueReader.fail()) {
                    if (stringValues.count(key)>0) throw "Error: Value in configuration file multiply defined.";
                    stringValues[key] = currentLine.substr(equalityPos+1,std::string::npos);
                } else {
                    if (values.count(key)>0) throw "Error: Value in configuration file multiply defined.";
                    values[key] = value;
                }
            }
        }

        if (dynamicsFile.bad()) throw "Error reading parameter file for the workspace properties.";
        dynamicsFile.close();

        // Parse the values
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getStateNofDimensions()>(stateLowerBound,"_lb",ConcreteDynamics::getStateVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getStateNofDimensions()>(stateUpperBound,"_ub",ConcreteDynamics::getStateVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getStateNofDimensions()>(stateETA,"_eta",ConcreteDynamics::getStateVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getInputNofDimensions()>(inputLowerBound,"_lb",ConcreteDynamics::getInputVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getInputNofDimensions()>(inputUpperBound,"_ub",ConcreteDynamics::getInputVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getInputNofDimensions()>(inputETA,"_eta",ConcreteDynamics::getInputVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getStateNofDimensions()>(transitionEnumeratorWindowLowerBound,"_window_lb",ConcreteDynamics::getStateVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getStateNofDimensions()>(transitionEnumeratorWindowUpperBound,"_window_ub",ConcreteDynamics::getStateVariableNames());
        parseStringDoublePairsToIntArrays<ConcreteDynamics::getStateNofDimensions()>(splittingFactors,"_splitting",ConcreteDynamics::getStateVariableNames());
        parseStringDoublePairsToDoubleArrays<ConcreteDynamics::getStateNofDimensions()>(maxSpreadingFactors,"_maxSpreadingFactor",ConcreteDynamics::getStateVariableNames());


        // Sanity check: All lower bounds of rotation-invariant properties must be 0
        for (unsigned int i=0;i<ConcreteDynamics::getStateNofDimensions();i++) {
            if (ConcreteDynamics::getDimensionIsWrapAround()[i]) {
                if (stateLowerBound[i]!=0.0) {
                    throw "Error: The lower bounds of all dimensions that are 'wrapping around' must be 0.";
                }
            }
        }

        // Sanity check: No translation-invariant wrap-around dynamics supported at the moment
        for (unsigned int i=0;i<ConcreteDynamics::getStateNofDimensions();i++) {
            if (ConcreteDynamics::getDimensionIsWrapAround()[i] && ConcreteDynamics::getDimensionIsTranslationInvariant()[i]) throw "Error: No dimension may be both wrap-around and translation-invariant. -- This cannot be handled by the respective transition-replication code in this implementation.";
        }

        // Parse tau
        if (values.count("tau")==0) throw "Error: No Tau (timestep) value defined.";
        samplingTime = values["tau"];
        usedValues.insert("tau");
        usedValues.insert("epsilon");

        // Everything used?
        for (auto it : values) {
            if (usedValues.count(it.first)==0) {
                std::cerr << "Warning: Configuration file value for variable " << it.first << " is not used.\n";
                throw "Warning treated as error";
            }
        }
    }


    std::array<double,ConcreteDynamics::getStateNofDimensions()>& getStateLowerBound() {return stateLowerBound;}
    std::array<double,ConcreteDynamics::getStateNofDimensions()>& getStateUpperBound() {return stateUpperBound;}
    std::array<double,ConcreteDynamics::getStateNofDimensions()>& getStateETA() {return stateETA;}
    std::array<double,ConcreteDynamics::getInputNofDimensions()>& getInputLowerBound() {return inputLowerBound;}
    std::array<double,ConcreteDynamics::getInputNofDimensions()>& getInputUpperBound() {return inputUpperBound;}
    std::array<double,ConcreteDynamics::getInputNofDimensions()>& getInputETA() {return inputETA;}
    std::array<int,ConcreteDynamics::getStateNofDimensions()>& getSplittingFactors() {return splittingFactors;}
    std::array<double,ConcreteDynamics::getStateNofDimensions()>& getMaxSpreadingFactors() {return maxSpreadingFactors;}

    std::array<int,ConcreteDynamics::getStateNofDimensions()> getNofDiscreteStateCellsFineAbstraction() {
        std::array<int,ConcreteDynamics::getStateNofDimensions()> retValue;
        for (unsigned int i=0;i<ConcreteDynamics::getStateNofDimensions();i++) {
            if (ConcreteDynamics::getDimensionIsWrapAround()[i]) {
                retValue[i] = static_cast<int>(((ConcreteDynamics::getDimensionWrapAroundLimits()[i])+stateETA[i]*0.1) / stateETA[i]);
            } else {
                retValue[i] = static_cast<int>(((stateUpperBound[i]-stateLowerBound[i])+stateETA[i]*0.1) / stateETA[i]);
            }
        }
        return retValue;
    }


    std::array<double,ConcreteDynamics::getStateNofDimensions()>& getTransitionEnumeratorWindowUpperBound() { return transitionEnumeratorWindowUpperBound; }
    std::array<double,ConcreteDynamics::getStateNofDimensions()>& getTransitionEnumeratorWindowLowerBound() { return transitionEnumeratorWindowLowerBound; }

    double getSamplingTime() const { return samplingTime; }

    std::string getStringConfigurationValue(std::string key) {
        auto it = stringValues.find(key);
        if (it==stringValues.end()) {
            std::ostringstream errorMsg;
            errorMsg << "Error: Did not find key '" << key << "' in configuration file.";
            throw errorMsg.str();
        }
        return it->second;
    }

private:

};





#endif

#!/usr/bin/env python2
#
# Generates the subfolders for a benchmark
import os, subprocess, sys, glob, shutil

# ===================================================
# Settings
# ===================================================

# The splitting factor when used
SPLITTING_FACTOR_X = 4
SPLITTING_FACTOR_Y = 4
SPLITTING_FACTOR_CUR_HEADING = 4
ETA_X = 0.125
ETA_Y = 0.125
ETA_CUR_HEADING = 0.0490875

# The abstraction definition using the splitting factors actually used for this example.
def getAbstractionConfiguration(splittingFactorX,splittingFactorY,splittingFactorCurHeading,etaX,etaY,etaCurHeading):
    return """
# Timestep
tau=1.0
epsilon=0.00001

# Workspace size
xbit_lb=0.0-epsilon
ybit_lb=0.0-epsilon
xbit_ub=8.0+epsilon
ybit_ub=8.0+epsilon
curHeading_lb=0.0
curHading_ub=8.0+epsilon
xbit_eta="""+str(etaX)+"""
ybit_eta="""+str(etaY)+"""
curHeading_eta="""+str(etaCurHeading)+"""


# Input properties
rot_lb=-1.70169-epsilon
rot_ub=1.70169+epsilon
rot_eta=0.261799


# Window settings
xbit_window_lb=0.0-epsilon
xbit_window_ub=8.0+epsilon
ybit_window_lb=0.0-epsilon
ybit_window_ub=8.0+epsilon

curHeading_window_lb=0.0-epsilon
curHeading_window_ub=8.0+epsilon

# Splitting factors
xbit_splitting="""+str(splittingFactorX)+"""
ybit_splitting="""+str(splittingFactorY)+"""
curHeading_splitting="""+str(splittingFactorCurHeading)+"""

# Maximum Spreading Factor
xbit_maxSpreadingFactor=1.5
ybit_maxSpreadingFactor=1.5
curHeading_maxSpreadingFactor=1.5

# Do not change the following line!
outFileName=slugsExample.bdd
"""

def getSpecification(splittingFactorX,splittingFactorY,splittingFactorCurHeading):
    return """
[MOTION STATE OUTPUT]
xbit:0..."""+str(63/splittingFactorX-1)+"""
ybit:0..."""+str(63/splittingFactorY-1)+"""
curHeading:0..."""+str(127/splittingFactorCurHeading-1)+"""

[MOTION CONTROLLER OUTPUT]
action:0...<TBDACTIONNUM>

[INPUT]
door8

[OTHER OUTPUT]

[ENV_INIT]
# ! door

[ENV_TRANS]
# door'
# | ! door ! door'

[ENV_TRANS]
1

[SYS_INIT]
taskCounter=1
# xbit=5
# ybit=5
# curHeading=0

[SYS_LIVENESS]
xbit<="""+str(19/splittingFactorX-1)+""" & ybit<="""+str(19/splittingFactorY-1)+""" 
xbit>="""+str(43/splittingFactorX-1)+""" & ybit<="""+str(43/splittingFactorY-1)+""" 

[SYS_TRANS]
# x'!=0 | y'!=0 | curHeading'!=0 | curSpeed'!=0"""


# ===================================================
# Write directories
# ===================================================
for caseName,parameterToTheAbstractionGenerator,xSplitConf,ySplitConf,curHeadingSplitConf,xSplitSpec,ySplitSpec,curHeadingSplitSpec,xEta,yEta,curHeadingEta in [
     ("DumbFine","--vehicleSingleSpeed --dumb",1,1,1,1,1,1,ETA_X,ETA_Y,ETA_CUR_HEADING),
     ("DumbCoarse","--vehicleSingleSpeed --dumb",1,1,1,SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,SPLITTING_FACTOR_X*ETA_X,SPLITTING_FACTOR_Y*ETA_Y,SPLITTING_FACTOR_CUR_HEADING*ETA_CUR_HEADING),
     ("SteadyNoFilter","--vehicleSingleSpeed --steady",SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,ETA_X,ETA_Y,ETA_CUR_HEADING),
     ("SteadyFilter","--vehicleSingleSpeed --steady --removeDominant",SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,ETA_X,ETA_Y,ETA_CUR_HEADING),
     ("SteadyRestrictSpreading","--vehicleSingleSpeed --steady --removeDominant --restrictSpreading",SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,ETA_X,ETA_Y,ETA_CUR_HEADING),
     ("SteadyHybrid","--vehicleSingleSpeed --hybrid",SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,SPLITTING_FACTOR_X,SPLITTING_FACTOR_Y,SPLITTING_FACTOR_CUR_HEADING,ETA_X,ETA_Y,ETA_CUR_HEADING),
     ]:
     
    # Generate directory
    try:
        os.mkdir(caseName)
    except OSError as e:
        if e.errno != os.errno.EEXIST:
            raise

    # Copy files
    assert os.path.exists("dataFiles")
    for filename in glob.glob("dataFiles/*"):
        newFileName = filename[10:]
        shutil.copy(filename,caseName+"/"+newFileName)    
    
    # Slugs Specification
    with open(caseName+"/slugsExample.structuredslugs","w") as outFile:
        outFile.write(getSpecification(xSplitSpec,ySplitSpec,curHeadingSplitSpec)+"\n")
    
    # Configuration file
    with open(caseName+"/abstraction.conf","w") as outFile:
        outFile.write(getAbstractionConfiguration(xSplitConf,ySplitConf,curHeadingSplitConf,xEta,yEta,curHeadingEta)+"\n")
        
    # run.sh
    with open(caseName+"/run.sh","w") as outFile:
        outFile.write("set -e\n")
        outFile.write("../../../mainProject "+parameterToTheAbstractionGenerator+" abstraction.conf\n")
        outFile.write("../updateNofActions.py slugsExample.bdd slugsExample.structuredslugs\n")
        
        


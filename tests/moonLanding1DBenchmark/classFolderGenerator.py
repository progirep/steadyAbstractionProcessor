#!/usr/bin/env python2
#
# Generates the subfolders for a benchmark
import os, subprocess, sys, glob, shutil

# ===================================================
# Settings
# ===================================================

# The splitting factor when used
SPLITTING_FACTOR_X = 2
SPLITTING_FACTOR_X_SPEED = 3
ETA_X = 0.5
ETA_XSPEED = 0.125

# The abstraction definition using the splitting factors actually used for this example.
def getAbstractionConfiguration(splittingFactorX,splittingFactorXSpeed,etaX,etaXSpeed):
    return """
# Timestep
tau=1.0
epsilon=0.00001

# Workspace size
xbit_lb=0.0-epsilon
xSpeed_lb=-2.0625-epsilon
xbit_ub=10.0+epsilon
xSpeed_ub=2.0625+epsilon
xbit_eta="""+str(etaX)+"""
xSpeed_eta="""+str(etaXSpeed)+"""


# Input properties
xAcc_lb=-2.0-epsilon
xAcc_ub=0.0+epsilon
xAcc_eta=0.125


# Window settings
xbit_window_lb=0.0-epsilon
xbit_window_ub=10.0+epsilon

xSpeed_window_lb=-2.0625-epsilon
xSpeed_window_ub=2.0625+epsilon

# Splitting factors
xbit_splitting="""+str(splittingFactorX)+"""
xSpeed_splitting="""+str(splittingFactorXSpeed)+"""

# Maximum Spreading Factor
xbit_maxSpreadingFactor=1.5
xSpeed_maxSpreadingFactor=1.5

# Do not change the following line!
outFileName=slugsExample.bdd
"""

def getSpecification(splittingFactorX,splittingFactorXSpeed):
    return """
[MOTION STATE OUTPUT]
xbit:0..."""+str(20/splittingFactorX-1)+"""
ybit:0...1
xSpeed:0..."""+str(33/splittingFactorXSpeed-1)+"""

[MOTION CONTROLLER OUTPUT]
action:0...<TBDACTIONNUM>

[INPUT]
door8

[OTHER OUTPUT]
taskCounter:0...5

[ENV_INIT]
# ! door

[ENV_TRANS]
# door'
# | ! door ! door'

[ENV_TRANS]
taskCounter!=5

[SYS_INIT]
taskCounter=1
# xbit=5
# ybit=5
# xSpeed=1
# ySpeed=1

[SYS_LIVENESS]
0

[SYS_TRANS]
# x'!=0 | y'!=0 | curHeading'!=0 | curSpeed'!=0

[SYS_TRANS]
taskCounter=1 -> taskCounter'=1 | xbit'<="""+str(4/splittingFactorX-1)+""" & taskCounter'=2
taskCounter=2 -> taskCounter'=2 | xbit'>="""+str(12/splittingFactorX-1)+""" & taskCounter'=3
taskCounter=3 -> taskCounter'=3 | xbit'<="""+str(10/splittingFactorX-1)+""" & taskCounter'=4
taskCounter=4 -> taskCounter'=4 | xbit'<="""+str(4/splittingFactorX-1)+""" & xSpeed'>="""+str(7/splittingFactorXSpeed-1)+""" & taskCounter'=5"""

#  & xSpeed>="""+str(16/splittingFactorXSpeed-1)+""" & xSpeed<="""+str(20/splittingFactorXSpeed-1)+"""

# ===================================================
# Write directories
# ===================================================
for caseName,parameterToTheAbstractionGenerator,xSplitConf,xSpeedSplitConf,xSplitSpec,xSpeedSplitSpec,xEta,xSpeedEta in [
     ("DumbFine","--moon1D --dumb",1,1,1,1,ETA_X,ETA_XSPEED),
     ("DumbCoarse","--moon1D --dumb",1,1,SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,SPLITTING_FACTOR_X*ETA_X,SPLITTING_FACTOR_X_SPEED*ETA_XSPEED),
     ("SteadyNoFilter","--moon1D --steady",SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,ETA_X,ETA_XSPEED),
     ("SteadyFilter","--moon1D --steady --removeDominant",SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,ETA_X,ETA_XSPEED),
     ("SteadyRestrictSpreading","--moon1D --steady --removeDominant --restrictSpreading",SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,ETA_X,ETA_XSPEED),
     ("SteadyHybrid","--moon1D --hybrid",SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,SPLITTING_FACTOR_X,SPLITTING_FACTOR_X_SPEED,ETA_X,ETA_XSPEED),
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
        outFile.write(getSpecification(xSplitSpec,xSpeedSplitSpec)+"\n")
    
    # Configuration file
    with open(caseName+"/abstraction.conf","w") as outFile:
        outFile.write(getAbstractionConfiguration(xSplitConf,xSpeedSplitConf,xEta,xSpeedEta)+"\n")
        
    # run.sh
    with open(caseName+"/run.sh","w") as outFile:
        outFile.write("set -e\n")
        outFile.write("../../../mainProject "+parameterToTheAbstractionGenerator+" abstraction.conf\n")
        outFile.write("../updateNofActions.py slugsExample.bdd slugsExample.structuredslugs\n")
        
        


#!/usr/bin/env python2
# Insert the number of actions to the structuredslugs file
import os, sys

if len(sys.argv)<3:
    print >>sys.stderr, "Error: Expected the BDD file and the structuredslugs file as parameters"
    
    
bddFileName = sys.argv[1]
structuredSlugsinFile = sys.argv[2]

for line in open(bddFileName).readlines():
    if line.startswith(".suppvarnames"):
        allParts = line.split(" ")
        for part in allParts:
            if part.startswith("action@0."):
                assert part.endswith("'")
                part = part[0:len(part)-1]
                part = part.split(".")
                maxAction = part[-1]
            
with open(structuredSlugsinFile,"r") as inFile:            
    slugsinFile = list(inFile.readlines())
    
with open(structuredSlugsinFile,"w") as outFile:
    for line in slugsinFile:
        outFile.write(line.replace("<TBDACTIONNUM>",maxAction))

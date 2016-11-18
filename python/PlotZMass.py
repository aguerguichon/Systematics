import ROOT
import csv
import math
from ROOT import *

path="/sps/atlas/a/aguerguichon/Calibration/Bias/"
rootFile="Toys/Plots/outZMassFile_"
setList=[ "set1", "set2" ]

for iConf in range(0, 6):
    for jConf in range (0, iConf+1):
        configFile = open("ZMass_"+str(iConf)+"_"+str(jConf)+".boost", "w") 
        configFile.write("inputType=0 \n")
        for setNumber in setList:
            openedFile= TFile.Open(path+rootFile+setNumber+".root")
            rootFileName="rootFileName="+path+rootFile+setNumber+".root\n"    
            objName = "ChiMatrix_" +str(iConf)+ "_"+str(jConf)+"_dataZMass"
            if openedFile.Get(objName):
                configFile.write( rootFileName )
                configFile.write( "objName="+objName+"\n" )
                configFile.write( "legend= "+str(setNumber)+", RMS= "+str(openedFile.Get(objName).GetRMS())+"\n" )
                if setNumber==setList[0]:
                    RMS1=openedFile.Get(objName).GetRMS()
                    #print openedFile.Get(objName).GetRMS()
                if setNumber==setList[1]:
                    RMS2=openedFile.Get(objName).GetRMS()

        configFile.write("legendPos= 0.65 0.9 \n" )
        #configFile.write("latex= c= "+str( math.sqrt(abs(RMS2**2-RMS1**2)) )+"\n")
        #configFile.write("latexOpt= 0.15 0.8 \n")
        configFile.write("normalize=1 \n")            
        configFile.write("latex=Configuration ("+str(iConf)+", "+str(jConf)+")\n")
        configFile.write("latexOpt= 0.15 0.85 \n")
        configFile.write("plotDirectory="+path+"Plots/ \n")
        configFile.close()


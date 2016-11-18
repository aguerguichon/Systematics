import ROOT
import csv
import math
from ROOT import *

path="/sps/atlas/a/aguerguichon/Calibration/Bias/"
rootFile="RootFiles/BiasConf_"
setList=[ "set1", "set2" ]
mean1=0
mean2=0

for iConf in range(0, 6):
    for jConf in range (0, iConf+1):
        configFile = open("Sets_"+str(iConf)+"_"+str(jConf)+".boost", "w")
        configFile.write("inputType=0 \n")
        for setNumber in setList:
            openedFile= TFile.Open(path+rootFile+setNumber+".root")
            rootFileName="rootFileName="+path+rootFile+setNumber+".root\n"
            objName = "inputC_7000_statTree_100000_indepTemplates_0_indepDistorded_2_bootstrap_0_iConf_" +str(iConf)+ "_jConf_"+str(jConf)
            if openedFile.Get(objName):
                hist=openedFile.Get(objName)
                configFile.write( rootFileName )
                configFile.write( "objName="+objName+"\n" )
                configFile.write( "legend= "+str(setNumber)+", mean= "+str(  openedFile.Get(objName).GetMean() )+"\n" )
                if setNumber==setList[0]:
                    mean1= hist.GetMean()
                    #print mean1
                if setNumber==setList[1]:
                    mean2= hist.GetMean()

        configFile.write("legendPos= 0.6 0.9 \n" )
       # configFile.write("latex= c= "+str( math.sqrt(abs(mean2**2-mean1**2)) ) +"\n")
        #configFile.write("latexOpt= 0.15 0.8 \n")
        configFile.write("normalize=1 \n")
        configFile.write("latex=Configuration ("+str(iConf)+", "+str(jConf)+")\n")
        configFile.write("xTitle=bias\n")
        configFile.write("latexOpt= 0.15 0.85 \n")
        configFile.write("plotDirectory="+path+"Plots/ \n")
        configFile.write("extendUp=0.4 \n")
        configFile.close()

import ROOT
import csv
from ROOT import *

path="/sps/atlas/a/aguerguichon/Calibration/Bias/"
rootFile= path+"RootFiles/BiasConf_newBootstrap.root"
openedFile= TFile.Open(rootFile)
statList=[100000, 1000000]
inputList=[0.007, 0.01]

rootFileName="rootFileName="+rootFile+"\n"

statFile=open(path+"Stats/BiasConf_newBootstrap.csv", "rb")

reader= csv.reader(statFile, delimiter=' ')

rootFileRef=path+"RootFiles/BiasConf_ref.root"
openedFileRef=TFile.Open(rootFileRef)
statFileRef=open(path+"Stats/BiasConf_ref.csv", "rb")
readerRef=csv.reader(statFileRef, delimiter=' ')

for iConf in range(0, 6):
    for jConf in range (0, iConf+1):
        configFile = open("InputStat_"+str(iConf)+"_"+str(jConf)+".boost", "w") 
        configFile.write("inputType=0 \n")
        for row in reader:
            for iStat in statList:
                for iInput in inputList:
                    objName = "inputC_"+str(int(iInput*1e6))+"_statTree_"+str(iStat)+"_indepTemplates_2_indepDistorded_2_bootstrap_2_iConf_" +str(iConf)+ "_jConf_"+str(jConf)
                    if openedFile.Get(objName):
                        if row[0]==objName:
                            configFile.write( rootFileName )
                            configFile.write( "objName="+objName+"\n" )
                            configFile.write( "legend= input: "+ str(iInput)+", stat: "+'{:.0e}'.format(iStat)+", Mean="+'{:.3e}'.format(float(row[10]))+", RMS="+'{:.3e}'.format(float(row[11]))+"\n" )

        for rowRef in readerRef:
            for iInput in inputList:
                objName = "inputC_"+str(int(iInput*1e6))+"_statTree_2774685_indepTemplates_2_indepDistorded_2_bootstrap_0_iConf_" +str(iConf)+ "_jConf_"+str(jConf)
                if openedFileRef.Get(objName) and rowRef[0]==objName:
                    configFile.write ( "rootFileName="+rootFileRef+"\n")
                    configFile.write("objName="+objName+"\n")
                    configFile.write( "legend= input: "+ str(iInput)+", stat: 2.7M , Mean="+'{:.3e}'.format(float(rowRef[10]))+", RMS="+'{:.3e}'.format(float(rowRef[11]))+"\n" )
                     
 
        configFile.write("legendPos= 0.3 0.85 \n" )
        configFile.write("normalize=1 \n")            
        configFile.write("latex=Configuration ("+str(iConf)+", "+str(jConf)+")\n")
        configFile.write("latexOpt= 0.3 0.9 \n")
        configFile.write("xTitle=c^{meas}-c^{input} \n")
        configFile.write("yTitle=Number of events \n")
        configFile.write("plotDirectory="+path+"Plots/ \n")
        configFile.write("extendUp=0.55\n")
        configFile.close()
        statFile.seek(0)
        statFileRef.seek(0)

statFile.close()
statFileRef.close()

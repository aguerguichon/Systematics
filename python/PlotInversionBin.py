import ROOT
import csv
from ROOT import *

#path="/sps/atlas/a/aguerguichon/Calibration/Bias/Inversion/IndepDistorded/"

path="/sps/atlas/a/aguerguichon/Calibration/Bias/Inversion/IndepDistorded/"

invProcedure= ["1", "11", "12"]
legendName=["analytical", "c", "c^{2}"]

for iBin in range(0, 6):
    configFile = open("CompareInvProcedure_"+str(iBin)+".boost", "w") 
    configFile.write("inputType=0 \n")
    for iInv in range(0, 3):
        statFile=open(path+"CiMatricesInv_"+invProcedure[iInv]+".csv", "rb")
        reader= csv.reader(statFile, delimiter=" ")
        for row in reader:
            if row[1]==str(iBin):
                configFile.write("rootFileName="+path+"CiMatricesInv_"+invProcedure[iInv]+".root\n")  
                configFile.write( "objName=biasBin_"+str(iBin)+"\n" )
                configFile.write( "legend= "+legendName[iInv]+ ", Mean="+str(row[2])+", RMS="+row[3]+"\n" )    
        statFile.seek(0)
        statFile.close()

                
    configFile.write("legendPos= 0.45 0.85 \n" )
    configFile.write("normalize=1 \n")            
    configFile.write("latex=Bin "+ str(iBin)+"\n")
    configFile.write("latexOpt= 0.15 0.85 \n")
    configFile.write("latex=Input: 0.007\n")
    configFile.write("latexOpt= 0.15 0.8 \n")    
    configFile.write("xTitle=c_{i}^{meas}-c^{input} \n")
    configFile.write("yTitle=Number of events \n")
    configFile.write("plotDirectory="+path+" \n")
    configFile.write("extendUp=0.4 \n")
    configFile.close()

import ROOT
from ROOT import *

configFile=open("CompMee.boost", "w")
configFile.write("inputType=0 \n")


path= "/sps/atlas/c/cgoudet/Calibration/DataxAOD/"
dataNameList=[ "Data_13TeV_Zee_2015_Lkh1_scaled", "Data_13TeV_Zee_2016_Lkh1_scaled" ] 


for dataName in dataNameList:
    rootFileName=path+dataName+"/"+dataName+"_0.root"
    objName=dataName+"_0_ZMass"
    rootFile=TFile.Open(rootFileName)
    configFile.write("rootFileName= "+rootFileName+"\n")
    configFile.write("objName= "+objName+"\n")
#    hist=rootFile.Get(objName)
    configFile.write("legend= "+dataName+" , mean= "+str(rootFile.Get(objName).GetMean())+"\n")
    configFile.write("varName= m12 \n")

configFile.write("varMin=80 \nvarMax=100 \nlegendPos= 0.3 0.85 \nnormalize=1 \nxTitle=M_{ee} \nplotDirectory= ~/public/Calibration \nextendUp=0.4")


    

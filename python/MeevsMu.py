import ROOT
from ROOT import *

yearList= ["2015","2016"]

path="/sps/atlas/c/cgoudet/Calibration/DataxAOD/"
configFile=open("/afs/in2p3.fr/home/a/aguergui/public/Calibration/Calibration_bias", "w")

configFile.write("inputType=8 \n")

for year in yearList:
    data="Data_13TeV_Zee_"+year+"_Lkh1_scaled"
    configFile.write("rootFileName= "+path+data+"/"+data+"_0.root \n")

# inputType=8

# rootFileName=/sps/atlas/c/cgoudet/Calibration/DataxAOD/Data_13TeV_Zee_2015_Lkh1_scaled/Data_13TeV_Zee_2015_Lkh1_scaled_0.root
# objName=Data_13TeV_Zee_2015_Lkh1_scaled_0_selectionTree

# legend= 2015 scaled
# varName=muPU m12
# varWeight=weight
# selectionCut=m12>=80 && m12<=100 

# rootFileName=/sps/atlas/c/cgoudet/Calibration/DataxAOD/Data_13TeV_Zee_2016_Lkh1_scaled/Data_13TeV_Zee_2016_Lkh1_scaled_0.root
# objName=Data_13TeV_Zee_2016_Lkh1_scaled_0_selectionTree

# legend= 2016 scaled
# varName=muPU m12
# varWeight=weight
# selectionCut=m12>=80 && m12<=100 

# rangeUserY= 89.9 90.3
# varMin=0
# varMax=30
# legendPos=0.75 0.85
# xTitle= __HASHTAGmu
# yTitle= M_{ee}
# plotDirectory=~/public/Calibration

typeList= ["MC","Data"]

yearList=["2016"]

prodList=["c"]

path="/sps/atlas/c/cgoudet/Calibration/DataxAOD/"
configFile=open("/afs/in2p3.fr/home/a/aguergui/public/Calibration/Calibration_bias/ConfigFile/MCDataRatio_"+yearList[0]+".boost", "w")
configFile.write("inputType=1 \n")

for i in range (0, 1):
    for typeData  in typeList: 
        data="Data_13TeV_Zee_"+yearList[i]+"_Lkh1_scaled"
        if typeData=="MC":
            data="MC_13TeV_Zee_2015"+prodList[i]+"_Lkh1_scaled"
        # configFile.write("rootFileName= "+path+data+"/"+data+"_0.root \n")
        # configFile.write("objName= "+data+"_0_ZMass \nlegend= "+yearList[i]+", "+typeData+" \n")
            
        configFile.write("loadFiles="+path+data+"/"+data+".boost\n")
        configFile.write("varName=m12 \n")
        configFile.write("legend= "+yearList[i]+", "+typeData+"\n")
        configFile.write("selectionCut=m12>=80 && m12<=100 \n") 

configFile.write("doRatio=1 \ndrawStyle=1 \n")
configFile.write("nComparedEvents=20 \nvarMin=80 \nvarMax=100 \nlegendPos= 0.75 0.9 \nnormalize= 1.021475e6 \nyTitle=#frac{1}{N}#frac{dN}{1 GeV} \nextendUp=0.3 \n")
#configFile.write("rangeUserY= 89.5 90.4 \nvarMin=275e3 \nvarMax=302e3 \nlegendPos= 0.55 0.85 \nxTitle=runNumber \nyTitle=M_{ee} \nplotDirectory= ~/public/Calibration \n")
configFile.write("plotDirectory= ~/public/Calibration")
configFile.close

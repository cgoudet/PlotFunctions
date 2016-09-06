selectionList= ["eta_calo_1>=-1.37 && eta_calo_1<=1.37 && eta_calo_2>=-1.37 && eta_calo_2<=1.37", "eta_calo_1>=1.55 && eta_calo_2>=1.55", "eta_calo_1<=-1.55 && eta_calo_2<=-1.55"]

nameList= ["barrel", "EC A", "EC C"]

yearList=["2015", "2016"]

path="/sps/atlas/c/cgoudet/Calibration/DataxAOD/"
configFile=open("/afs/in2p3.fr/home/a/aguergui/public/Calibration/Calibration_bias/ConfigFile/Select.boost", "w")
configFile.write("inputType=8 \n")

for year in yearList:
    data="Data_13TeV_Zee_"+year+"_Lkh1_scaled"
    counter=0
    for selection in selectionList:
        configFile.write("rootFileName= "+path+data+"/"+data+"_0.root \n")
        configFile.write("objName= "+data+"_0_selectionTree \nlegend= "+year+", "+nameList[counter]+" \n")
        counter=counter+1
        configFile.write("varName= runNumber m12 \nvarWeight= weight \n")
        configFile.write("selectionCut= m12>=80 && m12<=100 && "+selection+"\n")

configFile.write("rangeUserY= 89.5 90.4 \nvarMin=275e3 \nvarMax=302e3 \nlegendPos= 0.55 0.85 \nxTitle=runNumber \nyTitle=M_{ee} \nplotDirectory= ~/public/Calibration \n")
configFile.close

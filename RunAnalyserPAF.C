////////////////////////////////////////////////////////////////////////////////
//
//  RunAnalyserPAF("sampleName", "TheAnalysis", nSlots);
//  RunAnalyserPAF("sampleName", "TheAnalysis", nSlots, nEvents = 0, weight = 1);
//
//  To do: Automatic run on all _ext files
//
////////////////////////////////////////////////////////////////////////////////
R__LOAD_LIBRARY(DatasetManager/DatasetManager.C+)
#include "DatasetManager/DatasetManager.h"
#include "TLorentzVector.h"

Bool_t IsMCatNLO(TString sampleName);
void GetCount(vector<TString> Files, Bool_t IsData = false);
void RunAnalyserPAF(TString sampleName  = "TTbar_Powheg", TString Selection = "StopDilep", Int_t nSlots = 1, Long64_t nEvents = 0, Long64_t FirstEvent = 0, Float_t ThisWeight = 1.0, Int_t stopMass = 0, Int_t lspMass  = 0);
Float_t GetSMSnorm(Int_t mStop, Int_t mLsp);
Double_t GetStopXSec(Int_t StopMass);

vector<TString> Files;
Double_t SumOfWeights;
Long64_t Count;
Long64_t nTrueEntries;
Float_t xsec;
Bool_t verbose = true;

enum             sel         {iStopSelec, iTopSelec, iTWSelec, iWWSelec, ittDMSelec, ittHSelec, nSel};
const TString tagSel[nSel] = {"Stop", "        Top",     "TW",     "WW",     "ttDM",     "ttH"      };

void RunAnalyserPAF(TString sampleName, TString Selection, Int_t nSlots, Long64_t nEvents, Long64_t FirstEvent, Float_t ThisWeight,	Int_t stopMass, Int_t lspMass) {

	Int_t iChunck = Int_t(ThisWeight);
	if(FirstEvent != 0) verbose = false;
  TString orig_sampleName = sampleName;

  vector<TString> tempfiles;
  Files.clear();
	SumOfWeights = 0;
	Count = 0;
	nTrueEntries = 0;
	xsec = 0;


  TString WorkingDir = gSystem->WorkingDirectory();

  // VARIABLES TO BE USED AS PARAMETERS...
  Float_t G_Event_Weight  = 1.0;         
  Bool_t  G_IsData        = false;       
  Bool_t  G_IsMCatNLO     = false;
  Bool_t  G_DoSystematics = false;
  Bool_t  G_IsFastSim     = false;

  // Selection
  Int_t sel = 0;
  if     (Selection == "StopDilep" || Selection == "stop"    ) sel = iStopSelec;
  else if(Selection == "Top"       || Selection == "TOP"     ) sel = iTopSelec;
  else if(Selection == "TW"        || Selection == "tW"      ) sel = iTWSelec;
  else if(Selection == "ttDM"      || Selection == "ttMET"   ) sel = ittDMSelec;
  else if(Selection == "ttH"       || Selection == "TTH"     ) sel = ittHSelec;
  else if(Selection == "WW"                                  ) sel = iWWSelec;
  else{ cout << "\033[1;31m >>>> WRONG SELECTION <<<< \033[0m\n"; return;}
	cout << "\n" << endl;
  if(verbose) cout << Form("\033[1;35m >>> Analysis: %s \033[0m\n", tagSel[sel].Data());

  // INPUT DATA SAMPLE
  //----------------------------------------------------------------------------
  TString dataPath = "/pool/ciencias/"; // Base path to input files
  TString userhome = "/mnt_pool/fanae105/user/$USER/";
  DatasetManager* dm = DatasetManager::GetInstance();

  // Tab in the spreadsheet https://docs.google.com/spreadsheets/d/1b4qnWfZrimEGYc1z4dHl21-A9qyJgpqNUbhOlvCzjbE
  dm->SetTab("DR80XSummer16asymptoticMiniAODv2_2");
  
  TString pathToFiles = dataPath + dm->FindLocalFolder();
  // Deal with data samples
  if(sampleName == "DoubleEG" || sampleName == "DoubleMuon" || sampleName == "MuonEG" || sampleName.BeginsWith("Single")){
    G_Event_Weight = 1.;
    G_IsData = true;
    TString datasuffix[] = { 
      "16B_03Feb2017",
  /*    "16C_03Feb2017",
      "16D_03Feb2017",
      "16E_03Feb2017",
      "16F_03Feb2017",
      "16G_03Feb2017",
      "16H_03Feb2017_v2",
      "16H_03Feb2017_v3"*/
    };
    const unsigned int nDataSamples = 1;//8;
    for(unsigned int i = 0; i < nDataSamples; i++) {
      TString asample = Form("Tree_%s_%s",sampleName.Data(), datasuffix[i].Data());
      //myProject->AddDataFiles(dm->GetRealDataFiles(asample));
      vector<TString> tempFiles = dm->GetRealDataFiles(asample);
			Files.insert(Files.end(), (tempFiles).begin(), (tempFiles).end());
    }
    GetCount(Files, G_IsData);
    //cout << "Total events in the dataset: " << Count << endl;
    //cout << "Will loop on total number of entries: " << nTrueEntries << endl;  
  }
  else{ // Deal with MC samples           Double_t sumnormFromFiles = GetCount(path, dm->GetRealDataFiles(asample));
    G_IsData = false; 
    TString theSample = "";
		if(sampleName.BeginsWith("LocalFile:")){ // LocalFile
			theSample = sampleName.ReplaceAll("LocalFile:", ""); 
			if(verbose) cout << " >>> Analysing a local sample: " << theSample << endl;
			sampleName = TString( theSample(theSample.Last('/')+1, theSample.Sizeof())).ReplaceAll(".root", "").ReplaceAll("Tree_", "");
			//myProject->AddDataFile(theSample);
			G_Event_Weight = ThisWeight;
      xsec = 1;
		}
		else if(sampleName.BeginsWith("T2tt:")){ // T2tt sample
			theSample = sampleName.ReplaceAll("T2tt:", "");
			if(verbose) cout << " >>> Analysing a T2tt sample: " << theSample << endl;
			//myProject->AddDataFiles(dm->GetRealDataFiles(theSample));
      std::vector<TString> tempFiles = dm->GetRealDataFiles(theSample);
      Files.insert(Files.end(), (tempFiles).begin(), (tempFiles).end());
      GetCount(Files);
			sampleName = Form("T2tt_mStop%i_mLsp%i",stopMass, lspMass);
			G_IsFastSim = true;
      xsec = GetStopXSec(stopMass);
      nTrueEntries = GetSMSnorm(stopMass, lspMass);
			G_Event_Weight = xsec/nTrueEntries;
		} 
		else{ // Use dataset manager
			Float_t sumNorm = 1; long double totalXSec = 0; long double totalNorm = 0;
			TString sampleChain = TString(sampleName);  
			if(sampleName.Contains("&")) sampleName = TString(sampleName(0, sampleName.First('&'))); // For output file
      sampleName.ReplaceAll(" ", "");
			Int_t nFiles = sampleChain.CountChar('&') + 1;
			for(Int_t k = 0; k < nFiles; k++){
				if(sampleChain.Contains("&")){
					theSample  = TString(sampleChain(0,sampleChain.First('&')));
					sampleChain= TString(sampleChain(sampleChain.First('&')+1, sampleChain.Sizeof()));
				}
				else theSample  = sampleChain;
				theSample.ReplaceAll(" ", "");
				dm->LoadDataset(theSample);
				//myProject->AddDataFiles(dm->GetFiles()); 
				Files.insert(Files.end(), (dm->GetFiles()).begin(), (dm->GetFiles()).end());
				xsec    = dm->GetCrossSection();
			}
			GetCount(Files);
			if(IsMCatNLO(sampleName)){
				G_IsMCatNLO = true;
				if(verbose) cout << " >>> This is a aMCatNLO sample!!" << endl;
				G_Event_Weight = xsec/SumOfWeights;
			}
			else G_Event_Weight = xsec/Count;
		}
	}
	if(verbose){
		//cout << "\033[1;30m=================================================\033[0m\n";
		//for(Int_t i = 0; i < (Int_t) Files.size(); i++) cout << Form("\033[1;32m >>> Including file: %s \033[0m\n", Files.at(i).Data());
		cout << "\033[1;30m-------------------------------------------------\033[0m\n";
		if(!G_IsData)   cout << Form("\033[1;34m #### XSec             = %g \033[0m\n", xsec);
		cout << Form("\033[1;34m #### Total Entries    = %lld \033[0m\n", nTrueEntries);
		cout << Form("\033[1;34m #### Total gen events = %lld \033[0m\n", Count);
		cout << Form("\033[1;34m #### Weight for norm  = %g \033[0m\n", G_Event_Weight);
		if(G_IsMCatNLO) cout << Form("\033[1;34m #### Sum of weights   = %g \033[0m\n", SumOfWeights);
		cout << "\033[1;30m=================================================\033[0m\n";
	}

	// Output dir and tree name
	//----------------------------------------------------------------------------
	TString outputDir = "./" + tagSel[sel] + "_temp";
	if(sampleName.BeginsWith("T2tt")) outputDir += "/T2tt/";
	gSystem->mkdir(outputDir, kTRUE);
	if(sampleName.Contains("_ext2")) sampleName.ReplaceAll("_ext2",""); 
	if(sampleName.Contains("_ext"))  sampleName.ReplaceAll("_ext",""); 

	//if     (nEvents > 0 && FirstEvent == 0) myProject->SetNEvents(nEvents);
  if(nEvents < 0 && FirstEvent == 0){ // Divide the sample
    Int_t nChunks = TMath::Abs(nEvents);
    Int_t firstEvent = 0;
    cout << endl;
    cout << Form("\033[0;97m >>> The sample is going to be divided in %i chuncks!! \033[0m\n\n", nChunks);
    for(Int_t i = 0; i < nChunks; i++){
			firstEvent = (nTrueEntries/nChunks)*i+1;
      nEvents = nTrueEntries/nChunks;
			if(i == nChunks - 1) nEvents = nTrueEntries-firstEvent;
			RunAnalyserPAF(orig_sampleName, Selection, nSlots, nEvents, firstEvent, i, stopMass , lspMass);
      //gSystem->Exec("resetpaf -a");
      //gSystem->Exec("resetpaf -a");
    }
    cout << "\033[1;31m >>> Merging trees... \n\033[0m";
    TString haddCommand = "hadd " + outputDir + "/Tree_" + sampleName + ".root " + outputDir + "/Tree_" + sampleName + "_*.root";
    gSystem->Exec(haddCommand);
    cout << "\033[1;37m================================================\n\033[0m";
    cout << "\033[1;37m >>>>> >>>> >>> >> > Finito! < << <<< <<<< <<<<<\n\033[0m";
    cout << "\033[1;37m================================================\n\033[0m";
    return;
  }
  else if(FirstEvent != 0){
    if(FirstEvent == 1) FirstEvent = 0;
		cout << Form("\033[1;36m >>> Running chunck number %i, starting in event %lli... will loop over %lli events (last event = %lli)\n\n\033[0m", iChunck, FirstEvent, nEvents, FirstEvent + nEvents);
    sampleName += Form("_%i", iChunck);
  }


  // PAF mode
  //----------------------------------------------------------------------------
  PAFIExecutionEnvironment* pafmode = 0;
  if      (nSlots <=1 ) pafmode = new PAFSequentialEnvironment();
  else if (nSlots <=64) pafmode = new PAFPROOFLiteEnvironment(nSlots);
  else                  pafmode = new PAFPoDEnvironment(nSlots);
  PAFProject* myProject = new PAFProject(pafmode); // Create PAF Project whith that environment

	myProject->AddDataFiles(Files); 

	if     (nEvents > 0 && FirstEvent == 0) myProject->SetNEvents(nEvents);
	else if(FirstEvent != 0){
		myProject->SetNEvents(nEvents);
		myProject->SetFirstEvent(FirstEvent);
	}
				
	TString outputFile = outputDir + "/Tree_" + sampleName + ".root";
	cout << Form("\033[1;33m >>> Output file = %s \n\033[0m", outputFile.Data());
	cout << "\n" << endl;
	myProject->SetOutputFile(outputFile);

	// Parameters for the analysis
	//----------------------------------------------------------------------------
	// COMMON PARAMETERS
	myProject->SetInputParam("sampleName",    sampleName       );
	myProject->SetInputParam("IsData",        G_IsData         );
	myProject->SetInputParam("weight",        G_Event_Weight   );
	myProject->SetInputParam("IsMCatNLO",     G_IsMCatNLO      );  
	myProject->SetInputParam("iSelection",    sel              );
	myProject->SetInputParam("WorkingDir",    WorkingDir       );

	// EXTRA PARAMETERS
	myProject->SetInputParam("IsFastSim"    , G_IsFastSim);
	myProject->SetInputParam("stopMass"     , stopMass         );
	myProject->SetInputParam("lspMass"      , lspMass          );
	myProject->SetInputParam("doSyst"       , G_DoSystematics  ); 


	// Name of analysis class
	//----------------------------------------------------------------------------
	myProject->AddSelectorPackage("LeptonSelector");
	myProject->AddSelectorPackage("JetSelector");
	myProject->AddSelectorPackage("EventBuilder");
	if      (sel == iStopSelec)  myProject->AddSelectorPackage("StopAnalysis");
	else if (sel == iTopSelec )  myProject->AddSelectorPackage("TopAnalysis");
	else if (sel == iWWSelec  )  myProject->AddSelectorPackage("WWAnalysis");
	else                         myProject->AddSelectorPackage("CreateMiniTree");

	// Additional packages
	//----------------------------------------------------------------------------
	myProject->AddPackage("Lepton");
	myProject->AddPackage("Jet");
	myProject->AddPackage("mt2");
	myProject->AddPackage("Functions");
	myProject->AddPackage("LeptonSF");
	myProject->AddPackage("BTagSFUtil");
	myProject->AddPackage("PUWeight");

	// Let's rock!
	//----------------------------------------------------------------------------
	myProject->Run();
}

Bool_t IsMCatNLO(TString sampleName){
	if(	sampleName.Contains("amcatnlo")  ||
			sampleName.Contains("aMCatNLO")  || 
			sampleName.BeginsWith("TTW") || 
			sampleName.BeginsWith("TTZ") ||  
			sampleName.BeginsWith("WWZ") || 
			sampleName.BeginsWith("WZZ") || 
			sampleName.BeginsWith("ZZZ") )
		return true;
	else return false;
}

void GetCount(std::vector<TString> Files, Bool_t IsData){
	Int_t nFiles = Files.size(); TFile *f;
	TH1D *hcount; TH1D *hsum; TTree* tree;
	if(verbose) cout << "\033[1;30m=================================================\033[0m\n";
	for(Int_t i = 0; i < nFiles; i++){
		if(verbose) cout << Form("\033[1;32m >>> Including file: %s \033[0m\n", Files.at(i).Data());
		//cout << "Including file: " << Files.at(i) << endl;
		f = TFile::Open(Files.at(i));
    f->GetObject("tree", tree);
		f->GetObject("Count", hcount);
		Count        += hcount-> GetEntries();
    nTrueEntries += tree  -> GetEntries();
		if(!IsData){ 
			f->GetObject("SumGenWeights", hsum);
			SumOfWeights += hsum  -> GetBinContent(1);
		}
		f->Close();    
	}
}

Float_t GetSMSnorm(Int_t mStop, Int_t mLsp){
  cout << Form("\033[1;36m >>> Searching for normalization factor for stop point with masses [%i, %i]...\033[0m\n", mStop, mLsp);
	Int_t nFiles = Files.size(); TFile *f;
	TH3D *hcount; Float_t val = 0; Float_t ms = 0; Float_t mn = 0;
  Float_t count = 0;
	for(Int_t k = 0; k < nFiles; k++){
    f = TFile::Open(Files.at(k));
    f -> GetObject("CountSMS", hcount);
		Int_t nx = hcount->GetNbinsX();
		Int_t ny = hcount->GetNbinsY();
		for(Int_t i = 0; i < nx; i++){
			for(Int_t j = 0; j < ny; j++){
        val = hcount->GetBinContent(i,j,1);
        if(val != 0){
          ms = hcount->GetXaxis()->GetBinCenter(i);
          mn = hcount->GetYaxis()->GetBinCenter(j);
          if(ms == mStop && mLsp == mn) count += hcount->GetBinContent(hcount->FindBin(ms, mn, 0));
        } 
			}
		}
	}
  return count;
}

Double_t GetStopXSec(Int_t StopMass){
  if      (StopMass == 125) return 574.981;
  else if (StopMass == 150) return 249.409;
  else if (StopMass == 175) return 121.416;
  else if (StopMass == 200) return 64.5085;
  else if (StopMass == 225) return 36.3818;
  else if (StopMass == 250) return 21.5949;
  else if (StopMass == 275) return 13.3231;
  else if (StopMass == 300) return 8.51615;
  else if (StopMass == 325) return 5.60471;
  else if (StopMass == 350) return 3.78661;
  else if (StopMass == 375) return 2.61162;
  else if (StopMass == 400) return 1.83537;
  else if (StopMass == 425) return 1.31169;
  else if (StopMass == 450) return 0.948333;
  else if (StopMass == 475) return 0.697075;
  else if (StopMass == 500) return 0.51848;
  else if (StopMass == 525) return 0.390303;
  else if (StopMass == 550) return 0.296128;
  else if (StopMass == 575) return 0.226118;
  else if (StopMass == 600) return 0.174599;
  else if (StopMass == 625) return 0.136372;
  else if (StopMass == 650) return 0.107045;
  else if (StopMass == 675) return 0.0844877;
  else if (StopMass == 700) return 0.0670476;
  else if (StopMass == 725) return 0.0536438;
  else if (StopMass == 750) return 0.0431418;
  else if (StopMass == 775) return 0.0348796;
  else if (StopMass == 800) return 0.0283338;
  else if (StopMass == 825) return 0.0241099;
  else if (StopMass == 850) return 0.0189612;
  else if (StopMass == 875) return 0.015625;
  else if (StopMass == 900) return 0.0128895;
  else if (StopMass == 925) return 0.0106631;
	else if (StopMass == 950) return 0.00883465;
	else if (StopMass == 975) return 0.00735655;
	else{ 
		cout << Form(" >>> No Cross Section for that mass!! (mStop = %i) Extrapolating...\n", StopMass);
		Float_t v0 = GetStopXSec(StopMass - StopMass%25);
		Float_t vf = GetStopXSec(StopMass - StopMass%25 + 25);
		Float_t x  = float(StopMass%25)/25;
		return v0 + (vf-v0)*x;
	}
}

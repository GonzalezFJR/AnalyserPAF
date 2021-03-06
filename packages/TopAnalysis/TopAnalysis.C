#include "TopAnalysis.h"

ClassImp(TopAnalysis);
TopAnalysis::TopAnalysis() : PAFChainItemSelector() {
	fTree = 0;
  fhDummy = 0;
	for(Int_t ch = 0; ch < nChannels; ch++){
		for(Int_t cut = 0; cut < nLevels; cut++){
			for(Int_t sys = 1; sys < nSysts; sys++){
				fHLHEweights[ch][cut][sys]  = 0;
				fHMET[ch][cut][sys]         = 0;
				fHLep0Eta[ch][cut][sys]     = 0;
				fHLep1Eta[ch][cut][sys]     = 0;
				fHDelLepPhi[ch][cut][sys]   = 0;
				fHHT[ch][cut][sys]          = 0;
				fHJet0Eta[ch][cut][sys]     = 0;
				fHJet1Eta[ch][cut][sys]     = 0;
				fHInvMass[ch][cut][sys]       = 0;
				fHInvMass2[ch][cut][sys]      = 0;
				fHNBtagsNJets[ch][cut][sys]   = 0;
				fHNJets[ch][cut][sys]        = 0;
				fHNBtagJets[ch][cut][sys]    = 0;
				fHJet0Pt[ch][cut][sys]       = 0;
				fHJet1Pt[ch][cut][sys]       = 0;
				fHDiLepPt[ch][cut][sys]      = 0;
				fHLep0Pt[ch][cut][sys]       = 0;
				fHLep1Pt[ch][cut][sys]       = 0;
				fHJetCSV[ch][cut][sys]       = 0;
				fHvertices[ch][cut][sys]     = 0;
				if(cut == 0){
					fHyields[ch][sys]     = 0;
					fHSSyields[ch][sys]   = 0;
				}
			}
		}
	}
}
void TopAnalysis::Summary(){}

void TopAnalysis::Initialise(){
	gIsData      = GetParam<Bool_t>("IsData");
	gSelection   = GetParam<Int_t>("iSelection");
  gSampleName  = GetParam<TString>("sampleName");
  gDoSyst      = GetParam<Bool_t>("doSyst");

  if(makeTree){
		fTree = CreateTree("MiniTree","Created with PAF");
		SetLeptonVariables();
		SetJetVariables();
		SetEventVariables();
	}

  InitHistos();

  genLeptons  = std::vector<Lepton>();
  selLeptons  = std::vector<Lepton>();
  vetoLeptons = std::vector<Lepton>();
  selJets = std::vector<Jet>();
  Jets15  = std::vector<Jet>();
}

void TopAnalysis::InsideLoop(){
	// Vectors with the objects
	genLeptons  = GetParam<vector<Lepton>>("genLeptons");
	selLeptons  = GetParam<vector<Lepton>>("selLeptons");
	vetoLeptons = GetParam<vector<Lepton>>("vetoLeptons");
	selJets     = GetParam<vector<Jet>>("selJets");
	Jets15      = GetParam<vector<Jet>>("Jets15");

	// Weights and SFs
	NormWeight = GetParam<Float_t>("NormWeight");
	TrigSF       = GetParam<Float_t>("TriggerSF");
	TrigSF_Up    = GetParam<Float_t>("TriggerSF_Up");
	TrigSF_Down  = GetParam<Float_t>("TriggerSF_Down");
	PUSF         = GetParam<Float_t>("PUSF");
	PUSF_Up      = GetParam<Float_t>("PUSF_Up");
	PUSF_Down    = GetParam<Float_t>("PUSF_Down");
  TrigSF = 1; TrigSF_Up = 1; TrigSF_Down = 1;
  PUSF = 1; PUSF_Up = 1; PUSF_Down = 1;

	// Event variables
	gChannel        = GetParam<Int_t>("gChannel");
	passMETfilters = GetParam<Bool_t>("METfilters");
	passTrigger    = GetParam<Bool_t>("passTrigger");
	isSS           = GetParam<Bool_t>("isSS");

	// Leptons and Jets
	GetLeptonVariables(selLeptons, vetoLeptons);
	GetJetVariables(selJets, Jets15);
	GetMET();

  fhDummy->Fill(1);

	if(genLeptons.size() >=2 && TNSelLeps >= 2 && passTrigger && passMETfilters){ // dilepton event, 2 leptons // && !isSS
		// Deal with weights:
		Float_t lepSF   = selLeptons.at(0).GetSF( 0)*selLeptons.at(1).GetSF( 0);
		Float_t lepSFUp = selLeptons.at(0).GetSF( 1)*selLeptons.at(1).GetSF( 1);
		Float_t lepSFDo = selLeptons.at(0).GetSF(-1)*selLeptons.at(1).GetSF(-1);
		TWeight            = NormWeight*lepSF*TrigSF*PUSF;
		TWeight_LepEffUp   = NormWeight*lepSFUp*TrigSF*PUSF;
		TWeight_LepEffDown = NormWeight*lepSFDo*TrigSF*PUSF;
		TWeight_TrigUp     = NormWeight*lepSF*TrigSF_Up*PUSF;
		TWeight_TrigDown   = NormWeight*lepSF*TrigSF_Down*PUSF;
		TWeight_PUDown     = NormWeight*lepSF*TrigSF*PUSF_Up;
		TWeight_PUUp       = NormWeight*lepSF*TrigSF*PUSF_Down;

		// Event Selection
		// ===================================================================================================================
		if((selLeptons.at(0).p + selLeptons.at(1).p).M() > 20 && selLeptons.at(0).p.Pt() > 25){ // mll > 20 GeV, dilepton, leading lepton pT > 25 GeV
			if(isSS) fHSSyields[gChannel][0] -> Fill(idilepton, TWeight);
			else{      fHyields[gChannel][0] -> Fill(idilepton, TWeight);
				FillHistos(gChannel, idilepton);}
			if(gChannel == 1 || (TMath::Abs((selLeptons.at(0).p + selLeptons.at(1).p).M() - 91) > 15)  ){ //  Z Veto in ee, µµ
				if(TNJets > 1){ //At least 2 jets
					if(isSS) fHSSyields[gChannel][0] -> Fill(i2jets, TWeight);
					else{      fHyields[gChannel][0] -> Fill(i2jets, TWeight);
						FillHistos(gChannel, i2jets); }
					if(TNBtags > 0 || TNBtagsUp > 0 || TNBtagsDown > 0 || TNBtagsMisTagUp > 0 || TNBtagsMisTagDown > 0){ // At least 1 b-tag
						if(isSS) fHSSyields[gChannel][0] -> Fill(i1btag, TWeight);
						else{      fHyields[gChannel][0] -> Fill(i1btag, TWeight);
							FillHistos(gChannel, i1btag); }
					}
				}
			}
		}
	}
}


//#####################################################################
// Functions
//------------------------------------------------------------------
void TopAnalysis::GetLeptonVariables(std::vector<Lepton> selLeptons, std::vector<Lepton> VetoLeptons){
  TNSelLeps = selLeptons.size();
  Int_t nVetoLeptons = VetoLeptons.size();
  TNVetoLeps = (nVetoLeptons == 0) ? TNSelLeps : nVetoLeptons;
  for(Int_t i = 0; i < TNSelLeps; i++){
    TLep_Pt[i]     = selLeptons.at(i).Pt();    
    TLep_Eta[i]    = selLeptons.at(i).Eta();    
    TLep_Phi[i]    = selLeptons.at(i).Phi();    
    TLep_E[i]      = selLeptons.at(i).E();    
    TLep_Charge[i] = selLeptons.at(i).charge;    
  }
  if(TNSelLeps < 2) gChannel = -1;
  else if(selLeptons.at(0).isMuon && selLeptons.at(1).isElec) gChannel = 1;
  else if(selLeptons.at(0).isElec && selLeptons.at(1).isMuon) gChannel = 1;
  else if(selLeptons.at(0).isMuon && selLeptons.at(1).isMuon) gChannel = 2;
  else if(selLeptons.at(0).isElec && selLeptons.at(1).isElec) gChannel = 3;
  if(TNSelLeps > 1) TMll = (selLeptons.at(0).p + selLeptons.at(1).p).M();      
}

void TopAnalysis::GetJetVariables(std::vector<Jet> selJets, std::vector<Jet> cleanedJets15, Float_t ptCut){
  TNJets = selJets.size(); THT = 0;
  TNBtags = 0; TNBtagsUp = 0; TNBtagsDown = 0;
  TNBtagsMisTagUp = 0;  TNBtagsMisTagDown = 0;
  for(Int_t i = 0; i < TNJets; i++){
    TJet_Pt[i]     = selJets.at(i).Pt();
    TJet_Eta[i]    = selJets.at(i).Eta();
    TJet_Phi[i]    = selJets.at(i).Phi();
    TJet_E[i]      = selJets.at(i).E();
    TJet_isBJet[i] = selJets.at(i).isBtag;
    THT += TJet_Pt[i];
    if(selJets.at(i).isBtag)            TNBtags++;
    if(selJets.at(i).isBtag_BtagUp    ) TNBtagsUp++;
    if(selJets.at(i).isBtag_BtagDown  ) TNBtagsDown++;
    if(selJets.at(i).isBtag_MisTagUp  ) TNBtagsMisTagUp++;
    if(selJets.at(i).isBtag_MisTagDown) TNBtagsMisTagDown++;
  }
  // For systematics...
  TNJetsJESUp    = 0;
  TNJetsJESDown  = 0;
  TNJetsJER      = 0;  
  THTJESUp = 0; THTJESDown = 0;
  for(Int_t i = 0; i < (Int_t) cleanedJets15.size(); i++){
    if(cleanedJets15.at(i).pTJESUp > ptCut){
      TNJetsJESUp++;
      TJetJESUp_Pt[i] = cleanedJets15.at(i).pTJESUp;
      THTJESUp += TJetJESUp_Pt[i];
    }
    if(cleanedJets15.at(i).pTJESDown > ptCut){
      TNJetsJESDown++;
      TJetJESDown_Pt[i] = cleanedJets15.at(i).pTJESDown;
      THTJESDown += TJetJESUp_Pt[i];
    }
    if(cleanedJets15.at(i).pTJERUp > ptCut){
      TNJetsJER++;
      TJetJER_Pt[i] = cleanedJets15.at(i).pTJERUp;
    }
  }
}

void TopAnalysis::GetMET(){
		TMET        = Get<Float_t>("met_pt");
		TMET_Phi    = Get<Float_t>("met_phi");  // MET phi
    TMETJESUp   = Get<Float_t>("met_jecUp_pt"  );
    TMETJESDown = Get<Float_t>("met_jecDown_pt");
}

void TopAnalysis::InitHistos(){
  fhDummy = CreateH1F("fhDummy", "fhDummy", 1, 0, 2);
	for(Int_t ch = 0; ch < nChannels; ch++){
    fHyields[ch][0]     = CreateH1F("H_Yields_"+gChanLabel[ch],"", nLevels, -0.5, nLevels-0.5);
    fHSSyields[ch][0]   = CreateH1F("H_SSYields_"+gChanLabel[ch],"", nLevels, -0.5, nLevels-0.5);
  }
  if(!makeHistos) return;
	for(Int_t ch = 0; ch < nChannels; ch++){
		for(Int_t cut = 0; cut < nLevels; cut++){
			fHLHEweights[ch][cut][0]  = CreateH1F("H_LHEweights"  +gChanLabel[ch]+"_"+sCut[cut],"LHEweights", nWeights, -0.5, nWeights - 0.5);
			fHMET[ch][cut][0]         = CreateH1F("H_MET_"        +gChanLabel[ch]+"_"+sCut[cut],"MET"       , 3000, 0,300);
			fHLep0Eta[ch][cut][0]     = CreateH1F("H_Lep0Eta_"    +gChanLabel[ch]+"_"+sCut[cut],"Lep0Eta"   , 50  ,0 ,2.5);
			fHLep1Eta[ch][cut][0]     = CreateH1F("H_Lep1Eta_"    +gChanLabel[ch]+"_"+sCut[cut],"Lep1Eta"   , 50  ,0 ,2.5);
			fHDelLepPhi[ch][cut][0]   = CreateH1F("H_DelLepPhi_"  +gChanLabel[ch]+"_"+sCut[cut],"DelLepPhi" , 100,-3.2, 3.2);
			fHHT[ch][cut][0]          = CreateH1F("H_HT_"         +gChanLabel[ch]+"_"+sCut[cut],"HT"        , 4700,30,500);
			fHJet0Eta[ch][cut][0]     = CreateH1F("H_Jet0Eta_"  +gChanLabel[ch]+"_"+sCut[cut],"Jet0Eta"   , 50,0,2.5);
			fHJet1Eta[ch][cut][0]     = CreateH1F("H_Jet1Eta_"  +gChanLabel[ch]+"_"+sCut[cut],"Jet1Eta"   , 50,0,2.5);
			fHInvMass[ch][cut][0]       = CreateH1F("H_InvMass_"    +gChanLabel[ch]+"_"+sCut[cut],"InvMass"   ,  300,  0., 300.);
			fHInvMass2[ch][cut][0]      = CreateH1F("H_InvMass2_"   +gChanLabel[ch]+"_"+sCut[cut],"InvMass2"  ,  400, 70., 110.);
			fHNBtagsNJets[ch][cut][0]   = CreateH1F("H_NBtagsNJets_"+gChanLabel[ch]+"_"+sCut[cut]  ,"NBtagsNJets"   ,15 , -0.5, 14.5);
			fHNJets[ch][cut][0]       = CreateH1F("H_NJets_"      +gChanLabel[ch]+"_"+sCut[cut],"NJets"     , 8 ,-0.5, 7.5);
			fHNBtagJets[ch][cut][0]   = CreateH1F("H_NBtagJets_"  +gChanLabel[ch]+"_"+sCut[cut],"NBtagJets" , 4 ,-0.5, 3.5);
			fHJet0Pt[ch][cut][0]      = CreateH1F("H_Jet0Pt_"     +gChanLabel[ch]+"_"+sCut[cut],"Jet0Pt"    , 2700,30,300);
			fHJet1Pt[ch][cut][0]      = CreateH1F("H_Jet1Pt_"     +gChanLabel[ch]+"_"+sCut[cut],"Jet1Pt"    , 2200,30,250);
			fHDiLepPt[ch][cut][0]     = CreateH1F("H_DiLepPt_"    +gChanLabel[ch]+"_"+sCut[cut],"DiLepPt"   , 1600,20,180);
			fHLep0Pt[ch][cut][0]      = CreateH1F("H_Lep0Pt_"     +gChanLabel[ch]+"_"+sCut[cut],"Lep0Pt"    , 1800,20,200);
			fHLep1Pt[ch][cut][0]      = CreateH1F("H_Lep1Pt_"     +gChanLabel[ch]+"_"+sCut[cut],"Lep1Pt"    , 1800,20,200);
			fHJetCSV[ch][cut][0]  = CreateH1F("H_JetCSV_" +gChanLabel[ch]+"_"+sCut[cut],"CSV" , 100,0, 1.3);
			fHvertices[ch][cut][0]     = CreateH1F("H_Vtx_"+gChanLabel[ch]+"_"+sCut[cut],"", 51, -0.5, 50.5); 
		}
	}
	for(Int_t ch = 0; ch < nChannels; ch++){
		for(Int_t cut = 0; cut < nLevels; cut++){
			for(Int_t sys = 1; sys < nSysts; sys++){
				fHLHEweights[ch][cut][sys]  = CreateH1F("H_LHEweights"  +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"LHEweights", nWeights, -0.5, nWeights - 0.5);
				fHMET[ch][cut][sys]         = CreateH1F("H_MET_"        +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"MET"       , 3000, 0,300);
				fHLep0Eta[ch][cut][sys]     = CreateH1F("H_Lep0Eta_"    +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Lep0Eta"   , 50  ,0 ,2.5);
				fHLep1Eta[ch][cut][sys]     = CreateH1F("H_Lep1Eta_"    +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Lep1Eta"   , 50  ,0 ,2.5);
				fHDelLepPhi[ch][cut][sys]   = CreateH1F("H_DelLepPhi_"  +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"DelLepPhi" , 100,-3.2, 3.2);
				fHHT[ch][cut][sys]          = CreateH1F("H_HT_"         +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"HT"        , 4700,30,500);
				fHJet0Eta[ch][cut][sys]     = CreateH1F("H_Jet0Eta_"  +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Jet0Eta"   , 50,0,2.5);
				fHJet1Eta[ch][cut][sys]     = CreateH1F("H_Jet1Eta_"  +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Jet1Eta"   , 50,0,2.5);
				fHInvMass[ch][cut][sys]       = CreateH1F("H_InvMass_"    +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"InvMass"   ,  300,  0., 300.);
				fHInvMass2[ch][cut][sys]      = CreateH1F("H_InvMass2_"   +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"InvMass2"  ,  400, 70., 110.);
				fHNBtagsNJets[ch][cut][sys]   = CreateH1F("H_NBtagsNJets_"+gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys]  ,"NBtagsNJets"   ,15 , -0.5, 14.5);
				fHNJets[ch][cut][sys]        = CreateH1F("H_NJets_"      +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"NJets"     , 8 ,-0.5, 7.5);
				fHNBtagJets[ch][cut][sys]    = CreateH1F("H_NBtagJets_"  +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"NBtagJets" , 4 ,-0.5, 3.5);
				fHJet0Pt[ch][cut][sys]       = CreateH1F("H_Jet0Pt_"     +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Jet0Pt"    , 2700,30,300);
				fHJet1Pt[ch][cut][sys]       = CreateH1F("H_Jet1Pt_"     +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Jet1Pt"    , 2200,30,250);
				fHDiLepPt[ch][cut][sys]      = CreateH1F("H_DiLepPt_"    +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"DiLepPt"   , 1600,20,180);
				fHLep0Pt[ch][cut][sys]       = CreateH1F("H_Lep0Pt_"     +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Lep0Pt"    , 1800,20,200);
				fHLep1Pt[ch][cut][sys]       = CreateH1F("H_Lep1Pt_"     +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"Lep1Pt"    , 1800,20,200);
				fHJetCSV[ch][cut][sys]       = CreateH1F("H_JetCSV_" +gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"CSV" , 100,0, 1.3);
				fHvertices[ch][cut][sys]     = CreateH1F("H_Vtx_"+gChanLabel[ch]+"_"+sCut[cut]+"_"+gSys[sys],"", 51, -0.5, 50.5); 
			}
		}
	}
}

void TopAnalysis::FillHistos(Int_t ch, Int_t cut){
  if(!makeHistos) return;
	Int_t sys = 0;
  Float_t EventWeight = TWeight;

	fHMET[ch][cut][sys]         -> Fill(TMET, EventWeight);
	fHLep0Eta[ch][cut][sys]     -> Fill(TLep_Eta[0], EventWeight);
	fHLep1Eta[ch][cut][sys]     -> Fill(TLep_Eta[1], EventWeight);
	fHLep0Pt[ch][cut][sys]      -> Fill(TLep_Pt[0], EventWeight);
	fHLep1Pt[ch][cut][sys]      -> Fill(TLep_Pt[1], EventWeight);
	fHDiLepPt[ch][cut][sys]      -> Fill((selLeptons[0].p + selLeptons[1].p).Pt(), EventWeight);
	fHDelLepPhi[ch][cut][sys]   -> Fill(selLeptons[0].p.DeltaPhi(selLeptons[1].p), EventWeight);
	fHHT[ch][cut][sys]          -> Fill(THT, EventWeight);
  if(TNJets > 0) fHJet0Eta[ch][cut][sys]     -> Fill(TJet_Eta[0], EventWeight);
  if(TNJets > 0) fHJet0Pt [ch][cut][sys]     -> Fill(TJet_Pt[0], EventWeight);
	if(TNJets > 1) fHJet1Eta[ch][cut][sys]     -> Fill(TJet_Eta[1], EventWeight);
	if(TNJets > 1) fHJet1Pt [ch][cut][sys]     -> Fill(TJet_Pt[1], EventWeight);
	fHInvMass[ch][cut][sys]       -> Fill(TMll, EventWeight);
	fHInvMass2[ch][cut][sys]      -> Fill(TMll, EventWeight);
	fHNJets[ch][cut][sys]        -> Fill(TNJets, EventWeight);
	fHNBtagJets[ch][cut][sys]    -> Fill(TNBtags, EventWeight);
	for(Int_t i = 0; i < (Int_t) Jets15.size(); i++){ 
		fHJetCSV[ch][cut][sys]       -> Fill(Jets15.at(i).csv, EventWeight);
	}
	//fHvertices[ch][cut][sys]      -> Fill(Get<Int_t>("nVert"), EventWeight);
	fHNBtagsNJets[ch][cut][sys]   -> Fill(0., EventWeight);
}

void TopAnalysis::SetLeptonVariables(){
	fTree->Branch("TNVetoLeps",     &TNVetoLeps,     "TNVetoLeps/I");
	fTree->Branch("TNSelLeps",     &TNSelLeps,     "TNSelLeps/I");
	fTree->Branch("TLep_Pt",     TLep_Pt,     "TLep_Pt[TNSelLeps]/F");
  fTree->Branch("TLep_Eta",     TLep_Eta,     "TLep_Eta[TNSelLeps]/F");
  fTree->Branch("TLep_Phi",     TLep_Phi,     "TLep_Phi[TNSelLeps]/F");
  fTree->Branch("TLep_E" ,     TLep_E ,     "TLep_E[TNSelLeps]/F");
  fTree->Branch("TLep_Charge",  TLep_Charge, "TLep_Charge[TNSelLeps]/F");
  fTree->Branch("TChannel",      &TChannel,      "TChannel/I");
}

void TopAnalysis::SetJetVariables(){
  fTree->Branch("TNJets",           &TNJets,         "TNJets/I");
  fTree->Branch("TNBtags",       &TNBtags,     "TNBtags/I");
  fTree->Branch("TJet_isBJet",       TJet_isBJet,       "TJet_isBJet[TNJets]/I");
  fTree->Branch("TJet_Pt",           TJet_Pt,           "TJet_Pt[TNJets]/F");
  fTree->Branch("TJet_Eta",           TJet_Eta,           "TJet_Eta[TNJets]/F");
  fTree->Branch("TJet_Phi",           TJet_Phi,           "TJet_Phi[TNJets]/F");
  fTree->Branch("TJet_E",            TJet_E,            "TJet_E[TNJets]/F");

  fTree->Branch("TNJetsJESUp",           &TNJetsJESUp,         "TNJetsJESUp/I");
  fTree->Branch("TNJetsJESDown",           &TNJetsJESDown,         "TNJetsJESDown/I");
  fTree->Branch("TNJetsJER",           &TNJetsJER,         "TNJetsJER/I");

  fTree->Branch("TNBtagsUp",     &TNBtagsUp,   "TNBtagsUp/I");
  fTree->Branch("TNBtagsDown",   &TNBtagsDown, "TNBtagsDown/I");
  fTree->Branch("TNBtagsMisTagUp",     &TNBtagsMisTagUp,   "TNBtagsMisTagUp/I");
  fTree->Branch("TNBtagsMisTagDown",   &TNBtagsMisTagDown, "TNBtagsMisTagDown/I");

  fTree->Branch("TJetJESUp_Pt",      TJetJESUp_Pt,      "TJetJESUp_Pt[TNJetsJESUp]/F");
  fTree->Branch("TJetJESDown_Pt",    TJetJESDown_Pt,    "TJetJESDown_Pt[TNJetsJESDown]/F");
  fTree->Branch("TJetJER_Pt",        TJetJER_Pt,        "TJetJER_Pt[TNJetsJER]/F");

  fTree->Branch("THT",          &THT,          "THT/F");
  fTree->Branch("THTJESUp",     &THTJESUp,     "THTJESUp/F");
  fTree->Branch("THTJESDown",   &THTJESDown,   "THTJESDown/F");
}

void TopAnalysis::SetEventVariables(){
  fTree->Branch("TWeight",      &TWeight,      "TWeight/F");
  fTree->Branch("TWeight_LepEffUp",      &TWeight_LepEffUp,      "TWeight_LepEffUp/F");
  fTree->Branch("TWeight_LepEffDown",    &TWeight_LepEffDown,    "TWeight_LepEffDown/F");
  fTree->Branch("TWeight_TrigUp",        &TWeight_TrigUp,        "TWeight_TrigUp/F");
  fTree->Branch("TWeight_TrigDown",      &TWeight_TrigDown,      "TWeight_TrigDown/F");
  fTree->Branch("TWeight_PUUp",        &TWeight_PUUp,        "TWeight_PUUp/F");
  fTree->Branch("TWeight_PUDown",        &TWeight_PUDown,        "TWeight_PUDown/F");

  fTree->Branch("TMET",         &TMET,         "TMET/F");
  fTree->Branch("TMET_Phi",     &TMET_Phi,     "TMET_Phi/F");
  fTree->Branch("TMETJESUp",    &TMETJESUp,    "TMETJESUp/F");
  fTree->Branch("TMETJESDown",  &TMETJESDown,  "TMETJESDown/F");
}

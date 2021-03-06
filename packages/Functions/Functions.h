#ifndef FUNCTIONS_H
#define FUNCTIONS_H 1

#include "Lepton.h"
#include "Jet.h"
#include "TTree.h"
#include <iostream>
#include <vector>

using namespace std;

// enum 
enum iSFs{
	iMuonReco, iMuonId, iMuonIdSUSY, iMuonIsoTightId, iMuonIsoMediumId, iMuonIsoSUSY, iMuonIP2D, iMuonSIP3D,
	iElecReco, iElecId, iElecIdSUSY, iElecIso, iElecIsoSUSY, iElecIP2D, iElecSIP3D,
	iTrigDoubleMuon, iTrigDoubleElec, iTrigElMu,
	nfiles
};

enum wps{iVeto, iLoose, iMedium, iTight, iVeryTight, iExtreamlyTight, iWPforStop};

enum sel{iStopSelec, iTopSelec, iTWSelec, iWWSelec, ittDMSelec, ittHSelec};

enum sys{iNom, 
	iJesUp, iJesDown, iJERUp, iJERDown, 
	iBtagUp, iBtagDown, iBtagMisUp, iBtagMisDown, 
	iLepEffUp, iLepEffDown, 
	iTrigUp, iTrigDown, 
	iPUUp, iPUDown, 
	iFSUp, iFSDown,
	nSys
};

//enum eChannel{iElMu, iElec, iMuon, nChannels};

bool LepMomentumComparator(Lepton i, Lepton j);
bool JetMomentumComparator(Jet i   , Jet    j);

vector<Lepton> SortLeptonsByPt(vector<Lepton>& Leptons);
vector<Jet>       SortJetsByPt(vector<Jet>   & Jets);

Float_t JEStoMET(vector<Jet> vjets, Float_t met, Float_t met_phi, Int_t dir = 0);
Float_t getJetJERpt(Jet jet);
Float_t p2(Float_t x);

Float_t getDeltaPhillJet(Lepton lep1, Lepton lep2, Jet jet);
Float_t getDPhiClosestJet(vector<Jet> vJet, TLorentzVector lep);
Float_t getHT(vector<Jet> jet);
Float_t getMT(TLorentzVector v1, TLorentzVector v2);
Float_t getMinDPhiMetJets(vector<Jet> vjets, Float_t met, Float_t met_phi);
Float_t getDelPhi(TLorentzVector v1, TLorentzVector v2);
Float_t getDelEta(TLorentzVector v1, TLorentzVector v2);
Float_t getMT2(TLorentzVector plep1, TLorentzVector plep2, TLorentzVector pmet, Float_t mass);
Float_t getMT2ll(Lepton l1, Lepton l2, Float_t met, Float_t met_phi);
Float_t getMeff(Lepton l1, Lepton l2, vector<Jet> vjets, Float_t met);
TLorentzVector getPtllb(Lepton l1, Lepton l2, Float_t met, Float_t met_phi);

#endif

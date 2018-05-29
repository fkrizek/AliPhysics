
//
// Calculate flow in the forward and central regions using the Q cumulants method.
//
// Inputs:
//  - AliAODEvent
//
// Outputs:
//  - AnalysisResults.root or forward_flow.root
//
#include <iostream>
#include <TROOT.h>
#include <TSystem.h>
#include <TInterpreter.h>
#include <TList.h>
#include <THn.h>

#include "AliLog.h"
#include "AliForwardNUATask.h"
#include "AliForwardQCumulantRun2.h"
#include "AliForwardGenericFramework.h"

#include "AliAODForwardMult.h"
#include "AliAODCentralMult.h"
#include "AliAODEvent.h"

#include "AliForwardUtil.h"

#include "AliVVZERO.h"
#include "AliAODVertex.h"
#include "AliCentrality.h"

#include "AliESDEvent.h"
#include "AliVTrack.h"
#include "AliESDtrack.h"
#include "AliAODTrack.h"
#include "AliAODTracklets.h"

#include "AliAnalysisFilter.h"
#include "AliMultSelection.h"
#include "AliMultiplicity.h"
#include "AliAnalysisManager.h"
#include "AliInputEventHandler.h"
#include "TMath.h"

using namespace std;
ClassImp(AliForwardNUATask)
#if 0
; // For emacs 
#endif

//_____________________________________________________________________
AliForwardNUATask::AliForwardNUATask() : AliAnalysisTaskSE(),
  fAOD(0),           // input event
  fOutputList(0),    // output list
  fEventList(0),
  fSettings(),
  fEventCuts(),
  fMultTOFLowCut(),
  fMultTOFHighCut(),
  fMultCentLowCut()
  {
  //
  //  Default constructor
  //
  }

  //_____________________________________________________________________
  AliForwardNUATask::AliForwardNUATask(const char* name) : AliAnalysisTaskSE(name),
  fAOD(0),           // input event
  fOutputList(0),
  fEventList(0),
  fSettings(),
  fEventCuts(),
  fMultTOFLowCut(),
  fMultTOFHighCut(),
  fMultCentLowCut()
  {
  // 
  //  Constructor
  //
  //  Parameters:
  //   name: Name of task
  //
    DefineOutput(1, TList::Class());
  }

//_____________________________________________________________________
  void AliForwardNUATask::UserCreateOutputObjects()
  {
  //
  //  Create output objects
  //
    fOutputList = new TList();          // the final output list

    fOutputList->SetOwner(kTRUE);       // memory stuff: the list is owner of all objects it contains and will delete them if requested
    

    //..adding QA plots from AliEventCuts class
  fEventCuts.AddQAplotsToList(fOutputList);

    fEventList = new TList();

    fEventList->Add(new TH1D("Centrality","Centrality",100,0,100));
    fEventList->Add(new TH1D("Vertex","Vertex",fSettings.fNZvtxBins,fSettings.fZVtxAcceptanceLowEdge,fSettings.fZVtxAcceptanceUpEdge));
    fEventList->Add(new TH2F("hOutliers","Maximum #sigma from mean N_{ch} pr. bin", 
     20, 0., 100., 500, 0., 5.)); //((fFlags & kMC) ? 15. : 5. // Sigma <M> histogram 
    fEventList->Add(new TH1D("FMDHits","FMDHits",100,0,10));
    fEventList->Add(new TH2D("hybrid","hybrid",200,-4.0,6.0, 200, 0, 2*TMath::Pi()));

    fEventList->SetName("EventInfo");

    fOutputList->Add(new TH3F("NUAforward","NUAforward", 200, -4.0, 6.0, 20, 0., 2*TMath::Pi(),20,-10,10));
    fOutputList->Add(new TH3F("NUAhist","NUAhist", 200, -4.0, 6.0, 20, 0., 2*TMath::Pi(),20,-10,10));
    //fOutputList->Add(fEventList);
    fOutputList->Add(new TH3F("NUAcentral","NUAcentral", 400, -1.5, 1.5, 400, 0., 2*TMath::Pi(),20,-10,10));
    fOutputList->Add(fEventList);

    PostData(1, fOutputList);
  }


//_____________________________________________________________________
void AliForwardNUATask::UserExec(Option_t */*option*/)
{
  //
  //  Analyses the event with use of the helper class AliForwardQCumulantRun2
  //
  //  Parameters:
  //   option: Not used
  //

  fAOD = dynamic_cast<AliAODEvent*>(InputEvent());
  
  //..check if I have AOD
  fAOD = dynamic_cast<AliAODEvent*>(InputEvent());
  if(!fAOD){
    Printf("%s:%d AODEvent not found in Input Manager",(char*)__FILE__,__LINE__);
    return;
  }
  
  //..AliEventCuts selection
  if(!fEventCuts.AcceptEvent(fInputEvent)) {
    PostData(1, fOutputList);
    return;
  }  

  //..get variables for additional event selection cuts (from Alex)
  float v0Centr = 0;


  AliMultSelection *MultSelection = (AliMultSelection*)fInputEvent->FindListObject("MultSelection");
  v0Centr = MultSelection->GetMultiplicityPercentile("SPDTracklets");

  // Get detector objects
  AliAODForwardMult* aodfmult = static_cast<AliAODForwardMult*>(fAOD->FindListObject("Forward"));

  //AliAODCentralMult* aodcmult = static_cast<AliAODCentralMult*>(fAOD->FindListObject("CentralClusters")); // only exists if created by user from ESDs
  TH2D spddNdedp = TH2D("spddNdedp","spddNdedp",400,-4.0,6.0,400,0,2*TMath::Pi()); // Histogram to contain the central tracks

  TList* eventList = static_cast<TList*>(fOutputList->FindObject("EventInfo"));
  TH3F* nuahist = static_cast<TH3F*>(fOutputList->FindObject("NUAhist"));
  TH3F* nuaforward = static_cast<TH3F*>(fOutputList->FindObject("NUAforward"));
  TH3F* nuacentral = static_cast<TH3F*>(fOutputList->FindObject("NUAcentral"));
  TH2F* fOutliers = static_cast<TH2F*>(eventList->FindObject("hOutliers"));
  TH1D* fFMDHits = static_cast<TH1D*>(eventList->FindObject("FMDHits"));
  TH2D* fHybrid = static_cast<TH2D*>(eventList->FindObject("hybrid"));

  Int_t  iTracks(fAOD->GetNumberOfTracks());

  double cent = v0Centr;
  //const AliAODTracklets* spdmult = fAOD->GetMultiplicity();

  TH2D& forwarddNdedp = aodfmult->GetHistogram(); // also known as forwarddNdedp
  AliAODVertex* aodVtx = fAOD->GetPrimaryVertex();


  //AliAODTracklets* aodTracklets = fAOD->GetTracklets();
  //Int_t noTracklets = aodTracklets->GetNumberOfTracklets();
  
 

  bool useEvent = kTRUE;

  if (iTracks < 10) useEvent = kFALSE;

  Int_t phibins = forwarddNdedp.GetNbinsY();
  TString detType = "forward";

  for (Int_t etaBin = 1; etaBin <= forwarddNdedp.GetNbinsX(); etaBin++) {
    Double_t eta = forwarddNdedp.GetXaxis()->GetBinCenter(etaBin);


    for (Int_t phiBin = 0; phiBin <= phibins; phiBin++) {
      //Double_t phi = forwarddNdedp.GetYaxis()->GetBinCenter(phiBin);
      Double_t weight = forwarddNdedp.GetBinContent(etaBin, phiBin);
      if (!weight){
        weight = 0;
      }
      
      if (weight == 0) continue;
    } // End of phi loop
  } // End of eta bin

  if (!fSettings.ExtraEventCutFMD(forwarddNdedp, cent, true)) useEvent = false;
  if (useEvent){ 


    // loop  over  all  the  tracks
    /*
for (Int_t i = 0; i < aodTracklets->GetNumberOfTracklets(); i++) {
    spddNdedp.Fill(aodTracklets->GetEta(i),aodTracklets->GetPhi(i), 1);
        fHybrid->Fill(aodTracklets->GetEta(i),aodTracklets->GetPhi(i), 1);
        nuacentral->Fill(aodTracklets->GetEta(i),aodTracklets->GetPhi(i),aodVtx->GetZ(),1);
        nuahist->Fill(aodTracklets->GetEta(i),aodTracklets->GetPhi(i),aodVtx->GetZ(),1);
  }*/

  for(Int_t i(0); i < iTracks; i++) {

    AliAODTrack* track = static_cast<AliAODTrack *>(fAOD->GetTrack(i));
    if (track->TestFilterBit(kHybrid)){
      if (track->Pt() >= 0.2 && track->Pt() <= 5){
        //spddNdedp.Fill(track->Eta(),track->Phi(), 1);
        //fHybrid->Fill(track->Eta(),track->Phi(), 1);
        nuacentral->Fill(track->Eta(),track->Phi(),aodVtx->GetZ(),1);
        nuahist->Fill(track->Eta(),track->Phi(),aodVtx->GetZ(),1);
      }
    }
  }

  for (Int_t etaBin = 1; etaBin <= forwarddNdedp.GetNbinsX(); etaBin++) {
    
      Double_t acceptance = 1.;
      Double_t eta = forwarddNdedp.GetXaxis()->GetBinCenter(etaBin);

      for (Int_t phiBin = 0; phiBin <= phibins; phiBin++) {
        Double_t phi = forwarddNdedp.GetYaxis()->GetBinCenter(phiBin);
        Double_t weight = forwarddNdedp.GetBinContent(etaBin, phiBin);

        // We calculate the average Nch per. bin
        if (weight == 0) continue;
        
        // Fill into Cos() and Sin() hists
        fFMDHits->Fill(weight);
        nuaforward->Fill(eta,phi,aodVtx->GetZ(),weight);
        nuahist->Fill(eta,phi,aodVtx->GetZ(),weight);
      } // End of phi loop
    } // End of eta bin
  } // End of useEvent

  if(!fAOD) return;              

  PostData(1, fOutputList); 
  return;
}




//_____________________________________________________________________
void AliForwardNUATask::Terminate(Option_t */*option*/)
{
  return;
}


//_____________________________________________________________________
//
//
// EOF
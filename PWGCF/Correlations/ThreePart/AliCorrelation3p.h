//* This file is property of and copyright by the ALICE Project        * 
//* ALICE Experiment at CERN, All rights reserved.                     *
//* See cxx source for full Copyright notice                           *

/// @file   AliCorrelation3p.h
/// @author Matthias Richter
/// @date   2012-04-02
/// @brief  Three particle correlation
///

#ifndef ALICORRELATION3P_H
#define ALICORRELATION3P_H

#include "TNamed.h"
#include "TArrayD.h"

class TH1;
class TH1F;
class TH2F;
class TH1D;
class TH2D;
class TH3D;
class AliVParticle;
class AliVEvent;
class TCanvas;

class AliCorrelation3p : public TNamed {
 public:
  /// default constructor
  AliCorrelation3p(const char* name=NULL, TArrayD MBinEdges = TArrayD(1), TArrayD ZBinEdges = TArrayD(1));
  /// copy constructor
  AliCorrelation3p(const AliCorrelation3p& other);
  /// assignment operator
  AliCorrelation3p& operator=(const AliCorrelation3p& other);
  /// destructor
  virtual ~AliCorrelation3p();
  // init
  int Init(const char* arguments=NULL);
  int InitiateMc(){return 0;}
  int SetMultVZ(Double_t Mult,Double_t Vz);
  // check trigger particle cuts
  bool CheckTrigger( AliVParticle* trigger, bool doHistogram=false);
  // check associated particle cuts
  bool CheckAssociated( AliVParticle* p, const AliVParticle* trigger=NULL, bool doHistogram=false);
  /// fill histograms from particles
  int Fill(const AliVParticle* trigger		,const AliVParticle* p1	,const AliVParticle* p2	, const int weight=1);
  int Fill(const AliVParticle* trigger		,const AliVParticle* p1				);
  int FillTrigger();
  int MakeResultsFile(const char* scalingmethod, bool recreate=false, bool all=false);
  /// overloaded from TObject: cleanup
  virtual void Clear(Option_t * option ="");
  /// overloaded from TObject: print info
  virtual void Print(Option_t *option="") const;
  virtual TObject* FindObject(const char *name) const;
  /// overloaded from TObject: find object by pointer
  virtual TObject* FindObject(const TObject *obj) const;
  /// overloaded from TObject: save to file
  virtual void     SaveAs(const char *filename="",Option_t *option="") const; // *MENU*
  // interface for the TFileMerger
  virtual int      Merge(TCollection* list);
  /// overloaded from TObject: copy to target object
  virtual void     Copy(TObject &object) const;
  AliCorrelation3p& operator+=(const AliCorrelation3p& other);
  void SetMixedEvent(AliCorrelation3p* pME) {fMixedEvent=pME;}
  void SetAcceptanceCut(float AccCut){fAcceptanceCut = AccCut;}

  enum {
    kHistpT,  // TH1F
    kHistPhi, // TH1F
    kHistEta, // TH1F
    kHistTriggerpT,  // TH1F
    kHistTriggerPhi, // TH1F
    kHistTriggerEta, // TH1F
    kHistAssociatedpT,  // TH1F
    kHistAssociatedPhi, // TH1F
    kHistAssociatedEta, // TH1F
    kHistNassoc,        //TH1F
    kHistNTriggers, 	//TH1F
    khQAtocheckadressing, //TH1F
    khPhiEta,         // TH2F
//     khPhiEtaDublicate, // TH2F
    khPhiPhiDEta, //TH3F
    khPhiPhiDEtaScaled, //TH3F
    kNofHistograms//number, but points to itself
  };
  enum{
    kcentrvsvz, //TH2F
    kcentrvsvzbin, //TH2F
    kHistpTTriggerallbins, //TH1F
    kHistPhiTriggerallbins, //TH1F
    kHistEtaTriggerallbins, //TH1F
    kHistpTAssociatedallbins, //TH1F
    kHistPhiAssociatedallbins, //TH1F
    kHistEtaAssociatedallbins, //TH1F
    kNonbinnedhists //Number of the previous hists.
  };
  enum CollisionType{pp,PbPb,pPb};
  enum TriggerType{tracks,pi0};

 protected:
 private:
  //used functions
  const char* GetNameHist(const char* name,Int_t MBin,Int_t ZBin) const;
  Int_t GetNumberHist(Int_t khist,Int_t Mbin,Int_t ZBin) const;
  Int_t GetMultBin(Double_t Mult);
  Int_t GetZBin(Double_t Zvert);
  void HistFill(Int_t Histn,Double_t Val1);
  void HistFill(Int_t Histn,Double_t Val1,Double_t Val2);
  void HistFill(Int_t Histn,Double_t Val1,Double_t Val2, Double_t Val_3);
  void HistFill(Int_t Histn,Double_t Val1,Double_t Val2, Double_t Val_3, Double_t weight);
  TH2D * slice(TH3D* hist,const char* option, Int_t firstbin, Int_t lastbin, const char* name="slice", Bool_t baverage = kFALSE) const;
  TH2D * DetaDphiAss(TH3D * hist,const char * name = "detadphiAss");
  TH2D * DeltaEtaCut(TH3D* hist, const char* option, const char* name="deltaetacut", Bool_t baverage = kFALSE) const ;
  TCanvas * Makecanvas(TH1D* histtopl, TH1D* histtopm, TH1D* histtopr,TH1D* histmidl,TH1D* histmidm, TH1D* histmidr,TH1D* histbotl,TH1D* histbotm, TH1D* histbotr, const char* name, Bool_t Stats);
  TCanvas * Makecanvas(TH2D* histtopl, TH2D* histtopr, TH2D* histbotl, TH2D* histbotr,const char* name, Bool_t Stats);
  TCanvas * Makecanvas(TH2D* hist,const char* name, Bool_t Stats);
  Double_t FindScalingfactor(const char* scalingmethod,TH2D* sighist, TH2D* mixhist);
  Double_t GetPoint(TH1* hist,Double_t xpoint,Double_t ypoint = 0, Double_t zpoint=0);
  void AddHists(Bool_t isAverage,TH1* hist1, TH1* hist2);
  //Actual members
  TObjArray* fHistograms; // the histograms
  float fMinTriggerPt; // trigger pt threshold
  float fMaxTriggerPt; // trigger pt threshold
  float fMinAssociatedPt; // associated particle pt threshold
  float fMaxAssociatedPt; // associated particle pt threshold
  float fhPhiEtaDeltaPhi12Cut1; // phi vs. eta plots: cut on phi between associated particles 
  float fhPhiEtaDeltaPhi12Cut2; // phi vs. eta plots: cut on phi between associated particles 
  float fAcceptanceCut;
  AliCorrelation3p* fMixedEvent; // mixed event analysis
  TArrayD 	fMBinEdges; //Contains bin edges in centrality.
  TArrayD 	fZBinEdges; //Edges for vZ binning.
  float fMultiplicity; //Multiplicity in this event
  float fVZ; //z Vertex in this event
  int fMBin;
  int fVzBin;
  CollisionType fCollisionType;
  TriggerType fTriggerType;

  //Class definition.
  ClassDef(AliCorrelation3p, 2)
};
#endif

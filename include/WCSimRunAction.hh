#ifndef WCSimRunAction_h
#define WCSimRunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "G4String.hh"

#include "TFile.h"
#include "TTree.h"
#include "WCSimRootEvent.hh"
#include "WCSimRootGeom.hh"
#include "WCSimDetectorConstruction.hh"

class G4Run;
class WCSimRunActionMessenger;

class WCSimRunAction : public G4UserRunAction
{
public:
  WCSimRunAction(WCSimDetectorConstruction*);
  ~WCSimRunAction();

public:
  void BeginOfRunAction(const G4Run*);
  void EndOfRunAction(const G4Run*);
  void SetRootFileName(G4String fname) { RootFileName = fname; }
  G4String GetRootFileName() { return RootFileName; }
  void FillGeoTree();
  TTree* GetTree(){ return WCSimTree; }
  TBranch* GetBranch(G4String detectorElement){
    if(detectorElement=="tank") return wcsimrooteventbranch;
    else if(detectorElement=="mrd")  return wcsimrooteventbranch_mrd;
    else if(detectorElement=="facc") return wcsimrooteventbranch_facc;
    else G4cout<<"Unkown detector element"<<G4endl;}
  TTree* GetGeoTree(){return geoTree;}
  WCSimRootGeom* GetRootGeom(){return wcsimrootgeom;}
  WCSimRootEvent* GetRootEvent(G4String detectorElement){
    if(detectorElement=="tank") return wcsimrootsuperevent;
    if(detectorElement=="mrd") return wcsimrootsuperevent_mrd;
    if(detectorElement=="facc") return wcsimrootsuperevent_facc;}

  void SetTree(TTree* tree){WCSimTree=tree;}
  void SetBranch(TBranch* branchin, G4String detectorElement){
    if(detectorElement=="tank") wcsimrooteventbranch=branchin;
    if(detectorElement=="mrd") wcsimrooteventbranch_mrd=branchin;
    if(detectorElement=="facc") wcsimrooteventbranch_facc=branchin;}
  void SetGeoTree(TTree* tree){geoTree=tree;}
  void SetRootEvent(WCSimRootEvent* revent, G4String detectorElement){
    if(detectorElement=="tank") wcsimrootsuperevent=revent;
    if(detectorElement=="mrd") wcsimrootsuperevent_mrd=revent;
    if(detectorElement=="facc") wcsimrootsuperevent_facc=revent;}
  void SetRootGeom(WCSimRootGeom* rgeom){wcsimrootgeom=rgeom;}
  int  GetNumberOfEventsGenerated() { return numberOfEventsGenerated;}
  int  GetNtuples(){return ntuples;}

  void incrementEventsGenerated() { numberOfEventsGenerated++;}
  void incrementWaterTubeHits()   { numberOfTimesWaterTubeHit++;} 
  void incrementFVWaterTubeHits() { numberOfTimesFVWaterTubeHit++;} 
  void incrementCatcherHits()     { numberOfTimesCatcherHit++;}
  void SetNtuples(int ntup) {ntuples=ntup;}

private:
  // MFechner : set by the messenger
  std::string RootFileName;
  //
  TTree* WCSimTree;
  TBranch* wcsimrooteventbranch;
  TBranch* wcsimrooteventbranch_mrd;
  TBranch* wcsimrooteventbranch_facc;
  TTree* geoTree;
  WCSimRootEvent* wcsimrootsuperevent;
  WCSimRootEvent* wcsimrootsuperevent_mrd;
  WCSimRootEvent* wcsimrootsuperevent_facc;
  WCSimRootGeom* wcsimrootgeom;
  WCSimDetectorConstruction* wcsimdetector;

  int numberOfEventsGenerated;
  int numberOfTimesWaterTubeHit;
  int numberOfTimesFVWaterTubeHit;
  int numberOfTimesCatcherHit;

  WCSimRunActionMessenger* messenger;
  int ntuples;  // 1 for ntuples to be written
};

#endif

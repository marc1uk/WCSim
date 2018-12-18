#include "WCSimTrackingAction.hh"
#include "WCSimTrajectory.hh"
#include "G4ParticleTypes.hh"
#include "G4TrackingManager.hh"
#include "G4Track.hh"
#include "G4ios.hh"
#include "G4VProcess.hh"
#include "WCSimTrackInformation.hh"
#include "G4TransportationManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include <algorithm>

WCSimTrackingAction::WCSimTrackingAction(){
  ProcessList.insert("Decay") ;
  ProcessList.insert("nCapture");
  ProcessList.insert("MuonMinusCaptureAtRest");
  ProcessList.insert("muMinusCaptureAtRest");  // which syntax is correct?
  //ProcessList.insert("conv");
  ParticleList.insert(111);  // pi0
  ParticleList.insert(211);  // pion+
  ParticleList.insert(-211); // pion-
  ParticleList.insert(321);  // kaon+
  ParticleList.insert(-321); // kaon-
  ParticleList.insert(311);  // kaon0
  ParticleList.insert(-311); // kaon0 bar
  ParticleList.insert(12);   // nu_e
  ParticleList.insert(-12);  // nubar_e
  ParticleList.insert(13);   // mu-
  ParticleList.insert(-13);  // mu+
  ParticleList.insert(14);   // nu_mu
  ParticleList.insert(-14);  // nubar_mu
  ParticleList.insert(2112); // neutron
  ParticleList.insert(2212); // proton
  ParticleList.insert(100);  // photon XXX FIXME 
//  ParticleList.insert(11);   // e-    // do not save electrons unless they are from Decay process (mu decay)
//  ParticleList.insert(-11);  // e+
//  Don't put gammas there or there'll be too many -  we can add an energy cut later
  
}

WCSimTrackingAction::~WCSimTrackingAction(){;}

void WCSimTrackingAction::PreUserTrackingAction(const G4Track* aTrack){
  G4float percentageOfCherenkovPhotonsToDraw = 100.0; // XXX FIXME
  
  if (aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()
       || G4UniformRand() < percentageOfCherenkovPhotonsToDraw){
      WCSimTrajectory* thisTrajectory = new WCSimTrajectory(aTrack);
      fpTrackingManager->SetTrajectory(thisTrajectory);
      fpTrackingManager->SetStoreTrajectory(true);
  } else {
      fpTrackingManager->SetStoreTrajectory(false);
  }
  
  // XXX TEMP DISABLE ME XXX
  // implemented to allow photon tracks to be drawn during photon debugging, 
  // but interferes with saving of primary information.
  WCSimTrackInformation* anInfo = new WCSimTrackInformation();
  G4Track* theTrack = (G4Track*)aTrack;
  anInfo->WillBeSaved(false);
  anInfo->SetPrimaryParentID(aTrack->GetTrackID());
  theTrack->SetUserInformation(anInfo);
  // XXX TEMP END DISABLE XXX
  
  // clear scattering info as they are static members of the class. 
  // N.B. this requires photons will be tracked from generation to absorption in one go.
  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition() ){
    WCSimTrackInformation::numscatters=0;
    WCSimTrackInformation::scatterings.clear();
  }
}

void WCSimTrackingAction::PostUserTrackingAction(const G4Track* aTrack){
  
  // retrieve UserTrackInfo
  // This is used to keep track of parentage - PrimaryParentId, stored in Hits,
  // and ParentPdg, used by EndOfEventAction when recording Tracks -
  // and to mark Trajectories to be saved by EndOfEventAction
  WCSimTrackInformation* anInfo;
  if (aTrack->GetUserInformation()){
    anInfo = (WCSimTrackInformation*)(aTrack->GetUserInformation());
  } else {
    anInfo = new WCSimTrackInformation();
  }
  
  // get particle type and creator process
  const G4VProcess* creatorProcess = aTrack->GetCreatorProcess();
  G4int thispdg;
  if(aTrack->GetDefinition()==G4OpticalPhoton::OpticalPhotonDefinition()) thispdg=100;
  else thispdg = aTrack->GetDefinition()->GetPDGEncoding();
  
  // check if it's of interest
  // *  is it a primary ?
  // *  is the process in the set ?
  // *  is the particle in the set ?
  // *  is it a gamma with energy > threshold?
  if( ( aTrack->GetParentID()==0 ) ||
      ( (creatorProcess!=0) && ProcessList.count(creatorProcess->GetProcessName()) ) ||
      ( ParticleList.count(thispdg) ) ||
      ( thispdg==22 && aTrack->GetTotalEnergy()>50.0*MeV ) ||     // 50 MeV? 1MeV? what threshold?
      ( thispdg==22 && anInfo->GetParentPdg()==111 )              // gamma from a Pi0 decay
    ){
    anInfo->WillBeSaved(true);
  } else {
    anInfo->WillBeSaved(false);
  }
  
  // for primary particles, set the ParentID to the track's own ID
  if(aTrack->GetParentID()==0 /*&& aTrack->GetDefinition()!=G4OpticalPhoton::OpticalPhotonDefinition()*/){ //XXX
    anInfo->SetPrimaryParentID(aTrack->GetTrackID());
  }
  
  // bypass const-ness to update track information
  G4Track* theTrack = (G4Track*)aTrack;
  theTrack->SetUserInformation(anInfo);
  
  // pass parentage information to children
  G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();
  if(secondaries){
      for(size_t i=0;i<secondaries->size();i++){
        WCSimTrackInformation* infoSec = new WCSimTrackInformation(anInfo);
        infoSec->WillBeSaved(false);
        infoSec->SetParentPdg(thispdg);
        (*secondaries)[i]->SetUserInformation(infoSec);
      }
  }
  
  // Pass the information to the Trajectory, used by EndOfEventAction
  if ( aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()){ /// XXX FIXME
    G4ThreeVector currentPosition      = aTrack->GetPosition();
    G4VPhysicalVolume* currentVolume   = aTrack->GetVolume();
    G4double currentTime               = aTrack->GetGlobalTime();
    G4ThreeVector currentMomentum      = aTrack->GetMomentum();
    
    WCSimTrajectory *currentTrajectory = (WCSimTrajectory*)fpTrackingManager->GimmeTrajectory();
    currentTrajectory->SetStoppingPoint(currentPosition);
    currentTrajectory->SetStoppingVolume(currentVolume);
    currentTrajectory->SetStoppingTime(currentTime);
    currentTrajectory->SetStoppingMomentum(currentMomentum);
    currentTrajectory->SetParentPdg(anInfo->GetParentPdg());
    currentTrajectory->SetSaveFlag(anInfo->isSaved());
  }
  
  // report every 100000'th track, just to see progress
  static int line=0;
  if(line%100000==0){ //100000
    G4cout<<"  PostUserTrackingAction call number: "<<line
          <<", "<<aTrack->GetDefinition()->GetParticleName();
    if(creatorProcess) G4cout<<" from "<<creatorProcess->GetProcessName();
    else G4cout<<"primary";
    G4cout<<" in "<<aTrack->GetVolume()->GetName()<<G4endl; 
  }
  line++;
  
  /*
  if( (aTrack->GetParentID()==0) // primary particle
      && (abs(thispdg)==13) ){ // is a muon
      G4ThreeVector endpos = aTrack->GetPosition();
      WCSimTrajectory* trj = (WCSimTrajectory*)fpTrackingManager->GimmeTrajectory();
      G4TrajectoryPoint* startpnt = (G4TrajectoryPoint*)trj->GetPoint(0);
      G4ThreeVector startpos  = startpnt->GetPosition();
      G4cout<<"Primary muon started at ("<<startpos.x()<<", "<<startpos.y()<<", "<<startpos.z()<<")"
            <<"and ended at ("<<endpos.x()<<", "<<endpos.y()<<", "<<endpos.z()<<")"<<G4endl;
  }
  */
}






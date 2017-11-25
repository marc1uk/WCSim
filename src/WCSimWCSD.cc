#include "WCSimWCSD.hh"
#include "G4ParticleTypes.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "Randomize.hh"
#include "G4ios.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include <sstream>

#include "WCSimDetectorConstruction.hh"
#include "WCSimTrackInformation.hh"

WCSimWCSD::WCSimWCSD(G4String CollectionName, G4String name,WCSimDetectorConstruction* myDet, G4String detectorElement="tank")
:G4VSensitiveDetector(name), detectorElement(detectorElement)
{
  // Place the name of this collection on the list.  We can have more than one
  // in principle.  CollectionName is a vector.

  // Note there is some sort of problem here.  If I use the name
  // Which has a "/" in it, I can find this collection later using 
  // GetCollectionID()

  collectionName.insert(CollectionName);
  
  fdet = myDet;
  
  HCID = -1;
}

WCSimWCSD::~WCSimWCSD() {}

void WCSimWCSD::Initialize(G4HCofThisEvent* HCE)
{
  // Make a new hits collection. With the name we set in the constructor
  if(collectionName[0]!= fdet->GetIDCollectionName2()){
    hitsCollection = new WCSimWCHitsCollection
      (SensitiveDetectorName,collectionName[0]);
  //G4cout<<"collectionName[0]= ******* "<<collectionName[0]<<G4endl;
  } else{
    hitsCollectionlappd = new WCSimWCHitsCollection
      (SensitiveDetectorName,collectionName[0]);
  }
  // This is a trick.  We only want to do this once.  When the program
  // starts HCID will equal -1.  Then it will be set to the pointer to
  // this collection.

  
  // Get the Id of the "0th" collection
  if (HCID<0){
    HCID =  GetCollectionID(0); 
  }  
  // Add it to the Hit collection of this event.

  if(collectionName[0]!= fdet->GetIDCollectionName2()){
    HCE->AddHitsCollection( HCID, hitsCollection ); 
  } else {
    HCE->AddHitsCollection( HCID, hitsCollectionlappd ); 
  }

  // Initilize the Hit map to all tubes not hit.
  PMTHitMap.clear();
  LAPPDHitMap.clear();
  // Trick to access the static maxPE variable.  This will go away with the 
  // variable.

  WCSimWCHit* newHit = new WCSimWCHit();
  newHit->SetMaxPe(0);
  delete newHit;
}

G4bool WCSimWCSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{ 

  G4StepPoint*       preStepPoint = aStep->GetPreStepPoint();
  G4TouchableHandle  theTouchable = preStepPoint->GetTouchableHandle();
  G4VPhysicalVolume* thePhysical  = theTouchable->GetVolume();


  //XQ 3/30/11 try to get the local position try to add the position and direction
  G4ThreeVector worldPosition = preStepPoint->GetPosition();
  G4ThreeVector localPosition = theTouchable->GetHistory()->GetTopTransform().TransformPoint(worldPosition);
  G4ThreeVector worldDirection = preStepPoint->GetMomentumDirection();
  G4ThreeVector localDirection = theTouchable->GetHistory()->GetTopTransform().TransformAxis(worldDirection);

  

  WCSimTrackInformation* trackinfo 
    = (WCSimTrackInformation*)(aStep->GetTrack()->GetUserInformation());
  G4int primParentID;
  if (trackinfo)
    primParentID = trackinfo->GetPrimaryParentID();
  else // if there is no trackinfo, then it is a primary particle!
  if (aStep->GetTrack()->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
    primParentID = aStep->GetTrack()->GetTrackID();
  } else {
    // it is a primary photon
    primParentID=-1;
  }
  int numscatters= WCSimTrackInformation::GetNumScatterings();
  std::map<std::string, int> scatterings = WCSimTrackInformation::GetScatterings();

  G4int    trackID           = aStep->GetTrack()->GetTrackID();
  G4String volumeName        = aStep->GetTrack()->GetVolume()->GetName();
  
  
  //XQ Add the wavelength there
  G4float stepEnergy =aStep->GetTrack()->GetTotalEnergy()/CLHEP::eV;
  G4float  wavelength = (stepEnergy!=0) ? (2.0*M_PI*197.3)/(stepEnergy) : 0;
  
  G4double energyDeposition  = aStep->GetTotalEnergyDeposit();
  G4double hitTime           = aStep->GetPreStepPoint()->GetGlobalTime();

  G4ParticleDefinition *particleDefinition = 
    aStep->GetTrack()->GetDefinition();
    

  if ( particleDefinition != G4OpticalPhoton::OpticalPhotonDefinition() 
       && energyDeposition == 0.0) 
    return false;
  // MF : I don't see why other particles should register hits
  // they don't in skdetsim. 
  if ( particleDefinition != G4OpticalPhoton::OpticalPhotonDefinition()){
    return false;
  }
  // M Fechner : too verbose
  //  if (aStep->GetTrack()->GetTrackStatus() == fAlive)G4cout << "status is fAlive\n";
  if ((aStep->GetTrack()->GetTrackStatus() == fAlive )
      &&(particleDefinition == G4OpticalPhoton::OpticalPhotonDefinition())){
      return false;
  }

    
  // Make the tubeTag string based on the replica numbers
  // See WCSimDetectorConstruction::DescribeAndRegisterPMT() for matching
  // tag construction.

  std::stringstream tubeTag;
  std::stringstream lappdTag;

  // Start tubeTag with mother to distinguish different PMT hierarchies
//  G4LogicalVolume *theMother = thePhysical->GetMotherLogical();
//  if (theMother != NULL)
//    tubeTag << theMother->GetName() << ":";

//  tubeTag << thePhysical->GetName(); 
  if(collectionName[0]!= fdet->GetIDCollectionName2()){
    for (G4int i = theTouchable->GetHistoryDepth()-1 ; i >= 0; i--){
      tubeTag << ":" << theTouchable->GetVolume(i)->GetName();
      tubeTag << "-" << theTouchable->GetCopyNumber(i);
    }
    //  tubeTag << ":" << theTouchable->GetVolume(i)->GetCopyNo(); 
  } else {
    for (G4int ii = theTouchable->GetHistoryDepth()-1 ; ii >= 0; ii--){
      lappdTag << ":" << theTouchable->GetVolume(ii)->GetName();
      lappdTag << "-" << theTouchable->GetCopyNumber(ii);
      // G4cout<<"00000 lappdTag: "<<lappdTag.str()<<G4endl;
    }
  }

//  G4cout << tubeTag.str() << G4endl;

  // Get the tube ID from the tubeTag
  G4int replicaNumber;
  if(detectorElement=="tank"){
    replicaNumber = WCSimDetectorConstruction::GetTubeID(tubeTag.str());
  } else if(detectorElement=="mrd"){
    replicaNumber = WCSimDetectorConstruction::GetMrdTubeID(tubeTag.str());
  } else if(detectorElement=="facc"){
    replicaNumber = WCSimDetectorConstruction::GetFaccTubeID(tubeTag.str());
  }
  G4int replicaNumber2 = WCSimDetectorConstruction::GetLAPPDID(lappdTag.str());

    
  G4float theta_angle;
  G4float effectiveAngularEfficiency;
  G4float effectiveAngularEfficiency2;

  
  G4float ratio = 1.;
  G4float maxQE;
  G4float photonQE;
  if(volumeName != fdet->GetIDCollectionName2()){
  if (fdet->GetPMT_QE_Method()==1){
    photonQE = 1.1;
  }else if (fdet->GetPMT_QE_Method()==2){
    maxQE = fdet->GetPMTQE(volumeName,wavelength,0,240,660,ratio);
    photonQE = fdet->GetPMTQE(volumeName, wavelength,1,240,660,ratio);
    photonQE = photonQE/maxQE;
  }else if (fdet->GetPMT_QE_Method()==3){
    ratio = 1./(1.-0.25);
    photonQE = fdet->GetPMTQE(volumeName, wavelength,1,240,660,ratio);
  }else if (fdet->GetPMT_QE_Method()==4){
    maxQE = fdet->GetPMTQE(fdet->GetIDCollectionName(), wavelength,0,240,660,ratio);
    photonQE = fdet->GetPMTQE(volumeName, wavelength,1,240,660,ratio);
    photonQE = photonQE/maxQE;
  }else{ photonQE=0.3; }
  } else {
  //----- for lappds -------
    if (fdet->GetLAPPD_QE_Method()==1){
      photonQE = 1.1;
    }else if (fdet->GetLAPPD_QE_Method()==2){
      maxQE = fdet->GetLAPPDQE(volumeName,wavelength,0,240,660,ratio);
      photonQE = fdet->GetLAPPDQE(volumeName, wavelength,1,240,660,ratio);
      photonQE = photonQE/maxQE;
    }else if (fdet->GetLAPPD_QE_Method()==3){
      ratio = 1./(1.-0.25);
      photonQE = fdet->GetLAPPDQE(volumeName, wavelength,1,240,660,ratio);
    }else if (fdet->GetLAPPD_QE_Method()==4){
      maxQE = fdet->GetPMTQE(fdet->GetIDCollectionName(), wavelength,0,240,660,ratio);
      photonQE = fdet->GetLAPPDQE(volumeName, wavelength,1,240,660,ratio);
      photonQE = photonQE/maxQE;
    }else{ photonQE=0.3; }
  }

  
  if (G4UniformRand() <= photonQE){
   
     G4double local_x = localPosition.x();
     G4double local_y = localPosition.y();
     G4double local_z = localPosition.z();
     theta_angle = acos(fabs(local_z)/sqrt(pow(local_x,2)+pow(local_y,2)+pow(local_z,2)))/3.1415926*180.;
     
     if(volumeName != fdet->GetIDCollectionName2()){
       // for pmts
       effectiveAngularEfficiency = fdet->GetPMTCollectionEfficiency(theta_angle, volumeName);
       if (G4UniformRand() <= effectiveAngularEfficiency || fdet->UsePMT_Coll_Eff()==0){
         //Retrieve the pointer to the appropriate hit collection. Since volumeName is the same as the SD name, this works. 
         G4SDManager* SDman = G4SDManager::GetSDMpointer();
         G4RunManager* Runman = G4RunManager::GetRunManager();
         G4int collectionID = SDman->GetCollectionID(volumeName);
         const G4Event* currentEvent = Runman->GetCurrentEvent();
         G4HCofThisEvent* HCofEvent = currentEvent->GetHCofThisEvent();
         hitsCollection = (WCSimWCHitsCollection*)(HCofEvent->GetHC(collectionID));
        
         // If this tube hasn't been hit add it to the collection
         if (PMTHitMap[replicaNumber] == 0)
	   {
	     //G4cout<<"_________ new PMT hit_______"<<G4endl;
	     //G4cout<<"localPMTPosition : "<<localPosition(0)<<","<<localPosition(1)<<","<<localPosition(2)<<G4endl;
	     WCSimWCHit* newHit = new WCSimWCHit();
	     newHit->SetTubeID(replicaNumber);
	     newHit->SetTrackID(trackID);
	     newHit->SetEdep(energyDeposition); 
	     newHit->SetLogicalVolume(thePhysical->GetLogicalVolume());
	     //G4cout<<"it should have: replicaNumber= "<<replicaNumber<<" trackID= "<<trackID<<G4endl;
	     G4AffineTransform aTrans = theTouchable->GetHistory()->GetTopTransform();
	     newHit->SetRot(aTrans.NetRotation());
	     
	     aTrans.Invert();
	     newHit->SetPos(aTrans.NetTranslation());
	     
	     // Set the hitMap value to the collection hit number
	     PMTHitMap[replicaNumber] = hitsCollection->insert( newHit );
	     (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddPe(hitTime);
	     (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddParentID(primParentID);
	     (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddHitPos(worldPosition);
	     (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddHitNScatters(numscatters);
	     (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddHitScatters(scatterings);
	     
	     //     if ( particleDefinition != G4OpticalPhoton::OpticalPhotonDefinition() )
	     //       newHit->Print();
	   }
         else {
	   (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddPe(hitTime);
	   (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddParentID(primParentID);
	   (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddHitPos(worldPosition);
	   (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddHitNScatters(numscatters);
	   (*hitsCollection)[PMTHitMap[replicaNumber]-1]->AddHitScatters(scatterings);
	   
         }
       }
     } else {
     //__________ lappd _________
       effectiveAngularEfficiency2 = fdet->GetLAPPDCollectionEfficiency(theta_angle, volumeName);
       if (G4UniformRand() <= effectiveAngularEfficiency2 || fdet->UseLAPPD_Coll_Eff()==0){
       //Retrieve the pointer to the appropriate hit collection. Since volumeName is the same as the SD name, this works. 
       G4SDManager* SDman = G4SDManager::GetSDMpointer();
       G4RunManager* Runman = G4RunManager::GetRunManager();
       G4int collectionID = SDman->GetCollectionID(volumeName);
       const G4Event* currentEvent = Runman->GetCurrentEvent();
       G4HCofThisEvent* HCofEvent = currentEvent->GetHCofThisEvent();

       //G4cout<<"trackID= "<<trackID<<" volumeName= "<<volumeName<<" hitTime= "<<hitTime<<G4endl;

       /*G4StepPoint*       postStepPoint = aStep->GetPostStepPoint();
       G4ThreeVector worldPositionpost = postStepPoint->GetPosition();
       G4ThreeVector localPositionpost = theTouchable->GetHistory()->GetTopTransform().TransformPoint(worldPositionpost);
       */
       hitsCollectionlappd = (WCSimWCHitsCollection*)(HCofEvent->GetHC(collectionID));
       // If this tube hasn't been hit add it to the collection
       if (LAPPDHitMap[replicaNumber2] == 0)
	 {
	   //G4cout<<"_________ new LAPPD hit_______"<<G4endl;
	   WCSimWCHit* newHit1 = new WCSimWCHit();
	   newHit1->SetTubeID(replicaNumber2);

	   //G4cout<<"it should have: replicaNumber2= "<<replicaNumber2<<" trackID= "<<trackID<<G4endl;
	   newHit1->SetLogicalVolume(thePhysical->GetLogicalVolume());
	   
	   G4AffineTransform aTrans = theTouchable->GetHistory()->GetTopTransform();
	   newHit1->SetRot(aTrans.NetRotation());

	   //G4cout<<"worldPosition : "<<worldPosition(0)<<","<<worldPosition(1)<<","<<worldPosition(2)<<G4endl;
	   //G4cout<<"localPosition : "<<localPosition(0)<<","<<localPosition(1)<<","<<localPosition(2)<<G4endl;
          
	   aTrans.Invert();
	   //G4cout<<"aTrans.NetTranslation()= "<<aTrans.NetTranslation().x()<<","<<aTrans.NetTranslation().y()<<","<<aTrans.NetTranslation().z()<<G4endl;
	   //G4cout<<"HITdist_diffLV: "<<sqrt( (localPosition(0)-aTrans.NetTranslation().x())*(localPosition(0)-aTrans.NetTranslation().x()) + (localPosition(1)-aTrans.NetTranslation().y())*(localPosition(1)-aTrans.NetTranslation().y()) + (localPosition(2)-aTrans.NetTranslation().z())*(localPosition(2)-aTrans.NetTranslation().z()) )<<G4endl;
           //G4cout<<"HITdist_RLV: "<<sqrt( (localPosition(0)-aTrans.NetTranslation().x())*(localPosition(0)-aTrans.NetTranslation().x()) + (localPosition(1)-aTrans.NetTranslation().y())*(localPosition(1)-aTrans.NetTranslation().y()) )<<G4endl;

	   newHit1->SetPos(aTrans.NetTranslation());

	   // Set the hitMap value to the collection hit number
	   LAPPDHitMap[replicaNumber2] = hitsCollectionlappd->insert( newHit1 );
	   (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddPe(hitTime);
	   (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddParentID(primParentID);
	   (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddStripPosition(localPosition);
	   (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddHitPos(worldPosition);
	   (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddHitNScatters(numscatters);
	   (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddHitScatters(scatterings);
	   //G4cout<<"hitTime= "<<hitTime<<" primParentID= "<<primParentID<<G4endl;
	   //     if ( particleDefinition != G4OpticalPhoton::OpticalPhotonDefinition() )
	   //       newHit->Print();
	 }
       else {
	 (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddPe(hitTime);
	 (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddParentID(primParentID);
	 (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddHitNScatters(numscatters);
	 (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddHitScatters(scatterings);
	 //G4cout<<"add new localPosition : "<<localPosition(0)<<","<<localPosition(1)<<","<<localPosition(2)<<G4endl;
         (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddStripPosition(localPosition);
         (*hitsCollectionlappd)[LAPPDHitMap[replicaNumber2]-1]->AddHitPos(worldPosition);
 	 //G4cout<<"_________________________"<<G4endl;	
       }
     }
   }//for lappds
    //__________________________
  }

  return true;
}

void WCSimWCSD::EndOfEvent(G4HCofThisEvent*)
{
  if (verboseLevel>0) 
  { 
    G4int numHits = hitsCollection->entries();
    G4int numHitslappd = ((hitsCollectionlappd) ? hitsCollectionlappd->entries() : 0);

    G4cout << "There are " << numHits << " hits in the "<<detectorElement<<" : "<< G4endl;
    G4cout << "There are " << numHitslappd << " hits in the WC-LAPPDs: " << G4endl;
    for (G4int i=0; i < numHits; i++) 
      (*hitsCollection)[i]->Print();
    if(hitsCollectionlappd){
      for (G4int ii=0; ii < numHitslappd; ii++) 
        (*hitsCollectionlappd)[ii]->Print();
    }
  }
}


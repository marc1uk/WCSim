/* vim:set noexpandtab tabstop=4 wrap */
#include <stdlib.h>
#include <stdio.h>

#include "WCSimSteppingAction.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4VParticleChange.hh"
#include "G4SteppingVerbose.hh"
#include "G4SteppingManager.hh"
#include "G4PVParameterised.hh"
#include "G4PVReplica.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4OpBoundaryProcess.hh"


void WCSimSteppingAction::UserSteppingAction(const G4Step* aStep)
{
  //return;    // disable while investigating differences in validation plots.

  G4Track* track = aStep->GetTrack();
//  G4VPhysicalVolume* thePostPV = aStep->GetPostStepPoint()->GetPhysicalVolume();

  // For estimating tank energy loss vs digits, need an accurate energy on tank exit - kill particle on
  // tank exit,then end energy will be tank exit energy
  //G4String thePostPVname = (thePostPV) ? thePostPV->GetName() : "No Vol";
  //if(thePostPVname=="Hall"){ track->SetTrackStatus(fStopAndKill); return; }

  //DISTORTION must be used ONLY if INNERTUBE or INNERTUBEBIG has been defined in BidoneDetectorConstruction.cc
  
//  const G4Event* evt = G4RunManager::GetRunManager()->GetCurrentEvent();

//  const G4Track* track       = aStep->GetTrack();
//  G4VPhysicalVolume* volume  = track->GetVolume();
//  G4String volumeName        = volume->GetName();

//  G4SDManager* SDman   = G4SDManager::GetSDMpointer();
//  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
//  G4String processname = aStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();

//  debugging 
//  G4Track* theTrack = aStep->GetTrack();
//  const G4DynamicParticle* aParticle = theTrack->GetDynamicParticle();
//  G4ThreeVector aMomentum = aParticle->GetMomentumDirection();
//  G4double vx = aMomentum.x();
//  G4int ix = std::isnan(vx);
//  if(ix != 0){
//    G4cout << " PROBLEM! " << theTrack->GetCreatorProcess()->GetProcessName() <<
//  std::flush << G4endl;
//  }
  static int numbrokenphotons=0;

  static G4ThreadLocal G4OpBoundaryProcess* boundary=NULL;
  //find the boundary process only once
  if(!boundary){
    G4ProcessManager* pm = track->GetDefinition()->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    G4ProcessVector* pv = pm->GetProcessList();
    G4int i;
    for( i=0;i<nprocesses;i++){
      if((*pv)[i]->GetProcessName()=="OpBoundary"){
        boundary = (G4OpBoundaryProcess*)(*pv)[i];
        break;
      }
    }
  }
  
//  if(!thePostPV){//out of world
//    fExpectedNextStatus=Undefined;
//    return;
//  }
  
  if(track->GetDefinition()==G4OpticalPhoton::OpticalPhotonDefinition()){
   //G4cout<<"optical photon StepStatus is          "<<thePostPoint->GetStepStatus()<<G4endl
   //      <<"               fExpectedNextStatus is "<<fExpectedNextStatus<<G4endl
   //      <<"               boundaryStatus is      "<<boundary->GetStatus()<<G4endl;
   if ( track->GetCurrentStepNumber() > 50000 ){   // 50k steps: sufficiently generous?
     track->SetTrackStatus(fStopAndKill); 
     G4cout<<"killing broken photon "<<++numbrokenphotons<<" at ("
           <<aStep->GetPostStepPoint()->GetPosition().x()<<", "
           <<aStep->GetPostStepPoint()->GetPosition().y()<<", "
           <<aStep->GetPostStepPoint()->GetPosition().z()<<")"<<G4endl;
     fExpectedNextStatus=Undefined;
     return;
   }
   
   if ( track->GetCurrentStepNumber() == 1 ) fExpectedNextStatus = Undefined;
   
   if( aStep->GetPostStepPoint()->GetStepStatus()==fGeomBoundary){
     G4OpBoundaryProcessStatus boundaryStatus=boundary->GetStatus();
     if((fExpectedNextStatus==StepTooSmall)&&(boundaryStatus!=StepTooSmall)){
          G4cout<<"error: poststeppoint volume is "
                <<aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName()
                <<" prestep point is "
                <<aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName()
                <<", step length was "<<aStep->GetStepLength()
                <<G4endl;
                
          /*G4ExceptionDescription ed;
          ed << "WCSimSteppingAction::UserSteppingAction(): "
                << "No reallocation step after reflection!"
                << G4endl;
          G4Exception("WCSimSteppingAction::UserSteppingAction()", "WCSimExpl01",
          FatalException,ed,
          "Something is wrong with the surface normal or geometry");*/
          track->SetTrackStatus(fStopAndKill);
          fExpectedNextStatus=Undefined;
          return;
      }
      fExpectedNextStatus=Undefined;
      switch(boundaryStatus){
      case FresnelReflection:
      case TotalInternalReflection:
      case LambertianReflection:
      case LobeReflection:
      case SpikeReflection:
      case BackScattering:
        fExpectedNextStatus=StepTooSmall;
        break;
      default:
        break;
      }
    }
  }
}

///////////////////////////////////////////////////////

G4int WCSimSteppingAction::G4ThreeVectorToWireTime(G4ThreeVector *pos3d,
						    G4ThreeVector lArPos,
						    G4ThreeVector start,
						    G4int i)
{
  G4double x0 = start[0]-lArPos[0];
  G4double y0 = start[1]-lArPos[1];
//   G4double y0 = 2121.3;//mm
  G4double z0 = start[2]-lArPos[2];

  G4double dt=0.8;//mm
//   G4double midt = 2651.625;
  G4double pitch = 3;//mm
//   G4double midwir = 1207.10;
  G4double c45 = 0.707106781;
  G4double s45 = 0.707106781;

  G4double w1;
  G4double w2;
  G4double t;

//   G4double xField(0.);
//   G4double yField(0.);
//   G4double zField(0.);

//   if(detector->getElectricFieldDistortion())
//     {
//       Distortion(pos3d->getX(),
// 		 pos3d->getY());
      
//       xField = ret[0];
//       yField = ret[1];
//       zField = pos3d->getZ();
      
//       w1 = (int)(((zField+z0)*c45 + (x0-xField)*s45)/pitch); 
//       w2 = (int)(((zField+z0)*c45 + (x0+xField)*s45)/pitch); 
//       t = (int)(yField+1);

//       //G4cout<<" x orig "<<pos3d->getX()<<" y orig "<<(pos3d->getY()+y0)/dt<<G4endl;
//       //G4cout<<" x new "<<xField<<" y new "<<yField<<G4endl;
//     }
//   else 
//     {
      
  w1 = (int) (((pos3d->getZ()+z0)*c45 + (x0-pos3d->getX())*s45)/pitch); 
  w2 = (int)(((pos3d->getZ()+z0)*c45 + (x0+pos3d->getX())*s45)/pitch); 
  t  = (int)((pos3d->getY()+y0)/dt +1);
//     }

  if (i==0)
    return (int)w1;
  else if (i==1)
    return (int)w2;
  else if (i==2)
    return (int)t;
  else return 0;
}


void WCSimSteppingAction::Distortion(G4double /*x*/,G4double /*y*/)
{
 
//   G4double theta,steps,yy,y0,EvGx,EvGy,EField,velocity,tSample,dt;
//   y0=2121.3;//mm
//   steps=0;//1 mm steps
//   tSample=0.4; //micros
//   dt=0.8;//mm
//   LiquidArgonMedium medium;  
//   yy=y;
//   while(y<y0 && y>-y0 )
//     {
//       EvGx=FieldLines(x,y,1);
//       EvGy=FieldLines(x,y,2);
//       theta=atan(EvGx/EvGy);
//       if(EvGy>0)
// 	{
// 	  x+=sin(theta);
// 	  y+=cos(theta);
// 	}
//       else
// 	{
// 	  y-=cos(theta);
// 	  x-=sin(theta);
// 	}
//       EField=sqrt(EvGx*EvGx+EvGy*EvGy);//kV/mm
//       velocity=medium.DriftVelocity(EField*10);// mm/microsec
//       steps+=1/(tSample*velocity);

//       //G4cout<<" step "<<steps<<" x "<<x<<" y "<<y<<" theta "<<theta<<" Gx "<<eventaction->Gx->Eval(x,y)<<" Gy "<<eventaction->Gy->Eval(x,y)<<" EField "<<EField<<" velocity "<<velocity<<G4endl;
//     }

//   //numbers
//   //EvGx=FieldLines(0,1000,1);
//   //EvGy=FieldLines(0,1000,2);
//   //EField=sqrt(EvGx*EvGx+EvGy*EvGy);//kV/mm
//   //velocity=medium.DriftVelocity(EField*10);// mm/microsec
//   //G4double quenching;
//   //quenching=medium.QuenchingFactor(2.1,EField*10);
//   //G4cout<<" Gx "<<EvGx<<" Gy "<<EvGy<<" EField "<<EField<<" velocity "<<velocity<<" quenching "<<quenching<<G4endl;


//   ret[0]=x;
//   if(yy>0)
//     ret[1]=2*y0/dt -steps;
//   else
//     ret[1]=steps; 
}


double WCSimSteppingAction::FieldLines(G4double /*x*/,G4double /*y*/,G4int /*coord*/)
{ //0.1 kV/mm = field
  //G4double Radius=302;//mm
//   G4double Radius=602;//mm
//   if(coord==1) //x coordinate
//     return  (0.1*(2*Radius*Radius*x*abs(y)/((x*x+y*y)*(x*x+y*y))));
//   else //y coordinate
//     return 0.1*((abs(y)/y)*(1-Radius*Radius/((x*x+y*y)*(x*x+y*y))) + abs(y)*(2*Radius*Radius*y/((x*x+y*y)*(x*x+y*y))));
  return 0;
}

G4String ToName(G4OpBoundaryProcessStatus boundaryStatus){
G4String processnames[14]={"Undefined","FresnelRefraction","FresnelReflection","TotalInternalReflection","LambertianReflection","LobeReflection","SpikeReflection","BackScattering","Absorption","Detection","NotAtBoundary","SameMaterial","StepTooSmall","NoRINDEX"};
return processnames[boundaryStatus];}

G4String ToName2(G4StepStatus stepStatus){
G4String processnames[8]={"fWorldBoundary","fGeomBoundary","fAtRestDoItProc","fAlongStepDoItProc","fPostStepDoItProc","fUserDefinedLimit","fExclusivelyForcedProc","fUndefined"};
return processnames[stepStatus];}

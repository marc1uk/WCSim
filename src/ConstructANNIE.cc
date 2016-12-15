 //
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: WCLiteDetectorConstruction.cc,v 1.15 2006/06/29 17:54:17 gunter Exp $
// GEANT4 tag $Name: geant4-09-01-patch-03 $
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "WCSimDetectorConstruction.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4Sphere.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4Polyhedra.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4PVParameterised.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

//#include "MRDDetectorConstruction.hh"
#include "G4SDManager.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4ReflectionFactory.hh"
#include "NCVSD.hh"
#include "G4SystemOfUnits.hh"

  #include "WCSimWCSD.hh"
  #include "WCSimPMTObject.hh"

// tank is 7GA (7-gauge steel) = (0.1793"normal sheet steel or) 0.1875" stainless steel = 4.7625mm thick 'A36' steel (density 7,800 kg/m3 )
// ^ gauge represents the thickness of 1 ounce of copper rolled out to an area of 1 square foot

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4LogicalVolume* WCSimDetectorConstruction::ConstructANNIE()
{
  
  //Decide if adding Gd
  G4cout<<"Tank is full of ";
  G4bool isNCV;
  if(WCDetectorName=="ANNIEp1"){isNCV = true;} else {isNCV=false;} // Construct the Neutron Capture Volume
  G4String watertype = "Water";
  if (WCAddGd)
    {watertype = "Doped Water";
    G4cout<<"***Doped Water***"<<G4endl;}
  else 
    {G4cout<<"Refreshing Water"<<G4endl;}
  
  // Create Experimental Hall
  //G4Box* expHall_box = new G4Box("Hall",expHall_x,expHall_y,expHall_z);
  G4Box* expHall_box = new G4Box("Hall",5*m,5*m,5*m);		//just..nicer for now.
  G4LogicalVolume* MatryoshkaMother = new G4LogicalVolume(expHall_box,G4Material::GetMaterial("Air"),"MatryoshkaMother",0,0,0);
  G4LogicalVolume* expHall_log = new G4LogicalVolume(expHall_box,G4Material::GetMaterial("Air"),"Hall",0,0,0);
  G4RotationMatrix* rotannie = new G4RotationMatrix();
  rotannie->rotateY(-90.*deg);
  G4VPhysicalVolume* expHall_phys = new G4PVPlacement(rotannie, G4ThreeVector(0,0,-1.*m), expHall_log, "Hall", MatryoshkaMother, false, 0);
  
  G4RotationMatrix* rotm = new G4RotationMatrix();
  rotm->rotateX(90*deg); // rotate Y
  G4LogicalVolume* waterTank_log;
  G4VPhysicalVolume* waterTank_phys;
  if(WCDetectorName=="ANNIEp1") {
		// Manually Create Water Tank and add PMTs just to the bottom
		//G4Tubs* aTube = new G4Tubs("Name", innerRadius, outerRadius, hz, startAngle, spanningAngle);
		G4Tubs* waterTank_tubs = new G4Tubs("waterTank",0.*m,tankouterRadius,tankhy,0.*deg,360.*deg);	//prev dims: 5.5m radius, 11m height.
		waterTank_log = new G4LogicalVolume(waterTank_tubs,G4Material::GetMaterial(watertype),"waterTank",0,0,0);
		waterTank_phys = 
		new G4PVPlacement(rotm,G4ThreeVector(0,-tankyoffset*mm,tankouterRadius+tankzoffset),waterTank_log,"waterTank",expHall_log,false,0);
		AddANNIEPhase1PMTs(waterTank_log); 
	} else { 
		waterTank_log = ConstructCylinder();
		waterTank_phys = 
		new G4PVPlacement(rotm,G4ThreeVector(0,-tankyoffset*mm,tankouterRadius+tankzoffset),waterTank_log,"waterTank",expHall_log,false,0);
	}

  // set all the paddle dimensions etc and create solids, logical volumes etc. for the MRD & VETO
  DefineANNIEdimensions();												// part of MRDDetectorConstruction.cc
  // Create MRD																		// part of MRDDetectorConstruction.cc
  useadditionaloffset=false;
  ConstructMRD(expHall_log, expHall_phys);				// marcus' MRD construction
  // if desired, enable true 'hit' sensitive detector in MRDDetectorConstruction::ConstructMRD
  
  // Create FACC
  //G4cout<<"Calling construction for the VETO"<<G4endl;
  ConstructVETO(expHall_log, expHall_phys);				// part of MRDDetectorConstruction.cc
  // enable 'true hits' sensitive detector in MRDDetectorConstruction::ConstructVETO if desired.
  
  if (isNCV){
    G4ThreeVector NCVposition;
    NCVposition.setX(0.*m); // Position of the NCV
    NCVposition.setY(0.*m);
    NCVposition.setZ(0.*m);
    ConstructNCV(waterTank_log);									// subroutine below
    G4cout << "************ Neutron Capture Volume will be included in the simulation ! ************\n";
  } else{
    G4cout << "************ Neutron Capture Volume will NOT be included in the simulation ! ********\n";
  }
  
  expHall_log->SetVisAttributes (G4VisAttributes::Invisible);	// set hall volume invisible
  
  //return expHall_phys;
  return MatryoshkaMother;	// return a logical volume not a physical one
  //return expHall_log;
}

  // =====================================================================
  // =====================================================================
  //          Phase 1 PMT additions - not using ConstructCylinder
  // =====================================================================
void WCSimDetectorConstruction::AddANNIEPhase1PMTs(G4LogicalVolume* waterTank_log){
  
	G4LogicalVolume* logicWCPMT = ConstructPMT(WCPMTName, WCIDCollectionName, "tank");
		
	/*These lines of code will give color and volume to the PMTs if it hasn't been set in WCSimConstructPMT.cc.
I recommend setting them in WCSimConstructPMT.cc. 
If used here, uncomment the SetVisAttributes(WClogic) line, and comment out the SetVisAttributes(G4VisAttributes::Invisible) line.*/
		
	G4VisAttributes* WClogic = new G4VisAttributes(G4Colour(0.4,0.0,0.8));
	WClogic->SetForceSolid(true);
	WClogic->SetForceAuxEdgeVisible(true);
	//logicWCPMT->SetVisAttributes(WClogic);
	logicWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
	
	//cellpos.rotateZ(-(2*pi-totalAngle)/2.); // align with the symmetry  // for demo of code
	G4int numrows =8, numcols=8;
	G4ThreeVector PMTpos;
	G4double PMTradih = WCPMTRadius+35*mm;	// 8" diameter + 10mm(?) either side for holder
	G4cout<<"pmt effective radius: "<< PMTradih/cm <<" cm"<<G4endl;
	G4double xoff = -(numrows-1)*PMTradih;
	G4double yoff = -tankhy+WCPMTExposeHeight;
	G4double zoff = -(numcols-1)*PMTradih;
	G4cout<<"pmt x offset: "<< xoff/cm <<" cm"<<G4endl;
	G4cout<<"pmt y offset: "<< yoff/cm <<" cm"<<G4endl;
	G4cout<<"pmt z offset: "<< zoff/cm <<" cm"<<G4endl;
	G4ThreeVector PMT00=G4ThreeVector(xoff,zoff,yoff);	// the PMT's mother volume is the tank, so these are relative only to the tank
	G4int icopy=0;
	for(G4int irow=0;irow<numrows;irow++){
		for(G4int icol=0;icol<numcols;icol++){
			if((irow==0||irow==(numrows-1))&&(icol==0||icol==(numcols-1))){
				 //4 corners have no PMTs
			} else {
				PMTpos = G4ThreeVector(2*irow*PMTradih,2*icol*PMTradih,0)+PMT00;
				G4VPhysicalVolume* physiCapPMT =
					new G4PVPlacement(	0,						// no rotation
										PMTpos,									// its position
										logicWCPMT,							// its logical volume
										"WCPMT",								// its name
										waterTank_log,					// its mother volume
										false,									// no boolean os
										icopy);									// every PMT need a unique id.
				icopy++;
			}
		}
	}
	
	//============================================================
	//                    End Phase 1 PMT additions 
	//============================================================
}

// ********************SciBooNE Geometry code******************
// ============================================================
void WCSimDetectorConstruction::DefineMRD(G4PVPlacement* expHall)
{
  //#include "DefineMRD.icc"		// calls in the sciboone code to define MRD geometry
  
  // ======================================================================================================
  // Below are extensions to SciBooNE code that add paddle cladding and SDs for PMTs at paddle ends
  // ======================================================================================================

  G4cout<<"Placing Mylar on MRD paddles"<<G4endl; 			PlaceMylarOnMRDPaddles(expHall, 0);
  
  // if desired, enable true 'hit' sensitive detector on MRD in DefineMRD.icc

  /* sciboone MRD not compatible with PMT SDs due to incorrect positioning of SD sensitive boxes @ ends of paddles!
  G4cout<<"Placing MRD PMT SDs"<<G4endl; 			PlaceMRDSDSurfs(0, expHall);
  // Create sensitive detector for pmts that will detect photon hits at the ends of the paddles
  G4VSensitiveDetector* mrdpmtSD = new mrdPMTSD("MRDPMTSD"); 
  // Register detector with manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  SDman->AddNewDetector(mrdpmtSD);
  // gets called manually and tracks get killed before they enter it; don't need to associate with any logical volumes
  
  // alternatively use WCSim style PMTs
  //G4cout<<"Placing MRD PMTs"<<G4endl; 			PlaceMRDPMTs(0, expHall);
  G4cout<<"done placing mrd pmts"<<G4endl;  
  */
  
}
// *******************/End SciBooNE integration****************
// ============================================================

void WCSimDetectorConstruction::ConstructNCV(G4LogicalVolume* waterTank_log){
  //===============================================================================================================
  //NEUTRON CAPTURE VOLUME DEFINITION
  //===============================================================================================================
  // NCV is defined as two cylinders, one filled with liquid, the other with acrylic. Two 'caps' are added to the 
  // top of the NCV as well. 
  // The metal structure will be added as well (from pictures) since it will induce n-Fe captures
  //===============================================================================================================

  // Dimensions 
  G4double NCVliquid_radius = 25.*cm;
  G4double NCVliquid_height = 50.*cm;
  G4double NCVvessel_thickness = 1.*cm;
  G4double NCVvesselcap_thickness = 3.*cm;
  
  // NCV acrylic vessel
  // Cylinder
  G4VSolid* NCVvessel_tub
    = new G4Tubs("NCVvessel_tub",
		    0.,
		    NCVliquid_radius + NCVvessel_thickness, // r1, r2
		    NCVliquid_height/2., //Half height of the cylinder
		    0.0, 2.0*M_PI         // phi0, delta_phi
		    );

  G4LogicalVolume* NCVvessel_log
    = new G4LogicalVolume(NCVvessel_tub,
			  G4Material::GetMaterial("Acrylic"),
			  "NCVvessel_log",
			  0,
			  0,
			  0);
  G4VisAttributes* NCVvessel_vis
    = new G4VisAttributes(G4Color(0.1,0.,1.0,1));
  NCVvessel_log -> SetVisAttributes(NCVvessel_vis);


  G4VPhysicalVolume* NCVvessel_phys
  = new G4PVPlacement(0,  // no rotation
			G4ThreeVector(0.,0.,0.), // shifted in the water tank
			NCVvessel_log,
			"NCVvessel_phys",
			waterTank_log,          // mother
			false,
			0);
  
  // Caps
  G4VSolid* NCVvesselcap_box
    = new G4Box("NCVvesselcap_box",
		    NCVliquid_radius, // x
		    NCVliquid_radius, // y
		    NCVvesselcap_thickness/2. // thickness
		    );
    G4LogicalVolume* NCVvesselcap_log
    = new G4LogicalVolume(NCVvesselcap_box,
			  G4Material::GetMaterial("Acrylic"),
			  "NCVvesselcap_log",
			  0,
			  0,
			  0);
  G4VisAttributes* NCVvesselcap_vis
    = new G4VisAttributes(G4Color(0.1,0.,1.0,1));
  NCVvesselcap_log -> SetVisAttributes(NCVvesselcap_vis);

  G4VPhysicalVolume* NCVvesselcap1_phys
  = new G4PVPlacement(0,  // no rotation
			G4ThreeVector(0.,0.,NCVliquid_height/2. + NCVvesselcap_thickness/2.), // top of the cylinder
			NCVvesselcap_log,
			"NCVvesselcap1_phys",
			waterTank_log,          // mother
			false,
			0);
  
  G4VPhysicalVolume* NCVvesselcap2_phys
  = new G4PVPlacement(0,  // no rotation
			G4ThreeVector(0.,0., - NCVliquid_height/2. - NCVvesselcap_thickness/2.), // bottom of the cylinder
			NCVvesselcap_log,
			"NCVvesselcap2_phys",
			waterTank_log,          // mother
			false,
			0);
    
  
  // NCV liquid volume
     G4VSolid* NCVliquid_tub
      = new G4Tubs("NCVliquid_tub", //name
  		     0., // tube radius
  		     NCVliquid_radius, // r1, r2
		     NCVliquid_height/2., //Half height of the cylinder
  		     0.0, 2.0*M_PI         // phi0, delta_phi
  		     );

    G4LogicalVolume* NCVliquid_log
      = new G4LogicalVolume(NCVliquid_tub,
  			    G4Material::GetMaterial("NCVliquid"),
  			    "NCVliquid_log",
  			    0,
  			    0,
  			    0);
    G4VisAttributes* NCVliquid_vis
      = new G4VisAttributes(G4Color(0.1,1.0,0.,1));
    NCVliquid_log -> SetVisAttributes(NCVliquid_vis);
  
    G4VPhysicalVolume* NCVliquid_phys
     = new G4PVPlacement(0,  // no rotation
  			  G4ThreeVector(), // centered in the vessel
  			  NCVliquid_log,
			  "NCVliquid_phys",
  			  NCVvessel_log,          // mother
  			  false,
  			  0);
  			  
  // Make liquid scintillator a sensitive detector
  //----------------------------------------------
  // Get pointer to detector manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  
  // Create a new instance of NCV sensitive detector
  G4VSensitiveDetector* ncvSD = new NCVSD("NeutronCaptureVolume"); 
  // Register detector with manager
  SDman->AddNewDetector(ncvSD);
  // Attach detector to liquid scintillator volume
  NCVliquid_log->SetSensitiveDetector(ncvSD);

}

//#ifndef __CONSTRUCT_MRD_PMT_VERBOSE__
//#define __CONSTRUCT_MRD_PMT_VERBOSE__ 1
//#endif

#include "WCSimDetectorConstruction.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Torus.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4Polycone.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalBorderSurface.hh"

#include "G4SDManager.hh"
#include "WCSimWCSD.hh"
#include "WCSimPMTObject.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

//Flat faced PMT logical volume construction. Based on tank construction, but with different PMT geometry.

G4LogicalVolume* WCSimDetectorConstruction::ConstructFlatFacedPMT(G4String PMTName, G4String CollectionName, G4String detectorElement)
{
#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"Making PMTKey_t (PMTName="<<PMTName<<", CollectionName="<<CollectionName<<") "<<G4endl;
#endif
  PMTKey_t key(PMTName,CollectionName);
  PMTMap_t::iterator it = PMTLogicalVolumes.find(key);
  if (it != PMTLogicalVolumes.end()) {
#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
      G4cout<<"This key already exists in the PMTLogicalVolumes map. Restoring it."<<G4endl;
#endif
      //G4cout << "Restore PMT" << G4endl;
      return it->second;
  }
  //G4cout << "Create PMT" << G4endl;

#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
G4cout<<"Key not found; creating a new PMT logical volume"<<G4endl;
#endif

  G4double expose;
  G4double radius;
  G4double glassThickness;
#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"Getting pointer to PMT object corresponding to CollectionName "<<CollectionName<<G4endl;
#endif
  WCSimPMTObject *PMT = GetPMTPointer(CollectionName);
#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"Retrieved PMT with name "<<PMT->GetPMTName()<<G4endl;
#endif
  expose = PMT->GetExposeHeight();							// net thickness of the construction
  radius = PMT->GetRadius();
  glassThickness = PMT->GetPMTGlassThickness();
  G4double shamferrad=PMT->GetShamferRadius();	// arbitrary choice for now
  G4double gelthickness=PMT->GetGelThickness();		// equally arbitrary
//G4cout<<"making flat faced pmt of radius "<<radius/cm<<" with shamferrad "<<shamferrad/cm<<" and glass thickness "<<glassThickness/cm<<" and gel thickness "<<gelthickness/cm<<G4endl;

#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
G4cout<<"Making the geometry"<<G4endl;
#endif

// start with a box of silicon 'optical grease' gel - this will smoosh against the light guide
G4Box* PMTgelbox = new G4Box("WCPMT",
                            radius+1.*cm,
                            radius+1.*cm,
                            expose/2.);

G4LogicalVolume* logicWCPMT = new G4LogicalVolume(PMTgelbox,
                            G4Material::GetMaterial("Silicone"),
                            "FFPMT",
                            0,0,0);

if (Vis_Choice == "RayTracer"){
// Makes the volume containing the PMT visible, solid, and forces the auxiliary edges to be viewed.
  G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
  WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
  WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown 

    logicWCPMT->SetVisAttributes(WCPMTVisAtt);}

else{
// Makes the volume containg the PMT invisible for normal visualization
    logicWCPMT->SetVisAttributes(G4VisAttributes::Invisible);}

// now make up the PMT glass face from a series of parts
// first a torus to make up the chamfered edge
G4Torus* tmptorus = new G4Torus("tmptorus",
                            0,
                            shamferrad,
                            radius-shamferrad,
                            0,
                            2.0*pi);

// a solid disk to make up the main face of the PMT
G4Tubs* PMTface = new G4Tubs("PMTface",
                          0,
                          radius-shamferrad,
                          shamferrad+0.1*cm,
                          0,
                          2.0*pi);

// generate a union between the disk face and the remaining quarter torus acting as the chamfered edge
G4UnionSolid* roundedPMTface = new G4UnionSolid(CollectionName, 
                    tmptorus,
                    PMTface);

// generate a disk to remove the lower half
G4Tubs* tmpbottomhalf = new G4Tubs("tmpbottomhalf",
                          0,
                          radius+1.*cm,
                          shamferrad,
                          0,
                          2.0*pi);

G4Transform3D transform = G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,-shamferrad));

// subtract the lower half of the torus
G4SubtractionSolid* roundedPMTfacelowerhalf = new G4SubtractionSolid("roundedPMTfacelowerhalf",
                    roundedPMTface,
                    tmpbottomhalf,
                    transform);

// make the logical volume to turn this PMT glass face into glass...
G4LogicalVolume* logicGlassFaceWCPMT = new G4LogicalVolume(roundedPMTfacelowerhalf,
                    G4Material::GetMaterial("Glass"),
                    CollectionName,
                    0,0,0);

// this is a solid 'frisbee' of glass. We need to place a physical daughter volume of air into the logical glass to 
// displace it's internal cavity. So the same process again, this time for a slightly smaller 'frisbee' of air.

// make 'air' to go behind the glass cathode surface, as per other PMTs... dunno why it's not a vacuum.
G4Torus* airtorus = new G4Torus("airtorus",
                            0,
                            shamferrad-glassThickness,
                            radius-shamferrad,
                            0,
                            2.0*pi);

// define the air disk
G4Tubs* airdisk = new G4Tubs("airdisk",
                          0,
                          radius-shamferrad,
                          shamferrad-glassThickness+0.1*cm,
                          0,
                          2.0*pi);

// combine with the air torus
G4UnionSolid* aircavity = new G4UnionSolid("aircavity",
                          airtorus,
                          airdisk);

// subtract the lower half
G4SubtractionSolid* aircavitylowerhalf = new G4SubtractionSolid("aircavitylowerhalf",
                    aircavity,
                    tmpbottomhalf,
                    transform);

G4LogicalVolume* aircavity_log = new G4LogicalVolume(aircavitylowerhalf,
                    G4Material::GetMaterial("Air"),
                    "aircavity_log",
                    0,0,0);

// place the half air torus into half glass-face, displacing the internal cavity with air
G4VPhysicalVolume* aircavity_phys = new G4PVPlacement(0,
                        G4ThreeVector(0, 0, 0.),
                        aircavity_log,
                        "aircavity_phys",
                        logicGlassFaceWCPMT,
                        false,
                        0,
                        false);

// place the glass face logical volume into the box of optical grease
G4VPhysicalVolume* physiGlassFaceWCPMT = new G4PVPlacement(0,
                        G4ThreeVector(0, 0, -((shamferrad+0.1*cm)/2)),
                        logicGlassFaceWCPMT,
                        CollectionName,
                        logicWCPMT,
                        false,
                        0,
                        false);

/* ###### note: the solid, logical and physical volumes corresponding to the glass PMT face are all 
given the name CollectionName, which is passed in when ConstructPMT is called! ########### */

// give visualization properties to the glass face
G4VisAttributes* WCPMTVisAtt;
if (Vis_Choice == "RayTracer"){
	WCPMTVisAtt = new G4VisAttributes(G4Colour(0,0.5,1.));
	WCPMTVisAtt->SetForceSolid(true); 					// force the object to be visualized with a surface
	WCPMTVisAtt->SetForceAuxEdgeVisible(true); 	// force auxiliary edges to be shown
} else { // Gray wireframe visual style used in OGLSX visualizer
	WCPMTVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
	WCPMTVisAtt->SetForceWireframe(true);
}
logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);

// For either visualization type, logicGlassFaceWCPMT will either be visible or invisible depending on which
// line is commented at the end of the respective if statements

////////  if (Vis_Choice == "OGLSX")
////////   { // Gray wireframe visual style
////////    // used in OGLSX visualizer
////////  G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
////////  WCPMTVisAtt->SetForceWireframe(true);
////////  //logicGlassFaceWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
////////  logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);}

////////  if (Vis_Choice == "RayTracer"){
////////    // Blue wireframe visual style
////////    // Used in the RayTracer visualizer
////////  G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
////////  WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
////////  WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown 
////////  //logicGlassFaceWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
////////  logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);}

#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"Finished making the geometry"<<G4endl;
#endif
// end of physical construction
// ============================

  // Instantiate a new sensitive detector 
  // and register this sensitive detector volume with the SD Manager. 
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDName = "/WCSim/";
  SDName += CollectionName;

#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"Searching for sensitive detector "<<SDName<<G4endl;
#endif
  // If there is no such sensitive detector with that SDName yet,
  // make a new one
  if( ! SDman->FindSensitiveDetector(SDName, false) ) {
#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
    G4cout<<"Sensitive detector not found. Making aWCPMT = new WCSimWCSD("<<CollectionName<<", "<<SDName
          <<", "<<"{DetectorConstruction}"<<", "<<detectorElement<<"), and adding to the SD manager"<<G4endl;
#endif
    aMRDPMT = new WCSimWCSD(CollectionName,SDName,this, detectorElement );
    SDman->AddNewDetector( aMRDPMT );
  }

  logicGlassFaceWCPMT->SetSensitiveDetector( aMRDPMT );

  PMTLogicalVolumes[key] = logicWCPMT;

#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"Adding optical surfaces to cathode"<<G4endl;
#endif
  //Add Logical Border Surface
  new G4LogicalBorderSurface("GlassCathodeSurface",
                             physiGlassFaceWCPMT,
                             aircavity_phys,
                             OpGlassCathodeSurface);

#ifdef __CONSTRUCT_MRD_PMT_VERBOSE__
  G4cout<<"returning logical volume"<<G4endl;
#endif
  return logicWCPMT;
}

#include "WCSimDetectorConstruction.hh"

#include "G4Transform3D.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4Polycone.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalBorderSurface.hh"

#include "G4SDManager.hh"
#include "WCSimWCSD.hh"
#include "WCSimLAPPDObject.hh"

#include "G4SystemOfUnits.hh"

//LAPPD logical volume construction.

WCSimDetectorConstruction::LAPPDMap_t WCSimDetectorConstruction::LAPPDLogicalVolumes;

G4LogicalVolume* WCSimDetectorConstruction::ConstructLAPPD(G4String LAPPDName, G4String CollectionName2)
{
  LAPPDKey_t key(LAPPDName,CollectionName2);

  LAPPDMap_t::iterator it = LAPPDLogicalVolumes.find(key);
  if (it != LAPPDLogicalVolumes.end()) {
      //G4cout << "Restore LAPPD" << G4endl;
      return it->second;
  }

  //G4cout << "Create LAPPD" << G4endl;


if (Vis_Choice == "RayTracer"){
    // Blue wireframe visual style
    // Used in the RayTracer visualizer
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
  WCLAPPDVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
  WCLAPPDVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown 
}

else
   { // Gray wireframe visual style
    // used in OGLSX visualizer
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
  WCLAPPDVisAtt->SetForceWireframe(true);}

  G4double expose;
  G4double radius;
  G4double glassThickness;
  
  WCSimLAPPDObject *LAPPD = GetLAPPDPointer(CollectionName2);
  expose = LAPPD->GetExposeHeight(); //here expose is the LAPPD half height
  radius = LAPPD->GetRadius();
  glassThickness = LAPPD->GetLAPPDGlassThickness();

  G4double sphereRadius = radius; //radius is actually half length in x,y
  G4double extrudedRadius = 120.*cm;  // an artificially looong LAPPD vertically, for use with a masking system in analysis
  //(expose*expose+ radius*radius)/(2*expose);
  G4double LAPPDOffset = 0.;//expose-glassThickness/2.; //centre of glass position coordinates

  //All components of the LAPPD are now contained in a single logical volume logicWCLAPPD.
  //Origin is on the blacksheet, faces positive z-direction.
  
  G4Box* solidWCLAPPD = 
   new G4Box("WCLAPPD",                    
	     extrudedRadius+0.015*m, //half length in x ~10.3 cm //0.5cm is the sidewall border and the glass exceeds by ~1cm. 
	     sphereRadius+0.015*m, //half length in y ~ 10.3cm
	     expose+0.005*m //half length in z
	     ); //

  G4LogicalVolume* logicWCLAPPD =
    new G4LogicalVolume(    solidWCLAPPD,
                            G4Material::GetMaterial("Water"),
                            "WCLAPPD",
                            0,0,0);

if (Vis_Choice == "RayTracer"){
// Makes the volume containing the LAPPD visible, solid, and forces the auxiliary edges to be viewed.
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
  WCLAPPDVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
  WCLAPPDVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown 

    logicWCLAPPD->SetVisAttributes(WCLAPPDVisAtt);}

else{
// Makes the volume containg the LAPPD invisible for normal visualization
    logicWCLAPPD->SetVisAttributes(G4VisAttributes::Invisible);
 }
//Need a volume to cut away excess behind blacksheet-for LAPPDs no need for that 
// G4Box* solidCutOffTubs =
//   new G4Box(    "cutOffLAPPDs",
//		 0.01*m,
//		 0.01*m,
//		 0.01*m);

//LAPPD active area
  G4Box* tmpSolidInteriorWCLAPPD = 
   new G4Box("tmpInteriorWCLAPPD",                    
	     extrudedRadius,              //half length in x ~ 10.15cm
	     sphereRadius,              //half length in y ~ 10.15cm
	     (expose-glassThickness) //half length in z
	     );//

  G4Transform3D transform = G4Transform3D(G4RotationMatrix(), G4ThreeVector(-1*m, -1*m, -1*m));
//  G4SubtractionSolid* solidInteriorWCLAPPD =
//      new G4SubtractionSolid(    "InteriorWCLAPPD",
//				 tmpSolidInteriorWCLAPPD,
//				 solidCutOffTubs,
//                                 transform);

  // "Air" here is not true air, but a modified material
  // with n = 1 and a very short absorption length
  G4LogicalVolume* logicInteriorWCLAPPD =
    new G4LogicalVolume(    tmpSolidInteriorWCLAPPD, //solidInteriorWCLAPPD,
			    G4Material::GetMaterial("Air"),
			    "InteriorWCLAPPD",
			    0,0,0);

//  //G4cout<<"From ConstructLAPPD: LAPPDOffset= "<<LAPPDOffset<<" expose= "<<expose<<G4endl;
//  G4VPhysicalVolume* physiInteriorWCLAPPD =
//      new G4PVPlacement(0,
//			G4ThreeVector(0, 0, 0),
//			logicInteriorWCLAPPD,
//			"InteriorWCLAPPD",
//			logicWCLAPPD,
//			false,
//			0);

if (Vis_Choice == "RayTracer"){
// Adding color and forcing the inner portion of the LAPPD's to be viewed
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
  WCLAPPDVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
  WCLAPPDVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown 

  logicInteriorWCLAPPD->SetVisAttributes(WCLAPPDVisAtt);}

else {
// Making the inner portion of the detector invisible for OGLSX visualization
  logicInteriorWCLAPPD->SetVisAttributes(G4VisAttributes::Invisible);
 }
//we ned that to substract the LAPPD active area to get the glass volume
//G4Box* solidCutOffTubs2 =
//   new G4Box(    "cutOffLAPPDsglass",
//                 extrudedRadius,
//                 sphereRadius,
//                 expose);//-glassThickness));

//Create LAPPD Glass Face
 G4Box* tmpGlassFaceWCLAPPD = 
   new G4Box("tmpGlassFaceWCLAPPD",                    
	     (extrudedRadius + 0.0085*m), //half length in x
	     (sphereRadius + 0.013*m), //half length in y
	     (expose)//+glassThickness)       //half length in z
	     );//

// G4SubtractionSolid* solidGlassFaceWCLAPPD =
//   new G4SubtractionSolid(    CollectionName2,
//			      tmpGlassFaceWCLAPPD,
//			      solidCutOffTubs//,
//			      //transform  
//			); 

 G4LogicalVolume *logicGlassFaceWCLAPPD =
   new G4LogicalVolume(    tmpGlassFaceWCLAPPD, //solidGlassFaceWCLAPPD,
			   G4Material::GetMaterial("Glass"),
			   CollectionName2,
			   0,0,0);

 G4VPhysicalVolume* physiInteriorWCLAPPD =
    new G4PVPlacement(0,
		G4ThreeVector(0, 0, -glassThickness),
		logicInteriorWCLAPPD,
		"InteriorWCLAPPD",
		logicGlassFaceWCLAPPD,
		false,
		0);

 G4VPhysicalVolume* physiGlassFaceWCLAPPD =
   new G4PVPlacement(0,
		     G4ThreeVector(0, 0, LAPPDOffset),
		     logicGlassFaceWCLAPPD,
		     CollectionName2,
		     logicWCLAPPD,
		     false,
		     0,
		     checkOverlaps);

// For either visualization type, logicGlassFaceWCLAPPD will either be visible or invisible depending on which
// line is commented at the end of the respective if statements

  if (Vis_Choice == "OGLSX")
   { // Gray wireframe visual style
    // used in OGLSX visualizer
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
  WCLAPPDVisAtt->SetForceWireframe(true);
  // //logicGlassFaceWCLAPPD->SetVisAttributes(G4VisAttributes::Invisible);
    // logicGlassFaceWCLAPPD->SetVisAttributes(WCLAPPDVisAtt);
   }

  if (Vis_Choice == "RayTracer"){
    // Blue wireframe visual style
    // Used in the RayTracer visualizer
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
  WCLAPPDVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
  WCLAPPDVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown 
  ////logicGlassFaceWCLAPPD->SetVisAttributes(G4VisAttributes::Invisible);

  //logicGlassFaceWCLAPPD->SetVisAttributes(WCLAPPDVisAtt);
  }

  else
   { // Gray wireframe visual style
    // used in OGLSX visualizer
  G4VisAttributes* WCLAPPDVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
  WCLAPPDVisAtt->SetForceWireframe(true);
  ////logicGlassFaceWCLAPPD->SetVisAttributes(G4VisAttributes::Invisible);
  //logicGlassFaceWCLAPPD->SetVisAttributes(WCLAPPDVisAtt);
   }

  // Instantiate a new sensitive detector 
  // and register this sensitive detector volume with the SD Manager. 
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDName = "/WCSim/";
  SDName += CollectionName2;
  //G4cout<<"-------- testttttt: "<< SDName << G4endl;
  // If there is no such sensitive detector with that SDName yet,
  // make a new one
  if( ! SDman->FindSensitiveDetector(SDName, false) ) {
    // G4cout<<"-------- test2: "<<CollectionName2<<G4endl;
    aWCLAPPD = new WCSimWCSD(CollectionName2,SDName,this,"tank" );
    SDman->AddNewDetector( aWCLAPPD );
  }

  logicGlassFaceWCLAPPD->SetSensitiveDetector( aWCLAPPD );
  LAPPDLogicalVolumes[key] = logicWCLAPPD;

  //Add Logical Border Surface
  new G4LogicalBorderSurface("GlassCathodeSurface",
                             physiGlassFaceWCLAPPD,
                             physiInteriorWCLAPPD,
                             OpGlassCathodeSurface);
  
  return logicWCLAPPD;
}

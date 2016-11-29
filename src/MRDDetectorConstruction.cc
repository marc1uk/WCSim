// ====================================================================
//   MRDDetectorConstruction.cc
//
//   17/11/15 M. O'Flaherty
// ====================================================================
//===============================================================================================================
  //MRD DETECTOR DEFINITION
//===============================================================================================================

#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4OpBoundaryProcess.hh" 
#include "G4PVParameterised.hh"
//#include "MRDDetectorConstruction.hh"
#include "WCSimDetectorConstruction.hh"
#include "G4SDManager.hh"
#include "MRDSD.hh"
#include "mrdPMTSD.hh"
#include "FACCSD.hh"
#include "faccPMTSD.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4AssemblyVolume.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"

// Define Variables that give MRD (and some other) specifications
//===============================================================
void WCSimDetectorConstruction::DefineANNIEdimensions(){

	Xposition=0, Yposition=0, Zposition=0;		// used for positioning parameterisations.
	numpaddlesperpanelh=26;								// paddles per h scintillator panel
	numpaddlesperpanelv=30;								// paddles per v scintillator panel
	numpanels=12;													// scintillator panels
	numrpcs=10;														// rpc panels
	numplates=12;													// steel plates
	numalustructs=13;											// number of supporting structs. We may be dropping one as we have fewer scintillators?
	numvetopaddles=26;										// number of scintillator paddles in the FACC; 13 panels in 2 layers
	vetopaddlesperpanel=13;								// number of scintillator paddles in each FACC panel

	scintbordergap=0.3*cm;								// gap between each scintillator (cm) (border to account for cladding etc)
	steelscintgap=0.5*cm;									// gap between steel and scintillator
	scintalugap=0.2*cm;										// gap between scintillator and alu struct
	alusteelgap=2.0*cm; 									// gap between alu struct and subsequent steel of next layer
	layergap = steelscintgap + scintalugap + alusteelgap;	// total gaps of layers

	steelfullxlen = 305*cm;
	steelfullylen = 274*cm;
	steelfullzlen = 5*cm;

	scintfullxlen = 20*cm;
	scintfullzlen= 0.6*cm;
	scinthfullylen = 147.2*cm; //155cm - 7.8cm tapered section		147.1 according to sciboone gdml export		//swapped???
	scintvfullylen= 130.2*cm;  //138cm - 7.8cm tapered section		129.4 according to sciboone gdml export

	scinttapfullwidth = 17.1*cm; 			// width of the tapering part of scintillator paddles at the narrow end
	scinttapfullheight = 7.8*cm; 			// z length of tapering part of scint paddles.

	scintlgfullwidth = 5.08*cm; 			// tapered light guides at narrow end
	scintlgfullheight = 33.3*cm; 			// 

	alufullxlen = steelfullxlen+15*cm;	// outer thicknesses - total frame dims are about those of the steel plate
	alufullylen = steelfullylen+15*cm;	// basically making this up
	alufullzlen = 3.81*cm;							// from eye and a skim of the mrdmodule.txt file, i'm guessing depth is ~0.75 inches (1.9cm)
	alufullxthickness = 2.54*cm;				// as above, guessing frame to be 1 inch box cross-section
	alufullythickness = 2.54*cm;
	windowwidth = (steelfullxlen-(4*alufullxthickness))/3;	// (full length - 4 beams) / 3 windows
	windowheight= (steelfullylen-(4*alufullythickness))/3;
	
	mrdZlen = numplates*steelfullzlen + (numpanels+1)*scintfullzlen + numalustructs*alufullzlen + numpanels*layergap + scintalugap + MRDPMTRadius; 
	// add another panel to full length because last alu struct is placed back assuming it's there. Maybe need to change... 

	vetopaddlefullxlen = 320.0*cm;
	vetopaddlefullylen = 30.5*cm;
	vetopaddlefullzlen = 2.0*cm;
	vetolayer2offset = 1.27*cm;
	vetopaddlegap = 0.2*cm;
	nothickness = 0.01*cm;
	
	vetolgfullylen = 10.*cm;
	vetolgfullxlen = 50.*cm;

	vetoZlen = 2*vetopaddlefullzlen+vetopaddlegap;
	if((2.*FACCPMTRadius+vetopaddlegap)>vetoZlen){vetoZlen=(2.*FACCPMTRadius+vetopaddlegap);}

	//extern G4double tankouterRadius;

	// following measurements taken with tape measure; can be used to derive gaps. 
	// veto wall full height measured to be 158.75" = 403.2cm / 13 paddles = 31.1cm each in y height
	// paddle full x length measured to be 126.5" = 321.3cm in square section, followed by taper
	// paddle full z length measured to be 1" = 2.54cm;
	// taper length measured to be 14.5" = 36.8cm
	// taper narrow width measured to be 2" = 5.1cm (diameter of PMT)
	// taper depth at narrow end is difficult to measure due to interface between PMT and paddle/LG - assuming same
	// layer offset is 0.5" = 1.27cm

	/*
	Allowing for a typical deterioration rate of 5–10% per year, full efficiency should be retained more than 10 years and it will be over than useful lifetime of CDF. .. The technique relies on a wavelength shifter fibers to extract the light from the longer side of the scintillator bar. ... the scintillator used to construct the counters (UPS 923A) is a polystyrene-based plastic...The results of quality control tests performed at JINR show that the average ligh output ranges between 21ph.e./MIP (for the longest counters) for muons traversing the counters transversely at the furthest ends from the photomultipliers.
	*/

	mrdpmtfullheight = MRDPMTExposeHeight;
	G4double widths[] = {2*(scinthfullylen+scinttapfullheight+scintlgfullheight+(scintbordergap/2)+mrdpmtfullheight+nothickness),((numpaddlesperpanelv/2)*(scintfullxlen+scintbordergap))};	
	// 2* and y dim because we stack 2 rotated h scint paddles end-to-end. 
	//
	G4double heights[] = {2*(scintvfullylen+scinttapfullheight+scintlgfullheight+(scintbordergap/2)+mrdpmtfullheight+nothickness),((numpaddlesperpanelh/2)*(scintfullxlen+scintbordergap))};
	//
	maxwidth = *std::max_element(widths,widths+(sizeof(widths)/sizeof(widths[0])))+0.1*cm;
	maxheight = *std::max_element(heights,heights+(sizeof(heights)/sizeof(heights[0])))+0.1*cm;
	
	mrdZoffset = (2*tankouterRadius) + tankzoffset + (mrdZlen/2.) + 5*cm;
		      
	// Define solids 
	//==============  
	// G4Box* variableName = new G4Box("SolidName", x_halflength, y_halflength, z_halflength);
	// G4Trd("SolidName", x_halflength1, x_halflength2, y_halflength1, y_halflength2, z_halflength); 
	// 2 x and y dimensions given - define dims of the cross-sections at the two z ends. 

	// Paddles - h and v
	sciMRDhpaddle_box = new G4Box("scintHpaddle",scintfullxlen/2,scinthfullylen/2,scintfullzlen/2);
	sciMRDvpaddle_box = new G4Box("scintVpaddle",scintfullxlen/2,scintvfullylen/2,scintfullzlen/2);

	// Paddle Tapered ends
	mrdScintTap_box = new G4Trd("mrdScintTap_box", scintfullxlen/2, scinttapfullwidth/2, scintfullzlen/2, scintfullzlen/2, scinttapfullheight/2);	

	// Tapered Light Guides - same code as tapered ends. re-use rotations matrices as they're the same.
	mrdLG_box = new G4Trd("mrdLG_box", scinttapfullwidth/2, scintlgfullwidth/2, scintfullzlen/2, scintfullzlen/2, scintlgfullheight/2);	
	
	// Little boxes to go on the ends of the light guides - photons entering these will be killed at the boundary & recorded
	mrdSurface_box = new G4Box("mrdSurface_box",scintlgfullwidth/2,scintfullzlen/2,nothickness/2);

	// Steel plates
	steelMRDplate_box = new G4Box("steelPlate",steelfullxlen/2,steelfullylen/2,steelfullzlen/2);

	//The alu support structure is roughly the external size of the steel plates...
	aluMRDstruc_box = new G4Box("outer_Box", alufullxlen/2, alufullylen/2, alufullzlen/2);

	// ...with a 3x3 grid of ~even sized holes  
	aluMRDwindow_box = new G4Box("inner_Box", windowwidth/2, windowheight/2, alufullzlen/2);

	totMRD_box = new G4Box("totMRD",(maxwidth/2),(maxheight/2),mrdZlen/2);

	vetoPaddle_box = new G4Box("vetoPaddle_box",vetopaddlefullxlen/2, vetopaddlefullylen/2, vetopaddlefullzlen/2);
	vetoSurface_box = new G4Box("vetoSurface_box",nothickness/2,vetolgfullylen/2,vetopaddlefullzlen/2);
	vetoLG_box = new G4Trd("vetoLG_box", vetopaddlefullylen/2, vetolgfullylen/2, vetopaddlefullzlen/2, vetopaddlefullzlen/2, vetolgfullxlen/2);

	vetopmtfullheight = FACCPMTExposeHeight;
	totVeto_box = new G4Box("totVeto_box", (vetopaddlefullxlen/2)+vetolgfullxlen+vetopmtfullheight+nothickness+vetopaddlegap*2, (((vetopaddlefullylen+vetopaddlegap)*vetopaddlesperpanel)+vetolayer2offset)/2+vetopaddlegap*2+FACCPMTRadius*2, vetoZlen);

	// Define rotation matrices 
	//=========================
	// rotated and unrotated scintillator paddles. Could do away with one by changing dims but hey.
	noRot = new G4RotationMatrix();											// null rotation pointer
	rotatedmatx = new G4RotationMatrix(0,0,90*deg);			// horizontal config for scint paddles

	// Note trapezium narrows along z axis, so need to define a rotation of 90deg about the y(??)-axis to bring taper into the x-y plane. 
	upmtx = new G4RotationMatrix(180*deg,90*deg,0*deg);
	downmtx = new G4RotationMatrix(0*deg,90*deg,0*deg);
	rightmtx = new G4RotationMatrix(90*deg,90*deg,0*deg);
	leftmtx = new G4RotationMatrix(-90*deg,90*deg,0*deg);

	// Define Visualisation Attributes 
	//================================
	scinthatts = new G4VisAttributes(G4Colour(0.5,0.0,1.0));      // pink
	scintvatts = new G4VisAttributes(G4Colour(1.0,0.0,0.5));      // purple
	steelatts = new G4VisAttributes(G4Colour(0.0,1.0,1.0));       // blue?
	scinttapatts = new G4VisAttributes(G4Colour(0.6, 1.0, 0.8));  // light-green
	scintlgatts = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));   // grey
	senssurfatts = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0));  // yellow
	
  // Define Mylar cladding
  // =====================
  // For the optical boundary process to use a border surface, the two volumes must have been positioned with G4PVPlacement. 
  scintSurface_op = new G4OpticalSurface("mylarSurface",unified, polishedbackpainted, dielectric_dielectric);

/*
  scintSurface_op = new G4OpticalSurface("mylarSurface"
  scintSurface_op->SetType(dielectric_LUT);
  scintSurface_op->SetModel(LUT);
  scintSurface_op->SetFinish(polishedvm2000air);
*/
  const G4int mylarmptentries = 2;
//  G4double mylar_Energy[mylarmptentries] = {2.0*eV, 3.6*eV};
//  G4double mylar_reflectivity[mylarmptentries] = {0.9,0.9};
//  G4double mylar_efficiency[mylarmptentries] = {0.0, 0.0};

// from http://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/ForApplicationDeveloper/html/ch05s02.html
	G4double mylar_Energy[mylarmptentries] = {2.038*eV, 4.144*eV};
	G4double mylar_rindex[mylarmptentries] = {1.0, 1.0};
	G4double mylar_specularlobe[mylarmptentries] = {0.1, 0.1};
	G4double mylar_specularspike[mylarmptentries] = {0.8, 0.8};
	G4double mylar_backscatter[mylarmptentries] = {0.01, 0.01};
	G4double mylar_reflectivity[mylarmptentries] = {0.9, 0.9};
	//G4double mylar_efficiency[mylarmptentries] = {0.8, 0.1};

  G4MaterialPropertiesTable* MPTmylarSurface = new G4MaterialPropertiesTable();
  MPTmylarSurface->AddProperty("REFLECTIVITY",mylar_Energy,mylar_reflectivity,mylarmptentries);
  //MPTmylarSurface->AddProperty("EFFICIENCY",mylar_Energy,mylar_efficiency,mylarmptentries);
  // i think this actually relates to the airgap implied by 'polishedbackpainted'
	MPTmylarSurface -> AddProperty("RINDEX",mylar_Energy,mylar_rindex,mylarmptentries);
	// and these relate to the degree of roughness of the finish of the scintillator paddle, not the mylar
	MPTmylarSurface -> AddProperty("SPECULARLOBECONSTANT",mylar_Energy,mylar_specularlobe,mylarmptentries);
	MPTmylarSurface -> AddProperty("SPECULARSPIKECONSTANT",mylar_Energy,mylar_specularspike,mylarmptentries);
	MPTmylarSurface -> AddProperty("BACKSCATTERCONSTANT",mylar_Energy,mylar_backscatter,mylarmptentries);
	// the mylar itself is assumed perfectly mirrored - specular spike constant 1, everything else 0

	G4double sigma_alpha = 0.1;
	scintSurface_op -> SetSigmaAlpha(sigma_alpha);	// defines roughness granularity of paddle...

  scintSurface_op->SetMaterialPropertiesTable(MPTmylarSurface);
  
  // Define efficiency for reflection & detection between paddles and PMTs at their ends
  // ===================================================================================
  lgSurface_op = new G4OpticalSurface("lgopsurface",glisur, polished, dielectric_metal);  
  const G4int lgmptentries = 2;
  G4double ELGphoton[lgmptentries] = {2.038*eV, 4.144*eV};
  G4double lgsurf_EFF[lgmptentries]={0.95,0.95};
  G4double lgsurf_REFL[lgmptentries]={0.,0.};
  G4MaterialPropertiesTable* lgsurf_MPT = new G4MaterialPropertiesTable();
  lgsurf_MPT->AddProperty("EFFICIENCY",  ELGphoton, lgsurf_EFF,  lgmptentries);
  lgsurf_MPT->AddProperty("REFLECTIVITY",ELGphoton, lgsurf_REFL, lgmptentries);
  lgSurface_op->SetMaterialPropertiesTable(lgsurf_MPT);

}

  //===============================================================================================================
  // MRD DETECTOR DEFINITION
  //===============================================================================================================
void WCSimDetectorConstruction::ConstructMRD(G4LogicalVolume* expHall_log, G4VPhysicalVolume* expHall_phys){

  G4cout<<"CONSTRUCTING MRD"<<G4endl;
  // Set up some sort of offset for the MRD from the tank. Leave the tank centred on the origin. 
  // N.B. Hall is 50*500*500m
   
  // offset MRD by half of length of both so edges touch + 2cm offset, with 1.52m tank radius puts MRD at z = 1.54*m.
  mrdZoffset = (2*tankouterRadius) + tankzoffset + (mrdZlen/2.) + 5*cm;
  //G4cout<<"MRD z start: "<<(tankouterRadius + 2*cm)<<" and total length: "<<mrdZlen<<G4endl;

  G4LogicalVolume* totMRD_log = new G4LogicalVolume(totMRD_box, G4Material::GetMaterial("Vacuum"),"totMRDlog",0,0,0);
  G4VPhysicalVolume* totMRD_phys = new G4PVPlacement(0,G4ThreeVector(0,0,mrdZoffset),totMRD_log,"totMRDphys",expHall_log,false,0);
  
  // REAL MRD CREATION STARTS HERE
  // =============================
  // ADD SCINTS & STEEL TO MRD
  // =========================
  G4cout<<"Placing paddles"<<G4endl; 											PlacePaddles(totMRD_log);
  G4cout<<"Placing tapers"<<G4endl;  											PlaceTapers(totMRD_log);
  G4cout<<"Placing LGs"<<G4endl;     											PlaceLGs(totMRD_log);
  G4cout<<"Placing steel plates"<<G4endl; 								PlaceSteels(totMRD_log);
  //G4cout<<"Placing sensitive detector sufaces"<<G4endl; 	//PlaceMRDSDSurfs(totMRD_log);
  G4cout<<"Placing MRD PMTs"<<G4endl; 										PlaceMRDPMTs(totMRD_log);
  G4cout<<"Placing Mylar on MRD paddles"<<G4endl;					PlaceMylarOnMRDPaddles(expHall_phys, totMRD_phys);

  // ADD ALU STRUCTURE
  // =================
  G4AssemblyVolume* aluMRDassembly = new G4AssemblyVolume();
  makeAlu(aluMRDassembly);
  G4RotationMatrix rot (0,0,0);
  G4ThreeVector trans(0,0,0);
  G4cout<<"Making aluminium assembly"<<G4endl;							aluMRDassembly->MakeImprint(totMRD_log,trans,&rot);
  
  // ADD VIS ATTRIBS
  //================
  G4VisAttributes* simpleBoxVisAtt= new G4VisAttributes(G4Colour(0.0,1.0,1.0));
  simpleBoxVisAtt->SetVisibility(false);
  totMRD_log->SetVisAttributes(simpleBoxVisAtt);

  /*
  // Retrieve Logical volumes and associate with sensitive detectors
  //================================================================
  // Get pointer to detector manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  
  // Create a new instance of MRD sensitive detector
  G4VSensitiveDetector* mrdSD = new MRDSD("MuonRangeDetector"); 
  // Register detector with manager
  SDman->AddNewDetector(mrdSD);
  // Attach detector to volume defining scintillator paddles
  hpaddle_log->SetSensitiveDetector(mrdSD);
  vpaddle_log->SetSensitiveDetector(mrdSD);
  taper_log->SetSensitiveDetector(mrdSD);
  //steel_log->SetSensitiveDetector(mrdSD);
  //aluStructsMRD_log->SetSensitiveDetector(mrdSD);
  
  */
  
}

  //===============================================================================================================
  // END MRD DETECTOR DEFINITION
  //===============================================================================================================
  
  void WCSimDetectorConstruction::ConstructVETO(G4LogicalVolume* expHall_log, G4VPhysicalVolume* expHall_phys){
  //===============================================================================================================
  // VETO WALL DEFINITION
  //===============================================================================================================
 
  G4cout<<"Constructing VETO"<<G4endl;
  G4double vetoZoffset = -(0*tankouterRadius) + (vetoZlen/2.) + 2*cm;	// by definition of rob's geometry, FACC is ~z=0
  //G4cout << "Veto z start: " << vetoZoffset-(vetoZlen/2.) << " and total length: "<< vetoZlen << G4endl;
  
  G4LogicalVolume* totVeto_log = new G4LogicalVolume(totVeto_box, G4Material::GetMaterial("Vacuum"), "totVetolog",0,0,0);
  G4VPhysicalVolume* totVeto_phys = new G4PVPlacement(0,G4ThreeVector(0,-137.64875*mm,vetoZoffset),totVeto_log,"totVetoPhys",expHall_log,false,0); 
  
  // FACC VETO
  // =========
  G4cout<<"Placing veto paddles"<<G4endl; 			PlaceVetoPaddles(totVeto_log); 
  G4cout<<"Placing veto light guides"<<G4endl; 	PlaceVetoLGs(totVeto_log);
  //G4cout<<"Placing veto SD surfaces"<<G4endl; 	PlaceVetoSDsurfs(totVeto_log);
  G4cout<<"Placing veto PMTs"<<G4endl; 					PlaceVetoPMTs(totVeto_log);
  G4cout<<"Placing Mylar on paddles"<<G4endl;		PlaceMylarOnFACCPaddles(expHall_phys, totVeto_phys);
  
  // ADD VIS ATTRIBS
  //================
  G4VisAttributes* simpleBoxVisAtt= new G4VisAttributes(G4Colour(0.0,0.0,1.0));
  simpleBoxVisAtt->SetVisibility(false);
  totVeto_log->SetVisAttributes(simpleBoxVisAtt);
  
  /*
  // Associate logical volumes with sensitive detectors
  //================================================================  
  // Get pointer to detector manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  
  // Sensitive detector for Veto paddles
  G4VSensitiveDetector* faccSD = new FACCSD("FrontAntiCoincidenceCounter");
  SDman->AddNewDetector(faccSD);
  vetoPaddle_log->SetSensitiveDetector(faccSD);
  vetol2Paddle_log->SetSensitiveDetector(faccSD);
  
  */
  
}
  
  //===============================================================================================================
  // END VETO WALL DEFINITION
  //===============================================================================================================

// ====================================
// Definition of called functions below
// ====================================

// Place Mylar on paddles and light guides
void WCSimDetectorConstruction::PlaceMylarOnMRDPaddles(G4VPhysicalVolume* expHall_phys, G4VPhysicalVolume* totMRD_phys){
  // Place cladding on MRD paddles
  for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
  		G4LogicalBorderSurface* scintSurface_log;
  		 if(totMRD_phys){
  	   scintSurface_log
  		 = new G4LogicalBorderSurface("scintcladdinglog",paddles_phys.at(i),totMRD_phys,scintSurface_op);
  		 scintSurface_log
  		 = new G4LogicalBorderSurface("scintcladdinglog",tapers_phys.at(i),totMRD_phys,scintSurface_op);
  		 }// else {
  		scintSurface_log
  		 = new G4LogicalBorderSurface("scintcladdinglog",paddles_phys.at(i),expHall_phys,scintSurface_op);
  		 scintSurface_log
  		 = new G4LogicalBorderSurface("scintcladdinglog",tapers_phys.at(i),expHall_phys,scintSurface_op);
  		 //}
  }
  
	// Place cladding on Light guides
  for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
  		G4LogicalBorderSurface* scintSurface_log;
  		 if(totMRD_phys){
  		 scintSurface_log
  		 = new G4LogicalBorderSurface("scintcladdinglog",lgs_phys.at(i),totMRD_phys,scintSurface_op);
  		 }// else {
  		 scintSurface_log
  		 = new G4LogicalBorderSurface("scintcladdinglog",lgs_phys.at(i),expHall_phys,scintSurface_op);
  		 //}
  }
}

// Place Mylar on Veto paddles and light guides
void WCSimDetectorConstruction::PlaceMylarOnFACCPaddles(G4VPhysicalVolume* expHall_phys, G4VPhysicalVolume* totVeto_phys){
  // Place cladding on Veto paddles
  for(G4int i=0;i<(numvetopaddles);i++){
  		G4LogicalBorderSurface* vetoSurface_log;
  		vetoSurface_log
  		 = new G4LogicalBorderSurface("vetocladdinglog",vetopaddles_phys.at(i),expHall_phys,scintSurface_op);
  		 vetoSurface_log
  		 = new G4LogicalBorderSurface("vetocladdinglog",vetopaddles_phys.at(i),totVeto_phys,scintSurface_op);
  }
  
  // Place cladding on Veto Light guides
  for(G4int i=0;i<(numvetopaddles);i++){
  		G4LogicalBorderSurface* vetoSurface_log;
  		vetoSurface_log
  		 = new G4LogicalBorderSurface("vetocladdinglog",vetolgs_phys.at(i),expHall_phys,scintSurface_op);
  		 vetoSurface_log
  		 = new G4LogicalBorderSurface("vetocladdinglog",vetolgs_phys.at(i),totVeto_phys,scintSurface_op);
  }
}

// Define Positioning of Steel Plates
//===================================
void WCSimDetectorConstruction::ComputeSteelTransformation (const G4int copyNo, G4VPhysicalVolume* physVol) {
		Xposition=0, Yposition=0, Zposition=0;
			
		Zposition=(copyNo)*(steelfullzlen + scintfullzlen + alufullzlen + layergap);	// layer width offset is always constant
		//Zposition=Zposition + (scintfullzlen + scintalugap + alufullzlen + alusteelgap); 							// offset of first layer 
		// no z offset of steel: front of first layer is front face of MRD.
		Zposition=Zposition + (steelfullzlen/2);													// offset by half depth so we are placing front face not centre
		Zposition=Zposition + MRDPMTRadius - (mrdZlen/2);																// offset by half total length to shift to front.

		G4ThreeVector origin(Xposition,Yposition,Zposition);
		physVol->SetTranslation(origin);
		physVol->SetRotation(0);
		physVol->GetLogicalVolume()->SetVisAttributes(steelatts);
	}
	
// Define Positioning of Scintillator Paddles
//===========================================
void WCSimDetectorConstruction::ComputePaddleTransformation (const G4int copyNo, G4VPhysicalVolume* physVol) {

		Xposition=0, Yposition=0, Zposition=0;
		G4RotationMatrix* rotmtx;
		G4int panelpairnum = floor(copyNo/(numpaddlesperpanelv+numpaddlesperpanelh));	// which pair of panels
		G4int panelnumrem = copyNo - panelpairnum*(numpaddlesperpanelv+numpaddlesperpanelh);	// copy num within a pair of panels
		G4int panelnum;
		G4int paddlenum;
		G4bool isvpaddle=false, ishpaddle=false;
		if(panelnumrem>(numpaddlesperpanelv-1)){																	// first layer is a VERTICAL layer (leading horizontal layer removed)	//## -1
			panelnum = (panelpairnum*2) +1;
			ishpaddle = true;
			paddlenum = panelnumrem-numpaddlesperpanelv; // copyNo%numpaddlesperpanelh;	//## replaced!!!
		} else {
			panelnum = (panelpairnum*2);
			isvpaddle = true;
			paddlenum = panelnumrem; // copyNo%numpaddlesperpanelv;
		}
		G4int pairnum = floor(paddlenum/2);																		// Paddles 0&1 are a pair; 
																																					// then X offset is the same for every pair
		Zposition = panelnum*(steelfullzlen + alufullzlen + scintfullzlen + layergap);
																																					// layer width offset is always constant (except first)
		//if(panelnum==0){Zposition = Zposition + alufullzlen + scintalugap;}								// first layer intrudes into 'layer offset' 
		Zposition = Zposition + steelfullzlen + steelscintgap;								// scint follows closely behind first steel
		Zposition = Zposition + (scintfullzlen/2);														// offset by half depth so we place front face not centre
		Zposition = Zposition + MRDPMTRadius - (mrdZlen/2);																	// offset by half total length to shift to front
		
		//if(paddlenum==0){G4cout<<"scint layer placed at z = " << (Zposition + (mrdZlen/2) - (scintfullzlen/2) + tankouterRadius + 2*cm)<< " cm to "<< (Zposition + (mrdZlen/2) + (scintfullzlen/2) + tankouterRadius + 2*cm) << " cm." << G4endl;}
		
		// Y position is offset by half length, so that one end is at the origin. This needs to be the correct half-length
		// for the appropriate paddle (H or V type). Offsets are defined in the MOTHER ref frame. 
		// Then rotate paddles by 90deg in Y axis in every other panel. 
		if (ishpaddle){
			// horizontal panel
			if (paddlenum%2==0){
				Xposition=((scinthfullylen+scintbordergap)/2); 										// offset by +half length so one end is at x=0
			} else {
				Xposition=-((scinthfullylen+scintbordergap)/2);										// offset by -half length so one end is at x=0
			}
			Yposition = pairnum*(scintfullxlen+scintbordergap); 								// individual offset by pair number
			Yposition = Yposition - 0.5*(((scintfullxlen+scintbordergap)/2)*numpaddlesperpanelh)+(scintfullxlen/2);
			// shift whole set by 1/2 total X extent to shift center back to X=0: HalfLength cancels doubed num of paddles
			rotmtx=rotatedmatx;
		} else {
			// vertical panel
			if (paddlenum%2==0){
				Yposition=((scintvfullylen+scintbordergap)/2); 
			} else {
				Yposition=-((scintvfullylen+scintbordergap)/2);
			}
			// for vertical panels need to shift Y for each pair
			Xposition = pairnum*(scintfullxlen+scintbordergap); 	// individual offset by pair number
			// shift whole set by 1/2 total Y extent to shift center back to Y=0: HalfLength cancels doubed num of paddles
			Xposition = Xposition - 0.5*(((scintfullxlen+scintbordergap)/2)*numpaddlesperpanelv)+(scintfullxlen/2); 
			
			rotmtx=0;							// don't rotate vertical panels
		}
		
		G4ThreeVector origin(Xposition,Yposition,Zposition);
		physVol->SetTranslation(origin);
		physVol->SetRotation(rotmtx);
//	physVol->GetLogicalVolume()->SetVisAttributes(scintvatts);	//can set visualisation attributes like this

}

// Define Positioning of Trapezoidal Taper ends of MRD paddles
// ===========================================================
// Combined with trapezoidal light-guide tapers as the code is 90% the same
void WCSimDetectorConstruction::ComputeTaperTransformation (const G4int copyNo, G4VPhysicalVolume* physVol, G4int selector, G4bool additionaloffset) {

		Xposition=0, Yposition=0, Zposition=0;
		G4RotationMatrix* rotmtx;
		G4int panelpairnum = floor(copyNo/(numpaddlesperpanelv+numpaddlesperpanelh));	// which pair of panels
		G4int panelnumrem = copyNo - panelpairnum*(numpaddlesperpanelv+numpaddlesperpanelh);	// copy num within a pair of panels
		G4int panelnum;
		G4int paddlenum;
		G4bool isvpaddle=false, ishpaddle=false;
		if(panelnumrem>numpaddlesperpanelv){																	// first layer is a VERTICAL layer (leading horizontal layer removed)
			panelnum = (panelpairnum*2) +1;
			ishpaddle = true;
			paddlenum = panelnumrem-numpaddlesperpanelv;
		} else {
			panelnum = (panelpairnum*2);
			isvpaddle = true;
			paddlenum = panelnumrem;
		}
		G4int pairnum = floor(paddlenum/2);											// LGs 0,1 are a vertical pair; X offset is the same for every pair
		
		// exact same z position calculations as paddles
		Zposition = panelnum*(steelfullzlen + alufullzlen + scintfullzlen + layergap);
																																					// layer width offset is always constant (except first)
		//if(panelnum==0){Zposition = Zposition + alufullzlen + scintalugap;}		// first layer intrudes into 'layer offset' 
		Zposition = Zposition + steelfullzlen + steelscintgap;								// scint follows closely behind first steel
		Zposition = Zposition + (scintfullzlen/2);														// offset by half depth so we place front face not centre
		Zposition=Zposition + MRDPMTRadius - (mrdZlen/2);																		// offset by half total length to shift to front
		if(additionaloffset){																									// when adding on top of SciBooNE code
			Zposition+= (mrdZoffset - ((alufullzlen + scintfullzlen)/2));
		}
	
		// Y offset is the full length of the paddle plus half length of LG. Paddle length needs to be the correct type
		// (H or V type). Same rotation as paddles. 
		if (ishpaddle){
			// horizontal panel
			if(selector==0){					// scintillating tapers
			  Xposition=scinthfullylen+(scintbordergap/2)+(scinttapfullheight/2);
			} else if(selector==1) {	//light guides
				Xposition=scinthfullylen+(scintbordergap/2)+scinttapfullheight+(scintlgfullheight/2);
			} else if(selector==2) {	// pmts
				Xposition=scinthfullylen+(scintbordergap/2)+scinttapfullheight+scintlgfullheight+(mrdpmtfullheight/2);
			} else if(selector==3) {	// sensitive surface box
				Xposition=scinthfullylen+(scintbordergap/2)+scinttapfullheight+scintlgfullheight+(nothickness/2);
			}

			if (paddlenum%2==0){
				if(selector==2){
					rotmtx=leftmtx;
				} else{
					rotmtx=rightmtx;
				}
			} else {
				if(selector==2){
					Xposition=-Xposition;
					rotmtx=rightmtx;
				} else {
					Xposition=-Xposition;
					rotmtx=leftmtx;
				}
			}
			// Y offset exactly the same as paddles
			Yposition = pairnum*(scintfullxlen+scintbordergap);
			Yposition = Yposition - 0.5*(((scintfullxlen+scintbordergap)/2)*numpaddlesperpanelh)+(scintfullxlen/2); 
		} else {
		// vertical panel
			if(selector==0){
				Yposition=scintvfullylen+(scintbordergap/2)+(scinttapfullheight/2);
			} else if(selector==1){
				Yposition=scintvfullylen+(scintbordergap/2)+scinttapfullheight+(scintlgfullheight/2);
			} else if(selector==2){	//pmts
				Yposition=scintvfullylen+(scintbordergap/2)+scinttapfullheight+scintlgfullheight+(mrdpmtfullheight/2);
			} else if(selector==3){	//sds
				Yposition=scintvfullylen+(scintbordergap/2)+scinttapfullheight+scintlgfullheight+(nothickness/2);
			}

			if (paddlenum%2==0){
				if(selector==2){
					rotmtx=downmtx;
				} else {
					rotmtx=upmtx; 
				}
			} else {
				if(selector==2){
					Yposition=-Yposition;
					rotmtx=upmtx;
				} else {
					Yposition=-Yposition;
					rotmtx=downmtx;
				}
			}
			Xposition = pairnum*(scintfullxlen+scintbordergap);
			Xposition = Xposition - 0.5*(((scintfullxlen+scintbordergap)/2)*numpaddlesperpanelv)+(scintfullxlen/2); 
		}

		G4ThreeVector origin(Xposition,Yposition,Zposition);
		physVol->SetRotation(rotmtx);
		physVol->SetTranslation(origin);
		if(selector==0){physVol->GetLogicalVolume()->SetVisAttributes(scinttapatts);}
		else if(selector==1) {physVol->GetLogicalVolume()->SetVisAttributes(scintlgatts);}
		//else if(selector==3) {physVol->GetLogicalVolume()->SetVisAttributes(senssurfatts);}
		
}

// Define Positioning of Veto Paddles
//===========================================
void WCSimDetectorConstruction::ComputeVetoPaddleTransformation (const G4int copyNo, G4VPhysicalVolume* physVol, G4int selector) {
	
		Xposition=0, Yposition=0, Zposition=0;
		G4RotationMatrix* rotmtx=0;																								// No rotations.
		G4int panelnum = floor(copyNo/vetopaddlesperpanel);												// numbering from 0
		G4int paddlenum = copyNo%vetopaddlesperpanel; 														// numering from 0 within a panel
		Zposition = panelnum*(vetopaddlefullzlen+vetopaddlegap); 									// layer width offset
		Zposition = Zposition + (vetopaddlefullzlen/2);														// offset by half depth to place front face not centre
		Zposition = Zposition - (vetoZlen/2);																			// offset by half total length to shift to front
			
		// AFAIK paddle dims are the same in both layers.
		Xposition = 0;																														// all paddles are centered on the beam
		Yposition = paddlenum*(vetopaddlefullylen+vetopaddlegap); 								// individual offset by paddle number
		// shift whole set by 1/2 total Y extent to shift center back to Y=0: HalfLength cancels doubed num of paddles
		Yposition = Yposition - (0.5*((vetopaddlefullylen+vetopaddlegap)*vetopaddlesperpanel))+(vetopaddlefullylen/2);
		if (panelnum==0){
 			Yposition = Yposition - (vetolayer2offset/2);																// slight shift of second layer WRT first
 		}
 		
 		// element shifts
		if(selector==1){				// small volume at ends of light guides to detect photons crossing surface
			Xposition = Xposition + (vetopaddlefullxlen/2.) + vetolgfullxlen + (nothickness/2);
			if(copyNo<vetopaddlesperpanel){
				Xposition=-Xposition;
			}
		} else if(selector==2){	// light guides
			Xposition = Xposition + (vetopaddlefullxlen/2.) + (vetolgfullxlen/2);
			if(copyNo>(vetopaddlesperpanel-1)){
				rotmtx=rightmtx;
			} else {
				Xposition=-Xposition;
				rotmtx=leftmtx;
			}
		} else if(selector==3){	// pmts at ends of lgs
			Xposition = Xposition + (vetopaddlefullxlen/2.) + vetolgfullxlen + (vetopmtfullheight/2);
			if(copyNo>(vetopaddlesperpanel-1)){
				rotmtx=leftmtx;
			} else {
				Xposition=-Xposition;
				rotmtx=rightmtx;
			}
		}
		G4ThreeVector origin(Xposition,Yposition,Zposition);
		physVol->SetTranslation(origin);
		physVol->SetRotation(rotmtx);
		if(selector==1) {physVol->GetLogicalVolume()->SetVisAttributes(senssurfatts);}
		// set vis attributes when placing, since they share a logical volume
}

// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
// Done defining functions. Now do actual generation and placement of physical volumes
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

// Code to do generation and placement of scintillator paddles
// ===========================================================
void WCSimDetectorConstruction::PlacePaddles(G4LogicalVolume* totMRD_log){

	hpaddle_log = new G4LogicalVolume(sciMRDhpaddle_box, G4Material::GetMaterial("Scinti"), "vpaddle_log");
	vpaddle_log = new G4LogicalVolume(sciMRDvpaddle_box, G4Material::GetMaterial("Scinti"), "hpaddle_log");
	hpaddle_log->SetVisAttributes(scinthatts);
	vpaddle_log->SetVisAttributes(scintvatts);
	G4LogicalVolume* paddle_log;
	G4VPhysicalVolume* paddle_phys;
	for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
			G4int panelpairnum = floor(i/(numpaddlesperpanelv+numpaddlesperpanelh));					// which pair of panels
			G4int panelnumrem = i - panelpairnum*(numpaddlesperpanelv+numpaddlesperpanelh);		// copy num within a pair of panels
			G4bool isvpaddle=false, ishpaddle=false;
			if(panelnumrem>numpaddlesperpanelv){																	// first layer is a VERTICAL layer (leading horizontal layer removed)
				paddle_log = hpaddle_log;
			} else {
				paddle_log = vpaddle_log;
			}
			
			paddle_phys = new G4PVPlacement(noRot,G4ThreeVector(), paddle_log, "paddle_phys", totMRD_log, false, i);
			ComputePaddleTransformation(i, paddle_phys);
			paddles_phys.push_back(paddle_phys);
	}
}

// Code to do generation and placement of scintillator taper ends
// ==============================================================
void WCSimDetectorConstruction::PlaceTapers(G4LogicalVolume* totMRD_log){

	taper_log = new G4LogicalVolume(mrdScintTap_box, G4Material::GetMaterial("Scinti"), "taper_log");
	G4VPhysicalVolume* taper_phys;
	for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
			taper_phys = new G4PVPlacement(noRot,G4ThreeVector(), taper_log, "taper_phys", totMRD_log, false, i);
			ComputeTaperTransformation(i, taper_phys, 0, false);
			tapers_phys.push_back(taper_phys);
	}
}

// Code to do generation and placement of glass light-guides
// =========================================================
void WCSimDetectorConstruction::PlaceLGs(G4LogicalVolume* totMRD_log){

	lg_log = new G4LogicalVolume(mrdLG_box, G4Material::GetMaterial("Glass"), "lg_log");
	G4VPhysicalVolume* lg_phys;
	for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
			lg_phys = new G4PVPlacement(noRot,G4ThreeVector(), lg_log, "lg_phys", totMRD_log, false, i);
			ComputeTaperTransformation(i, lg_phys, 1, false);
			lgs_phys.push_back(lg_phys);
	}
}

// Code to do generation and placement of mrd sd surfaces at ends of light-guides
// ==============================================================================
void WCSimDetectorConstruction::PlaceMRDSDSurfs(G4LogicalVolume* totMRD_log){

	mrdsdsurf_log = new G4LogicalVolume(mrdSurface_box, G4Material::GetMaterial("Air"), "mrdsdsurf_log");
	G4VisAttributes* surfacevisatts= new G4VisAttributes(G4Colour(1.0, 1.0, 0.));
	mrdsdsurf_log->SetVisAttributes(surfacevisatts);
	G4VPhysicalVolume* mrdsdsurf_phys;
	
	for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
			mrdsdsurf_phys = new G4PVPlacement(noRot,G4ThreeVector(), mrdsdsurf_log, "mrdsdsurf_phys", totMRD_log, false, i);
			ComputeTaperTransformation(i, mrdsdsurf_phys, 3, useadditionaloffset);
			mrdsdsurfs_phys.push_back(mrdsdsurf_phys);
	}
	
	// must be dielectric_metal to invoke absorption/detection process - but is this overridden if both volumes have a ref index?
  // for dielectric_metal transmittance isn't possible, so either reflection or absorption with probability from mat. properties. 
  for(G4int i=0;i<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));i++){
      G4LogicalBorderSurface* lgSurface_log = new G4LogicalBorderSurface("lgborderlog",lgs_phys.at(i),mrdsdsurfs_phys.at(i),lgSurface_op);
  }
  
  // Create sensitive detector for pmts that will detect photon hits at the ends of the paddles
  G4VSensitiveDetector* mrdpmtSD = new mrdPMTSD("MRDPMTSD"); 
  // Register detector with manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  SDman->AddNewDetector(mrdpmtSD);
  // gets called manually and tracks get killed before they enter it; don't need to associate with any logical volumes
}

// Code to do generation and placement of MRD PMTs
// ===============================================
void WCSimDetectorConstruction::PlaceMRDPMTs(G4LogicalVolume* totMRD_log){

//	G4cout<<"calling ConstructPMT"<<G4endl;
	logicMRDPMT = ConstructFlatFacedPMT(MRDPMTName, WCMRDCollectionName, "mrd");
	G4VPhysicalVolume* mrdpmt_phys;
//	G4cout<<"making placements"<<G4endl;
	for(G4int icopy=0; icopy<((numpaddlesperpanelv+numpaddlesperpanelh)*(numpanels/2));icopy++){
		mrdpmt_phys = 
		new G4PVPlacement(	0,						// no rotation
							G4ThreeVector(),				// its position
							logicMRDPMT,						// its logical volume
							"MRDPMT",								// its name
							totMRD_log,							// its mother volume
							false,									// no boolean os
							icopy);									// every PMT need a unique id.
// do not check for overlaps - it doesn't work. 
		ComputeTaperTransformation(icopy, mrdpmt_phys, 2, useadditionaloffset);
	}
//	G4cout<<"end of loop"<<G4endl;
}

// Code to do generation and placement of steel plates
// ===================================================
void WCSimDetectorConstruction::PlaceSteels(G4LogicalVolume* totMRD_log){

	steel_log = new G4LogicalVolume(steelMRDplate_box, G4Material::GetMaterial("Steel"), "steel_log");
	G4VPhysicalVolume* steel_phys;
	for(G4int i=0;i<(numplates);i++){		// first layer of steel has been removed
			steel_phys = new G4PVPlacement(noRot,G4ThreeVector(), steel_log, "steel_phys", totMRD_log, false, i);
			ComputeSteelTransformation(i, steel_phys);
	}
}

// Define Alu Support Structure
//=============================

void WCSimDetectorConstruction::makeAlu(G4AssemblyVolume* aluMRDassembly){
	// this code must be in a function as it uses for loops.

	//N.B. subtraction solids can only use CGS solids or the output of boolean operations, not assemblies or other stuff
	G4RotationMatrix  Ra (0,0,0);
	G4ThreeVector  Ta (0,0,0);
	Ta.set(windowwidth+alufullxthickness,windowheight+alufullythickness,0);
	G4Transform3D transform = G4Transform3D(Ra,Ta);
	G4SubtractionSolid* aluMRDstruc_sol = new G4SubtractionSolid("aluStruct",aluMRDstruc_box,aluMRDwindow_box,transform);
	G4SubtractionSolid* aluMRDstruc_sol2;
	
	for (G4int row=-1;row<2;row++){
		for (G4int col=-1;col<2;col++){
			G4double xoffset=col*(alufullxthickness+windowwidth);
			G4double yoffset=row*(alufullythickness+windowheight);
			Ta.set(xoffset,yoffset,0);                        
			transform = G4Transform3D(Ra,Ta);
			// using method with G4Transform3D means we can change the transform after as it is passed byval.
			// If we used a passive transform we would need to maintain the G4Transform3D. 
			aluMRDstruc_sol2 = new G4SubtractionSolid("aluStruct",aluMRDstruc_sol,aluMRDwindow_box,transform);
			//delete aluMRDstruc_sol; -- can't do this! seems derived boolean solid requires it's originals are kept alive!
			aluMRDstruc_sol = aluMRDstruc_sol2;
		}
	}

	G4LogicalVolume* aluMRDstruc_log 
	= new G4LogicalVolume(aluMRDstruc_sol, G4Material::GetMaterial("Aluminum"), "aluStruct",0,0,0) ;

	Ra.set(0,0,0);
	Ta.set(0,0,0);
	for (G4int structnum=0;structnum<(numalustructs-1);structnum++){		// first alu layer removed
		G4double zpos = structnum*(steelfullzlen + alufullzlen + scintfullzlen + layergap); // layer width offset is always constant
		//if(structnum>0){zpos = zpos + scintfullzlen + scintalugap;}	// all layers > 1 have additional scint offset
		zpos+= steelfullzlen + steelscintgap + scintfullzlen + scintalugap;
		zpos+= (alufullzlen/2); 																		// offset by half depth to place front face not centre
		zpos+= MRDPMTRadius -(mrdZlen/2);																				// offset by half total length to shift to front
		Ta.set(0,0,zpos); 
		aluMRDassembly->AddPlacedVolume(aluMRDstruc_log,Ta,&Ra);
	}
	//aluMRDassembly->MakeImprint(expHall->GetLogicalVolume(), Tm,&Rm);  //placement done in DetectorConstruction
}

// Code to do generation and placement of veto paddles
// ===================================================
void WCSimDetectorConstruction::PlaceVetoPaddles(G4LogicalVolume* totVeto_log){

	// using same scintillator as MRD paddles
	vetoPaddle_log = new G4LogicalVolume(vetoPaddle_box, G4Material::GetMaterial("Scinti"), "vetoPaddle_log");
	vetol2Paddle_log = new G4LogicalVolume(vetoPaddle_box, G4Material::GetMaterial("Scinti"), "vetol2Paddle_log");
	vetoPaddle_log->SetVisAttributes(scinthatts);
	vetol2Paddle_log->SetVisAttributes(scintvatts);
	G4LogicalVolume* paddle_log;
	G4VPhysicalVolume* vetoPaddle_phys;
	
	for(G4int i=0;i<numvetopaddles;i++){
		G4int panelnum = floor(i/vetopaddlesperpanel);
		if(panelnum==0){
			paddle_log=vetoPaddle_log;
		} else {
			paddle_log=vetol2Paddle_log;
		}
		vetoPaddle_phys = new G4PVPlacement(noRot,G4ThreeVector(), paddle_log, "vetoPaddle_phys", totVeto_log, false, i);
		ComputeVetoPaddleTransformation(i, vetoPaddle_phys,0);
		vetopaddles_phys.push_back(vetoPaddle_phys);
	}
}

// Code to do generation and placement of veto LGs
// ===================================================
void WCSimDetectorConstruction::PlaceVetoLGs(G4LogicalVolume* totVeto_log){

	// tapered light guide placement
	vetolg_log = new G4LogicalVolume(vetoLG_box, G4Material::GetMaterial("Glass"),"vetolg_log");
	G4VisAttributes* surfacevisatts= new G4VisAttributes(G4Colour(0.6, 1.0, 0.8));
	vetolg_log->SetVisAttributes(surfacevisatts);
	G4VPhysicalVolume* vetolg_phys;
	for(G4int i=0;i<(numvetopaddles);i++){
		vetolg_phys = new G4PVPlacement(noRot,G4ThreeVector(), vetolg_log, "vetolg_phys", totVeto_log, false, i);
		ComputeVetoPaddleTransformation(i, vetolg_phys, 2);
		vetolgs_phys.push_back(vetolg_phys);
	}
}

// Code to do generation and placement of veto SD surfaces
// =======================================================
void WCSimDetectorConstruction::PlaceVetoSDsurfs(G4LogicalVolume* totVeto_log){

	// gotta make stupid little physical volumes so we can put a surface in. It shouldn't take up any space. 
	vetoSurface_log = new G4LogicalVolume(vetoSurface_box, G4Material::GetMaterial("Air"),"vetoSurface_log");
	G4VisAttributes* surfacevisatts= new G4VisAttributes(G4Colour(1.0, 1.0, 0.));
	vetoSurface_log->SetVisAttributes(surfacevisatts);
	G4VPhysicalVolume* vetoSurface_phys;

	for(G4int i=0;i<(numvetopaddles);i++){
		vetoSurface_phys = new G4PVPlacement(noRot,G4ThreeVector(), vetoSurface_log, "vetoSurface_phys", totVeto_log, false, i);
		ComputeVetoPaddleTransformation(i, vetoSurface_phys,1);
		vetosurfaces_phys.push_back(vetoSurface_phys);
	}
	
	  // must be dielectric_metal to invoke absorption/detection process - but is this overridden if both volumes have a ref index?
  // for dielectric_metal transmittance isn't possible, so either reflection or absorption with probability from mat. properties. 
  for(G4int i=0;i<(numvetopaddles);i++){
  	G4LogicalBorderSurface* vetoBorderSurface_log = new G4LogicalBorderSurface("vetoborderlog",vetopaddles_phys.at(i),vetosurfaces_phys.at(i),lgSurface_op);
  }
  
  // Sensitive detector for Veto PMTs
  G4VSensitiveDetector* faccpmtSD = new faccPMTSD("FACCPMTSD"); 
  // Register detector with manager
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  SDman->AddNewDetector(faccpmtSD);
  // gets called manually and tracks get killed before they enter it; don't need to associate with any logical volumes
}

// Code to do generation and placement of FACC PMTs
// ================================================
void WCSimDetectorConstruction::PlaceVetoPMTs(G4LogicalVolume* totVeto_log){

	logicFACCPMT = ConstructFlatFacedPMT(FACCPMTName, WCFACCCollectionName, "facc");
	G4VPhysicalVolume* faccpmt_phys;
	for(G4int icopy=0; icopy<numvetopaddles;icopy++){
		faccpmt_phys = 
		new G4PVPlacement(	0,						// no rotation
							G4ThreeVector(),				// its position
							logicFACCPMT,						// its logical volume
							"FACCPMT",							// its name (WCPMT?)
							totVeto_log,						// its mother volume
							false,									// no boolean os
							icopy/*,									// every PMT need a unique id.
							true*/);									// check for overlaps
		ComputeVetoPaddleTransformation(icopy, faccpmt_phys, 3);
	}
}

/* MRD z edges, from firing a geantino (set geantinogun in primary generator action)
volume column is the volume it is LEAVING
Step#      X         Y         Z        KineE    dEStep   StepLeng  TrakLeng    Volume     Process
    0      3 cm      3 cm      0 fm      1 GeV     0 eV      0 fm      0 fm    waterTank    initStep
    1      3 cm      3 cm   1.52 m       1 GeV     0 eV   1.52 m    1.52 m     waterTank  Transportation
    2      3 cm      3 cm   1.54 m       1 GeV     0 eV   2.03 cm   1.54 m          Hall  Transportation
    3      3 cm      3 cm   1.56 m       1 GeV     0 eV    2.1 cm   1.56 m    totMRDphys  Transportation
    4      3 cm      3 cm   1.57 m       1 GeV     0 eV      6 mm   1.57 m    paddle_phys  Transportation
    5      3 cm      3 cm   1.59 m       1 GeV     0 eV      2 cm   1.59 m    totMRDphys  Transportation
    6      3 cm      3 cm   1.64 m       1 GeV     0 eV      5 cm   1.64 m    steel_phys  Transportation
    7      3 cm      3 cm   1.64 m       1 GeV     0 eV      2 mm   1.64 m    totMRDphys  Transportation
    8      3 cm      3 cm   1.65 m       1 GeV     0 eV      6 mm   1.65 m    paddle_phys  Transportation
    9      3 cm      3 cm   1.69 m       1 GeV     0 eV    4.1 cm   1.69 m    totMRDphys  Transportation
   10      3 cm      3 cm   1.74 m       1 GeV     0 eV      5 cm   1.74 m    steel_phys  Transportation
   11      3 cm      3 cm   1.74 m       1 GeV     0 eV      2 mm   1.74 m    totMRDphys  Transportation
   12      3 cm      3 cm   1.75 m       1 GeV     0 eV      6 mm   1.75 m    paddle_phys  Transportation
   13      3 cm      3 cm   1.79 m       1 GeV     0 eV    4.1 cm   1.79 m    totMRDphys  Transportation
   14      3 cm      3 cm   1.84 m       1 GeV     0 eV      5 cm   1.84 m    steel_phys  Transportation
   15      3 cm      3 cm   1.84 m       1 GeV     0 eV      2 mm   1.84 m    totMRDphys  Transportation
   16      3 cm      3 cm   1.85 m       1 GeV     0 eV      6 mm   1.85 m    paddle_phys  Transportation
   17      3 cm      3 cm   1.89 m       1 GeV     0 eV    4.1 cm   1.89 m    totMRDphys  Transportation
   18      3 cm      3 cm   1.94 m       1 GeV     0 eV      5 cm   1.94 m    steel_phys  Transportation
   19      3 cm      3 cm   1.94 m       1 GeV     0 eV      2 mm   1.94 m    totMRDphys  Transportation
   20      3 cm      3 cm   1.95 m       1 GeV     0 eV      6 mm   1.95 m    paddle_phys  Transportation
   21      3 cm      3 cm   1.99 m       1 GeV     0 eV    4.1 cm   1.99 m    totMRDphys  Transportation
   22      3 cm      3 cm   2.04 m       1 GeV     0 eV      5 cm   2.04 m    steel_phys  Transportation
   23      3 cm      3 cm   2.04 m       1 GeV     0 eV      2 mm   2.04 m    totMRDphys  Transportation
   24      3 cm      3 cm   2.04 m       1 GeV     0 eV      6 mm   2.04 m    paddle_phys  Transportation
   25      3 cm      3 cm   2.09 m       1 GeV     0 eV    4.1 cm   2.09 m    totMRDphys  Transportation
   26      3 cm      3 cm   2.14 m       1 GeV     0 eV      5 cm   2.14 m    steel_phys  Transportation
   27      3 cm      3 cm   2.14 m       1 GeV     0 eV      2 mm   2.14 m    totMRDphys  Transportation
   28      3 cm      3 cm   2.14 m       1 GeV     0 eV      6 mm   2.14 m    paddle_phys  Transportation
   29      3 cm      3 cm   2.19 m       1 GeV     0 eV    4.1 cm   2.19 m    totMRDphys  Transportation
   30      3 cm      3 cm   2.23 m       1 GeV     0 eV      5 cm   2.23 m    steel_phys  Transportation
   31      3 cm      3 cm   2.24 m       1 GeV     0 eV      2 mm   2.24 m    totMRDphys  Transportation
   32      3 cm      3 cm   2.24 m       1 GeV     0 eV      6 mm   2.24 m    paddle_phys  Transportation
   33      3 cm      3 cm   2.28 m       1 GeV     0 eV    4.1 cm   2.28 m    totMRDphys  Transportation
   34      3 cm      3 cm   2.33 m       1 GeV     0 eV      5 cm   2.33 m    steel_phys  Transportation
   35      3 cm      3 cm   2.34 m       1 GeV     0 eV      2 mm   2.34 m    totMRDphys  Transportation
   36      3 cm      3 cm   2.34 m       1 GeV     0 eV      6 mm   2.34 m    paddle_phys  Transportation
   37      3 cm      3 cm   2.38 m       1 GeV     0 eV    4.1 cm   2.38 m    totMRDphys  Transportation
   38      3 cm      3 cm   2.43 m       1 GeV     0 eV      5 cm   2.43 m    steel_phys  Transportation
   39      3 cm      3 cm   2.44 m       1 GeV     0 eV      2 mm   2.44 m    totMRDphys  Transportation
   40      3 cm      3 cm   2.44 m       1 GeV     0 eV      6 mm   2.44 m    paddle_phys  Transportation
   41      3 cm      3 cm   2.48 m       1 GeV     0 eV    4.1 cm   2.48 m    totMRDphys  Transportation
   42      3 cm      3 cm   2.53 m       1 GeV     0 eV      5 cm   2.53 m    steel_phys  Transportation
   43      3 cm      3 cm   2.53 m       1 GeV     0 eV      2 mm   2.53 m    totMRDphys  Transportation
   44      3 cm      3 cm   2.54 m       1 GeV     0 eV      6 mm   2.54 m    paddle_phys  Transportation
   45      3 cm      3 cm   2.58 m       1 GeV     0 eV    4.1 cm   2.58 m    totMRDphys  Transportation
   46      3 cm      3 cm   2.63 m       1 GeV     0 eV      5 cm   2.63 m    steel_phys  Transportation
   47      3 cm      3 cm   2.63 m       1 GeV     0 eV      2 mm   2.63 m    totMRDphys  Transportation
   48      3 cm      3 cm   2.64 m       1 GeV     0 eV      6 mm   2.64 m    paddle_phys  Transportation
   49      3 cm      3 cm   2.68 m       1 GeV     0 eV    4.1 cm   2.68 m    totMRDphys  Transportation
   50      3 cm      3 cm   2.73 m       1 GeV     0 eV      5 cm   2.73 m    steel_phys  Transportation
   51      3 cm      3 cm   2.76 m       1 GeV     0 eV    2.9 cm   2.76 m    totMRDphys  Transportation
   52      3 cm      3 cm     25 m       1 GeV     0 eV   22.2 m      25 m    OutOfWorld  Transportation
*/

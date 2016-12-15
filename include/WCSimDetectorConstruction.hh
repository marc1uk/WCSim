#ifndef WCSimDetectorConstruction_H
#define WCSimDetectorConstruction_H 1

#include "WCSimPmtInfo.hh"
#include "WCSimPMTObject.hh"

#include "G4Transform3D.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4OpticalSurface.hh"
#include "globals.hh"

#include <fstream>
#include <map>
#include <vector>
//#include <hash_map.h>
// warning : hash_map is not part of the standard
#include <ext/hash_map>

// **************SciBooNE integration
#include "G4UserLimits.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4AssemblyVolume.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4Trd.hh"
#include "G4SystemOfUnits.hh"
static const G4double INCH = 2.54*cm;
// *************/SciBooNE integration



using __gnu_cxx::hash;
using __gnu_cxx::hashtable;
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_multimap;

// (JF) We don't need this distinction for DUSEL
//enum cyl_location {endcap1,wall,endcap2};

class G4Box;
class G4Tubs;
class G4Material;
class G4LogicalVolume;
class G4AssemblyVolume;
class G4VPhysicalVolume;
class WCSimTuningParameters;
class WCSimDetectorMessenger;
class WCSimWCSD;

namespace __gnu_cxx  {
  template<> struct hash< std::string >
  {
    size_t operator()( const std::string& x ) const
    {
      return hash< const char* >()( x.c_str() );
    }
  };
}

class WCSimDetectorConstruction : public G4VUserDetectorConstruction
{
public:

  WCSimDetectorConstruction(G4int DetConfig,WCSimTuningParameters* WCSimTuningPars);
  ~WCSimDetectorConstruction();
  
  G4VPhysicalVolume* Construct();

  // Related to the WC geometry
  void SetSuperKGeometry();
  void SuperK_20inchPMT_20perCent();
  void SuperK_20inchBandL_20perCent();
  void SuperK_12inchBandL_15perCent();
  void SuperK_20inchBandL_14perCent();
  void Cylinder_60x74_20inchBandL_14perCent();
  void Cylinder_60x74_20inchBandL_40perCent();
  void Cylinder_12inchHPD_15perCent();
  void SetHyperKGeometry();
  void UpdateGeometry();
  void SetANNIEPhase1Geometry(); //phase 1 geometry - just 60 PMTs on bottom, with NCV.
  void SetANNIEPhase2Geometry(); // phase 2 geometry - 60 PMTs on bottom, top, and 200 PMTs around barrel.
  

  G4String GetDetectorName()      {return WCDetectorName;}
  G4double GetWaterTubeLength()   {return WCLength;}
  G4double GetWaterTubePosition() {return WCPosition;}
  G4double GetPMTSize()           {return WCPMTRadius;}
  G4String GetPMTName()           {return WCPMTName;}
  G4int    GetMyConfiguration()   {return myConfiguration;}
  G4double GetGeo_Dm(G4int);
  G4int    GetTotalNumPmts() {return totalNumPMTs;}
  G4int    GetTotalNumMrdPmts() {return totalNumMrdPMTs;}
  G4int    GetTotalNumFaccPmts() {return totalNumFaccPMTs;}
  
  G4int    GetPMT_QE_Method(){return PMT_QE_Method;}
  G4double GetwaterTank_Length() {return waterTank_Length;} 
  G4int    UsePMT_Coll_Eff(){return PMT_Coll_Eff;}

  G4double GetPMTSize1() {return WCPMTSize;}

  G4float GetPMTQE(G4String,G4float, G4int, G4float, G4float, G4float);
  G4float GetPMTCollectionEfficiency(G4float theta_angle, G4String CollectionName) { return GetPMTPointer(CollectionName)->GetCollectionEfficiency(theta_angle); };

  WCSimPMTObject *CreatePMTObject(G4String, G4String);

  std::map<G4String, WCSimPMTObject*>  CollectionNameMap; 
  WCSimPMTObject * PMTptr;
 
  void SetPMTPointer(WCSimPMTObject* PMT, G4String CollectionName){
    CollectionNameMap[CollectionName] = PMT;
  }

  WCSimPMTObject* GetPMTPointer(G4String CollectionName){
    PMTptr = CollectionNameMap[CollectionName];
    if (PMTptr == NULL) {G4cout << CollectionName << " is not a recognized hit collection. Exiting WCSim." << G4endl; exit(1);}
    return PMTptr;
  }
 
  G4ThreeVector GetWCOffset(){return WCOffset;}
  
  // Related to the WC tube ID
  static G4int GetTubeID(std::string tubeTag){return tubeLocationMap[tubeTag];}
  static G4Transform3D GetTubeTransform(int tubeNo){return tubeIDMap[tubeNo];}
  // MRD tube ID
  static G4int GetMrdTubeID(std::string tubeTag){return mrdtubeLocationMap[tubeTag];}
  static G4Transform3D GetMrdTubeTransform(int tubeNo){return mrdtubeIDMap[tubeNo];}
  // FACC tube ID
  static G4int GetFaccTubeID(std::string tubeTag){return facctubeLocationMap[tubeTag];}
  static G4Transform3D GetFaccTubeTransform(int tubeNo){return facctubeIDMap[tubeNo];}

  // Related to Pi0 analysis
  G4bool SavePi0Info()              {return pi0Info_isSaved;}
  void   SavePi0Info(G4bool choice) {pi0Info_isSaved=choice;}
  
  void   SetPMT_QE_Method(G4int choice){PMT_QE_Method = choice;}
  void   SetPMT_Coll_Eff(G4int choice){PMT_Coll_Eff = choice;}
  void   SetVis_Choice(G4String choice){Vis_Choice = choice;}
  G4String GetVis_Choice() {return Vis_Choice;}

  //Partition Length
  void SetwaterTank_Length(G4double length){waterTank_Length = length;}
  void SetWaterTubeLength(G4double length){WCLength = length;}
  // Geometry options
  void   SetIsUpright(G4bool choice) {isUpright = choice;}

  // *** Begin Egg-Shaped HyperK Geometry ***

  void   SetIsEggShapedHyperK(G4bool choice) {isEggShapedHyperK = choice;}
  G4bool GetIsEggShapedHyperK() {return isEggShapedHyperK;}

  void SetEggShapedHyperKGeometry();
  void SetEggShapedHyperKGeometry_withHPD();


  // *** End Egg-Shaped HyperK Geometry ***

  std::vector<WCSimPmtInfo*>* Get_Pmts() {return &fpmts;}

  G4String GetIDCollectionName(){return WCIDCollectionName;}

 
private:

  // Tuning parameters

  WCSimTuningParameters* WCSimTuningParams;

  // Sensitive Detectors. We declare the pointers here because we need
  // to check their state if we change the geometry, otherwise will segfault
  // between events!
  WCSimWCSD* aWCPMT;

  //Water, Blacksheet surface
  G4OpticalSurface * OpWaterBSSurface;

  //Glass, Cathode surface in PMTs

  G4OpticalSurface * OpGlassCathodeSurface;

  //Tyvek surface - jl145
  G4OpticalSurface * OpWaterTySurface;

  // The messenger we use to change the geometry.

  WCSimDetectorMessenger* messenger;

  // The Construction routines
  G4LogicalVolume*   ConstructCylinder();
  G4LogicalVolume* ConstructPMT(G4String,G4String, G4String detectorElement="tank");
  G4LogicalVolume* ConstructFlatFacedPMT(G4String PMTName, G4String CollectionName, G4String detectorElement="mrd");

  G4LogicalVolume* ConstructCaps(G4int zflip);

  void  ConstructMaterials();

  G4LogicalVolume* logicWCBarrelCellBlackSheet;
  G4LogicalVolume* logicWCTowerBlackSheet;
  G4double capAssemblyHeight;

  G4bool WCAddGd;

  // Code for traversing the geometry and assigning tubeIDs.

  // First make a typedef for the pointer to the member fcn.  The
  // syntax is too wacked out to be using all over.

  typedef void (WCSimDetectorConstruction::*DescriptionFcnPtr)
    (G4VPhysicalVolume*, int, int, const G4Transform3D&);

  // Now Funcs for traversing the geometry
  void TraverseReplicas(G4VPhysicalVolume*, int, const G4Transform3D&,
			DescriptionFcnPtr);

  void DescribeAndDescendGeometry(G4VPhysicalVolume*, int, int, 
				  const G4Transform3D&, DescriptionFcnPtr);

  // Functions that the traversal routines call or we use to manipulate the
  // data we accumulate.
  void DumpGeometryTableToFile();

  void PrintGeometryTree(G4VPhysicalVolume*, int, int, const G4Transform3D&);
  void DescribeAndRegisterPMT(G4VPhysicalVolume*, int, int, 
			      const G4Transform3D&);
  void DescribeAndRegisterPMT_1KT(G4VPhysicalVolume*, int, int, 
				  const G4Transform3D&);
  void GetWCGeom(G4VPhysicalVolume*, int, int, 
			      const G4Transform3D&);

  //---Volume lengths

  // These are shared between the different member functions 
  // constructWC, constructFGD, constructlArD, constuctMRD
  // toggle between FGD(0) and lArD(1)
  // toggle between lArD readout types
  // toggle between MRDScint and MRDk2k

  G4bool pi0Info_isSaved;


  // XQ 08/17/10
  //   PMT_QE_Method == 1
  //   Only use it in the stacking function (no WLS)
  //   PMT_QE_Method == 2
  //   Use Part of it in the stacking function (MAX QE)
  //   Put the rest of it in the sensitive detector according to QE/Max_QE
  //   PMT_QE_Method == 3
  //   Put all of it in the sensitive detector according to QE
  //   Good for low energy photons
  G4int PMT_QE_Method;

  //XQ 03/31/11
  // 0 to not use collection efficiency
  // 1 to use
  G4int PMT_Coll_Eff;

  //NP 06/17/15
  // "OGLSX" for classic visualization
  // "RayTracer" for RayTracer visualization
  G4String Vis_Choice;
  

  G4double WCLength;

  G4double WCPosition;
  
  // Hit collection name parameters
  G4String WCDetectorName;
  G4String WCIDCollectionName;
  G4String WCODCollectionName;


  // WC PMT parameters
  G4String WCPMTName;
  typedef std::pair<G4String, G4String> PMTKey_t;
  typedef std::map<PMTKey_t, G4LogicalVolume*> PMTMap_t;

  static PMTMap_t PMTLogicalVolumes;

  // WC geometry parameters

  G4double WCPMTRadius;
  G4double WCPMTExposeHeight;
  G4double WCBarrelPMTOffset;

  G4double WCIDDiameter;

  G4double WCCapLength;
  G4double WCBackODLength;
  G4double WCFrontODLength;
  G4double WCIDHeight;

  G4double WCBarrelRingRadius;

  G4double WCBarrelRingNPhi;
  G4double WCBarrelNRings;
  G4double WCPMTperCellHorizontal;
  G4double WCPMTperCellVertical;

  G4double WCPMTPercentCoverage;

  G4double WCBarrelNumPMTHorizontal;
  G4double WCCapPMTSpacing;
  G4double WCCapEdgeWidth;//jh TODO: not used
  
  G4double WCCapEdgeLimit;
  G4double WCBlackSheetThickness;

// raise scope of derived parameters
  G4double WCIDRadius;
  G4double totalAngle;
  G4double dPhi;
  G4double barrelCellHeight;
  G4double mainAnnulusHeight;
  G4double innerAnnulusRadius;
  G4double outerAnnulusRadius;
  G4String water;


 //for 1kt
  G4double WCDiameter;
  G4double WCRadius;
  G4double WCBarrelPMTRadius;
  G4double WCBarrelRingdPhi;
  G4double WCBarrelCellLength;
  G4double WCCapNCell;
  G4double WCBarrelLength;

  // amb79: to universally make changes in structure and geometry
  bool isUpright;

  // *** Begin egg-shaped HyperK Geometry ***

    void MatchWCSimAndEggShapedHyperK();
    G4LogicalVolume* ConstructEggShapedHyperK();

    G4Material* FindMaterial(G4String);

    G4VSolid* ConstructHalf(G4double, G4double);

    G4LogicalVolume* ConstructRadialPMT(G4bool,
                                        G4double, G4double,
                                        G4double, G4double,
                                        G4double, G4double);

    G4LogicalVolume* ConstructEndWallPMT();

    G4LogicalVolume* ConstructCeilingPMT(G4bool,
                                         G4double, G4double,
                                         G4double, G4double);

    G4bool isEggShapedHyperK;

    G4double waterTank_TopR;
    G4double waterTank_BotR;
    G4double waterTank_Height;
    G4double waterTank_UpperA;
    G4double waterTank_LowerB;
    G4double waterTank_Length;

    G4double innerPMT_TopR;
    G4double innerPMT_BotR;
    G4double innerPMT_TopW;
    G4double innerPMT_BotW;
    G4double innerPMT_Height;
    G4double innerPMT_Radius;
    G4double innerPMT_Expose;
    G4double innerPMT_Rpitch;
    G4double innerPMT_Apitch;

    G4double outerPMT_TopR;
    G4double outerPMT_BotR;
    G4double outerPMT_TopW;
    G4double outerPMT_BotW;
    G4double outerPMT_Height;
    G4double outerPMT_Radius;
    G4double outerPMT_Expose;
    G4String outerPMT_Name;
    G4double outerPMT_TopRpitch;
    G4double outerPMT_BotRpitch;
    G4double outerPMT_Apitch;

    G4double blackSheetThickness;

    G4int innerPMT_TopN;
    G4int innerPMT_BotN;

    G4bool checkOverlaps;
    G4LogicalVolume* waterTankLV;

    G4int PMTCopyNo;
    G4int wallSlabCopyNo;

  // *** End egg-shaped HyperK Geometry ***

  // amb79: debug to display all parts
  bool debugMode;

  // Variables related to the geometry

  std::ofstream geoFile;   // File for text output

  G4int totalNumPMTs;      // The number of PMTs for this configuration     
  G4double WCCylInfo[3];    // Info for the geometry tree: radius & length or mail box, length, width and depth
  G4double WCPMTSize;       // Info for the geometry tree: pmt size
  G4ThreeVector WCOffset;   // Info for the geometry tree: WC center offset

  // Tube map information

  static std::map<int, G4Transform3D> tubeIDMap;
//  static std::map<int, cyl_location> tubeCylLocation;
  static hash_map<std::string, int, hash<std::string> >  tubeLocationMap; 
 
  // Variables related to configuration

  G4int myConfiguration;   // Detector Config Parameter
  G4double innerradius;
 
  std::vector<WCSimPmtInfo*> fpmts;
  
  // MRD & Veto variables
  // ====================================================
  public:
  G4LogicalVolume* ConstructANNIE();
  void DefineMRD(G4PVPlacement* expHall);
  void DefineANNIEdimensions();
  void AddANNIEPhase1PMTs(G4LogicalVolume* waterTank_log);
  void AddANNIEPhase2PMTs(G4LogicalVolume* waterTank_log);
  void ConstructMRD(G4LogicalVolume* expHall_log, G4VPhysicalVolume* expHall_phys);
  void ConstructVETO(G4LogicalVolume* expHall_log, G4VPhysicalVolume* expHall_phys);
  void ConstructNCV(G4LogicalVolume* waterTank_log);
  void ComputePaddleTransformation (const G4int copyNo, G4VPhysicalVolume* physVol);
  void ComputeTaperTransformation (const G4int copyNo, G4VPhysicalVolume* physVol, G4int selector, G4bool additionaloffset);
  void ComputeSteelTransformation (const G4int copyNo, G4VPhysicalVolume* physVol);
  void ComputeVetoPaddleTransformation (const G4int copyNo, G4VPhysicalVolume* physVol, G4int selector);
  void PlacePaddles(G4LogicalVolume* totMRD_log);
  void PlaceTapers(G4LogicalVolume* totMRD_log);
  void PlaceLGs(G4LogicalVolume* totMRD_log);
  void PlaceMRDSDSurfs(G4LogicalVolume* totMRD_log);
  void PlaceMRDPMTs(G4LogicalVolume* totMRD_log);
  void PlaceMylarOnMRDPaddles(G4VPhysicalVolume* expHall_phys, G4VPhysicalVolume* totMRD_phys);
  void PlaceSteels(G4LogicalVolume* totMRD_log);
  void makeAlu(G4AssemblyVolume* totMRD);
  void PlaceVetoPaddles(G4LogicalVolume* totVeto_log);
  void PlaceVetoLGs(G4LogicalVolume* totVeto_log);
  void PlaceVetoSDsurfs(G4LogicalVolume* totVeto_log);
  void PlaceVetoPMTs(G4LogicalVolume* totVeto_log);
  void PlaceMylarOnFACCPaddles(G4VPhysicalVolume* expHall_phys, G4VPhysicalVolume* totVeto_phys);
  G4String GetMRDPMTName()			  {return MRDPMTName;}
  G4String GetFACCPMTName()			  {return FACCPMTName;}
  G4String GetMRDCollectionName(){return WCMRDCollectionName;}
  G4String GetFACCCollectionName(){return WCFACCCollectionName;}
//  private:

  G4bool isANNIE;
  G4String GDMLFilename;
  G4double tankouterRadius;
  G4double tankhy;
  G4double tankzoffset;
  G4double tankyoffset;
  G4double expHall_x;
  G4double expHall_y;
  G4double expHall_z;
  G4int doOverlapCheck;
  
  G4String WCMRDCollectionName;
  G4String WCFACCCollectionName;
  G4String MRDPMTName;
  G4double MRDPMTExposeHeight;
  G4double MRDPMTRadius;
  G4String FACCPMTName;
  G4double FACCPMTExposeHeight;
  G4double FACCPMTRadius;
  WCSimWCSD* afacc;
  WCSimWCSD* aMRDPMT;
  
  // info for map of PMT locations
  G4int totalNumMrdPMTs;
  G4int totalNumFaccPMTs;
  static std::map<int, G4Transform3D> mrdtubeIDMap; 
  static std::map<int, G4Transform3D> facctubeIDMap;
  static hash_map<std::string, int, hash<std::string> > mrdtubeLocationMap;
  static hash_map<std::string, int, hash<std::string> > facctubeLocationMap; 
  std::vector<WCSimPmtInfo*> fmrdpmts, ffaccpmts;
  std::vector<WCSimPmtInfo*>* Get_MrdPmts() {return &fmrdpmts;}
  std::vector<WCSimPmtInfo*>* Get_FaccPmts() {return &ffaccpmts;}
  
	G4double Xposition, Yposition, Zposition;		// used for positioning parameterisations.
	G4int numpaddlesperpanelh;									// paddles per h scintillator panel
	G4int numpaddlesperpanelv;									// paddles per v scintillator panel
	G4int numpanels;														// scintillator panels
	G4int numhpanels;														// horizontal scintillator panels
	G4int numvpanels;														// vertical scintillator panels
	G4int numrpcs;															// rpc panels
	G4int numplates;														// steel plates
	G4int numalustructs;												// number of supporting structs. We may be dropping one as we have fewer scintillators?
	G4int numvetopaddles;												// number of scintillator paddles in the FACC; 13 panels in 2 layers
	G4int vetopaddlesperpanel;									// number of scintillator paddles in each FACC panel

	G4double scintbordergap;										// gap between each scintillator (cm) (border to account for cladding etc)
	G4double steelscintgap;											// gap between steel and scintillator
	G4double scintalugap;												// gap between scintillator and alu struct
	G4double alusteelgap; 											// gap between alu struct and subsequent steel of next layer
	G4double layergap;													// total gaps of layers

	G4double steelfullxlen;
	G4double steelfullylen;
	G4double steelfullzlen;

	G4double scintfullxlen;
	G4double scintfullzlen;
	G4double scinthfullylen;
	G4double scintvfullylen;

	G4double scinttapfullwidth; 								// width of the tapering part of scintillator paddles at the narrow end
	G4double scinttapfullheight; 								// z length of tapering part of scint paddles.

	G4double scintlgfullwidth; 									// tapered light guides at narrow end
	G4double scintlgfullheight;

	G4double alufullxlen;												// outer thicknesses - total frame dims are about those of the steel plate
	G4double alufullylen;												// 
	G4double alufullzlen;												// 
	G4double alufullxthickness;									// 
	G4double alufullythickness;
	G4double windowwidth;												// (full length - 4 beams) / 3 windows
	G4double windowheight;
	
	G4double mrdpmtfullheight;									// full length of MRD PMTs

	G4double mrdZlen; 

	G4double vetopaddlefullxlen;
	G4double vetopaddlefullylen;
	G4double vetopaddlefullzlen;
	G4double vetolayer2offset;
	G4double vetopaddlegap;
	G4double nothickness;
	
	G4double vetolgfullxlen;								// width of the veto light guides at narrow end
	G4double vetolgfullylen;
	
	G4double vetopmtfullheight;

	G4double vetoZlen;

	G4double maxwidth;
	G4double maxheight;
	G4double mrdZoffset;

	// Define solids 
	//==============  
	// G4Box* variableName = new G4Box("SolidName", x_halflength, y_halflength, z_halflength);
	// G4Trd("SolidName", x_halflength1, x_halflength2, y_halflength1, y_halflength2, z_halflength); 
	// 2 x and y dimensions given - define dims of the cross-sections at the two z ends. 

	G4Box* sciMRDhpaddle_box;										// Paddles - h 
	G4Box* sciMRDvpaddle_box;										// and v
	G4Trd* mrdScintTap_box;											// Paddle Tapered ends

	G4Trd* mrdLG_box;														// Tapered Light Guides
	G4Box* mrdSurface_box;											// little box for surface to detect photons

	G4Box* steelMRDplate_box;										// Steel plates

	G4Box* aluMRDstruc_box;											// The alu support structure is roughly the external size of the steel plates...
	G4Box* aluMRDwindow_box;										// with a 3x3 grid of ~even sized holes  

	G4Box* totMRD_box;

	G4Box* vetoPaddle_box;
	G4Box* vetoSurface_box;
	G4Box* totVeto_box;
	G4Trd* vetoLG_box;
	
	
	// Define logical volumes 
	//=======================
	// G4LogicalVolume* variableName = new G4LogicalVolume(solidVariableName, Material, "logicalVolName");
	// Can't do this outside a function - the materials aren't defined yet.

	G4LogicalVolume* hpaddle_log;
	G4LogicalVolume* vpaddle_log;
	G4LogicalVolume* taper_log;
	G4LogicalVolume* lg_log;
	G4LogicalVolume* steel_log;
	G4LogicalVolume* mrdsdsurf_log;

	G4LogicalVolume* vetoPaddle_log;
	G4LogicalVolume* vetol2Paddle_log;
	G4LogicalVolume* vetoSurface_log;
	G4LogicalVolume* vetolg_log;
	
	G4LogicalVolume* logicMRDPMT;
	G4LogicalVolume* logicFACCPMT;
	
	G4OpticalSurface* scintSurface_op;	// mylar reflective surface
	G4OpticalSurface* lgSurface_op;			// optical surface with efficiency of detection defined, for boundary detection on ends of LGs

	// Physical Volumes
	// ================
	// to place optical surface between tapers and LGs for optical detection we need pointers to the physical volumes
	// declare containers for the pointers here, they'll be filled by the placement function.
	std::vector<G4VPhysicalVolume*> paddles_phys;
	std::vector<G4VPhysicalVolume*> tapers_phys;
	std::vector<G4VPhysicalVolume*> lgs_phys;
	std::vector<G4VPhysicalVolume*> mrdsdsurfs_phys;
	std::vector<G4VPhysicalVolume*> vetopaddles_phys;
	std::vector<G4VPhysicalVolume*> vetosurfaces_phys;
	std::vector<G4VPhysicalVolume*> vetolgs_phys;

	// Define rotation matrices 
	//=========================
	// rotated and unrotated scintillator paddles. Could do away with one by changing dims but hey.
	G4RotationMatrix* noRot;										// null rotation pointer
	G4RotationMatrix* rotatedmatx;							// horizontal config for scint paddles

	// Note trapezium narrows along z axis, so need to define a rotation of 90deg about the y(??)-axis to bring taper into the x-y plane. 
	G4RotationMatrix* upmtx;
	G4RotationMatrix* downmtx;
	G4RotationMatrix* rightmtx;
	G4RotationMatrix* leftmtx;

	// Define Visualisation Attributes 
	//================================
	G4VisAttributes* scinthatts;
	G4VisAttributes* scintvatts;
	G4VisAttributes* steelatts;
	G4VisAttributes* scinttapatts;
	G4VisAttributes* scintlgatts;
	G4VisAttributes* senssurfatts;
	
  // End MRD & Veto variables
  // ====================================================
  
  public:
    // ****************SciBooNE integration
    int startindex;
    G4bool useadditionaloffset;
    
    G4Material *Air, *Al, *Fe, *Pb;
    G4Material *Steel, *Scinti, *Lucite;
    G4Material *ECScinti, *MRDIron;
    G4Material *Plywood;
    
//		// for MRD
//		G4VSolid *MRDIron_Solid[12];
//		G4VSolid *MRDVScinti_Solid, *MRDHScinti_Solid;
//		G4VSolid *MRDTScinti_Solid;
//		G4VSolid *MRDLG_Solid;

//		G4LogicalVolume *MRDIron_LV[12];
//		G4LogicalVolume *MRDVScinti_LV, *MRDHScinti_LV;
//		G4LogicalVolume *MRDTScinti_LV;
//		G4LogicalVolume *MRDLG_LV;

//		// for MRD Al support
//		G4SubtractionSolid *MRDAlV1_Solid;
//		G4SubtractionSolid *MRDAlV2_Solid;
//		G4SubtractionSolid *MRDAlV3_Solid;
//		G4SubtractionSolid *MRDAlV4_Solid;
//		G4SubtractionSolid *MRDAlV5_Solid;
//		G4SubtractionSolid *MRDAlH1_Solid;
//		G4SubtractionSolid *MRDAlH2_Solid;
//		G4SubtractionSolid *MRDAlH3_Solid;

//		G4VSolid *MRDAlV1_Outer;
//		G4VSolid *MRDAlV2_Outer;
//		G4VSolid *MRDAlV3_Outer;
//		G4VSolid *MRDAlV4_Outer;
//		G4VSolid *MRDAlV5_Outer;
//		G4VSolid *MRDAlH1_Outer;
//		G4VSolid *MRDAlH2_Outer;
//		G4VSolid *MRDAlH3_Outer;

//		G4VSolid *MRDAlV1_Inner;
//		G4VSolid *MRDAlV2_Inner;
//		G4VSolid *MRDAlV3_Inner;
//		G4VSolid *MRDAlV4_Inner;
//		G4VSolid *MRDAlV5_Inner;
//		G4VSolid *MRDAlH1_Inner;
//		G4VSolid *MRDAlH2_Inner;
//		G4VSolid *MRDAlH3_Inner;

//		G4LogicalVolume *MRDAlV1_LV;
//		G4LogicalVolume *MRDAlV2_LV;
//		G4LogicalVolume *MRDAlV3_LV;
//		G4LogicalVolume *MRDAlV4_LV;
//		G4LogicalVolume *MRDAlV5_LV;
//		G4LogicalVolume *MRDAlH1_LV;
//		G4LogicalVolume *MRDAlH2_LV;
//		G4LogicalVolume *MRDAlH3_LV;

//		G4AssemblyVolume * MRDAlSupportV;
//		G4AssemblyVolume * MRDAlSupportH;
//    // *********/SciBooNE integration
  
};

#endif


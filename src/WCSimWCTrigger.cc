#include "WCSimWCTrigger.hh"
#include "WCSimWCPMT.hh"
#include "WCSimWCLAPPD.hh"
#include "WCSimWCDigi.hh"
#include "WCSimWCHit.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4DigiManager.hh"
#include "G4ios.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"

#include "WCSimDetectorConstruction.hh"
#include "WCSimPmtInfo.hh"
#include "WCSimDarkRateMessenger.hh"

#include <vector>
// for memset
#include <cstring>
#include <algorithm>
#include <exception>
#include <limits>


// *******************************************
// BASE CLASS
// *******************************************

#ifndef WCSIMWCTRIGGER_VERBOSE
//#define WCSIMWCTRIGGER_VERBOSE
#endif

#ifndef HYPER_VERBOSITY
//#define HYPER_VERBOSITY
#endif

const float WCSimWCTriggerBase::offset = 0.;//950.0; // ns. apply offset to the digit time
const double WCSimWCTriggerBase::LongTime = 1E6; // ns = 1ms. event time


WCSimWCTriggerBase::WCSimWCTriggerBase(G4String name,
				       WCSimDetectorConstruction* inDetector,
				       WCSimWCDAQMessenger* myMessenger, G4String detectorElement)
  :G4VDigitizerModule(name), DAQMessenger(myMessenger), myDetector(inDetector), triggerClassName(""), detectorElement(detectorElement)
{
  G4String colName;
  if(detectorElement=="tank"){
    colName = "WCDigitizedCollection";
  } else if(detectorElement=="mrd"){
    colName = "WCDigitizedCollection_MRD";
  } else if(detectorElement=="facc"){
    colName = "WCDigitizedCollection_FACC";
  }
  collectionName.push_back(colName);
#ifdef HYPER_VERBOSITY
  if(detectorElement=="mrd")
  {G4cout<<"WCSimWCTriggerBase::WCSimWCTriggerBase ☆ recording collection name "<<colName<<" for "<<detectorElement<<G4endl;}
#endif
  ReInitialize();

  if(DAQMessenger == NULL) {
    G4cerr << "WCSimWCDAQMessenger pointer is NULL when passed to WCSimWCTriggerBase constructor. Exiting..."
           << G4endl;
    exit(-1);
  }

  digitizeCalled = false;

}

WCSimWCTriggerBase::~WCSimWCTriggerBase(){
}


void WCSimWCTriggerBase::GetVariables()
{
  //set the options to class-specific defaults
  multiDigitsPerTrigger    = GetDefaultMultiDigitsPerTrigger();
  ndigitsThreshold         = GetDefaultNDigitsThreshold();
  ndigitsWindow            = GetDefaultNDigitsWindow();
  ndigitsPreTriggerWindow  = GetDefaultNDigitsPreTriggerWindow();
  ndigitsPostTriggerWindow = GetDefaultNDigitsPostTriggerWindow();
  enablePromptTrigger      = GetDefaultPromptTrigger();
  promptPreTriggerWindow   = GetDefaultPromptPreTriggerWindow();
  promptPostTriggerWindow  = GetDefaultPromptPostTriggerWindow();

  //read the .mac file to override them
  if(DAQMessenger != NULL) {
    DAQMessenger->TellMeAboutTheTrigger(this);
    DAQMessenger->SetTriggerOptions(detectorElement);
  }
  else {
    G4cerr << "WCSimWCDAQMessenger pointer is NULL when used in WCSimWCTriggerBase::GetVariables(). Exiting..." 
	   << G4endl;
    exit(-1);
  }

#ifdef WCSIMWCTRIGGER_VERBOSE
if(detectorElement=="tank"){
  G4cout << (multiDigitsPerTrigger ? "Using mutiple digits per PMT per trigger" : "Using a maximum of 1 digit per PMT per trigger" ) << G4endl
	 << "Using NDigits threshold " << ndigitsThreshold
	 << (ndigitsAdjustForNoise ? " (will be adjusted for noise)" : "") << G4endl
	 << "Using NDigits trigger window " << ndigitsWindow << " ns" << G4endl
	 << "Using NDigits event pretrigger window " << ndigitsPreTriggerWindow << " ns" << G4endl
	 << "Using NDigits event posttrigger window " << ndigitsPostTriggerWindow << " ns" << G4endl;
  if(saveFailuresMode == 0)
    G4cout << "Saving only triggered digits" << G4endl;
  else if(saveFailuresMode == 1)
    G4cout << "Saving both triggered and not-triggered digits" << G4endl;
  else if(saveFailuresMode == 2)
    G4cout << "Saving only not-triggered digits" << G4endl;
  if(saveFailuresMode > 0)
    G4cout << "Using SaveFailures trigger time" << saveFailuresTime << " ns" << G4endl
	   << "Using SaveFailures event pretrigger window " << saveFailuresPreTriggerWindow << " ns" << G4endl
	   << "Using SaveFailures event posttrigger window " << saveFailuresPostTriggerWindow << " ns" << G4endl;
  if(enablePromptTrigger){
    G4cout << "Recording digits in a prompt window at the start of the event" << G4endl
           << "Using Prompt trigger window " << promptPostTriggerWindow << " ns" << G4endl;
  }
}
#endif
}

int WCSimWCTriggerBase::GetPreTriggerWindow(TriggerType_t t)
{
  switch(t) {
  case kTriggerNoTrig:
    return 0;
    break;
  case kTriggerNDigits:
  case kTriggerNDigitsTest:
  case kTriggerTankDigits:
    return ndigitsPreTriggerWindow;
    break;
  case kTriggerFailure:
    return saveFailuresPreTriggerWindow;
    break;
  case kPromptTrigger:
    return promptPreTriggerWindow;
    break;
  default:
    G4cerr << "WCSimWCTriggerBase::GetPreTriggerWindow() Unknown trigger type " << t
	   << "\t" << WCSimEnumerations::EnumAsString(t) << G4endl;
    exit(-1);
    break;
  }
}

int WCSimWCTriggerBase::GetPostTriggerWindow(TriggerType_t t)
{
  switch(t) {
  case kTriggerNoTrig:
    return WCSimWCTriggerBase::LongTime;
    break;
  case kTriggerNDigits:
  case kTriggerNDigitsTest:
  case kTriggerTankDigits:
    return ndigitsPostTriggerWindow;
    break;
  case kTriggerFailure:
    return saveFailuresPostTriggerWindow;
    break;
  case kPromptTrigger:
    return promptPostTriggerWindow;
    break;
  default:
    G4cerr << "WCSimWCTriggerBase::GetPostTriggerWindow() Unknown trigger type " << t
	   << "\t" << WCSimEnumerations::EnumAsString(t) << G4endl;
    exit(-1);
    break;
  }
}

void WCSimWCTriggerBase::AdjustNDigitsThresholdForNoise()
{
  int npmts;
  if(detectorElement=="tank"){
    npmts = this->myDetector->GetTotalNumPmts();
  } else if(detectorElement=="mrd"){
    npmts = this->myDetector->GetTotalNumMrdPmts();
  } else if(detectorElement=="facc"){
    npmts = this->myDetector->GetTotalNumFaccPmts();
  }
  double trigger_window_seconds = ndigitsWindow * 1E-9;
  double dark_rate_Hz = PMTDarkRate * 1000;
  double average_occupancy = dark_rate_Hz * trigger_window_seconds * npmts;
  
  
#ifdef WCSIMWCTRIGGER_VERBOSE
  G4cout << "Average number of PMTs in detector active in a " << ndigitsWindow
	 << "ns window with a dark noise rate of " << PMTDarkRate
	 << "kHz is " << average_occupancy
	 << " (" << npmts << " total PMTs)"
	 << G4endl
	 << "Updating the NDigits threshold, from " << ndigitsThreshold
	 << " to " << ndigitsThreshold + round(average_occupancy) << G4endl;
#endif
  ndigitsThreshold += round(average_occupancy);
}

void WCSimWCTriggerBase::Digitize()
{
  if(ndigitsAdjustForNoise && !digitizeCalled) {
#ifdef HYPER_VERBOSITY
    if(detectorElement=="mrd"){G4cout<<"WCSimWCTriggerBase::Digitize ☆ adjusting threshold for average occupancy"<<G4endl;}
#endif
    AdjustNDigitsThresholdForNoise();
    digitizeCalled = true;
  }

  //Input is collection of all digitized hits that passed the threshold
  //Output is all digitized hits which pass the trigger
  
  ReInitialize();

  //This is the output digit collection
  G4String collectiondetectorname;
  if(detectorElement=="tank"){
    collectiondetectorname="/WCSim/glassFaceWCPMT";
  } else if(detectorElement=="mrd"){
    collectiondetectorname="/WCSim/glassFaceWCPMT_MRD";
  } else if(detectorElement=="facc"){
    collectiondetectorname="/WCSim/glassFaceWCPMT_FACC";
  }
  DigitsCollection = new WCSimWCTriggeredDigitsCollection (collectiondetectorname,collectionName[0]);
  G4DigiManager* DigiMan = G4DigiManager::GetDMpointer();

  // Get the Digitized hits collection ID
  G4String untriggeredcollectionname;
  if(detectorElement=="tank"){
    untriggeredcollectionname="WCDigitizedStoreCollection";
  } else if(detectorElement=="mrd"){
    untriggeredcollectionname = "WCDigitizedStoreCollection_MRD";
  } else if(detectorElement=="facc"){
    untriggeredcollectionname = "WCDigitizedStoreCollection_FACC";
  }
  G4int WCDCID = DigiMan->GetDigiCollectionID(untriggeredcollectionname);
  // Get the PMT Digits Collection
  WCSimWCDigitsCollection* WCDCPMT = (WCSimWCDigitsCollection*)(DigiMan->GetDigiCollection(WCDCID));
#ifdef HYPER_VERBOSITY
  if(detectorElement=="mrd"){
  G4cout<<"WCSimWCTriggerBase::Digitize ☆ making triggered digits collection "<<collectionName[0]<<" for "<<detectorElement
  <<" and calling DoTheWork on "<<untriggeredcollectionname<<" to fill it."<<G4endl;}
#endif

  // Do the work  
  if (WCDCPMT) {
    DoTheWork(WCDCPMT);
  } else {G4cerr<<"could not find trigger PMT digits collection for "<<detectorElement<<G4endl;}
  
#ifdef HYPER_VERBOSITY
  if(detectorElement=="mrd"){G4cout<<"WCSimWCTriggerBase::Digitize ☆ storing the triggered digits collection "<<collectionName[0]
        <<" which has "<<DigitsCollection->entries()<<" entries"<<G4endl;}
#endif
  StoreDigiCollection(DigitsCollection);

}

void WCSimWCTriggerBase::AlgNDigits(WCSimWCDigitsCollection* WCDCPMT, bool remove_hits, bool test)
{

  //if test is true, we run the algorithm with 1/2 the threshold, and kTriggerNDigitsTest
  //for testing multiple trigger algorithms at once
  int this_ndigitsThreshold = ndigitsThreshold;
  TriggerType_t this_triggerType = kTriggerNDigits;
  if(test) {
    this_ndigitsThreshold /= 2;
    this_triggerType = kTriggerNDigitsTest;
  }

  //Now we will try to find triggers
  //loop over PMTs, and Digits in each PMT.  If ndigits > Threshhold in a time window, then we have a trigger

  int ntrig = 0;
  int window_start_time = 0;
  int window_end_time   = WCSimWCTriggerBase::LongTime - ndigitsWindow;
  int window_step_size  = 5; //step the search window along this amount if no trigger is found
  float lasthit;
  std::vector<int> digit_times;
  bool first_loop = true;

  G4cout << "WCSimWCTriggerBase::AlgNDigits. Number of entries in input digit collection: " << WCDCPMT->entries() << G4endl;
#ifdef WCSIMWCTRIGGER_VERBOSE
  int temp_total_pe = 0;
  for (G4int i = 0 ; i < WCDCPMT->entries() ; i++) {
    temp_total_pe += (*WCDCPMT)[i]->GetTotalPe();
  }
  G4cout << "WCSimWCTriggerBase::AlgNDigits. " << temp_total_pe << " total p.e. input" << G4endl;
#endif

  // the upper time limit is set to the final possible full trigger window
  while(window_start_time <= window_end_time) {
    int n_digits = 0;
    float triggertime; //save each digit time, because the trigger time is the time of the first hit above threshold
    bool triggerfound = false;
    digit_times.clear();
    
    //Loop over each PMT
    for (G4int i = 0 ; i < WCDCPMT->entries() ; i++) {
      //int tube=(*WCDCPMT)[i]->GetTubeID();
      //Loop over each Digit in this PMT
      //G4int atotalpe = (*WCDCPMT)[i]->GetTotalPe();
      // When stacking triggers, some digits are removed from the map, so the keys are no longer monotonic.
      // Use an iterator to loop over the digits present
      for (std::map<int,float>::const_iterator digit_time_it = (*WCDCPMT)[i]->GetTimeMapBegin();
               digit_time_it!=(*WCDCPMT)[i]->GetTimeMapEnd(); digit_time_it++) {
        int ip = digit_time_it->first;
        int digit_time=0;
      	try{
	  G4float temp_time = (*WCDCPMT)[i]->GetTime(ip);
	  if(temp_time<(std::numeric_limits<int>::max())){digit_time = (int)temp_time;} else {digit_time=-999;};
	}
	catch (...){
	  G4cerr<<"Exception in WCSimWCTriggerBase::AlgNDigits call to WCSimWCDigi::GetTime "
	        <<G4endl<<"Attempt to retrieve time from pe "<<ip<<" in WCDCPMT entry "<<i<<G4endl;
	  G4cerr<<"The digi had "<<(*WCDCPMT)[i]->GetTotalPe()<<" total pe's."<<G4endl;
	  digit_time=-998;
	}
	//hit in trigger window?
	if(digit_time >= window_start_time && digit_time <= (window_start_time + ndigitsWindow)) {
	  n_digits++;
	  digit_times.push_back(digit_time);
	}
	//get the time of the last hit (to make the loop shorter)
	if(first_loop && (digit_time > lasthit))
	  lasthit = digit_time;
      }//loop over Digits
    }//loop over PMTs

    //if over threshold, issue trigger
    if(n_digits > this_ndigitsThreshold) {
      ntrig++;
      //The trigger time is the time of the first hit above threshold
      std::sort(digit_times.begin(), digit_times.end());
      triggertime = digit_times[this_ndigitsThreshold];
      // triggertime -= (int)triggertime % 5; // ANNIE: do not round trigger times to multiples of 5ns
      TriggerTimes.push_back(triggertime);
      TriggerTypes.push_back(this_triggerType);
      TriggerInfos.push_back(std::vector<Float_t>(1, n_digits));
      triggerfound = true;
    }

#ifdef WCSIMWCTRIGGER_VERBOSE
    if(n_digits)
      G4cout << n_digits << " digits found in "<<ndigitsWindow<<"nsec trigger window ["
	     << window_start_time << ", " << window_start_time + ndigitsWindow
	     << "]. Threshold is: " << this_ndigitsThreshold << G4endl;
#endif

    //move onto the next go through the timing loop
    if(triggerfound) {
      window_start_time = triggertime + GetPostTriggerWindow(TriggerTypes.back());
    }//triggerfound
    else {
      window_start_time += window_step_size;
    }

    //shorten the loop using the time of the last hit
    if(first_loop) {
#ifdef WCSIMWCTRIGGER_VERBOSE
      G4cout << "Last hit found to be at " << lasthit
	     << ". Changing window_end_time from " << window_end_time
	     << " to " << lasthit - (ndigitsWindow - 10)
	     << G4endl;
#endif
      window_end_time = lasthit - (ndigitsWindow - 10);
      first_loop = false;
    }
  }
  
  G4cout << "Found " << ntrig << " NDigit triggers" << G4endl;
  //call FillDigitsCollection() whether any triggers are found or not
  // (what's saved depends on saveFailuresMode)
  FillDigitsCollection(WCDCPMT, remove_hits, this_triggerType);
}

void WCSimWCTriggerBase::AlgTankDigits(WCSimWCDigitsCollection* WCDCPMT, bool remove_hits, bool test)
{
  
  /** this actually doesn't need to search for triggers, but instead needs to retrieve the Trigger vectors
   from the tank trigger class. The storing of digits within those trigger windows is then done by FillDigitsCollection().
   NOTE: Pre-Trigger Window, Post-Trigger Window and MultiDigitsPerTrigger are taken from THIS trigger
   instance, NOT the tank trigger!
  */
  
  // ensures we save tank trigger windows, regardless of which trigger type the tank is using
  TriggerType_t this_triggerType = kTriggerTankDigits;
  
  // need to retrieve the trigger vectors from the tank trigger class.
  // Get a pointer to the Digitizing Module Manager
  G4DigiManager* DMman = G4DigiManager::GetDMpointer();
  //Get a pointer to the tank Trigger Module
  WCSimWCTriggerBase* WCTM_tank = (WCSimWCTriggerBase*)DMman->FindDigitizerModule("WCReadout");
  // copy across all the information about the triggers it found.
  if(WCTM_tank){
    int numtanktriggers = WCTM_tank->NumberOfGatesInThisEvent();
    for(int i=0;i<numtanktriggers;i++){
      TriggerTimes.push_back(WCTM_tank->GetTriggerTime(i));
      TriggerInfos.push_back(WCTM_tank->GetTriggerInfo(i));
      TriggerTypes.push_back(WCTM_tank->GetTriggerType(i));
    }
  }
  
#ifdef HYPER_VERBOSITY
  if(detectorElement=="mrd"){
    G4String untriggeredcollectionname;
    if(detectorElement=="tank"){
      untriggeredcollectionname="WCDigitizedStoreCollection";
    } else if(detectorElement=="mrd"){
      untriggeredcollectionname = "WCDigitizedStoreCollection_MRD";
    } else if(detectorElement=="facc"){
      untriggeredcollectionname = "WCDigitizedStoreCollection_FACC";
    }
    G4cout<<"WCSimWCTriggerBase::AlgTankDigits ☆ retrieved "<<TriggerTimes.size()
          <<" trigger times from tank. Finding matching hits in "<< untriggeredcollectionname
          <<", which has "<<WCDCPMT->entries()<<" entries."<<G4endl;
  }
#endif
  // then fill this digits collection with the digits from the corresponding triggers
  FillDigitsCollection(WCDCPMT, remove_hits, this_triggerType);
}

void WCSimWCTriggerBase::FillDigitsCollection(WCSimWCDigitsCollection* WCDCPMT, bool remove_hits, TriggerType_t save_triggerType)
{
  //Adds the digits within the trigger window to the output WCSimWCDigitsCollection
  // optionally removes digits from the input digits collection (when running different Alg* methods concurently) 
  // so they are not used in subsequent trigger decisions or saved twice
  //Also, only save digits of a specific type (again for when running different Alg* methods concurently)

  // Add dummy triggers / exit without saving triggers as required
  //
  //saveFailuresMode = 0 - save only triggered events
  //saveFailuresMode = 1 - save both triggered & not triggered events
  //saveFailuresMode = 2 - save only not triggered events
  if(TriggerTimes.size()) {
    if(saveFailuresMode == 2)
      return;
  }
  else {
#ifdef WCSIMWCTRIGGER_VERBOSE
    G4cout<<"no triggers in this event"<<G4endl;
#endif
    if(saveFailuresMode == 0)
      return;
    TriggerTypes.push_back(kTriggerFailure);
    TriggerTimes.push_back(saveFailuresTime);
    TriggerInfos.push_back(std::vector<Float_t>(1, -1));
    save_triggerType = kTriggerFailure;
  }

  std::vector<int> triggerstoremove;	// when using trigger on tank, remove triggers if they have no hits.
  //make sure the triggers are in time order
  SortTriggersByTime();
  //Loop over trigger times
  for(unsigned int itrigger = 0; itrigger < TriggerTimes.size(); itrigger++) {
    TriggerType_t triggertype = TriggerTypes[itrigger];
    //check if we've already saved this trigger
    // skip this digit if it's the wrong trigger type, or for kTriggerTankDigits, trigger on all digits
    if( triggertype != save_triggerType && save_triggerType!=kTriggerTankDigits && save_triggerType != kTriggerUndefined )
      continue;
    float         triggertime = TriggerTimes[itrigger];
    std::vector<Float_t> triggerinfo = TriggerInfos[itrigger];

    //these are the boundary of the trigger gate: we want to add all digits within these bounds to the output collection
    float lowerbound = triggertime + GetPreTriggerWindow(triggertype);
    float upperbound = triggertime + GetPostTriggerWindow(triggertype);
    //need to check for double-counting - check if the previous upperbound is above the lowerbound
    if(itrigger) {
      float upperbound_previous = TriggerTimes[itrigger - 1] + GetPostTriggerWindow(TriggerTypes[itrigger - 1]);
      if(upperbound_previous > lowerbound) {
	//also need to check whether the previous upperbound is above the upperbound
	//(different trigger windows for different trigger types can mean this trigger is completely contained within another)
	// if it is, we skip it
	if(upperbound_previous >= upperbound)
	  continue;
	lowerbound = upperbound_previous;
      }
    }

#ifdef WCSIMWCTRIGGER_VERBOSE
    G4cout << "Saving trigger " << itrigger << " of type " << WCSimEnumerations::EnumAsString(triggertype)
	   << " in time range [" << lowerbound << ", " << upperbound << "]"
	   << " with trigger time " << triggertime
	   << " and additional trigger info";
    for(std::vector<Float_t>::iterator it = triggerinfo.begin(); it != triggerinfo.end(); ++it)
      G4cout << " " << *it;
    G4cout << G4endl;
#endif
    int digitsinthistrigger=0;
    //loop over PMTs
    for (G4int i = 0; i < WCDCPMT->entries(); i++) {
      int tube=(*WCDCPMT)[i]->GetTubeID();
      //loop over digits in this PMT
      for (std::map<int,float>::const_iterator digit_time_it = (*WCDCPMT)[i]->GetTimeMapBegin();
               digit_time_it!=(*WCDCPMT)[i]->GetTimeMapEnd(); digit_time_it++) {
        int ip = digit_time_it->first;
	G4float digit_time=0;
	try{
	  G4float temp_time = (*WCDCPMT)[i]->GetTime(ip);
	  if(temp_time<(std::numeric_limits<float>::max())) {digit_time = temp_time;} else {digit_time=-999;}
	}
	catch (...){
	  G4cerr<<"Exception in WCSimWCTriggerBase::FillDigitsCollection call to WCSimWCDigi::GetTime "
	        <<G4endl<<"Attempt to retrieve time from pe "<<ip<<" in WCDCPMT entry "<<i<<G4endl;
	  G4cerr<<"The digi had "<<(*WCDCPMT)[i]->GetTotalPe()<<" total pe's."<<G4endl;
	  //assert(false);
	  digit_time=-996;
	}
	if(digit_time >= lowerbound && digit_time <= upperbound) {
	  //hit in event window
	  digitsinthistrigger++;
	  //add it to DigitsCollection

	  //first apply time offsets
	  float peSmeared = (*WCDCPMT)[i]->GetPe(ip);
	  float digihittime = 
	     -triggertime + WCSimWCTriggerBase::offset +	// temporarily disable for Bonsai
	     digit_time;

	  //get the composition information for the triggered digit
	  std::vector<int> triggered_composition = (*WCDCPMT)[i]->GetDigiCompositionInfo(ip);

#ifdef WCSIMWCTRIGGER_VERBOSE
	  G4cout << "Saving digit on PMT " << tube
		 << " time " << digihittime
		 << " pe "   << peSmeared
		 << " digicomp";
	  for(unsigned int iv = 0; iv < triggered_composition.size(); iv++)
	    G4cout << " " << triggered_composition[iv];
	  G4cout << G4endl;
#endif
	  assert(triggered_composition.size());

	  //add hit
	  if ( DigiHitMap[tube] == 0) {
	    //this PMT has no digits saved yet; create a new WCSimWCDigiTrigger
	    WCSimWCDigiTrigger* Digi = new WCSimWCDigiTrigger();
	    Digi->SetTubeID(tube);
	    Digi->AddGate  (itrigger);
	    Digi->SetTime  (itrigger,digihittime);
	    Digi->SetPe    (itrigger,peSmeared);
	    Digi->AddPe    ();
	    Digi->AddDigiCompositionInfo(itrigger,triggered_composition);
	    DigiHitMap[tube] = DigitsCollection->insert(Digi);
	  }
	  else {
	    //this PMT has digits saved already; add information to the WCSimWCDigiTrigger
	    (*DigitsCollection)[DigiHitMap[tube]-1]->AddGate(itrigger);
	    (*DigitsCollection)[DigiHitMap[tube]-1]->SetTime(itrigger, digihittime);
	    (*DigitsCollection)[DigiHitMap[tube]-1]->SetPe  (itrigger, peSmeared);
	    (*DigitsCollection)[DigiHitMap[tube]-1]->AddPe  ();
	    (*DigitsCollection)[DigiHitMap[tube]-1]->AddDigiCompositionInfo(itrigger,triggered_composition);
	  }
	  if(remove_hits)
	    (*WCDCPMT)[i]->RemoveDigitizedGate(ip);

	  //we've found a digit on this PMT. If we're restricting to just 1 digit per trigger window (e.g. SKI)
	  // then ignore later digits and break. This takes us to the next PMT
	  if(!multiDigitsPerTrigger)
	    break;
	}//digits within trigger window
      }//loop over Digits
    }//loop over PMTs
    if(digitsinthistrigger==0){
#ifdef WCSIMWCTRIGGER_VERBOSE
      G4cout<<"removing trigger "<<itrigger<<" as it contains no digits"<<G4endl;
#endif
      triggerstoremove.push_back(itrigger);
    }
  }//loop over Triggers
  std::reverse(triggerstoremove.begin(), triggerstoremove.end());
  for(int el=0; el<triggerstoremove.size(); el++){
    // remove this trigger from the current instance (for tank triggering case)
    TriggerTimes.erase(TriggerTimes.begin()+triggerstoremove.at(el));
    TriggerTypes.erase(TriggerTypes.begin()+triggerstoremove.at(el));
    TriggerInfos.erase(TriggerInfos.begin()+triggerstoremove.at(el));
  }
  G4cout << "WCSimWCTriggerBase::FillDigitsCollection. Number of entries in output digit collection: " << DigitsCollection->entries() << G4endl;

}

void WCSimWCTriggerBase::SaveOptionsToOutput(WCSimRootOptions * wcopt)
{
  wcopt->SetTriggerClassName(triggerClassName);
  wcopt->SetMultiDigitsPerTrigger(multiDigitsPerTrigger);
  //ndigits
  wcopt->SetNDigitsThreshold(ndigitsThreshold);
  wcopt->SetNDigitsWindow(ndigitsWindow);
  wcopt->SetNDigitsAdjustForNoise(ndigitsAdjustForNoise);
  wcopt->SetNDigitsPreTriggerWindow(ndigitsPreTriggerWindow);
  wcopt->SetNDigitsPostTriggerWindow(ndigitsPostTriggerWindow);
  //prompt/beam trigger
  wcopt->SetPromptTriggerEnabled(enablePromptTrigger);
  wcopt->SetPromptPreTriggerWindow(promptPreTriggerWindow);
  wcopt->SetPromptPostTriggerWindow(promptPostTriggerWindow);
  //savefailures
  wcopt->SetSaveFailuresMode(saveFailuresMode);
  wcopt->SetSaveFailuresTime(saveFailuresTime);
  wcopt->SetSaveFailuresPreTriggerWindow(saveFailuresPreTriggerWindow);
  wcopt->SetSaveFailuresPostTriggerWindow(saveFailuresPostTriggerWindow);
}



void WCSimWCTriggerBase::AlgNoTrigger(WCSimWCDigitsCollection* WCDCPMT, bool remove_hits, bool test) {

  //Does not doanything, just writes out all hits
  TriggerType_t this_triggerType = kTriggerNoTrig;
  std::vector<Float_t> triggerinfo;
  Int_t Ndigits=0;
  for (G4int i = 0 ; i < WCDCPMT->entries() ; i++) {
    for ( G4int ip = 0 ; ip < (*WCDCPMT)[i]->GetTotalPe() ; ip++) {
      Ndigits++;
    }
  }
  triggerinfo.push_back(Ndigits);
  TriggerTypes.push_back(kTriggerNoTrig);
  TriggerInfos.push_back(triggerinfo);
  TriggerTimes.push_back(0.);

  FillDigitsCollection(WCDCPMT, remove_hits, this_triggerType);
}

void WCSimWCTriggerBase::AlgPromptDigits(WCSimWCDigitsCollection* WCDCPMT, bool remove_hits, bool test) {

  //Does not doanything, just writes counts the hits in the prompt window
  TriggerType_t this_triggerType = kPromptTrigger;
  std::vector<Float_t> triggerinfo;
  Int_t Ndigits=0;
  int prompt_window_duration = GetPostTriggerWindow(this_triggerType);
  
  //loop over PMTs
  int totalndigits=0;
  for (G4int i = 0; i < WCDCPMT->entries(); i++) {
    //loop over digits in this PMT
    for (std::map<int,float>::const_iterator digit_time_it = (*WCDCPMT)[i]->GetTimeMapBegin();
             digit_time_it!=(*WCDCPMT)[i]->GetTimeMapEnd(); digit_time_it++) {
      totalndigits++;
      int ip = digit_time_it->first;
      int digit_time=0;
      try{
        G4float temp_time = (*WCDCPMT)[i]->GetTime(ip);
        if(temp_time<(std::numeric_limits<int>::max())){
          digit_time = static_cast<int>(temp_time);
        } else {digit_time=-999;}
      }
      catch (...){
        G4cerr<<"Exception in WCSimWCTriggerBase::FillDigitsCollection call to WCSimWCDigi::GetTime "
              <<G4endl<<"Attempt to retrieve time from pe "<<ip<<" in WCDCPMT entry "<<i<<G4endl;
        G4cerr<<"The digi had "<<(*WCDCPMT)[i]->GetTotalPe()<<" total pe's."<<G4endl;
        G4cerr<<"digit_time_it->first="<<digit_time_it->first<<", "
              <<"digit_time_it->second="<<digit_time_it->second;
        //assert(false);
        digit_time=-996;
      }
      //is hit in prompt window
      if(digit_time < prompt_window_duration) {
        Ndigits++;
      }
    }
  }
#ifdef HYPER_VERBOSITY
  if(detectorElement=="tank"){
    G4cout<<"WCSimWCTriggerBase::AlgPromptDigits ☆ capturing "<<Ndigits<<" digits"<<G4endl;
  }
#endif
  
#ifdef WCSIMWCTRIGGER_VERBOSE
  G4cout<<"Making prompt trigger for "<<detectorElement<<", containing "<<Ndigits<<"/"<<totalndigits<<" digits"<<G4endl;
#endif
  triggerinfo.push_back(Ndigits);
  TriggerTypes.push_back(kPromptTrigger);
  TriggerInfos.push_back(triggerinfo);
  TriggerTimes.push_back(0.);

  FillDigitsCollection(WCDCPMT, remove_hits, this_triggerType);
}

// *******************************************
// CONTAINER CLASS
// *******************************************

G4Allocator<WCSimWCDigiTrigger> WCSimWCDigiTriggerAllocator;

WCSimWCDigiTrigger::WCSimWCDigiTrigger()
{
  Gates.clear();
  tubeID = 0;
  pe.clear();
  time.clear();
  fDigiComp.clear();
  totalPe = 0;
}

WCSimWCDigiTrigger::~WCSimWCDigiTrigger(){;}

WCSimWCDigiTrigger::WCSimWCDigiTrigger(const WCSimWCDigiTrigger& right)
  :G4VDigi()
{
  // in principle assignment = is defined for containers...
  Gates = right.Gates;
  tubeID = right.tubeID;
  pe     = right.pe;
  time   = right.time;
  fDigiComp = right.fDigiComp;
  totalPe = right.totalPe;
}

const WCSimWCDigiTrigger& WCSimWCDigiTrigger::operator=(const WCSimWCDigiTrigger& right)
{
  Gates = right.Gates;
  tubeID = right.tubeID;
  pe     = right.pe;
  time   = right.time;
  fDigiComp = right.fDigiComp;
  totalPe = right.totalPe;
  return *this;
}

void WCSimWCDigiTrigger::Print()
{
  G4cout << "TubeID: " << tubeID
         << ", Number of Gates: " << NumberOfGates()
	 << G4endl;
  std::multimap<int,float>::iterator it_pe   = pe.begin();
  std::multimap<int,float>::iterator it_time = time.begin();
  for( ; it_pe != pe.end(), it_time != time.end(); ++it_pe, ++it_time) {
    if(it_pe->first != it_time->first) {
      G4cerr << "WCSimWCDigiTrigger::Print() pe and time gate counters disagree!" << G4endl;
      exit(-1);
    }
    G4cout  << "Gate = " << it_pe->first
            << " PE: "   << it_pe->second
            << " Time: " << it_time->second
	    << G4endl;
  }
}



// *******************************************
// DERIVED CLASS
// *******************************************

WCSimWCTriggerNDigits::WCSimWCTriggerNDigits(G4String name,
					 WCSimDetectorConstruction* myDetector,
					 WCSimWCDAQMessenger* myMessenger, G4String detectorElement)
  :WCSimWCTriggerBase(name, myDetector, myMessenger, detectorElement)
{
  triggerClassName = "NDigits";
  GetVariables();
}

WCSimWCTriggerNDigits::~WCSimWCTriggerNDigits()
{
}

void WCSimWCTriggerNDigits::DoTheWork(WCSimWCDigitsCollection* WCDCPMT) {
  
  bool remove_hits = false;
  WCSimWCDigitsCollection* WCDCPMTCopy = WCDCPMT;
  
  if(enablePromptTrigger){
    //Make a copy of the input DigitsCollection, so we can remove hits from the copy
    WCDCPMTCopy = new WCSimWCDigitsCollection(*WCDCPMT);
    remove_hits = true;
    
    //Apply the prompt trigger
    AlgPromptDigits(WCDCPMTCopy, remove_hits);
  }
  
  //Apply the NDigits trigger
  AlgNDigits(WCDCPMTCopy, remove_hits);
}


// *******************************************
// DERIVED CLASS
// *******************************************


WCSimWCTriggerNoTrigger::WCSimWCTriggerNoTrigger(G4String name,
					 WCSimDetectorConstruction* myDetector,
					 WCSimWCDAQMessenger* myMessenger, G4String detectorElement)
  :WCSimWCTriggerBase(name, myDetector, myMessenger, detectorElement)
{
  triggerClassName = "NoTrigger";
}

WCSimWCTriggerNoTrigger::~WCSimWCTriggerNoTrigger()
{
}

void WCSimWCTriggerNoTrigger::DoTheWork(WCSimWCDigitsCollection* WCDCPMT) {
  //Apply an NDigits trigger
  bool remove_hits = false;
  SetMultiDigitsPerTrigger(true);
  SetSaveFailuresMode(0);
  AlgNoTrigger(WCDCPMT, remove_hits);
}


// *******************************************
// DERIVED CLASS
// *******************************************

WCSimWCTriggerNDigits2::WCSimWCTriggerNDigits2(G4String name,
					 WCSimDetectorConstruction* myDetector,
					 WCSimWCDAQMessenger* myMessenger, G4String detectorElement)
  :WCSimWCTriggerBase(name, myDetector, myMessenger, detectorElement)
{
  triggerClassName = "NDigits2";
  GetVariables();
}

WCSimWCTriggerNDigits2::~WCSimWCTriggerNDigits2(){
}


void WCSimWCTriggerNDigits2::DoTheWork(WCSimWCDigitsCollection* WCDCPMT) {
  //This calls 2 trigger algorithms; the second algorithm is called on hits that failed the first algorithm
  //(for a second trigger working on hits that passed a pretrigger, FillDigitsCollection() needs to have a new option)

  //Make a copy of the input DigitsCollection, so we can remove hits from the copy
  WCSimWCDigitsCollection* WCDCPMTCopy = new WCSimWCDigitsCollection(*WCDCPMT);
  
  bool remove_hits = true;
  
  if(enablePromptTrigger){
    //Apply the prompt trigger first
    AlgPromptDigits(WCDCPMTCopy, remove_hits);
  }
  
  //Apply the first NDigits trigger
  AlgNDigits(WCDCPMTCopy, remove_hits);

  //Apply an NDigits trigger with a lower threshold & different saved trigger type
  remove_hits = false;
  bool ndigits_test = true;
  AlgNDigits(WCDCPMTCopy, remove_hits, ndigits_test);
}

// *******************************************
// DERIVED CLASS
// *******************************************

WCSimWCTriggerOnTankDigits::WCSimWCTriggerOnTankDigits(G4String name,
					 WCSimDetectorConstruction* myDetector,
					 WCSimWCDAQMessenger* myMessenger, G4String detectorElement)
  :WCSimWCTriggerBase(name, myDetector, myMessenger, detectorElement)
{
  triggerClassName = "TankDigits";
  GetVariables(); // need to get the pre- and post-triggerwindow sizes 
}

WCSimWCTriggerOnTankDigits::~WCSimWCTriggerOnTankDigits()
{
}

void WCSimWCTriggerOnTankDigits::DoTheWork(WCSimWCDigitsCollection* WCDCPMT) {
#ifdef HYPER_VERBOSITY
  if(detectorElement=="mrd"){G4cout<<"WCSimWCTriggerOnTankDigits::DoTheWork ☆ calling AlgTankDigits"<<G4endl;}
#endif
  
  bool remove_hits = false;
  WCSimWCDigitsCollection* WCDCPMTCopy = WCDCPMT;
  
  if(enablePromptTrigger){
    //Make a copy of the input DigitsCollection, so we can remove hits from the copy
    WCDCPMTCopy = new WCSimWCDigitsCollection(*WCDCPMT);
    remove_hits = true;
    
    //Apply the prompt trigger
    AlgPromptDigits(WCDCPMTCopy, remove_hits);
  }
  
  remove_hits = false;
  //Nab the triggered digits from the tank and record all hits that fall within those time windows
  AlgTankDigits(WCDCPMTCopy, remove_hits);
}

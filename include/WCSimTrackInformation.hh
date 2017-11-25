#ifndef WCSimTrackInformation_h
#define WCSimTrackInformation_h 1


#include "globals.hh"
#include "G4Track.hh"
#include "G4Allocator.hh"
#include "G4VUserTrackInformation.hh"

// Maximilien Fechner, december 2004
// Information class for flagging the secondaries
// I'm interested in (namely gammas from pi0s and secondaries
// from muon decay
class WCSimTrackInformation : public G4VUserTrackInformation {
private:
  G4bool saveit; 
  G4int  primaryParentID;
  long long int numreflections;

public:
  WCSimTrackInformation() : saveit(false), primaryParentID(-1) {}
  WCSimTrackInformation(const WCSimTrackInformation* aninfo) 
  { saveit = aninfo->saveit; primaryParentID = aninfo->primaryParentID;}
  virtual ~WCSimTrackInformation() {}
  WCSimTrackInformation(const G4Track* );
  
  G4bool isSaved() { return saveit;}
  void WillBeSaved(G4bool choice) { saveit = choice;}

  void SetPrimaryParentID(G4int i) { primaryParentID = i;}
  G4int GetPrimaryParentID() {return primaryParentID;}
  
  void IncrementNumReflections() {numreflections++;}
  long long int GetNumReflections() {return numreflections;}
  
  static long long int numscatters;
  static void IncrementScatterings() {numscatters++;}
  static long long int GetNumScatterings() { return numscatters;} 
  static std::map<std::string,int> scatterings;
  static void AddProcess(std::string processname){
    if(scatterings.count(processname)==0) scatterings.emplace(processname,1);
    else scatterings.at(processname)++;
  }
  static std::map<std::string,int> GetScatterings(){ return scatterings; }

  inline void *operator new(size_t);
  inline void operator delete(void *aTrackInfo);
  inline int operator ==(const WCSimTrackInformation& right) const
  {return (this==&right);}

  void Print() const;

};

extern G4Allocator<WCSimTrackInformation> aWCSimTrackInfoAllocator;

inline void* WCSimTrackInformation::operator new(size_t)
{ void* aTrackInfo;
 aTrackInfo = (void*)aWCSimTrackInfoAllocator.MallocSingle();
 return aTrackInfo;
}

inline void WCSimTrackInformation::operator delete(void *aTrackInfo)
{ aWCSimTrackInfoAllocator.FreeSingle((WCSimTrackInformation*)aTrackInfo);}


#endif

#include "WCSimTrackInformation.hh"
#include "G4ios.hh"

G4Allocator<WCSimTrackInformation> aWCSimTrackInfoAllocator;

WCSimTrackInformation::WCSimTrackInformation(const G4Track* /*atrack*/)
{
  saveit = true;
  numreflections=0;
}
long long int WCSimTrackInformation::numscatters = 0;
std::map<std::string,int> WCSimTrackInformation::scatterings = std::map<std::string,int>{};

void WCSimTrackInformation::Print() const
{
  G4cout << "WCSimTrackInformation : " << saveit << G4endl;
}

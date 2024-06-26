#ifndef Run_h
#define Run_h 1

#include "G4Run.hh"
#include "G4VProcess.hh"
#include "globals.hh"
#include <map>
#include <sys/types.h>
#include <unistd.h>

class DetectorConstruction;
class G4ParticleDefinition;


class Run : public G4Run
{
  public:
    Run(DetectorConstruction*);
   ~Run();

  public:
    void SetPrimary(G4ParticleDefinition* particle, G4double energy);         
    void CountProcesses(const G4VProcess* process);
    void ParticleCount(G4String, G4double, G4double); 
    void AddEdep (G4double edep);
    void AddEflow (G4double eflow);                   
    void ParticleFlux(G4String, G4double);

    G4int GetIonId (G4String);

    virtual void Merge(const G4Run*);
    void EndOfRun();     
   
  private:
    struct ParticleData {
     ParticleData()
       : fCount(0), fEmean(0.), fEmin(0.), fEmax(0.), fTmean(-1.) {}
     ParticleData(G4int count, G4double ekin, G4double emin, G4double emax,
                  G4double meanLife)
       : fCount(count), fEmean(ekin), fEmin(emin), fEmax(emax),
         fTmean(meanLife) {}
     G4int     fCount;
     G4double  fEmean;
     G4double  fEmin;
     G4double  fEmax;
     G4double  fTmean;
    };
     
  private:
    // utility function
    void Merge(std::map<G4String,ParticleData>& destinationMap,
               const std::map<G4String,ParticleData>& sourceMap) const;

    static std::map<G4String,G4int> fgIonMap;
    static G4int fgIonId;

    DetectorConstruction* fDetector;
    G4ParticleDefinition* fParticle;
    G4double              fEkin;

    G4double fEnergyDeposit, fEnergyDeposit2;
    G4double fEnergyFlow,    fEnergyFlow2;            
    std::map<G4String,G4int>        fProcCounter;
    std::map<G4String,ParticleData> fParticleDataMap1;                    
    std::map<G4String,ParticleData> fParticleDataMap2;
};


#endif


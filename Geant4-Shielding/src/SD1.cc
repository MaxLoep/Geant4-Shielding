#include "SensitiveDetector.hh"
#include "Analysis.hh"

#include "G4VTouchable.hh"
#include "G4Step.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"
#include "G4ParticleTypes.hh"

#include <filesystem>
namespace fs = std::filesystem;

// get folderName from where it is defined (RunAction.cc) - the really dirty way 
extern std::string folderName;
// get folderName from where it is defined (Run.cc) - the really dirty way
extern std::string ListFolder;

// Create a map of strings to integers
std::map<std::string, int> SD1map;

// initialize particleCounter outside of anything so this will work with multithreading
// G4int particle_counterSD1 = 0;

//Get process Main ID
std::thread::id main_idSD1 = std::this_thread::get_id();

SD1::SD1(const G4String& name)
 : G4VSensitiveDetector(name)
{
  // initialize oldTrackID with 0, which will never be a trackID - primaries have TrackID=1, secondaries have TrackID>1
  oldTrackId = 0;
}


SD1::~SD1()
{
  // Get thread process ID
  thread_id = std::this_thread::get_id();

  // If returned to main thread (after closing all threads created by multithreading) print secondary counter
  if (main_idSD1 == thread_id)
  {
    // Get an iterator pointing to the first element in the map
    std::map<std::string, int>::iterator it_console = SD1map.begin();
    std::map<std::string, int>::iterator it_file = SD1map.begin();

    // Iterate through the map and print the elements in console
    G4cout << "PARTICLE COUNT OF SD1" << G4endl;
    while (it_console != SD1map.end())
    {
      G4cout << "  " << std::setw(15) << it_console->first << ": " << std::setw(10) << it_console->second << G4endl;
      ++it_console;
    }

    //List of generated particles to file

    // create a folder for the files if it does not exists
    fs::create_directory(folderName);
    fs::create_directory(folderName + "/" + ListFolder);

    //Get main process ID
    G4long pid = getpid();

    // Check if "pid_ListOfGeneratedParticles in SDX.txt" is already existing; if yes, check if "pid+1_ListOfGeneratedParticles in SDX.txt" exists. 
    while(std::ifstream(folderName + "/" + ListFolder + "/" + std::to_string(pid) + "_ListOfGeneratedParticles in SD1" + ".txt"))
    {
      pid++;
    }
    // Set final file name 
    std::string fileName = std::to_string(pid) + "_ListOfGeneratedParticles in SD1" + ".txt";

    // flush output to file
    std::ofstream outFile(folderName + "/" + ListFolder + "/" + fileName);

    // Iterate through the map and print the elements in file
    outFile <<  "PARTICLE COUNT OF SD1" << G4endl;
    while (it_file != SD1map.end())
    {
      outFile << "  " << std::setw(15) << it_file->first << ": " << std::setw(10) << it_file->second << G4endl;
      ++it_file;
    }
  }
}


void SD1::Initialize(G4HCofThisEvent* /*hce*/)
{}

G4bool SD1::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/)
{
  // Get the Current trackID and the name of the particle
  const G4Track* track = step->GetTrack();
  currentTrackId = track->GetTrackID();
  G4String name   = track->GetDefinition()->GetParticleName();
  // G4double HalfLife = track->GetDefinition()->GetPDGLifeTime() / 1.443; // mean life time divided by 1.443 equals half-life

  // if particle is a secondary (trackID>1) and we have not counted it yet add it to the map
  if ( (currentTrackId > 1) && (currentTrackId != oldTrackId) )
  {
    //  particle_counterSD1++;
     SD1map[name] = SD1map[name]+1;
  }

  // overwrite oldTrackID with currentTrackID
  oldTrackId = currentTrackId;


  // keep only outgoing particle
  const G4ParticleDefinition* particle = track->GetParticleDefinition();
 
  // code PDG:
  // G4int pdgCode = track->GetDefinition()->GetPDGEncoding();

  // Remember preStepPoint:
  G4StepPoint* preStepPoint = step->GetPreStepPoint();

  // Ekin:
  G4double Ekin =  preStepPoint->GetKineticEnergy();

  // Obtain local coordinates:
  const G4VTouchable* touchable = preStepPoint->GetTouchable();
  G4ThreeVector globalPosition = preStepPoint->GetPosition();
  G4ThreeVector localPosition
    = touchable->GetHistory()->GetTopTransform().TransformPoint(globalPosition);
  // // Example for obtaining the local direction:
  // G4ThreeVector globalDirection = preStepPoint->GetMomentumDirection();
  // G4ThreeVector localDirection
  //   = touchable->GetHistory()->GetTopTransform().TransformAxis(localDirection);

  // Time
  G4double time = preStepPoint->GetGlobalTime();

  // Get Analysis Manager
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // ntuples and histograms are set up in RunAction.cc
  // Store neutron hit in the ntuple
  if(particle == G4Neutron::Neutron())  analysisManager->FillNtupleDColumn(1, 0, Ekin/MeV);
  if(particle == G4Neutron::Neutron())  analysisManager->FillNtupleDColumn(1, 1, localPosition.x()/cm);
  if(particle == G4Neutron::Neutron())  analysisManager->FillNtupleDColumn(1, 2, localPosition.y()/cm);
  if(particle == G4Neutron::Neutron())  analysisManager->FillNtupleDColumn(1, 3, time/ns);
  if(particle == G4Neutron::Neutron())  analysisManager->AddNtupleRow(1);

  // Store gamma hit in the ntuple
  if(particle == G4Gamma::Gamma())  analysisManager->FillNtupleDColumn(2, 0, Ekin/MeV);
  if(particle == G4Gamma::Gamma())  analysisManager->FillNtupleDColumn(2, 1, localPosition.x()/cm);
  if(particle == G4Gamma::Gamma())  analysisManager->FillNtupleDColumn(2, 2, localPosition.y()/cm);
  if(particle == G4Gamma::Gamma())  analysisManager->FillNtupleDColumn(2, 3, time/ns);
  if(particle == G4Gamma::Gamma())  analysisManager->AddNtupleRow(2);

  return true;
}


void SD1::EndOfEvent(G4HCofThisEvent* /*hce*/)
{
  // reset oldTrackID for the next event
  oldTrackId = 0;
}
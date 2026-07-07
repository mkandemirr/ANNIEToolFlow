#include <iostream>
#include "ANNIEEventReader.h"
#include "LAPPDMetaReader.h"
#include "RootTreeReader.h"
#include "INIReader.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void filterQualityEvent(const INIReader& iniReader);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char* argv[]) 
{
   INIReader iniReader(argv[1]);
   filterQualityEvent(iniReader);
   return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void filterQualityEvent(const INIReader& iniReader)
{
 
   std::string rootFile      = iniReader.Get("input", "rootFile", "");
   const bool useMC          = iniReader.GetBoolean("input", "useMC", false);

   const bool cutBeamOK      = (!useMC) && iniReader.GetBoolean("cuts", "beamOK", false);
   const bool cutBRFWindow   = (!useMC) && iniReader.GetBoolean("cuts", "brfWindow", false);
   const bool cutPPSMissing  = (!useMC) && iniReader.GetBoolean("cuts", "ppsMissing", false);
   const bool cutPairedEvent = iniReader.GetBoolean("cuts", "pairedEvent", false);
   const bool cutHasSelectedLAPPD = iniReader.GetBoolean("cuts", "hasSelectedLAPPD", false);

   const int selectedLAPPDID = iniReader.GetInteger("cutValues", "selectedLAPPDID", -1);
   const double brfMin = iniReader.GetReal("cutValues", "brfMin", 5.0);
   const double brfMax = iniReader.GetReal("cutValues", "brfMax", 30.0);
   
   std::cout << "\n===== CUT CONFIGURATION =====\n";

   std::cout << "useMC          : " << useMC << "\n";

   std::cout << "cutBeamOK      : " << cutBeamOK << "\n";
   std::cout << "cutBRFWindow   : " << cutBRFWindow << "\n";
   std::cout << "cutPPSMissing  : " << cutPPSMissing << "\n";
   std::cout << "cutPairedEvent : " << cutPairedEvent << "\n";
   std::cout << "cutHasSelectedLAPPD : " << cutHasSelectedLAPPD << "\n";
   

   std::cout << "selectedLAPPDID     : " << selectedLAPPDID << "\n";
   std::cout << "brfMin         : " << brfMin << "\n";
   std::cout << "brfMax         : " << brfMax << "\n";

   std::cout << "==============================\n\n";


   const std::string& treeName = "Event";
   RootTreeReader treeReader(rootFile, treeName);
   
   if(!treeReader.IsValid())
      return;
   
   
   // Output file and tree
   std::string oRootFile = iniReader.Get("output", "rootFile","");
   TFile* outputFile = new TFile(oRootFile.c_str(), "RECREATE");
   if (!outputFile || outputFile->IsZombie()) {
      std::cerr << "Could not create output file: " << oRootFile << std::endl;
      return;
   }

   // Clone input tree structure, but with 0 entries
   TTree* tree = treeReader.GetTree(); 
   TTree* outputTree = tree->CloneTree(0);
   outputTree->SetName(treeName.c_str());
   outputTree->SetTitle("Filtered Event Tree");
   
   Long64_t nEntries = tree->GetEntries();
   std::cout << "Number of events in this tree: " << nEntries << std::endl;

   ANNIEEventReader eventReader;
   eventReader.Init(tree);
   
   LAPPDMetaReader metaReader;

   if (!useMC) {
      metaReader.Init(tree);
   }

   Long64_t nPassedEvents = 0;
   
   int beam_ok = 1;
   double BRFFirstPeakFit = 0.0;

   if (!useMC) {
      tree->SetBranchAddress("beam_ok", &beam_ok);
      tree->SetBranchAddress("BRFFirstPeakFit", &BRFFirstPeakFit);
   }
   

   // Loop over entries and fill
   for (Long64_t i = 0; i < nEntries; ++i) 
   {
      
      tree->GetEntry(i);

      bool keepEvent = true;

      // Data-only cut: beam OK
      if (cutBeamOK && !beam_ok ) {
         keepEvent = false;
      }

      // Data-only cut: BRF window
      if (!useMC && cutBRFWindow) {
         double brf_us = BRFFirstPeakFit / 1000.0;

         if (brf_us < brfMin || brf_us > brfMax) {
            keepEvent = false;
         }
      }

      // Data-only cut: PPS missing
      if (!useMC && cutPPSMissing) {

         bool hasPPSMissing = false;

         for (size_t j = 0; j < metaReader.mLAPPD_TSPPSMissing->size(); ++j) {
            if (metaReader.mLAPPD_TSPPSMissing->at(j) != 0) {
               hasPPSMissing = true;
               break;
            }
         }

         if (hasPPSMissing) {
            keepEvent = false;
         }
      }

      // Paired cut
      bool isPairedEvent =
         (eventReader.HasLAPPD() == 1 &&
          eventReader.HasMRD()   == 1 &&
          eventReader.HasTank()  == 1);

      if (cutPairedEvent && !isPairedEvent) {
         keepEvent = false;
      }
      
      // Data-only cut: Selected LAPPD cut
      if (!useMC && cutHasSelectedLAPPD) {

         auto ite = std::find(metaReader.mLAPPD_ID->begin(),
                              metaReader.mLAPPD_ID->end(),
                              selectedLAPPDID);

         if (ite == metaReader.mLAPPD_ID->end()) {
            keepEvent = false;
         }
      }

      //final
      if (keepEvent) {
         outputTree->Fill();
         ++nPassedEvents;
      }
    
   }//event loop end
   
   outputFile->cd();
   outputTree->Write();
   outputFile->Close();

   std::cout << "Number of events passing all cuts: " << nPassedEvents << std::endl;
   std::cout << "Filtered tree written to: " << oRootFile << std::endl;

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......




#include <iomanip>
#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <TFile.h>
#include <TTree.h>

#include "MCANNIEEventReader.h"
#include "ANNIEEventReader.h"
#include "MCLAPPDHitReader.h"
#include "LAPPDMetaReader.h"
#include "TankClusterReader.h"
#include "MRDRecoReader.h"
#include "MRDClusterReader.h"
#include "RootTreeReader.h"
#include "INIReader.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//This function is for mrdTrackReconstructed cut
void ReadChannels(const std::string& filename,
                  std::vector<int>& verticalChannels,
                  std::vector<int>& horizontalChannels);
 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//This function is for muon topology cut
void ReadLAPPDNeighborPMTIDs(const std::string& pmtListStr,
                            std::vector<int>& lappdNeighborPMTIDs);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......                
void FilterCCEvent(const INIReader& iniReader);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char* argv[]) 
{
   if (argc < 2) {
      std::cerr << "Usage: ./filterCCEvent <config.ini>" << std::endl;
      return 1;
   }

   INIReader iniReader(argv[1]);

   if (iniReader.ParseError() != 0) {
      std::cerr << "Could not load config file: " << argv[1] << std::endl;
      return 1;
   }

   FilterCCEvent(iniReader);

   return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void FilterCCEvent(const INIReader& iniReader)
{
   std::string inputFileName  = iniReader.Get("input",  "rootFile", "");
   std::string mrdInfoFile    = iniReader.Get("input",  "mrdFile", "MRDInfo.txt");
   std::string outputFileName = iniReader.Get("output", "rootFile", "");
   const bool useMC           = iniReader.GetBoolean("input", "useMC", false);
   
   const std::string writeMode = iniReader.Get("output", "writeMode", "filtered");
   const bool writeAnnotated = (writeMode == "annotated");
   
   //data only
   const bool cutBeamOK                = (!useMC) && iniReader.GetBoolean("cuts", "beamOK", false);
   const bool cutBRFWindow             = (!useMC) && iniReader.GetBoolean("cuts", "brfWindow", false);
   const bool cutMissingPPSTick        = (!useMC) && iniReader.GetBoolean("cuts", "missingPPSTick", false);

   const double    ppsIntervalSec      = iniReader.GetReal("cuts", "ppsIntervalSec", 10);
   
   //for mc only
   const bool cutTrueCC = useMC && iniReader.GetBoolean("cuts", "trueCC", false);
   
   //for both mc and data
   const bool cutRequireLAPPD          = iniReader.GetBoolean("cuts", "requireLAPPD", false);
   const bool cutPairedEvent           = iniReader.GetBoolean("cuts", "pairedEvent", false);
   const bool cutPromptPMTCluster      = iniReader.GetBoolean("cuts", "promptPMTCluster", false);
   const bool cutHighQualityPMTCluster = iniReader.GetBoolean("cuts", "highQualityPMTCluster", false);
   const bool cutMRDTrackReconstructed = iniReader.GetBoolean("cuts", "mrdTrackReconstructed", false); //manuel check
   const bool cutMRDRecoTrack          = iniReader.GetBoolean("cuts", "mrdRecoTrack", false); // check data in tree
   const bool cutTankMRDCoinc          = iniReader.GetBoolean("cuts", "tankMRDCoinc", false);
   const bool cutMuonTopology          = iniReader.GetBoolean("cuts", "muonTopology", false);
   const bool cutNoVeto                = iniReader.GetBoolean("cuts", "noVeto", false);

   const double promptWindowMax        = iniReader.GetReal("cutValues", "promptWindowMax", 2000.0);
   const double chargeBalanceMax       = iniReader.GetReal("cutValues", "chargeBalanceMax", 0.2);
   const double mrdTimeWindow          = iniReader.GetReal("cutValues", "mrdTimeWindow", 30.0);
   const int    mrdMinSideHits         = iniReader.GetInteger("cutValues", "mrdMinSideHits", 2);
   const int    mrdMinTopHits          = iniReader.GetInteger("cutValues", "mrdMinTopHits", 2);
   const double brfMin                 = iniReader.GetReal("cutValues", "brfMin", 5.0);
   const double brfMax                 = iniReader.GetReal("cutValues", "brfMax", 30.0);
   const double lappdNeighborPMTPEThreshold = iniReader.GetReal("cutValues", "lappdNeighborPMTPEThreshold", 5.0);
   std::string pmtListStr              = iniReader.Get("cutValues", "lappdNeighborPMTIDs", "");

   if (inputFileName.empty()) {
      std::cerr << "Error: input.rootFile is empty in config." << std::endl;
      return;
   }

   if (outputFileName.empty()) {
      std::cerr << "Error: output.rootFile is empty in config." << std::endl;
      return;
   }

   const std::string& treeName = "Event";
   RootTreeReader treeReader(inputFileName.c_str(), treeName.c_str());
   
   if(!treeReader.IsValid())
      return;
   
   TTree* tree = treeReader.GetTree(); 
   
   // Output file 
   TFile* outputFile = new TFile(outputFileName.c_str(), "RECREATE");

   if (!outputFile || outputFile->IsZombie()) {
      std::cerr << "Could not create output file: " << outputFileName << std::endl;
      return;
   }
   

   // Clone input tree structure, but with 0 entries
   TTree* outputTree = tree->CloneTree(0);
   outputTree->SetName("Event");
   outputTree->SetTitle("Filtered Event Tree");
   
   int passTrueCC                = -1;
   int passBeamOK                = -1;
   int passBRFWindow             = -1;
   int passMissingPPSTick        = -1;
   int passRequireLAPPD          = -1;
   int passPairedEvent           = -1;
   int passPromptPMTCluster      = -1;
   int passHighQualityPMTCluster = -1;
   int passMRDTrackReconstructed = -1;
   int passMRDRecoTrack          = -1;
   int passTankMRDCoinc          = -1;
   int passMuonTopology          = -1;
   int passNoVeto                = -1;

   if (writeAnnotated) {
      outputTree->Branch("passTrueCC",                &passTrueCC,                "passTrueCC/I");
      outputTree->Branch("passBeamOK",                &passBeamOK,                "passBeamOK/I");
      outputTree->Branch("passBRFWindow",             &passBRFWindow,             "passBRFWindow/I");
      outputTree->Branch("passMissingPPSTick",        &passMissingPPSTick,        "passMissingPPSTick/I");
      outputTree->Branch("passRequireLAPPD",          &passRequireLAPPD,          "passRequireLAPPD/I");
      outputTree->Branch("passPairedEvent",           &passPairedEvent,           "passPairedEvent/I");
      outputTree->Branch("passPromptPMTCluster",      &passPromptPMTCluster,      "passPromptPMTCluster/I");
      outputTree->Branch("passHighQualityPMTCluster", &passHighQualityPMTCluster, "passHighQualityPMTCluster/I");
      outputTree->Branch("passMRDTrackReconstructed", &passMRDTrackReconstructed, "passMRDTrackReconstructed/I");
      outputTree->Branch("passMRDRecoTrack",          &passMRDRecoTrack,          "passMRDRecoTrack/I");
      outputTree->Branch("passTankMRDCoinc",          &passTankMRDCoinc,          "passTankMRDCoinc/I");
      outputTree->Branch("passMuonTopology",          &passMuonTopology,          "passMuonTopology/I");
      outputTree->Branch("passNoVeto",                &passNoVeto,                "passNoVeto/I");
   }
   
   Long64_t nEntries = tree->GetEntries();
   std::cout << "Number of events in this tree: " << nEntries << std::endl;

   Long64_t nPassedEvents = 0;
   
   
   int beam_ok = 1;
   double BRFFirstPeakFit = 0.0;

   if (!useMC) {
      tree->SetBranchAddress("beam_ok", &beam_ok);
      tree->SetBranchAddress("BRFFirstPeakFit", &BRFFirstPeakFit);
   }


   // Initialize readers
   MCLAPPDHitReader mcLAPPDHitReader;
   LAPPDMetaReader metaReader;    
   
   if(useMC)
      mcLAPPDHitReader.Init(tree);
   else
      metaReader.Init(tree);
 
 
   MCANNIEEventReader mcEventReader;
   if (useMC)
      mcEventReader.Init(tree);
 
   ANNIEEventReader eventReader; 
   eventReader.Init(tree);

   TankClusterReader tankClusterReader;
   tankClusterReader.Init(tree);
   
   MRDClusterReader mrdClusterReader;
   mrdClusterReader.Init(tree);
   
   MRDRecoReader mrdRecoReader;
   mrdRecoReader.Init(tree);


   //For MRDtrackReconstructed cut
   std::vector<int> sideMRDPMTs;
   std::vector<int> topMRDPMTs;
   ReadChannels(mrdInfoFile, sideMRDPMTs, topMRDPMTs);
   //std::cout << "Vertical size: " << sideMRDPMTs.size() << std::endl;
   //std::cout << "Horizontal size: " << topMRDPMTs.size() << std::endl;
   
   //For muon toplogy cut
   std::vector<int> lappdNeighborPMTIDs;
   ReadLAPPDNeighborPMTIDs(pmtListStr, lappdNeighborPMTIDs);

   
   for (Long64_t i = 0; i < nEntries; ++i) 
   {
      //std::cout<<"EVENT "<<i<<std::endl;
      
      tree->GetEntry(i);
      
      bool requireLAPPD = true;
      bool beamOK       = true;
      bool brfWindow    = true;
      bool noMissingPPSTick = true;
      
      // ------------------------------------------------------------
      // 1) Require LAPPD both for MC and data
      // ------------------------------------------------------------
      if (cutRequireLAPPD) {

         requireLAPPD = false;

         if (useMC) {
            EMCLAPPDHits eMCLAPPDHits;
            mcLAPPDHitReader.ReadEntry(i, eMCLAPPDHits);

            if (!eMCLAPPDHits.GetHits().empty())
               requireLAPPD = true;
         }
         else {
            if (!metaReader.mLAPPD_ID->empty())
               requireLAPPD = true;
         }
      }


      // ------------------------------------------------------------
      // 2) Data-only quality cuts
      // ------------------------------------------------------------
      if (!useMC) {

         // Beam OK
         if (cutBeamOK) {
            beamOK = (beam_ok == 1);
         }

         // BRF window
         if (cutBRFWindow) {
            double brf_us = BRFFirstPeakFit / 1000.0;
            brfWindow = (brf_us >= brfMin && brf_us <= brfMax);
         }

         
         // PPS missing tick.
         // if ANY LAPPD has an unexpected PPS tick difference,
         // the ENTIRE event is rejected. This is a hard cut.fix it later!
         if (cutMissingPPSTick) {

            // 1 tick = 3.125 ns
            const double tick_ns = 3.125;

            // expected tick difference between PPS signals
            const uint64_t expectedPPSDiff =
              static_cast<uint64_t>((ppsIntervalSec * 1e9) / tick_ns);
              
            //std::cout<<"expectedPPSDiff: "<<expectedPPSDiff<<std::endl;

            for (size_t j = 0; j < metaReader.mLAPPD_ID->size(); ++j) {

               uint64_t before = metaReader.mLAPPD_TSPPSBefore->at(j);
               uint64_t after  = metaReader.mLAPPD_TSPPSAfter->at(j);

               uint64_t diff = after - before;

               if (diff != expectedPPSDiff) {
                  noMissingPPSTick = false;
                  break;
               }
            }
         
         }
      
      }
      
      //------------------------------------------------------------------------
      // The belows are Event-level physics cuts
      ETankClusters eTankClusters;
      tankClusterReader.ReadEntry(i, eTankClusters);

      EMRDClusters eMRDClusters;
      mrdClusterReader.ReadEntry(i, eMRDClusters);
      
      EMRDRecos eMRDRecos;
      mrdRecoReader.ReadEntry(i, eMRDRecos);
            
      // Cut 0: Paired
      bool isPairedEvent = false;
      if (eventReader.HasLAPPD() == 1 && 
          eventReader.HasMRD()   == 1   && 
          eventReader.HasTank()  == 1 )
      {
         
         isPairedEvent = true;
      }
         
      
      // Cut 1: Any PMT cluster 
      // Find the cluster with the maximum charge (PE) in the initial time window
      bool hasClusterInPromptWindow = false;
      double maxPENumber = -1.0;

      const TankCluster* clusterPtrWithMaxPE = nullptr;
      for (const TankCluster& cl : eTankClusters.Get() )
      {
         
         if (cl.GetClusterTime() < promptWindowMax) //initial window
         {
          
            hasClusterInPromptWindow = true;

            if (cl.GetMaxPE() > maxPENumber) 
            {
                maxPENumber  = cl.GetMaxPE();
                clusterPtrWithMaxPE = &cl;
            }
            
            //std::cout<<"Max PE in this cluster : "<<cl.GetMaxPE()<<std::endl;
            //for(const PMTHit& hit: cl.GetHits())
            //std::cout<<hit.GetPE()<<std::endl;
                  
         }
           
      }
     
      // Cut 2: High-quality PMT cluster
      // Check if the previously selected cluster is a high-quality PMT cluster
      bool highQualityCluster = false;   
      if (hasClusterInPromptWindow && clusterPtrWithMaxPE )
      {
         
         if (clusterPtrWithMaxPE->GetChargeBalance() < chargeBalanceMax)
            highQualityCluster = true;
      }
         

      // Cut 3: MRD track reconstructed
      bool mrdTrackReconstructed = false;

      for (const MRDCluster& cluster : eMRDClusters.Get()) {

         // Her hit için: (time, type)
         // type: 0 -> side, 1 -> top, -1 -> ilgisiz
         std::vector<std::pair<double, int>> taggedHits;

         for (const MRDHit& hit : cluster.GetHits()) {

            int chankey = hit.GetChankey();
            double time = hit.GetTime();

            if (std::find(sideMRDPMTs.begin(), sideMRDPMTs.end(), chankey) != sideMRDPMTs.end()) {
               taggedHits.emplace_back(time, 0);
            }
            else if (std::find(topMRDPMTs.begin(), topMRDPMTs.end(), chankey) != topMRDPMTs.end()) {
               taggedHits.emplace_back(time, 1);
            }
         }

         if (taggedHits.size() < 4) continue;

         // Zamana göre sırala
         std::sort(taggedHits.begin(), taggedHits.end(),
                   [](const auto& a, const auto& b) {
                      return a.first < b.first;
                   });

         // Sliding window
         for (size_t i = 0; i < taggedHits.size(); ++i) {

            int nSideHits = 0;
            int nTopHits  = 0;

            for (size_t j = i; j < taggedHits.size(); ++j) {

               double dt = taggedHits[j].first - taggedHits[i].first;

               if (dt > mrdTimeWindow) break;

               if (taggedHits[j].second == 0) ++nSideHits;
               else if (taggedHits[j].second == 1) ++nTopHits;

               if (nSideHits >= mrdMinSideHits && nTopHits >= mrdMinTopHits) {
                  mrdTrackReconstructed = true;
                  break;
               }
            }

            if (mrdTrackReconstructed) break;
         }

         if (mrdTrackReconstructed) break;  // bir cluster yeterli
      }
      
      
      
      // Cut 4: MRDReco check

      bool mrdRecoTrack = false;

      if (!eMRDRecos.Get().empty())
      {
          mrdRecoTrack = true;
      }
      
      
      
      // Cut 5: In-time MRD coincidence
      bool tankMRDCoinc = false;
      if (eventReader.GetTankMRDCoinc() == 1)
         tankMRDCoinc = true;
      
      
      //Cut 6: Muon topology cut
      bool muonTopology = false;
            
      std::unordered_map<int, bool> passed;
      for (int id : lappdNeighborPMTIDs) 
         passed[id] = false;

      for (const TankCluster& cluster : eTankClusters.Get())
      {
         if (cluster.GetClusterTime() > promptWindowMax) //initial window
            continue;
            
         for (const PMTHit& hit : cluster.GetHits())
         {
             double q = hit.GetPE();
             int id   = hit.GetDetID();

             auto it = passed.find(id);
             if (it != passed.end() && q > lappdNeighborPMTPEThreshold)
             {
               it->second = true;
             }
         }
      }

      bool missing = false;
      for (auto& kv : passed)
      {
        if (!kv.second)
        {
          missing = true;
          break;
        }
      }

      if (!missing) 
         muonTopology = true;


      //Cut 6: No hit in FMV
      bool noVeto = false;
      if (eventReader.NoVeto() == 1)
         noVeto = true;
 
 
      // Cut 7: True CC, MC only
      bool trueCC = false;

      if (cutTrueCC) {
         mcEventReader.ReadEntry(i);
         trueCC = (mcEventReader.GetTrueCC() == 1);
      }
      
      //------------------------------------------------------------------------
 
      bool passedAllCuts = true;
      
      if (cutBeamOK) {
         passedAllCuts = passedAllCuts && beamOK;
         passBeamOK = beamOK;
      }

      if (cutBRFWindow) {
         passedAllCuts = passedAllCuts && brfWindow;
         passBRFWindow = brfWindow;
      }

      if (cutMissingPPSTick) {
         passedAllCuts = passedAllCuts && noMissingPPSTick;
         passMissingPPSTick = noMissingPPSTick;
      }

      if (cutRequireLAPPD) {
         passedAllCuts = passedAllCuts && requireLAPPD;
         passRequireLAPPD = requireLAPPD;
      }

      if (cutPairedEvent) {
         passedAllCuts = passedAllCuts && isPairedEvent;
         passPairedEvent = isPairedEvent;
      }

      if (cutPromptPMTCluster) {
         passedAllCuts = passedAllCuts && hasClusterInPromptWindow;
         passPromptPMTCluster = hasClusterInPromptWindow;
      }

      if (cutHighQualityPMTCluster) {
         passedAllCuts = passedAllCuts && highQualityCluster;
         passHighQualityPMTCluster = highQualityCluster;
      }

      if (cutMRDTrackReconstructed) { 
         passedAllCuts = passedAllCuts && mrdTrackReconstructed;
         passMRDTrackReconstructed = mrdTrackReconstructed;
      }
      
      if (cutMRDRecoTrack) { 
         passedAllCuts = passedAllCuts && mrdRecoTrack;
         passMRDRecoTrack = mrdRecoTrack;
      }

      if (cutTankMRDCoinc) {
         passedAllCuts = passedAllCuts && tankMRDCoinc;
         passTankMRDCoinc = tankMRDCoinc;
      }

      if (cutMuonTopology) {
         passedAllCuts = passedAllCuts && muonTopology;
         passMuonTopology = muonTopology;
      }

      if (cutNoVeto) {
         passedAllCuts = passedAllCuts && noVeto;
         passNoVeto = noVeto;
      }
      
      if (cutTrueCC) {
         passedAllCuts = passedAllCuts && trueCC;
         passTrueCC = trueCC;
      }


      //final
      if (writeAnnotated) {
         outputTree->Fill();
      }
      else {
         if (passedAllCuts) {
            outputTree->Fill();
         }
      }

      if (passedAllCuts) {
         ++nPassedEvents;
      }
    
   }//event loop end
   
  
   std::cout << "Events passing all cuts: " << nPassedEvents << std::endl;
   std::cout << "Output tree entries: " << outputTree->GetEntries() << std::endl;

   if (writeAnnotated) {
      std::cout << "Mode: annotated (all events written with cut branches)" << std::endl;
   }
   else {
      std::cout << "Mode: filtered (only events passing all cuts written)" << std::endl;
   }

   std::cout << "Output file: " << outputFileName << std::endl;
   
   outputFile->cd();
   outputTree->Write();
   outputFile->Close();

   


}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ReadChannels(const std::string& filename,
                  std::vector<int>& verticalChannels,
                  std::vector<int>& horizontalChannels)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "MRD File cannot be opened!" << std::endl;
        return;
    }

    std::string line;

    // header'ı atla
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        int channel_num;
        int detector_system;
        int orientation;

        // sadece ilk 3 column'u oku
        ss >> channel_num >> detector_system >> orientation;

        if (orientation == 0) {
            verticalChannels.push_back(channel_num);
        }
        else if (orientation == 1) {
            horizontalChannels.push_back(channel_num);
        }
    }

    file.close();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ReadLAPPDNeighborPMTIDs(const std::string& pmtListStr,
                std::vector<int>& lappdNeighborPMTIDs)
{
    std::stringstream ss(pmtListStr);
    std::string item;

    while (std::getline(ss, item, ',')) {
        lappdNeighborPMTIDs.push_back(std::stoi(item));
    }
    
   //for debug
   //for (int id : lappdNeighborPMTIDs)
   //   std::cout << "lappd neigbor id: "<<id << std::endl;
}


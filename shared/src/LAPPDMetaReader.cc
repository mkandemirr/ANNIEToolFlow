#include "LAPPDMetaReader.h"

void LAPPDMetaReader::Init(TTree* tree) {
    // Notes: Set branch addresses to read meta information from the input tree
    tree->SetBranchAddress("LAPPD_Count", &mLAPPD_Count);
    tree->SetBranchAddress("LAPPD_ID", &mLAPPD_ID);
    tree->SetBranchAddress("LAPPD_Beamgate_ns", &mLAPPD_Beamgate_ns);
    tree->SetBranchAddress("LAPPD_Timestamp_ns", &mLAPPD_Timestamp_ns);
    tree->SetBranchAddress("LAPPD_Beamgate_Raw", &mLAPPD_Beamgate_Raw);
    tree->SetBranchAddress("LAPPD_Timestamp_Raw", &mLAPPD_Timestamp_Raw);
    tree->SetBranchAddress("LAPPD_Offset", &mLAPPD_Offset);
    tree->SetBranchAddress("LAPPD_TSCorrection", &mLAPPD_TSCorrection);
    tree->SetBranchAddress("LAPPD_BGCorrection", &mLAPPD_BGCorrection);
    tree->SetBranchAddress("LAPPD_OSInMinusPS", &mLAPPD_OSInMinusPS);

    tree->SetBranchAddress("LAPPD_BGPPSBefore", &mLAPPD_BGPPSBefore);
    tree->SetBranchAddress("LAPPD_BGPPSAfter", &mLAPPD_BGPPSAfter);
    tree->SetBranchAddress("LAPPD_BGPPSDiff", &mLAPPD_BGPPSDiff);
    tree->SetBranchAddress("LAPPD_BGPPSMissing", &mLAPPD_BGPPSMissing);
    tree->SetBranchAddress("LAPPD_TSPPSBefore", &mLAPPD_TSPPSBefore);
    tree->SetBranchAddress("LAPPD_TSPPSAfter", &mLAPPD_TSPPSAfter);
    tree->SetBranchAddress("LAPPD_TSPPSDiff", &mLAPPD_TSPPSDiff);
    tree->SetBranchAddress("LAPPD_TSPPSMissing", &mLAPPD_TSPPSMissing);

    tree->SetBranchAddress("LAPPD_BG_switchBit0", &mLAPPD_BG_switchBit0);
    tree->SetBranchAddress("LAPPD_BG_switchBit1", &mLAPPD_BG_switchBit1);
}

void LAPPDMetaReader::ReadEntry(Long64_t entry) {
    // Notes: Load the specified entry to access LAPPD meta data.
    // This assumes that Init() has already been called and branches are correctly set.
    if (entry < 0) {
        std::cerr << "[LAPPDMetaReader] Invalid entry index: " << entry << std::endl;
        return;
    }

    // This call is typically managed externally (e.g., TTree::GetEntry),
    // so you may want to call it before this method.
    // Keeping this empty unless explicit access is needed.
}



/*
void LAPPDMetaReader::Print() const {
    // Notes: Print all LAPPD metadata variables (one by one)
    std::cout << "=== LAPPD Metadata ===\n";
    
    std::cout << "LAPPD_Count: " << mLAPPD_Count << "\n";

    auto printVec = [](const std::string& name, const auto* vec) {
        if (vec) {
            std::cout << name << ": ";
            for (const auto& val : *vec) std::cout << val << " ";
            std::cout << "\n";
        } else {
            std::cout << name << ": nullptr\n";
        }
    };

    printVec("LAPPD_ID", mLAPPD_ID);
    printVec("LAPPD_Beamgate_ns", mLAPPD_Beamgate_ns);
    printVec("LAPPD_Timestamp_ns", mLAPPD_Timestamp_ns);
    printVec("LAPPD_Beamgate_Raw", mLAPPD_Beamgate_Raw);
    printVec("LAPPD_Timestamp_Raw", mLAPPD_Timestamp_Raw);
    printVec("LAPPD_Offset", mLAPPD_Offset);
    printVec("LAPPD_TSCorrection", mLAPPD_TSCorrection);
    printVec("LAPPD_BGCorrection", mLAPPD_BGCorrection);
    printVec("LAPPD_OSInMinusPS", mLAPPD_OSInMinusPS);

    printVec("LAPPD_BGPPSBefore", mLAPPD_BGPPSBefore);
    printVec("LAPPD_BGPPSAfter", mLAPPD_BGPPSAfter);
    printVec("LAPPD_BGPPSDiff", mLAPPD_BGPPSDiff);
    printVec("LAPPD_BGPPSMissing", mLAPPD_BGPPSMissing);

    printVec("LAPPD_TSPPSBefore", mLAPPD_TSPPSBefore);
    printVec("LAPPD_TSPPSAfter", mLAPPD_TSPPSAfter);
    printVec("LAPPD_TSPPSDiff", mLAPPD_TSPPSDiff);
    printVec("LAPPD_TSPPSMissing", mLAPPD_TSPPSMissing);

    printVec("LAPPD_BG_switchBit0", mLAPPD_BG_switchBit0);
    printVec("LAPPD_BG_switchBit1", mLAPPD_BG_switchBit1);
}

*/


void LAPPDMetaReader::Print() const {

    std::cout << "=== LAPPD Metadata ===\n";
    std::cout << "LAPPD_Count: " << mLAPPD_Count << "\n";

    for (size_t i = 0; i < mLAPPD_ID->size(); ++i) {
        std::cout << "---- LAPPD[" << i << "] ----\n";
        std::cout << "LAPPD_ID: " << mLAPPD_ID->at(i) << "\n";
        std::cout << "LAPPD_Beamgate_ns: " << mLAPPD_Beamgate_ns->at(i) << "\n";
        std::cout << "LAPPD_Timestamp_ns: " << mLAPPD_Timestamp_ns->at(i) << "\n"; //I guess this is the LAPPDEvent time. 
        std::cout << "LAPPD_Beamgate_Raw: " << mLAPPD_Beamgate_Raw->at(i) << "\n";
        std::cout << "LAPPD_Timestamp_Raw: " << mLAPPD_Timestamp_Raw->at(i) << "\n";
        std::cout << "LAPPD_Offset: " << mLAPPD_Offset->at(i) << "\n";
        std::cout << "LAPPD_TSCorrection: " << mLAPPD_TSCorrection->at(i) << "\n";
        std::cout << "LAPPD_BGCorrection: " << mLAPPD_BGCorrection->at(i) << "\n";
        std::cout << "LAPPD_OSInMinusPS: " << mLAPPD_OSInMinusPS->at(i) << "\n";

        std::cout << "LAPPD_BGPPSBefore: " << mLAPPD_BGPPSBefore->at(i) << "\n";
        std::cout << "LAPPD_BGPPSAfter: " << mLAPPD_BGPPSAfter->at(i) << "\n";
        std::cout << "LAPPD_BGPPSDiff: " << mLAPPD_BGPPSDiff->at(i) << "\n";
        std::cout << "LAPPD_BGPPSMissing: " << mLAPPD_BGPPSMissing->at(i) << "\n";

        std::cout << "LAPPD_TSPPSBefore: " << mLAPPD_TSPPSBefore->at(i) << "\n";
        std::cout << "LAPPD_TSPPSAfter: " << mLAPPD_TSPPSAfter->at(i) << "\n";
        std::cout << "LAPPD_TSPPSDiff: " << mLAPPD_TSPPSDiff->at(i) << "\n";
        std::cout << "LAPPD_TSPPSMissing: " << mLAPPD_TSPPSMissing->at(i) << "\n";

        std::cout << "LAPPD_BG_switchBit0: " << mLAPPD_BG_switchBit0->at(i) << "\n";
        std::cout << "LAPPD_BG_switchBit1: " << mLAPPD_BG_switchBit1->at(i) << "\n"
        << std::endl;
    }
}


void LAPPDMetaReader::PrintMetaEntrySummary() const {

   //check
   if (mLAPPD_Count != mLAPPD_ID->size() ) 
      throw std::runtime_error("There sould be some problem!");


  
   std::cout << "=== LAPPD Metadata Entry Summary (this event) ===\n";

   if (!mLAPPD_ID) {
     std::cout << "[LAPPDMetaReader] mLAPPD_ID is null!\n\n";
     return;
   }

   const size_t n = mLAPPD_ID->size();
   std::cout << "Total metadata entries in this event: " << n << "\n";
  
   // count per LAPPDID
   std::map<int, int> countPerID;

   for (size_t i = 0; i < n; ++i) {
     int id = mLAPPD_ID->at(i);
     countPerID[id]++;
   }

  
    // print counts
    for (const auto& kv : countPerID) {
        std::cout << "LAPPD ID " << kv.first
                  << " : " << kv.second
                  << " metadata entries\n";
    }
    
    
   std::cout << "Number of different LAPPDs with metadata in this event: "
          << countPerID.size() << "\n";
   
                 
    std::cout << std::endl;
    
}





#include "ANNIEEventReader.h"
#include <iostream>
#include <cassert>  
#include <iomanip>  // std::setw, std::left

ANNIEEventReader::ANNIEEventReader() {}

void ANNIEEventReader::Init(TTree* tree) {
    if (!tree) {
        std::cerr << "Error: Input TTree pointer is null!" << std::endl;
        return;
    }

    auto checkBranch = [&](const char* name, void* address) {
        TBranch* b = tree->GetBranch(name);
        if (!b) {
            std::cerr << "Warning: Branch \"" << name << "\" not found in the TTree." << std::endl;
        } else {
            tree->SetBranchAddress(name, address);
            // TBranch::GetEntries() returns number of entries stored for that branch
            Long64_t n = b->GetEntries();
            if (n == 0) {
                std::cerr << "Warning: Branch \"" << name
                          << "\" exists but contains no data (0 entries)." << std::endl;
            }
        }
    };

    checkBranch("runNumber",         &mRunNumber);
    checkBranch("subrunNumber",      &mSubrunNumber);
    checkBranch("partFileNumber",    &mPartFileNumber);
    checkBranch("runType",           &mRunType);
    checkBranch("eventNumber",       &mEventNumber);
    checkBranch("PrimaryTriggerWord",&mPrimaryTriggerWord);
    checkBranch("trigword",          &mTriggerword);
    checkBranch("GroupedTriggerTime",&mGroupedTriggerTime);
    checkBranch("GroupedTriggerWord",&mGroupedTriggerWord);
    checkBranch("Extended",          &mExtended);
    checkBranch("TankMRDCoinc",      &mTankMRDCoinc);
    checkBranch("NoVeto",            &mNoVeto);
    checkBranch("HasTank",           &mHasTank);
    checkBranch("HasMRD",            &mHasMRD);
    checkBranch("HasLAPPD",          &mHasLAPPD);
    // checkBranch("MRDTriggerType",&mMRDTriggerType); // uncomment if you need it
    checkBranch("eventTimeTank",     &mEventTimeTank);
    checkBranch("eventTimeMRD",      &mEventTimeMRD);
}


void ANNIEEventReader::ReadEntry(Long64_t entry) {
    // Notes: Load the specified entry to access LAPPD meta data.
    // This assumes that Init() has already been called and branches are correctly set.
    if (entry < 0) {
        std::cerr << "[ANNIEEventReader] Invalid entry index: " << entry << std::endl;
        return;
    }

    // This call is typically managed externally (e.g., TTree::GetEntry),
    // so you may want to call it before this method.
    // Keeping this empty unless explicit access is needed.
}

/*
void ANNIEEventReader::Print() const {
    std::cout << "RunNumber: " << mRunNumber << "\n";
    std::cout << "SubrunNumber: " << mSubrunNumber << "\n";
    std::cout << "PartFileNumber: " << mPartFileNumber << "\n";
    std::cout << "RunType: " << mRunType << "\n";
    std::cout << "EventNumber: " << mEventNumber << "\n";
    std::cout << "PrimaryTriggerWord: " << mPrimaryTriggerWord << "\n";
    std::cout << "Triggerword: " << mTriggerword << "\n";
    

    std::cout << "GroupedTriggerTime: ";
    if (mGroupedTriggerTime) {
        for (auto t : *mGroupedTriggerTime) std::cout << t << " ";
    }
    std::cout << "\n";

    std::cout << "GroupedTriggerWord: ";
    if (mGroupedTriggerWord) {
        for (auto w : *mGroupedTriggerWord) std::cout << w << " ";
    }
    std::cout << "\n";

    std::cout << "Extended: " << mExtended << "\n";
    std::cout << "TankMRDCoinc: " << mTankMRDCoinc << "\n";
    std::cout << "NoVeto: " << mNoVeto << "\n";
    std::cout << "HasTank: " << mHasTank << "\n";
    std::cout << "HasMRD: " << mHasMRD << "\n";
    std::cout << "HasLAPPD: " << mHasLAPPD << "\n";
    //std::cout << "MRDTriggerType: " << mMRDTriggerType << "\n";
    std::cout << "EventTimeTank: " << mEventTimeTank << "\n";
    std::cout << "EventTimeMRD: " << mEventTimeMRD << "\n"
    << std::endl;
}
*/







//The below is from ANNIEEventTreeMaker.cc
//if (fPrimaryTriggerWord == 14)
//    trigword = 5;


void ANNIEEventReader::Print() const {
    std::cout << "RunNumber: " << mRunNumber << "\n";
    std::cout << "SubrunNumber: " << mSubrunNumber << "\n";
    std::cout << "PartFileNumber: " << mPartFileNumber << "\n";
    std::cout << "RunType: " << mRunType << "\n";
    std::cout << "EventNumber: " << mEventNumber << "\n";
    std::cout << "PrimaryTriggerWord: " << mPrimaryTriggerWord << "\n";
    std::cout << "Triggerword: " << mTriggerword << "\n";

   // GroupedTriggerTime & GroupedTriggerWord in two columns
if (mGroupedTriggerTime && mGroupedTriggerWord) {
    assert(mGroupedTriggerTime->size() == mGroupedTriggerWord->size());

    std::cout << "\nGrouped Trigger Info:\n";
    std::cout << "Size: "<<mGroupedTriggerTime->size()<<"\n";
    
    std::cout << std::left << std::setw(30) << "TriggerTime" << std::setw(25) << "TriggerWord" << "\n";

    for (size_t i = 0; i < mGroupedTriggerTime->size(); ++i) {
        std::cout << std::left
                  << std::setw(30) << (*mGroupedTriggerTime)[i]
                  << std::setw(25) << (*mGroupedTriggerWord)[i]
                  << "\n";
    }
} else {
    std::cout << "GroupedTriggerTime or GroupedTriggerWord not available.\n";
}

    std::cout << "\nExtended: " << mExtended << "\n";
    std::cout << "TankMRDCoinc: " << mTankMRDCoinc << "\n";
    std::cout << "NoVeto: " << mNoVeto << "\n";
    std::cout << "HasTank: " << mHasTank << "\n";
    std::cout << "HasMRD: " << mHasMRD << "\n";
    std::cout << "HasLAPPD: " << mHasLAPPD << "\n";
    //std::cout << "MRDTriggerType: " << mMRDTriggerType << "\n";
    std::cout << "EventTimeTank: " << mEventTimeTank << "\n";
    std::cout << "EventTimeMRD: " << mEventTimeMRD << "\n"
              << std::endl;
}

void ANNIEEventReader::Reset() {
    mRunNumber = -1;
   mSubrunNumber = -1;
   mPartFileNumber = -1;
   mRunType = -1;
   mEventNumber = -1;
   mPrimaryTriggerWord = -1;
   mTriggerword = -1;
   mExtended = -1;
   mTankMRDCoinc = -1;
   mNoVeto = -1;
   mHasTank = -1;
   mHasMRD = -1;
   mHasLAPPD = -1;
   mMRDTriggerType = -1;
   mEventTimeTank = -1;
   mEventTimeMRD = -1;


    if (mGroupedTriggerTime) mGroupedTriggerTime->clear();
    if (mGroupedTriggerWord) mGroupedTriggerWord->clear();
}



#ifndef LAPPDEVENTREADER_H
#define LAPPDEVENTREADER_H

#include <string>
#include <vector>
#include <cstdint>
#include "TTree.h"

/*

This class will be extended later..
*/


class ANNIEEventReader {
public:
    ANNIEEventReader();
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry);
    void Print() const;
    void Reset();  

    // Getter functions
    int GetRunNumber() const { return mRunNumber; }
    int GetSubrunNumber() const { return mSubrunNumber; }
    int GetPartFileNumber() const { return mPartFileNumber; }
    int GetRunType() const { return mRunType; }
    int GetEventNumber() const { return mEventNumber; }
    int GetPrimaryTriggerWord() const { return mPrimaryTriggerWord; }
    const std::vector<uint64_t>* GetGroupedTriggerTime() const { return mGroupedTriggerTime; }
    const std::vector<uint32_t>* GetGroupedTriggerWord() const { return mGroupedTriggerWord; }
    int GetTriggerword() const { return mTriggerword; }
    int GetExtended() const { return mExtended; }
    int GetTankMRDCoinc() const { return mTankMRDCoinc; }
    int NoVeto() const { return mNoVeto; }
    int HasTank() const { return mHasTank; }
    int HasMRD() const { return mHasMRD; }
    int HasLAPPD() const { return mHasLAPPD; }
    int GetMRDTriggerType() const { return mMRDTriggerType; }
    ULong64_t GetEventTimeTank() const { return mEventTimeTank; }
    ULong64_t GetEventTimeMRD() const { return mEventTimeMRD; }

private:
    int mRunNumber;
    int mSubrunNumber;
    int mPartFileNumber;
    int mRunType;
    int mEventNumber;
    int mPrimaryTriggerWord;
    std::vector<uint64_t>* mGroupedTriggerTime = nullptr;
    std::vector<uint32_t>* mGroupedTriggerWord = nullptr;
    int mTriggerword;
    int mExtended;
    int mTankMRDCoinc;
    int mNoVeto;
    int mHasTank;
    int mHasMRD;
    int mHasLAPPD;
    int mMRDTriggerType;
    ULong64_t mEventTimeTank;
    ULong64_t mEventTimeMRD;
};


#endif // LAPPDEVENTREADER_H


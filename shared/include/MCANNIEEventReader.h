#ifndef MC_ANNIE_EVENT_READER_H
#define MC_ANNIE_EVENT_READER_H

#include <vector>
#include <cstdint>
#include "TTree.h"

class MCANNIEEventReader {
public:
    MCANNIEEventReader();
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry);
    void Print() const;
    void Reset();

    
public:
   // Getter functions
   uint64_t GetMCEventNum() const { return fMCEventNum; }
   uint16_t GetMCTriggerNum() const { return fMCTriggerNum; }
   int GetiMCTriggerNum() const { return fiMCTriggerNum; }
   
   int GetTankMRDCoinc() const { return fTankMRDCoinc; }

   double GetTrueVtxX() const { return fTrueVtxX; }
   double GetTrueVtxY() const { return fTrueVtxY; }
   double GetTrueVtxZ() const { return fTrueVtxZ; }
   double GetTrueVtxTime() const { return fTrueVtxTime; }

   double GetTrueDirX() const { return fTrueDirX; }
   double GetTrueDirY() const { return fTrueDirY; }
   double GetTrueDirZ() const { return fTrueDirZ; }

   double GetTrueAngle() const { return fTrueAngle; }
   double GetTruePhi() const { return fTruePhi; }
   double GetTrueMuonEnergy() const { return fTrueMuonEnergy; }
   int GetTruePrimaryPdg() const { return fTruePrimaryPdg; }

   double GetTrueTrackLengthInWater() const { return fTrueTrackLengthInWater; }
   double GetTrueTrackLengthInMRD() const { return fTrueTrackLengthInMRD; }
   int GetTrueMultiRing() const { return fTrueMultiRing; }

   const std::vector<int>* GetTruePrimaryPdgs() const { return fTruePrimaryPdgs; }
   const std::vector<double>* GetTrueNeutCapVtxX() const { return fTrueNeutCapVtxX; }
   const std::vector<double>* GetTrueNeutCapVtxY() const { return fTrueNeutCapVtxY; }
   const std::vector<double>* GetTrueNeutCapVtxZ() const { return fTrueNeutCapVtxZ; }
   const std::vector<double>* GetTrueNeutCapNucleus() const { return fTrueNeutCapNucleus; }
   const std::vector<double>* GetTrueNeutCapTime() const { return fTrueNeutCapTime; }
   const std::vector<double>* GetTrueNeutCapGammas() const { return fTrueNeutCapGammas; }
   const std::vector<double>* GetTrueNeutCapE() const { return fTrueNeutCapE; }
   const std::vector<double>* GetTrueNeutCapGammaE() const { return fTrueNeutCapGammaE; }

   double GetTrueNeutrinoEnergy() const { return fTrueNeutrinoEnergy; }
   double GetTrueNeutrinoMomentum_X() const { return fTrueNeutrinoMomentum_X; }
   double GetTrueNeutrinoMomentum_Y() const { return fTrueNeutrinoMomentum_Y; }
   double GetTrueNeutrinoMomentum_Z() const { return fTrueNeutrinoMomentum_Z; }

   double GetTrueNuIntxVtx_X() const { return fTrueNuIntxVtx_X; }
   double GetTrueNuIntxVtx_Y() const { return fTrueNuIntxVtx_Y; }
   double GetTrueNuIntxVtx_Z() const { return fTrueNuIntxVtx_Z; }
   double GetTrueNuIntxVtx_T() const { return fTrueNuIntxVtx_T; }

   double GetTrueFSLVtx_X() const { return fTrueFSLVtx_X; }
   double GetTrueFSLVtx_Y() const { return fTrueFSLVtx_Y; }
   double GetTrueFSLVtx_Z() const { return fTrueFSLVtx_Z; }

   double GetTrueFSLMomentum_X() const { return fTrueFSLMomentum_X; }
   double GetTrueFSLMomentum_Y() const { return fTrueFSLMomentum_Y; }
   double GetTrueFSLMomentum_Z() const { return fTrueFSLMomentum_Z; }

   double GetTrueFSLTime() const { return fTrueFSLTime; }
   double GetTrueFSLMass() const { return fTrueFSLMass; }
   int GetTrueFSLPdg() const { return fTrueFSLPdg; }
   double GetTrueFSLEnergy() const { return fTrueFSLEnergy; }
   double GetTrueQ2() const { return fTrueQ2; }

   int GetTrueCC() const { return fTrueCC; }
   int GetTrueNC() const { return fTrueNC; }
   int GetTrueQEL() const { return fTrueQEL; }
   int GetTrueRES() const { return fTrueRES; }
   int GetTrueDIS() const { return fTrueDIS; }
   int GetTrueCOH() const { return fTrueCOH; }
   int GetTrueMEC() const { return fTrueMEC; }

   int GetTrueNeutrons() const { return fTrueNeutrons; }
   int GetTrueProtons() const { return fTrueProtons; }

   int GetTruePi0() const { return fTruePi0; }
   int GetTruePiPlus() const { return fTruePiPlus; }
   int GetTruePiPlusCher() const { return fTruePiPlusCher; }
   int GetTruePiMinus() const { return fTruePiMinus; }
   int GetTruePiMinusCher() const { return fTruePiMinusCher; }

   int GetTrueKPlus() const { return fTrueKPlus; }
   int GetTrueKPlusCher() const { return fTrueKPlusCher; }
   int GetTrueKMinus() const { return fTrueKMinus; }
   int GetTrueKMinusCher() const { return fTrueKMinusCher; }

       

private:
    uint64_t fMCEventNum;
    uint16_t fMCTriggerNum;
    int fiMCTriggerNum;

    double fTrueVtxX, fTrueVtxY, fTrueVtxZ, fTrueVtxTime;
    double fTrueDirX, fTrueDirY, fTrueDirZ;
    double fTrueAngle, fTruePhi, fTrueMuonEnergy;
    int fTruePrimaryPdg;
    double fTrueTrackLengthInWater, fTrueTrackLengthInMRD;
    int fTrueMultiRing;
    
    int fTankMRDCoinc;

    std::vector<int>* fTruePrimaryPdgs = nullptr;
    std::vector<double>* fTrueNeutCapVtxX = nullptr;
    std::vector<double>* fTrueNeutCapVtxY = nullptr;
    std::vector<double>* fTrueNeutCapVtxZ = nullptr;
    std::vector<double>* fTrueNeutCapNucleus = nullptr;
    std::vector<double>* fTrueNeutCapTime = nullptr;
    std::vector<double>* fTrueNeutCapGammas = nullptr;
    std::vector<double>* fTrueNeutCapE = nullptr;
    std::vector<double>* fTrueNeutCapGammaE = nullptr;
    

    // GENIE
    double fTrueNeutrinoEnergy;
    double fTrueNeutrinoMomentum_X, fTrueNeutrinoMomentum_Y, fTrueNeutrinoMomentum_Z;
    double fTrueNuIntxVtx_X, fTrueNuIntxVtx_Y, fTrueNuIntxVtx_Z, fTrueNuIntxVtx_T;
    double fTrueFSLVtx_X, fTrueFSLVtx_Y, fTrueFSLVtx_Z;
    double fTrueFSLMomentum_X, fTrueFSLMomentum_Y, fTrueFSLMomentum_Z;
    double fTrueFSLTime, fTrueFSLMass;
    int fTrueFSLPdg;
    double fTrueFSLEnergy, fTrueQ2;

    int fTrueCC, fTrueNC, fTrueQEL, fTrueRES, fTrueDIS, fTrueCOH, fTrueMEC;
    int fTrueNeutrons, fTrueProtons;
    int fTruePi0, fTruePiPlus, fTruePiPlusCher;
    int fTruePiMinus, fTruePiMinusCher;
    int fTrueKPlus, fTrueKPlusCher, fTrueKMinus, fTrueKMinusCher;
    
    
};

#endif


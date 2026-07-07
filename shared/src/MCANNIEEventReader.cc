#include "MCANNIEEventReader.h"
#include <iostream>
#include <cassert>
#include <iomanip>

MCANNIEEventReader::MCANNIEEventReader()
    : fMCEventNum(0),
      fMCTriggerNum(0),
      fiMCTriggerNum(-999),

      fTrueVtxX(-999.0), fTrueVtxY(-999.0),
      fTrueVtxZ(-999.0), fTrueVtxTime(-999.0),

      fTrueDirX(-999.0), fTrueDirY(-999.0), fTrueDirZ(-999.0),
      fTrueAngle(-999.0), fTruePhi(-999.0), fTrueMuonEnergy(-999.0),

      fTruePrimaryPdg(-999),
      fTrueTrackLengthInWater(-999.0), fTrueTrackLengthInMRD(-999.0),
      fTrueMultiRing(-999),

      // GENIE
      fTrueNeutrinoEnergy(-999.0),
      fTrueNeutrinoMomentum_X(-999.0),
      fTrueNeutrinoMomentum_Y(-999.0),
      fTrueNeutrinoMomentum_Z(-999.0),

      fTrueNuIntxVtx_X(-999.0), fTrueNuIntxVtx_Y(-999.0),
      fTrueNuIntxVtx_Z(-999.0), fTrueNuIntxVtx_T(-999.0),

      fTrueFSLVtx_X(-999.0), fTrueFSLVtx_Y(-999.0), fTrueFSLVtx_Z(-999.0),
      fTrueFSLMomentum_X(-999.0), fTrueFSLMomentum_Y(-999.0), fTrueFSLMomentum_Z(-999.0),
      fTrueFSLTime(-999.0), fTrueFSLMass(-999.0),
      fTrueFSLPdg(-999),
      fTrueFSLEnergy(-999.0), fTrueQ2(-999.0),

      fTrueCC(-999), fTrueNC(-999), fTrueQEL(-999), fTrueRES(-999),
      fTrueDIS(-999), fTrueCOH(-999), fTrueMEC(-999),

      fTrueNeutrons(-999), fTrueProtons(-999),

      fTruePi0(-999), fTruePiPlus(-999), fTruePiPlusCher(-999),
      fTruePiMinus(-999), fTruePiMinusCher(-999),

      fTrueKPlus(-999), fTrueKPlusCher(-999),
      fTrueKMinus(-999), fTrueKMinusCher(-999)
{
}



void MCANNIEEventReader::Init(TTree* tree) {
    if (!tree) {
        std::cerr << "Error: Null TTree pointer in MCANNIEEventReader::Init\n";
        return;
    }

    auto checkBranch = [&](const char* name, void* address) {
        TBranch* b = tree->GetBranch(name);
        if (!b) {
            std::cerr << "Warning: Branch \"" << name << "\" not found.\n";
        } else {
            tree->SetBranchAddress(name, address);
        }
    };

    checkBranch("mcEntryNumber", &fMCEventNum);
    checkBranch("triggerNumber", &fiMCTriggerNum);
    checkBranch("TankMRDCoinc", &fTankMRDCoinc);

    checkBranch("trueVtxX", &fTrueVtxX);
    checkBranch("trueVtxY", &fTrueVtxY);
    checkBranch("trueVtxZ", &fTrueVtxZ);
    checkBranch("trueVtxTime", &fTrueVtxTime);
    checkBranch("trueDirX", &fTrueDirX);
    checkBranch("trueDirY", &fTrueDirY);
    checkBranch("trueDirZ", &fTrueDirZ);
    checkBranch("trueAngle", &fTrueAngle);
    checkBranch("truePhi", &fTruePhi);
    checkBranch("trueMuonEnergy", &fTrueMuonEnergy);
    checkBranch("truePrimaryPdg", &fTruePrimaryPdg);
    checkBranch("trueTrackLengthInWater", &fTrueTrackLengthInWater);
    checkBranch("trueTrackLengthInMRD", &fTrueTrackLengthInMRD);
    checkBranch("trueMultiRing", &fTrueMultiRing);

    checkBranch("truePrimaryPdgs", &fTruePrimaryPdgs);
    checkBranch("trueNeutCapVtxX", &fTrueNeutCapVtxX);
    checkBranch("trueNeutCapVtxY", &fTrueNeutCapVtxY);
    checkBranch("trueNeutCapVtxZ", &fTrueNeutCapVtxZ);
    checkBranch("trueNeutCapNucleus", &fTrueNeutCapNucleus);
    checkBranch("trueNeutCapTime", &fTrueNeutCapTime);
    checkBranch("trueNeutCapGammas", &fTrueNeutCapGammas);
    checkBranch("trueNeutCapE", &fTrueNeutCapE);
    checkBranch("trueNeutCapGammaE", &fTrueNeutCapGammaE);

    checkBranch("trueNeutrinoEnergy", &fTrueNeutrinoEnergy);
    checkBranch("trueNeutrinoMomentum_X", &fTrueNeutrinoMomentum_X);
    checkBranch("trueNeutrinoMomentum_Y", &fTrueNeutrinoMomentum_Y);
    checkBranch("trueNeutrinoMomentum_Z", &fTrueNeutrinoMomentum_Z);
    checkBranch("trueNuIntxVtx_X", &fTrueNuIntxVtx_X);
    checkBranch("trueNuIntxVtx_Y", &fTrueNuIntxVtx_Y);
    checkBranch("trueNuIntxVtx_Z", &fTrueNuIntxVtx_Z);
    checkBranch("trueNuIntxVtx_T", &fTrueNuIntxVtx_T);
    checkBranch("trueFSLVtx_X", &fTrueFSLVtx_X);
    checkBranch("trueFSLVtx_Y", &fTrueFSLVtx_Y);
    checkBranch("trueFSLVtx_Z", &fTrueFSLVtx_Z);
    checkBranch("trueFSLMomentum_X", &fTrueFSLMomentum_X);
    checkBranch("trueFSLMomentum_Y", &fTrueFSLMomentum_Y);
    checkBranch("trueFSLMomentum_Z", &fTrueFSLMomentum_Z);
    checkBranch("trueFSLTime", &fTrueFSLTime);
    checkBranch("trueFSLMass", &fTrueFSLMass);
    checkBranch("trueFSLPdg", &fTrueFSLPdg);
    checkBranch("trueFSLEnergy", &fTrueFSLEnergy);
    checkBranch("trueQ2", &fTrueQ2);

    checkBranch("trueCC", &fTrueCC);
    checkBranch("trueNC", &fTrueNC);
    checkBranch("trueQEL", &fTrueQEL);
    checkBranch("trueRES", &fTrueRES);
    checkBranch("trueDIS", &fTrueDIS);
    checkBranch("trueCOH", &fTrueCOH);
    checkBranch("trueMEC", &fTrueMEC);

    checkBranch("trueNeutrons", &fTrueNeutrons);
    checkBranch("trueProtons", &fTrueProtons);
    checkBranch("truePi0", &fTruePi0);
    checkBranch("truePiPlus", &fTruePiPlus);
    checkBranch("truePiPlusCher", &fTruePiPlusCher);
    checkBranch("truePiMinus", &fTruePiMinus);
    checkBranch("truePiMinusCher", &fTruePiMinusCher);
    checkBranch("trueKPlus", &fTrueKPlus);
    checkBranch("trueKPlusCher", &fTrueKPlusCher);
    checkBranch("trueKMinus", &fTrueKMinus);
    checkBranch("trueKMinusCher", &fTrueKMinusCher);
}

void MCANNIEEventReader::ReadEntry(Long64_t entry) {
    // Placeholder: Use TTree::GetEntry externally
}



void MCANNIEEventReader::Print() const {
    std::cout << "\n===== MC Event Info =====\n";
    std::cout << "MC Event Number: " << fMCEventNum << "\n";
    std::cout << "MC Trigger Number: " << fiMCTriggerNum << "\n";

    std::cout << "\n===== True Muon Vertex & Direction =====\n";
    std::cout << "Vtx: (" << fTrueVtxX << ", " << fTrueVtxY << ", " << fTrueVtxZ << ") at T = " << fTrueVtxTime << " ns\n";
    std::cout << "Direction: (" << fTrueDirX << ", " << fTrueDirY << ", " << fTrueDirZ << ")\n";
    std::cout << "Angle: " << fTrueAngle << " deg, Phi: " << fTruePhi << " rad?\n";
    std::cout << "Muon Energy: " << fTrueMuonEnergy << " MeV\n";
    std::cout << "Primary PDG: " << fTruePrimaryPdg << "\n";
    std::cout << "Track Length (Water): " << fTrueTrackLengthInWater << " m?, (MRD): " << fTrueTrackLengthInMRD << " m?\n";
    std::cout << "Multi-ring: " << fTrueMultiRing << "\n";

    std::cout << "\n===== Neutron Capture Info =====\n";
    if (fTrueNeutCapVtxX && fTrueNeutCapVtxX->size() > 0) {
        std::cout << "Captured Neutrons: " << fTrueNeutCapVtxX->size() << "\n";
        for (size_t i = 0; i < fTrueNeutCapVtxX->size(); ++i) {
            std::cout << "  [" << i << "] Vtx = (" 
                      << (*fTrueNeutCapVtxX)[i] << ", "
                      << (*fTrueNeutCapVtxY)[i] << ", "
                      << (*fTrueNeutCapVtxZ)[i] << "), "
                      << "T = " << (*fTrueNeutCapTime)[i] 
                      << ", E = " << (*fTrueNeutCapE)[i]
                      << "\n";
        }
    } else {
        std::cout << "No neutron captures recorded.\n";
    }

    std::cout << "\n===== True Primary PDGs =====\n";
    if (fTruePrimaryPdgs) {
        for (auto pdg : *fTruePrimaryPdgs) std::cout << pdg << " ";
        std::cout << "\n";
    }

    std::cout << "\n===== GENIE True Neutrino Info =====\n";
    std::cout << "Neutrino Energy: " << fTrueNeutrinoEnergy << " MeV\n";
    std::cout << "Momentum (X,Y,Z): " << fTrueNeutrinoMomentum_X << ", "
              << fTrueNeutrinoMomentum_Y << ", " << fTrueNeutrinoMomentum_Z << "\n";
    std::cout << "Interaction Vertex (X,Y,Z,T): " << fTrueNuIntxVtx_X << ", "
              << fTrueNuIntxVtx_Y << ", " << fTrueNuIntxVtx_Z << ", "
              << fTrueNuIntxVtx_T << "\n";

    std::cout << "\n===== Final State Lepton (FSL) Info =====\n";
    std::cout << "Vtx: (" << fTrueFSLVtx_X << ", " << fTrueFSLVtx_Y << ", " << fTrueFSLVtx_Z << ")\n";
    std::cout << "Momentum: (" << fTrueFSLMomentum_X << ", " << fTrueFSLMomentum_Y << ", " << fTrueFSLMomentum_Z << ")\n";
    std::cout << "Time: " << fTrueFSLTime << ", Mass: " << fTrueFSLMass << ", PDG: " << fTrueFSLPdg << ", Energy: " << fTrueFSLEnergy << "\n";

    std::cout << "\n===== Interaction Types =====\n";
    std::cout << "CC: " << fTrueCC << ", NC: " << fTrueNC 
              << ", QEL: " << fTrueQEL << ", RES: " << fTrueRES 
              << ", DIS: " << fTrueDIS << ", COH: " << fTrueCOH 
              << ", MEC: " << fTrueMEC << ", Q2: " << fTrueQ2 << "\n";

    std::cout << "\n===== Final State Particles =====\n";
    std::cout << "Neutrons: " << fTrueNeutrons << ", Protons: " << fTrueProtons << "\n";
    std::cout << "Pi0: " << fTruePi0 << ", Pi+: " << fTruePiPlus << " (Cherenkov: " << fTruePiPlusCher << ")\n";
    std::cout << "Pi-: " << fTruePiMinus << " (Cherenkov: " << fTruePiMinusCher << ")\n";
    std::cout << "K+: " << fTrueKPlus << " (Cherenkov: " << fTrueKPlusCher << "), "
              << "K-: " << fTrueKMinus << " (Cherenkov: " << fTrueKMinusCher << ")\n";

    std::cout << "==============================\n\n";
}



void MCANNIEEventReader::Reset() {

    fTankMRDCoinc = -999;
    // Event & trigger
    fMCEventNum   = 0;
    fMCTriggerNum = 0;
    fiMCTriggerNum = -999;

    // True muon vertex & direction
    fTrueVtxX = fTrueVtxY = fTrueVtxZ = fTrueVtxTime = -999.0;
    fTrueDirX = fTrueDirY = fTrueDirZ = -999.0;
    fTrueAngle = fTruePhi = fTrueMuonEnergy = -999.0;
    fTruePrimaryPdg = -999;
    fTrueTrackLengthInWater = fTrueTrackLengthInMRD = -999.0;
    fTrueMultiRing = -999;

    // GENIE: neutrino kinematics
    fTrueNeutrinoEnergy = -999.0;
    fTrueNeutrinoMomentum_X = -999.0;
    fTrueNeutrinoMomentum_Y = -999.0;
    fTrueNeutrinoMomentum_Z = -999.0;

    // Neutrino interaction vertex
    fTrueNuIntxVtx_X = fTrueNuIntxVtx_Y = fTrueNuIntxVtx_Z = fTrueNuIntxVtx_T = -999.0;

    // Final State Lepton (FSL)
    fTrueFSLVtx_X = fTrueFSLVtx_Y = fTrueFSLVtx_Z = -999.0;
    fTrueFSLMomentum_X = fTrueFSLMomentum_Y = fTrueFSLMomentum_Z = -999.0;
    fTrueFSLTime = fTrueFSLMass = -999.0;
    fTrueFSLPdg = -999;
    fTrueFSLEnergy = fTrueQ2 = -999.0;

    // Interaction types
    fTrueCC  = -999;
    fTrueNC  = -999;
    fTrueQEL = -999;
    fTrueRES = -999;
    fTrueDIS = -999;
    fTrueCOH = -999;
    fTrueMEC = -999;

    // Final state hadrons
    fTrueNeutrons = -999;
    fTrueProtons  = -999;

    fTruePi0        = -999;
    fTruePiPlus     = -999;
    fTruePiPlusCher = -999;
    fTruePiMinus    = -999;
    fTruePiMinusCher= -999;

    fTrueKPlus      = -999;
    fTrueKPlusCher  = -999;
    fTrueKMinus     = -999;
    fTrueKMinusCher = -999;

    // Vectors
    if (fTruePrimaryPdgs)      fTruePrimaryPdgs->clear();
    if (fTrueNeutCapVtxX)      fTrueNeutCapVtxX->clear();
    if (fTrueNeutCapVtxY)      fTrueNeutCapVtxY->clear();
    if (fTrueNeutCapVtxZ)      fTrueNeutCapVtxZ->clear();
    if (fTrueNeutCapNucleus)   fTrueNeutCapNucleus->clear();
    if (fTrueNeutCapTime)      fTrueNeutCapTime->clear();
    if (fTrueNeutCapGammas)    fTrueNeutCapGammas->clear();
    if (fTrueNeutCapE)         fTrueNeutCapE->clear();
    if (fTrueNeutCapGammaE)    fTrueNeutCapGammaE->clear();
}


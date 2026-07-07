#include "TFile.h"
#include "TTree.h"
#include <cstdlib>  
#include <iostream>
#include "LAPPDPulseReader.h"
#include "LAPPDHitReader.h"
#include "PMTHitReader.h"
#include "FMVHitReader.h"
#include "ANNIEEventReader.h"
#include "LAPPDMetaReader.h"
#include "TankClusterReader.h"
#include "MRDClusterReader.h"
#include "MRDRecoReader.h"

// -----------------------------------------------------------------------------------

void printANNIE(const char* filename);

// -----------------------------------------------------------------------------------

int main(int argc, char* argv[]) 
{
   const char* filename = "ANNIETree.root";
   if (argc > 1) 
      filename = argv[1];

   printANNIE(filename);

   return 0;
}

// -----------------------------------------------------------------------------------

void printANNIE(const char* filename) {
    TFile* file = TFile::Open(filename);
    
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    TTree* tree = (TTree*)file->Get("Event");
    if (!tree) {
        std::cerr << "Error: Tree 'Event' not found in file!" << std::endl;
        file->Close();
        return;
    }

   Long64_t nEntries = tree->GetEntries();
   std::cout << "Number of events in this tree: " << nEntries << std::endl;

     
    ANNIEEventReader eventReader;
    eventReader.Init(tree);
    
    //LAPPDMetaReader metaReader;
    //metaReader.Init(tree);

    //LAPPDPulseReader lappdPulseReader;
    //lappdPulseReader.Init(tree);
    
    LAPPDHitReader lappdHitReader;
    lappdHitReader.Init(tree);
    
    PMTHitReader pmtHitReader;
    pmtHitReader.Init(tree);
    
    FMVHitReader fmvHitReader;
    fmvHitReader.Init(tree);
    
    TankClusterReader tankClusterReader;
    tankClusterReader.Init(tree);
    
    MRDClusterReader mrdClusterReader;
    mrdClusterReader.Init(tree);
    
    MRDRecoReader mrdRecoReader;
    mrdRecoReader.Init(tree);

    Long64_t firstEvent = 0;
    Long64_t lastEvent  = 500;
    
    for (Long64_t i = firstEvent; i < nEntries; ++i) {  
        tree->GetEntry(i);
        
        std::cout << "============ Event " << i << " ====================\n";
        
        eventReader.Print();
        //metaReader.Print();

        
        //ELAPPDPulses eLAPPDPulses;
        //lappdPulseReader.ReadEntry(i, eLAPPDPulses); 

        //ELAPPDHits eLAPPDHits;
        //lappdHitReader.ReadEntry(i, eLAPPDHits);
        
        EPMTHits ePMTHits;
        pmtHitReader.ReadEntry(i, ePMTHits); 
        
        EFMVHits eFMVHits;
        fmvHitReader.ReadEntry(i, eFMVHits);
        
        ETankClusters eTankClusters;
        tankClusterReader.ReadEntry(i, eTankClusters); 
        
        EMRDClusters eMRDClusters;
        mrdClusterReader.ReadEntry(i, eMRDClusters); 
        
        EMRDRecos eMRDRecos;
        mrdRecoReader.ReadEntry(i, eMRDRecos);
        
        //metaReader.Print();
        //eLAPPDPulses.Print(); 
        //eLAPPDHits.Print();
        //ePMTHits.Print();
        //eFMVHits.Print();
        //eTankClusters.Print();
        
        //std::cout<<"Has Veto: "<<eventReader.HasVeto()<<std::endl;
        //eFMVHits.Print();
        //eTankClusters.Print();
        //eMRDClusters.Print();
        eMRDRecos.Print();
        
        
        eventReader.Reset();  
        std::cout << "\n";
    } //event loop end

    // Clean up and close file
    file->Close();
    delete file;
}




// LikelihoodReco.cpp

#include "TFile.h"
#include "TTree.h"
#include "TMinuit.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "INIReader.h"
// ekle
#include "TankClusterReader.h"
#include "ETankClusters.h"
#include "TankCluster.h"
#include "PMTHit.h"

#include "ANNIEUtility.h"
#include "MCANNIEEventReader.h"

#include "MRDRecoReader.h"
#include "TRandom.h"

// ------------------------------------------------------------

struct HitInfo {
    double x;
    double y;
    double z;
    double t;
    double pe;
};

// ------------------------------------------------------------

std::vector<HitInfo> gHits;

double gDirX = 0.0;
double gDirY = 0.0;
double gDirZ = 1.0;

double gSigmaT   = 2.0;
double gC        = 0.299792458;  // m/ns;
double gNWater   = 1.33;
double gBetaMuon = 1.0;
double gThetaC   = 0.0;

// ------------------------------------------------------------

double ExpectedTime(
    double vx,
    double vy,
    double vz,
    double t0,
    const HitInfo& hit,
    bool& valid)
{
    valid = false;

    // r_vtx = (vx, vy, vz)
    // r_pmt = (hit.x, hit.y, hit.z)
    // w = r_pmt - r_vtx
    double wx = hit.x - vx;
    double wy = hit.y - vy;
    double wz = hit.z - vz;

    // a = w . u
    // u = muon direction
    double a = wx*gDirX + wy*gDirY + wz*gDirZ;

    // |w|^2
    double w2 = wx*wx + wy*wy + wz*wz;

    // rho^2 = |w|^2 - (w.u)^2
    double rho2 = w2 - a*a;

    if (rho2 < 0.0)
        rho2 = 0.0;

    double rho = std::sqrt(rho2);

    // theta_C = arccos(1/(beta*n))
    double tanTheta = std::tan(gThetaC);

    if (tanTheta <= 0.0)
        return 0.0;

    // Cherenkov emission point:
    //
    // r_emit = r_vtx + s*u
    //
    // tan(theta_C) = rho / (a - s)
    //
    // s = a - rho/tan(theta_C)
    double s = a - rho / tanTheta;

    // Emission point before vertex is rejected
    if (s < 0.0)
        return 0.0;

    // a - s = photon longitudinal distance
    double longitudinalPhoton = a - s;

    if (longitudinalPhoton <= 0.0)
        return 0.0;

    // L_gamma^2 = rho^2 + (a-s)^2
    double Lgamma = std::sqrt(
        rho*rho +
        longitudinalPhoton*longitudinalPhoton
    );

    double vMuon  = gBetaMuon * gC;
    double vLight = gC / gNWater;

    // t_expected = t0 + s/v_mu + L_gamma/(c/n)
    double expected =
        t0 +
        s / vMuon +
        Lgamma / vLight;

    valid = true;

    return expected;
}

// ------------------------------------------------------------
//function to be minimized
void FCN(
    Int_t& npar,
    Double_t* grad,
    Double_t& f,
    Double_t* par,
    Int_t flag)
{
    double vx = par[0];
    double vy = par[1];
    double vz = par[2];
    double t0 = par[3];

    double nll = 0.0;
    int nUsed = 0;

    for (const auto& hit : gHits) {

        bool valid = false;

        double tExp = ExpectedTime(
            vx, vy, vz, t0, hit, valid);

        if (!valid)
            continue;

        double residual = hit.t - tExp;

        // NLL = sum 0.5 * ((t_hit - t_exp)/sigma_t)^2
        nll += 0.5 * residual * residual /
               (gSigmaT * gSigmaT);

        ++nUsed;
    }

      //bu ne icin?
    if (nUsed < 4)
        nll = 1e12;

    f = nll;
}

// ------------------------------------------------------------

void makeLikelihoodReco(INIReader& iniReader)
{
   std::string inputFile =
     iniReader.Get("Input", "InputFile", "");

   std::string outputFile =
     iniReader.Get("Output", "OutputFile", "LikelihoodReco.root");

   std::string treeName =
     iniReader.Get("Input", "TreeName", "Event");

   gSigmaT =
     iniReader.GetReal("Likelihood", "SigmaT", 2.0);

   gNWater =
     iniReader.GetReal("Likelihood", "NWater", 1.33);

   gBetaMuon =
     iniReader.GetReal("Likelihood", "BetaMuon", 1.0);

   double minPE =
     iniReader.GetReal("Likelihood", "MinPE", 0.0);

   double minTime =
     iniReader.GetReal("Likelihood", "MinTime", 0.0);

   double maxTime =
     iniReader.GetReal("Likelihood", "MaxTime", 2000.0);

   double pmtPositionScale =
     iniReader.GetReal("Likelihood", "PMTPositionScale", 1.0);

   int minHits =
     iniReader.GetInteger("Likelihood", "MinHits", 4);

    gThetaC = std::acos(1.0 / (gBetaMuon * gNWater));

   TFile* inputRootFile =
     TFile::Open(inputFile.c_str(), "READ");

   if (!inputRootFile || inputRootFile->IsZombie()) {
      std::cerr << "Error opening file: "
               << inputFile << std::endl;
      return;
   }

   TTree* inputTree = (TTree*)inputRootFile->Get(treeName.c_str());

   if (!inputTree) {
      std::cerr << "Error: tree not found: "<< treeName << std::endl;
      inputRootFile->Close();
      return;
   }

   

   TankClusterReader tankClusterReader;
   tankClusterReader.Init(inputTree);
   
   MRDRecoReader mrdRecoReader;
   mrdRecoReader.Init(inputTree);
   
   MCANNIEEventReader mcEventReader;
   mcEventReader.Init(inputTree);

   TFile* outputRootFile = TFile::Open(outputFile.c_str(), "RECREATE");

   TTree* outputTree = inputTree->CloneTree(0);

   double LikelihoodRecoX  = -9999;
   double LikelihoodRecoY  = -9999;
   double LikelihoodRecoZ  = -9999;

   double LikelihoodRecoDirX = -9999;
   double LikelihoodRecoDirY = -9999;
   double LikelihoodRecoDirZ = -9999;
   double LikelihoodRecoEnergy = -9999;

   double LikelihoodRecoT0 = -9999;


   int LikelihoodRecoFlag = 0;
   
  

   outputTree->Branch("LikelihoodRecoX",
                    &LikelihoodRecoX,
                    "LikelihoodRecoX/D");

   outputTree->Branch("LikelihoodRecoY",
                    &LikelihoodRecoY,
                    "LikelihoodRecoY/D");

   outputTree->Branch("LikelihoodRecoZ",
                    &LikelihoodRecoZ,
                    "LikelihoodRecoZ/D");
                    
   outputTree->Branch("LikelihoodRecoDirX",
                &LikelihoodRecoDirX,
                "LikelihoodRecoDirX/D");

   outputTree->Branch("LikelihoodRecoDirY",
                   &LikelihoodRecoDirY,
                   "LikelihoodRecoDirY/D");

   outputTree->Branch("LikelihoodRecoDirZ",
                   &LikelihoodRecoDirZ,
                   "LikelihoodRecoDirZ/D");

   outputTree->Branch("LikelihoodRecoEnergy",
                   &LikelihoodRecoEnergy,
                   "LikelihoodRecoEnergy/D");                    

   outputTree->Branch("LikelihoodRecoT0",
                    &LikelihoodRecoT0,
                    "LikelihoodRecoT0/D");



   outputTree->Branch("LikelihoodRecoFlag",
                    &LikelihoodRecoFlag,
                    "LikelihoodRecoFlag/I");

 



    Long64_t nEntries = inputTree->GetEntries();
    Long64_t nSuccess = 0;

   for (Long64_t i = 0; i < nEntries; ++i) {
      
      inputTree->GetEntry(i);
      
      std::cout<< "==========Event = "<< i<< "=========="<< std::endl;

      //Default
      LikelihoodRecoX  = -9999;
      LikelihoodRecoY  = -9999;
      LikelihoodRecoZ  = -9999;

      LikelihoodRecoDirX = -9999;
      LikelihoodRecoDirY = -9999;
      LikelihoodRecoDirZ = -9999;
      
      LikelihoodRecoEnergy = -9999; //not implemented ye!

      LikelihoodRecoT0 = -9999; //taken from MC
     
      LikelihoodRecoFlag = 0;
     
    


      ///////////////////////////////
      ///////////////////////////////
      ///obtain direction from MRDReco for each event  
      EMRDRecos eMRDRecos;
      mrdRecoReader.ReadEntry(i, eMRDRecos);

      const auto& recos = eMRDRecos.Get();

      if (recos.size() != 1) {
          outputTree->Fill();
          continue;
      }

      const auto& reco = recos.at(0);

      if (!reco.IsStop()) {
          outputTree->Fill();
          continue;
      }

      //return wrt global cs in units of meter
      double startx = reco.GetStartX(); 
      double starty = reco.GetStartY();
      double startz = reco.GetStartZ();

      double stopx = reco.GetStopX();
      double stopy = reco.GetStopY();
      double stopz = reco.GetStopZ();

      double dx = stopx - startx;
      double dy = stopy - starty;
      double dz = stopz - startz;

      double dirNorm = std::sqrt(dx*dx + dy*dy + dz*dz);

      if (dirNorm <= 0.0) {
          outputTree->Fill();
          continue;
      }

      gDirX = dx / dirNorm;
      gDirY = dy / dirNorm;
      gDirZ = dz / dirNorm;

      LikelihoodRecoDirX = gDirX;
      LikelihoodRecoDirY = gDirY;
      LikelihoodRecoDirZ = gDirZ;
      //////////////////////////
     
     
      //clear for each event
      gHits.clear();

      ETankClusters eTankClusters;
      tankClusterReader.ReadEntry(i, eTankClusters);

      const auto& clusters = eTankClusters.Get();

      for (const auto& cluster : clusters) {

         for (const auto& pmtHit : cluster.GetHits() ) {

            double t  = pmtHit.GetT();
            double pe = pmtHit.GetPE();

            if (t < minTime || t > maxTime)
               continue;

            if (pe < minPE)
               continue;

            HitInfo h;
            double x = pmtHit.GetX() * pmtPositionScale; //return wrt tank center in units of meter
            double y = pmtHit.GetY() * pmtPositionScale;
            double z = pmtHit.GetZ() * pmtPositionScale;
            
            /*
            std::cout << "Before TankToGlobal : "
                      << "x = " << x
                      << ", y = " << y
                      << ", z = " << z
                      << std::endl;            
            */
            ANNIEUtility::TankToGlobal( x, y, z);
            h.x = x;
            h.y = y;
            h.z = z;
            h.t = t;
            h.pe = pe;
            
            /*
            std::cout << "After  TankToGlobal : "
                      << "x = " << h.x
                      << ", y = " <<h.y
                      << ", z = " << h.z
                      << std::endl;
            */

            /* in terms of meter
            std::cout
            << "PMT ID=" << pmtHit.GetDetID()
            << " X=" << h.x
            << " Y=" << h.y
            << " Z=" << h.z
            << " T=" << h.t
            << " Q=" << h.q
            << std::endl;
            */
            gHits.push_back(h);
         }//hit loop end
      
      }//cluster loop end

     

      if ((int)gHits.size() < minHits) {
         outputTree->Fill();
         continue;
      }



      double initX = -1.4 + 2.8 * gRandom->Rndm();
      double initY = -2.2 + 4.2 * gRandom->Rndm();
      double initZ =  0.2 + 3.0 * gRandom->Rndm();
      double initT0 = mcEventReader.GetTrueVtxTime();
         
      double initialNLL = 0.0;
      int initialUsedHits = 0;



      //for printing
      for (const auto& hit : gHits)
      {
          bool valid = false;

          double tExp =
              ExpectedTime(
                  initX,
                  initY,
                  initZ,
                  initT0,
                  hit,
                  valid);

          if (!valid)
              continue;

          double residual = hit.t - tExp;

          initialNLL +=
              0.5 * residual * residual /
              (gSigmaT * gSigmaT);

          ++initialUsedHits;
          
          
          
      }     
        
      /////////////////////////
      ////////////////////////  
      TMinuit minuit(4);
      minuit.SetFCN(FCN);

      double arglist[10];
      int ierflg = 0;

      arglist[0] = -1;
      minuit.mnexcm("SET PRINT", arglist, 1, ierflg);
      /*
      minuit.DefineParameter(0, "vtxX", initX, 0.05, -1.6, 1.6);
      minuit.DefineParameter(1, "vtxY", initY, 0.05, -2.4, 2.2);
      minuit.DefineParameter(2, "vtxZ", initZ, 0.05,  0.0, 3.4);
      minuit.DefineParameter(3, "t0",   initT0, 1, -100.0, 100.0);
      */    
      
      
      std::cout << "\nInitial values\n";
      std::cout << "initX  = " << initX
                << "   limits [-1.4, 1.4]\n";

      std::cout << "initY  = " << initY
                << "   limits [-2.2, 2.0]\n";

      std::cout << "initZ  = " << initZ
                << "   limits [0.2, 3.2]\n";

      std::cout << "initT0 = " << initT0
                << "   limits [-100,100]\n";
            
      
      minuit.DefineParameter(0, "vtxX", initX, 0.05, -1.4, 1.4);
      minuit.DefineParameter(1, "vtxY", initY, 0.05, -2.2, 2.0);
      minuit.DefineParameter(2, "vtxZ", initZ, 0.05,  0.2, 3.2);
      minuit.DefineParameter(3, "t0",   initT0, 1.0, -100, 100.0);
      minuit.FixParameter(3);

      arglist[0] = 5000;
      arglist[1] = 1.0;

      minuit.mnexcm("MIGRAD", arglist, 2, ierflg);

      double fitX, errX;
      double fitY, errY;
      double fitZ, errZ;
      double fitT0, errT0;

      minuit.GetParameter(0, fitX, errX);
      minuit.GetParameter(1, fitY, errY);
      minuit.GetParameter(2, fitZ, errZ);
      minuit.GetParameter(3, fitT0, errT0);

      double amin, edm, errdef;
      int nvpar, nparx, icstat;

      minuit.mnstat(amin,
                   edm,
                   errdef,
                   nvpar,
                   nparx,
                   icstat);
                      
         
      LikelihoodRecoX  = fitX;
      LikelihoodRecoY  = fitY;
      LikelihoodRecoZ  = fitZ;
      LikelihoodRecoT0 = fitT0;

      
         
      bool goodFit = (ierflg == 0) && (icstat >= 2) && (edm < 1e-2);

      if (goodFit)
      {
          LikelihoodRecoFlag = 1;
          ++nSuccess;
      }  
      
      
      //print for each event
                      
      std::cout
          << "Hits = "
          << gHits.size()
          << std::endl;
          
      std::cout << "EDM = " << edm << std::endl;

      std::cout << "fitX = " << fitX
                << " +/- " << errX << std::endl;

      std::cout << "fitY = " << fitY
                << " +/- " << errY << std::endl;

      std::cout << "fitZ = " << fitZ
                << " +/- " << errZ << std::endl;

      std::cout << "fitT0 = " << fitT0
                << " +/- " << errT0 << std::endl;

      std::cout
          << "Initial NLL = "
          << initialNLL
          << std::endl;

      std::cout
          << "Final NLL   = "
          << amin
          << std::endl;

      std::cout
          << "Improvement = "
          << (initialNLL - amin)
          << std::endl;

      std::cout
          << "ierflg = "
          << ierflg
          << std::endl;

      std::cout
          << "icstat = "
          << icstat
          << std::endl;                      
                            
      std::cout
          << "Fit Status = "
          << (goodFit ? "SUCCESS" : "FAILED")
          << std::endl;                      
                                                
      std::cout
          << "\nHit-by-hit timing check"
          << std::endl;

      for (size_t ihit = 0; ihit < gHits.size(); ++ihit)
      {
          const auto& hit = gHits[ihit];

          bool valid = false;

          double tExp =
              ExpectedTime(
                  fitX,
                  fitY,
                  fitZ,
                  fitT0,
                  hit,
                  valid);

          if (!valid)
              continue;

          double residual = hit.t - tExp;

          std::cout
              << "Hit "
              << ihit
              << "  Measured=" << hit.t
              << " ns"
              << "  Expected=" << tExp
              << " ns"
              << "  Residual=" << residual
              << " ns"
              << std::endl;
      }             
                      
      std::cout<< "\n=========Event ended========================="<< std::endl;                  
         
                   

      outputTree->Fill();
   
   }//end of event loop

   std::cout << "----------Run results------------------" << std::endl;
   std::cout << "LikelihoodReco" << std::endl;
   std::cout << "InputFile  : " << inputFile << std::endl;
   std::cout << "OutputFile : " << outputFile << std::endl;
   std::cout << "Entries    : " << nEntries << std::endl;
   std::cout << "Success    : " << nSuccess << std::endl;
   std::cout << "----------------------------------" << std::endl;

   outputRootFile->cd();
   outputTree->Write();

   outputRootFile->Close();
   inputRootFile->Close();

   delete outputRootFile;
   delete inputRootFile;
}

// ------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr
            << "Usage: ./LikelihoodReco config.ini"
            << std::endl;
        return 1;
    }

    INIReader iniReader(argv[1]);

    if (iniReader.ParseError() != 0) {
        std::cerr
            << "Error: cannot read config file: "
            << argv[1]
            << std::endl;
        return 1;
    }

    makeLikelihoodReco(iniReader);

    return 0;
}

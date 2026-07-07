#include <iostream>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TProfile.h>
#include <TF1.h>
#include <TAxis.h>
#include <TStyle.h>

#include "RootTreeReader.h"
#include "INIReader.h"
#include "MCANNIEEventReader.h"
#include "TankClusterReader.h"
#include "ANNIEUtility.h"


void fitChargeTrackLength(const INIReader& iniReader);

int main(int argc, char* argv[])
{
   if (argc < 2) {
      std::cerr << "Usage: ./fitChargeTrackLength <config.ini>" << std::endl;
      return 1;
   }

   INIReader iniReader(argv[1]);

   if (iniReader.ParseError() != 0) {
      std::cerr << "Error: could not load config file: "
                << argv[1] << std::endl;
      return 1;
   }

   fitChargeTrackLength(iniReader);

   return 0;
}


void fitChargeTrackLength(const INIReader& iniReader)
{
   std::string inputFileName  = iniReader.Get("input",  "rootFile", "");
   std::string outputFileName = iniReader.Get("output", "rootFile", "");
   std::string outputPdfName  = iniReader.Get("output", "pdfFile",
                                              "TrackLengthVsTotalPE.pdf");

   std::string eventTreeName       = iniReader.Get("input", "eventTreeName", "Event");
   std::string mcTreeName          = iniReader.Get("input", "mcTreeName", "MC");
   std::string tankClusterTreeName = iniReader.Get("input", "tankClusterTreeName",
                                                   "TankCluster");

   if (inputFileName.empty()) {
      std::cerr << "Error: input.rootFile is empty in config." << std::endl;
      return;
   }

   if (outputFileName.empty()) {
      std::cerr << "Error: output.rootFile is empty in config." << std::endl;
      return;
   }

   RootTreeReader treeReader(inputFileName, eventTreeName);

   if (!treeReader.IsValid())
      return;

   TTree* ttree = treeReader.GetTree();

    TankClusterReader tankClusterReader;
    tankClusterReader.Init(ttree);
    
    MCANNIEEventReader eventReader;
    eventReader.Init(ttree);

   Long64_t nEntries = ttree->GetEntries();

   TGraph* gr = new TGraph();
   gr->SetName("grTotalPEVsTrackLength");
   gr->SetTitle("Total PE vs True Track Length in Water;True Track Length in Water [m];Total PE");

   TProfile* prof = new TProfile(
      "pTotalPEVsTrackLength",
      "Mean Total PE vs True Track Length in Water;True Track Length in Water [m];Mean Total PE",
      50, 0, 500
   );

   Long64_t nUsedEvents = 0;

   for (Long64_t i = 0; i < nEntries; ++i) {

      ttree->GetEntry(i);
      

      double trackLength = eventReader.GetTrueTrackLengthInWater(); //return meter 

      if (trackLength <= 0)
         continue;

      double totalPE = ANNIEUtility::GetTotalPE(tankClusterReader, i,0,2000);

      if (totalPE <= 0)
         continue;

      gr->SetPoint(nUsedEvents, trackLength, totalPE);
      prof->Fill(trackLength, totalPE);

      ++nUsedEvents;
   }

   std::cout << "Number of events: " << nEntries << std::endl;
   std::cout << "Events used in fit: " << nUsedEvents << std::endl;

   if (nUsedEvents < 2) {
      std::cerr << "Error: not enough events for fitting." << std::endl;
      return;
   }

   double xmin, ymin, xmax, ymax;
   gr->ComputeRange(xmin, ymin, xmax, ymax);

   TF1* fLinearGraph = new TF1("fLinearGraph", "[0] + [1]*x", xmin, xmax);
   TF1* fLinearProf  = new TF1("fLinearProfile", "[0] + [1]*x", xmin, xmax);

   gr->Fit(fLinearGraph, "R");
   prof->Fit(fLinearProf, "R");

   std::cout << "\nScatter fit:" << std::endl;
   std::cout << "TotalPE = p0 + p1 * TrackLength" << std::endl;
   std::cout << "p0 = " << fLinearGraph->GetParameter(0) << std::endl;
   std::cout << "p1 = " << fLinearGraph->GetParameter(1) << " PE/cm" << std::endl;

   std::cout << "\nProfile fit:" << std::endl;
   std::cout << "p0 = " << fLinearProf->GetParameter(0) << std::endl;
   std::cout << "p1 = " << fLinearProf->GetParameter(1) << " PE/cm" << std::endl;

   TCanvas* c1 = new TCanvas("cTrackLengthVsTotalPEScatter",
                             "Track Length vs Total PE Scatter",
                             900, 700);

   gr->SetMarkerStyle(20);
   gr->SetMarkerSize(0.6);
   gr->Draw("AP");
   fLinearGraph->Draw("same");

   c1->Print((outputPdfName + "(").c_str());

   TCanvas* c2 = new TCanvas("cTrackLengthVsTotalPEProfile",
                             "Track Length vs Total PE Profile",
                             900, 700);

   prof->SetMarkerStyle(20);
   prof->SetMarkerSize(0.8);
   prof->Draw();
   fLinearProf->Draw("same");

   c2->Print((outputPdfName + ")").c_str());

   TFile* outputFile = new TFile(outputFileName.c_str(), "RECREATE");

   if (!outputFile || outputFile->IsZombie()) {
      std::cerr << "Error: could not create output file: "
                << outputFileName << std::endl;
      return;
   }

   outputFile->cd();

   gr->Write();
   prof->Write();
   fLinearGraph->Write();
   fLinearProf->Write();
   c1->Write();
   c2->Write();

   outputFile->Close();

   std::cout << "\nOutput ROOT file: " << outputFileName << std::endl;
   std::cout << "Output PDF file : " << outputPdfName << std::endl;
}

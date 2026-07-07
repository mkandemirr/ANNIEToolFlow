#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include <iostream>
#include <TMath.h>

#include "RootTreeReader.h"
#include "MCANNIEEventReader.h"
#include "INIReader.h"

// -----------------------------------------------------------------------------------

void plotRecoResult(INIReader& iniReader);

// -----------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
   INIReader iniReader(argv[1]);
   plotRecoResult(iniReader);

   return 0;
}

// -----------------------------------------------------------------------------------

void plotRecoResult(INIReader& iniReader)
{
   std::string inputFile =
    iniReader.Get("Input", "InputFile", "");

   std::string outputPdf =
       iniReader.Get("Output", "OutputPdf",
                     "RecoPlots.pdf");
                     
   std::string recoAlgorithm =
    iniReader.Get("Reco", "RecoAlgorithm", "Reco");
                     
                                                
   RootTreeReader treeReader(inputFile.c_str(), "Event");

   if (!treeReader.IsValid())
     return;

   TTree* tree = treeReader.GetTree();

   MCANNIEEventReader mcReader;
   mcReader.Init(tree);

   //meter cinsinden saklanacak
   double RecoX = -9999;
   double RecoY = -9999;
   double RecoZ = -9999;

   double RecoDirX = -9999;
   double RecoDirY = -9999;
   double RecoDirZ = -9999;

   double RecoEnergy = -9999;
   
   

   tree->SetBranchAddress((recoAlgorithm + "X").c_str(),      &RecoX);
   tree->SetBranchAddress((recoAlgorithm + "Y").c_str(),      &RecoY);
   tree->SetBranchAddress((recoAlgorithm + "Z").c_str(),      &RecoZ);

   tree->SetBranchAddress((recoAlgorithm + "DirX").c_str(),   &RecoDirX);
   tree->SetBranchAddress((recoAlgorithm + "DirY").c_str(),   &RecoDirY);
   tree->SetBranchAddress((recoAlgorithm + "DirZ").c_str(),   &RecoDirZ);

   tree->SetBranchAddress((recoAlgorithm + "Energy").c_str(), &RecoEnergy);
 
  

   Long64_t nEntries = tree->GetEntries();

   std::cout << "Number of events in this tree: "
           << nEntries << std::endl;


   TH1D* hVertexDistance = new TH1D(
       "hVertexDistance",
       "Vertex Distance;|Reco Vertex - True Vertex| [m];Events",
       100, 0, 5
   );

   TH1D* hAngle = new TH1D(
       "hRecoTrueAngle",
       "Reco-True Direction Angle;Angle [deg];Events",
       100, 0, 180
   );


   TH1D* hX = new TH1D(
       "hRecoMinusTrueX",
       "RecoX - TrueX;RecoX - TrueX [m];Events",
       100, -5, 5
   );

   TH1D* hY = new TH1D(
       "hRecoMinusTrueY",
       "RecoY - TrueY;RecoY - TrueY [m];Events",
       100, -5, 5
   );

   TH1D* hZ = new TH1D(
       "hRecoMinusTrueZ",
       "RecoZ - TrueZ;RecoZ - TrueZ [m];Events",
       100, -5, 5
   );

   TH1D* hDirX = new TH1D(
       "hRecoMinusTrueDirX",
       "RecoDirX - TrueDirX;RecoDirX - TrueDirX;Events",
       100, -2, 2
   );

   TH1D* hDirY = new TH1D(
       "hRecoMinusTrueDirY",
       "RecoDirY - TrueDirY;RecoDirY - TrueDirY;Events",
       100, -2, 2
   );

   TH1D* hDirZ = new TH1D(
       "hRecoMinusTrueDirZ",
       "RecoDirZ - TrueDirZ;RecoDirZ - TrueDirZ;Events",
       100, -2, 2
   );

   TH1D* hEnergy = new TH1D(
       "hRecoMinusTrueEnergy",
       "RecoEnergy - TrueMuonEnergy;RecoEnergy - TrueMuonEnergy [MeV];Events",
       100, -2000, 2000
   );



   for (Long64_t i = 0; i < nEntries; ++i) {

      tree->GetEntry(i);

     

      // Tank center in global coordinates [cm]
      const double tankCenterX = 0.0;
      const double tankCenterY = -14.4;
      const double tankCenterZ = 168.1;

      // Convert true vertex to global coordinates [cm]
      const double trueVtxX_global = mcReader.GetTrueVtxX() + tankCenterX;
      const double trueVtxY_global = mcReader.GetTrueVtxY() + tankCenterY;
      const double trueVtxZ_global = mcReader.GetTrueVtxZ() + tankCenterZ;

      // Vertex residuals [m]
      const double dx = RecoX - trueVtxX_global * 0.01;
      const double dy = RecoY - trueVtxY_global * 0.01;
      const double dz = RecoZ - trueVtxZ_global * 0.01;

      hX->Fill(dx);
      hY->Fill(dy);
      hZ->Fill(dz);

      // Vertex distance [m]
      const double vertexDistance = std::sqrt(dx*dx + dy*dy + dz*dz);
      hVertexDistance->Fill(vertexDistance);

      // Direction residuals
      hDirX->Fill(RecoDirX - mcReader.GetTrueDirX());
      hDirY->Fill(RecoDirY - mcReader.GetTrueDirY());
      hDirZ->Fill(RecoDirZ - mcReader.GetTrueDirZ());

      // Angle between reco and true direction
      const double trueDirX = mcReader.GetTrueDirX();
      const double trueDirY = mcReader.GetTrueDirY();
      const double trueDirZ = mcReader.GetTrueDirZ();

      const double dot =
        RecoDirX * trueDirX +
        RecoDirY * trueDirY +
        RecoDirZ * trueDirZ;

      const double recoNorm = std::sqrt(
        RecoDirX*RecoDirX +
        RecoDirY*RecoDirY +
        RecoDirZ*RecoDirZ
      );

      const double trueNorm = std::sqrt(
        trueDirX*trueDirX +
        trueDirY*trueDirY +
        trueDirZ*trueDirZ
      );

      if (recoNorm > 0.0 && trueNorm > 0.0) {

        double cosTheta = dot / (recoNorm * trueNorm);

        // Protect against numerical precision errors
        if (cosTheta > 1.0)  cosTheta = 1.0;
        if (cosTheta < -1.0) cosTheta = -1.0;

        const double angleDeg = TMath::ACos(cosTheta) * TMath::RadToDeg();
        hAngle->Fill(angleDeg);
      }

      // Energy residual [MeV]
      hEnergy->Fill(RecoEnergy - mcReader.GetTrueMuonEnergy());
   
   
   }//event loop end

    gStyle->SetOptStat(1110);

    const char* pdfName = outputPdf.c_str();

    TCanvas* c1 = new TCanvas("c1", "Simple Reco Plots", 1200, 800);

    c1->Print(TString::Format("%s[", pdfName));

    c1->Clear();
    hX->SetLineWidth(2);
    hX->Draw("hist");
    c1->Print(pdfName);

    c1->Clear();
    hY->SetLineWidth(2);
    hY->Draw("hist");
    c1->Print(pdfName);

    c1->Clear();
    hZ->SetLineWidth(2);
    hZ->Draw("hist");
    c1->Print(pdfName);

    c1->Clear();
    hDirX->SetLineWidth(2);
    hDirX->Draw("hist");
    c1->Print(pdfName);

    c1->Clear();
    hDirY->SetLineWidth(2);
    hDirY->Draw("hist");
    c1->Print(pdfName);

    c1->Clear();
    hDirZ->SetLineWidth(2);
    hDirZ->Draw("hist");
    c1->Print(pdfName);
    
   c1->Clear();
   hVertexDistance->SetLineWidth(2);
   hVertexDistance->Draw("hist");
   c1->Print(pdfName);

   c1->Clear();
   hAngle->SetLineWidth(2);
   hAngle->Draw("hist");
   c1->Print(pdfName);

    c1->Clear();
    hEnergy->SetLineWidth(2);
    hEnergy->Draw("hist");
    c1->Print(pdfName);

    c1->Print(TString::Format("%s]", pdfName));
    
   std::cout << "----------------------------------" << std::endl;
   std::cout << "plotRecoResult finished" << std::endl;
   std::cout << "InputFile  : " << inputFile << std::endl;
   std::cout << "OutputPdf  : " << outputPdf << std::endl;
   std::cout << "Entries    : " << nEntries << std::endl;
   std::cout << "RecoAlgorithm : " << recoAlgorithm << std::endl;
   std::cout << "----------------------------------" << std::endl;

    

    delete c1;
}

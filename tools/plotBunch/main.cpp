#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TRootCanvas.h>
#include <TApplication.h>

#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
#include "RootTreeReader.h"
#include "TankClusterReader.h"

#include "TStyle.h"
#include <vector>
#include <TSpectrum.h>
#include <TF1.h>
#include "TH2D.h"
#include <TGraph.h>

#include <TLine.h>
#include <TLatex.h>

#include "TLegend.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotBunch(const char* fileName);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv) {

    const char* fileName = "ANNIETree.root";

    if (argc > 1) {
        fileName = argv[1];
    }

    PlotBunch(fileName);

    return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotBunch(const char* fileName) {

    TApplication* app = new TApplication("Root Application", nullptr, nullptr);

    RootTreeReader treeReader(fileName, "Event");
    if (!treeReader.IsValid())
        return;
    TTree* tree = treeReader.GetTree();

    TankClusterReader tankClusterReader;
    tankClusterReader.Init(tree);

    Long64_t nEntries = tree->GetEntries();
    std::cout << "Number of events in this tree: " << nEntries << std::endl;

    double BRFFirstPeakFit;
    tree->SetBranchAddress("BRFFirstPeakFit", &BRFFirstPeakFit);
    double RWMFirstPeak;
    tree->SetBranchAddress("RWMFirstPeak", &RWMFirstPeak);

    // 2 ns bin width: 0–2000 ns
    TH1D* hBestClusterTime = new TH1D(
        "hBestClusterTime",
        "Best cluster time - BRFFirstPeakFit;Time [ns];Counts / 2 ns",
        1000, 0, 2000
    );

    TH1D* hEarliestHitTime = new TH1D(
        "hEarliestHitTime",
        "Earliest hit time in best cluster - BRFFirstPeakFit;Time [ns];Counts / 2 ns",
        1000, 0, 2000
    );
    
    TH1D* hBunchSpacing = new TH1D(
    "hBunchSpacing",
    "Time difference between consecutive bunch peaks;#Delta t [ns];Counts",
    40, 10, 30
   );

   TH1D* hPeakFolding = new TH1D(
       "hPeakFolding",
       "Peak folding;time [ns];Counts / 0.5 ns",
       40, -10, 10
   );

   TH2D* hBunchIdVsPeakTime = new TH2D(
       "hBunchIdVsPeakTime",
       "Bunch ID vs peak time;Bunch number;time [ns]",
       100, 0, 100,
       1000, 0, 2000
   );
   
   TGraph* gBunchSigma = new TGraph();
   gBunchSigma->SetName("gBunchSigma");
   gBunchSigma->SetTitle("Bunch number vs bunch width (sigma);Bunch number;Gaussian sigma [ns]");

   double clusterTimeMin = 0.0;
   double clusterTimeMax = 2000.0;

   for (Long64_t i = 0; i < nEntries; ++i) {

      tree->GetEntry(i);

      ETankClusters eTankClusters;
      tankClusterReader.ReadEntry(i, eTankClusters);

      double maxPENumber = -1.0;
      const TankCluster* bestCluster = nullptr;

      // En yüksek PE'ye sahip prompt cluster'ı bul
      for (const TankCluster& cl : eTankClusters.Get()) {

         double clusterTime = cl.GetClusterTime();

         if (clusterTime > clusterTimeMin && clusterTime < clusterTimeMax) {

             if (cl.GetMaxPE() > maxPENumber) {
                 maxPENumber = cl.GetMaxPE();
                 bestCluster = &cl;
             }
         }
      }

      // Eğer uygun cluster yoksa geç
      if (bestCluster == nullptr) {
         eTankClusters.Clear();
         continue;
      }

      // 1) Best cluster time
      double bestClusterTime = bestCluster->GetClusterTime();
      double t0 = BRFFirstPeakFit * 0.001; // ps -> ns varsayımı
      hBestClusterTime->Fill(bestClusterTime - t0);

      // 2) Best cluster içindeki en erken hit zamanı
      double earliestHitTime = 1e9;

      for (const PMTHit& hit : bestCluster->GetHits()) {
         if (hit.GetT() < earliestHitTime) {
             earliestHitTime = hit.GetT();
         }
      }

      if (earliestHitTime < 1e9) {
         hEarliestHitTime->Fill(earliestHitTime - t0);
      }

      eTankClusters.Clear();
   
   }//event loop




   // -------------------------
   // Find bunch peaks with TSpectrum
   // -------------------------
   TH1D* selectedHist = hBestClusterTime;
  
   // TSpectrum(maxPeaks)
   TSpectrum spectrum(200); // maximum number of peaks to search

   int nPeaks = spectrum.Search(
       selectedHist,   // histogram to search
       1.2,            // expected peak width (sigma in bins)
       "nobackground", // do not subtract background
       0.3             // threshold = 30% of highest peak
   );

   double* peakX = spectrum.GetPositionX();

   std::vector<double> peaks;
   double beamWindowMin = 200;
   double beamWindowMax = 1750;

   for (int i = 0; i < nPeaks; ++i) {
       double x = peakX[i];

       if (x > beamWindowMin && x < beamWindowMax) {
           peaks.push_back(x);
       }
   }

   std::sort(peaks.begin(), peaks.end());


   // -------------------------
   // Refine bunch peaks with Gaussian fits
   // -------------------------
   double fitWindow = 6.0; // Gaussian fit half-range in ns
   std::vector<double> fittedPeaks;

   for (size_t i = 0; i < peaks.size(); ++i) {

       double peak = peaks[i];

       TF1* fit = new TF1(
           Form("fit_bunch_%zu", i),
           "gaus",
           peak - fitWindow,
           peak + fitWindow
       );

       selectedHist->Fit(fit, "RQ+");

       double fittedPeak = fit->GetParameter(1);
       double sigma      = fit->GetParameter(2);

       fittedPeaks.push_back(fittedPeak);

       gBunchSigma->SetPoint(i, i, sigma);

      
       std::cout << "Bunch " << i
                 << " TSpectrum peak = " << peak
                 << " fitted peak = " << fittedPeak
                 << " sigma = " << sigma << " ns"
                 << std::endl;
   }


   // -------------------------
   // Calculate bunch spacing
   // -------------------------
   std::sort(fittedPeaks.begin(), fittedPeaks.end());

   for (size_t i = 0; i < fittedPeaks.size(); ++i) {
       hBunchIdVsPeakTime->Fill(i, fittedPeaks[i]);

       if (i > 0) {
           double dt = fittedPeaks[i] - fittedPeaks[i - 1];

           hBunchSpacing->Fill(dt);
           //std::cout << "bunch space: " << dt << std::endl;
       }
   }

   double bunchSpacing = hBunchSpacing->GetMean();

   std::cout << "Number of bunch peaks = " << fittedPeaks.size() << std::endl;
   std::cout << "Mean bunch spacing = " << bunchSpacing << " ns" << std::endl;


   // -------------------------
   // Peak folding using fitted bunch spacing
   // -------------------------
   for (int bin = 1; bin <= selectedHist->GetNbinsX(); ++bin) {

       double x = selectedHist->GetBinCenter(bin);
       double y = selectedHist->GetBinContent(bin);

       if (x < beamWindowMin || x > beamWindowMax)
           continue;

       double folded = std::fmod(x - fittedPeaks.front(), bunchSpacing);

       if (folded > bunchSpacing / 2.0) {
           folded -= bunchSpacing;
       }

       if (folded < -bunchSpacing / 2.0) {
           folded += bunchSpacing;
       }

       hPeakFolding->Fill(folded, y);
   }


   // -----------------------------
   // ROOT style settings
   // -----------------------------
   gStyle->SetLineScalePS(0.7);
   gStyle->SetOptStat(1);

   TCanvas* c1 = new TCanvas("c1", "BNB bunch structure", 1600, 400);

   const char* pdfName = "PMTBunchPlots.pdf";

   // PDF aç
   c1->Print((std::string(pdfName) + "[").c_str());


   // -------------------------
   // Page 1: best cluster time
   // -------------------------
   c1->Clear();

   //hBestClusterTime->SetLineColor(kRed);
   hBestClusterTime->SetLineWidth(1);
   hBestClusterTime->SetStats(1);
   hBestClusterTime->Draw();

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);


   // -------------------------
   // Page 2: earliest hit time
   // -------------------------
   c1->Clear();

   hEarliestHitTime->SetLineColor(kBlue);
   hEarliestHitTime->SetLineWidth(1);
   hEarliestHitTime->SetStats(1);
   hEarliestHitTime->Draw();

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);


   // -------------------------
   // Page 3: zoomed earliest hit time
   // -------------------------
   c1->Clear();

   TH1D* hZoom = (TH1D*)hBestClusterTime->Clone("hZoom");
   hZoom->GetXaxis()->SetRangeUser(940, 1050);

   hZoom->SetTitle("Best cluster time - BRF (zoom: 940-1050 ns);Time [ns];Counts / 2 ns");
   hZoom->SetLineColor(kBlue);
   hZoom->SetLineWidth(1);
   hZoom->SetStats(1);

   hZoom->Draw("hist");

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);



   // -------------------------
   // Page 4: Bunch ID vs peak time
   // -------------------------
   /*
   c1->Clear();

   hBunchIdVsPeakTime->SetStats(1);
   hBunchIdVsPeakTime->Draw("COLZ");

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);
   */

   // -------------------------
   // Page 5: bunch spacing
   // -------------------------
   c1->Clear();

   hBunchSpacing->SetLineColor(kBlue);
   hBunchSpacing->SetLineWidth(2);
   hBunchSpacing->SetStats(1);
   hBunchSpacing->Draw("hist");

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);

   
   // -------------------------
   // Page 6: bunch sigma
   // -------------------------
   c1->Clear();

   gBunchSigma->SetMarkerStyle(20);
   gBunchSigma->SetMarkerSize(0.8);
   gBunchSigma->SetLineColor(kBlue);
   gBunchSigma->Draw("APL");

   TLegend* legend = new TLegend(0.80, 0.8, 0.92, 0.88);

   legend->AddEntry(
       gBunchSigma,
       Form("Found bunches = %zu", peaks.size()),
       "p"
   );

   legend->AddEntry(
       (TObject*)0,
       Form("Mean sigma = %.2f ns", gBunchSigma->GetMean(2)),
       "p"
   );

   legend->Draw();

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);


   // -------------------------
   // Page 7: peak folding
   // -------------------------
   c1->Clear();

   hPeakFolding->SetLineColor(kBlue);
   hPeakFolding->SetLineWidth(2);
   hPeakFolding->SetStats(1);
   hPeakFolding->Draw("hist");

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);






   // PDF kapat
   c1->Print((std::string(pdfName) + "]").c_str());

   c1->Update();

   //hBestClusterTime->Draw();

   TRootCanvas* rc = (TRootCanvas*)c1->GetCanvasImp();
   rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");

   app->Run();
}

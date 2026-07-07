#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>      // std::isfinite

#include "ANNIEEventReader.h"
#include "LAPPDHitReader.h"
#include "ELAPPDHits.h"
#include "TPaveText.h"
#include "RootTreeReader.h"


#include <TRootCanvas.h>
#include <TApplication.h>

#include <map>
#include <string>

// --------------------------------------------------------------------------------------

void PlotLAPPDHit(const char* fileName);


// --------------------------------------------------------------------------------------
int main(int argc, char* argv[]) 
{
    const char* fileName = "ANNIETree.root"; // default file name
    if (argc > 1) {
        fileName = argv[1];
    }

    PlotLAPPDHit(fileName);
    return 0;
}


// --------------------------------------------------------------------------------------

void PlotLAPPDHit(const char* fileName) 
{
    TApplication *app = new TApplication("Root Application", nullptr, nullptr);

    RootTreeReader treeReader(fileName, "Event");

    // Check if file/tree is valid
    if (!treeReader.IsValid())
        return;

    TTree* tree = treeReader.GetTree(); 

    ANNIEEventReader eventReader;
    eventReader.Init(tree);
   
    LAPPDHitReader hitReader;
    hitReader.Init(tree);

    // -----------------------------------------------------------------------------------
    // Histograms:
    //  - hHitTimeTotal  : total time spectrum (all LAPPDs)
    //  - tubeTimeHists  : time spectrum per tubeID (LAPPD)
    //  - tubeHitCounts  : total hit counts per tubeID
    // -----------------------------------------------------------------------------------
    TH1D* hHitTimeTotal = new TH1D("hHitTimeTotal",
                                   "LAPPD Hit Time;Time [ns];Counts/1 ns",
                                   300, 0, 300);
    hHitTimeTotal->SetLineWidth(2);

    std::map<int, TH1D*> tubeTimeHists;        // tubeID -> time histogram
    std::map<int, long long> tubeHitCounts;    // tubeID -> total hit count

    // -----------------------------------------------------------------------------------
    // Loop over events
    // -----------------------------------------------------------------------------------
    Long64_t nEntries = tree->GetEntries();
    std::cout << "Number of events in this tree: " << nEntries << std::endl;

    int countZeroTime = 0;
    for (Long64_t i = 0; i < nEntries; ++i) 
    {
        tree->GetEntry(i);

        // örnek cut: sadece True CC olaylar
        //if (!eventReader.GetTrueCC())
        //    continue;

        ELAPPDHits eHits;
        hitReader.ReadEntry(i, eHits);

        
        // Loop over individual hits in this event
        for (const LAPPDHit& hit : eHits.GetHits())
        {
            double t = hit.GetTime();     // hit time
            int    lappdID = hit.GetLAPPDID();
            
           // if (t<0)
           //    std::cout<<"negatif: "<<t<<std::endl;
            
            if (t < 0)
               countZeroTime++;

            // time kesiti, NaN vs. at
           // if (!std::isfinite(t) || t <= 0.) continue;

            // Total zaman histogramı
            hHitTimeTotal->Fill(t);

            // Hit sayısını tut
            tubeHitCounts[lappdID]++;

            // Bu tubeID için zaman histogramı yoksa oluştur
            TH1D*& hTubeTime = tubeTimeHists[lappdID];
            if (!hTubeTime) {
                TString hname  = Form("hHitTime_LAPPD_%d", lappdID);
                TString htitle = Form("LAPPD Hit Time - Tube %d;Time [ns];Counts/10 ns", lappdID);
                hTubeTime = new TH1D(hname, htitle, 600, 0, 300.0);
                hTubeTime->SetLineWidth(2);
            }
            hTubeTime->Fill(t);
        }
    }



   std::cout<<"zero time count: "<<countZeroTime<<std::endl;

    // -----------------------------------------------------------------------------------
    // TubeID histogram: her LAPPD (tubeID) ne kadar hit görmüş?
    // -----------------------------------------------------------------------------------
    TH1D* hTubeCounts = nullptr;
    if (!tubeHitCounts.empty()) {
        int nLAPPD = (int)tubeHitCounts.size();
        hTubeCounts = new TH1D("hTubeCounts",
                               "LAPPD Hit Counts;LAPPD (Tube ID);Entries",
                               nLAPPD, 0.5, nLAPPD + 0.5);

        int bin = 0;
        for (const auto& kv : tubeHitCounts) {
            ++bin;
            int lappdID = kv.first;
            long long count = kv.second;
            hTubeCounts->SetBinContent(bin, (double)count);
            hTubeCounts->GetXaxis()->SetBinLabel(bin, Form("%d", lappdID));
        }
        hTubeCounts->SetStats(false);
        hTubeCounts->SetLineWidth(2);
    }

    // -----------------------------------------------------------------------------------
    // THStack: her LAPPD için zaman histogramı + legend
    // -----------------------------------------------------------------------------------
    THStack* hsTime = new THStack("hsTime", "LAPPD Hit Time by Tube ID ;Time [ns];Counts/10 ns");
    TLegend* leg = new TLegend(0.70, 0.70, 0.90, 0.90);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);

    int color = 1;
    for (auto& kv : tubeTimeHists) {
        int lappdID = kv.first;
        TH1D* h = kv.second;
        h->SetLineColor(color);
        // İstersen setFillColorAlpha kullanabilirsin, istersen sadece çizgi:
        // h->SetFillColorAlpha(color, 0.3);
        hsTime->Add(h, "HIST");
        leg->AddEntry(h, Form("Tube %d", lappdID), "l");
        ++color;
        if (color == 10) color = 1; // ROOT default renkleri döndür
    }

   
   
   //if (hTubeCounts) hTubeCounts->Draw("HIST");
    
   
   
   
   hsTime->Draw("NOSTACK HIST");
   leg->Draw();
   
   
   //hHitTimeTotal->Draw("HIST");
   
   
   int lastBin = hHitTimeTotal->FindLastBinAbove(0);
   double maxTime = hHitTimeTotal->GetXaxis()->GetBinUpEdge(lastBin);

   std::cout << "Max hit time (from filled bins) = "
             << maxTime << " ns" << std::endl;

  

    // -----------------------------------------------------------------------------------
    // Pencereyi kapatınca uygulamayı sonlandır
    // -----------------------------------------------------------------------------------
    TRootCanvas *rc = (TRootCanvas *)gPad->GetCanvas()->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app->Run();
}


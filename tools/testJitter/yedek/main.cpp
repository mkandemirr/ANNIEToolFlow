#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"

#include <map>
#include "THStack.h"
#include "TLegend.h"
#include "TString.h"


#include <cstdint>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <iomanip>



#include "ANNIEEventReader.h"
#include "LAPPDMetaReader.h"


#include <TRootCanvas.h>
#include <TApplication.h>
#include "TPaveText.h"

// -----------------------------
// Helpers
// -----------------------------
static inline __int128 to_i128(uint64_t x) { return static_cast<__int128>(x); }
static inline __int128 to_i128(int64_t  x) { return static_cast<__int128>(x); }
static inline __int128 to_i128(int  x) { return static_cast<__int128>(x); }
// 1 tick = 3.125 ns = 3125 ps (EXACT)
static inline __int128 ticks_to_ps_i128(__int128 ticks) {
  return ticks * 3125;
}

// BG_unix(ps) = (BG_raw + BG_corr)*3125ps + Offset_ns*1000 - OSInMinusPS_ps
static __int128 compute_BG_unix_ps(
    uint64_t BG_raw,
    int  BG_corr,
    uint64_t Offset_ns,
    int OSInMinusPS_ps
) {
  __int128 ticks = to_i128(BG_raw) + to_i128(BG_corr) ;
  __int128 ps_from_ticks = ticks_to_ps_i128(ticks);
  __int128 ps_offset = to_i128(Offset_ns) * 1000; // ns -> ps
  __int128 ps_os = to_i128(OSInMinusPS_ps);
  return ps_from_ticks + ps_offset - ps_os;
}

// -----------------------------
// Main logic
// -----------------------------
void testJitter(const char* filename);

int main(int argc, char* argv[]) {
  const char* filename = "ANNIETree.root";
  if (argc > 1) filename = argv[1];
  testJitter(filename);
  return 0;
}

void testJitter(const char* filename) {

  TApplication *app = new TApplication("Root Application", nullptr, nullptr);

  // -------------------------
  // Open input
  // -------------------------
  TFile* file = TFile::Open(filename);
  if (!file || file->IsZombie()) {
    std::cerr << "Error opening file: " << filename << "\n";
    return;
  }

  TTree* tree = dynamic_cast<TTree*>(file->Get("Event"));
  if (!tree) {
    std::cerr << "Error: Tree 'Event' not found in file!\n";
    file->Close();
    delete file;
    return;
  }

  // -------------------------
  // Init readers
  // -------------------------
  ANNIEEventReader eventReader;
  eventReader.Init(tree);

  LAPPDMetaReader metaReader;
  metaReader.Init(tree);


  const Long64_t nEntries = tree->GetEntries();
  std::cout << "Number of events in this tree: " << nEntries << "\n";


   double brfFirstPeakFit = 0.0;
   if (tree->GetBranch("BRFFirstPeakFit")) {
     tree->SetBranchAddress("BRFFirstPeakFit", &brfFirstPeakFit);
   } else {
     std::cerr << "Warning: Branch 'BRFFirstPeakFit' not found!\n";
   }



   //BRF-UBT
   TH1D* hTankMinusBRF = new TH1D(
    "hTankMinusBRF",
    "eventTimeTank + BRFFirstPeakFit - UBT - 335455 ;  #Delta t (ns); Counts",
    300, -10.0, 20.0
   );

  
    // -------------------------
   // Histogram: (eventTimeTank - primaryTriggerTime)
   // -------------------------
   TH1D* h_eventTimeTank_minus_UBT = new TH1D(
       "h_eventTimeTank_minus_UBT",
       "eventTimeTank - UBT - 335455;#Delta t (ns);Counts",
       1000, -500, 500
   );
  
   // -------------------------
   // Histogram: BG correction values
   // -------------------------
   TH1D* h_bg_correction = new TH1D(
       "h_bg_correction",
       "LAPPD BG correction values;BG correction (ticks);Counts",
       501, -250.5, 250.5   // genelde küçük integer değerler için uygun
   );


   /////////////////////yeniler

   // -------------------------
   // Histogram: ( BG_unix - UBT(primaryTriggerTime) )
   // LAPPD histogramları: ID'ye göre + hepsi için
   // -------------------------
   
   std::map<int, TH1D*> h_bg_minus_UBT__byID;

   //for all lappds
   TH1D* h_bg_minus_UBT__all = new TH1D(
       "h_bg_minus_UBT__all",
       "ALL LAPPDs: BG_{unix} - UBT_{unix} - 325250;#Delta t (ns);Counts",
       400, -300, 100
   );

   
  // -------------------------
  // Event loop
  // -------------------------
  Long64_t firstEvent = 0;
  //Long64_t lastEvent  = std::min<Long64_t>(500, nEntries);

   for (Long64_t i = firstEvent; i < nEntries; ++i) {
      tree->GetEntry(i);

      // ---- Event trigger info ----
      int primaryTriggerWord = eventReader.GetPrimaryTriggerWord();  //UBT

      const std::vector<uint64_t>& groupedTriggerTime =
        *(eventReader.GetGroupedTriggerTime()); // genelde ns
      const std::vector<uint32_t>& groupedTriggerWord =
        *(eventReader.GetGroupedTriggerWord());

      // 1) primary trigger time'ı (UBT) bul (word eşleşmesiyle)
      bool foundPrimary = false;
      uint64_t primaryTriggerTime_ns = 0;  //UBT

      for (size_t k = 0; k < groupedTriggerWord.size(); ++k) {
         if ((int)groupedTriggerWord[k] == primaryTriggerWord) {
           primaryTriggerTime_ns = groupedTriggerTime[k];
           foundPrimary = true;
           break;
         }
      }

      if (!foundPrimary) continue;

      // primary trigger time -> ps (exact)
      __int128 primaryTriggerTime_ps = to_i128(primaryTriggerTime_ns) * 1000;

      // ---- LAPPD meta ----
      int numberOfActiveLAPPD = metaReader.mLAPPD_ID->size();
      if (numberOfActiveLAPPD <= 0) continue;

      const std::vector<uint64_t> bg_raw   = *(metaReader.mLAPPD_Beamgate_Raw);
      const std::vector<int>  bg_corr      = *(metaReader.mLAPPD_BGCorrection);
      const std::vector<uint64_t> offsets  = *(metaReader.mLAPPD_Offset);
      const std::vector<int> os_ps         = *(metaReader.mLAPPD_OSInMinusPS);
      const std::vector<int> lappd_ids     = *(metaReader.mLAPPD_ID);


      // eventTimeTank (ns) → ps
      //bu aslinda ULong64_t (root). (uint64_t bunun c++ karsiligi)
      uint64_t eventTimeTank_ns = eventReader.GetEventTimeTank(); 
      __int128 eventTimeTank_ps = to_i128(eventTimeTank_ns) * 1000;

      // Δt = eventTimeTank - primaryTriggerTime (ps)
      __int128 dt_eventTimeTank_ps = eventTimeTank_ps - primaryTriggerTime_ps;


      // fill histogram in ns
      double dt_eventTimeTank_ns = (double)dt_eventTimeTank_ps / 1000.0;
      dt_eventTimeTank_ns = dt_eventTimeTank_ns - 335350.;


      //For PMTs
      h_eventTimeTank_minus_UBT->Fill(dt_eventTimeTank_ns);

      hTankMinusBRF->Fill( dt_eventTimeTank_ns + brfFirstPeakFit/1000.); 




      if (!metaReader.mLAPPD_ID ||
      !metaReader.mLAPPD_Beamgate_Raw ||
      !metaReader.mLAPPD_BGCorrection ||
      !metaReader.mLAPPD_Offset ||
      !metaReader.mLAPPD_OSInMinusPS) {
         // Bu event/dosyada LAPPD meta yok
         //throw std::runtime_error("asdsd");
         continue;
      }




      //loop over all lappds
      for (int j = 0; j < numberOfActiveLAPPD; ++j) {

         int lappdID = lappd_ids.at(j);

         h_bg_correction->Fill(bg_corr.at(j));

         __int128 bg_unix_ps = compute_BG_unix_ps(
            bg_raw.at(j),
            bg_corr.at(j),
            offsets.at(j),
            os_ps.at(j)
         );

         __int128 dt_ps = bg_unix_ps - primaryTriggerTime_ps;

         double dt_ns = (double)dt_ps / 1000.0;
         dt_ns -= 325250.0;

         // 1) ALL histogram
         h_bg_minus_UBT__all->Fill(dt_ns);



         // 2) ID'ye göre histogram: yoksa oluştur
         TH1D*& hID = h_bg_minus_UBT__byID[lappdID];  // <-- TH1D*& tam burada çok işe yarıyor
         if (!hID) {
          TString name  = Form("h_dt_bg_minus_primary_id%d", lappdID);
          TString title = Form("LAPPD %d: BG_{unix} - UBT_{unix} - 325250;#Delta t (ns);Counts", lappdID);

          hID = new TH1D(name, title, 400, -300, 100);

          // İstersen çizimde daha okunur olsun diye:
          hID->SetStats(0);
         }

         hID->Fill(dt_ns);



      }//lappd loop

    
    
   
  } //event loop end
  
  
  
   // Stack (ID histogramları bunun içine girecek)
   THStack* st_bg_minus_UBT = new THStack(
       "st_bg_minus_UBT",
       "BG_{unix} - UBT_{unix} - 325250;#Delta t (ns);Counts"
   );

   TLegend* leg = new TLegend(0.65, 0.65, 0.88, 0.88);
  
  
  // -------------------------
   // THStack'e ID histogramlarını ekle
   // -------------------------
   int color = 1; // ROOT color index (1=black)
   for (auto& kv : h_bg_minus_UBT__byID) {
      int id = kv.first;
      TH1D* h = kv.second;
      if (!h) continue;

      // 0 ve 10 gibi görünmeyen/garip renkleri atlayalım
      color++;
      if (color == 10) color++;
      if (color == 0)  color = 1;

      h->SetLineColor(color);
      h->SetMarkerColor(color);
      h->SetLineWidth(2);

      st_bg_minus_UBT->Add(h);
      leg->AddEntry(h, Form("LAPPD %d", id), "l");
   }

   
   //Add the historam which include all lappds 
   // ALL histogram stil
   h_bg_minus_UBT__all->SetLineColor(kBlack);
   h_bg_minus_UBT__all->SetLineWidth(3);
   h_bg_minus_UBT__all->SetStats(0);
   st_bg_minus_UBT->Add(h_bg_minus_UBT__all);
   leg->AddEntry(h_bg_minus_UBT__all, "ALL", "l");


   // -------------------------
   // Draw
   // -------------------------
   
   //1. draw by lappdID
   //st_bg_minus_UBT->Draw("nostack hist");
   //leg->Draw();


   //2. 
    //h_eventTimeTank_minus_UBT->Draw();
  
   //3.for all lappds
   //h_bg_correction->Draw();
   
   //4.
  hTankMinusBRF->Draw();
  
  
  
  // -----------------------------------------------------------------------------------
    // Pencereyi kapatınca uygulamayı sonlandır
    // -----------------------------------------------------------------------------------
    TRootCanvas *rc = (TRootCanvas *)gPad->GetCanvas()->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app->Run();

  //file->Close();
  //delete file;
}

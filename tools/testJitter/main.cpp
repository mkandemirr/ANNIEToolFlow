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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

static __int128 compute_TS_unix_ps(
    uint64_t TS_raw,
    int  TS_corr,
    uint64_t Offset_ns,
    int OSInMinusPS_ps
) {
    __int128 ticks = to_i128(TS_raw) + to_i128(TS_corr);
    __int128 ps_from_ticks = ticks_to_ps_i128(ticks);
    __int128 ps_offset = to_i128(Offset_ns) * 1000; // ns → ps
    __int128 ps_os = to_i128(OSInMinusPS_ps);

    return ps_from_ticks + ps_offset - ps_os;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

uint64_t getUBT(const ANNIEEventReader& eventReader, bool& foundUBT);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void testJitter(const char* filename);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char* argv[]) {
  const char* filename = "ANNIETree.root";
  if (argc > 1) filename = argv[1];
  testJitter(filename);
  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

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


   //1.
   TH1D* hTankTimeMinusUBT = new TH1D(
       "hTankTimeMinusUBT",
       "eventTimeTank_{unix} - UBT_{unix} - 335455;#Delta t (ns);Counts",
       1000, -2500, 2500
   );
   
   //2.
   TH1D* hBRFTimeMinusUBT = new TH1D(
    "hBRFTimeMinusUBT",
    "BRF_{unix} - UBT_{unix} - 335350 ;  #Delta t (ns); Counts",
    300, -10.0, 20.0
   );
   
   
   // 3. LAPPD histogramları: ID'ye göre + hepsi için
   std::map<int, TH1D*> hBGMinusUBTByID;

   //for all lappds
   TH1D* hBGMinusUBTAll = new TH1D(
       "hBGMinusUBTAll",
       "ALL LAPPDs: BG_{unix} - UBT_{unix} - 325250;#Delta t (ns);Counts",
       400, -300, 100
   );

   // 4.
   TH1D* hLAPPDTSMinusBG = new TH1D(
    "hLAPPDTSMinusBG",
    "LAPPDTS_{unix} - LAPPDBG_{unix};#Delta t (ns);Counts",
    1000, 0, 20000);
   
   // 5.
   TH1D* hLAPPDTSMinusBRFTime = new TH1D(
    "hLAPPDTSMinusBRFTime",
    "LAPPDTS_{unix} - BRF_{unix} + 10205;#Delta t (ns);Counts",
    1000, 0, 20000
   );


   //not used
   TH1D* hDeltaUBT = new TH1D(
    "hDeltaUBT",
    "Time difference between consecutive UBTs(15MHz-67ms);#Delta UBT (ms);Counts",
    1000, 0, 1200
   );
   
   //not used
   TH1D* hDeltaUBTZoom = new TH1D(
    "hDeltaUBTZoom",
    "Zoom near beam frequency peak;#Delta UBT (ms);Counts",
    3000, 66, 68
   );
   
  
  



   
  // -------------------------
  // Event loop
  // -------------------------
  Long64_t firstEvent = 0;
  //Long64_t lastEvent  = std::min<Long64_t>(500, nEntries);
  
   bool hasPreviousUBT = false;
   uint64_t previousUBT_ns = 0;

   for (Long64_t i = firstEvent; i < nEntries; ++i) 
   {
      tree->GetEntry(i);

      //1.
      bool foundUBT = false;
      uint64_t UBT_ns = getUBT(eventReader, foundUBT);
      __int128 UBT_ps = to_i128(UBT_ns) * 1000;
      if (!foundUBT) continue;
      
      
      // Fill consecutive UBT time difference histogram in seconds
      if (hasPreviousUBT) 
      {
          double deltaUBT_ms =
              static_cast<double>(UBT_ns - previousUBT_ns) * 1e-6;

          hDeltaUBT->Fill(deltaUBT_ms);
          hDeltaUBTZoom->Fill(deltaUBT_ms);
      }

      previousUBT_ns = UBT_ns;
      hasPreviousUBT = true;
      
      
       // 2.
      //bu aslinda ULong64_t (root). (uint64_t bunun c++ karsiligi)
      uint64_t eventTimeTank_ns = eventReader.GetEventTimeTank(); 
      __int128 eventTimeTank_ps = to_i128(eventTimeTank_ns) * 1000;

      __int128 dt_tankTimeMinusUBT_ps = eventTimeTank_ps - UBT_ps;

      double dt_tankTimeMinusUBT_ns = (double)dt_tankTimeMinusUBT_ps / 1000.0;
      

      //For PMTs
      hTankTimeMinusUBT->Fill( dt_tankTimeMinusUBT_ns - 335455 );

      hBRFTimeMinusUBT->Fill( dt_tankTimeMinusUBT_ns - 335350 + brfFirstPeakFit/1000. ); 
 
      
      //3.
      // ---- LAPPD meta ----
      int numberOfActiveLAPPD = metaReader.mLAPPD_ID->size();
      if (numberOfActiveLAPPD <= 0) continue;


      //Timestamp   
      const std::vector<uint64_t> bg_raw   = *(metaReader.mLAPPD_Beamgate_Raw);
      const std::vector<int>  bg_corr      = *(metaReader.mLAPPD_BGCorrection);
      
      const std::vector<uint64_t> ts_raw    = *(metaReader.mLAPPD_Timestamp_Raw);
      const std::vector<int> ts_corr        = *(metaReader.mLAPPD_TSCorrection); 
      
      const std::vector<uint64_t> offsets   = *(metaReader.mLAPPD_Offset);
      const std::vector<int> os_InMinus_ps         = *(metaReader.mLAPPD_OSInMinusPS);
      const std::vector<int> lappd_ids      = *(metaReader.mLAPPD_ID);


      if (!metaReader.mLAPPD_ID ||
      !metaReader.mLAPPD_Beamgate_Raw ||
      !metaReader.mLAPPD_BGCorrection ||
      !metaReader.mLAPPD_Offset ||
      !metaReader.mLAPPD_OSInMinusPS) {
         throw std::runtime_error("mo meta data");
      }


      //loop over all lappds
      for (int j = 0; j < numberOfActiveLAPPD; ++j) {

         int lappdID = lappd_ids.at(j);

         __int128 bg_unix_ps = compute_BG_unix_ps(
            bg_raw.at(j),
            bg_corr.at(j),
            offsets.at(j),
            os_InMinus_ps.at(j)
         );
         
         __int128 ts_unix_ps = compute_TS_unix_ps(
              ts_raw.at(j),
              ts_corr.at(j),
              offsets.at(j),
              os_InMinus_ps.at(j)
          );
          
         
         //4.
         // First subtract large timestamps
         __int128 dt_TSMinusTankTime_ps = ts_unix_ps - eventTimeTank_ps ;
         double dt_TSMinusTankTime_ns = (double)dt_TSMinusTankTime_ps / 1000.0;

         //if(dt_TSMinusTankTime_ns < 2000  )
         hLAPPDTSMinusBRFTime->Fill(dt_TSMinusTankTime_ns - (brfFirstPeakFit / 1000.0) + 10205);
          
          
         //5. 
        __int128 dt_TSMinusBG_ps = ts_unix_ps - bg_unix_ps ;
         double dt_TSMinusBG_ns = (double)dt_TSMinusBG_ps / 1000.0;  
          
         hLAPPDTSMinusBG->Fill(dt_TSMinusBG_ns); 
          
          
         //6.
         __int128 dt_BGMinusUBT_ps = bg_unix_ps - UBT_ps;
         double dt_BGMinusUBT_ns = (double)dt_BGMinusUBT_ps / 1000.0;
   
         hBGMinusUBTAll->Fill(dt_BGMinusUBT_ns - 325250 );

         // ID'ye göre histogram: yoksa oluştur
         TH1D*& hID = hBGMinusUBTByID[lappdID];  // <-- TH1D*& tam burada çok işe yarıyor
         if (!hID) {
          TString name  = Form("h_dt_bg_minus_primary_id%d", lappdID);
          TString title = Form("LAPPD %d: BG_{unix} - UBT_{unix} - 325250;#Delta t (ns);Counts", lappdID);

          hID = new TH1D(name, title, 400, -300, 100);

          // İstersen çizimde daha okunur olsun diye:
          hID->SetStats(0);
         }

         hID->Fill(dt_BGMinusUBT_ns - 325250);



      }//lappd loop

    
    
   
  } //event loop end
  
  
  
   // Stack (ID histogramları bunun içine girecek)
   THStack* stBGMinusUBT = new THStack(
       "stBGMinusUBT",
       "BG_{unix} - UBT_{unix} - 325250;#Delta t (ns);Counts"
   );

   TLegend* leg = new TLegend(0.65, 0.65, 0.88, 0.88);
  
  
  // -------------------------
   // THStack'e ID histogramlarını ekle
   // -------------------------
   int color = 1; // ROOT color index (1=black)
   for (auto& kv : hBGMinusUBTByID) {
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

      stBGMinusUBT->Add(h);
      leg->AddEntry(h, Form("LAPPD %d", id), "l");
   }

   
   //Add the historam which include all lappds 
   // ALL histogram stil
   hBGMinusUBTAll->SetLineColor(kBlack);
   hBGMinusUBTAll->SetLineWidth(3);
   hBGMinusUBTAll->SetStats(0);
   stBGMinusUBT->Add(hBGMinusUBTAll);
   leg->AddEntry(hBGMinusUBTAll, "ALL", "l");

   // -------------------------
   // Print all histograms to PDF
   // -------------------------
   TCanvas* c1 = new TCanvas("c1", "Histograms", 1200, 800);

   TString pdfName = "TimingHistograms.pdf";

   // Open multi-page PDF
   c1->Print(pdfName + "[");

   // Page 1
   c1->Clear();
   hTankTimeMinusUBT->Draw("hist");
   c1->Print(pdfName);

   // Page 2
   c1->Clear();
   hBRFTimeMinusUBT->Draw("hist");
   c1->Print(pdfName);

   // Page 3
   c1->Clear();
   stBGMinusUBT->Draw("nostack hist");
   leg->Draw();
   c1->Print(pdfName);

   // Page 4
   c1->Clear();
   hLAPPDTSMinusBG->Draw("hist");
   c1->Print(pdfName);

   // Page 5
   c1->Clear();
   hLAPPDTSMinusBRFTime->Draw("hist");
   c1->Print(pdfName);
   
   // Page 6 : full range
   //c1->Clear();
   //hDeltaUBT->Draw("hist");
   //c1->Print(pdfName);

   // Page 7 : zoomed range
   //c1->Clear();
   //hDeltaUBTZoom->Draw("hist");
   //c1->Print(pdfName);

   // Close multi-page PDF
   c1->Print(pdfName + "]");

   // -------------------------
   // Show only ONE histogram on screen
   // -------------------------
   c1->Clear();

   // Burada ekranda görmek istediğin tek histogramı seç
   //stBGMinusUBT->Draw("nostack hist");
   //leg->Draw();

   // hTankTimeMinusUBT->Draw("hist");
   // hBRFTimeMinusUBT->Draw("hist");
   // hDeltaUBT->Draw("hist");
   // hLAPPDTSMinusBRFTime->Draw("hist");
   //hLAPPDTSMinusBG->Draw("hist");
   hDeltaUBT->Draw();
   
   c1->Update();

   TRootCanvas *rc = (TRootCanvas*)c1->GetCanvasImp();
   rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
   app->Run();
   
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

uint64_t getUBT(const ANNIEEventReader& eventReader, bool& foundUBT)
{
    // ---- Event trigger info ----
    int primaryTriggerWord = eventReader.GetPrimaryTriggerWord();

    const std::vector<uint64_t>& groupedTriggerTime =
        *(eventReader.GetGroupedTriggerTime());

    const std::vector<uint32_t>& groupedTriggerWord =
        *(eventReader.GetGroupedTriggerWord());

    foundUBT = false;

    // Find the primary trigger time (UBT)
    for (size_t k = 0; k < groupedTriggerWord.size(); ++k) {

        if ((int)groupedTriggerWord[k] == primaryTriggerWord) {

            foundUBT = true;
            return groupedTriggerTime[k];
        }
    }

    return 0;
}

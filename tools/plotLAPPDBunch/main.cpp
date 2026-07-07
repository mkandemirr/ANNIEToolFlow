#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <algorithm>
#include <iostream>
#include "ANNIEEventReader.h"
#include "LAPPDMetaReader.h"
#include "TankClusterReader.h"
#include "MRDClusterReader.h"
#include <iomanip>
#include "TPaveText.h"
#include "RootTreeReader.h"
#include <TRootCanvas.h>
#include <TApplication.h>

#include "LAPPDPulse.h"
#include "LAPPDPulseReader.h"

#include "LAPPDHit.h"
#include "LAPPDHitReader.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "TStyle.h"

#include <TSpectrum.h>
#include <TF1.h>
#include "TH2D.h"

#include <TGraph.h>

#include <TLine.h>
#include <TLatex.h>

#include "TLegend.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotTSMinusBeamgate(const char* fileName);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
                        
std::string int128_to_string(__int128 value);


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

static inline __int128 to_i128(uint64_t x) { return static_cast<__int128>(x); }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

static inline __int128 to_i128(int64_t  x) { return static_cast<__int128>(x); }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

static inline __int128 to_i128(int  x) { return static_cast<__int128>(x); }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

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

int main(int argc, char* argv[]) 
{
    const char* fileName = "ANNIETree.root"; //default
    if (argc > 1) {
      fileName = argv[1];
    }

    PlotTSMinusBeamgate(fileName);
    return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotTSMinusBeamgate(const char* fileName) 
{

   TApplication *app = new TApplication("Root Application", nullptr, nullptr);
    
   RootTreeReader treeReader(fileName,"Event");
   
   if(!treeReader.IsValid())
      return;
   
   TTree* tree = treeReader.GetTree(); 
   
   
   // Initialize readers
   ANNIEEventReader eventReader;
   eventReader.Init(tree);
   
   // Initialize readers
   LAPPDMetaReader metaReader; 
   metaReader.Init(tree);
   
   LAPPDPulseReader pulseReader;
   pulseReader.Init(tree);
   
   LAPPDHitReader hitReader;
   hitReader.Init(tree);

   double BRFFirstPeakFit;
   tree->SetBranchAddress("BRFFirstPeakFit", &BRFFirstPeakFit);
   
   // Get entry count
   Long64_t nEntries = tree->GetEntries();
   std::cout << "Number of events in this tree: " << nEntries << std::endl;

   // Create histograms
   
   //1
   TH1D* hTSMinusBG = new TH1D(
    "hTSMinusBG",
    "LAPPDTS_{unix} - LAPPDBG_{unix};Time [ns];Counts / 3.125 ns",
    672, 7500, 9600 //2100/672=3.125 bin sayisini artirinca(orn 700) bazi binlerde event olmuyor! 
   );

   //2.
   TH1D* hTSMinusBRF = new TH1D(
       "hTSMinusBRF",
       "LAPPDTS_{unix} - BRF_{unix} + 10205;Time [ns];Counts / 3 ns",
       700, 7500, 9600
   );
   
   //3.
   TH1D* hEarliestHitMinusBRF = new TH1D(
    "hEarliestHitMinusBRF",
    "LAPPDTS_{unix} + LocalHitTime - BRF_{unix} + 10205 ;Time [ns];Counts/ 3ns",
    700, 7500, 9600
   );
    
  //4.  
  TH1D* hBunchSpacing = new TH1D(
    "hBunchSpacing",
    "LAPPD bunch spacing;#Delta t [ns];Counts",
    40, 10, 30
   );

   //5.
   TH1D* hPeakFolding = new TH1D(
       "hPeakFolding",
       "LAPPD peak folding;time [ns];entries / 0.5 ns",
       40, -10, 10
   );

   //6.
   TH2D* hBunchIdVsPeakTime = new TH2D(
       "hBunchIdVsPeakTime",
       "LAPPD bunch ID vs peak time;bunch index;time [ns]",
       100, 0, 100,
       1000, 7500, 9600   // senin LAPPD range'e uygun
   );
   
   TGraph* gBunchSigma = new TGraph();
   gBunchSigma->SetName("gBunchSigma");
   gBunchSigma->SetTitle("Bunch number vs bunch width (sigma);Bunch number;Gaussian sigma [ns]");
 
   
   std::map<int, long long> lappdCounts;   // ID -> kaç kez görüldü

   int selectedLAPPDID = -1;  // -1 means all active
   
   
   //nEntries = 100;
   // Loop over entries and fill
   for (Long64_t i = 0; i < nEntries; ++i) 
   {
      //std::cout<<"EVENT "<<i<<std::endl;
      
      tree->GetEntry(i);
      
      /*
      // ----------------------------------------------------
      // Find primary trigger time = UBT time
      // ----------------------------------------------------
      int primaryTriggerWord = eventReader.GetPrimaryTriggerWord();

      const std::vector<uint64_t>& groupedTriggerTime =
          *(eventReader.GetGroupedTriggerTime());

      const std::vector<uint32_t>& groupedTriggerWord =
          *(eventReader.GetGroupedTriggerWord());

      bool foundPrimary = false;
      uint64_t primaryTriggerTime_ns = 0;

      for (size_t k = 0; k < groupedTriggerWord.size(); ++k) {
          if ((int)groupedTriggerWord[k] == primaryTriggerWord) {
              primaryTriggerTime_ns = groupedTriggerTime[k];
              foundPrimary = true;
              break;
          }
      }

      if (!foundPrimary)
          continue;
     
      __int128 primaryTriggerTime_ps = to_i128(primaryTriggerTime_ns) * 1000;      
     */ 
      
      // count the IDs before skipping any events.
      for (int id : *metaReader.mLAPPD_ID)
         lappdCounts[id]++;
      
      
      //Skipped events 2
      //it works only if one LAPPD is active
      if (selectedLAPPDID != -1) //-1 means all active
      {   
         //Check the selected LAPPD exist in this event, if not skip
         auto ite = std::find(metaReader.mLAPPD_ID->begin(), 
                              metaReader.mLAPPD_ID->end(), 
                              selectedLAPPDID);
                             
         bool notFound = ( ite == metaReader.mLAPPD_ID->end() );

         
         if (notFound) continue;
         //throw std::runtime_error("selected id not foundQ");
      }
      
                  
      //------------------------------------------------------------------------
 
 
      //LAPPD meta reader 
      const std::vector<int> lappd_ids      = *(metaReader.mLAPPD_ID); 
      const std::vector<uint64_t> offset    = *(metaReader.mLAPPD_Offset);
      const std::vector<int> os_InMinus_ps  = *(metaReader.mLAPPD_OSInMinusPS);
       
      //TS   
      const std::vector<uint64_t> ts_raw    = *(metaReader.mLAPPD_Timestamp_Raw);
      const std::vector<int> ts_corr        = *(metaReader.mLAPPD_TSCorrection); 
      //Beamgate  
      const std::vector<uint64_t> bg_raw    = *(metaReader.mLAPPD_Beamgate_Raw);
      const std::vector<int>  bg_corr       = *(metaReader.mLAPPD_BGCorrection);


      { //check vector size mismatch
         const size_t n_offset   = offset.size();
         const size_t n_os       = os_InMinus_ps.size();
         const size_t n_ids      = lappd_ids.size();
         const size_t n_ts_raw   = ts_raw.size();
         const size_t n_ts_corr  = ts_corr.size();
         const size_t n_bg_raw   = bg_raw.size();
         const size_t n_bg_corr  = bg_corr.size();

         // Hepsi eşit mi?
         if (!(n_offset == n_os &&
               n_offset == n_ids &&
               n_offset == n_ts_raw &&
               n_offset == n_ts_corr &&
               n_offset == n_bg_raw &&
               n_offset == n_bg_corr))
         {
           std::ostringstream oss;
           oss << "LAPPD metadata vector sizes are NOT equal!\n"
               << "offset.size()      = " << n_offset  << "\n"
               << "os_InMinus_ps.size() = " << n_os      << "\n"
               << "lappd_ids.size()    = " << n_ids     << "\n"
               << "ts_raw.size()       = " << n_ts_raw  << "\n"
               << "ts_corr.size()      = " << n_ts_corr << "\n"
               << "bg_raw.size()       = " << n_bg_raw  << "\n"
               << "bg_corr.size()      = " << n_bg_corr << "\n";

           throw std::runtime_error(oss.str());
         }

      }
 
      int numberOfActiveLAPPPD = metaReader.mLAPPD_ID->size();  
              
      double BRFFirstPeakFit_ns = BRFFirstPeakFit * 0.001; // ps -> ns varsayımı
      
      ELAPPDHits eLAPPDHits;
      hitReader.ReadEntry(i, eLAPPDHits);
      
      //earliest pulse time
      ELAPPDPulses eLAPPDPulses;
      pulseReader.ReadEntry(i, eLAPPDPulses);
      
      
      uint64_t eventTimeTank_ns = eventReader.GetEventTimeTank(); 
      __int128 eventTimeTank_ps = to_i128(eventTimeTank_ns) * 1000;
      
   
      std::unordered_map<int, double> lappd_dt_bg_map;
      std::unordered_map<int, double> lappd_dt_tank_map;

      for (size_t j = 0; j < numberOfActiveLAPPPD; ++j)
      {
          int id = lappd_ids.at(j);
          
          if (selectedLAPPDID != -1 && id != selectedLAPPDID)
            continue;

          __int128 ts_unix_ps = compute_TS_unix_ps(
              ts_raw.at(j),
              ts_corr.at(j),
              offset.at(j),
              os_InMinus_ps.at(j)
          );

          __int128 bg_unix_ps = compute_BG_unix_ps(
              bg_raw.at(j),
              bg_corr.at(j),
              offset.at(j),
              os_InMinus_ps.at(j)
          );


         __int128 tsMinusBG_ps = ts_unix_ps - bg_unix_ps;
         double tsMinusBG_ns =
             static_cast<double>(tsMinusBG_ps) / 1000.0;

         __int128 tsMinusTankTime_ps = ts_unix_ps - eventTimeTank_ps;
         double tsMinusTankTime_ns =
             static_cast<double>(tsMinusTankTime_ps) / 1000.0;

         lappd_dt_bg_map[id] = tsMinusBG_ns;
         lappd_dt_tank_map[id] = tsMinusTankTime_ns;
         
         hTSMinusBG->Fill(tsMinusBG_ns);

         hTSMinusBRF->Fill(tsMinusTankTime_ns - BRFFirstPeakFit_ns + 10205);  
         
         
      }
      
      
      
      double earliestHitTankBRF = 1e9;

      for (const LAPPDHit& hit : eLAPPDHits.GetHits())
      {
          int id = hit.GetLAPPDID();

          auto itTank = lappd_dt_tank_map.find(id);
          if (itTank == lappd_dt_tank_map.end()) continue;

          double tsMinusTank_ns = itTank->second;

          // direct hit time
          double hitTimeLocal = hit.GetTime()*0.1;

          double hitTime = tsMinusTank_ns + hitTimeLocal - BRFFirstPeakFit_ns;

          if (hitTime < earliestHitTankBRF)
              earliestHitTankBRF = hitTime;
      }
      
      if (earliestHitTankBRF < 1e9)
         hEarliestHitMinusBRF->Fill(earliestHitTankBRF + 10205);
      
       
   
   
      //std::cout<<"End of event "<<i<<std::endl;
   } //end of event loop
   


   // -------------------------
   // Find bunch peaks with TSpectrum
   // -------------------------
   TH1D* selectedHist = hTSMinusBRF;
   TSpectrum spectrum(200); // max number of peaks

   int nPeaks = spectrum.Search(
       selectedHist,
       1.2,
       "nobackground",
       0.3
   );

   double* peakX = spectrum.GetPositionX();

   std::vector<double> peaks;
   
   double beamWindowMin = 7800;
   double beamWindowMax = 9480;
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
   double fitWindow = 8.0; // Gaussian fit half-range in ns
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


   std::cout << "==== LAPPD ID counters ====\n";
   for (const auto& [id, cnt] : lappdCounts) {
       double ratio = (nEntries > 0) ? (static_cast<double>(cnt) / nEntries) * 100.0 : 0.0;
       std::cout << "ID " << id << " : " << cnt << " , ratio = " << ratio << "%\n";
   }

   
   std::cout << "Selected LAPPD ID: " << selectedLAPPDID <<std::endl;
   
  

   // -----------------------------
   // ROOT style settings
   // -----------------------------
   gStyle->SetLineScalePS(0.7);
   gStyle->SetOptStat(1);

  
   // -----------------------------------------------------------------------------------
   // Canvas
   // -----------------------------------------------------------------------------------
   TCanvas* c1 = new TCanvas("c1","LAPPD timing",1600,400);
 //gStyle->SetLineScalePS(0.7);
   // -----------------------------------------------------------------------------------
   // PDF file name
   // -----------------------------------------------------------------------------------
   const char* pdfName = "LAPPDPlots.pdf";

   // -----------------------------------------------------------------------------------
   // Open multi-page PDF
   // -----------------------------------------------------------------------------------
   c1->Print(TString::Format("%s[", pdfName));

   // -----------------------------------------------------------------------------------
   // PAGE 1
   // -----------------------------------------------------------------------------------
   c1->Clear();

   hTSMinusBG->SetLineWidth(1);
   hTSMinusBG->SetStats(1);
   hTSMinusBG->Draw();

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);

   // -----------------------------------------------------------------------------------
   // PAGE 2
   // -----------------------------------------------------------------------------------
  

   // -----------------------------------------------------------------------------------
   // PAGE 3
   // -----------------------------------------------------------------------------------
   c1->Clear();

   hTSMinusBRF->SetLineWidth(1);
   hTSMinusBRF->SetStats(1);
   hTSMinusBRF->Draw();

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);
   
   // -----------------------------
   // PAGE 4 - Earliest Hit Time
   // -----------------------------
   c1->Clear();

   hEarliestHitMinusBRF->SetLineWidth(1);
   hEarliestHitMinusBRF->SetStats(1);
   hEarliestHitMinusBRF->Draw();

   c1->Modified();
   c1->Update();
   c1->Print(pdfName);
   
   //page 5
   //c1->Clear();

   //hBunchIdVsPeakTime->SetStats(0);
   //hBunchIdVsPeakTime->Draw("COLZ");

   //c1->Print(pdfName);

   //page6
   c1->Clear();

   hBunchSpacing->SetLineWidth(2);
   hBunchSpacing->Draw("hist");

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
   

   //page 7
   c1->Clear();
   c1->SetCanvasSize(1200, 800);

   hPeakFolding->SetLineWidth(2);
   hPeakFolding->Draw("hist");

   c1->Print(pdfName);

   // -----------------------------------------------------------------------------------
   // Close PDF
   // -----------------------------------------------------------------------------------
   c1->Print(TString::Format("%s]", pdfName));

   // -----------------------------------------------------------------------------------
   // Interactive display
   // -----------------------------------------------------------------------------------
   c1->Clear();

   //hTSMinusBG->Draw();

   //hist2->Draw();
   hTSMinusBRF->Draw();

   c1->Modified();
   c1->Update();


   
   // -----------------------------------------------------------------------------------
    // Pencereyi kapatınca uygulamayı sonlandır
    // -----------------------------------------------------------------------------------
    TRootCanvas *rc = (TRootCanvas *)gPad->GetCanvas()->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app->Run();
 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......






// __int128 tipi C++'ta cout ile dogrudan yazdirilamaz.
// Bu fonksiyon, __int128 sayiyi string'e cevirerek
// guvenli ve kayipsiz sekilde yazdirmayi saglar.
std::string int128_to_string(__int128 value) {

    // Eger sayi 0 ise, direkt "0" dondur.
    // (donguye girmeye gerek yok)
    if (value == 0)
        return "0";

    // Sayinin negatif olup olmadigini kontrol et
    bool negative = value < 0;

    // Negatifse, sayiyi pozitif yap.
    // Basamak ayirma islemini sadece pozitif sayilarla yapiyoruz.
    if (negative)
        value = -value;

    // Rakamlari biriktirmek icin string
    std::string result;

    // Sayi sifir olana kadar devam et
    while (value > 0) {

        // value % 10 :
        // Sayinin EN SAGDAKI basamagini verir
        // Ornek: 12345 % 10 = 5
        int digit = value % 10;

        // '0' + digit :
        // Sayisal rakami karaktere cevirir
        // Ornek: '0' + 5 -> '5'
        result.push_back('0' + digit);

        // value /= 10 :
        // Tam sayi bolmesi yapilir
        // Ondalik kisim atilir
        // Ornek: 12345 / 10 = 1234
        // Yani son basamak SILINMIS olur
        value /= 10;
    }

    // Eger sayi baslangicta negatifse,
    // en sona '-' karakterini ekle
    // (hala ters sirada oldugunu unutma)
    if (negative)
        result.push_back('-');

    // Rakamlar tersten eklendigi icin
    // string'i ters ceviriyoruz
    std::reverse(result.begin(), result.end());

    // Son haliyle string'i dondur
    return result;
}


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

#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>


#include "INIReader.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotCutFlowHists(INIReader& iniReader);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void GenerateTable(TH1D* hist, double& signal_counts, 
                        double& estimated_bck_in_signal, 
                        double& SoverB);

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
   INIReader iniReader(argv[1]);
   PlotCutFlowHists(iniReader);

   return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotCutFlowHists(INIReader& iniReader) 
{

   std::string inputFile = iniReader.Get("Input", "InputFile", "");
   std::string outputPdf = iniReader.Get("Output", "OutputPdf", "CutFlowHist.pdf");

   TApplication *app = new TApplication("Root Application", nullptr, nullptr);
    
   RootTreeReader treeReader(inputFile.c_str(), "Event");
   
   if(!treeReader.IsValid())
      return;
   
   TTree* tree = treeReader.GetTree(); 
   
   int passBeamOK                = -1;
   int passBRFWindow             = -1;
   int passMissingPPSTick        = -1;
   int passRequireLAPPD          = -1;
   int passPairedEvent           = -1;
   int passPromptPMTCluster      = -1;
   int passHighQualityPMTCluster = -1;
   int passMRDTrackReconstructed = -1;
   int passTankMRDCoinc          = -1;
   int passMuonTopology          = -1;
   int passNoVeto                = -1;

   tree->SetBranchAddress("passBeamOK",                &passBeamOK);
   tree->SetBranchAddress("passBRFWindow",             &passBRFWindow);
   tree->SetBranchAddress("passMissingPPSTick",        &passMissingPPSTick);
   tree->SetBranchAddress("passRequireLAPPD",          &passRequireLAPPD);
   tree->SetBranchAddress("passPairedEvent",           &passPairedEvent);
   tree->SetBranchAddress("passPromptPMTCluster",      &passPromptPMTCluster);
   tree->SetBranchAddress("passHighQualityPMTCluster", &passHighQualityPMTCluster);
   tree->SetBranchAddress("passMRDTrackReconstructed", &passMRDTrackReconstructed);
   tree->SetBranchAddress("passTankMRDCoinc",          &passTankMRDCoinc);
   tree->SetBranchAddress("passMuonTopology",          &passMuonTopology);
   tree->SetBranchAddress("passNoVeto",                &passNoVeto);
   
   
   // Initialize readers
   LAPPDMetaReader metaReader; 
   metaReader.Init(tree);

   
   // Get entry count
   Long64_t nEntries = tree->GetEntries();
   std::cout << "Number of events in this tree: " << nEntries << std::endl;

   // Create histograms
   TH1D* h0   = new TH1D("Paired", "", 160, 0, 20000);
   TH1D* h1   = new TH1D("Any PMT cluster", "",   160, 0, 20000);
   TH1D* h2   = new TH1D("High-quality PMT cluster", "",   160, 0, 20000);
   TH1D* h3   = new TH1D("MRD track reconstructed", "",   160, 0, 20000);
   TH1D* h4   = new TH1D("In-time MRD coincidence", "",   160, 0, 20000);
   TH1D* h5   = new TH1D("Muon topology cut", "",   160, 0, 20000);
   TH1D* h6   = new TH1D("No hit in FMV", "",   160, 0, 20000);
   
   std::map<int, long long> lappdCounts;   // ID -> kaç kez görüldü

   int selectedLAPPDID = -1;  // -1 means all active
   
   //nEntries = 100;
   // Loop over entries and fill
   for (Long64_t i = 0; i < nEntries; ++i) 
   {
      //std::cout<<"EVENT "<<i<<std::endl;
      
      tree->GetEntry(i);
      
      
      // count the IDs before skipping any events.
      for (int id : *metaReader.mLAPPD_ID)
         lappdCounts[id]++;
      
      
      if (passBeamOK == 0)
         continue;

      if (passMissingPPSTick == 0)
         continue;
         
      if (passBRFWindow == 0)
         continue;
         
      if (passRequireLAPPD == 0)
         continue;
      
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
       
      //Timestamp   
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
      
      for (size_t j = 0; j < numberOfActiveLAPPPD; ++j) // returns over all LAPPDs in this event
      {
      
         //int id = metaReader.mLAPPD_ID->at(j);
         //lappdCounts[id]++;   // otomatik: yeni ID ise map'te yaratır ve 1 yapar
     
         
         __int128 ts_unix_ps   = compute_TS_unix_ps(ts_raw.at(j),
                                                    ts_corr.at(j),
                                                    offset.at(j),
                                                    os_InMinus_ps.at(j)
                                                    );
   
   
         __int128 bg_unix_ps = compute_BG_unix_ps(bg_raw.at(j),
                                                bg_corr.at(j),
                                                offset.at(j),
                                                os_InMinus_ps.at(j)
                                                );
         
         
         __int128 dt_ps =  ts_unix_ps - bg_unix_ps;

         double dt_ns   = static_cast<double>(dt_ps) / 1000.0;
         
         
         /*
         //for debugging   
         std::cout<<"ts_unix_ps: "<<int128_to_string(ts_unix_ps)<<std::endl;
         std::cout<<"bg_unix_ps: "<<int128_to_string(bg_unix_ps)<<std::endl;
         std::cout<<"dt_ps: "<<int128_to_string(dt_ps)<<std::endl;
         std::cout<<"dt_ns: "<<dt_ns<<std::endl;
         */
         
         /*
         //this cut significantly decreases the number of pairs!!
         std::vector<uint64_t> ts_ppsBefore = *(metaReader.mLAPPD_TSPPSBefore);
         int pps_dt = double( int64_t(ts_raw[j]) - int64_t(ts_ppsBefore[j]) );
         if( pps_dt > 3.4e9 || pps_dt < 0 )
            continue;
         */

      
         // Fill histograms
         if (passPairedEvent == 1)
         {
            h0->Fill(dt_ns);

            if (passPromptPMTCluster == 1)
            {
               h1->Fill(dt_ns);

               if (passHighQualityPMTCluster == 1)
               {
                  h2->Fill(dt_ns);

                  if (passMRDTrackReconstructed == 1)
                  {
                     h3->Fill(dt_ns);

                     if (passTankMRDCoinc == 1)
                     {
                        h4->Fill(dt_ns);

                        if (passMuonTopology == 1)
                        {
                           h5->Fill(dt_ns);

                           if (passNoVeto == 1)
                           {
                              h6->Fill(dt_ns);
                           }
                        }
                     }
                  }
               }
            }
         }
                 
          
      } //lappd number loop
   
   
      //std::cout<<"End of event "<<i<<std::endl;
   } //end of event loop
   
   
   std::cout << "==== Histograms entries ==== " << std::endl;
   std::cout << "h0 entries: " << h0->GetEntries() << std::endl;
   std::cout << "h1 entries: " << h1->GetEntries() << std::endl;
   std::cout << "h2 entries: " << h2->GetEntries() << std::endl;
   std::cout << "h3 entries: " << h3->GetEntries() << std::endl;
   std::cout << "h4 entries: " << h4->GetEntries() << std::endl;
   std::cout << "h5 entries: " << h5->GetEntries() << std::endl;
   std::cout << "h6 entries: " << h6->GetEntries() << std::endl;
   



   std::cout << "==== LAPPD ID counters ====\n";
   for (const auto& [id, cnt] : lappdCounts) {
       double ratio = (nEntries > 0) ? (static_cast<double>(cnt) / nEntries) * 100.0 : 0.0;
       std::cout << "ID " << id << " : " << cnt << " , ratio = " << ratio << "%\n";
   }

   
   std::cout << "Selected LAPPD ID: " << selectedLAPPDID <<std::endl;
   
   // --------------------------------------------------
   // Stack 1: early cut levels
   // --------------------------------------------------
   TCanvas* cAll = new TCanvas("cAll", "Basic Cut Stack", 1000, 700);

   THStack* stackAll = new THStack("stackAll", "");

   h0->SetLineColor(kBlack);  h0->SetLineWidth(2);
   h1->SetLineColor(kRed);    h1->SetLineWidth(2);
   h2->SetLineColor(kBlue);   h2->SetLineWidth(2);

   stackAll->Add(h0);
   stackAll->Add(h1);
   stackAll->Add(h2);

   stackAll->Draw("nostack hist");
   stackAll->GetXaxis()->SetTitle("#Delta t [ns]");
   stackAll->GetYaxis()->SetTitle("Counts/125 ns");

   TLegend* legAll = new TLegend(0.60, 0.60, 0.88, 0.88);
   legAll->AddEntry(h0, "Paired event", "l");
   legAll->AddEntry(h1, "Any PMT cluster", "l");
   legAll->AddEntry(h2, "High-quality PMT cluster", "l");
   legAll->Draw();

   cAll->Print((outputPdf + "[").c_str());
   cAll->Print(outputPdf.c_str());


   // --------------------------------------------------
   // Stack 2: later cut levels
   // --------------------------------------------------
   TCanvas* cCuts = new TCanvas("cCuts", "Final Cut Stack", 1000, 700);

   THStack* stackCuts = new THStack("stackCuts", "");

   h2->SetLineColor(kBlue);     h2->SetLineWidth(2);
   h3->SetLineColor(kGreen);    h3->SetLineWidth(2);
   h4->SetLineColor(kYellow);   h4->SetLineWidth(2);
   h5->SetLineColor(kGray);     h5->SetLineWidth(2);
   h6->SetLineColor(kMagenta);  h6->SetLineWidth(2);

   stackCuts->Add(h2);
   stackCuts->Add(h3);
   stackCuts->Add(h4);
   stackCuts->Add(h5);
   stackCuts->Add(h6);

   stackCuts->Draw("nostack hist");
   stackCuts->GetXaxis()->SetTitle("#Delta t [ns]");
   stackCuts->GetYaxis()->SetTitle("Counts/125 ns");

   TLegend* legCuts = new TLegend(0.60, 0.60, 0.88, 0.88);
   legCuts->AddEntry(h2, "High-quality PMT cluster", "l");
   legCuts->AddEntry(h3, "MRD track reconstructed", "l");
   legCuts->AddEntry(h4, "In-time MRD coincidence", "l");
   legCuts->AddEntry(h5, "Muon topology", "l");
   legCuts->AddEntry(h6, "No hit in FMV", "l");
   legCuts->Draw();

   cCuts->Print(outputPdf.c_str());
   cCuts->Print((outputPdf + "]").c_str());

   std::cout << "PDF created: " << outputPdf << std::endl;
   
     
   // Değerleri tutmak için değişkenler
   double signal_counts1, bck1, ratio1;
   double signal_counts2, bck2, ratio2;
   double signal_counts3, bck3, ratio3;
   double signal_counts4, bck4, ratio4;
   double signal_counts5, bck5, ratio5;
   double signal_counts6, bck6, ratio6;
   double signal_counts7, bck7, ratio7;

   // Hesapla
   GenerateTable(h0, signal_counts1, bck1, ratio1);
   GenerateTable(h1, signal_counts2, bck2, ratio2);
   GenerateTable(h2, signal_counts3, bck3, ratio3);
   GenerateTable(h3, signal_counts4, bck4, ratio4);
   GenerateTable(h4, signal_counts5, bck5, ratio5);
   GenerateTable(h5, signal_counts6, bck6, ratio6);
   GenerateTable(h6, signal_counts7, bck7, ratio7);

   

   
   // -----------------------------------------------------------------------------------
    // Pencereyi kapatınca uygulamayı sonlandır
    // -----------------------------------------------------------------------------------
    TRootCanvas *rc = (TRootCanvas *)gPad->GetCanvas()->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app->Run();
 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void GenerateTable
(
   TH1D* hist, 
   double& signal_counts, 
   double& estimated_bck_in_signal, 
   double& SoverB)
{
   // Define signal and background regions (in ns)
   double signal_min = 7500;
   double signal_max = 9600;

   //region1   
   double bck1_min = 3000;
   double bck1_max = 7000;

   //region2
   double bck2_min = 11000;
   double bck2_max = 15000;

   // Convert ns to bin numbers
   int bin_signal_min = hist->FindBin(signal_min);
   int bin_signal_max = hist->FindBin(signal_max);

   int bin_bck1_min = hist->FindBin(bck1_min);
   int bin_bck1_max = hist->FindBin(bck1_max);

   int bin_bck2_min = hist->FindBin(bck2_min);
   int bin_bck2_max = hist->FindBin(bck2_max);

   // Integrate signal and background
   signal_counts = hist->Integral(bin_signal_min, bin_signal_max);

   double bck1_counts = hist->Integral(bin_bck1_min, bin_bck1_max);
   double bck2_counts = hist->Integral(bin_bck2_min, bin_bck2_max);

   // Calculate background per bin
   int bck1_bins = bin_bck1_max - bin_bck1_min + 1;
   int bck2_bins = bin_bck2_max - bin_bck2_min + 1;
   double bck_avg_per_bin = (bck1_counts + bck2_counts) / (bck1_bins + bck2_bins);

   // Estimate total background in signal region
   int signal_bins = bin_signal_max - bin_signal_min + 1;
   estimated_bck_in_signal = bck_avg_per_bin * signal_bins;

   // Compute S/B
   double estimatedSignal = signal_counts - estimated_bck_in_signal;
   double B = estimated_bck_in_signal;
   SoverB = (B > 0) ? estimatedSignal / B : -1;

   
   double events_20us   = hist->Integral(0,20000);
   double events_1_6us  = signal_counts;
   double back_events   = estimated_bck_in_signal;
   double beam_events   = estimatedSignal;
   double events_purity = (beam_events / events_1_6us) * 100;
   double netSoverB     = (back_events > 0) ? beam_events / back_events : -1;


   // -------------------
   // ERROR CALCULATION
   // -------------------

   // total background counts in sidebands
   double bck_total_counts = bck1_counts + bck2_counts;

   // Poisson error on background per bin
   double bck_err_per_bin = (bck_total_counts > 0)
       ? std::sqrt(bck_total_counts) / (bck1_bins + bck2_bins)
       : 0.0;

   // Background error in signal window
   double back_events_err = bck_err_per_bin * signal_bins;

   // Net beam events error (only background-driven)
   double beam_events_err = back_events_err;

   // Purity error
   double purity_err = (events_1_6us > 0)
       ? (beam_events_err / events_1_6us) * 100.0
       : 0.0;



////////////////////////////////////////////////////////////////////////////////

   const int cutWidth = 30;
   const int eventsWidth = 23;
   const int backEventsWidth = 25;
   const int beamEventsWidth = 25;
   const int SoverBWidth = 20;
   const int purityWidth = 15;

   if (std::string(hist->GetName()) == "Paired") {
       // Başlık satırı
       std::cout << std::left << std::setw(cutWidth) << "Cut"
                 << std::right << std::setw(eventsWidth) << "Events [20 us]"
                 << std::setw(eventsWidth) << "Events [1.6 us]"
                 << std::setw(backEventsWidth) << "Back Events (est.)"
                 << std::setw(beamEventsWidth) << "Net Beam Events (est.)"
                 << std::setw(SoverBWidth) << "NetBeamE/BackG"
                 << std::setw(purityWidth) << "Purity (%)"
                 << std::endl;

       // Alt çizgi
       std::cout << std::string(cutWidth + 2 * eventsWidth + backEventsWidth + 
       beamEventsWidth + SoverBWidth + purityWidth, '-') << std::endl;
   }

   // Veri satırı
   std::cout << std::left << std::setw(cutWidth) << hist->GetName()
             << std::right << std::setw(eventsWidth) << events_20us
             << std::setw(eventsWidth) << events_1_6us
             //<< std::setw(backEventsWidth) << back_events
             << std::setw(backEventsWidth) << Form("%.2f ± %.2f", back_events, back_events_err)
             << std::setw(beamEventsWidth)<< Form("%.2f ± %.2f", beam_events, beam_events_err)
             << std::setw(SoverBWidth) << netSoverB
             << std::setw(purityWidth)<< Form("%.2f ± %.2f", events_purity, purity_err)
             << std::endl;



   /*
    std::cout << std::left << std::setw(cutWidth) << "Any PMT cluster"
              << std::right << std::setw(eventsWidth) << pmtcluster_20us
              << std::setw(eventsWidth) << pmtcluster_1_6us
              << std::setw(beamEventsWidth) << pmtcluster_beam_est
              << std::setw(purityWidth) << pmtcluster_purity
              << std::endl;
   */
   
   

   //std::cout << "Signal counts: " << signal_counts << std::endl;
   //std::cout << "Estimated background in signal window: " << estimated_bck_in_signal << std::endl;
   //std::cout << "S/B ratio: " << SoverB << std::endl;
}




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


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
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void ReadChannels(const std::string& filename,
                  std::vector<int>& verticalChannels,
                  std::vector<int>& horizontalChannels);


//Functions used
void PlotTimestampMinusBeamgate(const char* fileName);
std::string GetBaseName(const std::string& fileName);
bool IsHighQualityPMTCluster(const TankCluster*);

void GenerateTable(TH1D* hist, double& signal_counts, 
                        double& estimated_bck_in_signal, 
                        double& SoverB);
                        
std::string int128_to_string(__int128 value);



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

    PlotTimestampMinusBeamgate(fileName);
    return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PlotTimestampMinusBeamgate(const char* fileName) 
{

   TApplication *app = new TApplication("Root Application", nullptr, nullptr);
    
   RootTreeReader treeReader(fileName,"Event");
   
   if(!treeReader.IsValid())
      return;
   
   TTree* tree = treeReader.GetTree(); 
   
   
   int beam_ok;
   tree->SetBranchAddress("beam_ok", &beam_ok);
   
   double BRFFirstPeakFit;
   tree->SetBranchAddress("BRFFirstPeakFit", &BRFFirstPeakFit);

   // Initialize readers
   LAPPDMetaReader metaReader; 
   metaReader.Init(tree);

   ANNIEEventReader eventReader; 
   eventReader.Init(tree);

   TankClusterReader tankClusterReader;
   tankClusterReader.Init(tree);
   
   MRDClusterReader mrdClusterReader;
   mrdClusterReader.Init(tree);


   //For MRDtrackReconstructed cut
   std::vector<int> sideMRDPMTs;
   std::vector<int> topMRDPMTs;
   ReadChannels("MRDInfo.txt", sideMRDPMTs, topMRDPMTs);

      //std::cout << "Vertical size: " << sideMRDPMTs.size() << std::endl;
      //std::cout << "Horizontal size: " << topMRDPMTs.size() << std::endl;


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
      
      //Skipped events 1 
      //just to increase speed. no impact 
      if (metaReader.mLAPPD_ID->size() == 0) //no lappd meta data
      {
         //throw std::runtime_error("mLAPPD_ID->size() returned to zero");
         continue; 
      }
      
      
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
      
                  
      //Skipped events 3
      if (!beam_ok)
         continue;      
        
      /*
      //Skipped events 4
      if ( (BRFFirstPeakFit/1000.) < 5 or (BRFFirstPeakFit/1000. ) > 30 )
      {
         //throw std::runtime_error("BRF passed");
         continue;
         
      }
      */
         
       
      //Skipped events 5
      // 3. PPS missing check. if there is a missing PPS, it will be 1 in the vector. All should be zero!
      bool hasPPSMissing = false;

      for (size_t i = 0; i < metaReader.mLAPPD_TSPPSMissing->size(); ++i) {
          if (metaReader.mLAPPD_TSPPSMissing->at(i) != 0) {
              hasPPSMissing = true;
              break;  // ilk bulduğunda çık (daha hızlı)
          }
      }

      if (hasPPSMissing) {
          continue;
      }
      
      
      
      //------------------------------------------------------------------------
      // The belows are Event-level cuts
      ETankClusters eTankClusters;
      tankClusterReader.ReadEntry(i, eTankClusters);

      EMRDClusters eMRDClusters;
      mrdClusterReader.ReadEntry(i, eMRDClusters);
            
      // Cut 0: Paired
      bool isPairedEvent = false;
      if (eventReader.HasLAPPD() == 1 && 
          eventReader.HasMRD()   == 1   && 
          eventReader.HasTank()  == 1 )
      {
         
         isPairedEvent = true;
      }
         
      
      // Cut 1: Any PMT cluster 
      // Find the cluster with the maximum charge (PE) in the initial time window
      bool hasClusterInPromptWindow = false;
      double maxPENumber = -1.0;

      const TankCluster* clusterPtrWithMaxPE = nullptr;
      for (const TankCluster& cl : eTankClusters.Get() )
      {
         
         if (cl.GetClusterTime() < 2000.0) //initial window
         {
          
            hasClusterInPromptWindow = true;

            if (cl.GetMaxPE() > maxPENumber) 
            {
                maxPENumber  = cl.GetMaxPE();
                clusterPtrWithMaxPE = &cl;
            }
            
            //std::cout<<"Max PE in this cluster : "<<cl.GetMaxPE()<<std::endl;
            //for(const PMTHit& hit: cl.GetHits())
            //std::cout<<hit.GetPE()<<std::endl;
                  
         }
           
      }
     
      //std::cout<<"The higest PE number in all clusters: "<<maxPENumber<<std::endl;
       
     
      // Cut 2: High-quality PMT cluster
      // Check if the previously selected cluster is a high-quality PMT cluster
      bool highQualityCluster = false;   
      if (hasClusterInPromptWindow && clusterPtrWithMaxPE )
      {
         
         if (clusterPtrWithMaxPE->GetChargeBalance() < 0.2)
            highQualityCluster = true;
      }
         

      // Cut 3: MRD track reconstructed
      bool mrdTrackReconstructed = false;

      for (const MRDCluster& cluster : eMRDClusters.Get()) {

         // Her hit için: (time, type)
         // type: 0 -> side, 1 -> top, -1 -> ilgisiz
         std::vector<std::pair<double, int>> taggedHits;

         for (const MRDHit& hit : cluster.GetHits()) {

            int chankey = hit.GetChankey();
            double time = hit.GetTime();

            if (std::find(sideMRDPMTs.begin(), sideMRDPMTs.end(), chankey) != sideMRDPMTs.end()) {
               taggedHits.emplace_back(time, 0);
            }
            else if (std::find(topMRDPMTs.begin(), topMRDPMTs.end(), chankey) != topMRDPMTs.end()) {
               taggedHits.emplace_back(time, 1);
            }
         }

         if (taggedHits.size() < 4) continue;

         // Zamana göre sırala
         std::sort(taggedHits.begin(), taggedHits.end(),
                   [](const auto& a, const auto& b) {
                      return a.first < b.first;
                   });

         // Sliding window
         for (size_t i = 0; i < taggedHits.size(); ++i) {

            int nSideHits = 0;
            int nTopHits  = 0;

            for (size_t j = i; j < taggedHits.size(); ++j) {

               double dt = taggedHits[j].first - taggedHits[i].first;

               if (dt > 30.0) break;

               if (taggedHits[j].second == 0) ++nSideHits;
               else if (taggedHits[j].second == 1) ++nTopHits;

               if (nSideHits >= 2 && nTopHits >= 2) {
                  mrdTrackReconstructed = true;
                  break;
               }
            }

            if (mrdTrackReconstructed) break;
         }

         if (mrdTrackReconstructed) break;  // bir cluster yeterli
      }
      
      
      
      
      
      // Cut 4: In-time MRD coincidence
      bool tankMRDCoinc = false;
      if (eventReader.GetTankMRDCoinc() == 1)
         tankMRDCoinc = true;
      
      
      //Cut 5: Muon topology cut
      bool muonTopology = false;
      
      //neigbors of lappd 39
      std::vector<int> pmtIDs {462, 428, 406, 412};
      //std::vector<int> pmtIDs {383, 389, 440, 441};

      std::unordered_map<int, bool> passed;
      for (int id : pmtIDs) 
         passed[id] = false;

      for (const TankCluster& cluster : eTankClusters.Get())
      {
         if (cluster.GetClusterTime() > 2000.0) //initial window
            continue;
            
         for (const PMTHit& hit : cluster.GetHits())
         {
             double q = hit.GetPE();
             int id   = hit.GetDetID();

             auto it = passed.find(id);
             if (it != passed.end() && q > 5)
             {
               it->second = true;
             }
         }
      }

      bool missing = false;
      for (auto& kv : passed)
      {
        if (!kv.second)
        {
          missing = true;
          break;
        }
      }

      if (!missing) 
         muonTopology = true;


      //Cut 6: No hit in FMV
      bool noVeto = false;
      if (eventReader.NoVeto() == 1)
         noVeto = true;
 
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
         if (isPairedEvent)
         {
            h0->Fill(dt_ns);
            if (hasClusterInPromptWindow)
            {
               h1->Fill(dt_ns);
               
               if (highQualityCluster)
               {
                  h2->Fill(dt_ns);
                  
                  if (mrdTrackReconstructed)
                  {
                     h3->Fill(dt_ns);
                     
                     if (tankMRDCoinc)
                     {
                        h4->Fill(dt_ns);
                        
                        if (muonTopology)
                        {
                           h5->Fill(dt_ns);
                           
                           if (noVeto)
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
   
   /*
   
   // Stack & draw as line histograms
   THStack* stack = new THStack("stack", "");
   h0->SetLineColor(kBlack);  h0->SetLineWidth(2);
   h1->SetLineColor(kRed);     h1->SetLineWidth(2);
   h2->SetLineColor(kBlue);    h2->SetLineWidth(2);
   
   stack->Add(h0);
   stack->Add(h1);
   stack->Add(h2);
   
   
   stack->Draw("nostack hist");
   stack->GetXaxis()->SetTitle("#Delta t [ns]");
   stack->GetYaxis()->SetTitle("Counts/125 ns");

   // Legend with full-cut descriptions
   TLegend* leg = new TLegend(0.60, 0.60, 0.88, 0.88);
   leg->AddEntry(h0,   "All built events", "l");
   leg->AddEntry(h1,    "Any PMT cluster ",      "l");
   leg->AddEntry(h2,   "High quality cluster",      "l");
   leg->Draw();
   */
   
   
   
   
   // Stack & draw as line histograms
   THStack* stack = new THStack("stack", "");
   h2->SetLineColor(kBlue);     h2->SetLineWidth(2);
   h3->SetLineColor(kGreen);    h3->SetLineWidth(2);
   h4->SetLineColor(kYellow);   h4->SetLineWidth(2);
   h5->SetLineColor(kGray);     h5->SetLineWidth(2);
   h6->SetLineColor(kMagenta);  h6->SetLineWidth(2);
  
   stack->Add(h2);
   stack->Add(h3);
   stack->Add(h4);
   stack->Add(h5);
   stack->Add(h6);
   
   stack->Draw("nostack hist");
   stack->GetXaxis()->SetTitle("#Delta t [ns]");
   stack->GetYaxis()->SetTitle("Counts/125 ns");

   // Legend with full-cut descriptions
   TLegend* leg = new TLegend(0.60, 0.60, 0.88, 0.88);
   leg->AddEntry(h2,   "High quality cluster",      "l");
   leg->AddEntry(h3,   "MRD track reconstructed",   "l");
   leg->AddEntry(h4,   "In-time MRD coincidence",   "l");
   leg->AddEntry(h5,   "Muon toplogy",   "l");
   leg->AddEntry(h6,   "No hit in FMV",   "l");
   leg->Draw();
   
     
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

std::string GetBaseName(const std::string& fileName) {
    size_t lastSlash = fileName.find_last_of("/\\");
    std::string base = (lastSlash == std::string::npos) ? fileName : fileName.substr(lastSlash + 1);

    size_t lastDot = base.find_last_of(".");
    return (lastDot == std::string::npos) ? base : base.substr(0, lastDot);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool IsHighQualityPMTCluster(const TankCluster* clusterPtrWithMaxPE) {
    const std::vector<PMTHit>& hits = clusterPtrWithMaxPE->GetHits();
    
      
    double sumQ = 0.0;
    double sumQ2 = 0.0;
    int N = hits.size();

    if (N == 0) return false;

   
    for (const auto& hit : hits) {
        double q = hit.GetQ();
        sumQ += q;
        sumQ2 += q * q;
    }

    double term = (sumQ2 / (sumQ * sumQ)) - (1.0 / N);
    double qualityMetric = std::sqrt(term);

   std::cout<<"CB1: "<<clusterPtrWithMaxPE->GetChargeBalance()<<std::endl;
   std::cout<<"CB2: "<<qualityMetric<<std::endl;
    return (qualityMetric < 0.2);
}



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




void ReadChannels(const std::string& filename,
                  std::vector<int>& verticalChannels,
                  std::vector<int>& horizontalChannels)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "File cannot be opened!" << std::endl;
        return;
    }

    std::string line;

    // header'ı atla
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        int channel_num;
        int detector_system;
        int orientation;

        // sadece ilk 3 column'u oku
        ss >> channel_num >> detector_system >> orientation;

        if (orientation == 0) {
            verticalChannels.push_back(channel_num);
        }
        else if (orientation == 1) {
            horizontalChannels.push_back(channel_num);
        }
    }

    file.close();
}



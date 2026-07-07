#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <string>
#include <algorithm>
#include "TString.h"

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TMarker.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TEllipse.h"
#include "TGraph2D.h"

#include "ANNIEEventReader.h"
#include "MCANNIEEventReader.h"
#include "RootTreeReader.h"
#include "MRDClusterReader.h"
#include "TankClusterReader.h"
#include "EPMTHits.h"
#include "FMVHitReader.h"
#include "EFMVHits.h"
#include "LAPPDHitReader.h"
#include "MCLAPPDHitReader.h"
#include "ELAPPDHits.h"

#include "INIReader.h"
//==================================================
// PMT info struct
//==================================================
struct PMTInfo {
   int         detector_num;
   int         channel_num;
   std::string tank_location;  // Bottom / Lateral / Top ...
   double      x_pos;
   double      y_pos;
   double      z_pos;
   double      x_dir;
   double      y_dir;
   double      z_dir;
   std::string pmt_type;       // LUX, R5912 etc.
};

///==================================================
// MRD / FMV bar info struct
//==================================================
struct MRDInfo {
   int    channel_num;      // kanal numarası
   int    detector_system;  // 0 = FMV, !=0 = MRD
   int    orientation;      // 0 / 1
   int    layer;            // layer index
   int    side;             // side index
   int    num;              // bar number within layer/side

   double x_center;         // bar center position X
   double y_center;         // bar center position Y
   double z_center;         // bar center position Z
};

//==================================================
// LAPPD info struct
//==================================================
struct LAPPDInfo {
   int    id;      // LAPPD ID (ör: 9, 11, 13, 15, 17)
   double x_pos;   // x [same units as file]
   double y_pos;   // y
   double z_pos;   // z
};


//==================================================
//  Read PMT positions file
//==================================================
std::vector<PMTInfo> LoadPMTInfo(const std::string& pmtFile)
{
   std::vector<PMTInfo> pmts;

   std::ifstream infile(pmtFile.c_str());
   if (!infile.is_open()) {
      std::cerr << "[LoadPMTInfo] ERROR: Cannot open file: "
                << pmtFile << std::endl;
      return pmts;
   }

   std::string line;

   // Skip header (first line)
   if (std::getline(infile, line)) {
      // assume header line
   }

   // Read line by line
   while (std::getline(infile, line)) {
      if (line.empty()) continue;

      std::istringstream iss(line);
      PMTInfo p;

      iss >> p.detector_num
          >> p.channel_num
          >> p.tank_location
          >> p.x_pos
          >> p.y_pos
          >> p.z_pos
          >> p.x_dir
          >> p.y_dir
          >> p.z_dir
          >> p.pmt_type;

      // converts global to tank center.units are in cm
      p.y_pos = p.y_pos + 0.1446; //wrt tank center
      p.z_pos = p.z_pos - 1.681; //wrt tank center

      if (!iss) {
         std::cerr << "[LoadPMTInfo] WARNING: Could not parse line:\n"
                   << line << std::endl;
         continue;
      }

      pmts.push_back(p);
   }

   std::cout << "[LoadPMTInfo] Loaded " << pmts.size()
             << " PMTs from " << pmtFile << std::endl;

   return pmts;
}


//==================================================
// Read MRD bars positions file
//==================================================

std::vector<MRDInfo> LoadMRDInfo(const std::string& mrdFile)
{
   std::vector<MRDInfo> mrdBars;

   std::ifstream infile(mrdFile.c_str());
   if (!infile.is_open()) {
      std::cerr << "[LoadMRDInfo] ERROR: Cannot open file: "
                << mrdFile << std::endl;
      return mrdBars;
   }

   std::string line;

   // Skip header (first line)
   if (std::getline(infile, line)) {
      // assume header line: orientation layer side num x_center y_center z_center
   }


   // Read line by line
   while (std::getline(infile, line)) {
      if (line.empty()) continue;

      std::istringstream iss(line);
      MRDInfo m;

         iss >> m.channel_num
          >> m.detector_system
          >> m.orientation
          >> m.layer
          >> m.side
          >> m.num
          >> m.x_center
          >> m.y_center
          >> m.z_center;


      if (!iss) {
         std::cerr << "[LoadMRDInfo] WARNING: Could not parse line:\n"
                   << line << std::endl;
         continue;
      }

      // NOTE:
      // If the positions in MRDInfo.txt are in cm and you want meters
      // for the event display, you can uncomment the following lines:
      /*
      m.x_center *= 0.01;
      m.y_center *= 0.01;
      m.z_center *= 0.01;
      */

      mrdBars.push_back(m);
   }

   std::cout << "[LoadMRDInfo] Loaded " << mrdBars.size()
             << " MRD bars from " << mrdFile << std::endl;

   return mrdBars;
}

//==================================================
// Read LAPPD positions file
//   File format 
//   id   x   y   z
//==================================================
std::vector<LAPPDInfo> LoadLAPPDInfo(const std::string& lappdFile)
{
   std::vector<LAPPDInfo> lappds;

   std::ifstream infile(lappdFile.c_str());
   if (!infile.is_open()) {
      std::cerr << "[LoadLAPPDInfo] ERROR: Cannot open file: "
                << lappdFile << std::endl;
      return lappds;
   }

   std::string line;

   while (std::getline(infile, line)) {
      if (line.empty()) continue;

      // Skip comment / header lines (non-numeric first char)
      char c = line[0];
      if (!std::isdigit(c) && c != '-' && c != '+') continue;

      std::istringstream iss(line);
      LAPPDInfo l;

      iss >> l.id
          >> l.x_pos
          >> l.y_pos
          >> l.z_pos;

      //converts mm to m
      //l.x_pos = 0.001*l.x_pos;
      //l.y_pos = 0.001*l.y_pos;
      //l.z_pos = 0.001*l.z_pos;
      
      l.y_pos = l.y_pos + 0.1446;
      l.z_pos = l.z_pos - 1.681;

      if (!iss) {
         std::cerr << "[LoadLAPPDInfo] WARNING: Could not parse line:\n"
                   << line << std::endl;
         continue;
      }

      // If needed, you can apply offsets or unit conversions here.
      // Example (commented out):
      // l.x_pos *= 0.01;  // cm -> m
      // l.y_pos *= 0.01;
      // l.z_pos *= 0.01;

      lappds.push_back(l);
   }

   std::cout << "[LoadLAPPDInfo] Loaded " << lappds.size()
             << " LAPPDs from " << lappdFile << std::endl;

   // Print them for checking
   for (const auto& l : lappds) {
      std::cout << "  LAPPD id=" << l.id
                << "  x=" << l.x_pos
                << "  y=" << l.y_pos
                << "  z=" << l.z_pos
                << std::endl;
   }

   return lappds;
}

//==================================================
// Helper: split PMTs by location (top / bottom / barrel)
//==================================================
void SplitPMTsByLocation(const std::vector<PMTInfo>& allPMTs,
                         std::vector<PMTInfo>& topPMTs,
                         std::vector<PMTInfo>& bottomPMTs,
                         std::vector<PMTInfo>& barrelPMTs)
{
   topPMTs.clear();
   bottomPMTs.clear();
   barrelPMTs.clear();

   for (const auto& p : allPMTs) {
      std::string loc = p.tank_location;
      std::transform(loc.begin(), loc.end(), loc.begin(), ::tolower);

      if (loc == "top") {
         topPMTs.push_back(p);
      } else if (loc == "bottom") {
         bottomPMTs.push_back(p);
      } else if (loc == "lateral" || loc == "barrel") {
         barrelPMTs.push_back(p);
      } else {
         std::cerr << "[SplitPMTsByLocation] WARNING: Unknown tank_location = "
                   << p.tank_location << std::endl;
      }
   }

   std::cout << "[SplitPMTsByLocation] Top    PMTs: " << topPMTs.size()    << std::endl;
   std::cout << "[SplitPMTsByLocation] Bottom PMTs: " << bottomPMTs.size() << std::endl;
   std::cout << "[SplitPMTsByLocation] Barrel PMTs: " << barrelPMTs.size() << std::endl;
}


//==================================================
// Helper: split MRD bars by detector_system
//   detector_system == 0  -> FMV
//   detector_system != 0  -> MRD
//==================================================
void SplitMRDByDetectorSystem(const std::vector<MRDInfo>& allMRD,
                              std::vector<MRDInfo>& fmvBars,
                              std::vector<MRDInfo>& mrdBars)
{
   fmvBars.clear();
   mrdBars.clear();

   for (const auto& m : allMRD) {
      if (m.detector_system == 0) {
         // FMV
         fmvBars.push_back(m);
      } else {
         // MRD
         mrdBars.push_back(m);
      }
   }

   std::cout << "[SplitMRDByDetectorSystem] FMV bars (detector_system=0): "
             << fmvBars.size() << std::endl;
   std::cout << "[SplitMRDByDetectorSystem] MRD bars (detector_system!=0): "
             << mrdBars.size() << std::endl;
}

//==================================================

double RotatePhi(double phi, double rotationAngle)
{
   double dphi = rotationAngle * M_PI / 180.0;

   // açıya ekle
   phi = phi + dphi;

   // wrap-around: [-π, +π] aralığına getir
   phi = fmod(phi + M_PI, 2*M_PI);
   if (phi < 0) phi += 2*M_PI;
   phi -= M_PI;
   return phi;

}


//==================================================

//for upper right info.display pad
void DrawDetectorSketch();


//==================================================

TH2D* CreateColorScale(const std::string& histName,
                       const std::string& yTitle,
                       double minVal,
                       double maxVal,
                       double yLabelSize  = 0.17,
                       double yTitleSize  = 0.17,
                       double yTitleOffset= 1.7,
                       int nYBins         = 100);

//==================================================

void displayEvent(const INIReader& iniReader);

//==================================================

int main(int argc, char* argv[])
{
 
   INIReader iniReader(argv[1]);
   displayEvent(iniReader);
   
   return 0;
}

//==================================================

void displayEvent(const INIReader& iniReader)
{
   
   // 1) Read MRD positions
   std::vector<MRDInfo> mrdList = LoadMRDInfo( iniReader.Get("input","mrdFile","") );
   if (mrdList.empty()) {
      std::cerr << "[displayEvent] No MRDs loaded, exiting." << std::endl;
      return;
   }
   
   std::vector<MRDInfo> fmvBars;   // detector_system == 0
   std::vector<MRDInfo> mrdBars;   // detector_system != 0
   SplitMRDByDetectorSystem(mrdList, fmvBars, mrdBars);

   // MRD bar: channel_num -> index (mrdBars içinde)
   std::map<int, size_t> mrdChanToIndex;
   for (size_t i = 0; i < mrdBars.size(); ++i) {
      int ch = mrdBars[i].channel_num;
      mrdChanToIndex[ch] = i;
   }

   //FMV bar
   std::map<int, size_t> fmvChanToIndex;
   for (size_t i = 0; i < fmvBars.size(); ++i) {
      int ch = fmvBars[i].channel_num;
      fmvChanToIndex[ch] = i;
   }

   // 2) Read PMT positions
   std::vector<PMTInfo> pmtList = LoadPMTInfo( iniReader.Get("input","pmtFile","") );
   if (pmtList.empty()) {
      std::cerr << "[displayEvent] No PMTs loaded, exiting." << std::endl;
      return;
   }

   // Split into top / bottom / barrel
   std::vector<PMTInfo> topPMTs;
   std::vector<PMTInfo> bottomPMTs;
   std::vector<PMTInfo> barrelPMTs;

   SplitPMTsByLocation(pmtList, topPMTs, bottomPMTs, barrelPMTs);

   //  Map from PMTID -> index in pmtList
   std::map<int, size_t> pmtIdToIndex;
   for (size_t i = 0; i < pmtList.size(); ++i) {
      int detId = pmtList[i].detector_num;  // adjust if needed
      pmtIdToIndex[detId] = i;
   }


   // 3. Read LAPPD positions
   std::vector<LAPPDInfo> lappdList = LoadLAPPDInfo( iniReader.Get("input","lappdFile","" ) );
   if (lappdList.empty()) {
      std::cerr << "[displayEvent] No LAPPDs loaded, exiting." << std::endl;
   }

   // LAPPD id -> index map
   std::map<int, size_t> lappdIdToIndex;
   for (size_t iL = 0; iL < lappdList.size(); ++iL) {
      int id = lappdList[iL].id;
      lappdIdToIndex[id] = iL;
   }


   // Open ROOT tree
   std::string rootFile = iniReader.Get("input", "rootFile", "");
   const std::string& treeName = "Event";
   RootTreeReader treeReader(rootFile, treeName);
   if (!treeReader.IsValid()) return;
   TTree* tree = treeReader.GetTree();

   Long64_t totalEntries = tree->GetEntries();
   std::cout << "Number of events in this tree: " << totalEntries << std::endl;


   const bool useMC = iniReader.GetBoolean("input", "useMC", false);

   // Common readers
   TankClusterReader tankClusterReader;
   tankClusterReader.Init(tree);

   MRDClusterReader mrdClusterReader;
   mrdClusterReader.Init(tree);

   FMVHitReader fmvHitReader;
   fmvHitReader.Init(tree);

   // Data / MC readers
   ANNIEEventReader   eventReader;
   MCANNIEEventReader mcEventReader;

   LAPPDHitReader     lappdHitReader;
   MCLAPPDHitReader   mcLappdHitReader;

   if (useMC) {
       mcEventReader.Init(tree);
       mcLappdHitReader.Init(tree);
   } else {
       eventReader.Init(tree);
       lappdHitReader.Init(tree);
   }
   

   // this is required.it turns off histo statistic box
   gStyle->SetOptStat(0);
   
   // this is required! dont remove
   gStyle->SetNumberContours(255);
   //gStyle->SetPalette(kRainBow);   // default genelde bu
   //gStyle->SetPalette(kViridis);   // çok güzel modern (öneririm)
   //gStyle->SetPalette(kBird);      
   //gStyle->SetPalette(kCool);      
   //gStyle->SetPalette(kDeepSea);   
   //gStyle->SetPalette(kGreyScale); // siyah-beyaz
   
                       
   TString outPdfName = iniReader.Get("output","pdfFileName","");
   int nPrinted  = 0;
   
   int maxEventNum = iniReader.GetInteger("event","maxEventNum", -1);

   if (maxEventNum != -1 && maxEventNum > totalEntries) {
      std::cout << "[WARNING] maxEventNum (" << maxEventNum 
                << ") is larger than total entries (" << totalEntries 
                << "). Using total entries instead.\n";
   }

   int nEntries = (maxEventNum == -1)
                     ? totalEntries
                     : std::min<Long64_t>(maxEventNum, totalEntries);
      
            
    // --- main event loop ---
   for (int i = 0; i < nEntries; ++i) 
   {

      tree->GetEntry(i);
      
      //if you want to display one event from a run
      //if (  !(eventReader.GetEventNumber() == 47 && eventReader.GetRunNumber()==4796) )
      //continue;




      // ===============================================================
      // Canvas and Pad Layout Setup for Event Display
      //
      // This block creates the main ROOT canvas and defines all pads
      // used in the event display. Each pad corresponds to a specific
      // detector view or information panel (Tank, MRD, FMV, LAPPD, etc.).
      //
      // Key features:
      // - A global canvas is created for each event.
      // - A helper lambda (MakePad) applies a global (x, y) shift to
      //   easily adjust the layout without modifying each pad manually.
      // - The global title pad is positioned independently (no shift).
      // - All other pads are created with consistent offsets for alignment.
      // - Pads are styled (background color, borders, etc.) according to
      //   their role (e.g. black for detector views, white for caps).
      // - Finally, all pads are drawn onto the canvas in the correct order.
      //
      // ===============================================================
     
      TCanvas* c = new TCanvas(Form("ev%d", i),
                         Form("Event %d", i),
                         1000, 800);
      //c->SetFillColor(kBlack);
      // -------------------------------------------------
      double xShift = 0.01;
      double yShift = 0.03;

      // Shift uygulayan helper
      auto MakePad = [&](const char* name,
                         const char* title,
                         double x1, double y1,
                         double x2, double y2) -> TPad*
      {
         return new TPad(name, title,
                         x1 + xShift, y1 + yShift,
                         x2 + xShift, y2 + yShift);
      };


      // Pad layout is defined in a top-to-bottom visual order 
      // -------------------------------------------------
      //  Global title → SHIFT YOK
      // -------------------------------------------------
      TPad* pEventDisplayTitle = new TPad("pEventDisplayTitle","pEventDisplayTitle",
                                   0.42, 0.94,   
                                   0.58, 0.97);

      // -------------------------------------------------
      // 🔵 Diğer tüm padler → SHIFT VAR
      // -------------------------------------------------

      TPad* pUpperLeftInfo = new TPad("pUpperLeftInfo","pUpperLeftInfo", 
                                 0.05, 0.70,
                                 0.35, 0.96
                                 );

      TPad* pTankTop = MakePad("pTankTop", "pTankTop",
                           0.40, 0.651,
                           0.61, 0.86);
      
                           
      TPad* pUpperRightInfo = new TPad("pUpperRightInfo","pUpperRightInfo", 
                                 0.68, 0.70,
                                 0.95, 0.96
                                 );
                                 

      TPad* pFMVSide = MakePad("pFMVSide", "pFMVSide",
                               0.01, 0.35,
                               0.08, 0.60);

      TPad* pFMVColorBar = MakePad("pFMVColorBar", "pFMVColorBar",
                                0.09, 0.35,
                                0.14, 0.60);

      TPad* pTankBarrel = MakePad("pTankBarrel", "pTankBarrel",
                              0.16, 0.30,
                              0.86, 0.65);

      TPad* pTankPMTColorBar = MakePad("pTankPMTColorBar", "pTankPMTColorBar",
                                0.87, 0.35,
                                0.92, 0.60);

      TPad* pLAPPDColorBar = MakePad("pLAPPDColorBar", "pLAPPDColorBar",
                                  0.93, 0.35,
                                  0.98, 0.60);


      TPad* pMRDColorBar = MakePad("pMRDColorBar", "pMRDColorBar",
                                0.02, 0.02,
                                0.07, 0.27);

      TPad* pMRDSide = MakePad("pMRDSide", "pMRDSide",
                               0.08, 0.01,
                               0.37, 0.29);
                               
      TPad* pTankBottom = MakePad("pTankBottom", "pTankBottom",
                              0.40, 0.09,
                              0.61, 0.298);

      TPad* pMRDTop = MakePad("pMRDTop", "pMRDTop",
                              0.65, 0.01,
                              0.90, 0.29);


      
      
      // -----------------------------
      // Styling: fill colors
      // -----------------------------
      pEventDisplayTitle->SetFillColor(kBlack);
      pTankBarrel->SetFillColor(kBlack);
      pMRDSide->SetFillColor(kBlack);
      pMRDTop->SetFillColor(kBlack);
      pFMVSide->SetFillColor(kBlack);

      pTankTop->SetFillColor(kWhite); //icinde elips cizilmis
      pTankBottom->SetFillColor(kWhite); //icinde elips cizilmis

      pUpperLeftInfo->SetFillColor(kGray);
      pUpperRightInfo->SetFillColor(kGray);


      // -----------------------------
      // Styling: border mode
      // -----------------------------
     // --- Border mode ---
      pEventDisplayTitle->SetBorderMode(-1);   // 1=raised
      pTankBarrel->SetBorderMode(1);
      pTankTop->SetBorderMode(1);
      pTankBottom->SetBorderMode(1);
      pTankPMTColorBar->SetBorderMode(1);
      pLAPPDColorBar->SetBorderMode(1);
      pMRDColorBar->SetBorderMode(1);
      pFMVColorBar->SetBorderMode(1);
   
      pMRDSide->SetBorderMode(1);
      pMRDTop->SetBorderMode(1);
      pFMVSide->SetBorderMode(1);
      pUpperLeftInfo->SetBorderMode(1);
      pUpperRightInfo->SetBorderMode(1);


      // -----------------------------
      // Styling: border size
      // -----------------------------
      pUpperLeftInfo->SetBorderSize(2);
      pUpperRightInfo->SetBorderSize(2);


      // -----------------------------
      // Styling: line color (border color)
      // -----------------------------
      //pUpperLeftInfo->SetLineColor(kWhite);
      //pUpperRightInfo->SetLineColor(kWhite);
      
      // --- Line color ---
      pEventDisplayTitle->SetLineColor(kBlue);
      pTankBarrel->SetLineColor(kRed);
      pTankTop->SetLineColor(kRed);
      pTankBottom->SetLineColor(kRed);
      pTankPMTColorBar->SetLineColor(kRed);
      pLAPPDColorBar->SetLineColor(kRed);
      pMRDColorBar->SetLineColor(kRed);
      pFMVColorBar->SetLineColor(kRed);
      pMRDSide->SetLineColor(kRed);
      pMRDTop->SetLineColor(kRed);
      pFMVSide->SetLineColor(kRed);
      pUpperLeftInfo->SetLineColor(kRed);
      pUpperRightInfo->SetLineColor(kRed);


      // -----------------------------
      // Debug: pad borders (visualize pad boundaries)
      // -----------------------------


      // --- Line width ---
      pEventDisplayTitle->SetLineWidth(2);
      pTankBarrel->SetLineWidth(2);
      pTankTop->SetLineWidth(2);
      pTankBottom->SetLineWidth(2);
      pTankPMTColorBar->SetLineWidth(2);
      pMRDColorBar->SetLineWidth(2);
      pFMVColorBar->SetLineWidth(2);
      pLAPPDColorBar->SetLineWidth(2);
      pMRDSide->SetLineWidth(2);
      pMRDTop->SetLineWidth(2);
      pFMVSide->SetLineWidth(2);
      pUpperLeftInfo->SetLineWidth(2);
      pUpperRightInfo->SetLineWidth(2);



      // -----------------------------
      // Draw
      //Burda istedigini cikartabilirsin event displayden
      // -----------------------------
      pEventDisplayTitle->Draw();   
      pTankBarrel->Draw();
      pTankTop->Draw();
      pTankBottom->Draw();
      pTankPMTColorBar->Draw();
      pMRDColorBar->Draw();
      pFMVColorBar->Draw();
      pLAPPDColorBar->Draw();
      pMRDSide->Draw();
      pMRDTop->Draw();
      pFMVSide->Draw();
      pUpperLeftInfo->Draw();
      pUpperRightInfo->Draw();

     


      pFMVColorBar->SetLeftMargin(0.7);
      pFMVColorBar->SetRightMargin(0.20);
      pFMVColorBar->SetTopMargin(0.05);
      pFMVColorBar->SetBottomMargin(0.15);

      pMRDColorBar->SetLeftMargin(0.7);
      pMRDColorBar->SetRightMargin(0.20);
      pMRDColorBar->SetTopMargin(0.05);
      pMRDColorBar->SetBottomMargin(0.15);

      pTankPMTColorBar->SetLeftMargin(0.7);
      pTankPMTColorBar->SetRightMargin(0.20);
      pTankPMTColorBar->SetTopMargin(0.05);
      pTankPMTColorBar->SetBottomMargin(0.15);

      pLAPPDColorBar->SetLeftMargin(0.7);
      pLAPPDColorBar->SetRightMargin(0.20);
      pLAPPDColorBar->SetTopMargin(0.05);
      pLAPPDColorBar->SetBottomMargin(0.15);
     
     
      // ========================================
      // 1. Tank PMT subsystem
      // ========================================
     
      // Tank PMTs display data
      // Collect PE per PMT ONLY for the maximum-PE cluster in prompt window

      double clusterTimeMin = iniReader.GetReal("tankPMT","clusterTimeMin", 0.0);
      double clusterTimeMax = iniReader.GetReal("tankPMT","clusterTimeMax", 0.0);
      
      std::vector<double> bestClusterPMTPE(pmtList.size(), 0.0);

      // Prompt window içindeki en büyük PE'li cluster'ı bul
      const TankCluster* bestTankCluster = nullptr;
      double bestTankClusterPE           = -1.0;
      double bestTankClusterTime         = -9999.0;

      ETankClusters eTankClusters;
      tankClusterReader.ReadEntry(i, eTankClusters);
      
      for (const TankCluster& cluster : eTankClusters.Get())
      {
          double tankClusterTime = cluster.GetClusterTime();  // [ns]
          double tankClusterPE   = cluster.GetClusterPE();    // [PE]

          if (tankClusterTime > clusterTimeMax || tankClusterTime < clusterTimeMin) 
            continue;
         
          if (tankClusterPE > bestTankClusterPE)
          {
              bestTankClusterPE   = tankClusterPE;
              bestTankClusterTime = tankClusterTime;
              bestTankCluster     = &cluster;
          }
      }

      // Sadece seçilen cluster'ın hitlerini kullan
      if (bestTankCluster != nullptr)
      {
          const std::vector<PMTHit>& chits = bestTankCluster->GetHits();

          for (const PMTHit& hit : chits)
          {
              int    detId = hit.GetDetID();
              double pe    = hit.GetPE();
              double tHit  = hit.GetT();   // istersen burada ayrıca hit-time cut da uygulayabilirsin

              auto it = pmtIdToIndex.find(detId);
              if (it == pmtIdToIndex.end())
                  continue;

              size_t pmtIndex = it->second;
              bestClusterPMTPE[pmtIndex] += pe;
          }
      }
     
      
      //Apply pe threshold cuts
      double tankPMTsPEThreshold = iniReader.GetReal("tankPMT",
                                                   "tankPMTsPEThreshold", 
                                                    30.);
      int numTriggeredPMTs = 0; // kaç PMT tetiklendi?

      for (size_t idx = 0; idx < bestClusterPMTPE.size(); ++idx) 
      {

         if (bestClusterPMTPE[idx] > tankPMTsPEThreshold) 
         {
            // threshold üzerindeyse: say ve değeri olduğu gibi bırak
            ++numTriggeredPMTs;
         } else 
         {
            // threshold altını 0 yap → "hit yok"
            bestClusterPMTPE[idx] = 0.0;
         }
      }

      // --- After filling bestClusterPMTPE vector ---

      /*
      std::cout << "\n=== PMT PE SUMMARY ===\n";

      for (const auto& kv : pmtIdToIndex) {
          int detId      = kv.first;
          size_t index   = kv.second;

            
          std::cout << "detId " << detId
                    << " (index " << index << ")"
                    << "  --> Total PE = " << bestClusterPMTPE[index]
                    << std::endl;
                    
      }
      */


      // ========================================
      // Tank PMT Visualization
      // ========================================
     
      //  1.1 Tank PMTs colorbar (PE) 
      pTankPMTColorBar->cd();
      
      
      // Tank PMTs pe color scale
      double maxTankPMTPE = 0.0;
      for (double v : bestClusterPMTPE) {
         if (v > maxTankPMTPE) 
            maxTankPMTPE = v;
      }

      double colorMinTankPE = tankPMTsPEThreshold;
      double colorMaxTankPE = maxTankPMTPE;

      // Güvenlik: axis range her zaman artan olsun
      if (colorMaxTankPE <= colorMinTankPE) {
          colorMaxTankPE = colorMinTankPE + 1.0;
      }

      TH2D* hTankPMTColorBar = CreateColorScale("hTankPMTColorBar",
                                    "PMT charge [PE]",
                                    colorMinTankPE,   
                                    colorMaxTankPE,
                                    0.17,
                                    0.17,
                                    1.70);

      hTankPMTColorBar->Draw("COL");
      //gPad->SetTicks(0,0);
      //gPad->SetFrameLineColor(0);
      //gPad->SetLineColor(0);
      
      
      // 1.2 Tank Barrel PMTs
      pTankBarrel->cd();

      double phiMin = +1e9, phiMax = -1e9;
      double yMin   = +1e9, yMax   = -1e9;

      for (const auto& p : barrelPMTs) {
         double phi = atan2(p.z_pos, p.x_pos);
         double y   = p.y_pos;
         if (phi < phiMin) phiMin = phi;
         if (phi > phiMax) phiMax = phi;
         if (y   < yMin)   yMin   = y;
         if (y   > yMax)   yMax   = y;
      }

      phiMin -= 0.1;
      phiMax += 0.1;
      yMin   -= 0.1;
      yMax   += 0.1;

      TH2D* hFrame = new TH2D("hFrame", ";#phi [rad];Y [m]",
                              10, phiMin, phiMax,
                              10, yMin,   yMax);
      hFrame->SetDirectory(nullptr);
      hFrame->SetStats(0);
      hFrame->SetLineColor(kBlack);
      hFrame->SetMarkerColor(kBlack);
      hFrame->Draw();

      gPad->SetFillColor(kBlack);
      gPad->SetFrameFillColor(kBlack);

      hFrame->GetXaxis()->SetLabelColor(kWhite);
      hFrame->GetYaxis()->SetLabelColor(kWhite);
      hFrame->GetXaxis()->SetTitleColor(kWhite);
      hFrame->GetYaxis()->SetTitleColor(kWhite);

      int nContours = gStyle->GetNumberContours();
      double R = 0.10;  // PMT radius in (phi, y) units

      //std::cout << "maxPE: " << maxTankPMTPE << std::endl;

      for (const auto& p : barrelPMTs) {

         auto it = pmtIdToIndex.find(p.detector_num);

         double pe = 0.0;
         if (it != pmtIdToIndex.end())
            pe = bestClusterPMTPE[it->second];

         double phi = atan2(p.z_pos, p.x_pos);
         double y   = p.y_pos;
         
         //dondurmek icin sadece
         phi = RotatePhi(phi, -90);
         
         double t = (maxTankPMTPE > 0.0) ? pe / maxTankPMTPE : 0.0;
         if (t < 0.0) t = 0.0;
         if (t > 1.0) t = 1.0;

         int paletteBin = int( t * (nContours-1) );
         int col = gStyle->GetColorPalette(paletteBin);

         TEllipse* e = new TEllipse(phi, y, R, R);
         if (pe <= 0) {
            e->SetFillColor(kBlack);
            e->SetFillStyle(0);
            e->SetLineColor(kGray+2);
         } else {
            e->SetFillColor(col);
            e->SetFillStyle(1001);
            e->SetLineColor(col);
         }
         e->SetLineWidth(1);
         e->Draw("same");

         TLatex* lab = new TLatex(phi, y, Form("%d", p.detector_num));
         lab->SetTextAlign(22);
         lab->SetTextColor(kWhite);
         lab->SetTextSize(0.035);
         lab->Draw("same");
      }
      
      
      //  1.3 Tank TOP PMTs
      pTankTop->cd();
      
      pTankTop->SetLeftMargin(0.015);
      pTankTop->SetRightMargin(0.015);
      pTankTop->SetTopMargin(0.015);
      pTankTop->SetBottomMargin(0.017);

      if (!topPMTs.empty()) {

         double xMin = +1e9, xMax = -1e9;
         double zMin = +1e9, zMax = -1e9;

         for (const auto& p : topPMTs) {
            if (p.x_pos < xMin) xMin = p.x_pos;
            if (p.x_pos > xMax) xMax = p.x_pos;
            if (p.z_pos < zMin) zMin = p.z_pos;
            if (p.z_pos > zMax) zMax = p.z_pos;
         }

         double dx = 0.1 * (xMax - xMin);
         double dz = 0.1 * (zMax - zMin);
         xMin -= dx; xMax += dx;
         zMin -= dz; zMax += dz;

         TH2D* hTopFrame = new TH2D("hTopFrame", ";X [m];Z [m]",
                                    10, xMin, xMax,
                                    10, zMin, zMax);
         hTopFrame->SetDirectory(nullptr);
         hTopFrame->SetStats(0);
         
         
         //eksenleri kapatiyor
         hTopFrame->GetXaxis()->SetAxisColor(0);
         hTopFrame->GetYaxis()->SetAxisColor(0);
         
         
         hTopFrame->GetXaxis()->SetLabelSize(0);
         hTopFrame->GetYaxis()->SetLabelSize(0);
         hTopFrame->GetXaxis()->SetTitleSize(0);
         hTopFrame->GetYaxis()->SetTitleSize(0);
         hTopFrame->GetXaxis()->SetTickLength(0);
         hTopFrame->GetYaxis()->SetTickLength(0);
         
      
         //hTopFrame->SetLineWidth(0);        // tamamen kaldır

         hTopFrame->SetLineColor(kBlack);
         hTopFrame->SetMarkerColor(kBlack);
         hTopFrame->Draw();
         
        
         
         gPad->SetFrameLineColor(kBlack);
         gPad->SetFrameLineWidth(0);

         // Tank cap
         double xCenterTop = 0.5*(xMin + xMax);
         double zCenterTop = 0.5*(zMin + zMax);
         double spanTop    = std::max(xMax - xMin, zMax - zMin);
         double RcapTankTop    = 0.5 * spanTop * 1.02;

         TEllipse* capTankTop = new TEllipse(xCenterTop, zCenterTop, RcapTankTop, RcapTankTop);
         capTankTop->SetFillColor(kBlack);
         capTankTop->SetFillStyle(1001);
         capTankTop->SetLineColor(kGray+2);
         capTankTop->SetLineWidth(2);
         capTankTop->Draw("same");

         //gPad->SetFillColor(kBlack);
         //gPad->SetFrameFillColor(kBlack);
         hTopFrame->GetXaxis()->SetLabelColor(kWhite);
         hTopFrame->GetYaxis()->SetLabelColor(kWhite);
         hTopFrame->GetXaxis()->SetTitleColor(kWhite);
         hTopFrame->GetYaxis()->SetTitleColor(kWhite);

         double Rtop = 0.09;

         for (const auto& p : topPMTs) {

            auto it = pmtIdToIndex.find(p.detector_num);
            double pe = 0.0;
            if (it != pmtIdToIndex.end())
               pe = bestClusterPMTPE[it->second];

            double t = (maxTankPMTPE > 0.0) ? pe / maxTankPMTPE : 0.0;
            if (t < 0.0) t = 0.0;
            if (t > 1.0) t = 1.0;

            int paletteBin = int( t * (nContours-1) );
            int col = gStyle->GetColorPalette(paletteBin);

            TEllipse* e = new TEllipse(p.x_pos, p.z_pos, Rtop, Rtop);
            if (pe <= 0) {
               e->SetFillColor(kBlack);
               e->SetFillStyle(0);
               e->SetLineColor(kGray+2);
            } else {
               e->SetFillColor(col);
               e->SetFillStyle(1001);
               e->SetLineColor(col);
            }
            e->Draw("same");

            TLatex* lab = new TLatex(p.x_pos, p.z_pos, Form("%d", p.detector_num));
            lab->SetTextAlign(22);
            lab->SetTextColor(kWhite);
            lab->SetTextSize(0.055);
            lab->Draw("same");
         }
      }

      
      // 1.4 Tank BOTTOM PMTs
      
      pTankBottom->cd();
      
      pTankBottom->SetLeftMargin(0.015);
      pTankBottom->SetRightMargin(0.015);
      pTankBottom->SetTopMargin(0.018);
      pTankBottom->SetBottomMargin(0.019);

      if (!bottomPMTs.empty()) {

         double xMinB = +1e9, xMaxB = -1e9;
         double zMinB = +1e9, zMaxB = -1e9;

         for (const auto& p : bottomPMTs) {
            if (p.x_pos < xMinB) xMinB = p.x_pos;
            if (p.x_pos > xMaxB) xMaxB = p.x_pos;
            if (p.z_pos < zMinB) zMinB = p.z_pos;
            if (p.z_pos > zMaxB) zMaxB = p.z_pos;
         }

         double dxB = 0.1 * (xMaxB - xMinB);
         double dzB = 0.1 * (zMaxB - zMinB);
         xMinB -= dxB; xMaxB += dxB;
         zMinB -= dzB; zMaxB += dzB;

         TH2D* hBottomFrame = new TH2D("hBottomFrame", ";X [m];Z [m]",
                                       10, xMinB, xMaxB,
                                       10, zMinB, zMaxB);
         hBottomFrame->SetDirectory(nullptr);
         hBottomFrame->SetStats(0);
         
         hBottomFrame->GetXaxis()->SetLabelSize(0);
         hBottomFrame->GetYaxis()->SetLabelSize(0);
         hBottomFrame->GetXaxis()->SetTitleSize(0);
         hBottomFrame->GetYaxis()->SetTitleSize(0);
         hBottomFrame->GetXaxis()->SetTickLength(0);
         hBottomFrame->GetYaxis()->SetTickLength(0);
         hBottomFrame->GetXaxis()->SetAxisColor(0);
         hBottomFrame->GetYaxis()->SetAxisColor(0);
         
         //
         hBottomFrame->GetXaxis()->SetAxisColor(0);
         hBottomFrame->GetYaxis()->SetAxisColor(0);
         
         hBottomFrame->SetLineColor(kBlack);
         hBottomFrame->SetMarkerColor(kBlack);
         hBottomFrame->Draw();
         
         gPad->SetFrameLineColor(kBlack);
         gPad->SetFrameLineWidth(0);

         double xCenterBot = 0.5*(xMinB + xMaxB);
         double zCenterBot = 0.5*(zMinB + zMaxB);
         double spanBot    = std::max(xMaxB - xMinB, zMaxB - zMinB);
         double RcapBot    = 0.5 * spanBot * 1.02;

         TEllipse* capBot = new TEllipse(xCenterBot, zCenterBot, RcapBot, RcapBot);
         capBot->SetFillColor(kBlack);
         capBot->SetFillStyle(1001);
         capBot->SetLineColor(kGray+2);
         capBot->SetLineWidth(2);
         capBot->Draw("same");

         //gPad->SetFillColor(kBlack);
         //gPad->SetFrameFillColor(kBlack);
         hBottomFrame->GetXaxis()->SetLabelColor(kWhite);
         hBottomFrame->GetYaxis()->SetLabelColor(kWhite);
         hBottomFrame->GetXaxis()->SetTitleColor(kWhite);
         hBottomFrame->GetYaxis()->SetTitleColor(kWhite);

         double Rbottom = 0.09;

         for (const auto& p : bottomPMTs) {

            auto it = pmtIdToIndex.find(p.detector_num);
            double pe = 0.0;
            if (it != pmtIdToIndex.end())
               pe = bestClusterPMTPE[it->second];

            double t = (maxTankPMTPE > 0.0) ? pe / maxTankPMTPE : 0.0;
            if (t < 0.0) t = 0.0;
            if (t > 1.0) t = 1.0;

            int paletteBin = int( t * (nContours-1) );
            int col = gStyle->GetColorPalette(paletteBin);

            TEllipse* e = new TEllipse(p.x_pos, p.z_pos, Rbottom, Rbottom);
            if (pe <= 0) {
               e->SetFillColor(kBlack);
               e->SetFillStyle(0);
               e->SetLineColor(kGray+2);
            } else {
               e->SetFillColor(col);
               e->SetFillStyle(1001);
               e->SetLineColor(col);
            }
            e->Draw("same");

            TLatex* lab = new TLatex(p.x_pos, p.z_pos, Form("%d", p.detector_num));
            lab->SetTextAlign(22);
            lab->SetTextColor(kWhite);
            lab->SetTextSize(0.055);
            lab->Draw("same");
         }
      }

      
      
      // ========================================
      // 2. Tank LAPPD subsystem
      // ========================================
      

      //----Tank LAPPDS display data ------
      // Collect LAPPD hit counts per LAPPD ID (with time window)
      const double lappdHitTimeMin = iniReader.GetReal("tankLAPPD",
                                                      "lappdHitTimeMin", 
                                                      0.);   
      const double lappdHitTimeMax = iniReader.GetReal("tankLAPPD",
                                                      "lappdHitTimeMax", 
                                                      250.); 
      std::vector<int> lappdHitCount(lappdList.size(), 0);

      if (useMC) {

         EMCLAPPDHits eLAPPDHits;
         mcLappdHitReader.ReadEntry(i, eLAPPDHits);

         for (const MCLAPPDHit& hit : eLAPPDHits.GetHits()) {
            int    lappdID = hit.GetTubeID();
            double t       = hit.GetTime();

            if (t < lappdHitTimeMin || t > lappdHitTimeMax)
               continue;

            auto itL = lappdIdToIndex.find(lappdID);
            if (itL == lappdIdToIndex.end()) continue;

            size_t idxL = itL->second;
            lappdHitCount[idxL]++;
         }

      } else {

         ELAPPDHits eLAPPDHits;
         lappdHitReader.ReadEntry(i, eLAPPDHits);

         for (const LAPPDHit& hit : eLAPPDHits.GetHits()) {
            int    lappdID = hit.GetLAPPDID();
            double t       = hit.GetTime();

            if (t < lappdHitTimeMin || t > lappdHitTimeMax)
               continue;

            auto itL = lappdIdToIndex.find(lappdID);
            if (itL == lappdIdToIndex.end()) continue;

            size_t idxL = itL->second;
            lappdHitCount[idxL]++;
         }
      
      }


      //Apply hit number cuts for lappds
      const int lappdHitThreshold = iniReader.GetInteger("tankLAPPD","lappdHitThreshold", 0); 
      int numTriggeredLAPPDs = 0;

      for (size_t k = 0; k < lappdHitCount.size(); ++k) {

         if (lappdHitCount[k] > lappdHitThreshold) {
            ++numTriggeredLAPPDs;
         } else {
            lappdHitCount[k] = 0; //hit yok kabul et.
         }
      }


      /*
      //to print how many hits each lappd detect.
      for (const auto& kv : lappdIdToIndex) {
          int detId      = kv.first;
          size_t index   = kv.second;

            
          std::cout << "detId " << detId
                    << " (index " << index << ")"
                    << "  -->  Photon count = " << lappdHitCount[index]
                    << std::endl;
                    
      }
      */
      
      
      
      
      // ========================================
      // Tank LAPPD Visualization
      // ========================================
      
      
      //  2.1. LAPPDs colorbar (hit count) 
      pLAPPDColorBar->cd();
      
      // LAPPD hit color scale
      int maxHitsL = 0;
      for (size_t k = 0; k < lappdList.size(); ++k) {
         if (lappdHitCount[k] > maxHitsL)
            maxHitsL = lappdHitCount[k];
      }
      
      int colorMinHitLAPPD = lappdHitThreshold;
      int colorMaxHitLAPPD = maxHitsL;

      // Güvenlik: axis range her zaman artan olsun
      if (colorMaxHitLAPPD <= colorMinHitLAPPD) {
          colorMaxHitLAPPD = colorMinHitLAPPD + 1.0;
      }
      
      TH2D* hTankLAPPDColorBar = CreateColorScale("hTankLAPPDColorBar",
                                     "LAPPD hits",
                                     (double)colorMinHitLAPPD,
                                     (double)colorMaxHitLAPPD,
                                     0.17,
                                     0.17,
                                     1.4);

      hTankLAPPDColorBar->Draw("COL");
      //gPad->SetTicks(0,0);
      //gPad->SetFrameLineColor(0);
      //gPad->SetLineColor(0);


      
      // 2.2. LAPPDs on barrel (phi, y)
      // Color = hit count
      pTankBarrel->cd();
      
      if (!lappdList.empty()) {

         int nContoursL = gStyle->GetNumberContours();

         // LAPPD'leri PMT'lerden biraz büyük kutu şeklinde çizelim
         double halfPhi = 0.10;   // phi yönde yarı-genişlik
         double halfY   = 0.14;   // y yönde yarı-yükseklik

         for (size_t k = 0; k < lappdList.size(); ++k) {

            const LAPPDInfo& L = lappdList[k];

            double phiL = atan2(L.z_pos, L.x_pos);
            double yL   = L.y_pos;
            
            
            phiL = RotatePhi(phiL, -90);
             
            int hits = lappdHitCount[k];

            // Hit yoksa sadece çerçeve çiz
            if (hits <= 0) {
               TBox* b = new TBox(phiL - halfPhi, yL - halfY,
                                  phiL + halfPhi, yL + halfY);
               b->SetFillColor(kBlack);
               b->SetFillStyle(0);
               b->SetLineColor(kRed-7);  // PMT'den farklı görünsün
               b->SetLineWidth(1);
               b->Draw("same");

               // LAPPD ID label
               TLatex* labL = new TLatex(phiL, yL, Form("%d", L.id));
               labL->SetTextAlign(22);
               labL->SetTextColor(kRed-7);
               labL->SetTextSize(0.040);
               labL->Draw("same");
               continue;
            }

            // Hit varsa: hit sayısını renge map et
            double u = (double)hits / (double)maxHitsL;  // 0–1 aralığı
            if (u < 0.0) u = 0.0;
            if (u > 1.0) u = 1.0;

            int paletteBinL = int(u * (nContoursL - 1));
            int colL = gStyle->GetColorPalette(paletteBinL);

            TBox* b = new TBox(phiL - halfPhi, yL - halfY,
                               phiL + halfPhi, yL + halfY);
            b->SetFillColor(colL);
            b->SetFillStyle(1001);
            b->SetLineColor(kRed-7);
            b->SetLineWidth(1);
            b->Draw("same");

            // LAPPD ID label
            TLatex* labL = new TLatex(phiL, yL, Form("%d", L.id));
            labL->SetTextAlign(22);
            labL->SetTextColor(kRed-7);
            labL->SetTextSize(0.040);
            labL->Draw("same");
         }
      }

    


      // ========================================
      // 3. MRD subsystem
      // ========================================

      // MRD display data
      // Select MRD cluster matched to the Tank cluster
      // For each MRD cluster:
      //   - Compute cluster time as average of hit times
      //   - Calculate time difference (Δt) w.r.t. selected Tank cluster
      //   - Apply timing window cut (e.g. 700–800 ns)
      //   - Choose the cluster closest to the expected Δt (~750 ns)
      // Result: best-matched MRD cluster in time
      // -------------------------------------------------
      std::vector<double> mrdBarHitTime(mrdBars.size(), 1e9);
      std::vector<bool>   mrdHasHit(mrdBars.size(), false);

      const MRDCluster* bestMRDCluster = nullptr;
      double bestMRDDeltaScore = 1e9;   // |deltaT - 750|
      double selectedMRDClusterTime = -9999.0;
      double selectedMRDDeltaT      = -9999.0;

      /*
      //hatirla MRDClusterTime bug vardi root dosyasi icinde.o yuzden bu kod blogu 
      // yorum satiri yapildi.
      
      for (const MRDCluster& cluster : eMRDClusters.Get()) {

         double mrdClusterTime = cluster.GetTime();   // <-- doğrudan cluster zamanı
         double deltaT = mrdClusterTime - bestTankClusterTime;

         // Zamansal uyum penceresi
         if (deltaT <= 700.0 || deltaT >= 800.0) continue;

         // 750 ns'e en yakın olanı seç
         double score = std::fabs(deltaT - 750.0);

         if (score < bestMRDDeltaScore) {
            bestMRDDeltaScore   = score;
            bestMRDCluster      = &cluster;
            selectedMRDClusterTime = mrdClusterTime;
            selectedMRDDeltaT      = deltaT;
         }
      }
      */

      // These settings may need to be adjusted after reviewing the histogram of 
      // MRD PMT difference.
      // 
      double dtMRDPMTMin      = iniReader.GetReal("mrd", "dtMRDPMTMin", 700);
      double dtMRDPMTMax      = iniReader.GetReal("mrd", "dtMRDPMTMax", 800);
      double dtMRDPMTHistPeak = iniReader.GetReal("mrd", "dtMRDPMTHistPeak", 750);
      
      EMRDClusters eMRDClusters;
      mrdClusterReader.ReadEntry(i, eMRDClusters);
      
      for (const MRDCluster& cluster : eMRDClusters.Get()) {

         // --- 1. Cluster içindeki hit'lerden zamanı hesapla ---
         const std::vector<MRDHit>& hits = cluster.GetHits();

         if (hits.empty()) continue;  // güvenlik

         double sumTime = 0.0;

         for (const MRDHit& hit : hits) {
            sumTime += hit.GetTime();   // hit zamanı (ns)
         }

         double mrdClusterTime = sumTime / hits.size();  // <-- YENİ cluster time

         // --- 2. Tank cluster ile zaman farkı ---
         double deltaT = mrdClusterTime - bestTankClusterTime;

         // --- 3. Zamansal uyum penceresi ---
         if (deltaT <= dtMRDPMTMin || deltaT >= dtMRDPMTMax) continue;

         // --- 4. 750 ns'e en yakın olanı seç ---
         double score = std::fabs(deltaT - dtMRDPMTHistPeak);

         if (score < bestMRDDeltaScore) {
            bestMRDDeltaScore   = score;
            bestMRDCluster      = &cluster;
            selectedMRDClusterTime = mrdClusterTime;
            selectedMRDDeltaT      = deltaT;
            
         }
      }


      // ===============================================================
      // MRD Cluster Selection & Time-Window-Based Visualization
      //
      // Bu kod iki ana işi yapar:
      //
      // 1) MRD CLUSTER SEÇİMİ
      //    - Tüm MRD cluster'lar gezilir
      //    - Her cluster için hitlerden ortalama zaman hesaplanır
      //    - Bu zaman ile best PMT cluster zamanı arasındaki fark (deltaT) bulunur
      //    - Sadece belirli zaman penceresinde (örn. 700–800 ns) olan cluster'lar alınır
      //    - Bu adaylar içinden deltaT ≈ 750 ns'e en yakın olan cluster seçilir
      //
      // 2) MRD HİTLERİNİN GÖRSELLEŞTİRİLMESİ (İKİ MOD)
      //    mrdDisplayMode ile kontrol edilir:
      //
      //    MODE 0 (eski yöntem):
      //       - Seçilen MRD cluster içindeki TÜM hitler kullanılır
      //       - Her bar için en erken hit zamanı tutulur
      //       - Color bar aralığı bu hitlerin min/max değerlerinden gelir
      //
      //    MODE 1 (yeni yöntem):
      //       - Seçilen MRD cluster içindeki hit zamanları alınır ve sıralanır
      //       - En yoğun (en fazla hit içeren) 30 ns zaman penceresi bulunur
      //       - Sadece bu pencereye düşen hitler kullanılır
      //       - Her bar için pencere içindeki en erken hit zamanı tutulur
      //       - Color bar aralığı doğrudan bu pencerenin [min, max] değeridir
      //
      // 3) SONUÇ
      //    - Elde edilen zaman aralığı kullanılarak bir color scale (renk çubuğu) çizilir
      //    - Bu color bar, MRD hit zamanlarının görsel karşılığını gösterir
      //
      // NOT:
      //    - Kod veri üretmez, seçilmiş MRD hitlerinin görselleştirilmesini yapar
      //    - MODE 1, zaman içinde kümelenmiş (yoğun) hitleri daha net gösterir
      // ===============================================================

      int mrdDisplayMode = 1;

      // Width of time window for new mode [ns]
      double mrdDisplayWindow = iniReader.GetReal("mrd","mrdDisplayWindow", 30);

      // Time range actually used for display / color bar
      double displayMinTime = -9999.0;
      double displayMaxTime = -9999.0;


     
      if (bestMRDCluster != nullptr) {

         // =========================================================
         // MODE 0:
         // Use all hits in the selected MRD cluster
         // =========================================================
         if (mrdDisplayMode == 0) {

            for (const MRDHit& hit : bestMRDCluster->GetHits()) {

               int    detId = hit.GetDetID();
               double t     = hit.GetTime();

               auto it = mrdChanToIndex.find(detId);
               if (it == mrdChanToIndex.end()) continue;

               size_t idx = it->second;

               // Keep earliest hit time for each bar
               if (!mrdHasHit[idx] || t < mrdBarHitTime[idx]) {
                  mrdBarHitTime[idx]   = t;
                  mrdHasHit[idx] = true;
               }
            }

            // In old mode, color bar range comes from selected hits
            double minTmp = 1e9;
            double maxTmp = -1e9;

            for (size_t k = 0; k < mrdBars.size(); ++k) {
               if (!mrdHasHit[k]) continue;
               double t = mrdBarHitTime[k];
               if (t < minTmp) minTmp = t;
               if (t > maxTmp) maxTmp = t;
            }

            if (minTmp > maxTmp) {
               displayMinTime = 0.0;
               displayMaxTime = 1.0;
            } else {
               displayMinTime = minTmp;
               displayMaxTime = maxTmp;
            }
         }

         // =========================================================
         // MODE 1:
         // Use only hits inside the densest time window
         // =========================================================
         else if (mrdDisplayMode == 1) {

            std::vector<double> hitTimes;
            hitTimes.reserve(bestMRDCluster->GetHits().size());

            for (const MRDHit& hit : bestMRDCluster->GetHits()) {
               hitTimes.push_back(hit.GetTime());
            }

            std::sort(hitTimes.begin(), hitTimes.end());

            if (!hitTimes.empty()) {

               int bestCount = 0;
               double bestWindowStart = hitTimes.front();
               double bestWindowEnd   = hitTimes.front() + mrdDisplayWindow;

               size_t j = 0;
               for (size_t i = 0; i < hitTimes.size(); ++i) {

                  while (j < hitTimes.size() && hitTimes[j] <= hitTimes[i] + mrdDisplayWindow) {
                     ++j;
                  }

                  int count = static_cast<int>(j - i);

                  if (count > bestCount) {
                     bestCount = count;
                     bestWindowStart = hitTimes[i];
                     bestWindowEnd   = hitTimes[i] + mrdDisplayWindow;
                  }
               }

               displayMinTime = bestWindowStart;
               displayMaxTime = bestWindowEnd;

               // Now keep only hits inside that best window
               for (const MRDHit& hit : bestMRDCluster->GetHits()) {

                  int    detId = hit.GetDetID();
                  double t     = hit.GetTime();

                  if (t < displayMinTime || t > displayMaxTime) continue;

                  auto it = mrdChanToIndex.find(detId);
                  if (it == mrdChanToIndex.end()) continue;

                  size_t idx = it->second;

                  // Keep earliest hit time for each bar within the selected window
                  if (!mrdHasHit[idx] || t < mrdBarHitTime[idx]) {
                     mrdBarHitTime[idx]   = t;
                     mrdHasHit[idx] = true;
                  }
               }
            }
         }
      }

      double minMRDHitTime = displayMinTime;
      double maxMRDHitTime = displayMaxTime;

      if (maxMRDHitTime <= minMRDHitTime) {
         minMRDHitTime = 0.0;
         maxMRDHitTime = 1.0;
      }




      // ========================================
      // MRD Visualization
      // ========================================

   
      //  3.1 MRD colorbar (time)
       
      pMRDColorBar->cd();
      

      TH2D* hMRDColorBar = CreateColorScale("hMRDColorBar",
                                               "MRD time [ns]",
                                               minMRDHitTime,
                                               maxMRDHitTime,
                                               0.17,
                                               0.17,
                                               2.2);

      hMRDColorBar->Draw("COL");
      
      gPad->SetTicks(0,0);
      gPad->SetFrameLineColor(0);
      gPad->SetLineColor(0);
      
      
   
      // 3.2. MRD SIDE VIEW  (orientation = 0, Zdraw vs Y, renk = time)
     
      pMRDSide->cd();
      
      
      pMRDSide->SetLeftMargin(0.005);
      pMRDSide->SetRightMargin(0.00);
     //pMRDSide->SetTopMargin(0.07);
      //pMRDSide->SetBottomMargin(0.00);
      

      // Ayarlanabilir offsetler
      const double layerShift = 400.0;   // layerlar arası Z kayması [cm]
      const double sideShift  = 300.0;   // side 0–1 arası Z kayması [cm].Yanyana olan barlar.bunu degistirince layerShift de degis

      // Box boyutu parametreleri
      const double boxScaleZ = 0.03;     // Z boyutu = aralığın %2'si
      const double boxScaleY = 0.03;     // Y boyutu = aralığın %4'ü

      double zMinS = +1e9, zMaxS = -1e9;
      double yMinS = +1e9, yMaxS = -1e9;

      // 1) Çizim aralığını hesapla (Zdraw ve Y)
      for (size_t k = 0; k < mrdBars.size(); ++k) {
         const MRDInfo& m = mrdBars[k];
         if (m.orientation != 0) continue;  // side-view: orientation=0

         double zDraw = m.z_center
                      + m.layer * layerShift
                      + m.side  * sideShift;
         double y     = m.y_center;

         zMinS = std::min(zMinS, zDraw);
         zMaxS = std::max(zMaxS, zDraw);
         yMinS = std::min(yMinS, y);
         yMaxS = std::max(yMaxS, y);
      }

      if (zMinS < 1e8) {

         // Barlar kenara yapışmasın diye %10 boşluk bırak.
         double dzS = 0.1 * (zMaxS - zMinS + 1e-6);
         double dyS = 0.1 * (yMaxS - yMinS + 1e-6);
         zMinS -= dzS;   
         zMaxS += dzS;
         yMinS -= dyS;   
         yMaxS += dyS;


         // ------------------------------------------------------------
         // Invisible frame histogram for MRD Side View
         //
         // This TH2D is NOT used to display data.
         // Its sole purpose is to:
         //   1) Define the world coordinate system (Z vs Y) for the pad
         //   2) Set the drawing ranges (zMinS–zMaxS, yMinS–yMaxS)
         //   3) Provide a reference frame so that TBox / TLatex objects
         //      (MRD bars and channel labels) can be positioned correctly
         //
         // All axis labels, ticks, and frame lines are disabled below
         // so that the histogram itself is completely invisible in the
         // final event display.
         //
         // IMPORTANT:
         //   Removing this histogram will also remove the coordinate
         //   system, causing MRD bars to disappear.
         // ------------------------------------------------------------

         TH2D* hMRDSide = new TH2D("hMRDSide", ";Z [cm];Y [cm]",
                                   10, zMinS, zMaxS,
                                   10, yMinS, yMaxS);
       
         hMRDSide->SetDirectory(nullptr);
         hMRDSide->Draw();
         
         // eksenleri kapat
         hMRDSide->GetXaxis()->SetLabelSize(0);
         hMRDSide->GetXaxis()->SetTitleSize(0);
         hMRDSide->GetXaxis()->SetTickLength(0);

         hMRDSide->GetYaxis()->SetLabelSize(0);
         hMRDSide->GetYaxis()->SetTitleSize(0);
         hMRDSide->GetYaxis()->SetTickLength(0);

         // frame çizgisini kapat
         gPad->SetFrameLineColor(0);
         gPad->SetFrameLineWidth(0);


         // Kutu boyutları
         double halfZ = boxScaleZ * (zMaxS - zMinS);
         double halfY = boxScaleY * (yMaxS - yMinS);

         int nContours = gStyle->GetNumberContours();

         // 2) Barları çiz
         for (size_t k = 0; k < mrdBars.size(); ++k) {
            const MRDInfo& m = mrdBars[k];
            if (m.orientation != 0) continue;

            // Bu barda hit yoksa: boş çiz (sadece çerçeve)
            if (!mrdHasHit[k]) {
               double zDraw = m.z_center
                            + m.layer * layerShift
                            + m.side  * sideShift;
               double y     = m.y_center;

               TBox* b = new TBox(zDraw - halfZ, y - halfY,
                                  zDraw + halfZ, y + halfY);
               b->SetFillColor(kBlack);
               b->SetFillStyle(0);
               b->SetLineColor(kGray+2);
               b->SetLineWidth(1);
               b->Draw("same");
               
               TLatex* tnum = new TLatex(zDraw, y, Form("%d", m.channel_num));
               tnum->SetTextAlign(22);      // kutunun ortası
               tnum->SetTextFont(42);
               tnum->SetTextSize(0.04);    // küçük tut
               tnum->SetTextColor(kWhite); // hit yok → gri yazı
               tnum->Draw("same");
               
               continue;
            }

            double tHit = mrdBarHitTime[k];  // en erken time

            // 0–1 aralığına normalize et (minMRDHitTime–maxMRDHitTime arasında)
            
            double u = (maxMRDHitTime > minMRDHitTime)
               ? (tHit - minMRDHitTime) / (maxMRDHitTime - minMRDHitTime)
               : 0.0;
                                 
            if (u < 0.0) u = 0.0;
            if (u > 1.0) u = 1.0;

            int paletteBin = int( u * (nContours - 1) );
            int col = gStyle->GetColorPalette(paletteBin);

            double zDraw = m.z_center
                         + m.layer * layerShift
                         + m.side  * sideShift;
            double y     = m.y_center;

            TBox* b = new TBox(zDraw - halfZ, y - halfY,
                               zDraw + halfZ, y + halfY);

            b->SetFillColor(col);
            b->SetFillStyle(1001);
            b->SetLineColor(col);
            b->SetLineWidth(1);
            b->Draw("same");
            
            
            TLatex* tnum = new TLatex(zDraw, y, Form("%d", m.channel_num));
            tnum->SetTextAlign(22);
            tnum->SetTextFont(42);
            tnum->SetTextSize(0.04);
            tnum->SetTextColor(kWhite);
            tnum->Draw("same");

            
         }

         // Başlık
         TLatex* labSide = new TLatex(zMinS, yMaxS + 0.05*(yMaxS-yMinS),
                                      "Side view");
         labSide->SetTextColor(kWhite);
         labSide->SetTextSize(0.05);
         labSide->Draw("same");
      }


      
      //  3.3 MRD TOP VIEW  (orientation = 1, Zdraw vs X, renk = time)
      //    - X ekseni: Z (shift'li)
      //    - Y ekseni: X
      pMRDTop->cd();
      
      pMRDTop->SetLeftMargin(0.005);
      pMRDTop->SetRightMargin(0.00);
      //pMRDTop->SetTopMargin(0.00);
      //pMRDTop->SetBottomMargin(0.00);
      
      
      
      double zMinT = +1e9, zMaxT = -1e9;
      double xMinT = +1e9, xMaxT = -1e9;

      // 1) Çizim aralığını hesapla (Zdraw ve X)
      for (size_t k = 0; k < mrdBars.size(); ++k) {
         const MRDInfo& m = mrdBars[k];
         if (m.orientation != 1) continue;  // top-view: orientation=1

         double zDraw = m.z_center
                      + m.layer * layerShift
                      + m.side  * sideShift;
         double x     = m.x_center;

         if (zDraw < zMinT) zMinT = zDraw;
         if (zDraw > zMaxT) zMaxT = zDraw;
         if (x     < xMinT) xMinT = x;
         if (x     > xMaxT) xMaxT = x;
      }

      if (zMinT < 1e8) {

         // Biraz marj
         double dzT = 0.15 * (zMaxT - zMinT + 1e-6);
         double dxT = 0.15 * (xMaxT - xMinT + 1e-6);
         zMinT -= dzT;   zMaxT += dzT;
         xMinT -= dxT;   xMaxT += dxT;

         TH2D* hMRDTop = new TH2D("hMRDTop", ";Z [cm];X [cm]",
                                  10, zMinT, zMaxT,   // X ekseni = Zdraw
                                  10, xMinT, xMaxT);  // Y ekseni = X
         hMRDTop->SetDirectory(nullptr);
         hMRDTop->Draw();

         hMRDTop->GetXaxis()->SetLabelSize(0);
         hMRDTop->GetXaxis()->SetTitleSize(0);
         hMRDTop->GetXaxis()->SetTickLength(0);

         hMRDTop->GetYaxis()->SetLabelSize(0);
         hMRDTop->GetYaxis()->SetTitleSize(0);
         hMRDTop->GetYaxis()->SetTickLength(0);

         // frame çizgisini kapat
         gPad->SetFrameLineColor(0);
         gPad->SetFrameLineWidth(0);

         
         // Box boyutları: aynı oranlar, bu sefer X aralığına göre halfY
         double boxScaleX = 0.03;
         double halfZ_T = boxScaleZ * (zMaxT - zMinT);  // yatay (Z)
         double halfX_T = boxScaleX * (xMaxT - xMinT);  // dikey (X)

         int nContoursTop = gStyle->GetNumberContours();

         // 2) Barları çiz
         for (size_t k = 0; k < mrdBars.size(); ++k) {
            const MRDInfo& m = mrdBars[k];
            if (m.orientation != 1) continue; 

            double zDraw = m.z_center
                         + m.layer * layerShift
                         + m.side  * sideShift;
                         
            double xScaleTop = 1.5;             
            double x     = xScaleTop*m.x_center;

            // Bu barda hit yoksa: boş kutu
            if (!mrdHasHit[k]) {
               TBox* b = new TBox(zDraw - halfZ_T, x - halfX_T,
                                  zDraw + halfZ_T, x + halfX_T);
               b->SetFillColor(kBlack);
               b->SetFillStyle(0);
               b->SetLineColor(kGray+2);
               b->SetLineWidth(1);
               b->Draw("same");
               
               
               TLatex* tnum = new TLatex(zDraw, x, Form("%d", m.channel_num));
               tnum->SetTextAlign(22);
               tnum->SetTextFont(42);
               tnum->SetTextSize(0.037);
               tnum->SetTextColor(kWhite);            // boş kutuda gri yazı
               tnum->Draw("same");


               
               
               continue;
            }

            double tHit = mrdBarHitTime[k];  // en erken time

            // 0–1 aralığına normalize et (minMRDHitTime–maxMRDHitTime arasında)
            double u = (maxMRDHitTime > minMRDHitTime) ? ( (tHit - minMRDHitTime) / (maxMRDHitTime - minMRDHitTime) ) : 0.0;
            if (u < 0.0) u = 0.0;
            if (u > 1.0) u = 1.0;

            int paletteBin = int( u * (nContoursTop - 1) );
            int col = gStyle->GetColorPalette(paletteBin);

            TBox* b = new TBox(zDraw - halfZ_T, x - halfX_T,
                               zDraw + halfZ_T, x + halfX_T);
            b->SetFillColor(col);
            b->SetFillStyle(1001);
            b->SetLineColor(col);
            b->SetLineWidth(1);
            b->Draw("same");
            
            TLatex* tnum = new TLatex(zDraw, x, Form("%d", m.channel_num));
            tnum->SetTextAlign(22);                 // center
            tnum->SetTextFont(42);
            tnum->SetTextSize(0.037);               // ayarla
            tnum->SetTextColor(kWhite);             // renkli kutu üstünde beyaz iyi
            tnum->Draw("same");

            
         }

         // Başlık
         TLatex* labTop = new TLatex(zMinT, xMaxT + 0.05*(xMaxT-xMinT),
                                     "Top view");
         labTop->SetTextColor(kWhite);
         labTop->SetTextSize(0.05);
         labTop->Draw("same");
      }
      
      
      // ========================================
      // 4. FMV subsystem
      // ========================================
      
      // FMV display data
      // FMV: Collect time-correlated hit per bar relative to best PMT cluster
      // Keep, for each bar, the hit whose deltaT is inside the window
      // and closest to the center of the window (dtFMVPMTHistPeak).
      // -----------------------------
      std::vector<double> fmvTime(fmvBars.size(), 1e9);
      std::vector<bool>   fmvHasHit(fmvBars.size(), false);
      std::vector<double> fmvBestScore(fmvBars.size(), 1e9);   // |deltaT - 800|

      EFMVHits eFMVHits;
      fmvHitReader.ReadEntry(i, eFMVHits);

      // These settings may need to be adjusted after reviewing the histogram of 
      // FMV PMT difference.
      double dtFMVPMTMin    = iniReader.GetReal("fmv","dtFMVPMTMin", 700);
      double dtFMVPMTMax    = iniReader.GetReal("fmv","dtFMVPMTMax", 800);
      double dtFMVPMTHistPeak = iniReader.GetReal("fmv","dtFMVPMTHistPeak", 750);
      
      for (const FMVHit& hit : eFMVHits.GetHits()) {

         int    detId = hit.GetDetID();
         double t     = hit.GetTime();   // [ns]

         auto it = fmvChanToIndex.find(detId);
         if (it == fmvChanToIndex.end()) continue;

         size_t idx = it->second;

         double deltaT = t - bestTankClusterTime;

         // Time-coincidence window relative to best PMT cluster
         if (deltaT <= dtFMVPMTMin || deltaT >= dtFMVPMTMax ) continue;

         // Choose the hit closest to the center of the window: dtFMVPMTHistPeak
         double score = std::fabs(deltaT - dtFMVPMTHistPeak);

         if (!fmvHasHit[idx] || score < fmvBestScore[idx]) {
            fmvTime[idx]      = t;
            fmvHasHit[idx]    = true;
            fmvBestScore[idx] = score;
         }
      }

      // FMV min/max time (for color scale)
      double minTimeFMV = 1e9;
      double maxTimeFMV = -1e9;

      for (size_t k = 0; k < fmvBars.size(); ++k) {
         if (!fmvHasHit[k]) continue;
         double t = fmvTime[k];
         if (t < minTimeFMV) minTimeFMV = t;
         if (t > maxTimeFMV) maxTimeFMV = t;
      }

      if (minTimeFMV > maxTimeFMV) {
         minTimeFMV = 0.0;
         maxTimeFMV = 1.0;
      }

      // single-hit / zero-range case
      if (std::fabs(maxTimeFMV - minTimeFMV) < 1e-9) {
         minTimeFMV -= 1.0;
         maxTimeFMV += 1.0;
      }

      
      // ========================================
      // FMV Visualization
      // ========================================
      
     
      //  4.1. FMV colorbar (time)
      
      pFMVColorBar->cd();



      TH2D* hFMVColorBar = CreateColorScale("hFMVTimeColorScale",
                                            "FMV time [ns]",
                                            minTimeFMV,
                                            maxTimeFMV,
                                            0.17,
                                            0.17,
                                            2.2);

      hFMVColorBar->Draw("COL");
   
   

      gPad->SetTicks(0,0);
      gPad->SetFrameLineColor(kBlue);
      gPad->SetLineColor(0.9);
      
     
      
      //  4.2. FMV side view  (grid layout, only channel_num + side)
      
      pFMVSide->cd();

      // ---- layout parameters you control ----
      const double zLeft   = 0.0;     // left column Z
      const double colGap  = 1.0;     // distance between columns in Z units (cm or arbitrary)
      const double y0      = 0.0;     // bottom Y start
      const double rowStep = 1.0;     // distance between rows in Y units
      const double halfZ   = 0.35;    // half width of a box in Z
      const double halfY   = 0.35;    // half height of a box in Y

      // how many rows per column
      const int nRows = 13;           // 0-12 and 13-25 => 13 rows

      // ---- define drawing range from our grid (NOT from geometry file) ----
      double zMinFS = zLeft - 1.2*halfZ;
      double zMaxFS = (zLeft + colGap) + 1.2*halfZ;
      double yMinFS = y0 - 1.2*halfY;
      double yMaxFS = y0 + (nRows-1)*rowStep + 1.2*halfY;

      TH2D* hFMVSide = new TH2D("hFMVSide", ";Z [arb];Y [arb]",
                                10, zMinFS, zMaxFS,
                                10, yMinFS, yMaxFS);
                                
      hFMVSide->SetDirectory(nullptr);
      hFMVSide->Draw();

      // hide axes/frame like before
      hFMVSide->GetXaxis()->SetLabelSize(0);
      hFMVSide->GetXaxis()->SetTitleSize(0);
      hFMVSide->GetXaxis()->SetTickLength(0);
      hFMVSide->GetYaxis()->SetLabelSize(0);
      hFMVSide->GetYaxis()->SetTitleSize(0);
      hFMVSide->GetYaxis()->SetTickLength(0);
      gPad->SetFrameLineColor(0);
      gPad->SetFrameLineWidth(0);

      int nContoursF = gStyle->GetNumberContours();

      // -----------------------------
      // helper lambda: map channel -> (col,row)
      // Default mapping to match your view:
      //   ch 0..12  -> left column,  row = ch
      //   ch 13..25 -> right column, row = ch-13
      // If you want to FORCE using m.side instead, see comment below.
      // -----------------------------
      auto channel_to_col_row = [&](int ch, int side, int& col, int& row) {
        // Option A (recommended): follow channel numbering pattern (0-12 left, 13-25 right)
        if (ch <= 12) { col = 0; row = ch; }
        else          { col = 1; row = ch - 13; }

        // Option B (force side): uncomment if side info is always correct and you prefer it
        /*
        col = (side == 1) ? 1 : 0;
        if (col == 0) row = ch;        // assumes left side channels are 0..12
        else          row = ch - 13;   // assumes right side channels are 13..25
        */
      };

      // -----------------------------
      // draw all FMV bars using our grid positions
      // -----------------------------
      for (size_t k = 0; k < fmvBars.size(); ++k) {
        const MRDInfo& m = fmvBars[k];
        if (m.orientation != 0) continue;

        int col = 0, row = 0;
        channel_to_col_row(m.channel_num, m.side, col, row);

        // safety: skip if out of expected range
        if (row < 0 || row >= nRows) continue;

        double zDraw = zLeft + col * colGap;
        double yDraw = y0    + row * rowStep;

        // no hit -> outline
        if (!fmvHasHit[k]) {
          TBox* b = new TBox(zDraw - halfZ, yDraw - halfY, zDraw + halfZ, yDraw + halfY);
          b->SetFillColor(kBlack);
          b->SetFillStyle(0);
          b->SetLineColor(kGray+2);
          b->SetLineWidth(1);
          b->Draw("same");

          TLatex* tnum = new TLatex(zDraw, yDraw, Form("%d", m.channel_num));
          tnum->SetTextAlign(22);
          tnum->SetTextFont(42);
          tnum->SetTextSize(0.10);      // grid is small -> increase if needed
          tnum->SetTextColor(kWhite);
          tnum->Draw("same");
          continue;
        }

        // hit -> color by time
        double tHit = fmvTime[k];
        double u = (maxTimeFMV > minTimeFMV) ? ((tHit - minTimeFMV) / (maxTimeFMV - minTimeFMV)) : 0.0;
        u = std::max(0.0, std::min(1.0, u));

        int paletteBin = int(u * (nContoursF - 1));
        int colr = gStyle->GetColorPalette(paletteBin);

        TBox* b = new TBox(zDraw - halfZ, yDraw - halfY, zDraw + halfZ, yDraw + halfY);
        b->SetFillColor(colr);
        b->SetFillStyle(1001);
        b->SetLineColor(colr);
        b->SetLineWidth(1);
        b->Draw("same");

        TLatex* tnum = new TLatex(zDraw, yDraw, Form("%d", m.channel_num));
        tnum->SetTextAlign(22);
        tnum->SetTextFont(42);
        tnum->SetTextSize(0.10);
        tnum->SetTextColor(kWhite);
        tnum->Draw("same");
      }

      // title
      TLatex* lab = new TLatex(zMinFS, yMaxFS + 0.05*(yMaxFS-yMinFS),
                               "FMV Side view");
      lab->SetTextColor(kWhite);
      lab->SetTextSize(0.12);
      lab->Draw("same");

 
      //Title. it shows run and event number
      pEventDisplayTitle->cd();
      TPaveText* eventDisplayTitlePT = new TPaveText(0.0, 0.0, 1.0, 1.0, "NDC");
      eventDisplayTitlePT->SetFillStyle(0);
      eventDisplayTitlePT->SetBorderSize(0);
      eventDisplayTitlePT->SetTextAlign(22);   // center-center
      eventDisplayTitlePT->SetTextFont(42);
      eventDisplayTitlePT->SetTextColor(kOrange-3);
      eventDisplayTitlePT->SetTextSize(0.65);
                          
      if (useMC) {
         eventDisplayTitlePT->AddText(Form("Run %d, Event %lu",
         0, //no run number for MC
         mcEventReader.GetMCEventNum() ) );
      } else {
         eventDisplayTitlePT->AddText(Form("Run %d, Event %d",
         eventReader.GetRunNumber(),
         eventReader.GetEventNumber() ) );
      }                                       
      
      eventDisplayTitlePT->Draw();

      
      // Upper right info pad
      pUpperRightInfo->cd();
      if (useMC)
      {
       
         //TPaveText* upperRightPT = new TPaveText(0.05, 0.05, 0.95, 0.95, "NDC");
         TPaveText* upperRightPT = new TPaveText(0.02, 0.02, 0.98, 0.98, "NDC");
         upperRightPT->SetFillColor(kBlack);
         upperRightPT->SetFillStyle(0);
         upperRightPT->SetBorderSize(0);
         upperRightPT->SetTextAlign(12);
         upperRightPT->SetTextColor(kBlack);
         upperRightPT->SetTextFont(42);
         //upperRightPT->SetTextSize(0.055);
         upperRightPT->SetTextSize(0.070);
         
         
         // --- True muon info from MCANNIEEventReader ---
         int ccFlag  =         mcEventReader.GetTrueCC();
         int mrdCoinc =        mcEventReader.GetTankMRDCoinc(); 
         double trueMuonE    = mcEventReader.GetTrueMuonEnergy();       // [GeV]
         double trueTheta    = mcEventReader.GetTrueAngle();            // [rad]
         double truePhi      = mcEventReader.GetTruePhi();              // [rad]
         truePhi = truePhi * (180.0 / M_PI);
         double trueLWater   = mcEventReader.GetTrueTrackLengthInWater()*100; // [cm]
         double trueLMRD     = mcEventReader.GetTrueTrackLengthInMRD();   // [cm]
         //int multiRing       = mcEventReader.GetTrueMultiRing(); 
         double trueVtxX     = mcEventReader.GetTrueVtxX();
         double trueVtxY     = mcEventReader.GetTrueVtxY();
         double trueVtxZ     = mcEventReader.GetTrueVtxZ();
         double trueVtxT     = mcEventReader.GetTrueVtxTime();   // [ns]
         double trueDirX     = mcEventReader.GetTrueDirX();
         double trueDirY     = mcEventReader.GetTrueDirY();
         double trueDirZ     = mcEventReader.GetTrueDirZ();
         
         
         TText* t = upperRightPT->AddText("True #mu info:");
         t->SetTextFont(62);  // 62 = bold font
         
         const char* ccLabel = (ccFlag == 1) ? "CC" : "NC";
         upperRightPT->AddText(Form("Interaction = %s ", ccLabel));
         upperRightPT->AddText(Form("Tank-MRD coinc = %d", mrdCoinc));
         upperRightPT->AddText(Form("E_{#mu} = %.2f MeV", trueMuonE));
         upperRightPT->AddText(Form("Vtx = (%.2f, %.2f, %.2f) cm", 
                              trueVtxX, trueVtxY, trueVtxZ));
         //upperRightPT->AddText(Form("Vtx time = %.1f ns", trueVtxT));
         upperRightPT->AddText(Form("Dir = (%.2f, %.2f, %.2f)",
                              trueDirX, trueDirY, trueDirZ));
         upperRightPT->AddText(Form("#theta_{#mu} = %.1f deg", trueTheta));
         upperRightPT->AddText(Form("#phi_{#mu} = %.1f deg",   truePhi));
         upperRightPT->AddText(Form("L_{water} = %.2f cm",  trueLWater));
         upperRightPT->AddText(Form("L_{MRD}   = %.1f cm", trueLMRD));
         //upperRightPT->AddText(Form("Multi-ring: %d", multiRing));
         
         
         upperRightPT->Draw("same");

      }else
      {
         
         DrawDetectorSketch(); 
      }
      pUpperRightInfo->Draw();
      
      // === Draw event info box on the canvas ===
      pUpperLeftInfo->cd();  // go to the canvas (not a sub-pad)
      TPaveText* upperLeftPT = new TPaveText(0.001, 0.001, 0.99, 0.99, "NDC");
      upperLeftPT->SetFillColor(kBlack);
      upperLeftPT->SetFillStyle(0);
      upperLeftPT->SetBorderSize(0);
      upperLeftPT->SetTextAlign(12);
      upperLeftPT->SetTextColor(kBlack);
      upperLeftPT->SetTextFont(42);
      upperLeftPT->SetTextSize(0.070);
      //upperLeftPT->SetTextSize(0.05);
      //upperLeftPT->SetTextSize(0.035);
   
      /*
      if(bestTankCluster)
         upperLeftPT->AddText(Form("Tank cluster time (ns): %.2f", bestTankCluster->GetClusterTime() ) );
      
      if(bestMRDCluster)
         upperLeftPT->AddText(Form("MRD cluster time (ns): %.2f", selectedMRDClusterTime ) );
       */
       
       
      // Tank
      if (bestTankCluster)
         upperLeftPT->AddText(Form("Tank cluster time (ns): %.2f",
                                   bestTankCluster->GetClusterTime()));
      else
         upperLeftPT->AddText(Form("Tank cluster time (ns): No cluster in [%.0f, %.0f] ns",
                                   clusterTimeMin, clusterTimeMax));

      // MRD
      if (bestMRDCluster)
         upperLeftPT->AddText(Form("MRD cluster time (ns): %.2f",
                                   selectedMRDClusterTime));
      else
         upperLeftPT->AddText("MRD cluster time (ns): No correlated cluster");
         
      upperLeftPT->AddText(Form("MRD display window: %.0f ns (%.0f - %.0f ns)",
                   mrdDisplayWindow,
                   displayMinTime,
                   displayMaxTime));   
      
       upperLeftPT->AddText(Form("%.0f ns < #Delta t_{MRD,PMT} < %.0f ns",
                      dtMRDPMTMin, dtMRDPMTMax));

      upperLeftPT->AddText(Form("%.0f ns < #Delta t_{FMV,PMT} < %.0f ns",
                         dtFMVPMTMin, dtFMVPMTMax));
      
      
      
      upperLeftPT->AddText(Form("PMTs triggered (PE > %.1f): %d",tankPMTsPEThreshold, numTriggeredPMTs));
      upperLeftPT->AddText(Form("LAPPDs triggered (# of hits > %d): %d", lappdHitThreshold, numTriggeredLAPPDs));

      upperLeftPT->Draw("same");

      
      
      c->Modified();
      
      c->Update();
      
      
      bool isFirstToPrint = (nPrinted == 0);
      bool isLastToPrint  = (i == nEntries - 1);

      if (isFirstToPrint && isLastToPrint) {
         c->Print(outPdfName); // sadece tek sayfalık normal PDF
      }else if (isFirstToPrint) {
         c->Print(outPdfName + "(");
      }else if (isLastToPrint) {
         c->Print(outPdfName + ")");
      }else {
         c->Print(outPdfName);
      }


      ++nPrinted;
      delete c;

      
   } //end of event loop
   
   
   std::cout<<"Event loop finished "<<std::endl;
   

} //end of displayEvent function



//==================================================

// ===============================================================
// CreateColorScale
//
// Amaç:
//   ROOT'ta doğrudan color bar (legend) çizmek zor olduğu için,
//   "fake" bir TH2D histogram oluşturarak dikey bir renk skalası üretir.
//   Bu histogram gerçek veri içermez, sadece renk → değer mapping'ini gösterir.
//
// Nasıl çalışır:
//   - X ekseni: anlamsız (1 bin) → sadece placeholder
//   - Y ekseni: minVal → maxVal → fiziksel değer aralığı
//   - Bin content: min → max arasında lineer doldurulur
//   - Draw("COL"): bu içerikleri renge çevirir → dikey gradient oluşur
//
// ROOT Trick:
//   ROOT'ta renk = bin content olduğu için,
//   yapay olarak düzgün artan değerler vererek color bar elde ediyoruz.
//
// Güvenlik:
//   minVal < maxVal olmalı.
//   Eğer değilse (örneğin threshold üstü hit yoksa),
//   otomatik olarak küçük bir aralık açılır.
//
// Kullanım:
//   TH2D* h = CreateColorScale(...);
//   h->Draw("COL");
//
// Not:
//   Bu histogram veri değildir, sadece görselleştirme aracıdır.
// ===============================================================

TH2D* CreateColorScale(const std::string& histName,
                       const std::string& yTitle,
                       double minVal,
                       double maxVal,
                       double yLabelSize,
                       double yTitleSize,
                       double yTitleOffset,
                       int nYBins  )
{
    TH2D* h = new TH2D(histName.c_str(), "",
                       1, 0.0, 1.0,
                       nYBins, minVal, maxVal);

    h->SetDirectory(nullptr);

    // Full gradient fill
    for (int by = 1; by <= nYBins; ++by) {
        double frac = (nYBins == 1) ? 0.0 : double(by - 1) / double(nYBins - 1);
        double val  = minVal + (maxVal - minVal) * frac;
        h->SetBinContent(1, by, val);
    }

    // Hide X axis
    h->GetXaxis()->SetLabelSize(0);
    h->GetXaxis()->SetTitleSize(0);
    h->GetXaxis()->SetTickLength(0);

    // Y axis style
    h->GetYaxis()->SetLabelSize(yLabelSize);
    h->GetYaxis()->SetTitleSize(yTitleSize);
    h->GetYaxis()->SetTitleOffset(yTitleOffset);
    h->GetYaxis()->SetTitle(yTitle.c_str());

    h->SetMinimum(minVal);
    h->SetMaximum(maxVal);

    return h;
}

//==================================================

void DrawDetectorSketch()
{
 
   // ---------- Title ----------
   TLatex title;
   title.SetTextAlign(22);   // center
   title.SetTextSize(0.12);  // biraz büyük
   title.SetTextFont(62);    // bold
   title.SetTextColor(kRed-2);

   // üst orta
   title.DrawLatex(0.5, 0.90, "ANNIE");
     
   // ---------- FMV ----------
   TBox* fmv = new TBox(0.16, 0.18, 0.22, 0.62);
   fmv->SetFillColor(kBlack);
   fmv->SetLineColor(kGray+2);
   fmv->SetLineWidth(2);
   fmv->Draw();

   // ---------- Tank ----------
   TEllipse* tank = new TEllipse(0.47, 0.42, 0.22, 0.22);
   tank->SetFillColor(kBlack);
   tank->SetLineColor(kGray+2);
   //tank->SetLineWidth(2);
   tank->Draw();

   // ---------- MRD ----------
   TBox* mrd = new TBox(0.70, 0.18, 0.92, 0.62);
   mrd->SetFillColor(kBlack);
   mrd->SetLineColor(kGray+2);
   mrd->SetLineWidth(2);
   mrd->Draw();

   // ---------- Labels ----------
   TLatex latex;
   latex.SetTextAlign(22);
   latex.SetTextSize(0.09);
   latex.SetTextFont(42);
   latex.SetTextColor(kBlack);

   latex.DrawLatex(0.15, 0.10, "FMV");
   latex.DrawLatex(0.47, 0.10, "Tank");
   latex.DrawLatex(0.81, 0.10, "MRD");

}


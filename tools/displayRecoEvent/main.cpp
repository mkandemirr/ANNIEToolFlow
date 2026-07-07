// displayRecoEvent.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TH2D.h"
#include "TBox.h"
#include "TEllipse.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TLine.h"

#include "RootTreeReader.h"
#include "MRDClusterReader.h"
#include "INIReader.h"
#include "TMarker.h"

#include "ANNIEEventReader.h"
#include "MCANNIEEventReader.h"

#include "MRDRecoReader.h"
#include "EMRDRecos.h"
#include "MRDReco.h"

#include "TArrow.h"
#include "TLegend.h"
#include "TPaveText.h"

// ==================================================
// MRD bar info
// ==================================================
struct MRDInfo {
    int channel_num;
    int detector_system;   // 0 = FMV, !=0 = MRD
    int orientation;       // 0 = side view, 1 = top view
    int layer;
    int side;
    int num;

    double x_center;       // [cm]
    double y_center;       // [cm]
    double z_center;       // [cm]
};

// ==================================================
// Read MRD geometry
// ==================================================
std::vector<MRDInfo> LoadMRDInfo(const std::string& mrdFile)
{
    std::vector<MRDInfo> mrdBars;

    std::ifstream infile(mrdFile.c_str());
    if (!infile.is_open()) {
        std::cerr << "[LoadMRDInfo] ERROR: Cannot open " << mrdFile << std::endl;
        return mrdBars;
    }

    std::string line;
    std::getline(infile, line); // header

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

        if (!iss) continue;

        mrdBars.push_back(m);
    }

    std::cout << "[LoadMRDInfo] Loaded " << mrdBars.size()
              << " MRD/FMV bars" << std::endl;

    return mrdBars;
}

// ==================================================
// Split FMV and MRD
// ==================================================
void SplitMRDByDetectorSystem(const std::vector<MRDInfo>& allMRD,
                              std::vector<MRDInfo>& fmvBars,
                              std::vector<MRDInfo>& mrdBars)
{
    fmvBars.clear();
    mrdBars.clear();

    for (const auto& m : allMRD) {
        if (m.detector_system == 0)
            fmvBars.push_back(m);
        else
            mrdBars.push_back(m);
    }

    std::cout << "[SplitMRDByDetectorSystem] MRD bars = "
              << mrdBars.size() << std::endl;
}

// ==================================================
// Simple vertical color scale
// ==================================================
TH2D* CreateColorScale(const std::string& name,
                       const std::string& title,
                       double minVal,
                       double maxVal)
{
    TH2D* h = new TH2D(name.c_str(), "", 1, 0, 1, 100, minVal, maxVal);
    h->SetDirectory(nullptr);
    h->SetStats(0);

    for (int iy = 1; iy <= 100; ++iy)
        h->SetBinContent(1, iy, h->GetYaxis()->GetBinCenter(iy));

    h->GetXaxis()->SetLabelSize(0);
    h->GetXaxis()->SetTickLength(0);

    h->GetYaxis()->SetTitle(title.c_str());
    h->GetYaxis()->SetLabelColor(kWhite);
    h->GetYaxis()->SetTitleColor(kWhite);
    h->GetYaxis()->SetLabelSize(0.09);
    h->GetYaxis()->SetTitleSize(0.10);
    h->GetYaxis()->SetTitleOffset(0.65);

    return h;
}

// ==================================================
// Draw tank in side view: Y vs Z
// Cylinder axis is Y
// ==================================================
void DrawTankSide(double zc, double yc,
                  double radiusZ,
                  double halfHeightY)
{
    double zMin = zc - radiusZ;
    double zMax = zc + radiusZ;
    double yMin = yc - halfHeightY;
    double yMax = yc + halfHeightY;

    TBox* body = new TBox(zMin, yMin, zMax, yMax);
    body->SetFillStyle(0);
    body->SetLineColor(kGray+1);
    body->SetLineWidth(2);
    body->Draw("same");

    TEllipse* top = new TEllipse(zc, yMax, radiusZ, 0.12 * halfHeightY);
    top->SetFillStyle(0);
    top->SetLineColor(kGray+1);
    top->SetLineWidth(2);
    top->Draw("same");

    TEllipse* bottom = new TEllipse(zc, yMin, radiusZ, 0.12 * halfHeightY);
    bottom->SetFillStyle(0);
    bottom->SetLineColor(kGray+1);
    bottom->SetLineWidth(2);
    bottom->Draw("same");

    TLine* axis = new TLine(zc, yMin, zc, yMax);
    axis->SetLineColor(kGray+1);
    axis->SetLineStyle(2);
    axis->Draw("same");
}

// ==================================================
// Draw tank in top view: X vs Z
// ==================================================
void DrawTankTop(double zc, double xc, double radius)
{
    TEllipse* tank = new TEllipse(zc, xc, radius, radius);
    tank->SetFillStyle(0);
    tank->SetLineColor(kGray+1);
    tank->SetLineWidth(2);
    tank->Draw("same");

    TLine* lx = new TLine(zc - radius, xc, zc + radius, xc);
    lx->SetLineColor(kGray+1);
    lx->SetLineStyle(2);
    lx->Draw("same");

    TLine* lz = new TLine(zc, xc - radius, zc, xc + radius);
    lz->SetLineColor(kGray+1);
    lz->SetLineStyle(2);
    lz->Draw("same");
}

// ==================================================
// Main display
// ==================================================
void displayRecoEvent(const INIReader& ini)
{
   // --------------------------------------------------
// Configuration parameters
// --------------------------------------------------

   // Input files
   std::string rootFile =
       ini.Get("input", "rootFile", "");

   const bool useMC =
       ini.GetBoolean("input", "useMC", false);

   // Event selection
   int maxEventNum =
       ini.GetInteger("event", "maxEventNum", -1);
       
   std::string recoAlgorithm =
    ini.Get("Reco", "RecoAlgorithm", "Reco");

   // Output
   TString outPdf =
       ini.Get("output", "pdfFileName", "displayRecoEvent.pdf");

   // Tank geometry [cm]
   const double tankCenterX =
       ini.GetReal("tank", "centerX", 0.0);

   const double tankCenterY =
       ini.GetReal("tank", "centerY", -14.45);

   const double tankCenterZ =
       ini.GetReal("tank", "centerZ", 168.1);

   const double tankRadiusCM =
       ini.GetReal("tank", "radius", 152.4);

   const double tankHalfHeightCM =
       ini.GetReal("tank", "halfHeightY", 198.0);

   // MRD display settings
   const double mrdTimeWindow =
       ini.GetReal("mrd", "displayWindow", 30.0);
       
       
   // --------------------------------------------------
   // ROOT global drawing style
   // --------------------------------------------------
   gStyle->SetOptStat(0);
   gStyle->SetNumberContours(255);
   //gStyle->SetPalette(kRainBow);


   // --------------------------------------------------
   // Load MRD/FMV geometry information
   // --------------------------------------------------
   std::vector<MRDInfo> allMRD = LoadMRDInfo(ini.Get("input", "mrdFile", ""));
   if (allMRD.empty()) return;


   // --------------------------------------------------
   // Separate FMV and MRD bars
   // --------------------------------------------------
   std::vector<MRDInfo> fmvBars;
   std::vector<MRDInfo> mrdBars;
   SplitMRDByDetectorSystem(allMRD, fmvBars, mrdBars);


   // --------------------------------------------------
   // Build channel-to-bar index map for fast MRD hit lookup
   // --------------------------------------------------
   std::map<int, size_t> mrdChanToIndex;
   for (size_t i = 0; i < mrdBars.size(); ++i)
       mrdChanToIndex[mrdBars[i].channel_num] = i;


   // --------------------------------------------------
   // Open input ROOT file and get the Event tree
   // --------------------------------------------------
   RootTreeReader treeReader(rootFile, "Event");
   if (!treeReader.IsValid()) return;

   TTree* tree = treeReader.GetTree();


   // --------------------------------------------------
   // Set Reco vertex branches
   // Coordinates are stored in meters
   // --------------------------------------------------
   double RecoX = 0.0;
   double RecoY = 0.0;
   double RecoZ = 0.0;
   
   double RecoDirX = 0.0;
   double RecoDirY = 0.0;
   double RecoDirZ = 0.0;

   double RecoEnergy = 0;
   int RecoFV   = false;
   int RecoFlag = false;
   


   
   tree->SetBranchAddress((recoAlgorithm + "X").c_str(),      &RecoX);
   tree->SetBranchAddress((recoAlgorithm + "Y").c_str(),      &RecoY);
   tree->SetBranchAddress((recoAlgorithm + "Z").c_str(),      &RecoZ);

   tree->SetBranchAddress((recoAlgorithm + "DirX").c_str(),   &RecoDirX);
   tree->SetBranchAddress((recoAlgorithm + "DirY").c_str(),   &RecoDirY);
   tree->SetBranchAddress((recoAlgorithm + "DirZ").c_str(),   &RecoDirZ);

   tree->SetBranchAddress((recoAlgorithm + "Energy").c_str(), &RecoEnergy);
   tree->SetBranchAddress((recoAlgorithm + "Flag").c_str(),   &RecoFlag);
   tree->SetBranchAddress((recoAlgorithm + "FV").c_str(),     &RecoFV);
   
   


   // --------------------------------------------------
   // Initialize event-level readers
   // --------------------------------------------------
   MRDClusterReader mrdClusterReader;
   mrdClusterReader.Init(tree);

   MRDRecoReader mrdRecoReader;
   mrdRecoReader.Init(tree);

   ANNIEEventReader eventReader;
   eventReader.Init(tree);


   // --------------------------------------------------
   // Initialize MC reader only when MC information is requested
   // --------------------------------------------------
   MCANNIEEventReader mcEventReader;
   if (useMC) {
       mcEventReader.Init(tree);
   }


   // --------------------------------------------------
   // Decide how many entries will be displayed
   // --------------------------------------------------
   const Long64_t totalEntries = tree->GetEntries();
   
   std::cout
    << " Number of events in this tree: "
    << totalEntries
    << std::endl;

   int nEntries = (maxEventNum < 0)
                ? totalEntries
                : std::min<Long64_t>(maxEventNum, totalEntries);

   // --------------------------------------------------
   // PDF printing state
   // --------------------------------------------------
   bool firstPage = true;




   for (int i = 0; i < nEntries; ++i) {

      tree->GetEntry(i);

      // --------------------------------------------------
      // Read true MC vertex and direction
      // True vertex coordinates are stored relative to the tank center,
      // so they are converted to global coordinates here.
      // --------------------------------------------------
      double trueVtxX = 0.0;
      double trueVtxY = 0.0;
      double trueVtxZ = 0.0;

      double trueDirX = 0.0;
      double trueDirY = 0.0;
      double trueDirZ = 0.0;

      if (useMC) {
          trueVtxX = tankCenterX + mcEventReader.GetTrueVtxX();
          trueVtxY = tankCenterY + mcEventReader.GetTrueVtxY();
          trueVtxZ = tankCenterZ + mcEventReader.GetTrueVtxZ();

          trueDirX = mcEventReader.GetTrueDirX();
          trueDirY = mcEventReader.GetTrueDirY();
          trueDirZ = mcEventReader.GetTrueDirZ();
      }
        
          
        
      int runNumber   = eventReader.GetRunNumber();
      int eventNumber = eventReader.GetEventNumber();
      


      // --------------------------------------------------
      // Prepare MRD hit containers for drawing
      // hasHit  : marks whether each MRD bar was hit
      // hitTime : stores the earliest hit time for each MRD bar
      // --------------------------------------------------
      std::vector<bool> hasHit(mrdBars.size(), false);
      std::vector<double> hitTime(mrdBars.size(), 1e9);

      EMRDClusters eMRDClusters;
      mrdClusterReader.ReadEntry(i, eMRDClusters);

      EMRDRecos eMRDRecos;
      mrdRecoReader.ReadEntry(i, eMRDRecos);


      // --------------------------------------------------
      // Select a valid MRD reco track and extract the hits
      // from the associated MRD cluster.
      // The earliest hit time for each MRD bar is stored and
      // the overall time range is determined for color mapping.
      // --------------------------------------------------
      double tMin = 1e9;
      double tMax = -1e9;

      const std::vector<MRDCluster>& clusters = eMRDClusters.Get();

      const MRDReco* selectedReco = nullptr;

      for (const MRDReco& reco : eMRDRecos.Get()) {

          int clusterIndex = reco.GetClusterIndex();

          if (clusterIndex >= 0 &&
              clusterIndex < static_cast<int>(clusters.size())) {

              selectedReco = &reco;
              break;
          }
      }

      if (selectedReco != nullptr) {

          int recoClusterIndex = selectedReco->GetClusterIndex();

          const MRDCluster& cluster = clusters.at(recoClusterIndex);

          for (const MRDHit& hit : cluster.GetHits()) {

              int ch = hit.GetDetID();
              double t = hit.GetTime();

              auto it = mrdChanToIndex.find(ch);
              if (it == mrdChanToIndex.end()) continue;

              size_t idx = it->second;

              hasHit[idx] = true;

              if (t < hitTime[idx]) {
                  hitTime[idx] = t;
              }

              if (t < tMin) tMin = t;
              if (t > tMax) tMax = t;
          }
      }


      // --------------------------------------------------
      // Set a safe default time range when no valid MRD hits
      // were found for the selected reco track.
      // --------------------------------------------------
      if (tMax <= tMin) {
          tMin = 0.0;
          tMax = 1.0;
      }



      // --------------------------------------------------
      // Create canvas for the current event
      // --------------------------------------------------
      TCanvas* c = new TCanvas(Form("mrdRecoDisplay_%d", i),
      Form("displayRecoEvent Event %d", i),
      1200, 850);

      c->SetFillColor(kBlack);

      // --------------------------------------------------
      // Create pads
      // --------------------------------------------------
      TPad* pTitle = new TPad("pTitle", "pTitle",
      0.30, 0.925,
      0.70, 0.975);

      TPad* pSide = new TPad("pSide", "pSide",
      0.06, 0.32,
      0.48, 0.90);

      TPad* pTop = new TPad("pTop", "pTop",
      0.52, 0.32,
      0.94, 0.90);

      TPad* pBar = new TPad("pBar", "pBar",
      0.30, 0.25,
      0.70, 0.33);
 
      TPad* pSummary = new TPad("pSummary", "pSummary",
      0.05, 0.05,
      0.30, 0.30);
      
      TPad* pLegend = new TPad("pLegend", "pLegend",
      0.40, 0.05,
      0.80, 0.15);
      
      pLegend->SetFillColor(kGray+1); //boyutu gormek icin farkli renk yapabilirsin
      pLegend->SetBorderMode(0);
      pLegend->Draw();
      
      // --------------------------------------------------
      // Style pads
      // --------------------------------------------------
      pTitle->SetFillColor(kBlack);
      pTitle->SetBorderMode(1);
      pTitle->SetLineColor(kOrange);
      pTitle->SetLineWidth(2);
      pTitle->Draw();

      pSide->SetFillColor(kBlack);
      pSide->SetLeftMargin(0.16);
      pSide->SetRightMargin(0.03);
      pSide->SetTopMargin(0.08);
      pSide->SetBottomMargin(0.12);
      pSide->Draw();

      pTop->SetFillColor(kBlack);
      pTop->SetLeftMargin(0.16);
      pTop->SetRightMargin(0.03);
      pTop->SetTopMargin(0.08);
      pTop->SetBottomMargin(0.12);
      pTop->Draw();

      pBar->SetFillColor(kBlack);
      pBar->Draw();

      pSummary->SetFillColor(kGray+1); //sadece cerceveyi gormek icin rengini degistir
      //pSummary->SetFrameLineColor(kGray + 1);
      //pSummary->SetFrameLineWidth(1);
      pSummary->SetBorderMode(0);
      pSummary->Draw();

      // --------------------------------------------------
      // Draw pads on the canvas
      // --------------------------------------------------
      
      
      
      
      

      // --------------------------------------------------
      // Draw event title
      // --------------------------------------------------
      pTitle->cd();
      pTitle->Range(0, 0, 1, 1);

      TLatex title;
      title.SetNDC(true);
      title.SetTextAlign(22);
      title.SetTextColor(kOrange);
      title.SetTextSize(0.45);

      title.DrawLatex(0.5, 0.5,
      Form("Run %d, Event %d",
      runNumber,
      eventNumber));

         
         
      // --------------------------------------------------
      // Side view (Y vs Z)
      // Create the drawing frame and draw the tank geometry.
      // The same Z range is used for both side and top views.
      // --------------------------------------------------

      pSide->cd();

      const double zMin = -50.0; //common for side and top
      const double zMax = 500.0; //common for side and top
      const double yMin = -250.0;
      const double yMax = 250.0; 

      TH2D* hSide = new TH2D(Form("hSide_%d", i),
                            ";Z [cm];Y [cm]",
                            10, zMin, zMax,
                            10, yMin, yMax);
      hSide->SetDirectory(nullptr);
      hSide->SetStats(0);
      hSide->Draw();

      //gPad->SetFixedAspectRatio();

      hSide->GetXaxis()->SetLabelColor(kWhite);
      hSide->GetYaxis()->SetLabelColor(kWhite);
      hSide->GetXaxis()->SetTitleColor(kWhite);
      hSide->GetYaxis()->SetTitleColor(kWhite);

      gPad->SetFrameLineColor(kWhite);

      DrawTankSide(tankCenterZ, tankCenterY, tankRadiusCM, tankHalfHeightCM);
        
        
      // --------------------------------------------------
      // Draw global coordinate axes in the side view
      // Side view coordinates: horizontal axis = Z, vertical axis = Y
      // --------------------------------------------------
      double axisX0  = 0.0;   // Z coordinate of the global origin
      double axisY0  = 0.0;   // Y coordinate of the global origin
      double axisLen = 35.0;

      TArrow* zAxisSide = new TArrow(axisX0, axisY0,
      axisX0 + axisLen, axisY0,
      0.0036, "|>");
      zAxisSide->SetLineColor(kBlue);
      zAxisSide->SetFillColor(kBlue);
      zAxisSide->SetLineWidth(2);
      zAxisSide->Draw();

      TArrow* yAxisSide = new TArrow(axisX0, axisY0,
      axisX0, axisY0 + axisLen,
      0.0036, "|>");
      yAxisSide->SetLineColor(kGreen + 1);
      yAxisSide->SetFillColor(kGreen + 1);
      yAxisSide->SetLineWidth(1);
      yAxisSide->Draw();

      TMarker* originSide = new TMarker(axisX0, axisY0, 20);
      originSide->SetMarkerColor(kWhite);
      originSide->SetMarkerSize(0.8);
      originSide->Draw("same");

      TLatex* labZSide = new TLatex(axisX0 + axisLen + 5,
      axisY0 - 5,
      "Z");
      labZSide->SetTextColor(kBlue);
      labZSide->SetTextSize(0.035);
      labZSide->Draw("same");

      TLatex* labYSide = new TLatex(axisX0 - 8,
      axisY0 + axisLen + 5,
      "Y");
      labYSide->SetTextColor(kGreen + 1);
      labYSide->SetTextSize(0.035);
      labYSide->Draw("same");

      TLatex* labOriginSide = new TLatex(axisX0,
      axisY0 - 18,
      "(0,0,0)");
      labOriginSide->SetTextAlign(22);
      labOriginSide->SetTextColor(kWhite);
      labOriginSide->SetTextSize(0.025);
      labOriginSide->Draw("same");

      // --------------------------------------------------
      // Draw tank center marker and label in the side view
      // Side view coordinates: (Z, Y)
      // --------------------------------------------------
      TMarker* mCenterSide =
      new TMarker(tankCenterZ,
      tankCenterY,
      20);

      mCenterSide->SetMarkerColor(kWhite);
      mCenterSide->SetMarkerSize(0.8);
      mCenterSide->Draw("same");

      TLatex* labCenterSide =
      new TLatex(tankCenterZ,
      tankCenterY - 25,
      Form("Tank Center (%.1f, %.1f, %.1f)",
      tankCenterX,
      tankCenterY,
      tankCenterZ));

      labCenterSide->SetTextAlign(22);
      labCenterSide->SetTextColor(kWhite);
      labCenterSide->SetTextSize(0.03);
      labCenterSide->Draw("same");

      // --------------------------------------------------
      // Draw side view title
      // --------------------------------------------------
      TLatex sideTitle;
      sideTitle.SetTextColor(kWhite);
      sideTitle.SetTextSize(0.035);
      sideTitle.DrawLatexNDC(0.04, 0.94,
      "SIDE VIEW (Y vs Z)");

      // --------------------------------------------------
      // Read MRD bar drawing parameters for the side view
      // --------------------------------------------------
      int nContours = gStyle->GetNumberContours();

      const double halfZSide = ini.GetReal("mrd", "halfZSide", 8.0);
      const double halfYSide = ini.GetReal("mrd", "halfYSide", 8.0);
      const double sideOffset = ini.GetReal("mrd", "sideOffset", 8.0);

      // --------------------------------------------------
      // Draw MRD bars in the side view.
      // Only bars with side-view orientation are drawn.
      // Hit bars are color-coded by hit time.
      // Non-hit bars are drawn as gray outlines.
      // --------------------------------------------------
      for (size_t k = 0; k < mrdBars.size(); ++k) {

   
         const MRDInfo& m = mrdBars[k];

         if (m.orientation != 0)
             continue;

         int col = kBlack;

         if (hasHit[k]) {
            double u = (hitTime[k] - tMin) / (tMax - tMin);

            if (u < 0.0) u = 0.0;
            if (u > 1.0) u = 1.0;

            col = gStyle->GetColorPalette(int(u * (nContours - 1)));
         }

         double zDraw = m.z_center;

         if (m.side == 1)
            zDraw += sideOffset;

         TBox* b = new TBox(zDraw - halfZSide,
                            m.y_center - halfYSide,
                            zDraw + halfZSide,
                            m.y_center + halfYSide);

         if (hasHit[k]) {
            b->SetFillColor(col);
            b->SetFillStyle(1001);
            b->SetLineColor(col);
         } else {
            b->SetFillColor(kBlack);
            b->SetFillStyle(0);
            b->SetLineColor(kGray + 2);
         }

         b->SetLineWidth(1);
         b->Draw("same");
     

      }

      // --------------------------------------------------
      // Draw reconstructed MRD tracks in the side view
      // Side view coordinates: horizontal axis = Z, vertical axis = Y
      // MRD track positions are stored in meters and converted to cm.
      // --------------------------------------------------
      for (const MRDReco& reco : eMRDRecos.Get()) {

         double startY = 100.0 * reco.GetStartY();
         double startZ = 100.0 * reco.GetStartZ();

         double stopY = 100.0 * reco.GetStopY();
         double stopZ = 100.0 * reco.GetStopZ();

         TArrow* trkSide = new TArrow(startZ, startY,
                                      stopZ, stopY,
                                      0.0036, "|>");

         trkSide->SetLineColor(kCyan);
         trkSide->SetFillColor(kCyan);
         trkSide->SetLineWidth(1);
         trkSide->Draw();


         // --------------------------------------------------
         // Draw back-projected Reco track and vertex
         // The Reco vertex is stored in meters and converted to cm.
         // --------------------------------------------------
         /*
         double vtxY = 100.0 * RecoY;
         double vtxZ = 100.0 * RecoZ;

         double mrdStartY = startY;
         double mrdStartZ = startZ;

         TArrow* backSide = new TArrow(vtxZ, vtxY,
                                       mrdStartZ, mrdStartY,
                                       0.0035, "|>");

         backSide->SetLineColor(kCyan + 2);
         backSide->SetFillColor(kCyan + 2);
         backSide->SetLineWidth(1);
         //backSide->Draw();

         TMarker* vtxSide = new TMarker(vtxZ, vtxY, 24);
         vtxSide->SetMarkerColor(kCyan + 2);
         vtxSide->SetMarkerSize(0.5);
         vtxSide->Draw("same");
         */
         
         
         const double recoArrowLen = 120.0; // cm

         double vtxY = 100.0 * RecoY;
         double vtxZ = 100.0 * RecoZ;

         double recoNormSide = std::sqrt(RecoDirY*RecoDirY + RecoDirZ*RecoDirZ);

         if (RecoFlag && recoNormSide > 0.0) {

             double dirY = RecoDirY / recoNormSide;
             double dirZ = RecoDirZ / recoNormSide;

             TArrow* recoDirSide = new TArrow(vtxZ, vtxY,
                                              vtxZ + recoArrowLen * dirZ,
                                              vtxY + recoArrowLen * dirY,
                                              0.0035, "|>");

             recoDirSide->SetLineColor(kCyan + 2);
             recoDirSide->SetFillColor(kCyan + 2);
             recoDirSide->SetLineWidth(1);
             recoDirSide->Draw();
         }

         TMarker* vtxSide = new TMarker(vtxZ, vtxY, 24);
         vtxSide->SetMarkerColor(kCyan + 2);
         vtxSide->SetMarkerSize(0.5);
         vtxSide->Draw("same");
         

      }

      // --------------------------------------------------
      // Draw true MC vertex and true muon direction
      // in the side view, if MC information is available.
      // True vertex coordinates are already converted to global cm.
      // --------------------------------------------------
      if (useMC) {

         const double trueArrowLen = 120.0; // cm

         TMarker* trueVtxSide = new TMarker(trueVtxZ,
                                            trueVtxY,
                                            24);
         trueVtxSide->SetMarkerColor(kMagenta);
         trueVtxSide->SetMarkerSize(0.5);
         trueVtxSide->Draw();

         TArrow* trueDirSide = new TArrow(trueVtxZ,
                                          trueVtxY,
                                          trueVtxZ + trueArrowLen * trueDirZ,
                                          trueVtxY + trueArrowLen * trueDirY,
                                          0.0035, "|>");

         trueDirSide->SetLineColor(kMagenta);
         trueDirSide->SetFillColor(kMagenta);
         trueDirSide->SetLineWidth(1);
         trueDirSide->Draw();

      }
     
         
      // --------------------------------------------------
      // Top view (X vs Z)
      // Initialize the drawing frame and draw the tank outline.
      // --------------------------------------------------
      pTop->cd();

      const double xMin = -200.0;
      const double xMax = 200.0;

      TH2D* hTop = new TH2D(Form("hTop_%d", i),
      ";Z [cm];X [cm]",
      10, zMin, zMax,
      10, xMin, xMax);

      hTop->SetDirectory(nullptr);
      hTop->SetStats(0);
      hTop->Draw();

      hTop->GetXaxis()->SetLabelColor(kWhite);
      hTop->GetYaxis()->SetLabelColor(kWhite);
      hTop->GetXaxis()->SetTitleColor(kWhite);
      hTop->GetYaxis()->SetTitleColor(kWhite);

      gPad->SetFrameLineColor(kWhite);

      DrawTankTop(tankCenterZ,
      tankCenterX,
      tankRadiusCM);

      // --------------------------------------------------
      // Draw global coordinate axes in the top view
      // Top view coordinates: horizontal axis = Z, vertical axis = X
      // --------------------------------------------------
      double axisZ0Top  = 0.0;
      double axisX0Top  = 0.0;
      double axisLenTop = 35.0;

      TArrow* zAxisTop = new TArrow(axisZ0Top, axisX0Top,
      axisZ0Top + axisLenTop, axisX0Top,
      0.0036, "|>");
      zAxisTop->SetLineColor(kBlue);
      zAxisTop->SetFillColor(kBlue);
      zAxisTop->SetLineWidth(1);
      zAxisTop->Draw();

      TArrow* xAxisTop = new TArrow(axisZ0Top, axisX0Top,
      axisZ0Top, axisX0Top + axisLenTop,
      0.0036, "|>");
      xAxisTop->SetLineColor(kRed);
      xAxisTop->SetFillColor(kRed);
      xAxisTop->SetLineWidth(1);
      xAxisTop->Draw();

      TMarker* originTop = new TMarker(axisZ0Top, axisX0Top, 20);
      originTop->SetMarkerColor(kWhite);
      originTop->SetMarkerSize(0.8);
      originTop->Draw("same");

      TLatex* labZTop = new TLatex(axisZ0Top + axisLenTop + 5,
      axisX0Top - 5,
      "Z");
      labZTop->SetTextColor(kBlue);
      labZTop->SetTextSize(0.035);
      labZTop->Draw("same");

      TLatex* labXTop = new TLatex(axisZ0Top - 8,
      axisX0Top + axisLenTop + 5,
      "X");
      labXTop->SetTextColor(kRed);
      labXTop->SetTextSize(0.035);
      labXTop->Draw("same");

      TLatex* labOriginTop = new TLatex(axisZ0Top,
      axisX0Top - 18,
      "(0,0,0)");
      labOriginTop->SetTextAlign(22);
      labOriginTop->SetTextColor(kWhite);
      labOriginTop->SetTextSize(0.025);
      labOriginTop->Draw("same");

      // --------------------------------------------------
      // Draw top view title
      // --------------------------------------------------
      TLatex topTitle;
      topTitle.SetTextColor(kWhite);
      topTitle.SetTextSize(0.035);
      topTitle.DrawLatexNDC(0.04, 0.94,
      "TOP VIEW (X vs Z)");

      // --------------------------------------------------
      // Read MRD bar drawing parameters for the top view
      // --------------------------------------------------
      const double halfZTop =
      ini.GetReal("mrd", "halfZTop", 8.0);

      const double halfXTop =
      ini.GetReal("mrd", "halfXTop", 8.0);

      // --------------------------------------------------
      // Draw MRD bars in the top view.
      // Only bars with top-view orientation are drawn.
      // Hit bars are color-coded by hit time.
      // Non-hit bars are drawn as gray outlines.
      // --------------------------------------------------
      for (size_t k = 0; k < mrdBars.size(); ++k) {


         const MRDInfo& m = mrdBars[k];

         if (m.orientation != 1)
             continue;

         int col = kBlack;

         if (hasHit[k]) {
             double u = (hitTime[k] - tMin) / (tMax - tMin);

             if (u < 0.0) u = 0.0;
             if (u > 1.0) u = 1.0;

             col = gStyle->GetColorPalette(int(u * (nContours - 1)));
         }

         double zDraw = m.z_center;

         if (m.side == 1)
             zDraw += sideOffset;

         TBox* b = new TBox(zDraw - halfZTop,
                            m.x_center - halfXTop,
                            zDraw + halfZTop,
                            m.x_center + halfXTop);

         if (hasHit[k]) {
             b->SetFillColor(col);
             b->SetFillStyle(1001);
             b->SetLineColor(col);
         } else {
             b->SetFillColor(kBlack);
             b->SetFillStyle(0);
             b->SetLineColor(kGray + 2);
         }

         b->SetLineWidth(1);
         b->Draw("same");


      }

      // --------------------------------------------------
      // Draw reconstructed MRD tracks in the top view
      // Top view coordinates: horizontal axis = Z, vertical axis = X
      // MRD track positions are stored in meters and converted to cm.
      // --------------------------------------------------
      for (const MRDReco& reco : eMRDRecos.Get()) {

         double startX = 100.0 * reco.GetStartX();
         double startZ = 100.0 * reco.GetStartZ();

         double stopX = 100.0 * reco.GetStopX();
         double stopZ = 100.0 * reco.GetStopZ();

         TArrow* trkTop = new TArrow(startZ, startX,
                                     stopZ, stopX,
                                     0.0035, "|>");

         trkTop->SetLineColor(kCyan);
         trkTop->SetFillColor(kCyan);
         trkTop->SetLineWidth(1);
         trkTop->Draw();


         // --------------------------------------------------
         // Draw back-projected Reco track and vertex
         // The Reco vertex is stored in meters and converted to cm.
         // --------------------------------------------------
         /*
         double vtxX = 100.0 * RecoX;
         double vtxZ = 100.0 * RecoZ;

         double mrdStartX = startX;
         double mrdStartZ = startZ;

         TArrow* backTop = new TArrow(vtxZ, vtxX,
                                      mrdStartZ, mrdStartX,
                                      0.0035, "|>");

         backTop->SetLineColor(kCyan + 2);
         backTop->SetFillColor(kCyan + 2);
         backTop->SetLineWidth(1);
         backTop->Draw("same");

         TMarker* vtxTop = new TMarker(vtxZ, vtxX, 24);
         vtxTop->SetMarkerColor(kCyan + 2);
         vtxTop->SetMarkerSize(0.5);
         vtxTop->Draw("same");
         */
         
         
         // --------------------------------------------------
         // Draw Reco direction and Reco vertex
         // The Reco vertex is stored in meters and converted to cm.
         // --------------------------------------------------
         const double recoArrowLen = 120.0; // cm

         double vtxX = 100.0 * RecoX;
         double vtxZ = 100.0 * RecoZ;

         double recoNorm = std::sqrt(
             RecoDirX * RecoDirX +
             RecoDirZ * RecoDirZ);

         if (RecoFlag && recoNorm > 0.0)
         {
             double dirX = RecoDirX / recoNorm;
             double dirZ = RecoDirZ / recoNorm;

             TArrow* recoDirTop = new TArrow(
                 vtxZ, vtxX,
                 vtxZ + recoArrowLen * dirZ,
                 vtxX + recoArrowLen * dirX,
                 0.0035, "|>");

             recoDirTop->SetLineColor(kCyan + 2);
             recoDirTop->SetFillColor(kCyan + 2);
             recoDirTop->SetLineWidth(1);
             recoDirTop->Draw();
         }

         TMarker* vtxTop = new TMarker(vtxZ, vtxX, 24);
         vtxTop->SetMarkerColor(kCyan + 2);
         vtxTop->SetMarkerSize(0.5);
         vtxTop->Draw("same");


      }

      // --------------------------------------------------
      // Draw true MC vertex and true muon direction
      // in the top view, if MC information is available.
      // True vertex coordinates are already converted to global cm.
      // --------------------------------------------------
      if (useMC) {


         const double trueArrowLen = 120.0; // cm

         TMarker* trueVtxTop = new TMarker(trueVtxZ,
                                           trueVtxX,
                                           24);
         trueVtxTop->SetMarkerColor(kMagenta);
         trueVtxTop->SetMarkerSize(0.5);
         trueVtxTop->Draw("same");

         TArrow* trueDirTop = new TArrow(trueVtxZ,
                                         trueVtxX,
                                         trueVtxZ + trueArrowLen * trueDirZ,
                                         trueVtxX + trueArrowLen * trueDirX,
                                         0.0035, "|>");

         trueDirTop->SetLineColor(kMagenta);
         trueDirTop->SetFillColor(kMagenta);
         trueDirTop->SetLineWidth(1);
         trueDirTop->Draw();


      }

        

      // --------------------------------------------------
      // Draw MRD hit-time color bar
      // The color scale uses the minimum and maximum hit times
      // found in the selected MRD cluster.
      // --------------------------------------------------
      pBar->cd();

      pBar->SetLeftMargin(0.08);
      pBar->SetRightMargin(0.08);
      pBar->SetTopMargin(0.15);
      pBar->SetBottomMargin(0.40);

      TH2D* hColor = new TH2D(Form("hColor_%d", i),
      "",
      100, tMin, tMax,
      1, 0, 1);

      hColor->SetDirectory(nullptr);
      hColor->SetStats(0);

      for (int ix = 1; ix <= 100; ++ix) {
      hColor->SetBinContent(ix, 1,
      hColor->GetXaxis()->GetBinCenter(ix));
      }

      hColor->GetXaxis()->SetTitle("MRD hit time [ns]");
      hColor->GetXaxis()->SetLabelColor(kWhite);
      hColor->GetXaxis()->SetTitleColor(kWhite);
      hColor->GetXaxis()->SetLabelSize(0.18);
      hColor->GetXaxis()->SetTitleSize(0.20);
      hColor->GetXaxis()->SetTitleOffset(0.80);

      hColor->GetYaxis()->SetLabelSize(0);
      hColor->GetYaxis()->SetTickLength(0);

      hColor->Draw("COL");

      // --------------------------------------------------
      // Calculate reconstruction quality quantities
      // Vertex separation is computed in cm.
      // The angular separation is computed from the normalized
      // dot product between reco and true direction vectors.
      // --------------------------------------------------
      double recoVtxX = 100.0 * RecoX;
      double recoVtxY = 100.0 * RecoY;
      double recoVtxZ = 100.0 * RecoZ;

      double vertexSep  = -999.0;
      double angularSep = -999.0;
      double cosTheta   = -999.0;
      
      double trueTheta = -999.0; // angle from +Z axis
      double truePhi   = -999.0; // azimuth angle in X-Y plane
      
      double recoTheta = -999.0;
      double recoPhi   = -999.0;
      
      double trueMuonEnergy = -999.0;
      double energyDiff = -999.0;

      if (useMC) {
          trueMuonEnergy = mcEventReader.GetTrueMuonEnergy();
      }

      if (RecoFlag && useMC && trueMuonEnergy > -900) {
          energyDiff = RecoEnergy - trueMuonEnergy;
      }


      if (RecoFlag) {   

         double recoNorm = std::sqrt(
             RecoDirX * RecoDirX +
             RecoDirY * RecoDirY +
             RecoDirZ * RecoDirZ
         );

         if (recoNorm > 0.0) {
            const double pi = std::acos(-1.0);

            double recoDirX = RecoDirX / recoNorm;
            double recoDirY = RecoDirY / recoNorm;
            double recoDirZ = RecoDirZ / recoNorm;

            recoTheta = std::acos(recoDirZ) * 180.0 / pi;
            recoPhi   = std::atan2(recoDirY, recoDirX) * 180.0 / pi;

            if (useMC) {

               vertexSep = std::sqrt(
                   std::pow(recoVtxX - trueVtxX, 2) +
                   std::pow(recoVtxY - trueVtxY, 2) +
                   std::pow(recoVtxZ - trueVtxZ, 2)
               );

               double trueNorm = std::sqrt(
                   trueDirX * trueDirX +
                   trueDirY * trueDirY +
                   trueDirZ * trueDirZ
               );

               if (trueNorm > 0.0) {

                  double trueUnitX = trueDirX / trueNorm;
                  double trueUnitY = trueDirY / trueNorm;
                  double trueUnitZ = trueDirZ / trueNorm;

                  trueTheta = std::acos(trueUnitZ) * 180.0 / pi;
                  truePhi   = std::atan2(trueUnitY, trueUnitX) * 180.0 / pi;

                  cosTheta =
                     recoDirX * trueUnitX +
                     recoDirY * trueUnitY +
                     recoDirZ * trueUnitZ;

                  cosTheta = std::max(-1.0, std::min(1.0, cosTheta));

                  angularSep = std::acos(cosTheta) * 180.0 / pi;
               }
            }
         }
      }

      // --------------------------------------------------
      // Draw reconstruction summary box
      // --------------------------------------------------
      pSummary->cd();
      pSummary->Range(0, 0, 1, 1);

      TPaveText* summary = new TPaveText(0.02, 0.02, 0.98, 0.98, "NDC");

      summary->SetFillColor(kGray + 1);
      summary->SetBorderSize(0);
      summary->SetTextColor(kWhite);
      summary->SetTextSize(0.080);
      summary->SetTextAlign(12);
      summary->SetMargin(0.04);

      if (RecoFlag)
      {
         if (useMC && vertexSep > -900 && angularSep > -900)
         {
            summary->AddText(Form("Algorithm : %s", recoAlgorithm.c_str()));
            summary->AddText("====================================");
            summary->AddText(Form("|Reco Vtx - True Vtx| : %.1f cm", vertexSep));

            summary->AddText(Form("Reco-True Angle (#theta) : %.1f deg", angularSep));

            summary->AddText(Form("cos#theta : %.3f", cosTheta));

            summary->AddText(Form("Reco Energy : %.1f MeV", RecoEnergy));

            summary->AddText(Form("True Energy : %.1f MeV", trueMuonEnergy));

            summary->AddText(Form("#DeltaE : %.1f MeV",
                                  RecoEnergy - trueMuonEnergy));

            summary->AddText(Form("True Dir: #theta= %.1f deg (+Z), #phi=%.1f deg",
                                  trueTheta, truePhi));

            summary->AddText(Form("Reco Dir: #theta= %.1f deg (+Z), #phi=%.1f deg",
                                  recoTheta, recoPhi));
         }
         else
         {
            summary->AddText(Form("Reco Dir: #theta=%.1f deg, #phi=%.1f deg",
                                  recoTheta, recoPhi));
         }
      }
      else
      {
         summary->AddText("Reco not available");
      }

      summary->Draw();


      




      
      // --------------------------------------------------
      // Draw legend
      // --------------------------------------------------
      pLegend->cd();

      TMarker* mTrue = new TMarker(0, 0, 24);
      mTrue->SetMarkerColor(kMagenta);

      TMarker* mReco = new TMarker(0, 0, 24);
      mReco->SetMarkerColor(kCyan);

      TLine* lTrue = new TLine();
      lTrue->SetLineColor(kMagenta);
      lTrue->SetLineWidth(1);

      TLine* lReco = new TLine();
      lReco->SetLineColor(kCyan);
      lReco->SetLineWidth(1);

      TLegend* leg = new TLegend(
      0.05, 0.20,
      0.95, 0.80
      );

      leg->SetFillStyle(0);
      leg->SetBorderSize(0);
      leg->SetTextColor(kWhite);
      leg->SetTextSize(0.18);
      leg->SetNColumns(2);

      leg->AddEntry(mTrue, "True vertex", "p");
      leg->AddEntry(mReco, "Reco vertex", "p");
      leg->AddEntry(lTrue, "True muon direction", "l");
      leg->AddEntry(lReco, "Reco muon direction (MRD track)", "l");

      leg->Draw();

      // --------------------------------------------------
      // Write current canvas page to the output PDF
      // --------------------------------------------------
      if (firstPage) {
      c->Print(outPdf + "[");
      firstPage = false;
      }

      c->Print(outPdf);

      delete c;
   }// end of event loop

   // --------------------------------------------------
   // Close the multi-page PDF file
   // --------------------------------------------------
   TCanvas cClose;
   cClose.Print(outPdf + "]");

   std::cout << "[displayRecoEvent] Saved PDF: "
   << outPdf << std::endl;

}

// ==================================================
// main
// ==================================================
int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: ./displayRecoEvent config.ini" << std::endl;
        return 1;
    }

    INIReader ini(argv[1]);
    displayRecoEvent(ini);

    return 0;
}

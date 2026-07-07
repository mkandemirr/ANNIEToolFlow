// LikelihoodReco.cpp
//
// New version:
//   - Vertex is fitted from timing.
//   - Direction is no longer taken from MRD.
//   - Direction is fitted using two angular parameters: theta and phi.
//   - Charge information is included with a Poisson negative log-likelihood.
//   - t0 is still taken from MC and fixed during the fit.
//   - Charge scale A is fitted.
//
// Fit parameters:
//   par[0] = vtxX   [m, global]
//   par[1] = vtxY   [m, global]
//   par[2] = vtxZ   [m, global]
//   par[3] = t0     [ns] fixed from MC
//   par[4] = theta  [rad]
//   par[5] = phi    [rad]
//   par[6] = A      [PE*m^2] charge normalization

#include "TFile.h"
#include "TTree.h"
#include "TMinuit.h"
#include "TRandom.h"
#include "TMath.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "INIReader.h"

#include "TankClusterReader.h"
#include "ETankClusters.h"
#include "TankCluster.h"
#include "PMTHit.h"

#include "ANNIEUtility.h"
#include "MCANNIEEventReader.h"
#include "MRDRecoReader.h"

// ------------------------------------------------------------
// Hit information used by the likelihood
// ------------------------------------------------------------

struct HitInfo {
    int detID;  // PMT detector ID

    double x;   // PMT x position [m, global]
    double y;   // PMT y position [m, global]
    double z;   // PMT z position [m, global]
    double t;   // measured hit time [ns]
    double pe;  // measured charge [PE]
};

// ------------------------------------------------------------
// Global objects used by TMinuit FCN
// ------------------------------------------------------------

std::vector<HitInfo> gHits;

// Timing model constants
double gSigmaT   = 2.0;          // timing resolution [ns]
double gC        = 0.299792458;  // speed of light [m/ns]
double gNWater   = 1.33;         // refractive index
double gBetaMuon = 1.0;          // muon beta
double gThetaC   = 0.0;          // Cherenkov angle [rad]

// Charge model constants
double gAttLength     = 20.0;    // attenuation length [m]
double gChargeWeight  = 1.0;     // relative weight of charge NLL
double gMinExpectedPE = 1.0e-9;  // numerical floor for expected PE

// Minimum number of valid hits required inside FCN
int gMinUsedHits = 4;

// ------------------------------------------------------------
// Convert angular fit parameters to a unit direction vector
//
// theta: polar angle from +Z axis
// phi  : azimuth angle in X-Y plane
//
// u = (sin(theta) cos(phi),
//      sin(theta) sin(phi),
//      cos(theta))
// ------------------------------------------------------------

void DirectionFromAngles(
    double theta,
    double phi,
    double& ux,
    double& uy,
    double& uz)
{
    ux = std::sin(theta) * std::cos(phi);
    uy = std::sin(theta) * std::sin(phi);
    uz = std::cos(theta);
}

// ------------------------------------------------------------
// Geometry + timing calculation
//
// Inputs:
//   vertex = (vx, vy, vz)
//   direction = (ux, uy, uz)
//   t0
//   hit
//
// Outputs:
//   expected time
//   photon path length Lgamma
//   valid flag
//
// Geometry:
//   w   = r_pmt - r_vtx
//   a   = w . u
//   rho = perpendicular distance from PMT to muon axis
//
// Cherenkov condition:
//   tan(thetaC) = rho / (a - s)
//
// Therefore:
//   s = a - rho / tan(thetaC)
//
// Expected time:
//   t_exp = t0 + s/(beta*c) + Lgamma/(c/n)
// ------------------------------------------------------------



// -----------------------------------------------------------------------------
// ExpectedTimeAndGeometry
//
// Purpose
// -------
// For a single PMT hit, compute the expected photon arrival time assuming:
//
//   • a fixed event hypothesis:
//         - vertex      (vx, vy, vz)
//         - muon direction (ux, uy, uz)
//         - event start time t0
//
//   • direct Cherenkov emission
//   • straight muon track
//   • straight photon propagation
//
// Event-level quantities (same for every PMT)
// -------------------------------------------
// These parameters remain fixed while looping over PMTs:
//
//   Vertex:
//       (vx, vy, vz)
//
//   Muon direction:
//       (ux, uy, uz)
//
//   Event time:
//       t0
//
//   Detector constants:
//       thetaC
//       nWater
//       betaMuon
//
// PMT-level calculation
// ---------------------
// This function is called ONCE for every PMT hit.
//
// For each PMT independently:
//
//   1. Compute vector from vertex to PMT.
//
//   2. Project PMT onto the muon track.
//
//   3. Find the unique Cherenkov emission point on the muon track
//      that can produce a photon reaching this PMT.
//
//   4. Compute:
//
//         s       = muon distance
//         Lgamma  = photon distance
//
//   5. Compute expected arrival time
//
//         texp = t0 + s/vMuon + Lgamma/vLight
//
// Physical validity checks
// ------------------------
// A PMT contributes to the likelihood only if a physically valid
// Cherenkov emission point exists.
//
// Invalid cases include:
//
//   • rho² < 0
//   • s < 0                     (emission before the vertex)
//   • longitudinalPhoton <= 0
//   • Lgamma <= 0
//
// If any of these conditions fail:
//
//     valid = false
//
// and this PMT is excluded from the likelihood.
//
// Output
// ------
// expected : expected photon arrival time
//
// Lgamma   : photon path length
//
// s         : muon path length from vertex to emission point
//
// valid     : true only if a physical Cherenkov solution exists.
//
// -----------------------------------------------------------------------------


double ExpectedTimeAndGeometry(
    double vx,
    double vy,
    double vz,
    double t0,
    double ux,
    double uy,
    double uz,
    const HitInfo& hit,
    double& Lgamma,
    double& s,
    bool& valid)
{
    valid = false;
    Lgamma = 0.0;
    s = 0.0;

    // Vector from vertex to PMT
    double wx = hit.x - vx;
    double wy = hit.y - vy;
    double wz = hit.z - vz;

    // Projection of PMT vector onto muon direction
    double a = wx*ux + wy*uy + wz*uz;

    // |w|^2
    double w2 = wx*wx + wy*wy + wz*wz;

    // Perpendicular distance squared to the track axis
    double rho2 = w2 - a*a;

    // Protect against tiny negative values from floating-point roundoff
    if (rho2 < 0.0 && rho2 > -1.0e-12)
        rho2 = 0.0;

    if (rho2 < 0.0)
        return 0.0;

    double rho = std::sqrt(rho2);

    double tanTheta = std::tan(gThetaC);

    if (tanTheta <= 0.0)
        return 0.0;

    // Distance from vertex to Cherenkov emission point along muon direction
    s = a - rho / tanTheta;

    // Reject emission before the vertex
    if (s < 0.0)
        return 0.0;

    // Longitudinal photon distance
    double longitudinalPhoton = a - s;

    if (longitudinalPhoton <= 0.0)
        return 0.0;

    // Photon path length from emission point to PMT
    Lgamma = std::sqrt(
        rho*rho +
        longitudinalPhoton*longitudinalPhoton
    );

    if (Lgamma <= 0.0)
        return 0.0;

    double vMuon  = gBetaMuon * gC;
    double vLight = gC / gNWater;

    if (vMuon <= 0.0 || vLight <= 0.0)
        return 0.0;

    double expected =
        t0 +
        s / vMuon +
        Lgamma / vLight;

    valid = true;
    return expected;
}

// ------------------------------------------------------------
// Expected charge model
//
// Simple first version:
//
//   mu_i = A * exp(-Lgamma/lambda_att) / Lgamma^2
//
// where:
//   mu_i      = expected PE on PMT i
//   A         = fitted charge normalization
//   Lgamma    = photon path length [m]
//   lambda_att = attenuation length [m]
//
// This is intentionally simple. Later, PMT angular acceptance,
// quantum efficiency, dark noise, scattering, and track-length effects
// can be added here.
// ------------------------------------------------------------

double ExpectedCharge(
    double A,
    double Lgamma)
{
    if (A <= 0.0)
        return gMinExpectedPE;

    if (Lgamma <= 0.0)
        return gMinExpectedPE;

    double attenuation = 1.0;

    if (gAttLength > 0.0)
        attenuation = std::exp(-Lgamma / gAttLength);

    double mu = A * attenuation / (Lgamma * Lgamma);

    if (mu < gMinExpectedPE)
        mu = gMinExpectedPE;

    return mu;
}

// ------------------------------------------------------------
// Poisson charge negative log-likelihood
//
// Stable form:
//
//   NLL_q = mu - q + q log(q/mu)
//
// For q = 0:
//
//   q log(q/mu) = 0
//
// This form is always >= 0 and is numerically better than
// using mu - q log(mu) directly.
// ------------------------------------------------------------

double ChargeNLLPoisson(
    double q,
    double mu)
{
    if (mu < gMinExpectedPE)
        mu = gMinExpectedPE;

    if (q <= 0.0)
        return mu;

    return mu - q + q * std::log(q / mu);
}

// ------------------------------------------------------------
// Function minimized by TMinuit
//
// Total NLL:
//
//   NLL = NLL_time + w_charge * NLL_charge
//
// Time:
//
//   NLL_time = sum_i 0.5 * ((t_obs - t_exp)/sigma_t)^2
//
// Charge:
//
//   NLL_charge = sum_i [mu_i - q_i + q_i log(q_i/mu_i)]
// ------------------------------------------------------------

void FCN(
    Int_t& npar,
    Double_t* grad,
    Double_t& f,
    Double_t* par,
    Int_t flag)
{
    double vx    = par[0];
    double vy    = par[1];
    double vz    = par[2];
    double t0    = par[3];
    double theta = par[4];
    double phi   = par[5];
    double A     = par[6];
    

double xc = 0.0;
double zc = 1.681;
double R  = 1.524;

double dx = vx - xc;
double dz = vz - zc;

if (dx*dx + dz*dz > R*R) {
    f = 1.0e12;
    return;
}

    double ux, uy, uz;
    DirectionFromAngles(theta, phi, ux, uy, uz);

    double nllTime   = 0.0;
    double nllCharge = 0.0;

    int nUsed = 0;

    for (const auto& hit : gHits) {

        bool valid = false;
        double Lgamma = 0.0;
        double s = 0.0;

        double tExp = ExpectedTimeAndGeometry(
            vx, vy, vz,
            t0,
            ux, uy, uz,
            hit,
            Lgamma,
            s,
            valid);

        if (!valid)
            continue;

        // Timing contribution
        double residual = hit.t - tExp;

        nllTime += 0.5 * residual * residual /
                   (gSigmaT * gSigmaT);

        // Charge contribution
        double mu = ExpectedCharge(A, Lgamma);

        nllCharge += ChargeNLLPoisson(hit.pe, mu);

        ++nUsed;
    }

    // Not enough valid hits means this point in parameter space is bad.
    if (nUsed < gMinUsedHits) {
        f = 1.0e12;
        return;
    }

    f = nllTime + gChargeWeight * nllCharge;
}

// ------------------------------------------------------------
// Estimate a reasonable initial value for A
//
// Since:
//   mu_i = A * G_i
//
// with:
//   G_i = exp(-Lgamma/lambda_att)/Lgamma^2
//
// a simple estimate is:
//   A_init = sum(q_i) / sum(G_i)
//
// This depends on the initial vertex and direction.
// ------------------------------------------------------------

double EstimateInitialA(
    double vx,
    double vy,
    double vz,
    double t0,
    double theta,
    double phi)
{
    double ux, uy, uz;
    DirectionFromAngles(theta, phi, ux, uy, uz);

    double sumQ = 0.0;
    double sumG = 0.0;

    for (const auto& hit : gHits) {

        bool valid = false;
        double Lgamma = 0.0;
        double s = 0.0;

        ExpectedTimeAndGeometry(
            vx, vy, vz,
            t0,
            ux, uy, uz,
            hit,
            Lgamma,
            s,
            valid);

        if (!valid)
            continue;

        double attenuation = 1.0;

        if (gAttLength > 0.0)
            attenuation = std::exp(-Lgamma / gAttLength);

        double G = attenuation / (Lgamma * Lgamma);

        sumQ += hit.pe;
        sumG += G;
    }

    if (sumG <= 0.0)
        return 1.0;

    double A = sumQ / sumG;

    if (A <= 0.0)
        A = 1.0;

    return A;
}


// ------------------------------------------------------------
// Get initial muon direction from MRDReco.
//
// MRDReco gives the track start and stop points.
// The direction is defined as:
//
//   u = (stop - start) / |stop - start|
//
// Then this direction is converted to theta and phi:
//
//   theta = acos(uz)
//   phi   = atan2(uy, ux)
//
// Return value:
//   true  -> valid MRD direction found
//   false -> no usable MRD direction
// ------------------------------------------------------------

bool GetMuonDirFromMRD(
    MRDRecoReader& mrdRecoReader,
    Long64_t entry,
    double& dirX,
    double& dirY,
    double& dirZ,
    double& theta,
    double& phi)
{
    EMRDRecos eMRDRecos;
    mrdRecoReader.ReadEntry(entry, eMRDRecos);

    const auto& recos = eMRDRecos.Get();

    // Require exactly one MRD reco object.
    if (recos.size() != 1)
        return false;

    const auto& reco = recos.at(0);

    // Keep the same stopping-track requirement
    // used in your older algorithm.
    if (!reco.IsStop())
        return false;

    double startx = reco.GetStartX();
    double starty = reco.GetStartY();
    double startz = reco.GetStartZ();

    double stopx = reco.GetStopX();
    double stopy = reco.GetStopY();
    double stopz = reco.GetStopZ();

    double dx = stopx - startx;
    double dy = stopy - starty;
    double dz = stopz - startz;

    double norm = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (norm <= 0.0)
        return false;

    dirX = dx / norm;
    dirY = dy / norm;
    dirZ = dz / norm;

    // Protect acos against tiny numerical overshoot.
    if (dirZ >  1.0) dirZ =  1.0;
    if (dirZ < -1.0) dirZ = -1.0;

    theta = std::acos(dirZ);
    phi   = std::atan2(dirY, dirX);

    return true;
}

// ------------------------------------------------------------

void makeLikelihoodReco(INIReader& iniReader)
{
    std::string inputFile =
        iniReader.Get("Input", "InputFile", "");

    std::string outputFile =
        iniReader.Get("Output", "OutputFile", "LikelihoodReco.root");

    std::string treeName =
        iniReader.Get("Input", "TreeName", "Event");

    // Timing likelihood settings
    gSigmaT =
        iniReader.GetReal("Likelihood", "SigmaT", 2.0);

    gNWater =
        iniReader.GetReal("Likelihood", "NWater", 1.33);

    gBetaMuon =
        iniReader.GetReal("Likelihood", "BetaMuon", 1.0);

    // Charge likelihood settings
    gAttLength =
        iniReader.GetReal("Likelihood", "AttLength", 20.0);

    gChargeWeight =
        iniReader.GetReal("Likelihood", "ChargeWeight", 1.0);

    gMinExpectedPE =
        iniReader.GetReal("Likelihood", "MinExpectedPE", 1.0e-9);

    // Hit selection
    double minPE =
        iniReader.GetReal("Likelihood", "MinPE", 0.0);

    double minTime =
        iniReader.GetReal("Likelihood", "MinTime", 0.0);

    double maxTime =
        iniReader.GetReal("Likelihood", "MaxTime", 2000.0);

    double pmtPositionScale =
        iniReader.GetReal("Likelihood", "PMTPositionScale", 1.0);

    int minHits =
        iniReader.GetInteger("Likelihood", "MinHits", 4);
        
   bool fixDirection =
    iniReader.GetBoolean("Likelihood", "FixDirection", true);

    gMinUsedHits = minHits;

    // Cherenkov angle
    if (gBetaMuon * gNWater <= 1.0) {
        std::cerr
            << "Error: beta*n <= 1. Cherenkov light cannot be produced."
            << std::endl;
        return;
    }

    gThetaC = std::acos(1.0 / (gBetaMuon * gNWater));

    TFile* inputRootFile =
        TFile::Open(inputFile.c_str(), "READ");

    if (!inputRootFile || inputRootFile->IsZombie()) {
        std::cerr << "Error opening file: "
                  << inputFile << std::endl;
        return;
    }

    TTree* inputTree =
        (TTree*)inputRootFile->Get(treeName.c_str());

    if (!inputTree) {
        std::cerr << "Error: tree not found: "
                  << treeName << std::endl;
        inputRootFile->Close();
        return;
    }

    TankClusterReader tankClusterReader;
    tankClusterReader.Init(inputTree);

    MCANNIEEventReader mcEventReader;
    mcEventReader.Init(inputTree);
    
    MRDRecoReader mrdRecoReader;
    mrdRecoReader.Init(inputTree);

    TFile* outputRootFile =
        TFile::Open(outputFile.c_str(), "RECREATE");

    TTree* outputTree = inputTree->CloneTree(0);

    // Output branches
    double LikelihoodRecoX = -9999;
    double LikelihoodRecoY = -9999;
    double LikelihoodRecoZ = -9999;

    double LikelihoodRecoDirX = -9999;
    double LikelihoodRecoDirY = -9999;
    double LikelihoodRecoDirZ = -9999;

    double LikelihoodRecoTheta = -9999;
    double LikelihoodRecoPhi   = -9999;

    double LikelihoodRecoChargeScale = -9999;

    double LikelihoodRecoEnergy = -9999; // not implemented yet

    double LikelihoodRecoT0 = -9999;

    double LikelihoodRecoNLL       = -9999;
    double LikelihoodRecoNLLTime   = -9999;
    double LikelihoodRecoNLLCharge = -9999;

    int LikelihoodRecoFlag = 0;

    outputTree->Branch("LikelihoodRecoX",
                       &LikelihoodRecoX,
                       "LikelihoodRecoX/D");

    outputTree->Branch("LikelihoodRecoY",
                       &LikelihoodRecoY,
                       "LikelihoodRecoY/D");

    outputTree->Branch("LikelihoodRecoZ",
                       &LikelihoodRecoZ,
                       "LikelihoodRecoZ/D");

    outputTree->Branch("LikelihoodRecoDirX",
                       &LikelihoodRecoDirX,
                       "LikelihoodRecoDirX/D");

    outputTree->Branch("LikelihoodRecoDirY",
                       &LikelihoodRecoDirY,
                       "LikelihoodRecoDirY/D");

    outputTree->Branch("LikelihoodRecoDirZ",
                       &LikelihoodRecoDirZ,
                       "LikelihoodRecoDirZ/D");

    outputTree->Branch("LikelihoodRecoTheta",
                       &LikelihoodRecoTheta,
                       "LikelihoodRecoTheta/D");

    outputTree->Branch("LikelihoodRecoPhi",
                       &LikelihoodRecoPhi,
                       "LikelihoodRecoPhi/D");

    outputTree->Branch("LikelihoodRecoChargeScale",
                       &LikelihoodRecoChargeScale,
                       "LikelihoodRecoChargeScale/D");

    outputTree->Branch("LikelihoodRecoEnergy",
                       &LikelihoodRecoEnergy,
                       "LikelihoodRecoEnergy/D");

    outputTree->Branch("LikelihoodRecoT0",
                       &LikelihoodRecoT0,
                       "LikelihoodRecoT0/D");

    outputTree->Branch("LikelihoodRecoNLL",
                       &LikelihoodRecoNLL,
                       "LikelihoodRecoNLL/D");

    outputTree->Branch("LikelihoodRecoNLLTime",
                       &LikelihoodRecoNLLTime,
                       "LikelihoodRecoNLLTime/D");

    outputTree->Branch("LikelihoodRecoNLLCharge",
                       &LikelihoodRecoNLLCharge,
                       "LikelihoodRecoNLLCharge/D");

    outputTree->Branch("LikelihoodRecoFlag",
                       &LikelihoodRecoFlag,
                       "LikelihoodRecoFlag/I");

    Long64_t nEntries = inputTree->GetEntries();
    Long64_t nSuccess = 0;

    for (Long64_t i = 0; i < nEntries; ++i) {

        inputTree->GetEntry(i);

        std::cout
            << "========== Event = "
            << i
            << " =========="
            << std::endl;

        // Reset output values
        LikelihoodRecoX = -9999;
        LikelihoodRecoY = -9999;
        LikelihoodRecoZ = -9999;

        LikelihoodRecoDirX = -9999;
        LikelihoodRecoDirY = -9999;
        LikelihoodRecoDirZ = -9999;

        LikelihoodRecoTheta = -9999;
        LikelihoodRecoPhi   = -9999;

        LikelihoodRecoChargeScale = -9999;

        LikelihoodRecoEnergy = -9999;
        LikelihoodRecoT0 = -9999;

        LikelihoodRecoNLL       = -9999;
        LikelihoodRecoNLLTime   = -9999;
        LikelihoodRecoNLLCharge = -9999;

        LikelihoodRecoFlag = 0;

        // ------------------------------------------------------------
        // Build hit list for this event
        // ------------------------------------------------------------

        gHits.clear();

        ETankClusters eTankClusters;
        tankClusterReader.ReadEntry(i, eTankClusters);

        const auto& clusters = eTankClusters.Get();

        for (const auto& cluster : clusters) {

            for (const auto& pmtHit : cluster.GetHits()) {

                double t  = pmtHit.GetT();
                double pe = pmtHit.GetPE();
                double id = pmtHit.GetDetID();

                if (t < minTime || t > maxTime)
                    continue;

                if (pe < minPE)
                    continue;

                HitInfo h;

                // PMT positions are returned with respect to tank center.
                // Convert them to global coordinates, because the fitted
                // vertex is also treated as global.
                double x = pmtHit.GetX() * pmtPositionScale;
                double y = pmtHit.GetY() * pmtPositionScale;
                double z = pmtHit.GetZ() * pmtPositionScale;

                ANNIEUtility::TankToGlobal(x, y, z);

                h.detID = id;  
                h.x  = x;
                h.y  = y;
                h.z  = z;
                h.t  = t;
                h.pe = pe;

                gHits.push_back(h);
            }
        }

        if ((int)gHits.size() < minHits) {
            outputTree->Fill();
            continue;
        }

        // ------------------------------------------------------------
        // Initial values
        //
        // Vertex limits follow your previous code.
        // t0 is taken from MC and fixed.
        // Direction is initialized randomly.
        // A is estimated from total observed charge and geometry.
        // ------------------------------------------------------------

        double initX = -1.4 + 2.8 * gRandom->Rndm();
        double initY = -2.2 + 4.2 * gRandom->Rndm();
        double initZ =  0.2 + 3.0 * gRandom->Rndm();

        double initT0 = mcEventReader.GetTrueVtxTime();

        
        
        
        
        //bu kisim yorum satiri yapildi
        // Random direction:
        // theta in [0, pi], phi in [-pi, pi]
        //double initTheta = TMath::Pi() * gRandom->Rndm();
        //double initPhi   = -TMath::Pi() + 2.0*TMath::Pi()*gRandom->Rndm();
        
        
        
      // ------------------------------------------------------------
      // Initial direction from MRD.
      //
      // This keeps the old reconstruction information as the
      // starting direction. If FixDirection=true in the config,
      // theta and phi will also be fixed in Minuit.
      // ------------------------------------------------------------

      double initDirX = 0.0;
      double initDirY = 0.0;
      double initDirZ = 1.0;

      double initTheta = 0.0;
      double initPhi   = 0.0;

      bool hasMRDDir =
          GetMuonDirFromMRD(
              mrdRecoReader,
              i,
              initDirX,
              initDirY,
              initDirZ,
              initTheta,
              initPhi);

      if (!hasMRDDir) {
          outputTree->Fill();
          continue;
      }
        
        
        
        
        

        double initA = EstimateInitialA(
            initX,
            initY,
            initZ,
            initT0,
            initTheta,
            initPhi);

                  
                  
                  
                  
                  
                  
                  
                  
       
      // ------------------------------------------------------------
      // Compute the initial NLL before the fit starts.
      //
      // This is useful to verify that the minimization really
      // decreases the negative log-likelihood.
      //
      // The same timing and charge models used inside the FCN
      // are evaluated using the initial parameter values.
      // ------------------------------------------------------------

      double initialNLLTime   = 0.0;
      double initialNLLCharge = 0.0;

      int initialUsedHits = 0;


      DirectionFromAngles(
          initTheta,
          initPhi,
          initDirX,
          initDirY,
          initDirZ);

      // Loop over all PMT hits
      for (const auto& hit : gHits)
      {
          bool valid = false;

          double Lgamma = 0.0;
          double s = 0.0;

          // Compute the expected hit time and the photon path
          // using the initial fit parameters.
          double tExp =
              ExpectedTimeAndGeometry(
                  initX,
                  initY,
                  initZ,
                  initT0,
                  initDirX,
                  initDirY,
                  initDirZ,
                  hit,
                  Lgamma,
                  s,
                  valid);

          // Ignore hits that do not have a valid
          // Cherenkov geometry.
          if (!valid)
              continue;

          // --------------------------------------------------------
          // Timing contribution
          // --------------------------------------------------------

          double residual = hit.t - tExp;

          initialNLLTime +=
              0.5 *
              residual * residual /
              (gSigmaT * gSigmaT);

          // --------------------------------------------------------
          // Charge contribution
          // --------------------------------------------------------

          double mu =
              ExpectedCharge(
                  initA,
                  Lgamma);

          initialNLLCharge +=
              ChargeNLLPoisson(
                  hit.pe,
                  mu);

          ++initialUsedHits;
      }

      // Total initial NLL
      double initialNLLTotal =
          initialNLLTime +
          gChargeWeight * initialNLLCharge;        
                  
                  
                  
                  
                  

        // ------------------------------------------------------------
        // Minuit setup
        // ------------------------------------------------------------

        TMinuit minuit(7);
        minuit.SetFCN(FCN);

        double arglist[10];
        int ierflg = 0;

        arglist[0] = -1;
        minuit.mnexcm("SET PRINT", arglist, 1, ierflg);

        minuit.DefineParameter(0, "vtxX",
                               initX,
                               0.02,
                               -1.4,
                               1.4);

        minuit.DefineParameter(1, "vtxY",
                               initY,
                               0.02,
                               -2.2,
                               2.0);

        minuit.DefineParameter(2, "vtxZ",
                               initZ,
                               0.02,
                               0.2,
                               3.2);

        minuit.DefineParameter(3, "t0",
                               initT0,
                               1.0,
                               -100.0,
                               100.0);

        minuit.DefineParameter(4, "theta",
                               initTheta,
                               0.02,
                               0.0,
                               TMath::Pi());

        minuit.DefineParameter(5, "phi",
                               initPhi,
                               0.02,
                               -TMath::Pi(),
                               TMath::Pi());

         
         double Amin = std::max(1.0e-6, 0.01 * initA);
         double Amax = std::max(10.0,   100.0 * initA);

         minuit.DefineParameter(6, "A",
                                initA,
                                std::max(0.1*initA, 1.0e-3),
                                Amin,
                                Amax);
                               
                               
                               
                               
                               
                               

         // t0 is taken from MC and kept fixed.
         minuit.FixParameter(3);
        
        
         // Direction is initialized from MRD.
         //
         // If FixDirection=true:
         //   theta and phi are fixed.
         //   This reproduces the old MRD-direction approach.
         //
         // If FixDirection=false:
         //   theta and phi are free fit parameters.
         //   MRD only provides the initial guess.
         if (fixDirection) {
             minuit.FixParameter(4); // theta
             minuit.FixParameter(5); // phi
             minuit.FixParameter(6); 
             
             // If charge is not used, A does not affect the NLL.
            // Fix it to avoid an unconstrained parameter.
            //minuit.FixParameter(6); // A
         }
        

        arglist[0] = 5000; // maximum number of function calls
        arglist[1] = 1.0;  // tolerance

        minuit.mnexcm("MIGRAD", arglist, 2, ierflg);


        // ------------------------------------------------------------
        // Read fitted parameters
        // ------------------------------------------------------------

        double fitX, errX;
        double fitY, errY;
        double fitZ, errZ;
        double fitT0, errT0;
        double fitTheta, errTheta;
        double fitPhi, errPhi;
        double fitA, errA;

        minuit.GetParameter(0, fitX, errX);
        minuit.GetParameter(1, fitY, errY);
        minuit.GetParameter(2, fitZ, errZ);
        minuit.GetParameter(3, fitT0, errT0);
        minuit.GetParameter(4, fitTheta, errTheta);
        minuit.GetParameter(5, fitPhi, errPhi);
        minuit.GetParameter(6, fitA, errA);

        double fitDirX, fitDirY, fitDirZ;
        DirectionFromAngles(
            fitTheta,
            fitPhi,
            fitDirX,
            fitDirY,
            fitDirZ);

        double amin, edm, errdef;
        int nvpar, nparx, icstat;

        minuit.mnstat(
            amin,
            edm,
            errdef,
            nvpar,
            nparx,
            icstat);

        // ------------------------------------------------------------
        // Compute separated timing and charge NLL at best fit
        // ------------------------------------------------------------

        double bestNLLTime = 0.0;
        double bestNLLCharge = 0.0;
        int nUsedBest = 0;

        for (const auto& hit : gHits) {

            bool valid = false;
            double Lgamma = 0.0;
            double s = 0.0;

            double tExp = ExpectedTimeAndGeometry(
                fitX,
                fitY,
                fitZ,
                fitT0,
                fitDirX,
                fitDirY,
                fitDirZ,
                hit,
                Lgamma,
                s,
                valid);

            if (!valid)
                continue;

            double residual = hit.t - tExp;

            bestNLLTime += 0.5 * residual * residual /
                           (gSigmaT * gSigmaT);

            double mu = ExpectedCharge(fitA, Lgamma);

            bestNLLCharge += ChargeNLLPoisson(hit.pe, mu);

            ++nUsedBest;
        }

        // ------------------------------------------------------------
        // Save output
        // ------------------------------------------------------------

        LikelihoodRecoX = fitX;
        LikelihoodRecoY = fitY;
        LikelihoodRecoZ = fitZ;

        LikelihoodRecoDirX = fitDirX;
        LikelihoodRecoDirY = fitDirY;
        LikelihoodRecoDirZ = fitDirZ;

        LikelihoodRecoTheta = fitTheta;
        LikelihoodRecoPhi   = fitPhi;

        LikelihoodRecoChargeScale = fitA;

        LikelihoodRecoT0 = fitT0;

        LikelihoodRecoNLL       = amin;
        LikelihoodRecoNLLTime   = bestNLLTime;
        LikelihoodRecoNLLCharge = bestNLLCharge;





// ------------------------------------------------------------
// Compute additional fit-quality quantities.
//
// Besides the standard MINUIT status flags, we also evaluate:
//
//   • Improvement of the total negative log-likelihood.
//   • Fraction of PMT hits with a valid Cherenkov solution.
//   • Average charge NLL per valid hit.
//   • Estimated parameter uncertainties (fit resolution).
//
// These quantities are used to reject fits that formally converge
// but have poor reconstruction quality or unrealistically large
// parameter uncertainties.
// ------------------------------------------------------------

double finalNLLTotal =
    bestNLLTime + gChargeWeight * bestNLLCharge;

double improvementTotal =
    initialNLLTotal - finalNLLTotal;

double validFrac =
    gHits.empty() ? 0.0 : (double)nUsedBest / gHits.size();

double chargeNLLPerHit =
    nUsedBest > 0 ? bestNLLCharge / nUsedBest : 1.0e12;

// ------------------------------------------------------------
// Parameter resolution cuts.
//
// Reject fits with excessively large MINUIT uncertainties.
// These cuts help eliminate poorly constrained solutions,
// even if MIGRAD reports successful convergence.
// ------------------------------------------------------------



double errVtx =
    std::sqrt(
        errX * errX +
        errY * errY +
        errZ * errZ);

bool positionResolutionOK =
    errX > 0.0 &&
    errY > 0.0 &&
    errZ > 0.0 &&
    errVtx < 1;      // m

bool directionResolutionOK = true;

if (!fixDirection) {
    directionResolutionOK =
        errTheta > 0.0 &&
        errPhi   > 0.0 &&
        errTheta < 0.50 &&
        errPhi   < 0.50;
}
/*
bool chargeScaleOK =
    fitA > 0.0 &&
    errA > 0.0 &&
    errA / fitA < 1.0;
*/


// ------------------------------------------------------------
// Final reconstruction quality criteria.
//
// A fit is accepted only if:
//
//   • MIGRAD converged successfully.
//   • Covariance matrix is accurate (icstat = 3).
//   • Estimated Distance to Minimum (EDM) is sufficiently small.
//   • Enough PMT hits remain valid.
//   • Total NLL improves after minimization.
//   • Charge likelihood is reasonable.
//   • Parameter uncertainties satisfy the resolution cuts.
//
// Events failing any of these criteria are marked as failed.
// ------------------------------------------------------------

bool goodFit =
    (ierflg == 0) &&
    (icstat == 3) &&
    (edm < 1.0e-2) &&
    //(nUsedBest >= minHits) &&
    (validFrac > 0.5) &&
    (improvementTotal > 0.0) &&
    //(chargeNLLPerHit < 20.0) &&
    positionResolutionOK &&
    directionResolutionOK;

if (goodFit) {
    LikelihoodRecoFlag = 1;
    ++nSuccess;
}

// ------------------------------------------------------------
// Print summary
// ------------------------------------------------------------

double initThetaDeg = initTheta * 180.0 / TMath::Pi();
double initPhiDeg   = initPhi   * 180.0 / TMath::Pi();

double fitThetaDeg    = fitTheta * 180.0 / TMath::Pi();
double errThetaDeg    = errTheta * 180.0 / TMath::Pi();

double fitPhiDeg      = fitPhi * 180.0 / TMath::Pi();
double errPhiDeg      = errPhi * 180.0 / TMath::Pi();

double dirNorm = std::sqrt(
    fitDirX*fitDirX +
    fitDirY*fitDirY +
    fitDirZ*fitDirZ
);

std::cout << "\n================ Fit summary ================\n";

std::cout << "\nInitial parameters\n";
std::cout << "initX      = " << initX << " m\n";
std::cout << "initY      = " << initY << " m\n";
std::cout << "initZ      = " << initZ << " m\n";
std::cout << "initT0     = " << initT0 << " ns   fixed from MC\n";

std::cout << "initTheta  = " << initTheta
          << " rad  (" << initThetaDeg << " deg)\n";

std::cout << "initPhi    = " << initPhi
          << " rad  (" << initPhiDeg << " deg)\n";

std::cout << "initA      = " << initA << " PE*m^2\n";

std::cout << "\nHit statistics\n";
std::cout << "Hits total        = " << gHits.size() << "\n";
std::cout << "Initial used hits = " << initialUsedHits << "\n";
std::cout << "Final used hits   = " << nUsedBest << "\n";
std::cout << "Final valid frac  = "
          << (double)nUsedBest / gHits.size() << "\n";

std::cout << "\nFit parameters\n";

std::cout << "fitX      = " << fitX
          << " +/- " << errX << " m\n";

std::cout << "fitY      = " << fitY
          << " +/- " << errY << " m\n";

std::cout << "fitZ      = " << fitZ
          << " +/- " << errZ << " m\n";

std::cout << "fitT0     = " << fitT0
          << " ns   fixed\n";

std::cout << "fitTheta  = " << fitTheta
          << " +/- " << errTheta << " rad"
          << "  (" << fitThetaDeg
          << " +/- " << errThetaDeg << " deg)\n";

std::cout << "fitPhi    = " << fitPhi
          << " +/- " << errPhi << " rad"
          << "  (" << fitPhiDeg
          << " +/- " << errPhiDeg << " deg)\n";

std::cout << "fitDirX   = " << fitDirX << "\n";
std::cout << "fitDirY   = " << fitDirY << "\n";
std::cout << "fitDirZ   = " << fitDirZ << "\n";
std::cout << "|fitDir|  = " << dirNorm << "\n";

std::cout << "fitA      = " << fitA
          << " +/- " << errA << " PE*m^2";

if (fixDirection)
    std::cout << "   fixed";

std::cout << "\n";

std::cout << "\nNLL summary\n";
std::cout << "Initial total   = " << initialNLLTotal << "\n";
std::cout << "Initial time    = " << initialNLLTime << "\n";
std::cout << "Initial charge  = " << initialNLLCharge << "\n";

std::cout << "Final total     = " << amin << "\n";
std::cout << "Final time      = " << bestNLLTime << "\n";
std::cout << "Final charge    = " << bestNLLCharge << "\n";

std::cout << "Improvement total  = "
          << initialNLLTotal - amin << "\n";

std::cout << "Improvement time   = "
          << initialNLLTime - bestNLLTime << "\n";

std::cout << "Improvement charge = "
          << initialNLLCharge - bestNLLCharge << "\n";

std::cout << "\nMinuit status\n";
std::cout << "EDM        = " << edm << "\n";
std::cout << "ierflg     = " << ierflg << "\n";
std::cout << "icstat     = " << icstat << "\n";
std::cout << "Fit Status = "
          << (goodFit ? "SUCCESS" : "FAILED")
          << "\n";

std::cout << "=============================================\n";

        

// ------------------------------------------------------------
// Hit-by-hit timing and charge check
// Compare initial prediction and final fit prediction
// ------------------------------------------------------------

std::cout
    << "\nHit-by-hit timing and charge check"
    << std::endl;

for (size_t ihit = 0; ihit < gHits.size(); ++ihit) {

    const auto& hit = gHits[ihit];

    // ---------- Initial prediction ----------
    bool validInit = false;
    double LgammaInit = 0.0;
    double sInit = 0.0;

    double tExpInit = ExpectedTimeAndGeometry(
        initX,
        initY,
        initZ,
        initT0,
        initDirX,
        initDirY,
        initDirZ,
        hit,
        LgammaInit,
        sInit,
        validInit);

    // ---------- Final fit prediction ----------
    bool validFit = false;
    double LgammaFit = 0.0;
    double sFit = 0.0;

    double tExpFit = ExpectedTimeAndGeometry(
        fitX,
        fitY,
        fitZ,
        fitT0,
        fitDirX,
        fitDirY,
        fitDirZ,
        hit,
        LgammaFit,
        sFit,
        validFit);

    if (!validInit && !validFit)
        continue;

    double dtInit = 0.0;
    double dtFit  = 0.0;

    double qExpInit = 0.0;
    double qExpFit  = 0.0;

    double nllTimeInit = 0.0;
    double nllTimeFit  = 0.0;

    double nllChargeInit = 0.0;
    double nllChargeFit  = 0.0;

    if (validInit) {
        dtInit = hit.t - tExpInit;
        qExpInit = ExpectedCharge(initA, LgammaInit);

        nllTimeInit =
            0.5 * dtInit * dtInit /
            (gSigmaT * gSigmaT);

        nllChargeInit =
            ChargeNLLPoisson(hit.pe, qExpInit);
    }

    if (validFit) {
        dtFit = hit.t - tExpFit;
        qExpFit = ExpectedCharge(fitA, LgammaFit);

        nllTimeFit =
            0.5 * dtFit * dtFit /
            (gSigmaT * gSigmaT);

        nllChargeFit =
            ChargeNLLPoisson(hit.pe, qExpFit);
    }

    std::cout
        << "Hit detID=" << hit.detID
        << "  index=" << ihit
        << "\n";

    std::cout
        << "  tObs       = " << hit.t << " ns\n";

    if (validInit) {
        std::cout
            << "  tExp init  = " << tExpInit << " ns"
            << "   dt init = " << dtInit << " ns\n";
    } else {
        std::cout
            << "  tExp init  = invalid\n";
    }

    if (validFit) {
        std::cout
            << "  tExp fit   = " << tExpFit << " ns"
            << "   dt fit  = " << dtFit << " ns\n";
    } else {
        std::cout
            << "  tExp fit   = invalid\n";
    }

    std::cout
        << "  qObs       = " << hit.pe << " PE\n";

    if (validInit) {
        std::cout
            << "  qExp init  = " << qExpInit << " PE"
            << "   Lgamma init = " << LgammaInit << " m\n";
    } else {
        std::cout
            << "  qExp init  = invalid\n";
    }

    if (validFit) {
        std::cout
            << "  qExp fit   = " << qExpFit << " PE"
            << "   Lgamma fit  = " << LgammaFit << " m\n";
    } else {
        std::cout
            << "  qExp fit   = invalid\n";
    }

    if (validInit && validFit) {
        std::cout
            << "  Delta NLL time   = "
            << nllTimeInit - nllTimeFit << "\n";

        std::cout
            << "  Delta NLL charge = "
            << nllChargeInit - nllChargeFit << "\n";
    }

    std::cout << std::endl;
}

        std::cout
            << "\n========= Event ended ========================="
            << std::endl;

        outputTree->Fill();
    }

    std::cout << "---------- Run results ------------------" << std::endl;
    std::cout << "LikelihoodReco with fitted direction + charge" << std::endl;
    std::cout << "InputFile  : " << inputFile << std::endl;
    std::cout << "OutputFile : " << outputFile << std::endl;
    std::cout << "Entries    : " << nEntries << std::endl;
    std::cout << "Success    : " << nSuccess << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    outputRootFile->cd();
    outputTree->Write();

    outputRootFile->Close();
    inputRootFile->Close();

    delete outputRootFile;
    delete inputRootFile;
}

// ------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr
            << "Usage: ./LikelihoodReco config.ini"
            << std::endl;
        return 1;
    }

    INIReader iniReader(argv[1]);

    if (iniReader.ParseError() != 0) {
        std::cerr
            << "Error: cannot read config file: "
            << argv[1]
            << std::endl;
        return 1;
    }

    makeLikelihoodReco(iniReader);

    return 0;
}




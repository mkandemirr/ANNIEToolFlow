#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TF1.h"

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#include "LAPPDPulseReader.h"
#include "LAPPDHitReader.h"
#include "PMTHitReader.h"
#include "FMVHitReader.h"
#include "ANNIEEventReader.h"
#include "LAPPDMetaReader.h"
#include "TankClusterReader.h"
#include "MRDClusterReader.h"
#include "MRDRecoReader.h"
#include "MCANNIEEventReader.h"
#include "ANNIEUtility.h"
#include "INIReader.h"

// -----------------------------------------------------------------------------------
// Simple reconstruction output container
// -----------------------------------------------------------------------------------

struct SimpleRecoResult {
   bool success = false;
   bool fv = false;   // Fiducial volume flag. eger fv= true ise, success = true her zaman

   double x = -9999;
   double y = -9999;
   double z = -9999;

   double dirx = -9999;
   double diry = -9999;
   double dirz = -9999;

   double exitx = -9999;
   double exity = -9999;
   double exitz = -9999;

   double E = -9999;
};

// -----------------------------------------------------------------------------------

struct EnergyCalibration {
   bool valid = false;

   double p0 = 0.0;   // p0 [MeV]
   double p1  = 0.0;   // p1 [MeV / PE]

   Long64_t nUsed = 0;
};

// -----------------------------------------------------------------------------------

SimpleRecoResult DoSimpleRecoFromMRD(
   const MRDReco& reco,
   double totalPE,
   const EnergyCalibration& calib
);

// ------------------------------------------------------------

bool ComputeOuterDistanceFromMRD(
   const MRDReco& reco,
   double& dist_pmtvol_tank
);

// ------------------------------------------------------------

EnergyCalibration FitTankEnergyCalibration(
   TTree* inputTree,
   MRDRecoReader& mrdRecoReader,
   TankClusterReader& tankClusterReader,
   MCANNIEEventReader& mcReader
);

// ------------------------------------------------------------

void makeSimpleReco(INIReader& iniReader);

// -----------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
   INIReader iniReader(argv[1]);
   makeSimpleReco(iniReader);

   return 0;
}

// -----------------------------------------------------------------------------------

void makeSimpleReco(INIReader& iniReader)
{
   std::string inputFile = iniReader.Get("Input", "InputFile", "");

   std::string outputFile = iniReader.Get("Output", "OutputFile", "SimpleReco.root");

   TFile* inputRootFile = TFile::Open(inputFile.c_str(), "READ");

   if (!inputRootFile || inputRootFile->IsZombie()) {
      std::cerr << "Error opening file: " << inputFile << std::endl;
      return;
   }

   TTree* inputTree = (TTree*)inputRootFile->Get("Event");

   if (!inputTree) {
      std::cerr << "Error: Tree 'Event' not found in file!" << std::endl;
      inputRootFile->Close();
      return;
   }


   TFile* outputRootFile = TFile::Open(outputFile.c_str(), "RECREATE");

   if (!outputRootFile || outputRootFile->IsZombie()) {
      std::cerr << "Error creating output file: " << outputFile << std::endl;
      inputRootFile->Close();
      return;
   }

   TTree* outputTree = inputTree->CloneTree(0);

   Long64_t nEntries = inputTree->GetEntries();
   std::cout << "Number of events in this tree: " << nEntries << std::endl;


   // -------------------------------------------------------------------------------
   // Output variables
   // -------------------------------------------------------------------------------
   // will be stored wrt global coordinate and in units of meter
   double SimpleRecoX = -9999;
   double SimpleRecoY = -9999;
   double SimpleRecoZ = -9999;

   double SimpleRecoDirX = -9999;
   double SimpleRecoDirY = -9999;
   double SimpleRecoDirZ = -9999;

   double SimpleRecoEnergy = -9999;

   int SimpleRecoFlag = -9999;  // 1 if simple reco succeeds, 0 otherwise
   int SimpleRecoFV   = -9999;  // 1 if reco vertex is inside fiducial volume

   outputTree->Branch("SimpleRecoX",      &SimpleRecoX,      "SimpleRecoX/D");
   outputTree->Branch("SimpleRecoY",      &SimpleRecoY,      "SimpleRecoY/D");
   outputTree->Branch("SimpleRecoZ",      &SimpleRecoZ,      "SimpleRecoZ/D");

   outputTree->Branch("SimpleRecoDirX",   &SimpleRecoDirX,   "SimpleRecoDirX/D");
   outputTree->Branch("SimpleRecoDirY",   &SimpleRecoDirY,   "SimpleRecoDirY/D");
   outputTree->Branch("SimpleRecoDirZ",   &SimpleRecoDirZ,   "SimpleRecoDirZ/D");

   outputTree->Branch("SimpleRecoEnergy", &SimpleRecoEnergy, "SimpleRecoEnergy/D");
   outputTree->Branch("SimpleRecoFlag",   &SimpleRecoFlag,   "SimpleRecoFlag/I");
   outputTree->Branch("SimpleRecoFV",     &SimpleRecoFV,     "SimpleRecoFV/I");

   MRDRecoReader mrdRecoReader;
   mrdRecoReader.Init(inputTree);

   TankClusterReader tankClusterReader;
   tankClusterReader.Init(inputTree);

   MCANNIEEventReader mcReader;
   mcReader.Init(inputTree);

   // -------------------------------------------------------------------------------
   // Fit tank energy calibration using MC truth
   // ETank = Etrue - E_outer - E_mrd
   // ETank = p0 + p1 * totalPE
   // -------------------------------------------------------------------------------

   EnergyCalibration calib = FitTankEnergyCalibration(
                                inputTree,
                                mrdRecoReader,
                                tankClusterReader,
                                mcReader
                                );

   if (!calib.valid) {
      std::cerr << "Error: tank energy calibration failed." << std::endl;
      outputRootFile->Close();
      inputRootFile->Close();
      delete outputRootFile;
      delete inputRootFile;
      return;
   }



   // -------------------------------------------------------------------------------
   // Event loop
   // -------------------------------------------------------------------------------

   Long64_t nRecoSuccess = 0;
   Long64_t nMultipleRecoEvents = 0;
   Long64_t nFVEvents = 0;
   for (Long64_t i = 0; i < nEntries; ++i) {

      inputTree->GetEntry(i);

      // Reset output values for each event
      SimpleRecoX = -9999;
      SimpleRecoY = -9999;
      SimpleRecoZ = -9999;

      SimpleRecoDirX = -9999;
      SimpleRecoDirY = -9999;
      SimpleRecoDirZ = -9999;

      SimpleRecoEnergy = -9999;
      SimpleRecoFlag = 0;
      SimpleRecoFV = 0;

      // Read MRD reconstructed tracks
      EMRDRecos eMRDRecos;
      mrdRecoReader.ReadEntry(i, eMRDRecos);

      // Get total PMT charge in prompt window: 0 - 2000 ns
      double totalPE =
      ANNIEUtility::GetBestClusterPE(tankClusterReader,
                                    i,
                                    0.0,
                                    2000.0);

      const auto& recos = eMRDRecos.Get();

      // Require exactly one MRDReco track
      if (recos.size() == 1)
      {
         const auto& reco = recos.at(0);

         SimpleRecoResult simple =
         DoSimpleRecoFromMRD(reco,
                           totalPE,
                           calib
                           );

         SimpleRecoX = simple.x;
         SimpleRecoY = simple.y;
         SimpleRecoZ = simple.z;

         SimpleRecoDirX = simple.dirx;
         SimpleRecoDirY = simple.diry;
         SimpleRecoDirZ = simple.dirz;

         SimpleRecoEnergy = simple.E;

         SimpleRecoFlag = simple.success ? 1 : 0;
         SimpleRecoFV   = simple.fv ? 1 : 0;

         if(simple.success)
         {
            ++nRecoSuccess;
         }

         if(simple.fv)
         {
            ++nFVEvents;
         }

      }

      if(recos.size() > 1)
         ++nMultipleRecoEvents;

      outputTree->Fill();
   }
    

   std::cout << "----------------------------------" << std::endl;
   std::cout << "makeSimpleReco" << std::endl;
   std::cout << "InputFile : " << inputFile << std::endl;
   std::cout << "OutputFile: " << outputFile << std::endl;
   std::cout << "Input entries            : " << inputTree->GetEntries() << std::endl;
   std::cout << "Output tree entries      : " << outputTree->GetEntries() << std::endl;
   std::cout << "Reco success events      : " << nRecoSuccess << std::endl;
   std::cout << "FV events                : " << nFVEvents << std::endl;
   std::cout << "Multiple MRDReco events  : " << nMultipleRecoEvents<< std::endl;
   std::cout << "----------------------------------" << std::endl;

   outputRootFile->cd();
   outputTree->Write();

   outputRootFile->Close();
   inputRootFile->Close();

   delete outputRootFile;
   delete inputRootFile;

    
}

// -----------------------------------------------------------------------------------

SimpleRecoResult DoSimpleRecoFromMRD(const MRDReco& reco,
                                     double totalPE,
                                     const EnergyCalibration& calib)
{
    SimpleRecoResult out;

    // -------------------------------------------------------------------------------
    // Geometry constants
    // -------------------------------------------------------------------------------

    const double tank_z_center   = 1.681;   // Tank center position in global z [m]
    const double innerCylRadius   = 1.0;     // PMT Inner volume radius [m]
    const double outerCylRadius   = 1.5;     // tank radius [m]

    const double fv_radius_cut = 1.0;     // Fiducial radial cut [m]
    const double fv_y_cut      = 0.5;     // Fiducial y cut [m]

    // -------------------------------------------------------------------------------
    // Energy reconstruction constants
    // -------------------------------------------------------------------------------

    const double tank_water_eloss_per_m = 199.2;   // Approximate energy loss per meter [MeV/m]

    const double p0 = calib.p0;  // fitted p0 [MeV]
    const double p1  = calib.p1;   // fitted p1 [MeV / PE]

    // -------------------------------------------------------------------------------
    // Basic event quality checks
    //
    // Use only stopping MRD tracks.
    // Also require non-negative PMT charge.
    // -------------------------------------------------------------------------------

    if (!reco.IsStop() || totalPE < 0)
    {
      out.success = false; //zaten defaul false ama yine de bulunsun okunurluk acisindan
      return out;
    }
        
    // -------------------------------------------------------------------------------
    // MRD track start and stop positions in global coordinates [m]
    // -------------------------------------------------------------------------------
   //meter 
    const double startx = reco.GetStartX();
    const double starty = reco.GetStartY();
    const double startz = reco.GetStartZ();

    const double stopx = reco.GetStopX();
    const double stopy = reco.GetStopY();
    const double stopz = reco.GetStopZ();

    // -------------------------------------------------------------------------------
    // MRD track direction vector before normalization
    //
    // This vector points from MRD start to MRD stop.
    // -------------------------------------------------------------------------------

    const double diffx = stopx - startx;
    const double diffy = stopy - starty;
    const double diffz = stopz - startz;

    const double L = std::sqrt(diffx*diffx + diffy*diffy + diffz*diffz);

    if (L <= 0)
    {
      out.success = false; //zaten defaul false ama yine de bulunsun okunurluk acisindan
      return out;
    }
        

    // -------------------------------------------------------------------------------
    // Normalize direction vector
    //
    // dir = (stop - start) / |stop - start|
    // -------------------------------------------------------------------------------

    out.dirx = diffx / L;
    out.diry = diffy / L;
    out.dirz = diffz / L;

    // -------------------------------------------------------------------------------
    // Cylinder-line intersection for PMT volume exit point
    //
    // Tank is assumed to be a cylinder along y-axis.
    // Therefore the cylinder equation is:
    //
    // x^2 + (z - tank_z_center)^2 = innerCylRadius^2
    //
    // Track is extrapolated backward from MRD start:
    //
    // x(t) = startx - t * dirx
    // y(t) = starty - t * diry
    // z(t) = startz - t * dirz
    //
    // Substitute x(t), z(t) into the cylinder equation:
    //
    // (startx - t*diffx)^2
    // + (startz - tank_z_center - t*diffz)^2
    // = innerCylRadius^2
    //
    // This gives:
    //
    // a t^2 + b t + c = 0
    //
    // where:
    //
    // a = diffx^2 + diffz^2
    // b = -2*diffx*startx - 2*diffz*(startz - tank_z_center)
    // c = startx^2 + (startz - tank_z_center)^2 - innerCylRadius^2
    // -------------------------------------------------------------------------------

    const double startz_c = startz - tank_z_center;

    const double a = out.dirx*out.dirx + out.dirz*out.dirz;
    const double b = -2.0*out.dirx*startx - 2.0*out.dirz*startz_c;
    
    const double c = startx*startx + startz_c*startz_c - innerCylRadius*innerCylRadius;

    const double disc = b*b - 4.0*a*c;

    if (a == 0.0 || disc < 0.0)
    {
       out.success = false; //zaten defaul false ama yine de bulunsun okunurluk acisindan
       return out;
    }
       

    const double sqrt_disc = std::sqrt(disc);

    const double t1 = (-b + sqrt_disc) / (2.0*a);
    const double t2 = (-b - sqrt_disc) / (2.0*a);

    // -------------------------------------------------------------------------------
    // Choose physical solution
    //
    // t > 0 corresponds to going backward from MRD start toward the tank.
    //
    // If both solutions are positive, choose the smaller one:
    // this is the closest tank surface intersection.
    // -------------------------------------------------------------------------------

    double t = -9999;

    if (t1 >= 0.0 && t2 >= 0.0)
        t = (t1 < t2) ? t1 : t2;
    else if (t1 >= 0.0)
        t = t1;
    else if (t2 >= 0.0)
        t = t2;
    else
        return out; //burda da out.success = false doner default

    // -------------------------------------------------------------------------------
    // Tank PMT volume exit position
    //
    // This is where the extrapolated MRD track intersects the tank cylinder.
    // -------------------------------------------------------------------------------
    
    out.exitx = startx - t*out.dirx;
    out.exity = starty - t*out.diry;
    out.exitz = startz - t*out.dirz;

    // -------------------------------------------------------------------------------
    // Estimate muon track length inside tank from PMT charge
    //
    // Original expression:
    // totalPE = p0 + p1*trackLength; //assuming lineer relation
    
    //this fit parameters are obtained using the fitChargeTrackLength tool 
    //double p0 = -383.898; //pe
    //double p1 = 1642.77;  // unit : PE/m 
    //const double trackLengthTank = (totalPE -p0)/p1 ; 
    const double trackLengthTank = (totalPE*p1)/tank_water_eloss_per_m;

    // -------------------------------------------------------------------------------
    // Vertex reconstruction
    //
    // Move backward from the tank exit point along the reconstructed direction.
    //
    // vertex = exit - trackLengthTank * direction
    // -------------------------------------------------------------------------------

    out.x = out.exitx - trackLengthTank*out.dirx;
    out.y = out.exity - trackLengthTank*out.diry;
    out.z = out.exitz - trackLengthTank*out.dirz;

    // -------------------------------------------------------------------------------
    // Fiducial volume check
    //
    // Reconstructed vertex must satisfy:
    //
    // 1) radial position inside tank:
    //    sqrt(x^2 + (z - tank_z_center)^2) < fv_radius_cut
    //
    // 2) central region in y:
    //    |y| < fv_y_cut
    //
    // 3) negative-z side relative to tank center:
    //    z - tank_z_center < 0
    //
    // If all are true, SimpleRecoFV = 1.
    // -------------------------------------------------------------------------------

    const double reco_r = std::sqrt(
        out.x*out.x +
        (out.z - tank_z_center)*(out.z - tank_z_center)
    );

    if (reco_r < fv_radius_cut &&
        std::fabs(out.y) < fv_y_cut &&
        (out.z - tank_z_center) < 0.0) {
        out.fv = true;
    }

   // -------------------------------------------------------------------------------
   // Distance between PMT-volume exit point and outer tank volume
   // -------------------------------------------------------------------------------

   double dist_pmtvol_tank = -9999;

   bool okOuter =
       ComputeOuterDistanceFromMRD(
           reco,
           dist_pmtvol_tank
       );

   if (!okOuter) {
       out.success = false;
       return out;
   }

   // -------------------------------------------------------------------------------
   // Simple energy reconstruction
   //
   // E_mu_reco = alpha * Q + beta * d_outer + E_loss_MRD + c
   // -------------------------------------------------------------------------------

   const double E_tank        = p1 * totalPE + p0;
   const double E_outer       = tank_water_eloss_per_m * dist_pmtvol_tank;
   const double E_mrd         = reco.GetEnergyLoss();

   out.E = E_tank + E_outer + E_mrd;
       
   out.success = true;
    return out;
}


// -------------------------------------------------------------------------------
 
bool ComputeOuterDistanceFromMRD(
    const MRDReco& reco,
    double& dist_pmtvol_tank
)
{
    dist_pmtvol_tank = -9999;

    const double tank_z_center = 1.681;
    const double innerCylRadius = 1.0;
    const double outerCylRadius = 1.5;

    if (!reco.IsStop())
        return false;

    const double startx = reco.GetStartX();
    const double starty = reco.GetStartY();
    const double startz = reco.GetStartZ();

    const double stopx = reco.GetStopX();
    const double stopy = reco.GetStopY();
    const double stopz = reco.GetStopZ();

    const double diffx = stopx - startx;
    const double diffy = stopy - starty;
    const double diffz = stopz - startz;

    const double L = std::sqrt(diffx*diffx + diffy*diffy + diffz*diffz);

    if (L <= 0.0)
        return false;

    const double dirx = diffx / L;
    const double diry = diffy / L;
    const double dirz = diffz / L;

    const double startz_c = startz - tank_z_center;

    const double a = dirx*dirx + dirz*dirz;
    const double b = -2.0*dirx*startx - 2.0*dirz*startz_c;

    if (a == 0.0)
        return false;

    // -------------------------------------------------------------------------------
    // Inner PMT-volume intersection
    // -------------------------------------------------------------------------------

    const double c_inner = startx*startx + startz_c*startz_c -
                            innerCylRadius*innerCylRadius;

    const double disc_inner = b*b - 4.0*a*c_inner;

    if (disc_inner < 0.0)
        return false;

    const double sqrt_disc_inner = std::sqrt(disc_inner);

    const double t1_inner = (-b + sqrt_disc_inner) / (2.0*a);
    const double t2_inner = (-b - sqrt_disc_inner) / (2.0*a);

    double t_inner = -9999;

    if (t1_inner >= 0.0 && t2_inner >= 0.0)
        t_inner = (t1_inner < t2_inner) ? t1_inner : t2_inner;
    else if (t1_inner >= 0.0)
        t_inner = t1_inner;
    else if (t2_inner >= 0.0)
        t_inner = t2_inner;
    else
        return false;

    const double inner_x = startx - t_inner * dirx;
    const double inner_y = starty - t_inner * diry;
    const double inner_z = startz - t_inner * dirz;

    // -------------------------------------------------------------------------------
    // Outer tank intersection
    // -------------------------------------------------------------------------------

    const double c_outer = startx*startx + startz_c*startz_c -
                           outerCylRadius*outerCylRadius;

    const double disc_outer = b*b - 4.0*a*c_outer;

    if (disc_outer < 0.0)
        return false;

    const double sqrt_disc_outer = std::sqrt(disc_outer);

    const double t1_outer = (-b + sqrt_disc_outer) / (2.0*a);
    const double t2_outer = (-b - sqrt_disc_outer) / (2.0*a);

    double t_outer = -9999;

    if (t1_outer >= 0.0 && t2_outer >= 0.0)
        t_outer = (t1_outer < t2_outer) ? t1_outer : t2_outer;
    else if (t1_outer >= 0.0)
        t_outer = t1_outer;
    else if (t2_outer >= 0.0)
        t_outer = t2_outer;
    else
        return false;

    const double outer_x = startx - t_outer * dirx;
    const double outer_y = starty - t_outer * diry;
    const double outer_z = startz - t_outer * dirz;

    dist_pmtvol_tank =
        std::sqrt(
            (outer_x - inner_x)*(outer_x - inner_x) +
            (outer_y - inner_y)*(outer_y - inner_y) +
            (outer_z - inner_z)*(outer_z - inner_z)
        );

    return true;
}

 // -------------------------------------------------------------------------------
 
EnergyCalibration FitTankEnergyCalibration(
    TTree* inputTree,
    MRDRecoReader& mrdRecoReader,
    TankClusterReader& tankClusterReader,
    MCANNIEEventReader& mcReader
)
{
    EnergyCalibration calib;

    if (!inputTree) {
        std::cerr << "Error: inputTree is null in FitTankEnergyCalibration."
                  << std::endl;
        return calib;
    }

    const double tank_water_eloss_per_m = 199.2; // MeV/m

    std::vector<double> totalPE_values;
    std::vector<double> Etank_values;

    Long64_t nEntries = inputTree->GetEntries();

    Long64_t nSkippedReco = 0;
    Long64_t nSkippedPE = 0;
    Long64_t nSkippedGeom = 0;
    Long64_t nSkippedEnergy = 0;

    for (Long64_t i = 0; i < nEntries; ++i) {

        inputTree->GetEntry(i);

        EMRDRecos eMRDRecos;
        mrdRecoReader.ReadEntry(i, eMRDRecos);

        const auto& recos = eMRDRecos.Get();

        if (recos.size() != 1) {
            ++nSkippedReco;
            continue;
        }

        const auto& reco = recos.at(0);

        if (!reco.IsStop()) {
            ++nSkippedReco;
            continue;
        }

        double totalPE =
            ANNIEUtility::GetTotalPE(
                tankClusterReader,
                i,
                0.0,
                2000.0
            );

        if (totalPE <= 30.0) {
            ++nSkippedPE;
            continue;
        }

        double dist_pmtvol_tank = -9999;

        bool ok =
            ComputeOuterDistanceFromMRD(
                reco,
                dist_pmtvol_tank
            );

        if (!ok) {
            ++nSkippedGeom;
            continue;
        }

        double Etrue    = mcReader.GetTrueMuonEnergy();
        double E_outer  = tank_water_eloss_per_m * dist_pmtvol_tank;
        double E_mrd    = reco.GetEnergyLoss();
        double E_tank   = Etrue - E_outer - E_mrd;

        if (!std::isfinite(E_tank) || E_tank <= 0.0) {
            ++nSkippedEnergy;
            continue;
        }

        totalPE_values.push_back(totalPE);
        Etank_values.push_back(E_tank);
    }

    calib.nUsed = totalPE_values.size();

    if (calib.nUsed < 2) {
        std::cerr << "Error: not enough events for tank energy calibration."
                  << std::endl;
        std::cerr << "Used events: " << calib.nUsed << std::endl;
        return calib;
    }

    TGraph graph(calib.nUsed);

    for (Long64_t i = 0; i < calib.nUsed; ++i) {
        graph.SetPoint(
            i,
            totalPE_values[i],
            Etank_values[i]
        );
    }

    TF1 fitFunc(
        "fitFunc",
        "[0] + [1]*x",
        0.0,
        *std::max_element(totalPE_values.begin(), totalPE_values.end())
    );

    fitFunc.SetParName(0, "p0");
    fitFunc.SetParName(1, "p1");

    graph.Fit(&fitFunc, "Q");

    calib.p0 = fitFunc.GetParameter(0);
    calib.p1  = fitFunc.GetParameter(1);
    calib.valid = true;

    std::cout << "----------------------------------" << std::endl;
    std::cout << "Tank energy calibration" << std::endl;
    std::cout << "Model: ETank = p0 + p1 * totalPE" << std::endl;
    std::cout << "Used events       : " << calib.nUsed << std::endl;
    std::cout << "Skipped reco      : " << nSkippedReco << std::endl;
    std::cout << "Skipped PE        : " << nSkippedPE << std::endl;
    std::cout << "Skipped geometry  : " << nSkippedGeom << std::endl;
    std::cout << "Skipped energy    : " << nSkippedEnergy << std::endl;
    std::cout << "p0                : " << calib.p0 << " MeV" << std::endl;
    std::cout << "p1                : " << calib.p1 << " MeV/PE" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    return calib;
}

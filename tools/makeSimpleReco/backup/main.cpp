#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <vector>

#include "LAPPDPulseReader.h"
#include "LAPPDHitReader.h"
#include "PMTHitReader.h"
#include "FMVHitReader.h"
#include "ANNIEEventReader.h"
#include "LAPPDMetaReader.h"
#include "TankClusterReader.h"
#include "MRDClusterReader.h"
#include "MRDRecoReader.h"
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

SimpleRecoResult DoSimpleRecoFromMRD(const MRDReco& reco, double totalPE);
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
   std::string inputFile =
    iniReader.Get("Input", "InputFile", "");

   std::string outputFile =
    iniReader.Get("Output", "OutputFile",
                  "SimpleReco.root");
   
   TFile* inputRootFile = TFile::Open(
    inputFile.c_str(), "READ");

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
            ANNIEUtility::GetTotalPE(
                tankClusterReader,
                i,
                0.0,
                2000.0);

         const auto& recos = eMRDRecos.Get();

         // Require exactly one MRDReco track
         if (recos.size() == 1)
         {
             const auto& reco = recos.at(0);

             SimpleRecoResult simple = DoSimpleRecoFromMRD(reco, totalPE);

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

SimpleRecoResult DoSimpleRecoFromMRD(const MRDReco& reco, double totalPE)
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

    const double energy_offset = 87.3;          // Empirical offset [MeV]
    const double tank_water_eloss_per_m = 200;   // Approximate energy loss per meter [MeV/m]
    const double pe_to_energy = 0.08534;        // PE to energy coefficient [MeV / PE]

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
    // Tank exit position
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
    const double trackLengthTank = (totalPE*pe_to_energy)/tank_water_eloss_per_m;

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
    // Cylinder-line intersection for tank volume
    //
    // Same calculation as pmtVolume exit, but with radius = outerCylRadius.
    //
    // This gives another intersection point on the tank volume cylinder.
    // Distance between PMT-volume intersection and tank intersection is used
    // in the simple energy reconstruction.
    // -------------------------------------------------------------------------------

    const double cp =
        startx*startx + startz_c*startz_c - outerCylRadius*outerCylRadius;

    const double discp = b*b - 4.0*a*cp;

    double dist_pmtvol_tank = 0.0;

    if (discp >= 0.0) {

        const double sqrt_discp = std::sqrt(discp);

        const double t1p = (-b + sqrt_discp) / (2.0*a);
        const double t2p = (-b - sqrt_discp) / (2.0*a);

        double tp = -9999;

        if (t1p >= 0.0 && t2p >= 0.0)
            tp = (t1p < t2p) ? t1p : t2p;
        else if (t1p >= 0.0)
            tp = t1p;
        else if (t2p >= 0.0)
            tp = t2p;

        if (tp >= 0.0) {

            const double exitxp = startx - tp*out.dirx;
            const double exityp = starty - tp*out.diry;
            const double exitzp = startz - tp*out.dirz;

            dist_pmtvol_tank = std::sqrt(
                (exitxp - out.exitx)*(exitxp - out.exitx) +
                (exityp - out.exity)*(exityp - out.exity) +
                (exitzp - out.exitz)*(exitzp - out.exitz)
            );
        }
    }

    // -------------------------------------------------------------------------------
    // Simple energy reconstruction
    //
    // E = offset
    //   + air/path energy correction
    //   + MRD energy loss
    //   + PMT charge term
    // -------------------------------------------------------------------------------

    

   double E_outher = tank_water_eloss_per_m * dist_pmtvol_tank;
   double E_mrd = reco.GetEnergyLoss();
   double E_tank = energy_offset + pe_to_energy * totalPE;


    out.E = E_tank + E_outher + E_mrd;
       
  
  
  
  
/*    
//daha sonra bunu yap    
const double tank_eloss =
    tank_water_eloss_per_m * trackLengthTank;

const double outer_water_eloss =
    tank_water_eloss_per_m * dist_pmtvol_tank;

out.E =
    energy_offset +
    tank_eloss +
    outer_water_eloss +
    mrd_eloss;        
*/



    out.success = true;

    return out;
}

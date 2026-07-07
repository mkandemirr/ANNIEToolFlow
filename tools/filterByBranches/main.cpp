#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TLeaf.h>

#include "RootTreeReader.h"
#include "INIReader.h"

std::vector<std::string> SplitBranchList(const std::string& str);
void filterByBranches(const INIReader& iniReader);

int main(int argc, char* argv[])
{
   if (argc < 2) {
      std::cerr << "Usage: ./filterByBranches <config.ini>" << std::endl;
      return 1;
   }

   INIReader iniReader(argv[1]);

   if (iniReader.ParseError() != 0) {
      std::cerr << "Could not load config file: " << argv[1] << std::endl;
      return 1;
   }

   filterByBranches(iniReader);

   return 0;
}

void filterByBranches(const INIReader& iniReader)
{
   std::string inputFileName  = iniReader.Get("input",  "rootFile", "");
   std::string outputFileName = iniReader.Get("output", "rootFile", "");
   std::string treeName       = iniReader.Get("input",  "treeName", "Event");

   std::string branchListStr  = iniReader.Get("cuts", "branches", "");

   if (inputFileName.empty()) {
      std::cerr << "Error: input.rootFile is empty in config." << std::endl;
      return;
   }

   if (outputFileName.empty()) {
      std::cerr << "Error: output.rootFile is empty in config." << std::endl;
      return;
   }

   if (branchListStr.empty()) {
      std::cerr << "Error: cuts.branches is empty in config." << std::endl;
      return;
   }

   std::vector<std::string> branchNames = SplitBranchList(branchListStr);

   RootTreeReader treeReader(inputFileName.c_str(), treeName.c_str());

   if (!treeReader.IsValid())
      return;

   TTree* tree = treeReader.GetTree();

   std::vector<TLeaf*> leaves;

   for (const std::string& branchName : branchNames) {

      TBranch* branch = tree->GetBranch(branchName.c_str());

      if (!branch) {
         std::cerr << "Error: branch not found: " << branchName << std::endl;
         return;
      }

      TLeaf* leaf = branch->GetLeaf(branchName.c_str());

      if (!leaf) {
         leaf = branch->GetLeaf(branch->GetName());
      }

      if (!leaf) {
         std::cerr << "Error: could not get leaf for branch: "
                   << branchName << std::endl;
         return;
      }

      leaves.push_back(leaf);

      std::cout << "Using branch: " << branchName << std::endl;
   }

   TFile* outputFile = new TFile(outputFileName.c_str(), "RECREATE");

   if (!outputFile || outputFile->IsZombie()) {
      std::cerr << "Could not create output file: "
                << outputFileName << std::endl;
      return;
   }

   TTree* outputTree = tree->CloneTree(0);
   outputTree->SetName(treeName.c_str());
   outputTree->SetTitle("Branch Filtered Event Tree");

   Long64_t nEntries = tree->GetEntries();
   Long64_t nPassedEvents = 0;

   std::cout << "Number of events in this tree: "
             << nEntries << std::endl;

   for (Long64_t i = 0; i < nEntries; ++i) {

      tree->GetEntry(i);

      bool passedAllBranches = true;

      for (TLeaf* leaf : leaves) {

         double value = leaf->GetValue();

         if (value == 0) {
            passedAllBranches = false;
            break;
         }
      }

      if (passedAllBranches) {
         outputTree->Fill();
         ++nPassedEvents;
      }
   }

   std::cout << "Events passing branch filter: "
             << nPassedEvents << std::endl;
   std::cout << "Output tree entries: " << outputTree->GetEntries() << std::endl;          
   std::cout << "Output file: " << outputFileName << std::endl;
   
   
   outputFile->cd();
   outputTree->Write();
   outputFile->Close();

   
   
}

std::vector<std::string> SplitBranchList(const std::string& str)
{
   std::vector<std::string> result;
   std::stringstream ss(str);
   std::string item;

   while (std::getline(ss, item, ',')) {

      item.erase(0, item.find_first_not_of(" \t"));
      item.erase(item.find_last_not_of(" \t") + 1);

      if (!item.empty())
         result.push_back(item);
   }

   return result;
}

#include "RootTreeReader.h"

RootTreeReader::RootTreeReader(const std::string& filename, const std::string& treeName)
    : mFile(nullptr), mTree(nullptr), mValid(false)
{
    mFile = TFile::Open(filename.c_str());
    if (!mFile || mFile->IsZombie()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    mTree = dynamic_cast<TTree*>(mFile->Get(treeName.c_str()));
    if (!mTree) {
        std::cerr << "Error: Tree '" << treeName << "' not found in file: " << filename << std::endl;
        mFile->Close();
        delete mFile;
        mFile = nullptr;
        return;
    }

    mValid = true;
}

RootTreeReader::~RootTreeReader()
{
    if (mFile) {
        mFile->Close();
        delete mFile;
    }
}

bool RootTreeReader::IsValid() const {
    return mValid;
}

TTree* RootTreeReader::GetTree() const {
    return mTree;
}



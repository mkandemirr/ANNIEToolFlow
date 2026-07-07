#ifndef ROOTTREEREADER_H
#define ROOTTREEREADER_H

#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <string>

class RootTreeReader {
public:
    RootTreeReader(const std::string& filename, const std::string& treeName = "Event");
    ~RootTreeReader();

    bool IsValid() const;
    TTree* GetTree() const;

private:
    TFile* mFile;
    TTree* mTree;
    bool mValid;
};

#endif // ROOTTREEREADER_H



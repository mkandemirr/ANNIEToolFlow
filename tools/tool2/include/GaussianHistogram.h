#ifndef GAUSSIAN_HISTOGRAM_H
#define GAUSSIAN_HISTOGRAM_H

#include "INIReader.h"

class GaussianHistogram
{
public:
   explicit GaussianHistogram(const INIReader& iniReader);

   void Run();

private:
   const INIReader& fIniReader;
};

#endif

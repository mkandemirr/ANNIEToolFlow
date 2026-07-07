#include "GaussianHistogram.h"

#include <TCanvas.h>
#include <TF1.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <TStyle.h>

GaussianHistogram::GaussianHistogram(const INIReader& iniReader)
   : fIniReader(iniReader)
{
}

void GaussianHistogram::Run()
{
   int nEvents =
      fIniReader.GetInteger("histogram", "nEvents", 100000);

   int nBins =
      fIniReader.GetInteger("histogram", "nBins", 100);

   double mean =
      fIniReader.GetReal("histogram", "mean", 0.0);

   double sigma =
      fIniReader.GetReal("histogram", "sigma", 1.0);

   double xMin =
      fIniReader.GetReal("histogram", "xMin", -5.0);

   double xMax =
      fIniReader.GetReal("histogram", "xMax", 5.0);

   gStyle->SetOptFit(1111);

   TH1D* histogram = new TH1D(
      "histogram",
      "Gaussian Distribution;X;Entries",
      nBins,
      xMin,
      xMax);

   TRandom3 random(0);

   for (int i = 0; i < nEvents; ++i)
      histogram->Fill(random.Gaus(mean, sigma));

   TCanvas* canvas = new TCanvas(
   "canvas",
   "ToolFlow Example",
   900,
   600);

   histogram->Draw();

   TF1* fit = new TF1("fit", "gaus", xMin, xMax);
   histogram->Fit(fit, "Q");

   //canvas->Update();
}

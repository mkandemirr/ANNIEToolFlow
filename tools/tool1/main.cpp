#include <iostream>
#include <string>

#include <TRootCanvas.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TRandom3.h>
#include <TStyle.h>

#include "INIReader.h"

void tool1(const INIReader& iniReader);

int main(int argc, char* argv[])
{
   if (argc < 2) {
      std::cerr << "Usage: ./tool1 <config.ini>" << std::endl;
      return 1;
   }

   INIReader iniReader(argv[1]);

   if (iniReader.ParseError() != 0) {
      std::cerr << "Error: could not load config file: "
                << argv[1] << std::endl;
      return 1;
   }

   TApplication app("app", &argc, argv);

   tool1(iniReader);

    // -----------------------------------------------------------------------------------
    // Pencereyi kapatınca uygulamayı sonlandır
    // -----------------------------------------------------------------------------------
    TRootCanvas *rc = (TRootCanvas *)gPad->GetCanvas()->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app.Run();

    return 0;
}

void tool1(const INIReader& iniReader)
{
   int nEvents = iniReader.GetInteger("histogram", "nEvents", 10000);
   int nBinsX  = iniReader.GetInteger("histogram", "nBinsX", 100);
   int nBinsY  = iniReader.GetInteger("histogram", "nBinsY", 100);

   double xMin = iniReader.GetReal("histogram", "xMin", -5.0);
   double xMax = iniReader.GetReal("histogram", "xMax",  5.0);
   double yMin = iniReader.GetReal("histogram", "yMin", -5.0);
   double yMax = iniReader.GetReal("histogram", "yMax",  5.0);

   gStyle->SetOptStat(1110);

   TH2D* h2 = new TH2D(
      "h2",
      "Example 2D Histogram;X variable;Y variable",
      nBinsX, xMin, xMax,
      nBinsY, yMin, yMax
   );

   TRandom3 random(0);

   for (int i = 0; i < nEvents; ++i) {
      double x = random.Gaus(0.0, 1.0);
      double y = 0.5 * x + random.Gaus(0.0, 1.0);

      h2->Fill(x, y);
   }

   TCanvas* canvas = new TCanvas(
      "canvas",
      "ToolFlow Example Tool 1",
      900,
      700
   );
   //canvas->Connect("Closed()", "TApplication", gApplication, "Terminate()");

   h2->Draw("COLZ");

   canvas->Update();
}

#include <iostream>

#include <TApplication.h>
#include <TRootCanvas.h>

#include "GaussianHistogram.h"
#include "INIReader.h"
#include <TCanvas.h>

int main(int argc, char* argv[])
{
   if (argc < 2) {
      std::cerr << "Usage: ./tool2 <config.ini>" << std::endl;
      return 1;
   }

   INIReader iniReader(argv[1]);

   if (iniReader.ParseError() != 0) {
      std::cerr << "Error: could not load config file: "
                << argv[1] << std::endl;
      return 1;
   }

   TApplication app("app", &argc, argv);

   GaussianHistogram histogram(iniReader);
   histogram.Run();
   
   // -----------------------------------------------------------------------------------
    // Pencereyi kapatınca uygulamayı sonlandır
    // -----------------------------------------------------------------------------------
    TRootCanvas *rc = (TRootCanvas *)gPad->GetCanvas()->GetCanvasImp();
    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
    app.Run();

   return 0;
}

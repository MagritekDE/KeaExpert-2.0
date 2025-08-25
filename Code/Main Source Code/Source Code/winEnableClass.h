#ifndef WINENABLECLASS_H
#define WINENABLECLASS_H

// Class 

class CWinEnable
{
   public:
      void Enable(HWND);
      void Disable(HWND);
   		
   private:
      bool prospaWinEnabled;
      bool plot1DWinEnabled;
      bool plot2DWinEnabled;
      bool plot3DWinEnabled;
      bool cliWinEnabled;
      bool editWinEnabled;
};

#endif // define WINENABLECLASS_H
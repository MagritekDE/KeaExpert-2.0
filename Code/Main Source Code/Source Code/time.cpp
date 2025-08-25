#include "stdafx.h"
#include "mytime.h"
#include "globals.h"
#include "interface.h"
#include "mymath.h"
#include "process.h"
#include <time.h>
#include "scanstrings.h"
#include "variablesOther.h"
#include "variablesClass.h"
#include "hr_time.h"
#include "memoryLeak.h"

//float currentTime = 0;
double currentTime = 0.0;


/******************************************************************************
*                           Get the current time                              *
*                                                                             *
* Syntax:  gettime([STR format])                                              *
*                                                                             *
* Possible formats: use standard C formating for hr:min:sec:ms                *
******************************************************************************/

int GetTimeOfDay(Interface *itfc, char arg[])
{
   SYSTEMTIME loctime;
   CText time;
   CText format = "%02hd:%02hd:%02hd";
   short r;

   GetLocalTime(&loctime);

   if((r = ArgScan(itfc,arg,0,"format","e","t",&format)) < 0)
      return(r);  

   time.Format(format.Str(),loctime.wHour,loctime.wMinute,loctime.wSecond,loctime.wMilliseconds);

   itfc->retVar[1].MakeAndSetString(time.Str());
   itfc->nrRetValues = 1;

   return(OK);
}

/******************************************************************************
*                           Get the current date                              *
*                                                                             *
* Syntax:  getdate([STR format])                                              *
*                                                                             *
* Possible formats: dd:mm|mmm:yy|yyyy  or mm|mmm:dd:yy|yyyy                   *
*                   or yy|yyyy:mm|mmm:dd                                      *
* The delimiter (here :) may be any single character                         *
******************************************************************************/

int GetDate(Interface *itfc, char arg[])
{
   SYSTEMTIME loctime;

   char *months[] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
   static CText dateFormat = "dd-mmm-yy";
   int r;
   extern int RegexMatch(Interface* itfc, char args[]);

   if((r = ArgScan(itfc,arg,0,"[format]","e","t",&dateFormat)) < 0)
      return(r); 

   CText argTxt;
   argTxt.Format("\"%s\",\"(dd|yy|yyyy|mm|mmm)(.)(mm|mmm|dd)(.)(yy|yyyy|dd)\"",dateFormat.Str());

   if(RegexMatch(itfc,argTxt.Str()) == ERR)
      return(ERR);

   if(itfc->nrRetValues != 5)
   {
      ErrorMessage("should be 5 substrings - check date format string");
      return(ERR);
   }

   GetLocalTime(&loctime);

   CText result;
   CText val;
   CText date;
   for(int i = 1 ; i <= 5; i++)
   {
      if(itfc->retVar[i].GetType() == UNQUOTED_STRING)
      {
         val = itfc->retVar[i].GetString();
         if(val == "dd")
         {
            date.Format("%02d",loctime.wDay);
            result = result + date;
         }
         else if(val == "mm")
         {
            date.Format("%02d",loctime.wMonth);
            result = result + date;
         }
         else if(val == "mmm")
         {
            date.Format("%s",months[loctime.wMonth]);
            result = result + date;
         }
         else if(val == "yy")
         {
            date.Format("%02d",loctime.wYear);
            result = result + date.Middle(2,3);
         }
         else if(val == "yyyy")
         {
            date.Format("%4d",loctime.wYear);
            result = result + date;
         }
         else
            result = result + val;
      }
      else
      {
         ErrorMessage("arguments should be strings");
         return(ERR);
      }
   }


   itfc->retVar[1].MakeAndSetString(result.Str());
   itfc->nrRetValues = 1;

   return(OK);
}

/******************************************************************************
*                           Set or get current time in seconds                *
*                                                                             *
* Syntax:  time(currentTime, precision) # set time to currentTime             *
*          time() # function call to extract current time.                    *
******************************************************************************/

int GetorSetElapsedTime(Interface* itfc ,char arg[])
{
   static float time;
   short r;
   static CText precision = "single";
   
   if(arg[0] != '\0')
   {
      if((r = ArgScan(itfc,arg,1,"time in seconds[, precision]","ee","ft",&time, &precision)) < 0)
         return(r);  

    //  if (r == 1)
    //     precision = "single";

      currentTime = (double)GetMsTime()/1000.0L - time;
      itfc->nrRetValues = 0;
   }
   else
   {
      if (precision == "single" || precision == "float")
      {
         itfc->retVar[1].MakeAndSetFloat((float)(GetMsTime() / 1000.0L - currentTime));
      }
      else
      {
         itfc->retVar[1].MakeAndSetDouble((GetMsTime() / 1000.0L - currentTime));
      }
      itfc->nrRetValues = 1;
	}
   return(0);
}

/******************************************************************************
*                           Pause a predetermined length of time              *
*                                                                             *
* Syntax:  pause(delay) # pause 'delay' seconds (1ms is smallest delay)       *
******************************************************************************/

int Pause(Interface* itfc ,char args[])
{
   float fdelay;
   double ddelay;
   short r;
   CText sleepModeStr = "nosleep";
   CText eventModeStr = "events";
   bool sleepMode = false;
   bool eventMode = true;

	if((r = ArgScan(itfc,args,1,"time in seconds","eee","ftt",&fdelay,&sleepModeStr,&eventModeStr)) < 0){
		itfc->nrRetValues = 0;
      return(r);  
	}

   if(sleepModeStr == "sleep")
      sleepMode = true;

   if(eventModeStr == "noevents")
      eventMode = false;

// Check for valid delays
   if(fdelay == 0)
      return(OK);

   if(fdelay < 0)
   {
      ErrorMessage("Negative delay");
      return(ERR);
   }

   ddelay = (double)fdelay*1000;
   double t1;

   t1= GetMsTime();

// Delay but allowing user to move windows and halt process
   bool bak1 = gBlockWaitCursor; // Don't allow the wait cursor to come on
   bool bak2 = gShowWaitCursor; // Don't allow the wait cursor to come on
   gBlockWaitCursor = true;
   gShowWaitCursor = false;
   while((GetMsTime()-t1) < ddelay)
   {
      if(eventMode)
      {
         gCheckForEvents = true; // Make sure other programs get access to background events each ms
         if(ProcessBackgroundEvents() != OK)
            return(ABORT);
      }
      if(sleepMode)
         Sleep(1); // Allow the rest of the computer some time to work

   }
   gShowWaitCursor = bak1;
   gBlockWaitCursor = bak2;

	itfc->nrRetValues = 0;
   return(OK);
}


/*************************************************************************
*       Return the value of the highres clock in milliseconds
*************************************************************************/

double GetMsTime()
{
	LARGE_INTEGER tick,freq;
	QueryPerformanceCounter(&tick);
	QueryPerformanceFrequency(&freq);
	double time = 1000.0L*(double)tick.QuadPart/(double)freq.QuadPart;
	return(time);
}

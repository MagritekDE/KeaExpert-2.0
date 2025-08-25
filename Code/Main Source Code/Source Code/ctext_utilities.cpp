#include "stdafx.h"
#include "ctext.h"
#include "ctext_utilities.h"
#include "memoryLeak.h"


// Replace all instances of findStr in inStr with replaceStr

void ReplaceSubStr(CText &inStr,char *findStr, char* replaceStr)
{
   int i,j;
   CText result = "";
   int szIn = inStr.Size();
   int szFind = strlen(findStr);
   int szReplace = strlen(replaceStr);

   for(i = 0; i < szIn; i++)
   {
      for(j = 0; j < szFind; j++)
      {
         if(inStr[i+j] != findStr[j])
            break;
      }
      if(j == szFind)
      {
         result = result + replaceStr;
         i += szFind-1;
      }
      else
      {
         result.Append(inStr[i]);
      }
   }
   inStr = result;
}

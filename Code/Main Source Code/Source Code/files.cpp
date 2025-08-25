#include "stdafx.h"
#include "files.h"
#include "windows.h"
//#include "winnls.h"
//#include "objbase.h"
//#include "objidl.h"

#include <Shobjidl.h>
#include <Shlobj.h>
#include <shlguid.h>
#include <shellapi.h>
#include "globals.h"
#include "interface.h"
#include "list_functions.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "string_utilities.h"
#include "memoryLeak.h"

void GetBasePathCore(char *fullPath, char *basePath);
int RemoveFolder(Interface *itfc, char args[]);
HRESULT CreateLink(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc); 
int MakeLink(Interface *itfc, char args[]);
void GetFileDate(HANDLE hFile, char* date, int mode);
void GetFileDateAndTime(HANDLE hFile, char* dateNTime, int mode);
float GetFileAge(HANDLE hFile);

// Resolve a link replacing it with its correct address

int ResolveLink(Interface *itfc, char args[])
{
   char linkIn[MAX_PATH];
   char linkOut[MAX_PATH];
   short nrArgs;
   CText nameIn;
   memset(linkIn, 0, MAX_PATH);
   IShellLinkA * pISL;   

// Extract the full link path name and file (target.Lnk)
   if((nrArgs = ArgScan(itfc,args,1,"name","e","t",&nameIn)) < 0)
      return(nrArgs); 

    strncpy(linkIn,nameIn.Str(),MAX_PATH);


//
//// Check to see if the directory exists
//   GetDirectory(curDir);
//   if(!SetDirectory(dir))
//   {
//      ErrorMessage("Directory '%s' does not exist", dir.Str());
//      return(ERR);
//   }
//
//// Does a target link file exist?
//// if not just return full path name
//   if(!IsFile("target.Lnk"))
//   {
//      GetDirectory(fullPath);
//      SetDirectory(curDir);
//      itfc->retVar[1].MakeAndSetString(fullPath.Str());
//      itfc->nrRetValues = 1;
//      return(OK);
//   }
//
//// Get the full path name
//
//   GetDirectory(fullPath);
//   fullPath = fullPath + "\\target.Lnk";
//   strncpy(linkIn,fullPath.Str(),MAX_PATH);
//
//   SetDirectory(curDir);

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**) &pISL);
   
    if(SUCCEEDED(hr))
    {
        IPersistFile *ppf;

        hr = pISL->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if(SUCCEEDED(hr))
        {
            WCHAR wsz[MAX_PATH];
           
            //Get a UNICODE wide string wsz from the Link path
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, linkIn, -1, wsz,MAX_PATH);

            //Read the link into the persistent file
            hr = ppf->Load(wsz, 0);
           
            if(SUCCEEDED(hr))
            {
                //Read the target information from the link object
                //UNC paths are supported (SLGP_UNCPRIORITY)

                if (pISL->GetPath(linkOut, MAX_PATH, NULL,SLGP_UNCPRIORITY) == S_OK)
                {
                   pISL->Release();
        //           TextMessage("Symbolic Link : '%s' resolved to '%s'", linkIn, linkOut);
                   itfc->retVar[1].MakeAndSetString(linkOut);
                   itfc->nrRetValues = 1;
                   return(OK);
                }
                else 
                {
                   ErrorMessage("Symbolic link '%s': Could not be resolved", linkIn);
                   return(ERR);
                }
            }
            else  
            {
               ErrorMessage("IPerSistFile Could not load %s", linkIn);
               return(ERR);
            }
        }
       else  
       {
          ErrorMessage("Query Interface failed");
          return(OK);
       }
    }
 
    ErrorMessage("CLSID object creation failed");
    return(ERR);
 
}


/***************************************************************
   Return, as a list, the subdirectories present in the 
	specified directory
****************************************************************/

int GetDirectoryList(Interface* itfc ,char args[])
{
	short nrArgs;
	CText dirName;
	CText saveDir;
   CText incDots;
   bool ignoreDot = true,ignoreDotDot= true;
   WIN32_FIND_DATA findData;
   HANDLE h;
	char **dirList;
	char **timeList;
   CText time;

// Save the current directory
   GetDirectory(saveDir);

// Default to not showing dot folders
   incDots = "";

// Get the directory to scan
   if((nrArgs = ArgScan(itfc,args,1,"directory name, [dot folders to include]","ee","tt",&dirName,&incDots)) < 0)
      return(nrArgs);

// Check to see if we want to include the '.' and '..' folders
   if(incDots == "..")
      ignoreDotDot = false;
   else if(incDots == ".")
      ignoreDot = false;
   else if(incDots == ".&.." || incDots == ".+..")
   {
      ignoreDotDot = false;
      ignoreDot = false;
   }
   else
   {
      ignoreDotDot = true;
      ignoreDot = true;
   }

// Move into this directory
	if(!SetDirectory(dirName))
	{
		ErrorMessage("invalid directory '%s' in search path", dirName);
	   return(ERR);
	}

// Find the first file in this directory
   h = FindFirstFile("*",&findData);
   
   if(h == INVALID_HANDLE_VALUE)
   {
      itfc->nrRetValues = 1;
		itfc->retVar[1].MakeAndSetList(NULL,0);
      return(OK);
   }
	dirList = NULL;
	timeList = NULL;
// Look at each file in turn and if it is a directory then add to dirlist
   long cnt = 0;
   do
   {
	   if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	   {
       // Ignore dot directory?
         if(ignoreDot && !strcmp(findData.cFileName,"."))
				continue;
       // Ignore dot dot directory?
         if(ignoreDotDot && !strcmp(findData.cFileName,".."))
            continue;
      // Ignore hidden folders
         if(findData.cFileName[0] == '.' && strlen(findData.cFileName) > 1 && findData.cFileName[1] != '.')
            continue;
       // Read the file time
         time.Format("%ld% ld",findData.ftLastWriteTime.dwHighDateTime,
                               findData.ftLastWriteTime.dwLowDateTime);

        // Add directory and time list

			AppendStringToList(findData.cFileName, &dirList, cnt);
			AppendStringToList(time.Str(), &timeList, cnt);
	      cnt++;
	   }
   }
   while(FindNextFile(h,&findData));
   FindClose(h);

// Return the list to the user (single blank element if empty)
   if(cnt > 0)
   {
      itfc->retVar[1].MakeAndSetList(dirList,cnt);
      itfc->nrRetValues = 1;
	   FreeList(dirList,cnt);
	   FreeList(timeList,cnt);
   }
   else
   {
      itfc->nrRetValues = 1;
		itfc->retVar[1].MakeAndSetList(NULL,0);
   }

// Restore the original directory
	SetDirectory(saveDir);

	return(OK);  
}

/***************************************************************
   Return, as a list, the files present in the 
	specified directory
****************************************************************/

int GetFileList(Interface *itfc, char arg[])
{
	short nrArgs;
	char dirName[MAX_PATH];
	char saveDir[MAX_PATH];
   WIN32_FIND_DATA findData;
   HANDLE h;
	char **fileList;

// Save the current directory
   GetCurrentDirectory(MAX_PATH,saveDir);

// Get the directory to scan
   if((nrArgs = ArgScan(itfc,arg,1,"directory name","e","s",dirName)) < 0)
      return(nrArgs);

// Move into this directory
	if(!SetCurrentDirectory(dirName))
	{
      ErrorMessage("invalid directory '%s' in search path", dirName);
      return(ERR);
	}

// Find the first file in this directory
   h = FindFirstFile("*",&findData);
   
   if(h == INVALID_HANDLE_VALUE)
   {
      itfc->nrRetValues = 1;
		itfc->retVar[1].MakeAndSetList(NULL,0);
      return(OK);
   }
	fileList = NULL;

// Look at each file in turn and if it is not a directory then add to filelist
   long cnt = 0;
   do
   {
	   if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	   {
         if(findData.cFileName[0] != '.') // Ignore hidden files
         {
         // Add file to list
			   AppendStringToList(findData.cFileName,&fileList,cnt);
	         cnt++;
         }
	   }
   }
   while(FindNextFile(h,&findData));
   FindClose(h);

// Return the list to the user (single blank element if empty)
   if(cnt > 0)
   {
      itfc->retVar[1].AssignList(fileList,cnt);
   }
   else
   {
		itfc->retVar[1].MakeAndSetList(NULL,0);
   }
   itfc->nrRetValues = 1;

// Restore the original directory
	SetCurrentDirectory(saveDir);

	return(OK);  
}



/********************************************************************
* Return some information about a file
*
* Parameters are:
*
* file name .............. name of file or file-path
* information required ... either "age" or "length"
*
* Returns the age of the file in seconds (since modification)
* or the length of the file in bytes.
* If the file is not present it returns a very large number 1e29 
* (is there a better way???)
*
*********************************************************************/


int FileInfo(Interface* itfc ,char args[])
{
   short nrArgs;
   CText fileName;
   CText infoMode;
   CText extraInfo = "modified";
   HANDLE hFile;

// Get the directory to scan ******************
   if((nrArgs = ArgScan(itfc,args,2,"file name, information required","eee","ttt",&fileName,&infoMode,&extraInfo)) < 0)
      return(nrArgs);

// Open the file ******************************
   hFile = CreateFile(fileName.Str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);	
   if(hFile == INVALID_HANDLE_VALUE)
   {
      itfc->retVar[1].MakeAndSetFloat(1e29);
      itfc->nrRetValues = 1;
      return(OK);
   }

// Process based on required information ******
   if(infoMode == "age")
   {
      itfc->retVar[1].MakeAndSetFloat(GetFileAge(hFile));
      itfc->nrRetValues = 1;
   }
   else if(infoMode == "date")
   {
		char date[100];
		if(extraInfo == "created")
		   GetFileDate(hFile, date, 0);
	   else if(extraInfo == "modified")
			GetFileDate(hFile, date, 1);
		else
			GetFileDate(hFile, date, 2);
      itfc->retVar[1].MakeAndSetString(date);
      itfc->nrRetValues = 1;
   }
	else if(infoMode == "dateandtime")
   {
		char dateNTime[100];
		if(extraInfo == "created")
		   GetFileDateAndTime(hFile, dateNTime, 0);
	   else if(extraInfo == "modified")
			GetFileDateAndTime(hFile, dateNTime, 1);
		else
			GetFileDateAndTime(hFile, dateNTime, 2);
      itfc->retVar[1].MakeAndSetString(dateNTime);
      itfc->nrRetValues = 1;
   }
   else if(infoMode == "length")
   {
      itfc->retVar[1].MakeAndSetFloat(GetFileSize(hFile));
      itfc->nrRetValues = 1;
   }
	else if(infoMode == "readonly")
	{
      itfc->retVar[1].MakeAndSetFloat(IsReadOnly(fileName.Str()));
      itfc->nrRetValues = 1;
	}
   else
   {
      ErrorMessage("invalid info option '%s'",infoMode);
      CloseHandle(hFile);
      return(ERR);
   }
	
   CloseHandle(hFile);
   return(OK);
}


float GetFileAge(HANDLE hFile)
{
   FILETIME creationTime, lastAccessTime, lastWriteTime, ft_current_time;
   ULARGE_INTEGER  li_file_time,li_cur_time;
   float elapsedTime;
   SYSTEMTIME current_time;

   GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime);

   li_file_time.HighPart = lastWriteTime.dwHighDateTime;
   li_file_time.LowPart = lastWriteTime.dwLowDateTime;

   GetSystemTime(&current_time);
   SystemTimeToFileTime(&current_time,&ft_current_time);
   li_cur_time.HighPart = ft_current_time.dwHighDateTime;
   li_cur_time.LowPart = ft_current_time.dwLowDateTime;

   elapsedTime = (float)(li_cur_time.QuadPart*100e-9 - li_file_time.QuadPart*100e-9);
   
   return(elapsedTime);
}


void GetFileDate(HANDLE hFile, char* date, int mode)
{
	char months[][4]  = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

   FILETIME creationTime, lastAccessTime, lastWriteTime;
   SYSTEMTIME convertedTime;

   GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime);

	if(mode == 0)
		 FileTimeToSystemTime(&creationTime, &convertedTime);
	else if(mode == 1)
		FileTimeToSystemTime(&lastWriteTime, &convertedTime);
	else if(mode == 2)
	   FileTimeToSystemTime(&lastAccessTime, &convertedTime);

	sprintf(date, "%d-%s-%d",convertedTime.wDay, months[convertedTime.wMonth-1], convertedTime.wYear);

}


void GetFileDateAndTime(HANDLE hFile, char* dateNTime, int mode)
{
	char months[][4]  = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

   FILETIME creationTime, lastAccessTime, lastWriteTime,LocalFileTime;
   SYSTEMTIME convertedTime, utcTime;

   GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime);

// Convert the times into local time (the conversion to local file time
// includes the local summer time shift so the result might be wrong
// if the file was recored during summer time and it is now normal
// time, or viceversa)
	if(mode == 0)
	{
      FileTimeToSystemTime(&creationTime, &utcTime);
      SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &convertedTime);
	}
	else if(mode == 1)
	{
      FileTimeToSystemTime(&lastWriteTime, &utcTime);
      SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &convertedTime);
	}
	else if(mode == 2)
	{
      FileTimeToSystemTime(&lastAccessTime, &utcTime);
      SystemTimeToTzSpecificLocalTime(NULL, &utcTime, &convertedTime);
	}

	sprintf(dateNTime, "%d-%d-%d-%d-%d-%d",convertedTime.wYear, convertedTime.wMonth, convertedTime.wDay, convertedTime.wHour, convertedTime.wMinute,convertedTime.wSecond);

}
//
//BOOL GetLastWriteTime(HANDLE hFile, LPTSTR lpszString, DWORD dwSize)
//{
//    FILETIME ftCreate, ftAccess, ftWrite;
//    SYSTEMTIME stUTC, stLocal;
//    DWORD dwRet;
//
//    // Retrieve the file times for the file.
//    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
//        return FALSE;
//
//    // Convert the last-write time to local time.
//    FileTimeToSystemTime(&ftWrite, &stUTC);
//    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
//
//    // Build a string showing the date and time.
//    dwRet = StringCchPrintf(lpszString, dwSize, 
//        TEXT("%02d/%02d/%d  %02d:%02d"),
//        stLocal.wMonth, stLocal.wDay, stLocal.wYear,
//        stLocal.wHour, stLocal.wMinute);
//
//    if( S_OK == dwRet )
//        return TRUE;
//    else return FALSE;
//}

float GetFileSize(HANDLE hFile)
{
   return((float)(GetFileSize (hFile, NULL)));
}

/********************************************************************
* Return the parent directory for a file
*********************************************************************/

int GetParentDirectory(Interface *itfc, char args[])
{
   short nrArgs;
   char fullPath[MAX_PATH];
   char baseDir[MAX_PATH];
   long i,j,k;

// Get the directory to scan
   if((nrArgs = ArgScan(itfc,args,1,"full path","e","s",fullPath)) < 0)
      return(nrArgs);

// Scan the full-path string for the base name
   long len = strlen(fullPath);
   for(i = len-1; i >= 0; i--)
   {
      if(fullPath[i] == '\\') 
         break;
   }

// Now scan for the parent directory
   for(j = i-1; j >= 0; j--)
      if(fullPath[j] == '\\') break;

// Copy this base name into baseDir
   for(k = j+1; k < i; k++)
      baseDir[k-j-1] = fullPath[k];
   baseDir[k-j-1] = '\0';

// Check for volume
   if(k-j-2 >= 0 && baseDir[k-j-2] == ':')
   {
      baseDir[k-j-1] = '\\';
      baseDir[k-j] = '\0';
   }

// Return to user
   itfc->retVar[1].MakeAndSetString(baseDir);
   itfc->nrRetValues = 1;

   return(OK);
}

/********************************************************************
* Return the base directory for a file
*********************************************************************/

int GetBaseDirectory(Interface *itfc, char args[])
{
   short nrArgs;
   char fullPath[MAX_PATH];
   char baseDir[MAX_PATH];
   long i,j;

// Get the directory to scan
   if((nrArgs = ArgScan(itfc,args,1,"full path","e","s",fullPath)) < 0)
      return(nrArgs);

// Scan the full-path string for the base name
   long len = strlen(fullPath);
   for(i = len-1; i > 0; i--)
      if(fullPath[i] == '\\') break;

// In case no backslash found
	if(i == 0)
		i--;

// Copy this base name into baseDir
   for(j = i+1; j < len; j++)
      baseDir[j-i-1] = fullPath[j];
   baseDir[j-i-1] = '\0';

// Return to user
   itfc->retVar[1].MakeAndSetString(baseDir);
   itfc->nrRetValues = 1;

   return(OK);
}

/********************************************************************
* Check if the argument is a valid file or (single) folder name
*********************************************************************/

int ValidFileName(Interface *itfc, char args[])
{
   short nrArgs;
   CText fileName;

// Get the directory to scan
   if((nrArgs = ArgScan(itfc,args,1,"filename","e","t",&fileName)) < 0)
      return(nrArgs);

	itfc->retVar[1].MakeAndSetFloat(IsValidFileName(fileName.Str()));
	itfc->nrRetValues = 1;

	return(OK);
}
		


int GetBasePath(Interface *itfc, char args[])
{
   short nrArgs;
   char fullPath[MAX_PATH];
   char basePath[MAX_PATH];
   long i,j;

// Get the directory to scan
   if((nrArgs = ArgScan(itfc,args,1,"full path","e","s",fullPath)) < 0)
      return(nrArgs);

   GetBasePathCore(fullPath,basePath);

// Return to user
   itfc->retVar[1].MakeAndSetString(basePath);
   itfc->nrRetValues = 1;

   return(OK);
}


/********************************************************************
* Fill folder with the current directory 
 (note: last character is not '\')
*********************************************************************/

void GetCurrentDirectory(CText &folder)
{
   long n = GetCurrentDirectory(0,NULL);
   char *buffer = new char[n];
   GetCurrentDirectory(n,buffer);
   if(buffer[n-1] == '\\')
      buffer[n-1] = '\0';
   folder = buffer;
	delete [] buffer;
}

/********************************************************************
* Set the current directory
*********************************************************************/

bool SetCurrentDirectory(CText &folder)
{
   return(SetCurrentDirectory(folder.Str()));
}

/********************************************************************
* Compare two directories - ignoring last character if it is a '\'
*********************************************************************/

bool CompareDirectories(char* folder1, char* folder2)
{
   long len1 = strlen(folder1);
   long len2 = strlen(folder2);

   if(folder1[len1-1] == '\\') len1--;
   if(folder2[len2-1] == '\\') len2--;

   if(len1 != len2)
      return(false);

   return(!strncmp(folder1,folder2,len1));
}

bool CompareDirectories(CText &folder1, CText &folder2)
{
   long len1 = strlen(folder1.Str());
   long len2 = strlen(folder2.Str());

   if(folder1.Str()[len1-1] == '\\') len1--;
   if(folder2.Str()[len2-1] == '\\') len2--;

   if(len1 != len2)
      return(false);

   return(!strncmp(folder1.Str(),folder2.Str(),len1));
}

// Returns true filename represents a file which exists in the current directory

bool IsFile(char *fileName)
{
    DWORD fileAttr;

    fileAttr = GetFileAttributes(fileName);
    if (fileAttr == INVALID_FILE_ATTRIBUTES)
        return(false);

    if(fileAttr & FILE_ATTRIBUTE_DIRECTORY)
       return(false);

    return(true);
}

// Returns true if the file is read-only

bool IsReadOnly(char *fileName)
{
    DWORD fileAttr;

    fileAttr = GetFileAttributes(fileName);
    if (fileAttr == INVALID_FILE_ATTRIBUTES)
        return(false);

    if(fileAttr & FILE_ATTRIBUTE_READONLY)
       return(true);

    return(false);
}

bool IsReadOnly(char* pathName, char *fileName)
{
   char oldPath[MAX_PATH];

	// Save the original path
	GetCurrentDirectory(MAX_PATH,oldPath);
	SetCurrentDirectory(pathName);

	DWORD fileAttr;

	fileAttr = GetFileAttributes(fileName);
	if (fileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if(fileAttr & FILE_ATTRIBUTE_READONLY)
		{
			SetCurrentDirectory(oldPath);
			return(true);
		}
	}

	SetCurrentDirectory(oldPath);
   return(false);
}


/*********************************************************************************
        Check to see if str is a valid pathname - does not check for blank
*********************************************************************************/

static char invalidFileChar[] = {'\\', '/', ':', '*', '?', '\"', '<',  '>', '|'};
bool IsValidFileName(char *str)
{
   int sz = strlen(str);
   int szIVF = strlen(invalidFileChar);


// No blanks at ends of name
   if(str[0] == ' ' || str[sz-1] == ' '|| str[sz-1] == '.')
      return(false);

// No illegal characters inside name
   for(int i = 0; i < sz; i++)
   {
      for(int j = 0; j < szIVF; j++)
      {
         if(str[i] == invalidFileChar[j])
            return(false);
      }
   }

   return(true);
}

/*********************************************************************************
        Check to see if str is a valid pathname - does not check for blank
*********************************************************************************/

static char invalidPathChar[] = {'*', '?', '\"', '<',  '>', '|'};
bool IsValidPathName(char *str)
{
   int sz = strlen(str);
   int sz2IVP = strlen(invalidPathChar);

// An exception for drive paths
   if(sz == 3 && str[1] == ':' && str[2] == '\\')
      return(true);

// No blanks at ends of name or '\' (Prospa requirement)
   if(str[0] == ' ' || str[sz-1] == ' ' || str[sz-1] == '\\' )
      return(false);

// No illegal characters inside name
   for(int i = 0; i < sz; i++)
   {
      for(int j = 0; j < sz2IVP; j++)
      {
         if(str[i] == invalidPathChar[j])
            return(false);
      }
   }

   return(true);
}

/*********************************************************************************
           Add extension 'ext' to fileName if one doesn't exist already
*********************************************************************************/
  
short AddExtension(char *fileName,char *ext)
{
   long i,len;
   char *curExt = NULL;

// Find current extension **************************   
   len = strlen(fileName);
   for(i = len-1; i >= 0; i--)
   {
      if(fileName[i] == '.')
      {
         curExt = &fileName[i];
         break;
      }
   }
   
// If the extension exists - check its a valid one *****   
   if(curExt) 
   {
      if(strcmp(ext,curExt) != 0)
         return(ERR); // No

      return(OK); // Yes
   }
   
 // No extension so add one ****************************
   strcat(fileName,ext);
   return(OK);
}

void ExtractFileNames(char *fullName, char *path, char *file)
{
	short len = strlen(fullName);
	short i,j;

	for(i = len-1; i >= 0; i--)
	{
		if(fullName[i] == '\\')
			break;
	}


	for(j = 0; j <= i; j++)
	{
		path[j] = fullName[j];
	}
	path[j] = '\0';


	for(j = i+1; j < len; j++)
	{
		file[j-i-1] = fullName[j];
	}
	file[j-i-1] = '\0';
}


int RemoveFolder(Interface *itfc, char args[])
{
   CText folder;
   short nrArgs;
   extern HWND prospaWin;

// Get the directory to delete
   if((nrArgs = ArgScan(itfc,args,1,"full path of directory to remove","e","t",&folder)) < 0)
      return(nrArgs);

  int len = folder.Size() + 2; // required to set 2 nulls at end of argument to SHFileOperation.
  char* tempdir = (char*) malloc(len);
  memset(tempdir,0,len);
  strcpy(tempdir,folder.Str());

  SHFILEOPSTRUCT file_op = {prospaWin,
                           FO_DELETE,
                           tempdir,
                           "",
                           FOF_ALLOWUNDO, /*FOF_NOCONFIRMATION |
                           FOF_NOERRORUI |
                           FOF_SILENT, */
                           false,
                           0,
                           "" };
  int ret = SHFileOperation(&file_op);
  free(tempdir);

  itfc->retVar[1].MakeAndSetFloat(ret > 0);
  itfc->nrRetValues = 1;

  return (OK); 
}

/********************************************************************************
   Remove filename extension core
********************************************************************************/

void RemoveExtension(char *file)
{
   short i,len;
   
   len = strlen(file);

   if(len > 0)
   {
      for(i = len-1; i >= 0; i--)
      {
         if(file[i] == '.') break;
      }
      if(i == -1)
         return;
      else
      {
         file[i] = '\0';
      }
   }
}


/********************************************************************************
   Return filename extension as a pointer to part of filename
********************************************************************************/

char* GetExtension(char *file)
{
   short i,j,len;
   static char extension[10];
   
   len = strlen(file);
   
   for(i = len; i >= 0; i--)
   {
      if(file[i] == '.') break;
   }
   if(i == -1)
   {
      extension[0] = '\0';
      return(extension);
   }
   else
   {
      for(j = i+1; j < len; j++)
      {
         extension[j-i-1] = file[j];
      }
      extension[j-i-1] = '\0';
      return(extension);
   }
}

/********************************************************************************
   Return filename extension core (does not include '.')
********************************************************************************/

void GetExtension(char *file, char *extension)
{
   short i,k,len;
   
   len = strlen(file);
   
   for(i = len-1; i >= 0; i--)
   {
      if(file[i] == '.') break;
   }
   if(i == -1)
   {
      extension[0] = '\0';
   }
   else
   {
      for(k = i+1; k < len; k++)
        extension[k-i-1] =  file[k];

      extension[k-i-1] = '\0';
   }
}

void GetExtension(CText &file, CText &extension)
{
   int pos = file.ReverseSearch('.');
   if(pos == -1)
      extension = "";
   else
      extension = file.End(pos+1);
}

// Return pointer to start of extension which is part of file
char* GetExtension(CText &file)
{
   char *extension;
   int pos = file.ReverseSearch('.') + 1;
   if(pos == 0)
      extension = file.Str() + file.Size();
   else
      extension = file.Str() + pos;
   return(extension);
}

// Check to see if dir is a valid directory
bool IsDirectory(char *dir)
{
    DWORD fileAttr = GetFileAttributes(dir);
    if(fileAttr == INVALID_FILE_ATTRIBUTES)
       return(false);

    if(fileAttr & FILE_ATTRIBUTE_DIRECTORY)
       return(true);

    return(false);
}


// Return the current directory in dir
void GetDirectory(CText &dir)
{
	long size = GetCurrentDirectory(0,NULL); // Modified V2.2.5
	dir.Reallocate(size);
	GetCurrentDirectory(size,dir.Str());
}

// Set the current directory
bool SetDirectory(CText &dir)
{
	return(SetCurrentDirectory(dir.Str()));
}

void GetBasePathCore(char *fullPath, char *basePath)
{
	int i,j;
	// Scan the full-path string for the base name
	long len = strlen(fullPath);
	for(i = len-1; i > 0; i--)
		if(fullPath[i] == '\\') break;

	// Copy path into basePath
	for(j = 0; j < i; j++)
		basePath[j] = fullPath[j];
	basePath[j] = '\0';
}


void GetFileNameFromPath(char *fullPath, char *name)
{
	int i,j;
	// Scan the full-path string for the base name
	long len = strlen(fullPath);
	for(i = len-1; i > 0; i--)
		if(fullPath[i] == '\\') break;

	// Copy this base name into baseDir
	for(j = i+1; j < len; j++)
		name[j-i-1] = fullPath[j];
	name[j-i-1] = '\0';
}

short GetNumberOfDroppedFiles(HDROP hDrop)
{
   int n = DragQueryFile(hDrop,-1,NULL,MAX_PATH-1);
   return(n);
}

// Extract the drag and drop file information and return it
short GetDropFileInfo(HDROP hDrop, CText &path, CText &file, CText &ext, int which) 
{
   char fullName[MAX_PATH];
   char fileName[MAX_PATH];
   char basePath[MAX_PATH];

   DragQueryFile(hDrop,which,fullName,MAX_PATH-1);
   GetBasePathCore(fullName,basePath);
   GetFileNameFromPath(fullName,fileName);
   file = fileName;
   GetExtension(file,ext);
   ext.LowerCase();

// If it is a directory
   if(IsDirectory(fullName))
   {
 //     ReplaceSpecialCharacters(fullName,"\\","\\\\",MAX_PATH);
      path = fullName;
      file = "";
      ext = "";
      return(OK);
   }
   
// Is it a file
   else if(IsFile(fullName))
   {
//      ReplaceSpecialCharacters(basePath,"\\","\\\\",MAX_PATH);
      path = basePath;
      return(OK);  
   }

   path = "";
   file = "";
   ext = "";

   DragFinish((HDROP)hDrop);

   return(ERR);
}

// CreateLink - Uses the Shell's IShellLink and IPersistFile interfaces 
//              to create and store a shortcut to the specified object. 


// https://stackoverflow.com/questions/21601809/create-a-shortcut-to-my-application-in-startup

int MakeLink(Interface *itfc, char args[])
{ 
   CText linkPath;
	CText execPath;
	CText execArgs;
	CText iconLocation;
	CText workingDir;
	LPCWSTR lpszDesc;
   HRESULT hres; 
   IShellLink* psl; 
	short nrArgs;

	if((nrArgs = ArgScan(itfc,args,5,"link, executable, arguments, icon, working_directory","eeeee","ttttt",&linkPath, &execPath, &execArgs, &iconLocation, &workingDir)) < 0)
      return(nrArgs); 

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
	 CoInitialize(0);
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
    if (SUCCEEDED(hres)) 
    { 
        IPersistFile* ppf; 

        // Set the path to the shortcut target and add the description. 
        psl->SetPath(execPath.Str()); 
		  psl->SetArguments(execArgs.Str());
		  psl->SetIconLocation(iconLocation.Str(),0);
		  psl->SetWorkingDirectory(workingDir.Str());

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 

        if (SUCCEEDED(hres)) 
        { 
            WCHAR wsz[MAX_PATH]; 

            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, linkPath.Str(), -1, wsz, MAX_PATH); 

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(wsz, TRUE); 
            ppf->Release(); 
        } 
        psl->Release(); 
		  CoUninitialize();

    } 
    return OK;
}

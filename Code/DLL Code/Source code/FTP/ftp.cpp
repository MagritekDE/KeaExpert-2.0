#include "stdafx.h"
#include "Wininet.h"
#include "../Global files/includesDLL.h"
#include "stdlib.h"

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
short FTPOpen(DLLParameters*,char *arg);
short FTPClose(DLLParameters*,char *arg);
short FTPDirectoryList(DLLParameters*,char *arg);
short FTPGetCWD(DLLParameters*,char *arg);
short FTPChangeDirectory(DLLParameters*,char *arg);
short FTPIsFile(DLLParameters*,char *arg);
short FTPFileList(DLLParameters*,char *arg);
short FTPGetFile(DLLParameters*,char *arg);
short FTPIsOpen(DLLParameters*,char *arg);
short FTPLoadFile(DLLParameters*,char *arg);
bool CloseInternetHandle(HINTERNET h);
long GetFileLength(FILE *fp);


/*******************************************************************************
    Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;


   if(!strcmp(command,"ropen"))           r = FTPOpen(dpar,parameters);      
   else if(!strcmp(command,"risopen"))    r = FTPIsOpen(dpar,parameters);      
   else if(!strcmp(command,"rclose"))     r = FTPClose(dpar,parameters);      
   else if(!strcmp(command,"rdirlist"))   r = FTPDirectoryList(dpar,parameters);      
   else if(!strcmp(command,"rfilelist"))  r = FTPFileList(dpar,parameters);      
   else if(!strcmp(command,"rpwd"))       r = FTPGetCWD(dpar,parameters);      
   else if(!strcmp(command,"rcd"))        r = FTPChangeDirectory(dpar,parameters);      
   else if(!strcmp(command,"risfile"))    r = FTPIsFile(dpar,parameters);      
   else if(!strcmp(command,"rgetfile"))   r = FTPGetFile(dpar,parameters);         
   else if(!strcmp(command,"rload"))      r = FTPLoadFile(dpar,parameters);      

   return(r);
}

/*******************************************************************************
    Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   FTP DLL module (V2.00)\n\n");
   TextMessage("   ropen ..... open an ftp connection\n");
   TextMessage("   risopen ... check to see if ftp connection is present\n");
   TextMessage("   rclose .... close an ftp connection\n");
   TextMessage("   rpwd ...... get remote working directory\n");
   TextMessage("   rcd ....... change remote working directory\n");
   TextMessage("   rdirlist .. list all remote directories\n"); 
   TextMessage("   rfilelist . list all remote files\n"); 
   TextMessage("   risfile ... checks to see if a remote file exists\n"); 
   TextMessage("   rgetfile .. get a remote a file exists\n"); 
   TextMessage("   rload  .... load a file into a variable\n"); 
}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"ropen"))          strcpy(syntax,"INT result = ropen(STR server, STR username, STR password)");
   else if(!strcmp(cmd,"risopen"))   strcpy(syntax,"INT result= risopen()");
   else if(!strcmp(cmd,"rclose"))    strcpy(syntax,"rclose()");
   else if(!strcmp(cmd,"rpwd"))      strcpy(syntax,"STR dir = rpwd()");
   else if(!strcmp(cmd,"rcd"))       strcpy(syntax,"INT result = rcd(STR new_directory)");
   else if(!strcmp(cmd,"rdirlist"))  strcpy(syntax,"LIST lst = rdirlist(STR root_folder)");
   else if(!strcmp(cmd,"rfilelist")) strcpy(syntax,"LIST lst = rfilelist(STR root_folder)");
   else if(!strcmp(cmd,"risfile"))   strcpy(syntax,"INT result = risfile(STR file_name)");
   else if(!strcmp(cmd,"rgetfile"))  strcpy(syntax,"INT result = rgetfile(STR \"b/a\",STR remote_file, STR local_file)");
   else if(!strcmp(cmd,"rload"))     strcpy(syntax,"STR txt = rload(STR file_name, [STR var_type])");


   if(syntax[0] == '\0')
      return(false);
   return(true);
}
HINTERNET hftpOpen,hftpConnect;

/*******************************************************************************
   This function attempts to open a connection to a remote server 
********************************************************************************/

short FTPOpen(DLLParameters* par, char *arg)
{
   short nrArgs;
   static char server[200] = "192.168.2.102";
   static char userName[200] = "craig";
   static char passWord[200] = "eccles";


// Get window number and list variable *******************************   
	if((nrArgs = ArgScan(par->itfc,arg,0,"server, username, password","eee","sss",server,userName,passWord)) < 0)
	   return(nrArgs);

// Check for open connection
   par->nrRetVar = 1;
   if(hftpOpen != NULL)
   {
      par->retVar[1].MakeAndSetFloat(0);
      return(OK);
   }
   SetCursor(LoadCursor(NULL,IDC_WAIT));
// Open connection
   hftpOpen = InternetOpen("prospa",INTERNET_OPEN_TYPE_DIRECT,NULL,NULL,0);

   if(hftpOpen == NULL)
   {
      SetCursor(LoadCursor(NULL,IDC_ARROW));
      par->retVar[1].MakeAndSetFloat(-1);
      return(OK);
   }

//   hftpConnect = InternetConnect(hftpOpen,server,INTERNET_DEFAULT_FTP_PORT,userName,passWord,INTERNET_SERVICE_FTP,NULL,NULL);
   hftpConnect = InternetConnect(hftpOpen,server,INTERNET_DEFAULT_FTP_PORT,userName,passWord,INTERNET_SERVICE_FTP,INTERNET_FLAG_PASSIVE,NULL);

   if(hftpConnect == NULL)
   {
      FTPClose(par,"");
      SetCursor(LoadCursor(NULL,IDC_ARROW));
      par->retVar[1].MakeAndSetFloat(-1);
      return(OK);
   }

   SetCursor(LoadCursor(NULL,IDC_ARROW));
   par->retVar[1].MakeAndSetFloat(1);
   return(OK);
}

/*******************************************************************************
   This function closes a connection to a remote server 
********************************************************************************/

short FTPClose(DLLParameters* par, char *arg)
{
   if(hftpConnect)
   {
      CloseInternetHandle(hftpConnect);
      hftpConnect = NULL;
   }

   if(hftpOpen)
   {
      CloseInternetHandle(hftpOpen);
      hftpOpen = NULL;
   }
   par->nrRetVar = 0;
   return(OK);
}


/*******************************************************************************
   Return a list of all the subdirectories on the remote machine 
********************************************************************************/

#define SIZE_1BYTEALIGN 318
#define SIZE_8BYTEALIGN 320

short FTPDirectoryList(DLLParameters* par, char *arg)
{
   short nrArgs;
   char rootDir[MAX_PATH];
   char fileName[MAX_PATH];
   HINTERNET       hFind;
   DWORD           dwError;
   WIN32_FIND_DATA dirInfo;
	char **dirList;

// This step is necessary since the compiler is set to 1-byte alignment
// but the wininet.lib seems to require 8 byte alignment so need to pad
// the local data structure by two bytes to keep thinks happy.
   LPWIN32_FIND_DATAA myDirInfo = (LPWIN32_FIND_DATAA)calloc(SIZE_8BYTEALIGN,1);
   memcpy((void*)myDirInfo,(void*)&dirInfo,SIZE_1BYTEALIGN);

// Get window number and list variable *******************************   
   if((nrArgs = ArgScan(par->itfc,arg,0,"root directory","e","s",rootDir)) < 0)
	   return(nrArgs);

   if(hftpConnect == NULL)
   {
      ErrorMessage("FTP connection not open");
      return(ERR);
   }

// Find the first file/directory on the FTP server
   for(int i = 0; i < 10; i++)
   {
       hFind = FtpFindFirstFile(hftpConnect, TEXT("*"), myDirInfo, 0, 0 );
       if(hFind) break;
       Sleep(10);
   }

// Check for errors
   if(hFind == NULL)
   {
      dwError = GetLastError( );
      if( dwError == ERROR_NO_MORE_FILES )
      {
         ErrorMessage("No files found at FTP location specified");
         return(ERR);
      }
      else
      {
         ErrorMessage( "FTP error: %ld",dwError);
         return(ERR);
      }
   }

// List all the directories
//	dirList = MakeList(1);
	dirList = NULL;
   long cnt = 0;
   do
   {
      memcpy((void*)&dirInfo,(void*)myDirInfo,SIZE_1BYTEALIGN);
      if(dirInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         strcpy(fileName,dirInfo.cFileName);
       // Ignore dot directories?
         if(!strcmp(fileName,".") || !strcmp(fileName,".."))
				continue;
        // Add directory to list
			InsertStringIntoList(dirInfo.cFileName,&dirList,cnt,cnt);
	      cnt++;
      }
   } 
   while(InternetFindNextFile(hFind, (void*)myDirInfo));

// Tidy up
   free(myDirInfo);

   if((dwError = GetLastError()) == ERROR_NO_MORE_FILES)
   {
      CloseInternetHandle(hFind);

      if(cnt > 0)
      {
         par->retVar[1].MakeAndSetList(dirList,cnt);
         par->nrRetVar = 1;
         FreeList(dirList,cnt);
      }
      else
      {
         InsertStringIntoList("",&dirList,0,0);
         par->retVar[1].MakeAndSetList(dirList,1);
         par->nrRetVar = 1;
     //    FreeList(dirList,1);
      }
      return(OK);
  }
  FreeList(dirList,cnt);
  CloseInternetHandle(hFind);
  ErrorMessage( "FTP error: %d",dwError);
  return(ERR); 
}


short FTPFileList(DLLParameters* par, char *arg)
{
   short nrArgs;
   char rootDir[MAX_PATH];
   char fileName[MAX_PATH];
   HINTERNET       hFind;
   DWORD           dwError;
   WIN32_FIND_DATA dirInfo;
	char **dirList;

// This step is necessary since the compiler is set to 1-byte alignment
// but the wininet.lib seems to require 8 byte alignment so need to pad
// the local data structure by two bytes to keep thinks happy.
   LPWIN32_FIND_DATAA myDirInfo = (LPWIN32_FIND_DATAA)calloc(SIZE_8BYTEALIGN,1);
   memcpy((void*)myDirInfo,(void*)&dirInfo,SIZE_1BYTEALIGN);

// Get window number and list variable *******************************   
   if((nrArgs = ArgScan(par->itfc,arg,0,"root directory","e","s",rootDir)) < 0)
	   return(nrArgs);

   if(hftpConnect == NULL)
   {
      ErrorMessage("FTP connection not open");
      return(ERR);
   }

// Find the first file/directory on the FTP server
   for(int i = 0; i < 10; i++)
   {
       hFind = FtpFindFirstFile(hftpConnect, TEXT("*.*"), myDirInfo, 0, 0 );
       if(hFind) break;
       Sleep(10);
   }

// Check for errors
   if(hFind == NULL)
   {
      dwError = GetLastError( );
      if( dwError == ERROR_NO_MORE_FILES )
      {
         ErrorMessage("No files found at FTP location specified");
         return(ERR);
      }
      else
      {
         ErrorMessage( "FTP error: %ld",dwError);
         return(ERR);
      }
   }

// List all the directories
	dirList = NULL;
//	dirList = MakeList(1);
   long cnt = 0;
   do
   {
      memcpy((void*)&dirInfo,(void*)myDirInfo,SIZE_1BYTEALIGN);
      if(!(dirInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         strcpy(fileName,dirInfo.cFileName);
        // Add file to list
			InsertStringIntoList(dirInfo.cFileName,&dirList,cnt,cnt);
	      cnt++;
      }
   } 
   while(InternetFindNextFile(hFind, (void*)myDirInfo));

// Tidy up
   free(myDirInfo);

   if((dwError = GetLastError()) == ERROR_NO_MORE_FILES)
   {
      CloseInternetHandle(hFind);

      if(cnt > 0)
      {
         par->retVar[1].MakeAndSetList(dirList,cnt);
         par->nrRetVar = 1;
         FreeList(dirList,cnt);
      }
      else
      {
         InsertStringIntoList("",&dirList,0,0);
         par->retVar[1].MakeAndSetList(dirList,1);
         par->nrRetVar = 1;
  //       FreeList(dirList,1);
      }
      return(OK);
  }
  FreeList(dirList,cnt);
  CloseInternetHandle(hFind);
  ErrorMessage( "FTP error: %d",dwError);
  return(ERR); 
}

/*******************************************************************************
   Lists the current remote directory  
********************************************************************************/

short FTPGetCWD(DLLParameters* par, char *arg)
{
  char directory[MAX_PATH];
  DWORD dwError,len = MAX_PATH;

  par->nrRetVar = 1;
  if(!FtpGetCurrentDirectory(hftpConnect, directory, &len))
  {
     par->retVar[1].MakeAndSetString("FTP Error");
     return(OK);
  }  
  par->retVar[1].MakeAndSetString(directory);
  return(OK);
}
 
/*******************************************************************************
   Change the current remote directory  
********************************************************************************/

short FTPChangeDirectory(DLLParameters* par, char *arg)
{
  char directory[MAX_PATH];
  DWORD dwError;
  short nrArgs;

// Get window number and list variable *******************************   
   if((nrArgs = ArgScan(par->itfc,arg,1,"new directory","e","s",directory)) < 0)
	   return(nrArgs);

  par->nrRetVar = 1;
  if(!FtpSetCurrentDirectory(hftpConnect, directory))
  {
     par->retVar[1].MakeAndSetFloat(0.0);
     return(OK);
  }  
  par->retVar[1].MakeAndSetFloat(1.0);

  return(OK);
}
  
/*******************************************************************************
   Check to see if a file exists in the current directory 
********************************************************************************/

short FTPIsFile(DLLParameters* par, char *arg)
{
   char filename[MAX_PATH];
   char str[MAX_PATH];
   short nrArgs;

// Get window number and list variable *******************************   
   if((nrArgs = ArgScan(par->itfc,arg,1,"filename","e","s",filename)) < 0)
	   return(nrArgs);

// Try and copy this file to local temporary file
   sprintf(str,"\"b\",\"%s\",\"ftpTempFile\"",filename);
   FTPGetFile(par,str);
   DeleteFile("ftpTempFile");

  return(OK);
}
  
/*******************************************************************************
   Attempt to download a remote file 
********************************************************************************/

short FTPGetFile(DLLParameters* par, char *arg)
{
  char local[MAX_PATH];
  char remote[MAX_PATH];
  char mode[100] = "b";
  DWORD dwError;
  DWORD type;
  short nrArgs;
  bool success;

// Get window number and list variable *******************************   
   if((nrArgs = ArgScan(par->itfc,arg,3,"mode,remotefile,localDir","eee","sss",mode,remote,local)) < 0)
	   return(nrArgs);

  par->nrRetVar = 1;
  (mode[0] == 'b') ? (type = FTP_TRANSFER_TYPE_BINARY) : (type = FTP_TRANSFER_TYPE_ASCII);
  if(!(success = FtpGetFile(hftpConnect,remote,local,false,FILE_ATTRIBUTE_NORMAL,type, 0)))
  {
     par->retVar[1].MakeAndSetFloat(0.0);
     return(OK);
  }  
  par->retVar[1].MakeAndSetFloat(1.0);

  return(OK);
}


/*******************************************************************************
  Attempt to load a remote file and copy into a string (other types supported)
********************************************************************************/

short FTPLoadFile(DLLParameters* par, char *arg)
{
   char fileName[MAX_PATH];
   char str[MAX_PATH];
   char varType[100] = "string";
   char *txt;
   short nrArgs;
   FILE *fp;


// Get window number and list variable *******************************   
   if((nrArgs = ArgScan(par->itfc,arg,1,"filename, [type]","ee","ss",fileName,varType)) < 0)
      return(nrArgs);

   sprintf(str,"\"b\",\"%s\",\"ftpTempFile\"",fileName);
   FTPGetFile(par,str);

// Abort if file not found
   if(par->retVar[1].GetReal() == 0.0)
   {
      par->retVar[1].MakeAndSetString("file not found");
      return(OK);
   }

// Load a text file and store as a string *****************************
   if(!strcmp(varType,"string"))
   {
    // Open remote file ***********************
      if(!(fp = fopen("ftpTempFile","rb")))
      {
         DeleteFile("ftpTempFile");
         ErrorMessage("Can't open local temporary FTP file"); // Shouldn't happen
         return(ERR);
      }

    // Get the file length ********************
      DWORD len = GetFileLength(fp);
      txt = new char[len+1];

    // Read the file **************************
      if(fread(txt,1,len,fp) != len)
      {
         fclose(fp);
         ErrorMessage("Error reading temporary FTP file");
         return(OK);
      }
      fclose(fp);
      DeleteFile("ftpTempFile");

    // Return to user *************************
      txt[len] = '\0';
      par->retVar[1].MakeAndSetString(txt);
      delete [] txt;
   }
   else
   {
      ErrorMessage("invalid data type");
      return(ERR);
   }

   return(OK);
}

  
/*******************************************************************************
   Check if there is connection open 
********************************************************************************/

short FTPIsOpen(DLLParameters* par, char *arg)
{
  par->nrRetVar = 1;

  if(hftpConnect)
     par->retVar[1].MakeAndSetFloat(1.0);
  else
    par->retVar[1].MakeAndSetFloat(0.0);

  return(OK);
}

/*******************************************************************************
  Close the internet handle 
********************************************************************************/

bool CloseInternetHandle(HINTERNET h)
{
   return(InternetCloseHandle(h));
}

/*********************************************************************************
* Returns the size of the file (in bytes) as pointed to by fp.                   *
*********************************************************************************/

long GetFileLength(FILE *fp)
{
   int currentPos, fileLength;

   currentPos = ftell(fp);
   fseek(fp, 0, SEEK_END);
   fileLength = ftell(fp);
   fseek(fp, currentPos, SEEK_SET);
   return(fileLength);
}

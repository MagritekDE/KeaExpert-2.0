#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include "sqlite3.h"

#define VERSION 2.0

#define ABORT -5

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short sqlite(DLLParameters*,char *arg);
short HelpFolder(DLLParameters*,char *args);
CText output;


/*******************************************************************************
    Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"sqlite"))      r = sqlite(dpar,parameters);                         

   return(r);
}

/*******************************************************************************
    Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   SQLite Module (V1.00)\n\n");
   TextMessage("   sqlite ... interface to sqlite functions\n");
}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"sqlite"))  strcpy(syntax,"VAR result = sqlite(STR file, STR query)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short HelpFolder(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\Biot");
   par->nrRetVar = 1;
   return(OK);
}


// Interface for Sqlite datebase commands (read only)

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    CText txt;
    for(i=0; i<argc; i++)
    {
       txt.Format("%s=%s\n", azColName[i], argv[i] ? argv[i] : "NULL");
       output.Concat(txt.Str());
    }
    return 0;
}

/*******************************************************************************
  Interface to SQLite
*******************************************************************************/

short sqlite(DLLParameters* par,char *args)
{
   short nrArgs;
   CText dbFile;
   CText command;

   if((nrArgs = ArgScan(par->itfc,args,2,"database,command","ee","tt",&dbFile,&command)) < 0)
	return(nrArgs);

    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
  
    // Open or create and open a database file
    rc = sqlite3_open(dbFile.Str(), &db);
    if(rc != SQLITE_OK)
    {
      ErrorMessage("Can't open database: %s", dbFile.Str());
      sqlite3_close(db);
      return(ERR);
    }
    // Apply a query to this data base
    output = "";
    rc = sqlite3_exec(db, command.Str(), callback, 0, &zErrMsg);
    if(rc != SQLITE_OK )
    {
      ErrorMessage("SQL error: %s", zErrMsg);
      sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    // Return any result
    par->retVar[1].MakeAndSetString(output.Str());
    par->nrRetVar = 1;
    return(OK);
}

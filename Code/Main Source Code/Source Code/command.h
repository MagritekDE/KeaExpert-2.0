#ifndef COMMAND_H
#define COMMAND_H

class Command
{
	public :
	   Command();
	   ~Command();
		void		setCmdStr(char*);	
		char*		getCmdStr(void);					
		int		splitExpression(char**,char**);
		int		splitExpression2(void);
      short    extractCmdNameAndArg(void);
      void     freeMemory(void); 
      void     setLineNr(short line); // Set the line number for this command
      short    getLineNr(void); // Get the line number ofr this command

		void setType(short);
		short getType();
		void setCmdName(char* name);
		char* getCmdName();
		void setCmdLeft(char *left);
		char* getCmdLeft();
		void setCmdRight(char *right);
		char* getCmdRight();
		void setCmdArg(char *arg);
		char* getCmdArg();

   private :
      char*    cmdLeft;
      char*    cmdRight;  
      char*    cmdName;
      char*    cmdArg;
      short    type;
		char  *cmdStr;   // String containing command  
		int   lineNr;    // Line this command appears on (0 indexed).
};

#define NOT_FOUND 1
#define FOUND     2

#endif //define COMMAND_H
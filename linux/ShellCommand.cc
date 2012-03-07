﻿//Implementation of ShellCommand.hh
//Author: Irene
//
//      Copyright 2011 - 2012 Mank <Mank1@seznam.cz>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include <cstring>
#include "ShellCommand.hh"

ShellCommand::ShellCommand(char* input, int len, int shell)
{
	thirdPerson = false;
	resultsize = len;
	output = new char[resultsize];
	strcpy(output,"");
	errormessage = new char[strlen(input)+100];
	strcpy(errormessage,"");
	error = 0;
	char command[strlen(input)+11];//declaration for the final command that will be executed
	if (shell == 0)
	{
		char testscript[strlen(input)+28];
	    strcpy(testscript,"test -e extensions/Scripts/");
        strcat(testscript,input);
		//test if script exists
		if (system(testscript)!=0)
		{
			error = 1;
			strcpy(errormessage,"No file ");
			strcat(errormessage,input);
			strcat(errormessage," in extensions/Scripts directory.");
		}
		else
		{
			//test if script is an executable
			strcpy(testscript,"test -x extensions/Scripts/");
        		strcat(testscript,input);
			if (system(testscript)!=0)
			{
				error = 1;
				//if(BOOLSETTING(SCRIPT_EXECUTABLES))
				//{
					char com[strlen(command)+29];
					strcpy(com,"chmod +x extensions/Scripts/");
					strcat(com,input);
					if (system(com)==0)
					{
						strcpy(errormessage,"Executing ");
						strcat(errormessage,com);
						error = 0;
					}
					else
					{
						strcpy(errormessage,"Unable to execute ");
						strcat(errormessage,com);
					}
				/*}
				else
				{
					strcpy(errormessage,"File ");
					strcat(errormessage,input);
					strcat(errormessage," is not an executable. Please use chmod +x to set file permissions.");
				}*/
			}
		}
		if (error == 0)
		{
    			strcpy(command,"./extensions/Scripts/");
    			strcat(command,input);
		}
	}
	else
	{
		strcpy(command,input);
	}

	if (error == 0)
	{
	        FILE* f;
        	f=popen(command,"r");
        	fgets(output,resultsize,f);
        	pclose(f);
        	//remove trailing newline
			output[strlen(output)-1]='\0';

			if(strncmp(output,"/me",3) ==0)
			{
				thirdPerson = true;
				output = substr(output,4,strlen(output)+1);
			}
	}
}

ShellCommand::~ShellCommand()
{
	delete[] output;
	delete[] errormessage;
}

bool ShellCommand::Error()
{
	return error;
}

char* ShellCommand::Output()
{
	return output;
}

char* ShellCommand::ErrorMessage()
{
	return errormessage;
}

int ShellCommand::GetResultSize()
{
	return resultsize;
}

bool ShellCommand::isThirdPerson()
{
	return thirdPerson;
}

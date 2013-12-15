//Implementation of ShellCommand.hh
//Author: Irene
#include <cstring>
#include "ShellCommand.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;

ShellCommand::ShellCommand(char* input, int len, int shell)
{
	thirdPerson = false;
	resultsize=len;
	output = new char[resultsize];
	strcpy(output,"");
	errormessage = new char[strlen(input)+100];
	strcpy(errormessage,"");
	error = 0;
	char command[strlen(input)+11];//declaration for the final command that will be executed
	if (shell == 0)
	{
		char testscript[strlen(input)+28];
	        strcpy(testscript,("test -e "+WulforManager::get()->getPath()+"/extensions/Scripts/").c_str());
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
			strcpy(testscript,string("test -x "+WulforManager::get()->getPath()+"/extensions/Scripts/").c_str());
        		strcat(testscript,input);
			if (system(testscript)!=0)
			{
				error = 1;
				//if(BOOLSETTING(SCRIPT_EXECUTABLES))
				//{
					char com[strlen(command)+29];
					strcpy(com,string("chmod +x "+WulforManager::get()->getPath()+"/extensions/Scripts/").c_str());
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
    			strcpy(command,(WulforManager::get()->getPath()+"/extensions/Scripts/").c_str());
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
				output = WulforUtil::g_substr(output,4,strlen(output)+1);

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


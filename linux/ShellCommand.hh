//Class to take care of communication with the shell
//Author: Irene //@Modified by Mank

#ifndef SHELL_COMMAND_HH
#define SHELL_COMMAND_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "settingsmanager.hh"

class ShellCommand
{
	public:
		/* input = shell command or name a script in the Script directory
		   len (optional) = maximum resultsize, standard set to 256
		*/
		ShellCommand(std::string input, int len=256);
		~ShellCommand();
		char* Output(); //Returns output. If unfixable error has occurred, output = ""
		const char* ErrorMessage(); //Returns errormessage. If no error has occurred, errormessage = ""
		bool isThirdPerson();
	private:
		char* output;
		std::string errormessage;
		bool thirdPerson;
		int resultsize;
		std::string path;
};

#else
class ShellCommand;
#endif

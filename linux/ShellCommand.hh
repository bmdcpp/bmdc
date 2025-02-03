//Class to take care of communication with the shell
/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SHELL_COMMAND_HH
#define SHELL_COMMAND_HH

#include "../dcpp/stdinc.h"
#include "../dcpp/DCPlusPlus.h"
#include "settingsmanager.hh"

class ShellCommand
{
	public:
		/* input = shell command or name a script in the Script directory
		   len (optional) = maximum resultsize, standard set to 256
		*/
		ShellCommand(std::string input, int len=256);
		~ShellCommand();
		const char* Output(); //Returns output. If unfixable error has occurred, output = ""
		const char* ErrorMessage(); //Returns error message. If no error has occurred, errormessage = ""
		bool isThirdPerson();
	private:
		std::string output;
		std::string errormessage;
		bool thirdPerson;
		int resultsize;
		std::string path;
};

#else
class ShellCommand;
#endif

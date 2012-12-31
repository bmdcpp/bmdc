//Class to take care of communication with the shell
//Author: Irene
//
//      Copyright 2011 - 2013 Mank <Mank1 at seznam dot cz>
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

#ifndef BMDC_SHELL_COMMAND_HH
#define BMDC_SHELL_COMMAND_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "settingsmanager.hh"

class ShellCommand
{
	public:
		/* input = shell command or name a script in the Script directory
		   len (optional) = maximum resultsize, standard set to 256
		   shell (optional) = 0 for a script, 1 for a shellcommand */
		ShellCommand(char* input, int len=265, bool shell =  false);
		~ShellCommand();
		bool Error() const; //Returns true if an error has occurred
		char* Output(); //Returns output. If unfixable error has occurred, output = ""
		char* ErrorMessage(); //Returns errormessage. If no error has occurred, errormessage = ""
		int GetResultSize() const; //Returns size of the result
		bool isThirdPerson() const; //Returns if is 3rd person

	private:
		//char* output;
		std::string output;
		char* errormessage;
		bool error;
		int resultsize;
		bool thirdPerson;
};

#else
class ShellCommand;
#endif

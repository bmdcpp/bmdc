/*
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstring>
#include "../dcpp/stdinc.h"
#include "../dcpp/Util.h"
#include "ShellCommand.hh"
#include "wulformanager.hh"
#include "GuiUtil.hh"

using namespace std;

ShellCommand::ShellCommand(std::string input, int len):
output(dcpp::Util::emptyString),
errormessage(dcpp::Util::emptyString),thirdPerson(false), resultsize(len),
path(WulforManager::get()->getPath()+"/extensions/Scripts/"+input)
{
	if(!dcpp::Util::fileExists(path))
	{
		errormessage = output = _("File doesn't exist");
		return;
	}
	if(!g_file_test(path.c_str(),G_FILE_TEST_IS_EXECUTABLE))
	{
		errormessage = output = _("File doesn't executable");
		return;
	}
	
	char* temp = new char[resultsize+1];
	FILE *p = popen( (path).c_str(), "r");
	char* x = fgets(temp,resultsize,p);
	dcdebug("%s",x);
	pclose(p);
	temp[strlen(temp)-1]='\0';
	temp[resultsize]='\0';
		
	if(strncmp(temp,"/me",3) ==0)
	{
		thirdPerson = true;
		output = temp+4;
	} else {
		output = temp;
	}
	delete [] temp;
}

ShellCommand::~ShellCommand()
{

}

const char* ShellCommand::Output()
{
	return output.c_str();
}

const char* ShellCommand::ErrorMessage()
{
	return errormessage.c_str();
}

bool ShellCommand::isThirdPerson()
{
	return thirdPerson;
}

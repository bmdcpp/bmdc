//Implementation of ShellCommand.hh
//Author: Irene//Modified by Mank
// * Copyright © 2012-2017 BMDC++
#include <cstring>
#include "../dcpp/stdinc.h"
#include "../dcpp/Util.h"
#include "ShellCommand.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;

ShellCommand::ShellCommand(std::string input, int len):
output(dcpp::Util::emptyString),
errormessage(""),thirdPerson(false), resultsize(len),
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

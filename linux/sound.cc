/*
* Copyright © 2009-2012 freedcpp, http://code.google.com/p/freedcpp
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/

#ifdef _WIN32
	#include "../dcpp/w.h"
	#include <mmsystem.h>
#elif defined(_HAVECANBERRA)
	#include <canberra-gtk.h> 
#endif

#include "settingsmanager.hh"
#include "../dcpp/Text.h"
#include "sound.hh"

using namespace std;
using namespace dcpp;

Sound *Sound::pSound = NULL;

void Sound::start()
{
	dcassert(!pSound);
	pSound = new Sound();
}

void Sound::stop()
{
	dcassert(pSound);
	delete pSound;
	pSound = NULL;
}

Sound* Sound::get()
{
	dcassert(pSound);
	return pSound;
}

void Sound::sound_init() const
{
	#ifdef _HAVECANBERRA
	int res = ca_context_create(&context); 
	dcdebug("Sound::sound_init: connection %d...\n", res);
	#endif
}

void Sound::playSound(TypeSound sound)
{
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();

	switch (sound)
	{
		case DOWNLOAD_BEGINS:
 
			if (wsm->getInt("sound-download-begins-use"))
				playSound(wsm->getString("sound-download-begins"));
		break;

		case DOWNLOAD_FINISHED:

			if (wsm->getInt("sound-download-finished-use"))
				playSound(wsm->getString("sound-download-finished"));
		break;

		case DOWNLOAD_FINISHED_USER_LIST:

			if (wsm->getInt("sound-download-finished-ul-use"))
				playSound(wsm->getString("sound-download-finished-ul"));
		break;

		case UPLOAD_FINISHED:

			if (wsm->getInt("sound-upload-finished-use"))
				playSound(wsm->getString("sound-upload-finished"));
		break;

		case PRIVATE_MESSAGE:

			if (wsm->getInt("sound-private-message-use"))
				playSound(wsm->getString("sound-private-message"));
		break;

		case HUB_CONNECT:

			if (wsm->getInt("sound-hub-connect-use"))
				playSound(wsm->getString("sound-hub-connect"));
		break;

		case HUB_DISCONNECT:

			if (wsm->getInt("sound-hub-disconnect-use"))
				playSound(wsm->getString("sound-hub-disconnect"));
		break;

		case FAVORITE_USER_JOIN:

			if (wsm->getInt("sound-fuser-join-use"))
				playSound(wsm->getString("sound-fuser-join"));
		break;

		case FAVORITE_USER_QUIT:

			if (wsm->getInt("sound-fuser-quit-use"))
				playSound(wsm->getString("sound-fuser-quit"));
		break;

		default: break;
	}
}

void Sound::playSound(const string &target)
{
	//we on win call PlaySound not cannbera
	#ifdef _WIN32
		PlaySound(Text::toT(target).c_str(), NULL, SND_FILENAME | SND_ASYNC);
	#elif defined(_HAVECANBERRA)
		ca_context_play(context, 1,CA_PROP_MEDIA_FILENAME, target.c_str(), NULL);
	#else
		#ifdef _WIN32
		::PlaySound(Text::toT(target), NULL, SND_FILENAME | SND_ASYNC);
		#else
		FILE *pipe = popen((WulforSettingsManager::getInstance()->getString("sound-command") + " \"" +target+"\" &" ).c_str(), "w" );
		pclose(pipe);
		#endif
	#endif
	
}

void Sound::sound_finalize() const
{
	#ifdef _HAVECANBERRA
		ca_context_destroy(context);
	#endif
}

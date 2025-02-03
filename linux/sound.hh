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
#ifndef SOUND_HH
#define SOUND_HH

class Sound
{
	public:
		enum TypeSound
		{
			DOWNLOAD_BEGINS,
			DOWNLOAD_FINISHED,
			DOWNLOAD_FINISHED_USER_LIST,
			UPLOAD_FINISHED,
			PRIVATE_MESSAGE,
			HUB_CONNECT,
			HUB_DISCONNECT,
			FAVORITE_USER_JOIN,
			FAVORITE_USER_QUIT,
			NONE
		};

		static Sound* get();
		static void start();
		static void stop();

		Sound() { sound_init(); }
		~Sound() { sound_finalize(); }

		void playSound(TypeSound sound);
		void playSound(const std::string &target);

	private:
		static Sound *pSound;
	#ifdef _HAVECANBERRA
		ca_context *context;
	#endif

		void sound_init() const;
		void sound_finalize() const;
};

#else
class Sound;
#endif

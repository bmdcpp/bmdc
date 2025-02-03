/*
* Copyright © 2009-2018 Leliksan Floyd <leliksan@Quadrafon2>
* Copyright © 2011-2025 BMDC
* This file is part of BMDC.
* BMDC is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
*
* BMDC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Xfce-nameday-plugin. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef NOTIFY_HH
#define NOTIFY_HH
class Notify
{
	public:
		enum TypeNotify
		{
			DOWNLOAD_FINISHED,
			DOWNLOAD_FINISHED_USER_LIST,
			PRIVATE_MESSAGE,
			HUB_CONNECT,
			HUB_DISCONNECT,
			FAVORITE_USER_JOIN,
			FAVORITE_USER_QUIT,
			HIGHLITING,
			HUB_CHAT,
			NONE
		};
		static Notify* get();
		static void start();
		static void stop();

		Notify() : bAction(false) { init(); }
		~Notify() { finalize(); }

		void showNotify(const std::string head, const std::string body, TypeNotify notify);
		void showNotify(const std::string title, const std::string head, const std::string body,
			const std::string icon, const int iconSize, GNotificationPriority urgency);
	private:
		static Notify *notify;
		void init();
		void finalize() ;
        GApplication* application;//todo?
		bool bAction;
		//GUI callback functions
		static void onAction(GSimpleAction*, GVariant*, gpointer);
};

#else
class Notify;
#endif

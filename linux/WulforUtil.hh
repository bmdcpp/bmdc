/*
 * Copyright © 2004-2014 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#ifndef _BMDC_UTIL_H
#define _BMDC_UTIL_H

#include <gtk/gtk.h>
#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/CID.h>
#include <dcpp/User.h>
#include <dcpp/HintedUser.h>
#include <dcpp/typedefs.h>
#include <dcpp/Util.h>

#define C_EMPTY(x) ((x) == NULL || (x)[0] == '\0')

class WulforUtil
{
	public:
		static std::vector<int> splitString(const std::string &str, const std::string &delimiter);
		static std::string linuxSeparator(const std::string &ps);
		static std::string windowsSeparator(const std::string &ps);
		/** get nick */
		static std::string getNicks(const std::string &cid, const std::string& hintUrl);
		static std::string getNicks(const dcpp::CID& cid, const std::string& hintUrl);
		static std::string getNicks(const dcpp::UserPtr& user, const std::string& hintUrl);
		static std::string getNicks(const dcpp::HintedUser& user) {return getNicks(user.user->getCID(), user.hint);}
		/** get hub name */
		static std::string getHubNames(const std::string &cid, const std::string& hintUrl);
		static std::string getHubNames(const dcpp::CID& cid, const std::string& hintUrl);
		static std::string getHubNames(const dcpp::UserPtr& user, const std::string& hintUrl);
		static std::string getHubNames(const dcpp::HintedUser& user) {return getHubNames(user.user->getCID(), user.hint);}
		/** get hub address */
		static dcpp::StringList getHubAddress(const dcpp::CID& cid, const std::string& hintUrl);
		static dcpp::StringList getHubAddress(const dcpp::UserPtr& user, const std::string& hintUrl);
//NOTE: FreeDC++
		static std::string getTextFromMenu(GtkMenuItem *item);
		static std::vector<std::string>& getCharsets();
		static void openURI(const std::string &uri, std::string &_error = dcpp::Util::emptyString);
		static void openURItoApp(const std::string &cmd);
		#if !GTK_CHECK_VERSION(3,4,0)
		static std::string colorToString(const GdkColor *color); /* gdk < 2.12 */
		#else
		static std::string colorToString(const GdkRGBA *color);
		#endif
		static GdkPixbuf* scalePixbuf(const GdkPixbuf *pixbuf,
			const int width, const int height, GdkInterpType type = GDK_INTERP_BILINEAR);

 		// Magnet links
		static std::string makeMagnet(const std::string &name, const int64_t size, const std::string &tth);
		static bool splitMagnet(const std::string &magnet, std::string &name, int64_t &size, std::string &tth);
		static bool splitMagnet(const std::string &magnet, std::string &line);
		static bool isMagnet(const std::string &text);
		static bool isLink(const std::string &text);
		static bool isHubURL(const std::string &text);
		// Profile locking
		static bool profileIsLocked();
		static gboolean getNextIter_gui(GtkTreeModel *model, GtkTreeIter *iter, bool children = TRUE, bool parent = TRUE);
		static GtkTreeIter copyRow_gui(GtkListStore *store, GtkTreeIter *fromIter, int position = -1);
		static void copyValue_gui(GtkListStore* store, GtkTreeIter *fromIter, GtkTreeIter *toIter, int position);
		static GtkTreeIter copyRow_gui(GtkTreeStore *store, GtkTreeIter *fromIter, GtkTreeIter *parent = NULL, int position = -1);
		static void copyValue_gui(GtkTreeStore* store, GtkTreeIter *fromIter, GtkTreeIter *toIter, int position);
		static void registerIcons();
		//NOTE: BMDC++
		static void drop_combo(GtkWidget *widget, std::vector<std::pair<std::string,int> > CONTEUDO);//Used in Detections Settinsg only

		static GdkPixbuf *LoadCountryPixbuf(const std::string& country);
		static std::string getCountryCode(std::string _countryname);

		static GdkPixbuf *loadIconShare(std::string ext);
		static std::string formatReport(const dcpp::Identity& identity);
		static bool checkCommand(std::string& cmd, std::string& param, std::string& message, std::string& status, bool& thirdperson);

		static bool isHighlightingWorld( GtkTextBuffer *buffer, GtkTextTag* &tag, std::string word, bool &tTab, gpointer hub);

		static std::vector<std::pair<std::string,int> > getActions();
		static std::string StringToUpper(std::string myString);
		static gboolean HitIP(std::string &name, std::string& sIp);

		static gchar* g_substr(const gchar* string, gint start, gint  end)
		{	gsize len = (end - start + 1);
			gchar *output = (gchar *)g_malloc0 (len + 1);
			return g_utf8_strncpy (output, &string[start], len);
		}

		static const std::string ENCODING_LOCALE;
		static const std::string commands;

	private:
		static std::string generateLeech();
		static std::map<std::string,std::string> loadmimetypes();
		static std::string getStatsForMem();
		static bool Ipv4Hit(std::string &name, std::string &sIp);
	#if GTK_CHECK_VERSION(3,9,0)
	public:
		static GtkIconTheme *icon_theme;
	#else
	private:	
		static GtkIconFactory *iconFactory;
	#endif
		static std::vector<std::string> charsets;
		static const std::string magnetSignature;
		static std::map<std::string,std::string> m_mimetyp;
		static std::map<std::string,GdkPixbuf*> countryIcon;
		static const char* CountryNames[];
		static const char* CountryCodes[];
		static const char* msgs_dc[];
		static std::string formatSized(std::string& nonf);
};

#endif

/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2011-2017 BMDC++
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
#include <dcpp/SettingsManager.h>
#include <dcpp/Util.h>

#include <glib/gi18n.h>


#define C_EMPTY(x) ((x) == NULL || (x)[0] == '\0')
// Some Global typedefs
typedef std::unordered_map<std::string, GtkTreeIter> UnMapIter;

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
		static void drop_combo(GtkWidget *widget, std::map<std::string,int> CONTEUDO);//Used in Detections Settinsg only

		static GdkPixbuf* LoadCountryPixbuf(const std::string country);
		static std::string getCountryCode(const std::string _countryname);

		static GdkPixbuf* loadIconShare(const std::string ext);
		static std::string formatReport(const dcpp::Identity& identity);
		static bool checkCommand(std::string& cmd, std::string& param, std::string& message, std::string& status, bool& thirdperson);

		static bool isHighlightingWorld( GtkTextBuffer *buffer, GtkTextTag* &tag, std::string word, bool &tTab, gpointer hub);

		static std::map<std::string,int> getActions();

		static bool HitIP(const std::string name);

		static const std::string ENCODING_LOCALE;
		static const std::string commands;

		static void setTextDeufaults(GtkWidget* widget, std::string strcolor, std::string back_image_path = dcpp::Util::emptyString,bool pm = false,std::string hubUrl = dcpp::Util::emptyString,std::string where = dcpp::Util::emptyString);
		//Note : selected is red, because most themes get white or black
		static void setTextColor(std::string color,std::string where = dcpp::Util::emptyString);
	private:
		static std::string formatTimeDifference(uint64_t diff, size_t levels = 3);
		static std::string generateLeech();
		static std::string getStatsForMem();
		static std::string cpuinfo();
		static bool Ipv4Hit(std::string name);
		static std::string formatSized(std::string nonf);
	#if GTK_CHECK_VERSION(3,9,0)
	public:
		static GtkIconTheme *icon_theme;
	#else
	private:
		static GtkIconFactory *iconFactory;
	#endif
		static std::vector<std::string> charsets;
		static const std::string magnetSignature;
		static std::unordered_map<std::string,GdkPixbuf*> countryIcon;
		static const char* CountryNames[];
		static const char* CountryCodes[];
		static const char* msgs_dc[];
	public:
static gboolean	is_format_supported (const gchar *uri)
{
	GSList *pixbuf_formats = NULL;
	GSList *iter;
	int i;

	pixbuf_formats = gdk_pixbuf_get_formats ();

	for (iter = pixbuf_formats; iter; iter = iter->next) {
		gchar **extension_list;
		GdkPixbufFormat *format = (GdkPixbufFormat*)iter->data;
		
		if (gdk_pixbuf_format_is_disabled (format))
		            continue;

	        extension_list = gdk_pixbuf_format_get_extensions (format);

		for (i = 0; extension_list[i] != 0; i++) {
			if (g_str_has_suffix (uri, extension_list[i])) {
			    g_slist_free (pixbuf_formats);
				g_strfreev (extension_list);
				return TRUE;
			}
		}
		g_strfreev (extension_list);
	}

	g_slist_free (pixbuf_formats);
	return FALSE;
};

	
};

#endif

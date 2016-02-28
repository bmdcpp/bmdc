/*
 * Copyright © 2004-2012 Jens Oknelid, paskharen@gmail.com
 * Copyright © 2010-2016 BMDC
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

#include "WulforUtil.hh"
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <dcpp/ClientManager.h>
#include <dcpp/Util.h>
#include <dcpp/version.h>
#include <dcpp/ShareManager.h>
#include <dcpp/HashManager.h>
#include <dcpp/StringTokenizer.h>
#include <dcpp/RegEx.h>
#include <dcpp/HighlightManager.h>
#include <dcpp/RawManager.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "ShellCommand.hh"
#include "hub.hh"
#include "version.hh"
#include "freespace.h"

using namespace std;
using namespace dcpp;

const string WulforUtil::ENCODING_LOCALE = _("System default");
vector<string> WulforUtil::charsets;
unordered_map<std::string,GdkPixbuf*> WulforUtil::countryIcon;
const string WulforUtil::magnetSignature = "magnet:?xt=urn:tree:tiger:";
#if GTK_CHECK_VERSION(3,9,0)
	GtkIconTheme* WulforUtil::icon_theme = NULL;
#else
	GtkIconFactory* WulforUtil::iconFactory = NULL;
#endif
const string WulforUtil::commands =
string("\r\n/away\r\n\t") + _("Set away mode") +
+"\r\n/back\r\n\t" + _("Set normal mode") +
+"\r\n/bmdc [mc]\r\n\t" + _("Show version") +
+"\r\n/ratio [mc]\r\n\t" + _("Show ratio") +
+"\r\n/refresh\r\n\t" + _("Refresh share") +
+"\r\n/slots [n]\r\n\t" + _("Set Slots to n") +
+"\r\n/stats\r\n\t" + _("Show stats") +
+"\r\n/amar\r\n\t" + _("Media Spam") +
+"\r\n/auda,/w\r\n\t" + _("Media Spam") +
+"\r\n/kaff\r\n\t" + _("Media Spam") +
+"\r\n/rb\r\n\t" + _("Media Spam") +
+"\r\n/vlc\r\n\t" + _("Media Spam") +
+"\r\n/debf\r\n\t" + _("Media Spam") +
+"\r\n/df\r\n\t" + _("Show df of disk(s)") +
+"\r\n/uptime\r\n\t" + _("Show uptime") +
+"\r\n/rebuild\r\n\t" + _("Rebuild Share") +
+"\r\n/cleanmc\r\n\t" + _("Clean Mainchat") +
+"\r\n/leech [mc]\r\n\t" + _("Show Leech stats") +
+"\r\n/ws [set] [value]\r\n\t" + _("Set GUI settings") +
+"\r\n/dcpps [set] [value]\r\n\t" + _("Set dcpp settings") +
+"\r\n/alias list\r\n\t" + _("List Aliases") +
+"\r\n/alias purge ::A\r\n\t" + _("Remove Aliases A") +
+"\r\n/alias A::uname -a\r\n\t" + _("Add alias A with uname -a exec") +
+"\r\n/A\r\n\t"+ _("Execution of alias A") +
+"\r\n/g\r\n\t"+ _("Search on Google")+
+"\r\n/google\r\n\t"+ _("Search on Google")+
+"\r\n/imdb\r\n\t"+ _("Search on imdb")
;

const char* WulforUtil::CountryNames[] = {
"ANDORRA", "UNITED ARAB EMIRATES", "AFGHANISTAN", "ANTIGUA AND BARBUDA",
"ANGUILLA", "ALBANIA", "ARMENIA", "NETHERLANDS ANTILLES", "ANGOLA", "ANTARCTICA", "ARGENTINA", "AMERICAN SAMOA",
"AUSTRIA", "AUSTRALIA", "ARUBA", "ALAND", "AZERBAIJAN", "BOSNIA AND HERZEGOVINA", "BARBADOS", "BANGLADESH",
"BELGIUM", "BURKINA FASO", "BULGARIA", "BAHRAIN", "BURUNDI", "BENIN", "BERMUDA", "BRUNEI DARUSSALAM", "BOLIVIA",
"BRAZIL", "BAHAMAS", "BHUTAN", "BOUVET ISLAND", "BOTSWANA", "BELARUS", "BELIZE", "CANADA", "COCOS ISLANDS",
"THE DEMOCRATIC REPUBLIC OF THE CONGO", "CENTRAL AFRICAN REPUBLIC", "CONGO", "SWITZERLAND", "COTE D'IVOIRE", "COOK ISLANDS",
"CHILE", "CAMEROON", "CHINA", "COLOMBIA", "COSTA RICA", "SERBIA AND MONTENEGRO", "CUBA", "CAPE VERDE",
"CHRISTMAS ISLAND", "CYPRUS", "CZECH REPUBLIC", "GERMANY", "DJIBOUTI", "DENMARK", "DOMINICA", "DOMINICAN REPUBLIC",
"ALGERIA", "ECUADOR", "ESTONIA", "EGYPT", "WESTERN SAHARA", "ERITREA", "SPAIN", "ETHIOPIA", "EUROPEAN UNION", "FINLAND", "FIJI",
"FALKLAND ISLANDS", "MICRONESIA", "FAROE ISLANDS", "FRANCE", "GABON", "UNITED KINGDOM", "GRENADA", "GEORGIA",
"FRENCH GUIANA", "GUERNSEY", "GHANA", "GIBRALTAR", "GREENLAND", "GAMBIA", "GUINEA", "GUADELOUPE", "EQUATORIAL GUINEA",
"GREECE", "SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS", "GUATEMALA", "GUAM", "GUINEA-BISSAU", "GUYANA",
"HONG KONG", "HEARD ISLAND AND MCDONALD ISLANDS", "HONDURAS", "CROATIA", "HAITI", "HUNGARY",
"INDONESIA", "IRELAND", "ISRAEL",  "ISLE OF MAN", "INDIA", "BRITISH INDIAN OCEAN TERRITORY", "IRAQ", "IRAN", "ICELAND",
"ITALY","JERSEY", "JAMAICA", "JORDAN", "JAPAN", "KENYA", "KYRGYZSTAN", "CAMBODIA", "KIRIBATI", "COMOROS",
"SAINT KITTS AND NEVIS", "DEMOCRATIC PEOPLE'S REPUBLIC OF KOREA", "SOUTH KOREA", "KUWAIT", "CAYMAN ISLANDS",
"KAZAKHSTAN", "LAO PEOPLE'S DEMOCRATIC REPUBLIC", "LEBANON", "SAINT LUCIA", "LIECHTENSTEIN", "SRI LANKA",
"LIBERIA", "LESOTHO", "LITHUANIA", "LUXEMBOURG", "LATVIA", "LIBYAN ARAB JAMAHIRIYA", "MOROCCO", "MONACO",
"MOLDOVA", "MONTENEGRO", "MADAGASCAR", "MARSHALL ISLANDS", "MACEDONIA", "MALI", "MYANMAR", "MONGOLIA", "MACAO",
"NORTHERN MARIANA ISLANDS", "MARTINIQUE", "MAURITANIA", "MONTSERRAT", "MALTA", "MAURITIUS", "MALDIVES",
"MALAWI", "MEXICO", "MALAYSIA", "MOZAMBIQUE", "NAMIBIA", "NEW CALEDONIA", "NIGER", "NORFOLK ISLAND",
"NIGERIA", "NICARAGUA", "NETHERLANDS", "NORWAY", "NEPAL", "NAURU", "NIUE", "NEW ZEALAND", "OMAN", "PANAMA",
"PERU", "FRENCH POLYNESIA", "PAPUA NEW GUINEA", "PHILIPPINES", "PAKISTAN", "POLAND", "SAINT PIERRE AND MIQUELON",
"PITCAIRN", "PUERTO RICO", "PALESTINIAN TERRITORY", "PORTUGAL", "PALAU", "PARAGUAY", "QATAR", "REUNION",
"ROMANIA", "SERBIA", "RUSSIAN FEDERATION", "RWANDA", "SAUDI ARABIA", "SOLOMON ISLANDS", "SEYCHELLES", "SUDAN",
"SWEDEN", "SINGAPORE", "SAINT HELENA", "SLOVENIA", "SVALBARD AND JAN MAYEN", "SLOVAKIA", "SIERRA LEONE",
"SAN MARINO", "SENEGAL", "SOMALIA", "SURINAME", "SAO TOME AND PRINCIPE", "EL SALVADOR", "SYRIAN ARAB REPUBLIC",
"SWAZILAND", "TURKS AND CAICOS ISLANDS", "CHAD", "FRENCH SOUTHERN TERRITORIES", "TOGO", "THAILAND", "TAJIKISTAN",
"TOKELAU", "TIMOR-LESTE", "TURKMENISTAN", "TUNISIA", "TONGA", "TURKEY", "TRINIDAD AND TOBAGO", "TUVALU", "TAIWAN",
"TANZANIA", "UKRAINE", "UGANDA", "UNITED STATES MINOR OUTLYING ISLANDS", "UNITED STATES", "URUGUAY", "UZBEKISTAN",
"VATICAN", "SAINT VINCENT AND THE GRENADINES", "VENEZUELA", "BRITISH VIRGIN ISLANDS", "U.S. VIRGIN ISLANDS",
"VIET NAM", "VANUATU", "WALLIS AND FUTUNA", "SAMOA", "YEMEN", "MAYOTTE", "YUGOSLAVIA", "SOUTH AFRICA", "ZAMBIA",
"ZIMBABWE" };

const char* WulforUtil::CountryCodes[] = {
 "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AN", "AO", "AQ", "AR", "AS", "AT", "AU", "AW", "AX", "AZ", "BA", "BB",
 "BD", "BE", "BF", "BG", "BH", "BI", "BJ", "BM", "BN", "BO", "BR", "BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC",
 "CD", "CF", "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", "CR", "CS", "CU", "CV", "CX", "CY", "CZ", "DE", "DJ",
 "DK", "DM", "DO", "DZ", "EC", "EE", "EG", "EH", "ER", "ES", "ET", "EU", "FI", "FJ", "FK", "FM", "FO", "FR", "GA",
 "GB", "GD", "GE", "GF", "GG", "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT", "GU", "GW", "GY", "HK",
 "HM", "HN", "HR", "HT", "HU", "ID", "IE", "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT", "JE", "JM", "JO", "JP",
 "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW", "KY", "KZ", "LA", "LB", "LC", "LI", "LK", "LR", "LS", "LT",
 "LU", "LV", "LY", "MA", "MC", "MD", "ME", "MG", "MH", "MK", "ML", "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT",
 "MU", "MV", "MW", "MX", "MY", "MZ", "NA", "NC", "NE", "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM",
 "PA", "PE", "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT", "PW", "PY", "QA", "RE", "RO", "RS", "RU",
 "RW", "SA", "SB", "SC", "SD", "SE", "SG", "SH", "SI", "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "ST", "SV", "SY",
 "SZ", "TC", "TD", "TF", "TG", "TH", "TJ", "TK", "TL", "TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG",
 "UM", "US", "UY", "UZ", "VA", "VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YT", "YU", "ZA", "ZM", "ZW" };

#define LINE2 "-- http://launchpad.net/bmdc++ <BMDC++ " GUI_VERSION_STRING "." BMDC_REVISION_STRING ">"

const char* WulforUtil::msgs_dc[] = {
		"\r\n-- I'm a happy DC++ user. You could be happy too.\r\n" LINE2,
		"\r\n-- Neo-...what? Nope...never heard of it...\r\n" LINE2,
		"\r\n-- Evolution of species: Ape --> Man\r\n-- Evolution of science: \"The Earth is Flat\" --> \"The Earth is Round\"\r\n-- Evolution of DC: NMDC --> ADC\r\n" LINE2,
		"\r\n-- I share, therefore I am.\r\n" LINE2,
		"\r\n-- I came, I searched, I found...\r\n" LINE2,
		"\r\n-- I came, I shared, I sent...\r\n" LINE2,
		"\r\n-- I don't have to see any ads, do you?\r\n" LINE2,
		"\r\n-- My client supports passive-passive downloads, does yours?\r\n" LINE2,
		"\r\n-- My client automatically detects the connection, does yours?\r\n" LINE2,
		"\r\n-- These addies are pretty annoying, aren't they? Get revenge by sending them yourself!\r\n" LINE2,
		"\r\n-- My client supports grouping favorite hubs, does yours?\r\n" LINE2,
		"\r\n-- My client supports segmented downloading, does yours?\r\n" LINE2,
		"\r\n-- My client support pack of Emoticons per Fav Hubs does yours?\r\n" LINE2,//[BMDC++
		"\r\n-- My client support Flags in Chat does Yours ?\r\n" LINE2,
 };
#define MSGS 14

vector<int> WulforUtil::splitString(const string &str, const string &delimiter)
{
	string::size_type loc, len, pos = 0;
	vector<int> array;

	if (!str.empty() && !delimiter.empty())
	{
		while ((loc = str.find(delimiter, pos)) != string::npos)
		{
			len = loc - pos;
			array.push_back(Util::toInt(str.substr(pos, len)));
			pos = loc + delimiter.size();
		}
		len = str.size() - pos;
		array.push_back(Util::toInt(str.substr(pos, len)));
	}
	return array;
}

string WulforUtil::linuxSeparator(const string &ps)
{
	string str = ps;
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		if ((*it) == '\\')
			(*it) = '/';
	return str;
}

string WulforUtil::windowsSeparator(const string &ps)
{
	string str = ps;
	for (string::iterator it = str.begin(); it != str.end(); ++it)
		if ((*it) == '/')
			(*it) = '\\';
	return str;
}

string WulforUtil::getNicks(const string &cid, const string& hintUrl)
{
	return getNicks(CID(cid), hintUrl);
}

string WulforUtil::getNicks(const CID& cid, const string& hintUrl)
{
	return Util::toString(ClientManager::getInstance()->getNicks(cid, hintUrl));
}

string WulforUtil::getNicks(const UserPtr& user, const string& hintUrl)
{
	return getNicks(user->getCID(), hintUrl);
}

string WulforUtil::getHubNames(const string &cid, const string& hintUrl)
{
	return getHubNames(CID(cid), hintUrl);
}

string WulforUtil::getHubNames(const CID& cid, const string& hintUrl)
{
	StringList hubs = ClientManager::getInstance()->getHubNames(cid, hintUrl);
	if (hubs.empty())
		return _("Offline");
	else
		return Util::toString(hubs);
}

string WulforUtil::getHubNames(const UserPtr& user, const string& hintUrl)
{
	return getHubNames(user->getCID(), hintUrl);
}

StringList WulforUtil::getHubAddress(const CID& cid, const string& hintUrl)
{
	return ClientManager::getInstance()->getHubs(cid, hintUrl);
}

StringList WulforUtil::getHubAddress(const UserPtr& user, const string& hintUrl)
{
	return getHubAddress(user->getCID(), hintUrl);
}

string WulforUtil::getTextFromMenu(GtkMenuItem *item)
{
	string text;
	GtkWidget *child = gtk_bin_get_child(GTK_BIN(item));

	if (child && GTK_IS_LABEL(child))
		text = gtk_label_get_text(GTK_LABEL(child));

	return text;
}

vector<string>& WulforUtil::getCharsets()
{
	if (charsets.empty())
	{
		charsets.push_back(ENCODING_LOCALE);
		charsets.push_back(_("UTF-8 (Unicode)"));
		charsets.push_back(_("CP1252 (Western Europe)"));
		charsets.push_back(_("CP1250 (Central Europe)"));
		charsets.push_back(_("ISO-8859-2 (Central Europe)"));
		charsets.push_back(_("ISO-8859-7 (Greek)"));
		charsets.push_back(_("ISO-8859-8 (Hebrew)"));
		charsets.push_back(_("ISO-8859-9 (Turkish)"));
		charsets.push_back(_("ISO-2022-JP (Japanese)"));
		charsets.push_back(_("SJIS (Japanese)"));
		charsets.push_back(_("CP949 (Korean)"));
		charsets.push_back(_("KOI8-R (Cyrillic)"));
		charsets.push_back(_("CP1251 (Cyrillic)"));
		charsets.push_back(_("CP1256 (Arabic)"));
		charsets.push_back(_("CP1257 (Baltic)"));
		charsets.push_back(_("GB18030 (Chinese)"));
		charsets.push_back(_("TIS-620 (Thai)"));
	}
	return charsets;
}

void WulforUtil::openURI(const string &uri, string &_error)
{
	GError* error = NULL;

	gtk_show_uri(NULL,uri.c_str(),GDK_CURRENT_TIME,&error);
	if(error != NULL)
	{
		cerr << "Failed to open URI: " << error->message << endl;
		_error = error->message;
		g_error_free(error);
	}
}

void WulforUtil::openURItoApp(const string &cmd)
{
	GError* error = NULL;

	g_spawn_command_line_async((gchar *)Text::fromUtf8(cmd).c_str(), &error);

	if (error != NULL)
	{
		cerr << "Failed to open application: " << error->message << endl;
		g_error_free(error);
	}
}
#if !GTK_CHECK_VERSION(3,4,0)
string WulforUtil::colorToString(const GdkColor *color)
{
	gchar strcolor[14];

	g_snprintf(strcolor, sizeof(strcolor), "#%04X%04X%04X",
		color->red, color->green, color->blue);

	return strcolor;
}
#else
string WulforUtil::colorToString(const GdkRGBA *color)
{
	gchar* tmp = gdk_rgba_to_string(color);
	string s(tmp);
	g_free(tmp);
	return s;
}
#endif
GdkPixbuf* WulforUtil::scalePixbuf(const GdkPixbuf *pixbuf, const int width, const int height, GdkInterpType type)
{
	g_return_val_if_fail(pixbuf != NULL, NULL);
	g_return_val_if_fail(width > 0, NULL);
	g_return_val_if_fail(height > 0, NULL);

	GdkPixbuf *scale = NULL;

	int Width = gdk_pixbuf_get_width(pixbuf);
	int Height = gdk_pixbuf_get_height(pixbuf);

	double w, h, k;

	w = (double) width / Width;
	h = (double) height / Height;
	k = MIN (w, h);

	if (Width > width || Height > height)

		scale = gdk_pixbuf_scale_simple(pixbuf, (int)(Width * k), (int)(Height * k), type);
	else
		scale = gdk_pixbuf_scale_simple(pixbuf, (int)(Width * k * 0.85), (int)(Height * k * 0.85), type);
	return
		scale;
}

string WulforUtil::makeMagnet(const string &name, const int64_t size, const string &tth)
{
	if (name.empty() || tth.empty())
		return string();

	// other clients can return paths with different separators, so we should catch both cases
	string::size_type i = name.find_last_of("/\\");
	string path = (i != string::npos) ? name.substr(i + 1) : name;

	return magnetSignature + tth + "&xl=" + Util::toString(size) + "&dn=" + Util::encodeURI(path);
}

bool WulforUtil::splitMagnet(const string &magnet, string &name, int64_t &size, string &tth)
{
	name = _("Unknown");
	size = 0;
	tth = _("Unknown");

	if (!isMagnet(magnet) || magnet.size() <= magnetSignature.length())
		return FALSE;

	string::size_type nextpos = 0;

	for (string::size_type pos = magnetSignature.length(); pos < magnet.size(); pos = nextpos + 1)
	{
		nextpos = magnet.find('&', pos);
		if (nextpos == string::npos)
			nextpos = magnet.size();

		if (pos == magnetSignature.length())
			tth = magnet.substr(magnetSignature.length(), nextpos - magnetSignature.length());
		else if (magnet.compare(pos, 3, "xl=") == 0)
			size = Util::toInt64(magnet.substr(pos + 3, nextpos - pos - 3));
		else if (magnet.compare(pos, 3, "dn=") == 0)
			name = Util::encodeURI(magnet.substr(pos + 3, nextpos - pos - 3), TRUE);
	}

	return TRUE;
}

bool WulforUtil::splitMagnet(const string &magnet, string &line)
{
	string name;
	string tth;
	int64_t size;

	if (splitMagnet(magnet, name, size, tth))
		line = name + " (" + Util::formatBytes(size) + ")";
	else
		return FALSE;

	return TRUE;
}

bool WulforUtil::isMagnet(const string &text)
{
	return g_ascii_strncasecmp(text.c_str(), magnetSignature.c_str(), magnetSignature.length()) == 0;
}

bool WulforUtil::isLink(const string &text)
{
	return g_ascii_strncasecmp(text.c_str(), "http://", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "https://", 8) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "www.", 4) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "ftp://", 6) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "sftp://", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "irc://", 6) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "ircs://", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "im:", 3) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "mailto:", 7) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "news:", 5) == 0;
}

bool WulforUtil::isHubURL(const string &text)
{
	return g_ascii_strncasecmp(text.c_str(), "dchub://", 8) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "adc://", 6) == 0 ||
		g_ascii_strncasecmp(text.c_str(), "adcs://", 7) == 0;
}

bool WulforUtil::profileIsLocked()
{
	static bool profileIsLocked = false;

	if (profileIsLocked)
		return TRUE;

	// We can't use Util::getConfigPath() since the core has not been started yet.
	// Also, Util::getConfigPath() is utf8 and we need system encoding for g_open().
	char *home = getenv("HOME");
	string configPath = home ? string(home) + "/.bmdc++-s/" : "/tmp/";//@
	string profileLockingFile = configPath + "profile.lck";
	int flags = O_WRONLY | O_CREAT;
	int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	int fd = g_open(profileLockingFile.c_str(), flags, mode);
	if (fd != -1) // read error
	{
		struct flock lock;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		struct flock testlock = lock;

		if (fcntl(fd, F_GETLK, &testlock) != -1) // Locking not supported
		{
			if (fcntl(fd, F_SETLK, &lock) == -1)
				profileIsLocked = true;
		}
	}
	g_close(fd,NULL);
	return profileIsLocked;
}

gboolean WulforUtil::getNextIter_gui(GtkTreeModel *model, GtkTreeIter *iter, bool children /* = TRUE */, bool parent /* = TRUE */)
{
	gboolean valid = FALSE;
	GtkTreeIter old = *iter;

	if (children && gtk_tree_model_iter_has_child(model, iter))
	{
		valid = gtk_tree_model_iter_children(model, iter, &old);
	}
	else
	{
		valid = gtk_tree_model_iter_next(model, iter);
	}

	// Try to go up one level if next failed
	if (!valid && parent)
	{
		valid = gtk_tree_model_iter_parent(model, iter, &old);
		if (valid)
			valid = gtk_tree_model_iter_next(model, iter);
	}

	return valid;
}

GtkTreeIter WulforUtil::copyRow_gui(GtkListStore *store, GtkTreeIter *fromIter, int position /* = -1 */)
{
	GtkTreeIter toIter;
	gtk_list_store_insert(store, &toIter, position);
	GtkTreeModel *m = GTK_TREE_MODEL(store);
	int count = gtk_tree_model_get_n_columns(m);

	for (int col = 0; col < count; col++)
	{
		copyValue_gui(store, fromIter, &toIter, col);
	}

	return toIter;
}

void WulforUtil::copyValue_gui(GtkListStore *store, GtkTreeIter *fromIter, GtkTreeIter *toIter, int position)
{
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(GTK_TREE_MODEL(store), fromIter, position, &value);
	gtk_list_store_set_value(store, toIter, position, &value);
	g_value_unset(&value);
}

GtkTreeIter WulforUtil::copyRow_gui(GtkTreeStore *store, GtkTreeIter *fromIter, GtkTreeIter *parent /* = NULL */, int position /* = -1 */)
{
	GtkTreeIter toIter;
	gtk_tree_store_insert(store, &toIter, parent, position);
	GtkTreeModel *m = GTK_TREE_MODEL(store);
	int count = gtk_tree_model_get_n_columns(m);

	for (int col = 0; col < count; col++)
	{
		copyValue_gui(store, fromIter, &toIter, col);
	}

	return toIter;
}

void WulforUtil::copyValue_gui(GtkTreeStore *store, GtkTreeIter *fromIter, GtkTreeIter *toIter, int position)
{
	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(GTK_TREE_MODEL(store), fromIter, position, &value);
	gtk_tree_store_set_value(store, toIter, position, &value);
	g_value_unset(&value);
}

/*
 * Registers either the custom icons or the GTK+ icons as stock icons in
 * GtkIconFactory according to the user's preference. If the icons have
 * previously been loaded, they are removed and re-added.
 */
void WulforUtil::registerIcons()
{
	#if GTK_CHECK_VERSION(3,9,0)
	icon_theme = gtk_icon_theme_get_default ();
	gtk_icon_theme_prepend_search_path(icon_theme,_DATADIR PATH_SEPARATOR_STR GUI_LOCALE_PACKAGE "icons");
	#else
	// Holds a mapping of custom icon names -> stock icon names.
	// Not all icons have stock representations.
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	map<string, string> icons;
	icons["bmdc"] = "bmdc";
	icons["bmdc-dc++"] = wsm->getString("icon-dc++");
	icons["bmdc-dc++-fw"] = wsm->getString("icon-dc++-fw");
	icons["bmdc-dc++-fw-op"] = wsm->getString("icon-dc++-fw-op");
	icons["bmdc-dc++-op"] = wsm->getString("icon-dc++-op");
	icons["bmdc-normal"] = wsm->getString("icon-normal");
	icons["bmdc-normal-fw"] = wsm->getString("icon-normal-fw");
	icons["bmdc-normal-fw-op"] = wsm->getString("icon-normal-fw-op");
	icons["bmdc-normal-op"] = wsm->getString("icon-normal-op");
	icons["bmdc-smile"] = wsm->getString("icon-smile");
	icons["bmdc-download"] = wsm->getString("icon-download");
	icons["bmdc-favorite-hubs"] = wsm->getString("icon-favorite-hubs");
	icons["bmdc-favorite-hubs-on"] = wsm->getString("icon-favorite-hubs-on");
	icons["bmdc-favorite-users"] = wsm->getString("icon-favorite-users");
	icons["bmdc-favorite-users-on"] = wsm->getString("icon-favorite-users-on");
	icons["bmdc-finished-downloads"] = wsm->getString("icon-finished-downloads");
	icons["bmdc-finished-downloads-on"] = wsm->getString("icon-finished-downloads-on");
	icons["bmdc-finished-uploads"] = wsm->getString("icon-finished-uploads");
	icons["bmdc-finished-uploads-on"] = wsm->getString("icon-finished-uploads-on");
	icons["bmdc-hash"] = wsm->getString("icon-hash");
	icons["bmdc-preferences"] = wsm->getString("icon-preferences");
	icons["bmdc-public-hubs"] = wsm->getString("icon-public-hubs");
	icons["bmdc-public-hubs-on"] = wsm->getString("icon-public-hubs-on");
	icons["bmdc-queue"] = wsm->getString("icon-queue");
	icons["bmdc-queue-on"] = wsm->getString("icon-queue-on");
	icons["bmdc-search"] = wsm->getString("icon-search");
	icons["bmdc-search-adl"] = wsm->getString("icon-search-adl");
	icons["bmdc-search-adl-on"] = wsm->getString("icon-search-adl-on");
	icons["bmdc-search-spy"] = wsm->getString("icon-search-spy");
	icons["bmdc-search-spy-on"] = wsm->getString("icon-search-spy-on");
	icons["bmdc-upload"] = wsm->getString("icon-upload");
	icons["bmdc-quit"] = wsm->getString("icon-quit");
	icons["bmdc-connect"] = wsm->getString("icon-connect");
	icons["bmdc-file"] = wsm->getString("icon-file");
	icons["bmdc-directory"] = wsm->getString("icon-directory");
	icons["bmdc-pm-online"] = wsm->getString("icon-pm-online");
	icons["bmdc-pm-offline"] = wsm->getString("icon-pm-offline");
	icons["bmdc-hub-online"] = wsm->getString("icon-hub-online");
	icons["bmdc-hub-offline"] = wsm->getString("icon-hub-offline");
	/**/
	icons["bmdc-notepad"] = wsm->getString("icon-notepad");
	icons["bmdc-notepad-on"] = wsm->getString("icon-notepad-on");
	icons["bmdc-system"] = wsm->getString("icon-system");
	icons["bmdc-system-on"] = wsm->getString("icon-system-on");
	icons["bmdc-away"] = wsm->getString("icon-away");
	icons["bmdc-away-on"] = wsm->getString("icon-away-on");

	icons["bmdc-limiting"] = wsm->getString("icon-limiting");
	icons["bmdc-limiting-on"] = wsm->getString("icon-limiting-on");
	/**/
	icons["bmdc-pm-online"] = wsm->getString("icon-pm-online");
	icons["bmdc-pm-offline"] = wsm->getString("icon-pm-offline");
	icons["bmdc-hub-online"] = wsm->getString("icon-hub-online");
	icons["bmdc-hub-offline"] = wsm->getString("icon-hub-offline");
	/**/
	icons["bmdc-connect"] = wsm->getString("icon-connect");
	icons["bmdc-file"] = wsm->getString("icon-file");
	icons["bmdc-directory"] = wsm->getString("icon-directory");

	icons["bmdc-normal"] = wsm->getString("icon-normal");
    /* normal mode */
	icons["bmdc-op"] = wsm->getString("icon-op");
	icons["bmdc-modem"] = wsm->getString("icon-modem");
	icons["bmdc-wireless"] = wsm->getString("icon-wireless");
	icons["bmdc-dsl"] = wsm->getString("icon-dsl");
	icons["bmdc-lan"] = wsm->getString("icon-lan");
	icons["bmdc-netlimiter"] = wsm->getString("icon-netlimiter");
	/***/
	icons["bmdc-ten"] = wsm->getString("icon-ten");
	icons["bmdc-zeroone"] = wsm->getString("icon-zeroone");
	icons["bmdc-zerozeroone"] = wsm->getString("icon-zerozeroone");
	icons["bmdc-other"] = wsm->getString("icon-other");
	/* aways mode */
	icons["bmdc-op-away"] = wsm->getString("icon-op-away");
	icons["bmdc-modem-away"] = wsm->getString("icon-modem-away");
	icons["bmdc-wireless-away"] = wsm->getString("icon-wireless-away");
	icons["bmdc-dsl-away"] = wsm->getString("icon-dsl-away");
	icons["bmdc-lan-away"] = wsm->getString("icon-lan-away");
	icons["bmdc-netlimiter-away"] = wsm->getString("icon-netlimiter-away");
	/**/
	icons["bmdc-ten-away"] = wsm->getString("icon-ten-away");
	icons["bmdc-zeroone-away"] = wsm->getString("icon-zeroone-away");
	icons["bmdc-zerozeroone-away"] = wsm->getString("icon-zerozeroone-away");
	icons["bmdc-other-away"] = wsm->getString("icon-other-away");
	/* normal pasive mod */
	icons["bmdc-op-pasive"] = wsm->getString("icon-op-pasive");
	icons["bmdc-modem-pasive"] = wsm->getString("icon-modem-pasive");
	icons["bmdc-wireless-pasive"] = wsm->getString("icon-wireless-pasive");
	icons["bmdc-dsl-pasive"] = wsm->getString("icon-dsl-pasive");
	icons["bmdc-lan-pasive"] = wsm->getString("icon-lan-pasive");
	icons["bmdc-netlimiter-pasive"] = wsm->getString("icon-netlimiter-pasive");
	/**/
	icons["bmdc-ten-pasive"] = wsm->getString("icon-ten-pasive");
	icons["bmdc-zeroone-pasive"] = wsm->getString("icon-zeroone-pasive");
	icons["bmdc-zerozeroone-pasive"] = wsm->getString("icon-zerozeroone-pasive");
	icons["bmdc-other-pasive"] = wsm->getString("icon-other-pasive");
	/* aways pasive mode */
	icons["bmdc-op-away-pasive"] = wsm->getString("icon-op-away-pasive");
	icons["bmdc-modem-away-pasive"] = wsm->getString("icon-modem-away-pasive");
	icons["bmdc-wireless-away-pasive"] = wsm->getString("icon-wireless-away-pasive");
	icons["bmdc-dsl-away-pasive"] = wsm->getString("icon-dsl-away-pasive");
	icons["bmdc-lan-away-pasive"] = wsm->getString("icon-lan-away-pasive");
	icons["bmdc-netlimiter-away-pasive"] = wsm->getString("icon-netlimiter-away-pasive");
	/**/
	icons["bmdc-ten-away-pasive"] = wsm->getString("icon-ten-away-pasive");
	icons["bmdc-zeroone-away-pasive"] = wsm->getString("icon-zeroone-away-pasive");
	icons["bmdc-zerozeroone-away-pasive"] = wsm->getString("icon-zerozeroone-away-pasive");
	icons["bmdc-other-away-pasive"] = wsm->getString("icon-other-away-pasive");

	if (iconFactory)
	{
		gtk_icon_factory_remove_default(iconFactory);
		iconFactory = NULL;
	}

	iconFactory = gtk_icon_factory_new();

	for (map<string, string>::const_iterator i = icons.begin(); i != icons.end(); ++i)
	{
		GtkIconSource *iconSource = gtk_icon_source_new();
		GtkIconSet *iconSet = gtk_icon_set_new();
		gtk_icon_source_set_icon_name(iconSource, i->second.c_str());
		gtk_icon_set_add_source(iconSet, iconSource);
		gtk_icon_factory_add(iconFactory, i->first.c_str(), iconSet);
		gtk_icon_source_free(iconSource);
		gtk_icon_set_unref(iconSet);
	}

	gtk_icon_factory_add_default(iconFactory);
	g_object_unref(iconFactory);
	#endif
}

GdkPixbuf *WulforUtil::LoadCountryPixbuf(const string &country)
{
	if(country.empty())
	{
		GdkPixbuf* buf = NULL;
		#if GTK_CHECK_VERSION(3,9,0)
		GError* error = NULL;
		buf = gtk_icon_theme_load_icon(icon_theme,"gtk-dialog-question",GTK_ICON_SIZE_MENU,GTK_ICON_LOOKUP_USE_BUILTIN,&error);
		if(error != NULL || buf == NULL) {
			g_print("[BMDC:Country] Failed %s",error->message);
			g_error_free(error);
			return NULL;
		}	
		#else
		GtkWidget* iwid = gtk_invisible_new ();
		buf = gtk_widget_render_icon_pixbuf(iwid, BMDC_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_MENU);
		#endif
		return buf;
	}
	unordered_map<string,GdkPixbuf*>::const_iterator it = countryIcon.find(country);
	if( it  != countryIcon.end() )
			return it->second;
	GError *error = NULL;
	gchar *path = g_strdup_printf(_DATADIR PATH_SEPARATOR_STR "bmdc/country/%s.png",
		                              (gchar *)country.c_str());
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(path,15,15,&error);
	if (error != NULL || pixbuf == NULL) {
			g_warning("[BMDC::Country] Cannot open image: %s => %s", path, error->message);
			g_error_free(error);
			g_free(path);
			return NULL;
	}
	g_free(path);
	countryIcon.insert(make_pair(country,pixbuf));
	return pixbuf;
}

string WulforUtil::getCountryCode(string _countryname)
{
	if(_countryname.empty())
		return Util::emptyString;
	std::transform(_countryname.begin(), _countryname.end(), _countryname.begin(), (int(*)(int))toupper);

	for(uint8_t q = 0; q < (sizeof(CountryNames) / sizeof(CountryNames[0])); ++q)
	{
		if(_countryname == CountryNames[q])
				return CountryCodes[q];

	}
	return Util::emptyString;
}

string WulforUtil::formatReport(const Identity& identity)
{
	map<string, string> reportMap = identity.getReport();

	string report = _("*** Info on " )+ identity.getNick() + " ***" + "\n";

	for(map<string,string>::const_iterator i = reportMap.begin(); i != reportMap.end(); ++i)
	{
		report += "\n" + i->first + ": " +  i->second;
	}

	return report + "\n";
}
///From CrzDC++
string WulforUtil::generateLeech() {

	char buf[650];
	snprintf(buf, sizeof(buf), "\n\t [ BMDC++ %s %s Leech Stats ]\r\n [ Downloaded:\t\t\t %s ]\r\n [ Uploaded:\t\t\t %s ]\r\n [ Total Download:\t\t %s ]\r\n [ Total Upload:\t\t\t %s ]\r\n [ Ratio: \t\t\t\t %s ]\r\n [ Current Uploads:\t\t %s Running Upload(s) ]\r\n [ Current Upload Speed: \t\t %s/s ]\r\n [ Current Downloads:\t\t %s Running Download(s) ]\r\n [ Current Download Speed: \t %s/s ]",
		VERSIONSTRING, GUI_VERSION_STRING, Util::formatBytes(Socket::getTotalDown()).c_str(), Util::formatBytes(Socket::getTotalUp()).c_str(),
		Util::formatBytes(static_cast<double>(SETTING(TOTAL_DOWNLOAD))).c_str(), Util::formatBytes(static_cast<double>(SETTING(TOTAL_UPLOAD))).c_str(),
		Util::toString(((static_cast<double>(SETTING(TOTAL_UPLOAD)))) / static_cast<double>(SETTING(TOTAL_DOWNLOAD))).c_str(),
		Util::toString(UploadManager::getInstance()->getUploadCount()).c_str(), Util::formatBytes(UploadManager::getInstance()->getRunningAverage()).c_str(),
		Util::toString(DownloadManager::getInstance()->getDownloadCount()).c_str(), Util::formatBytes(DownloadManager::getInstance()->getRunningAverage()).c_str());
	return buf;
}
//Return True if hadles by this func otherwise False
bool WulforUtil::checkCommand(string& cmd, string& param, string& message, string& status, bool& thirdperson)
{
	string::size_type separator = cmd.find_first_of(' ');

	if(separator != string::npos && cmd.size() > separator +1)
	{
		param = cmd.substr(separator + 1);
		cmd = cmd.substr(1, separator - 1);
	}
	else
	{
		if(separator != string::npos){
			cmd = cmd.substr(1,separator - 1);
		}else{
			cmd = cmd.substr(1);
			
		}
	}

	std::transform(cmd.begin(), cmd.end(), cmd.begin(), (int(*)(int))tolower);
	/* commnads */
	if(WGETB("show-commands"))
	{
	    status = _("Command send: ");
	    status += cmd;
	    status += '\0';
	    status += "\n";
	}

	if( cmd == "away" )
	{
		if (Util::getAway() && param.empty())
		{
				Util::setAway(FALSE);
				Util::setManualAway(FALSE);
				status += _("Away mode off");
				WulforManager::get()->getMainWindow()->setAwayIcon(false);
		}
		else
		{
				Util::setAway(TRUE);
				Util::setManualAway(TRUE);
				Util::setAwayMessage(param);
				ParamMap p;
				p["message"] = param;

				status += _("Away mode on: ") + Util::getAwayMessage(p);
				WulforManager::get()->getMainWindow()->setAwayIcon(true);
		}
		ClientManager::getInstance()->infoUpdated();
		return true;
	}
	else if ( cmd == "back" )
	{
		Util::setAway(FALSE);
		status += _("Away mode off");
		WulforManager::get()->getMainWindow()->setAwayIcon(false);
		ClientManager::getInstance()->infoUpdated();

		return true;
	} else if ( cmd == "bmdc" )
	{
		string msg = string(msgs_dc[GET_TICK() % MSGS]);

        if(param == "mc") {
            message += msg;
        } else
            status  += string(GUI_PACKAGE " " GUI_VERSION_STRING "." BMDC_REVISION_STRING "/" VERSIONSTRING "/" DCPP_REVISION_STRING ", ") + _("project home: ") + "http://launchpad.net/bmdc++";
		return true;
	} else if ( cmd == "ratio")
	{
			dcpp::ParamMap map;
			double ratio;
			double dw =  static_cast<double>(SETTING(TOTAL_DOWNLOAD));
			double up = static_cast<double>(SETTING(TOTAL_UPLOAD));
			if(dw > 0)
				ratio = up / dw;
			else
				ratio = 0;
				
			map["ratio"] = Util::toString(ratio);
			map["down"]	= Util::formatBytes(dw);
			map["up"] = Util::formatBytes(up);
			map["client"] = GUI_PACKAGE;
			
			string result = dcpp::Util::formatParams(SETTING(RATIO_TEMPLATE),map);

			if(param == "mc")
				message = result;
			else
				status += result;

			return true;
	}
	else if ( cmd == "refresh" )
	{
		try
		{
			ShareManager::getInstance()->setDirty();
			ShareManager::getInstance()->refresh(true);
		}
		catch (const ShareException& e)
		{
			status += e.getError();
		}
		return true;
	}
	else if ( cmd == "slots")
	{
		if (param == "0")
					status += _("Invalid number of slots");
		else
		{
			SettingsManager *sm = SettingsManager::getInstance();

			sm->set(SettingsManager::SLOTS, Util::toInt(param));
			sm->set(SettingsManager::SLOTS_PRIMARY, Util::toInt(param));
			status += _("Set Slots to:") + param;
		}
		return true;
	}
	else if (cmd == "stats")
	{
			int z = 0 ,y = 0;
			struct utsname u_name; //instance of utsname
			z = uname(&u_name);
			if (z == -1)
				dcdebug("Failed on uname");
			string rel(u_name.release);
			string mach(u_name.machine);
			struct sysinfo sys; //instance of acct;
			y = sysinfo(&sys);
			if(y != 0)
				dcdebug("Failed on sysinfo");

			const long minute = 60;
			const long hour = minute * 60;
			const long day = hour * 24;
			long upt = sys.uptime;
			long udays = upt/day;
			long uhour = (upt % day) / hour;
			long umin = (upt % hour) / minute;
			/**/
			int dettotal = SETTING(DETECTIONS);
			int detfail = SETTING(DETECTIONF);
			string build = "Built with ";
			#ifdef __clang__
				build += "clang " __clang_version__;
			#elif defined(__GNUC__)
				build += "gcc " __VERSION__;
				#ifdef _DEBUG
				build += " Debug";
				#else
				build += " Release";
				#endif
			#endif
			
		message =  "\n-= Stats " + dcpp::fullVersionString+" =-"
					+"\n-= " +build+" =-\n"
					+ "-= " + rel + " " + mach + " =-\n"
					+ "-= Uptime: " + formatTimeDifference(time(NULL) - Util::getUptime()) + " =-\n"
					+ "-= System Uptime: " + Util::toString(udays) + " days," + Util::toString(uhour) + " Hours," + Util::toString(umin) + " min. =-\n"
					+ "-= Detection (Failed/Successful): " + Util::toString(detfail) + " /" + Util::toString(dettotal) + " =-\n"
					+ "-=" + getStatsForMem() + " =-\n"
					+ "-=" + cpuinfo() + " =-\n";
		return true;
	}
	else if ( cmd == "g" || cmd == "google"){
	  if(param.empty())
		status += _("Specify a search string");
	   else
		openURI("http://www.google.com/search?q=" + param);
		return true;
	}else if ( cmd == "imdb") {
	  if(param.empty())
		status += _("Specify a search string");
	   else
		openURI("http://www.imdb.com/find?q=" + param);
		return true;
	/// "Now Playing" spam // added by curse and Irene
	}else if (cmd == "amar")
	{
		ShellCommand s("amarok-now-playing.sh");
		//test if script is in the right directory and set executable and if so run it
		if (strcmp(s.Output(),"Amarok is not running.")==0)
		{
			status += s.Output();
		}
		else if (strcmp(s.Output(),"Amarok is not playing.")==0)
		{
			status += s.Output();
		}
		else
		{
			message = s.Output();
			status += s.ErrorMessage();
			thirdperson = s.isThirdPerson();
		}
		return true;
	}
 	else if (cmd == "auda" || cmd == "w")
	{
		ShellCommand s("audacious-now-playing.sh");
		if (strcmp(s.Output(),"Audacious is not running.")==0)
		{
			status += s.Output();
		}
		else
		{
			message = s.Output();
			status += s.ErrorMessage();
			thirdperson = s.isThirdPerson();
		}
		return true;
	}
	else if (cmd == "kaff")
	{
		ShellCommand s("kaffeine-now-playing.sh");

		if (strcmp(s.Output(),"Kaffeine is not running.")==0)
		{
			status += s.Output();
		}
		else if (strcmp(s.Output(),"Kaffeine is not playing.")==0)
		{
			status += s.Output();
		}
		else
		{
			message = s.Output();
			status += s.ErrorMessage();
			thirdperson = s.isThirdPerson();
		}
		return true;
	}
	else if  (cmd == "rb")
	{
		ShellCommand s("rhytmobox-now-playing.sh");

		if (strcmp(s.Output(),"Kaffeine is not running.")==0)
		{
			status += s.Output();
		}
		else if (strcmp(s.Output(),"Kaffeine is not playing.")==0)
		{
			status += s.Output();
		}
		else
		{
			message = s.Output();
			status += s.ErrorMessage();
			thirdperson = s.isThirdPerson();
		}
		return true;
	}
	else if (cmd == "vlc")
	{
		ShellCommand s("vlc-np.sh");
		//test if script is in the right directory and set executable and if so run it
		if (strcmp(s.Output(),"VLC is not running.") == 0)
		{
			status += s.Output();
		}
		else if (strcmp(s.Output(),"VLC is not playing.") == 0)
		{
			status += s.Output();
		}
		else
		{
			message = s.Output();
			status += s.ErrorMessage();
			thirdperson = s.isThirdPerson();
		}
		return true;
	}
	else if ( cmd == "debf")
	{
			ShellCommand s("deadbeaf.sh");
			if (strcmp(s.Output(),"Deadbeef is not running.") == 0)
			{
				status += s.Output();
			}
			else if (strcmp(s.Output(),"Deadbeef is not playing.") == 0)
			{
				status += s.Output();
			}
			else
			{
				message = s.Output();
				status += s.ErrorMessage();
				thirdperson = s.isThirdPerson();
			}
		return true;
	}
	// End of "Now Playing"
	else if ( cmd == "df" )
	{
		string tmp = "\n\t\t\t-=Free Space=-\t\t\t\n" +  FreeSpace::process_mounts("/etc/mtab");
		tmp += "\n\t\t\tTotal:\t" + Util::formatBytes(FreeSpace::_aviable) + "/" + Util::formatBytes(FreeSpace::_total) + "\t\n";
		message += tmp;
		return true;
	}
	else if (cmd == "uptime")
	{
		message = "Uptime: " +  formatTimeDifference(time(NULL) - Util::getUptime());
		return true;
	}
	else if ( cmd == "rebuild" )
	{
		HashManager::getInstance()->rebuild();
		return true;
	}
	else if ( cmd == "cleanmc" )
	{
		message = "\n\t\t";
		message += "---------- Cleaning the chat ----------";
		message += "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
		message += "---------- The chat has been cleaned ----------";
		return true;
	}
	//From CrZDC
	else if ( cmd == "leech" )
	{
		if (param == "mc")
			message = generateLeech();
		else
			status += generateLeech();
		return true;
	}
	else if ( cmd == "ws")
	{
		status += WSCMD(param);
		return true;
	}
	else if ( cmd == "dcpps" )
	{
		status  += SettingsManager::getInstance()->parseCoreCmd(param);
		return true;
	}
	//aliases
	else if (cmd == "alias" && !param.empty())
 	{
 			StringTokenizer<string> sl(param, ' ');
 			if( sl.getTokens().size() >= 1 )
 			{
 				StringTokenizer<string> aliases( WGETS("custom-aliases"), '#' );

 				if( sl.getTokens().at(0) == "list" )
 				/// List aliasu
 				{
 					if( !aliases.getTokens().empty() )
 					{
 						status += string ( "Alias List:") ;

 						for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 						{
 							status += *i;
 						}
 					}
 					else
 					{
 						status += _("Aliases not found.");
  					}
					return true;
 				}
 				else if( sl.getTokens().at(0) == "purge" )
 					///odstraneni aliasu
 				{
 						string store(""), name("");
 						for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 						{
 							name = i->substr( 0, i->find_first_of( "::", 0 ) );
 							if( name.compare( sl.getTokens().at(1) ) != 0 )
 								store = store + *i + "#";
 						}
 						WSET( "custom-aliases", store );
						status += "Added alias: "+param;
						return true;
 				}
 				else
 				{
					///pridani aliasu
 					StringTokenizer<string> command( param, "::" );//'
 					string name("");
 					bool exists = false;
 					for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 					{
 						name = i->substr( 0, i->find_first_of( "::", 0 ) );
 						if( name.compare( param.substr( 0, param.find_first_of( "::", 0 ) ) ) == 0 )
 						{
 							exists = true;
 							status += string( "This alias already exists: " + *i );
 							break;
 						}
 					}
 					if( command.getTokens().size() == 2  && !exists )//3
 					{
 						aliases.getTokens().push_back( param );
 						string store("");
 						for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 						{
 							store = store + *i + "#";
 						}
 						WSET( "custom-aliases", store );
						status += "Added alias: "+param;
 					}
					return true;
 				}
 			}
 		}
 		else if ( !WGETS("custom-aliases").empty() )
 		{
 			StringTokenizer<string> aliases( WGETS("custom-aliases"), '#' );
 			string name("");
 			for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 			{
				g_print("%s",i->c_str());
				size_t s = i->find_first_of("::");				
				if( s  == string::npos)continue;
				if(i->empty())continue;
				name = i->substr( 0, s );
 				
				if( name.compare( cmd ) == 0 )
 				{
 					string exec = i->substr( i->find_first_of("::") + 2, i->size() );

 					if( !exec.empty() )
 					{
 						gchar *output = NULL;
 						GError *error = NULL;

 						g_spawn_command_line_sync( exec.c_str(), &output, NULL, NULL, &error);

 						if (error != NULL)
 						{
 							status += error->message;
 							g_error_free(error);
							return true;
 						}
 						else
 						{
 							string trash( output );
 							g_free( output );
							message += trash;
							return true;
 						}

					}
 					break;
 				}

 			}
			return false;
  		}
  		else
 		{
           	return false;
 		}

  return false;
}
//SDDCPP Originaly
string WulforUtil::formatTimeDifference(uint64_t diff, size_t levels /*= 3*/) {
	string	buf;
	int		n;

#define DCPP_FORMATTIME(calc, name) \
	if((n = (diff / (calc))) != 0) { \
		if(!buf.empty()) \
			buf += L' '; \
		buf += Util::toString(n); \
		buf += L' '; \
		buf += name; \
		if(n != 1) \
			buf += L's'; \
		levels--; \
		if(levels == 0) \
			return buf; \
		diff %= (calc); \
	}

	DCPP_FORMATTIME(60 * 60 * 24 * 7,	"week");
	DCPP_FORMATTIME(60 * 60 * 24,		"day");
	DCPP_FORMATTIME(60 * 60,			"hour");
	DCPP_FORMATTIME(60,				"minute");
	DCPP_FORMATTIME(1,				"second");
	return buf;
}	

bool WulforUtil::isHighlightingWorld( GtkTextBuffer *buffer, GtkTextTag* &tag, string word, bool &tTab, gpointer hub)
{
		string sMsgLower;
		sMsgLower.resize(word.size()+1);
		std::transform(word.begin(), word.end(), sMsgLower.begin(), _tolower);
		bool ret = false;

		ColorList* cList = HighlightManager::getInstance()->getList();
		for(auto i = cList->begin();i != cList->end(); ++i) {
			ColorSettings* cs = &(*i);
			bool tBold = false;
			bool tItalic = false;
			bool tUnderline = false;
			string fore("");
			string back("");

			if(cs->getBold())
				tBold = true;
			if(cs->getItalic())
				tItalic = true;
			if(cs->getUnderline())
				tUnderline = true;

			if(cs->getHasBgColor())
				back = cs->getBgColor();
			else
				back = WGETS("text-general-back-color");

			if(cs->getHasFgColor())
				fore = cs->getFgColor();
			else
				fore = WGETS("text-general-fore-color");

			if(cs->getTab())
				tTab = true;

			string w = cs->getMatch();
			string sW;
			sW.resize(w.size()+1);
			std::transform(w.begin(), w.end(), sW.begin(), _tolower);

			int ffound = sMsgLower.compare(sW);

			if(!ffound && cs->usingRegexp() == false) {
				
				if((Hub *)hub)
				{
					Hub *p = (Hub *)hub;
					if(((cs->getIncludeNick()) && p->findNick_gui_p(word)))
					{
						if(!tag) {

							tag = gtk_text_buffer_create_tag(buffer, word.c_str(),
							"foreground", fore.c_str(),
							"background", back.c_str(),
							"weight", tBold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
							"style", tItalic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
							"underline", tUnderline ? PANGO_UNDERLINE_DOUBLE : PANGO_UNDERLINE_NONE,
							NULL);
						}
						ret = true;
//						continue;
					}
				}
			}

			if((ret == false) && cs->usingRegexp())
			{
				string q = cs->getMatch().substr(4);
				//bool reMatch  = dcpp::RegEx::match<string>(sMsgLower,q,cs->getCaseSensitive());
				bool reMatch = g_pattern_match_simple (q.c_str(),word.c_str());
				ret = false;
				if(reMatch)
				{
					if(!tag) {
						tag = gtk_text_buffer_create_tag(buffer, word.c_str(),
						"foreground", fore.c_str(),
						"background", back.c_str(),
						"weight", tBold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
						"style", tItalic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
						"underline", tUnderline ? PANGO_UNDERLINE_DOUBLE : PANGO_UNDERLINE_NONE,
						NULL);
					}
					dcdebug("regexp hilg");
					ret = true;
//					continue;
				}
			}

			int found = sMsgLower.compare(sW);
			if( (ret == false) && (!found) ) {

				if(!tag)
				{
				 tag = gtk_text_buffer_create_tag(buffer, word.c_str(),
					"foreground", fore.c_str(),
					"background", back.c_str(),
					"weight", tBold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
					"style", tItalic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
					"underline", tUnderline ? PANGO_UNDERLINE_DOUBLE : PANGO_UNDERLINE_NONE,
					NULL);

				}
				ret = true;
//				continue;
			}	

			if(ret && cs->getPopup())
				WulforManager::get()->getMainWindow()->showNotification_gui(cs->getNoti()+" : ", word, Notify::HIGHLITING);

			if(ret && cs->getPlaySound())
				Sound::get()->playSound(cs->getSoundFile());
					
			if(!ret)
				continue;
		
	}
	return ret;
}

map<string,int> WulforUtil::getActions()
{
	map<string,int> actions;
	const dcpp::Action::ActionList& a = dcpp::RawManager::getInstance()->getActions();
		for(auto i = a.begin(); i != a.end(); ++i) {
				const std::string name = (*i)->getName();
				const int Id = (*i)->getId();
				if((*i)->getEnabled())
						actions.insert(make_pair(name,Id));
		}
	return actions;
}
// TODO: remove if not used...
void WulforUtil::drop_combo(GtkWidget *widget, map<std::string,int> m)
{
	gtk_cell_layout_clear(GTK_CELL_LAYOUT(widget));
	GtkListStore *list_store;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	list_store = gtk_list_store_new(1,G_TYPE_STRING);

	for (auto i=m.begin();i!=m.end();++i)
	{
		char conteude[130];
		sprintf(conteude,"%s",i->first.c_str());
		gtk_list_store_append(list_store,&iter);
		gtk_list_store_set(list_store,&iter,0,conteude,-1);
	}

	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(list_store));

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget),renderer, "text", 0, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 0);

}

GdkPixbuf *WulforUtil::loadIconShare(string ext)
{
	if(ext == "directory" || ext.empty())
	{
		#if GTK_CHECK_VERSION(3,9,0)
		GError* error = NULL;
		GdkPixbuf* buf = gtk_icon_theme_load_icon(icon_theme,"folder",GTK_ICON_SIZE_MENU,GTK_ICON_LOOKUP_USE_BUILTIN,&error);
		if(error){
			g_print("Failed %s",error->message);
			g_error_free(error);
			return NULL;
		}	
		return buf;
		#else
		GtkWidget* iwid = gtk_invisible_new ();
		GdkPixbuf* buf = gtk_widget_render_icon_pixbuf(iwid, BMDC_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
		return buf;
		#endif
	}
	std::transform(ext.begin(), ext.end(), ext.begin(), (int(*)(int))tolower);

	gboolean is_certain = FALSE;
	gchar *content_type = g_content_type_guess ( (gchar*)(string("dummy.")+ext).c_str(), NULL, 0, &is_certain);
	if (content_type == NULL)
	{
		
		#if GTK_CHECK_VERSION(3,9,0)
		GError* error = NULL;
		GdkPixbuf* buf = gtk_icon_theme_load_icon(icon_theme,"text-x-generic",GTK_ICON_SIZE_MENU,GTK_ICON_LOOKUP_USE_BUILTIN,&error);
		if(error){
			g_print("Failed %s",error->message);
			g_error_free(error);
			return NULL;
		}
		return buf;
		#else
		GtkWidget *iwid = gtk_invisible_new ();
		GdkPixbuf *buf = gtk_widget_render_icon_pixbuf(iwid, BMDC_STOCK_FILE, GTK_ICON_SIZE_MENU);
		return buf;
		#endif
	}
	char *mime_type = g_content_type_get_mime_type (content_type);
	GIcon *icon = g_content_type_get_icon(mime_type);
	GtkIconTheme *theme = gtk_icon_theme_get_default ();
	GtkIconInfo *info = gtk_icon_theme_lookup_by_gicon(theme,icon, (GtkIconSize)16, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
	GdkPixbuf *icon_d = gtk_icon_info_load_icon (info, NULL);
	g_object_unref(icon);
	g_free (mime_type);
	g_free (content_type);
	return icon_d;
}
//Main point of this code is from ? Px
string WulforUtil::getStatsForMem() {
	
	string temp = Util::emptyString;
	FILE *fp = fopen("/proc/self/status", "r");
				if(fp != NULL) {
					string memrss, memhwm, memvms, memvmp, memstk, memlib;
					char buf[1024];
					while(fgets(buf, 1024, fp) != NULL) {
						if(strncmp(buf, "VmRSS:", 6) == 0) {
							char * tmp = buf+6;
							while(isspace(*tmp) && *tmp) {
								tmp++;
							}
							memrss = string(tmp, strlen(tmp)-1);
						} else if(strncmp(buf, "VmHWM:", 6) == 0) {
							char * tmp = buf+6;
							while(isspace(*tmp) && *tmp) {
								tmp++;
							}
							memhwm = string(tmp, strlen(tmp)-1);
						} else if(strncmp(buf, "VmSize:", 7) == 0) {
							char * tmp = buf+7;
							while(isspace(*tmp) && *tmp) {
								tmp++;
							}
							memvms = string(tmp, strlen(tmp)-1);
						} else if(strncmp(buf, "VmPeak:", 7) == 0) {
							char * tmp = buf+7;
							while(isspace(*tmp) && *tmp) {
								tmp++;
							}
							memvmp = string(tmp, strlen(tmp)-1);
						} else if(strncmp(buf, "VmStk:", 6) == 0) {
							char * tmp = buf+6;
							while(isspace(*tmp) && *tmp) {
								tmp++;
							}
							memstk = string(tmp, strlen(tmp)-1);
						} else if(strncmp(buf, "VmLib:", 6) == 0) {
							char * tmp = buf+6;
							while(isspace(*tmp) && *tmp) {
								tmp++;
							}
							memlib = string(tmp, strlen(tmp)-1);
						}
					}

					fclose(fp);

					if(!memhwm.empty() && !memrss.empty()) {
						temp+=" Mem usage (Peak): "+formatSized(memrss)+ " ("+formatSized(memhwm)+") =-\n";
					} else if(memrss.size() != 0) {
						temp+="-= Mem usage: "+memrss+"\n =-";
					}

					if(!memvmp.empty() && !memvms.empty()) {
						temp+="-= VM size (Peak): "+formatSized(memvms)+ " ("+formatSized(memvmp)+") =-\n";
					} else if(memvms.size() != 0) {
						temp+="-= VM size: "+memvms+" =-\n";
					}

					if(!memstk.empty() && !memlib.empty()) {
						temp+="-= Stack size / Libs size: "+memstk+ " / "+formatSized(memlib)+" ";
					}
			}
			return temp;
}

std::string WulforUtil::formatSized(std::string& nonf)
{
	size_t needle = nonf.find_last_of(' ');
	if(needle != string::npos) {
		string sub = nonf.substr(0,needle);
		int64_t i = dcpp::Util::toInt64(sub);
		i*=1000;
		return dcpp::Util::formatBytes(i);
	}
	return nonf;
}

bool WulforUtil::HitIP(string& name/*, string &sIp*/)
{
	/*bool isOkIpV6 = false;
	if(name.empty()) return false;
	size_t n = std::count(name.begin(), name.end(), ':');
	if( (n==2) && (name.size() == 2) ) return true;//Fix for "::"
	if(n == 0 || n < 2)
			return Ipv4Hit(name,sIp);
	bool ok = false;
	for(auto i = name.begin();i!=name.end();++i) {
			if(*i==':') {
				for(int j = 5; j>0;--j){
						if(isxdigit(name[j])){ ok = true; }
				}
		}
			if(ok){break;}
	}
	bool ok2 = false;
	for(auto i = name.end();i!=name.begin();--i) {
			if(*i==':') {
				for(int q = 0; q<5;++q){
						if(isxdigit(name[q])) { ok2 = true; }
				}
			}
		if(ok2) {break;}
	}
	if( (ok == true ) || (ok2 == true)) {
		struct sockaddr_in sa;
		int result = inet_pton(AF_INET6,name.c_str() , &(sa.sin_addr));
		isOkIpV6 = result == 1;
	}

	if(isOkIpV6)
	{
		sIp = name;
		return isOkIpV6;
	}*/
	bool isOk = Util::isIp6(name);
	if(isOk) {
		return true;
	}	
	return Ipv4Hit(name/*,sIp*/);
}
/* Inspired by StrongDC catch code ips */
bool WulforUtil::Ipv4Hit(string &name/*, string &sIp*/) {
	for(uint32_t i = 0;i < name.length(); i++)
	{
		if(!((name[i] == 0) || (name[i] == '.') || ((name[i] >= '0') && (name[i] <= '9')))) {
			return false;
		}
	}

	name += ".";
	size_t begin = 0, pos = string::npos, end = 0;
	bool isOk = true;
	for(int i = 0; i < 4; i++) {
		pos = name.find('.', begin);
		if(pos == string::npos) {
			isOk = false;
			break;
		}
		end = atoi(Text::fromT(name.substr(begin)).c_str());
		if(end > 255) {
			isOk = false;
			break;
		}
		begin = pos + 1;
	}

	if(isOk)
	{
		size_t nedle = name.find_last_of(".");
		name = name.substr(0,nedle);
//		sIp = name.substr(0,pos);
		struct sockaddr_in sa;
		int result = inet_pton(AF_INET,name.c_str() , &(sa.sin_addr));
		isOk = result == 1;
	}
	return isOk;

}

string WulforUtil::cpuinfo()
{
	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cpu_info[2048];
	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep - 1 - line);
			strncpy(val, sep + 1, strlen(sep) - 1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			} 
		}

		fclose(cpuinfo);
		cpu_type[strlen(cpu_type)-1] = '\0';
		sprintf(cpu_info," CPU:  %d Core, %s ", num_cpus, cpu_type);
	}
	return string(cpu_info);
}

void WulforUtil::setTextDeufaults(GtkWidget* widget, std::string strcolor, std::string back_image_path /*= dcpp::Util::emptyString*/,bool pm/* = false*/,std::string hubUrl /*= dcpp::Util::emptyString*/,std::string where /*= dcpp::Util::emptyString*/)
{
		if( (pm == false) && hubUrl.empty())
			gtk_widget_set_name(widget,"Hub");

		if( pm == true)
			gtk_widget_set_name(widget,"pm");

		std::string hubCid;
		if(!hubUrl.empty() && (pm == false)) {
			hubCid = dcpp::CID(hubUrl.c_str()).toBase32();
			gtk_widget_set_name(widget,hubCid.c_str());
		}
		if(!where.empty())
			gtk_widget_set_name(widget,where.c_str());

		// Intialize the chat window
		std::string mono = dcpp::Util::emptyString;
		if (SETTING(USE_OEM_MONOFONT))
		{
			mono = "Monospace";
		}

		if( !back_image_path.empty() && (dcpp::Util::fileExists(back_image_path) == true) ) {
		///NOTE: CSS
			dcdebug("Test:img %s\n",hubUrl.c_str());
			GtkCssProvider *provider = gtk_css_provider_new ();
			GdkDisplay *display = gdk_display_get_default ();
			GdkScreen *screen = gdk_display_get_default_screen (display);

			std::string t_css = std::string("GtkTextView#") + (pm ? "pm" : ( hubCid.empty() ? "Hub": hubCid )) + "{\n"
                            "   background: url('"+back_image_path+"');\n"
                            "}\n\0";
			
			if(!mono.empty()) {
				t_css = std::string("GtkTextView#") + (pm ? "pm" : ( hubCid.empty() ? "Hub": hubCid )) + "{\n"
                            "   background: url('"+back_image_path+"');\n"
                            "	font: "+mono+";\n"
                            "}\n\0";
           }                

			gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);

			gtk_style_context_add_provider_for_screen (screen,
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

			g_object_unref (provider);
			return;
		}
		
		GtkCssProvider *provider = gtk_css_provider_new ();
		GdkDisplay *display = gdk_display_get_default ();
		GdkScreen *screen = gdk_display_get_default_screen (display);
		std::string strwhat = (pm ? "pm" : ( hubCid.empty() ? "Hub": hubCid ));
		if(!where.empty()) strwhat = where;
				
		std::string t_css =std::string("GtkTextView#"+strwhat+":insensitive, GtkTextView#"+strwhat+" { background: "+strcolor+" ;}\n\0");
				
		if(!mono.empty()) {
			t_css =	std::string("GtkTextView#"+strwhat+":insensitive, GtkTextView#"+strwhat+" { background: "+strcolor+" ;\n font: "+mono+"; }\n\0");	
		}	

		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);

		gtk_style_context_add_provider_for_screen (screen,
														GTK_STYLE_PROVIDER(provider),
														GTK_STYLE_PROVIDER_PRIORITY_USER);
		g_object_unref (provider);
}

void WulforUtil::setTextColor(std::string color,std::string where /*= dcpp::Util::emptyString*/)
//Note : selected is red, because most themes get white or black
{
		GtkCssProvider *provider = gtk_css_provider_new ();
		GdkDisplay *display = gdk_display_get_default ();
		GdkScreen *screen = gdk_display_get_default_screen (display);
		std::string t_css = std::string("GtkTextView#"+where+" ,GtkTextView#"+where+":insensitive, GtkTextView#"+where+":focused, GtkTextView#"+where+":active { color: "+color+" ;} GtkTextView#"+where+":selected { color: red ; }	\n\0");

		gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (provider),t_css.c_str(),-1, NULL);

		gtk_style_context_add_provider_for_screen (screen,
																GTK_STYLE_PROVIDER(provider),
																GTK_STYLE_PROVIDER_PRIORITY_USER);
		g_object_unref (provider);
}

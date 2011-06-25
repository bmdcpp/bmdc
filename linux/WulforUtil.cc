/*
 * Copyright © 2004-2011 Jens Oknelid, paskharen@gmail.com
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
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include "settingsmanager.hh"
#include "version.hh"

#include <dcpp/HashManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/version.h>
#include <dcpp/StringTokenizer.h>

#include <dcpp/UploadManager.h>
#include <dcpp/DownloadManager.h>
#include <dcpp/RawManager.h>

#include "ShellCommand.hh"
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include <regex.h>

#ifdef HAVE_IFADDRS_H
	#include <ifaddrs.h>
	#include <net/if.h>
#endif

using namespace std;
using namespace dcpp;

const string WulforUtil::ENCODING_LOCALE = _("System default");
vector<string> WulforUtil::charsets;
vector<std::pair<std::string,int> > WulforUtil::actions;
const string WulforUtil::magnetSignature = "magnet:?xt=urn:tree:tiger:";
GtkIconFactory* WulforUtil::iconFactory = NULL;

std::map<std::string,std::string> WulforUtil::m_mimetyp;//.avi - > mimetype

const char* WulforUtil::CountryNames[] = { "ANDORRA", "UNITED ARAB EMIRATES", "AFGHANISTAN", "ANTIGUA AND BARBUDA",
"ANGUILLA", "ALBANIA", "ARMENIA", "NETHERLANDS ANTILLES", "ANGOLA", "ANTARCTICA", "ARGENTINA", "AMERICAN SAMOA",
"AUSTRIA", "AUSTRALIA", "ARUBA", "ALAND", "AZERBAIJAN", "BOSNIA AND HERZEGOVINA", "BARBADOS", "BANGLADESH",
"BELGIUM", "BURKINA FASO", "BULGARIA", "BAHRAIN", "BURUNDI", "BENIN", "BERMUDA", "BRUNEI DARUSSALAM", "BOLIVIA",
"BRAZIL", "BAHAMAS", "BHUTAN", "BOUVET ISLAND", "BOTSWANA", "BELARUS", "BELIZE", "CANADA", "COCOS ISLANDS",
"THE DEMOCRATIC REPUBLIC OF THE CONGO", "CENTRAL AFRICAN REPUBLIC", "CONGO", "COTE D'IVOIRE", "COOK ISLANDS",
"CHILE", "CAMEROON", "CHINA", "COLOMBIA", "COSTA RICA", "SERBIA AND MONTENEGRO", "CUBA", "CAPE VERDE",
"CHRISTMAS ISLAND", "CYPRUS", "CZECH REPUBLIC", "GERMANY", "DJIBOUTI", "DENMARK", "DOMINICA", "DOMINICAN REPUBLIC",
"ALGERIA", "ECUADOR", "ESTONIA", "EGYPT", "WESTERN SAHARA", "ERITREA", "SPAIN", "ETHIOPIA", "FINLAND", "FIJI",
"FALKLAND ISLANDS", "MICRONESIA", "FAROE ISLANDS", "FRANCE", "GABON", "UNITED KINGDOM", "GRENADA", "GEORGIA",
"FRENCH GUIANA", "GHANA", "GIBRALTAR", "GREENLAND", "GAMBIA", "GUINEA", "GUADELOUPE", "EQUATORIAL GUINEA",
"GREECE", "SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS", "GUATEMALA", "GUAM", "GUINEA-BISSAU", "GUYANA",
"HONG KONG", "HEARD ISLAND AND MCDONALD ISLANDS", "HONDURAS", "CROATIA", "HAITI", "HUNGARY", "SWITZERLAND",
"INDONESIA", "IRELAND", "ISRAEL", "INDIA", "BRITISH INDIAN OCEAN TERRITORY", "IRAQ", "IRAN", "ICELAND",
"ITALY", "JAMAICA", "JORDAN", "JAPAN", "KENYA", "KYRGYZSTAN", "CAMBODIA", "KIRIBATI", "COMOROS",
"SAINT KITTS AND NEVIS", "DEMOCRATIC PEOPLE'S REPUBLIC OF KOREA", "SOUTH KOREA", "KUWAIT", "CAYMAN ISLANDS",
"KAZAKHSTAN", "LAO PEOPLE'S DEMOCRATIC REPUBLIC", "LEBANON", "SAINT LUCIA", "LIECHTENSTEIN", "SRI LANKA",
"LIBERIA", "LESOTHO", "LITHUANIA", "LUXEMBOURG", "LATVIA", "LIBYAN ARAB JAMAHIRIYA", "MOROCCO", "MONACO",
"MOLDOVA", "MADAGASCAR", "MARSHALL ISLANDS", "MACEDONIA", "MALI", "MYANMAR", "MONGOLIA", "MACAO",
"NORTHERN MARIANA ISLANDS", "MARTINIQUE", "MAURITANIA", "MONTSERRAT", "MALTA", "MAURITIUS", "MALDIVES",
"MALAWI", "MEXICO", "MALAYSIA", "MOZAMBIQUE", "NAMIBIA", "NEW CALEDONIA", "NIGER", "NORFOLK ISLAND",
"NIGERIA", "NICARAGUA", "NETHERLANDS", "NORWAY", "NEPAL", "NAURU", "NIUE", "NEW ZEALAND", "OMAN", "PANAMA",
"PERU", "FRENCH POLYNESIA", "PAPUA NEW GUINEA", "PHILIPPINES", "PAKISTAN", "POLAND", "SAINT PIERRE AND MIQUELON",
"PITCAIRN", "PUERTO RICO", "PALESTINIAN TERRITORY", "PORTUGAL", "PALAU", "PARAGUAY", "QATAR", "REUNION",
"ROMANIA", "RUSSIAN FEDERATION", "RWANDA", "SAUDI ARABIA", "SOLOMON ISLANDS", "SEYCHELLES", "SUDAN",
"SWEDEN", "SINGAPORE", "SAINT HELENA", "SLOVENIA", "SVALBARD AND JAN MAYEN", "SLOVAKIA", "SIERRA LEONE",
"SAN MARINO", "SENEGAL", "SOMALIA", "SURINAME", "SAO TOME AND PRINCIPE", "EL SALVADOR", "SYRIAN ARAB REPUBLIC",
"SWAZILAND", "TURKS AND CAICOS ISLANDS", "CHAD", "FRENCH SOUTHERN TERRITORIES", "TOGO", "THAILAND", "TAJIKISTAN",
"TOKELAU", "TIMOR-LESTE", "TURKMENISTAN", "TUNISIA", "TONGA", "TURKEY", "TRINIDAD AND TOBAGO", "TUVALU", "TAIWAN",
"TANZANIA", "UKRAINE", "UGANDA", "UNITED STATES MINOR OUTLYING ISLANDS", "UNITED STATES", "URUGUAY", "UZBEKISTAN",
"VATICAN", "SAINT VINCENT AND THE GRENADINES", "VENEZUELA", "BRITISH VIRGIN ISLANDS", "U.S. VIRGIN ISLANDS",
"VIET NAM", "VANUATU", "WALLIS AND FUTUNA", "SAMOA", "YEMEN", "MAYOTTE", "YUGOSLAVIA", "SOUTH AFRICA", "ZAMBIA",
"ZIMBABWE", "EUROPEAN UNION", "SERBIA", "MONTENEGRO", "GUERNSEY", "ISLE OF MAN", "JERSEY" };

static const char* WulforUtil::CountryCodes[] = { "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AN", "AO", "AQ", "AR", "AS",
"AT", "AU", "AW", "AX", "AZ", "BA", "BB", "BD", "BE", "BF", "BG", "BH", "BI", "BJ", "BM", "BN", "BO", "BR",
"BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC", "CD", "CF", "CG", "CI", "CK", "CL", "CM", "CN", "CO", "CR",
"CS", "CU", "CV", "CX", "CY", "CZ", "DE", "DJ", "DK", "DM", "DO", "DZ", "EC", "EE", "EG", "EH", "ER", "ES",
"ET", "FI", "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE", "GF", "GH", "GI", "GL", "GM", "GN", "GP",
"GQ", "GR", "GS", "GT", "GU", "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "CH", "ID", "IE", "IL", "IN",
"IO", "IQ", "IR", "IS", "IT", "JM", "JO", "JP", "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW", "KY",
"KZ", "LA", "LB", "LC", "LI", "LK", "LR", "LS", "LT", "LU", "LV", "LY", "MA", "MC", "MD", "MG", "MH", "MK",
"ML", "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT", "MU", "MV", "MW", "MX", "MY", "MZ", "NA", "NC", "NE",
"NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", "PE", "PF", "PG", "PH", "PK", "PL", "PM",
"PN", "PR", "PS", "PT", "PW", "PY", "QA", "RE", "RO", "RU", "RW", "SA", "SB", "SC", "SD", "SE", "SG", "SH",
"SI", "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "ST", "SV", "SY", "SZ", "TC", "TD", "TF", "TG", "TH", "TJ",
"TK", "TL", "TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG", "UM", "US", "UY", "UZ", "VA", "VC",
"VE", "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YT", "YU", "ZA", "ZM", "ZW", "EU", "RS", "ME", "GG", "IM",
"JE" };

#define AD "AD"
#define AE "AE"
#define AF "AF"
#define AG "AG"
#define AI "AI"
#define AL "AL"
#define AM "AM"
#define AN "AN"
#define AO "AO"
#define AQ "AQ"
#define AR "AR"
#define AS "AS"
#define AT "AT"
#define AU "AU"
#define AW "AW"
#define AX "AX"
#define AZ "AZ"
#define BA "BA"
#define BB "BB"
#define BD "BD"
#define BE "BE"
#define BF "BF"
#define BG "BG"
#define BH "BH"
#define BI "BI"
#define BJ "BJ"
#define BM "BM"
#define BN "BN"
#define BO "BO"
#define BR "BR"
#define BS "BS"
#define BT "BT"
#define BV "BV"
#define BW "BW"
#define BY "BY"
#define BZ "BZ"
#define CA "CA"
#define CC "CC"
#define CD "CD"
#define CF "CF"
#define CG "CG"
#define CI "CI"
#define CK "CK"
#define CL "CL"
#define CM "CM"
#define CN "CN"
#define CO "CO"
#define CR "CR"
#define CS "CS"
#define CU "CU"
#define CV "CV"
#define CX "CX"
#define CY "CY"
#define CZ "CZ"
#define DE "DE"
#define DJ "DJ"
#define DK "DK"
#define DM "DM"
#define DO "DO"
#define DZ "DZ"
#define EC "EC"
#define EE "EE"
#define EG "EG"
#define EH "EH"
#define ER "ER"
#define ES "ES"
#define ET "ET"
#define FI "FI"
#define FJ "FJ"
#define FK "FK"
#define FM "FM"
#define FO "FO"
#define FR "FR"
#define GA "GA"
#define GB "GB"
#define GD "GD"
#define GE "GE"
#define GF "GF"
#define GH "GH"
#define GI "GI"
#define GL "GL"
#define GM "GM"
#define GN "GN"
#define GP "GP"
#define GQ "GQ"
#define GR "GR"
#ifdef GS
#undef GS
#endif
#define GS "GS"
#define GT "GT"
#define GU "GU"
#define GW "GW"
#define GY "GY"
#define HK "HK"
#define HM "HM"
#define HN "HN"
#define HR "HR"
#define HT "HT"
#define HU "HU"
#define CH "CH"
#define ID "ID"
#define IE "IE"
#define IL "IL"
#define IN "IN"
#define IO "IO"
#define IQ "IQ"
#define IR "IR"
#define IS "IS"
#define IT "IT"
#define JM "JM"
#define JO "JO"
#define JP "JP"
#define KE "KE"
#define KG "KG"
#define KH "KH"
#define KI "KI"
#define KM "KM"
#define KN "KN"
#define KP "KP"
#define KR "KR"
#define KW "KW"
#define KY "KY"
#define KZ "KZ"
#define LA "LA"
#define LB "LB"
#define LC "LC"
#define LI "LI"
#define LK "LK"
#define LR "LR"
#define LS "LS"
#define LT "LT"
#define LU "LU"
#define LV "LV"
#define LY "LY"
#define MA "MA"
#define MC "MC"
#define MD "MD"
#define MG "MG"
#define MH "MH"
#define MK "MK"
#define ML "ML"
#define MM "MM"
#define MN "MN"
#define MO "MO"
#define MP "MP"
#define MQ "MQ"
#define MR "MR"
#define MS "MS"
#define MT "MT"
#define MU "MU"
#define MV "MV"
#define MW "MW"
#define MX "MX"
#define MY "MY"
#define MZ "MZ"
#define NA "NA"
#define NC "NC"
#define NE "NE"
#define NF "NF"
#define NG "NG"
#define NI "NI"
#define NL "NL"
#define NO "NO"
#define NP "NP"
#define NR "NR"
#define NU "NU"
#define NZ "NZ"
#define OM "OM"
#define PA "PA"
#define PE "PE"
#define PF "PF"
#define PG "PG"
#define PH "PH"
#define PK "PK"
#define PL "PL"
#define PM "PM"
#define PN "PN"
#define PR "PR"
#define PS "PS"
#define PT "PT"
#define PW "PW"
#define PY "PY"
#define QA "QA"
#define RE "RE"
#define RO "RO"
#define RU "RU"
#define RW "RW"
#define SA "SA"
#define SB "SB"
#define SC "SC"
#define SD "SD"
#define SE "SE"
#define SG "SG"
#define SH "SH"
#define SI "SI"
#define SJ "SJ"
#define SK "SK"
#define SL "SL"
#define SM "SM"
#define SN "SN"
#define SO "SO"
#define SR "SR"
#define ST "ST"
#define SV "SV"
#define SY "SY"
#define SZ "SZ"
#define TC "TC"
#define TL "TL"
#define TM "TM"
#define TN "TN"
#define TO "TO"
#define TR "TR"
#define TT "TT"
#define TV "TV"
#define TW "TW"
#define TZ "TZ"
#define UA "UA"
#define UG "UG"
#define UM "UM"
#define US "US"
#define UY "UY"
#define UZ "UZ"
#define VA "VA"
#define VC "VC"
#define VE "VE"
#define VG "VG"
#define VI "VI"
#define VN "VN"
#define VU "VU"
#define WF "WF"
#define WS "WS"
#define YE "YE"
#define YT "YT"
#define YU "YU"
#define ZA "ZA"
#define ZM "ZM"
#define ZW "ZW"
#define EU "EU"
#define RS "RS"
#define ME "ME"
#define GG "GG"
#define IM "IM"
#define JE "JE"
#define NONE "questionb"

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

vector<string> WulforUtil::getLocalIPs()
{
	vector<string> addresses;

#ifdef HAVE_IFADDRS_H
	struct ifaddrs *ifap;

	if (getifaddrs(&ifap) == 0)
	{
		for (struct ifaddrs *i = ifap; i != NULL; i = i->ifa_next)
		{
			struct sockaddr *sa = i->ifa_addr;

			// If the interface is up, is not a loopback and it has an address
			if ((i->ifa_flags & IFF_UP) && !(i->ifa_flags & IFF_LOOPBACK) && sa != NULL)
			{
				void* src = NULL;
				socklen_t len;

				// IPv4 address
				if (sa->sa_family == AF_INET)
				{
					struct sockaddr_in* sai = (struct sockaddr_in*)sa;
					src = (void*) &(sai->sin_addr);
					len = INET_ADDRSTRLEN;
				}
				// IPv6 address
				else if (sa->sa_family == AF_INET6)
				{
					struct sockaddr_in6* sai6 = (struct sockaddr_in6*)sa;
					src = (void*) &(sai6->sin6_addr);
					len = INET6_ADDRSTRLEN;
				}

				// Convert the binary address to a string and add it to the output list
				if (src != NULL)
				{
					char address[len];
					inet_ntop(sa->sa_family, src, address, len);
					addresses.push_back(address);
				}
			}
		}
		freeifaddrs(ifap);
	}
#endif

	return addresses;
}

//NOTE: core 0.762
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
//NOTE: core 0.762
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
	if (charsets.size() == 0)
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

void WulforUtil::openURI(const string &uri)
{
	GError* error = NULL;
	gchar *argv[3];

#if defined(__APPLE__)
	argv[0] = (gchar *)"open";
#elif defined(_WIN32)
	argv[0] = (gchar *)"start";
#else
	argv[0] = (gchar *)"xdg-open";
#endif
	argv[1] = (gchar *)Text::fromUtf8(uri).c_str();
	argv[2] = NULL;

	g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error);

	if (error != NULL)
	{
		cerr << "Failed to open URI: " << error->message << endl;
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

string WulforUtil::colorToString(const GdkColor *color)
{
	gchar strcolor[14];

	g_snprintf(strcolor, sizeof(strcolor), "#%04X%04X%04X",
		color->red, color->green, color->blue);

	return strcolor;
}

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

	if (!isMagnet(magnet.c_str()) || magnet.size() <= magnetSignature.length())
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
	string configPath = home ? string(home) + "/.bmdc++/" : "/tmp/";
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
		struct flock testlock = lock;

		if (fcntl(fd, F_GETLK, &testlock) != -1) // Locking not supported
		{
			if (fcntl(fd, F_SETLK, &lock) == -1)
				profileIsLocked = true;
		}
	}

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
	GValue value = {0, };
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
	GValue value = {0, };
	gtk_tree_model_get_value(GTK_TREE_MODEL(store), fromIter, position, &value);
	gtk_tree_store_set_value(store, toIter, position, &value);
	g_value_unset(&value);
}

GtkStockItem WulforUtil::icons[] ={
		{ AD , "AD" },
		{ AE , "AE"},
		{ AF , "AF"},
		{ AG , "AG"},
		{AI , "AI"},
		{AL , "AL"},
	{AM , "AM"},
	{ AN , "AN"},
	{ AO , "AO"},
	{ AQ , "AQ"},
	{ AR , "AR"},
	{ AS ,"AS"},
	{ AT ,"AT"},
	{ AU ,"AU"},
	{ AW ,"AW"},
	{ AX ,"AX"},
	{ AZ ,"AZ"},
	{ BA ,"BA"},
	{ BB ,"BB"},
	{ BD ,"BD"},
	{ BE ,"BE"},
	{ BF ,"BF"},
	{ BG ,"BG"},
	{ BH ,"BH"},
	{ BI ,"BI"},
	{ BJ ,"BJ"},
	{ BM ,"BM"},
	{ BN ,"BN"},
	{ BO ,"BO"},
	{ BR ,"BR"},
	{ BS ,"BS"},
	{ BT ,"BT"},
	{ BV ,"BV"},
	{ BW ,"BW"},
	{ BY ,"BY"},
	{ BZ ,"BZ"},
	{ CA ,"CA"},
	{ CC ,"CC"},
	{ CD ,"CD"},
	{ CF ,"CF"},
	{ CG ,"CG"},
	{ CI ,"CI"},
	{ CK ,"CK"},
	{ CL ,"CL"},
	{ CM ,"CM"},
	{ CN ,"CN"},
	{ CO ,"CO"},
	{ CR ,"CR"},
	{ CS ,"CS"},
	{ CU ,"CU"},
	{ CV ,"CV"},
	{ CX ,"CX"},
	{ CY ,"CY"},
	{ CZ ,"CZ"},
	{ DE ,"DE"},
	{ DJ ,"DJ"},
	{ DK ,"DK"},
	{ DM ,"DM"},
	{ DO ,"DO"},
	{ DZ ,"DZ"},
	{ EC ,"EC"},
	{ EE ,"EE"},
	{ EG ,"EG"},
	{ EH ,"EH"},
	{ ER ,"ER"},
	{ ES ,"ES"},
	{ ET ,"ET"},
	{ FI ,"fi"},
	{ FJ ,"FJ"},
	{ FK ,"FK"},
	{ FM ,"FM"},
	{ FO ,"FO"},
	{ FR ,"FR"},
	{ GA ,"GA"},
	{ GB ,"GB"},
	{ GD ,"GD"},
	{ GE ,"GE"},
	{ GF ,"GF"},
	{ GH ,"GH"},
	{ GI ,"GI"},
	{ GL ,"GL"},
	{ GM ,"GM"},
	{ GN ,"GN"},
	{ GP ,"GP"},
	{ GQ ,"GQ"},
	{ GR ,"GR"},
	{ GS ,"GS"},
	{ GT ,"GT"},
	{ GU ,"GU"},
	{ GW ,"GW"},
	{ GY ,"GY"},
	{ HK ,"HK"},
	{ HM ,"HM"},
	{ HN ,"HN"},
	{ HR ,"HR"},
	{ HT ,"HT"},
	{ HU, "HU"},
	{ CH ,"CH"},
	{ ID ,"ID"},
	{ IE ,"IE"},
	{ IL ,"IL"},
	{ IN ,"IN"},
	{ IO ,"IO"},
	{ IQ ,"IQ"},
	{ IR ,"IR"},
	{ IS ,"IS"},
	{ IT ,"IT"},
	{ JM ,"JM"},
	{ JO ,"JO"},
	{ JP ,"JP"},
	{ KE ,"KE"},
	{ KG ,"KG"},
	{ KH ,"KH"},
	{ KI ,"KI"},
	{ KM ,"KM"},
	{ KN ,"KN"},
	{ KP ,"KP"},
	{ KR ,"KR"},
	{ KW ,"KW"},
	{ KY ,"KY"},
	{ KZ ,"KZ"},
	{ LA ,"LA"},
	{ LB ,"LB"},
	{ LC ,"LC"},
	{ LI ,"LI"},
	{ LK ,"LK"},
	{ LR ,"LR"},
	{ LS ,"LS"},
	{ LT ,"LT"},
	{ LU ,"LU"},
	{ LV ,"LV"},
	{ LY ,"LY"},
	{ MA ,"MA"},
	{ MC ,"MC"},
	{ MD ,"MD"},
	{ MG ,"MG"},
	{ MH ,"MH"},
	{ MK ,"MK"},
	{ ML ,"ML"},
	{ MM ,"MM"},
	{ MN ,"MN"},
	{ MO ,"MO"},
	{ MP ,"MP"},
	{ MQ ,"MQ"},
	{ MR ,"MR"},
	{ MS ,"MS"},
	{ MT ,"MT"},
	{ MU ,"MU"},
	{ MV ,"MV"},
	{ MW ,"MW"},
	{ MX ,"MX"},
	{ MY ,"MY"},
	{ MZ ,"MZ"},
	{ NA ,"NA"},
	{ NC ,"NC"},
	{ NE ,"NE"},
	{ NF ,"NF"},
	{ NG ,"NG"},
	{ NI ,"NI"},
	{ NL ,"NL"},
	{ NO ,"NO"},
	{ NP ,"NP"},
	{ NR ,"NR"},
	{ NU ,"NU"},
	{ NZ ,"NZ"},
	{ OM ,"OM"},
	{ PA ,"PA"},
	{ PE ,"PE"},
	{ PF ,"PF"},
	{ PG ,"PG"},
	{ PH ,"PH"},
	{ PK ,"PK"},
	{ PL ,"PL"},
	{ PM ,"PM"},
	{ PN ,"PN"},
	{ PR ,"PR"},
	{ PS ,"PS"},
	{ PT ,"PT"},
	{ PW ,"PW"},
	{ PY ,"PY"},
	{ QA ,"QA"},
	{ RE ,"RE"},
	{ RO ,"RO"},
	{ RU ,"RU"},
	{ RW ,"RW"},
	{ SA ,"SA"},
	{ SB ,"SB"},
	{ SC ,"SC"},
	{ SD ,"SD"},
	{ SE ,"SE"},
	{ SG ,"SG"},
	{ SH ,"SH"},
	{ SI ,"SI"},
	{ SJ ,"SJ"},
	{ SK ,"SK"},
	{ SL ,"SL"},
	{ SM ,"SM"},
	{ SN ,"SN"},
	{ SO ,"SO"},
	{ SR ,"SR"},
	{ ST ,"ST"},
	{ SV ,"SV"},
	{ SY ,"SY"},
	{ SZ ,"SZ"},
	{ TC ,"TC"},
	{ TL ,"TL"},
	{ TM ,"TM"},
	{ TN ,"TN"},
	{ TO ,"TO"},
	{ TR ,"TR"},
	{ TT ,"TT"},
	{ TV ,"TV"},
	{ TW ,"TW"},
	{ TZ ,"TZ"},
	{ UA ,"UA"},
	{ UG ,"UG"},
	{ UM ,"UM"},
	{ US ,"US"},
	{ UY ,"UY"},
	{ UZ ,"UZ"},
	{ VA ,"VA"},
	{ VC ,"VC"},
	{ VE ,"VE"},
	{ VG ,"VG"},
	{ VI ,"VI"},
	{ VN ,"VN"},
	{ VU ,"VU"},
	{ WF ,"WF"},
	{ WS ,"WS"},
	{ YE ,"YE"},
	{ YT ,"YT"},
	{ YU ,"YU"},
	{ ZA ,"ZA"},
	{ ZM ,"ZM"},
	{ ZW ,"ZW"},
	{ EU ,"EU"},
	{ RS ,"RS"},
	{ ME ,"ME"},
	{ GG ,"GG"},
	{ IM ,"IM"},
	{ JE ,"JE"}
	};

/*
* load Country Flag
*/

GdkPixbuf *WulforUtil::loadCountry(const string &country)
{

	gtk_stock_add(icons,G_N_ELEMENTS(icons));
	GtkIconFactory *ffactory = gtk_icon_factory_new();
	for (int i = 0; i < G_N_ELEMENTS(icons); i++)
	{

			if(country==icons[i].stock_id)
			{
				gchar *path = g_strdup_printf(_DATADIR "/country/%s.png",
		                              icons[i].stock_id);
				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
					if (pixbuf == NULL)
						g_error("Cannot open stock image: %s", path);
				g_free(path);

				gtk_icon_factory_add(ffactory, icons[i].stock_id,
		                    gtk_icon_set_new_from_pixbuf(pixbuf));

				return pixbuf;
			}
	}

	GtkStockItem item[] ={{NONE, "questionb"}};
	gchar *path = g_strdup_printf(_DATADIR "/country/%s.png", item[0].stock_id);
	GdkPixbuf *buf = gdk_pixbuf_new_from_file(path, NULL);
	if(buf != NULL)
		return buf;
	else
		return NULL;

}

std::string WulforUtil::StringToUpper(std::string myString)
{
  const int length = myString.length();
  for(int i=0; i!=length ; ++i)
  {
    myString[i] = std::toupper(myString[i]);
  }
  return myString;
}

string WulforUtil::getCC(string _countryname)
{
	string _cc=StringToUpper(_countryname);
	 for(uint8_t q = 0; q < (sizeof(CountryNames) / sizeof(CountryNames[0])); q++)
	 {
		if(_cc == CountryNames[q])
				return CountryCodes[q];

	 }
	return "bmdc-none";
}

/*
 * Registers either the custom icons or the GTK+ icons as stock icons in
 * GtkIconFactory according to the user's preference. If the icons have
 * previously been loaded, they are removed and re-added.
 */
void WulforUtil::registerIcons()
{
	// Holds a mapping of custom icon names -> stock icon names.
	// Not all icons have stock representations.
	WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
	map<string, string> icons;
	icons["bmdc"] = "bmdc";
	icons["bmdc-smile"] = wsm->getString("icon-smile");
	icons["bmdc-download"] = wsm->getString("icon-download");
	icons["bmdc-favorite-hubs"] = wsm->getString("icon-favorite-hubs");
	icons["bmdc-favorite-users"] = wsm->getString("icon-favorite-users");
	icons["bmdc-finished-downloads"] = wsm->getString("icon-finished-downloads");
	icons["bmdc-finished-uploads"] = wsm->getString("icon-finished-uploads");
	icons["bmdc-hash"] = wsm->getString("icon-hash");
	icons["bmdc-preferences"] = wsm->getString("icon-preferences");
	icons["bmdc-public-hubs"] = wsm->getString("icon-public-hubs");
	icons["bmdc-queue"] = wsm->getString("icon-queue");
	icons["bmdc-search"] = wsm->getString("icon-search");
	icons["bmdc-search-spy"] = wsm->getString("icon-search-spy");
	icons["bmdc-upload"] = wsm->getString("icon-upload");
	icons["bmdc-quit"] = wsm->getString("icon-quit");
	icons["bmdc-connect"] = wsm->getString("icon-connect");
	icons["bmdc-notepad"] = wsm->getString("icon-notepad");
	icons["bmdc-adlsearch"] = wsm->getString("icon-adlsearch");
	icons["bmdc-ignore-users"] = wsm->getString("icon-ignore");
	icons["bmdc-system"] = wsm->getString("icon-system");
	icons["bmdc-away"] = wsm->getString("icon-away");
	icons["bmdc-away-on"] = wsm->getString("icon-away-on");
	icons["bmdc-none"] = wsm->getString("icon-none");
	icons["bmdc-limiting"] = wsm->getString("icon-limiting");
	icons["bmdc-limiting-on"] = wsm->getString("icon-limiting-on");
	icons["bmdc-highlight"] = wsm->getString("icon-highlight");
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
    /*normal mode*/
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
	/*aways mode*/
	/**/
	icons["bmdc-op-away"] = wsm->getString("icon-op-away");
	icons["bmdc-modem-away"] = wsm->getString("icon-modem-away");
	icons["bmdc-wireless-away"] = wsm->getString("icon-wireless-away");
	icons["bmdc-dsl-away"] = wsm->getString("icon-dsl-away");
	icons["bmdc-lan-away"] = wsm->getString("icon-lan-away");
	icons["bmdc-netlimiter-away"] = wsm->getString("icon-netlimiter-away");
	/***/
	icons["bmdc-ten-away"] = wsm->getString("icon-ten-away");
	icons["bmdc-zeroone"] = wsm->getString("icon-zeroone-away");
	icons["bmdc-zerozeroone-away"] = wsm->getString("icon-zerozeroone-away");
	icons["bmdc-other-away"] = wsm->getString("icon-other-away");
	/*end*/
	/*normal pasive mod*/
	icons["bmdc-op-pasive"] = wsm->getString("icon-op-pasive");
	icons["bmdc-modem-pasive"] = wsm->getString("icon-modem-pasive");
	icons["bmdc-wireless-pasive"] = wsm->getString("icon-wireless-pasive");
	icons["bmdc-dsl-pasive"] = wsm->getString("icon-dsl");
	icons["bmdc-lan-pasive"] = wsm->getString("icon-lan-pasive");
	icons["bmdc-netlimiter-pasive"] = wsm->getString("icon-netlimiter-pasive");
	/***/
	icons["bmdc-ten-pasive"] = wsm->getString("icon-ten-pasive");
	icons["bmdc-zeroone-pasive"] = wsm->getString("icon-zeroone-pasive");
	icons["bmdc-zerozeroone-pasive"] = wsm->getString("icon-zerozeroone-pasive");
	icons["bmdc-other-pasive"] = wsm->getString("icon-other-pasive");
	/*aways pasive mode*/
	/**/
	icons["bmdc-op-away-pasive"] = wsm->getString("icon-op-away-pasive");
	icons["bmdc-modem-away-pasive"] = wsm->getString("icon-modem-away-pasive");
	icons["bmdc-wireless-away-pasive"] = wsm->getString("icon-wireless-away-pasive");
	icons["bmdc-dsl-away-pasive"] = wsm->getString("icon-dsl-away-pasive");
	icons["bmdc-lan-away-pasive"] = wsm->getString("icon-lan-away-pasive");
	icons["bmdc-netlimiter-away-pasive"] = wsm->getString("icon-netlimiter-away-pasive");
	/***/
	icons["bmdc-ten-away-pasive"] = wsm->getString("icon-ten-away-pasive");
	icons["bmdc-zeroone-pasive"] = wsm->getString("icon-zeroone-away-pasive");
	icons["bmdc-zerozeroone-away"] = wsm->getString("icon-zerozeroone-away-pasive");
	icons["bmdc-other-away-pasive"] = wsm->getString("icon-other-away-pasive");
	/*end*/

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

}

bool WulforUtil::checkCommand(string& cmd, string& param, string& message, string& status, bool& thirdperson)
{
	string tmp = cmd;
	bool isCMDSend = false;
	string::size_type separator = cmd.find_first_of(' ');

	if(separator != string::npos && cmd.size() > separator +1)
	{
		param = cmd.substr(separator + 1);
		cmd = cmd.substr(1, separator - 1);
	}
	else
	{
		cmd = cmd.substr(1);
	}

	std::transform(cmd.begin(), cmd.end(), cmd.begin(), (int(*)(int))tolower);
	/*commnads*/
	if(WGETB("show-commnads"))
	{
	    status = _("Command send: ");
	    status += tmp;
	    status += "\n";
	}

	if( cmd == "away" )
	{
		if (Util::getAway() && param.empty())
		{
				Util::setAway(FALSE);
				Util::setManualAway(FALSE);
				status += _("Away mode off");
		}
		else
		{
			Util::setAway(TRUE);
			Util::setManualAway(TRUE);
			Util::setAwayMessage(param);
			status += _("Away mode on: ") + Util::getAwayMessage();
		}
	}
	else if ( cmd == "back" )
	{
		Util::setAway(FALSE);
		status += _("Away mode off");

	} else if ( cmd == "bmdc" )
	{
        if(param == "mc")
            message = string(GUI_PACKAGE " " GUI_VERSION_STRING "." GUI_VERSION_BUILD_STRING "/" VERSIONSTRING " ");
        else
            status += string(GUI_PACKAGE " " GUI_VERSION_STRING "." GUI_VERSION_BUILD_STRING "/" VERSIONSTRING ", ") + _("project home: ") + "(Original version => http://freedcpp.googlecode.com), My Version => http://bmdc.no-ip.sk";

	} else if ( cmd == "ratio")
	{
			double ratio;
			double up =Util::toInt64(WGETS("up-st"));
			double dw =Util::toInt64(WGETS("dw-st"));
			if(dw > 0)
				ratio = up /dw;
			else
				ratio = 0;

			if(param == "mc")
				message = string("Ratio: " )+ Util::toString(ratio) + string(" ( Uploads: ")+ Util::formatBytes(up)+ "/ Downloads " + Util::formatBytes(dw)+" )";
			else
				status += string("Ratio: " )+ Util::toString(ratio) + string(" ( Uploads: ")+Util::formatBytes(up) + string("/ Downloads ") + Util::formatBytes(dw)+" )";
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
	}	else if ( cmd == "slots")
	{
		if (param == "0")
					status = _("No 0 Slots!!");
		else
		{
					SettingsManager *sm = SettingsManager::getInstance();

					sm->set(SettingsManager::SLOTS,Util::toInt(param));
					sm->set(SettingsManager::SLOTS_PRIMARY,Util::toInt(param));
					status += _("Set Slots to:")+param;
		}
	}
	else if (cmd == "stats")
	{
			int z,y;
			struct utsname u_name;//instance of utsname
			z = uname(&u_name);
			string sys_name(u_name.sysname);
			string node_name(u_name.nodename);
			string rel(u_name.release);
			string mach(u_name.machine);
			struct sysinfo sys;//instance of acct;
			y=sysinfo(&sys);
			unsigned long toram = sys.totalram * sys.mem_unit/1024;
			unsigned long uram = sys.freeram * sys.mem_unit/1024;
			const long minute = 60;
			const long hour = minute * 60;
			const long day = hour * 24;
			long upt=sys.uptime;
			long udays=upt/day;
			long uhour= (upt % day) / hour;
			long umin = (upt % hour) / minute;
			const unsigned long megabyte = 1024;
			/**/
			int dettotal = SETTING(DETECTT);
			int detfail = SETTING(DETECTF);

		message = "\n-=Stats " + string(GUI_PACKAGE) + " " + string(GUI_VERSION_STRING) + "/" + dcpp::fullVersionString + "=-\n"
					+ sys_name + " " + node_name + " " + rel + " " + mach + "\n"
					+ "Uptime: " + Util::formatSeconds(Util::getUptime()) + "\n"
					+ "Sys Uptime: " + Util::toString(udays) + " days," + Util::toString(uhour) + " Hours," + Util::toString(umin) + " min.\n"
					+ "Time: " + Util::getShortTimeString() + "\n"
					+ "Mem Usage (Free/Total):" + Util::toString(uram/megabyte) + " MB /" + Util::toString(toram/megabyte) + " MB \n"
					+ "Detection (failed/total) :" + Util::toString(detfail) + " /" + Util::toString(dettotal) + "\n";

	}
	/// "Now Playing" spam // added by curse and Irene
	else if (cmd == "amar")
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
	}
	// End of "Now Playing"
	else if ( cmd == "df" )
	{
        ShellCommand s("df.sh");

		if (param=="mc")
			message = s.Output();
		else
			status += s.Output();
	}
	else if (cmd == "uptime")
	{
		message = _("Uptime: ")+Util::formatSeconds(Util::getUptime());
	}
	else if ( cmd == "rebuild" )
	{
			HashManager::getInstance()->rebuild();
	}
	else if ( cmd == "cleanmc" )
	{
		message = "\n\t\t";
		message += _("---------- Cleaning the chat ----------");
		message += "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
		message += _("---------- The chat has been cleaned ----------");
	}
	//From CrZDC
	else if ( cmd == "leech" )
	{
		if (param == "mc")
			message = generateLeech();
		else
			status += generateLeech();
	}
	//alias patch
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

 				}
 				else
 				{
					///pridani aliasu
 					StringTokenizer<string> command( param, '::' );
 					string store(""), name("");
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
 					if( command.getTokens().size() == 3  && !exists )
 					{
 						aliases.getTokens().push_back( param );
 						string store("");
 						for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 						{
 							store = store + *i + "#";
 						}
 						WSET( "custom-aliases", store );
 					}
 				}
 			}
 		}
 		else if ( !WGETS("custom-aliases").empty() )
 		{
 			StringTokenizer<string> aliases( WGETS("custom-aliases"), '#' );
 			string name("");
 			for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
 			{
 				name = i->substr( 0, i->find_first_of( "::", 0 ) );
 				if( name.compare( cmd ) == 0 )
 				{
 					string exec = i->substr( i->find_first_of( "::", 0 ) + 2, i->size() );

 					if( !exec.empty() )
 					{
 						gchar *output = NULL;
 						GError *error = NULL;

 						g_spawn_command_line_sync( exec.c_str(), &output, NULL, NULL, &error);

 						if (error != NULL)
 						{
 							status = error->message;
 							g_error_free(error);
 						}
 						else
 						{
 							string trash( output );
 							g_free( output );
							message = trash;

 						}
					}
 					break;
 				}

 			}
  		}
  		else
 		{
           return false;
 		}

return true;
}

string WulforUtil::getReport(const Identity& identity)
{
	map<string, string> reportMap = identity.getReport();

	string report = _("*** Info on " )+ identity.getNick() + " ***" + "\n";

	for(map<string, string>::const_iterator i = reportMap.begin(); i != reportMap.end(); ++i)
	{
		report += "\n" + i->first + ": " +  i->second;
	}

	return report + "\n";
}


int WulforUtil::matchRe(const std::string/*&*/ strToMatch, const std::string/*&*/ expression, bool caseSensative /*= true*/) {
		/*try {
			const boost::regex reg(expression, caseSensative ? 0 : boost::regex::icase);
			return boost::regex_search(strToMatch.begin(), strToMatch.end(), reg);
		} catch(...) {
			//...
		}
		return false;*/
			int    status;
			regex_t    re;
			const char *pattern=expression.c_str();
			const char *strings=strToMatch.c_str();
			    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
			        return 0;      /* Report error. */
    				}
			    status = regexec(&re, strings, (size_t) 0, NULL, 0);
			    regfree(&re);
			    if (status != 0) {
			        return 0;    /* Report error. */
				    }
    			return 1;
}
///From Crzdc
string WulforUtil::generateLeech() {

	char buf[650];
	snprintf(buf, sizeof(buf), "\n\t [ BMDC++ %s %s Leech Stats ]\r\n [ Downloaded:\t\t\t %s ]\r\n [ Uploaded:\t\t\t %s ]\r\n [ Total Download:\t\t %s ]\r\n [ Total Upload:\t\t\t %s ]\r\n [ Ratio: \t\t\t\t %s ]\r\n [ Current Uploads:\t\t %s Running Upload(s) ]\r\n [ Current Upload Speed: \t\t %s/s ]\r\n [ Current Downloads:\t\t %s Running Download(s) ]\r\n [ Current Download Speed: \t %s/s ]",
		VERSIONSTRING, GUI_VERSION_STRING, Util::formatBytes(Socket::getTotalDown()).c_str(), Util::formatBytes(Socket::getTotalUp()).c_str(),
		Util::formatBytes(Util::toDouble(WGETS("dw-st"))).c_str(), Util::formatBytes(Util::toDouble(WGETS("up-st"))).c_str(),
		Util::toString((((double)Util::toDouble(WGETS("up-st"))) / ((double)Util::toDouble(WGETS("dw-st"))))).c_str(),
		Util::toString(UploadManager::getInstance()->getUploadCount()).c_str(), Util::formatBytes(UploadManager::getInstance()->getRunningAverage()).c_str(),
		Util::toString(DownloadManager::getInstance()->getDownloadCount()).c_str(), Util::formatBytes(DownloadManager::getInstance()->getRunningAverage()).c_str());
	return buf;
}

void WulforUtil::drop_combo(GtkWidget *widget, vector<pair<std::string,int> > CONTEUDO)
{
	gtk_cell_layout_clear(GTK_CELL_LAYOUT(widget));
	int cont;
	GtkListStore *list_store;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	list_store = gtk_list_store_new(1,G_TYPE_STRING);

	for (cont=0; cont < CONTEUDO.size(); cont++)
	{
		char conteude[130];
		sprintf(conteude,"%s",CONTEUDO.at(cont).first.c_str());
		gtk_list_store_append(list_store,&iter);
		gtk_list_store_set(list_store,&iter,0,conteude,-1);
	}

	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(list_store));

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget),renderer, "text", 0, NULL);

   // gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget), 0);
   gtk_combo_box_set_active(GTK_COMBO_BOX(widget),  0);

}

void WulforUtil::drop_combo(GtkWidget *widget, vector<string> CONTEUDO)
{
	gtk_cell_layout_clear(GTK_CELL_LAYOUT(widget));
	int cont;
	GtkListStore *list_store;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;

	list_store = gtk_list_store_new(1,G_TYPE_STRING);

	for (cont=0; cont < CONTEUDO.size();cont++)
	{
	char conteude[130];
	sprintf(conteude,"%s",CONTEUDO.at(cont).c_str());
	gtk_list_store_append(list_store,&iter);
	gtk_list_store_set(list_store,&iter,0,conteude,-1);
	}

	gtk_combo_box_set_model(GTK_COMBO_BOX(widget),GTK_TREE_MODEL(list_store));

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget),renderer,TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget),renderer,"text",0,NULL);

    gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(widget),0);

}

vector<std::pair<std::string,int> >& WulforUtil::getActions()
{
			if(actions.size() == 0) {
				const dcpp::Action::ActionList& a = dcpp::RawManager::getInstance()->getActions();
				for(dcpp::Action::ActionList::const_iterator i = a.begin(); i != a.end(); ++i) {
					const std::string name = (*i)->getName();
					const int Id = (*i)->getId();
					if((*i)->getEnabled())
						actions.push_back(make_pair(name,Id));
				}
			}
			return actions;
}

void WulforUtil::loadmimetypes()
{
	m_mimetyp.insert( std::pair<std::string, std::string>(".zip","application/zip"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".pdf","application/pdf"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".py","text/python"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".bin","application/octet-stream"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".iso","application/octet-stream"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".tar","application/x-tar"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".gz","application/x-tar"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mid","audio/mid"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".rmi","audio/mid"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".midi","audio/mid"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".jpeg","image/jpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".jpe","image/jpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".jpg","image/jpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".png","image/png"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".svg","image/svg+xml"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mpg","video/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mpeg","video/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mpe","video/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mov","video/quicktime"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".qt","video/quicktime"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".bmp","image/bmp"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".doc","application/msword"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".docx","application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".rtf", "application/rtf"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".xls", "application/vnd.ms-excel"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".xlsx","application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".ppt", "application/vnd.ms-powerpoint"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".gif", "image/gif"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".exe", "application/octet-stream"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".dll", "application/x-msdownload"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".css", "text/css"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".html", "text/html"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".htm", "text/html"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".txt", "text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".c","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".h","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".srt","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".sfv","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".nfo","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".lua","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".ini","text/plain"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mp3", "audio/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mp4", "audio/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".avi", "audio/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".vob", "audio/mpeg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".mkv", "video/x-matroska"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".xhtml","application/xhtml+xml"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".7z", "application/x-7z-compressed"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".wmv", "video/x-ms-wmv"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".wma", "audio/x-ms-wma"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".m3u", "audio/x-mpegurl"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".xcf", "image/x-xcf"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".chm","application/x-chm"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".rar","application/rar"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".ogg","application/ogg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".ogv","application/ogg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".oga","application/ogg"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".flac","audio/x-flac"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".wav","audio/x-wav"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".cue","application/x-cue"));
	m_mimetyp.insert( std::pair<std::string, std::string>(".rm", "application/vnd.rn-realmedia"));

}

GdkPixbuf *WulforUtil::loadIconSB(std::string ext)
{
	if(ext == "directory" || ext.empty())
	{
		GtkWidget *iwid = gtk_invisible_new ();
		GdkPixbuf *buf = gtk_widget_render_icon(iwid,GTK_STOCK_DIRECTORY,GTK_ICON_SIZE_MENU,NULL);
		return buf;
	}

	loadmimetypes();

	std::map<std::string,std::string>::iterator it = m_mimetyp.find(ext);
	if(it == m_mimetyp.end())
	{
		GtkWidget *iwid = gtk_invisible_new ();
		GdkPixbuf *buf = gtk_widget_render_icon(iwid,GTK_STOCK_FILE,GTK_ICON_SIZE_MENU,NULL);
		return buf;
	}

	GIcon *icon =g_content_type_get_icon((const gchar *)it->second.c_str());
	GtkIconTheme *theme = gtk_icon_theme_get_default ();
	GtkIconInfo *info = gtk_icon_theme_lookup_by_gicon(theme,icon,(GtkIconSize)16,GTK_ICON_LOOKUP_GENERIC_FALLBACK);
	GdkPixbuf *icon_d = gtk_icon_info_load_icon (info,NULL);
	g_object_unref(icon);
	return icon_d;

}

/* see http://svn.xiph.org/trunk/sushivision/gtksucks.c */
void WulforUtil::my_gtk_widget_remove_events (GtkWidget *widget,gint events)
{

  g_return_if_fail (GTK_IS_WIDGET (widget));

  GQuark quark_event_mask = g_quark_from_static_string ("gtk-event-mask");
  gint *eventp = g_object_get_qdata (G_OBJECT (widget), quark_event_mask);
  gint original_events = events;

  if (!eventp){
    eventp = g_slice_new (gint);
    *eventp = 0;
  }

  events = ~events;
  events &= *eventp;

  if(events)
  {
    *eventp = events;
    g_object_set_qdata (G_OBJECT (widget), quark_event_mask, eventp);
  }
  else
  {
    g_slice_free (gint, eventp);
    g_object_set_qdata (G_OBJECT (widget), quark_event_mask, NULL);
  }

  if (GTK_WIDGET_REALIZED (widget))
  {
    GList *window_list;

    if (GTK_WIDGET_NO_WINDOW (widget))
      window_list = gdk_window_get_children (widget->window);
    else
      window_list = g_list_prepend (NULL, widget->window);

    remove_events_internal (widget, original_events, window_list);

    g_list_free (window_list);
  }

  g_object_notify (G_OBJECT (widget), "events");
}

void WulforUtil::remove_events_internal (GtkWidget *widget, gint events, GList     *window_list)
{
  GList *l;

  for (l = window_list; l != NULL; l = l->next)
  {
    GdkWindow *window = l->data;
    gpointer user_data;

    gdk_window_get_user_data (window, &user_data);
    if (user_data == widget){
      GList *children;

      gdk_window_set_events (window, gdk_window_get_events(window) & (~events));

      children = gdk_window_get_children (window);
      remove_events_internal (widget, events, children);
      g_list_free (children);
    }
  }
}

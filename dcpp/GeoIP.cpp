/*
 * Copyright (C) 2001-2018 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "GeoIPc.h"

#include "File.h"
#include "format.h"
#include "SettingsManager.h"
#include "Util.h"
#include "ZUtils.h"

#include <string.h>

namespace dcpp {
using namespace std;

char *strndup(char *str, int chars)
{
    char *buffer;
    int n;

    buffer = (char *) malloc(chars +1);
    if (buffer)
    {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

static char *get_value (MMDB_lookup_result_s res, ...)
{
  MMDB_entry_data_s entry_data;
  char *value = NULL;
  int status = 0;
  va_list keys;
  va_start (keys, res);

  status = MMDB_vget_value (&res.entry, &entry_data, keys);
  va_end (keys);

  if (status != MMDB_SUCCESS)
    return NULL;

  if (!entry_data.has_data)
    return NULL;

  if (entry_data.type != MMDB_DATA_TYPE_UTF8_STRING)
    dcdebug ("Invalid data UTF8 GeoIP2 data %d:\n", entry_data.type);

  if ((value = strndup (entry_data.utf8_string, entry_data.data_size)) == NULL)
    dcdebug ("Unable to allocate buffer %s: ", strerror (errno));

  return value;
}


GeoIP::GeoIP(string&& _path) : path(move(_path)) {
	if(Util::fileExists(path) == true)
	{
		open();
	}
}

GeoIP::~GeoIP() {
	Lock l(cs);
	close();
}

static int LogCheckError(string ip,int gai,int mm_err)
{
	
	if(0 != gai)
	{
	  dcdebug("\n  Error from getaddrinfo for %s - %s\n\n",
                ip.c_str(), gai_strerror(gai));
         return 1; // Not SuccessFull       
    }	
	
	if(mm_err != MMDB_SUCCESS)
	{
		dcdebug("\n  Got an error from libmaxminddb: %s\n\n",
                MMDB_strerror(mm_err));
        return 1; // Not SuccessFull              	
	}
	return 0;
}

const string GeoIP::getCountry(const string& ip) const {
	Lock l(cs);

	int gai_error, mmdb_error;
    MMDB_lookup_result_s result =
        MMDB_lookup_string(const_cast<MMDB_s*>(&mmdb), ip.c_str(), &gai_error, &mmdb_error);
    int ret = LogCheckError(ip,gai_error,mmdb_error);
    
    if(ret == 0)
	{
		char *country = NULL, *code = NULL, *cont = NULL;
		country = get_value (result, "country", "names", "en", ( char*)0L);
		code = get_value (result, "country", "iso_code", ( char*)0L);
		cont = get_value (result,"continent", "code" ,( char*)0L);
	
		const string& setting = SETTING(COUNTRY_FORMAT);
		ParamMap params;
		params["2code"] = code;
		//		params["3code"] = forwardRet(GeoIP_code3_by_id(id)); 
		params["continent"] = cont; 
		params["engname"] = country; 
		params["name"] =  country; 
		params["officialname"] = country; 
		string rr = Util::formatParams(setting, params);
	
		//return  std::string(""+string(code)+" - "+string(country)+"");
		return rr;
	}
	return Util::emptyString;
}

const string GeoIP::getCountryAB(const string& ip) const {
    Lock l(cs);
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(const_cast<MMDB_s*>(&mmdb), ip.c_str(), &gai_error, &mmdb_error);
    int ret = LogCheckError(ip,gai_error,mmdb_error);
	if(ret == 0) {
		char *code = NULL;
		code = get_value (result, "country", "iso_code", ( char*)0L); 
		return  std::string(code);
	}
	return Util::emptyString;	
}

const string GeoIP::GetAnyInfoItem(const string ip, ...)
{
	int gai_error, mmdb_error;
    MMDB_lookup_result_s result = MMDB_lookup_string(const_cast<MMDB_s*>(&mmdb), ip.c_str(), &gai_error, &mmdb_error);
    int ret = LogCheckError(ip,gai_error,mmdb_error);
	if(ret == 0) {
		va_list keys;
		va_start (keys, ip);
		char *code = NULL;
		code = get_value (result, keys);
		va_end(keys);
		if(code != NULL) 
			return  std::string(code);
	}
	return Util::emptyString;		
}


void GeoIP::update() {
	Lock l(cs);
	close();
	open();
}

/*#ifdef _WIN32
bool GeoIP::decompress() const {
	if(File::getSize(path + ".gz") <= 0) {
		return false;
	}

	try { GZ::decompress(path + ".gz", path); }
	catch(const Exception&) { return false; }

	return true;
}
#endif*/
void GeoIP::open() {
	dcdebug("%s",path.c_str());
	int status = MMDB_open(path.c_str(), MMDB_MODE_MMAP, &mmdb);

    if (status != MMDB_SUCCESS) {
        dcdebug("\n  Can't open %s - %s\n",
                path.c_str(), MMDB_strerror(status));

        if( status == MMDB_IO_ERROR) {
            dcdebug("   IO error: %s\n", strerror(errno));
        }
        
    }
}

void GeoIP::close() {
	//MMDB_close(&mmdb);
}

} // namespace dcpp

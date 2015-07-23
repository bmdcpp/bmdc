/*
 * Copyright © 2010-2015 Mank
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
 
#ifndef _BMDC_EXPORT_MANAGER_HH
#define _BMDC_EXPORT_MANAGER_HH
#include "stdinc.h"
#include "Singleton.h"
#include "LogManager.h"
#include "TarFile.h"
#include "Util.h"
#include "format.h"
#include "RegEx.h"
#include "SettingsManager.h"
#include "Semaphore.h"
#include "Thread.h"
#include <deque>


namespace dcpp {
	
using std::deque;	
	
class ExportManager:
		public Singleton<ExportManager>,
		private Thread
{
public:
	ExportManager(): stop(false) { start();}
	virtual ~ExportManager(){ }
	void export_(const string &to, StringList &paths);
private:
	
	CriticalSection cs;
	Semaphore s;

	int run();
	void exportData(const string &path, StringList &paths);
	deque< map<string, StringList> > ofexporteddata;
	bool stop;	
};	
}
#endif			

/*
 * Copyright (C) 2011 - 2013 iceman50, iceman50@adcportal.com
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
#ifdef HAVE_LIBTAR 

#include "stdinc.h"
#include "noexcept.h"
#include "format.h"
#include "BackupManager.h"
#include "RegEx.h"
#include "SettingsManager.h"
#include "TarFile.h"

namespace dcpp {

int BackupManager::run() {
	dcdebug("BackupManager::run() start %p\n", (void*)this);
	setThreadPriority(Thread::LOW);
	
	while(true) {
		s.wait(1000);
		if(stop) {
			break;
		}
		
		//We need to prune the amount of backups, so how about the last 10 backups?
		
		Lock l(cs);
		
		const string zipFile = Util::getBackupPath() + "SettingsBackup-" + "[" + Util::getBackupTimeString() + "]" + ".tar";
		try {
			File::ensureDirectory(Util::getBackupPath());
			StringPairList files;

			const auto& paths = File::findFiles(Util::getPath(Util::PATH_USER_CONFIG), "*");
			for(auto i = paths.cbegin(); i != paths.cend(); ++i) {
					if(!Wildcard::match(Util::getFileName(*i), SETTING(BACKUP_FILE_PATTERN), ';')){
						continue;
					}
								
				files.push_back(make_pair(*i, Util::getFileName(*i)));
			}
			TarFile zip;
			zip.CreateTarredFile(zipFile,files);
			LogManager::getInstance()->message(_("Settings have been backed up!"));
			} catch (...){
				dcdebug("Exception caught");
		}
		stop = true;
	}

	dcdebug("BackupManager::run() end %p\n", (void*)this);
	stop = true;
	return 0;
}

void BackupManager::createBackup() {
	start();
}

void BackupManager::on(TimerManagerListener::Minute, uint64_t aTick) noexcept {
	uint64_t backupTime = SETTING(AUTOBACKUP_TIME) * 60;
	if(SETTING(ENABLE_AUTOBACKUP) && aTick > backupTime) {
		stop = false;
		start();
		LogManager::getInstance()->message(_("Settings files have been automatically backed up!"));
	}
}

int RestoreManager::run() {
	dcdebug("RestoreManager::run() start %p\n", (void*)this);
	setThreadPriority(Thread::LOW);
	
	while(true) {
		s.wait(1000);
		if(stop) {
			break;
		}
		
		Lock l(cs);
		
		StringList files = File::findFiles(Util::getBackupPath(), "SettingsBackup*.tar");
		auto recentBackup = files.front();
		
		try {
			TarFile tar;
			tar.DecompresTarredFile(recentBackup, Util::getPath(Util::PATH_USER_CONFIG));
			LogManager::getInstance()->message(_("Settings have been restored!"));
		} catch(...) {
			LogManager::getInstance()->message(_("SettingsBackup; failed to read from .tar!"));
			return 0;
		}
		stop = true;
	}
	
	dcdebug("RestoreManager::run() end %p\n", (void*)this);
	stop = true;
	return 0;
}

void RestoreManager::restoreBackup() {
	start();
}

}//namespace dcpp
#endif

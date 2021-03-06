/*
 * Copyright (C) 2001-2017 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_STDINC_H
#define DCPLUSPLUS_DCPP_STDINC_H

#include "compiler.h"
#include "nullptr.h"

#ifndef BZ_NO_STDIO
#define BZ_NO_STDIO 1
#endif

#ifndef DCAPI_HOST
#define DCAPI_HOST 1
#endif

#ifdef HAS_PCH

#ifdef _WIN32
#include "w.h"
#else
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <atomic>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <bzlib.h>
#include <openssl/ssl.h>

#endif

//NOTE for uint64
// see http://stackoverflow.com/questions/8132399/how-to-printf-uint64-t
// also https://sourceware.org/bugzilla/show_bug.cgi?id=15366
// and http://stackoverflow.com/questions/986426/what-do-stdc-limit-macros-and-stdc-constant-macros-mean
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <stdint.h>

// always include
#include <utility>
using std::move;

#endif // !defined(STDINC_H)

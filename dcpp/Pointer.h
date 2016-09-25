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

#ifndef DCPLUSPLUS_DCPP_POINTER_H
#define DCPLUSPLUS_DCPP_POINTER_H

#include <atomic>
#include <memory>
#include "noexcept.h"

namespace dcpp {

using std::unique_ptr;
using std::forward;

struct DeleteFunction {
	template<typename T>
	void operator()(const T& p) const { delete p; }
};

//used in QueueManager::file
struct DeleteFunction2 {
	template<typename T>
	void operator()(const T& p) const { delete p.second; }
};

template<typename T>
inline unique_ptr<T> make_unique()
{
    return unique_ptr<T>(new T);
}

template<typename T, typename A0>
inline unique_ptr<T> make_unique(A0&& a0)
{
    return unique_ptr<T>(new T(forward<A0>(a0)));
}

template<typename T, typename A0, typename A1>
inline unique_ptr<T> make_unique(A0 && a0, A1 && a1)
{
    return unique_ptr<T>(new T(forward<A0>(a0), forward<A1>(a1)));
}

template<typename T, typename A0, typename A1, typename A2>
inline unique_ptr<T> make_unique(A0 && a0, A1 && a1, A2 && a2)
{
    return unique_ptr<T>(new T(forward<A0>(a0), forward<A1>(a1), forward<A2>(a2)));
}

} // namespace dcpp

#endif // !defined(POINTER_H)

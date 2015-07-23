/*
 * Copyright (C) 2001-2015 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_SOCKET_H
#define DCPLUSPLUS_DCPP_SOCKET_H

#ifdef _WIN32
typedef int socklen_t;
typedef SOCKET socket_t;
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

typedef int socket_t;
const int INVALID_SOCKET = -1;
#define SOCKET_ERROR -1
#endif

#include "GetSet.h"
#include "Util.h"
#include "Exception.h"

#include <memory>

namespace dcpp {

class SocketException : public Exception {
public:
#ifdef _DEBUG
	SocketException(const string& aError) noexcept : Exception("SocketException: " + aError) { }
#else //_DEBUG
	SocketException(const string& aError) noexcept : Exception(aError) { }
#endif // _DEBUG

	SocketException(int aError) noexcept;
	virtual ~SocketException() throw() { }
private:
	static string errorToString(int aError) noexcept;
};

/** RAII socket handle */
class SocketHandle {
public:
	SocketHandle() : sock(INVALID_SOCKET) { }
	SocketHandle(socket_t sock) : sock(sock) { }
	~SocketHandle() { reset(); }

	operator socket_t() const { return get(); }
	SocketHandle& operator=(socket_t s) { reset(s); return *this; }

	socket_t get() const { return sock; }
	bool valid() const { return sock != INVALID_SOCKET; }
	void reset(socket_t s = INVALID_SOCKET);
private:
	socket_t sock;
};

class Socket
{
public:
	enum SocketType {
		TYPE_TCP = IPPROTO_TCP,
		TYPE_UDP = IPPROTO_UDP
	};

	explicit Socket(SocketType type) : type(type) { }

	virtual ~Socket() { }

	/**
	 * Connects a socket to an address/ip, closing any other connections made with
	 * this instance.
	 * @param aAddr Server address, in dns or xxx.xxx.xxx.xxx format.
	 * @param aPort Server port.
	 * @throw SocketException If any connection error occurs.
	 */
	virtual void connect(const string& aIp, const int16_t& aPort, const string& localPort = Util::emptyString);
	void connect(const string& aIp, uint16_t aPort, uint16_t localPort = 0) { connect(aIp, aPort == 0 ? -1 : aPort, localPort == 0 ? Util::emptyString : Util::toString(localPort)); }

	/**
	 * Same as connect(), but through the SOCKS5 server
	 */
	void socksConnect(const string& aIp, const int16_t& aPort, uint32_t timeout = 0);

	/**
	 * Sends data, will block until all data has been sent or an exception occurs
	 * @param aBuffer Buffer with data
	 * @param aLen Data length
	 * @throw SocketExcpetion Send failed.
	 */
	void writeAll(const void* aBuffer, int aLen, uint32_t timeout = 0);
	virtual int write(const void* aBuffer, int aLen);
	int write(const string& aData) { return write(aData.data(), (int)aData.length()); }
	virtual void writeTo(const string& aIp, const string& aPort, const void* aBuffer, int aLen, bool proxy = true);
	void writeTo(const string& aIp, const string& aPort, const string& aData) { writeTo(aIp, aPort, aData.data(), (int)aData.length()); }
	virtual void shutdown() noexcept;
	virtual void close() noexcept;
	void disconnect() noexcept;

	virtual bool waitConnected(int32_t millis);
	virtual bool waitAccepted(int32_t millis);

	/**
	 * Reads zero to aBufLen characters from this socket,
	 * @param aBuffer A buffer to store the data in.
	 * @param aBufLen Size of the buffer.
	 * @return Number of bytes read, 0 if disconnected and -1 if the call would block.
	 * @throw SocketException On any failure.
	 */
	virtual int read(void* aBuffer, int aBufLen);
	/**
	 * Reads zero to aBufLen characters from this socket,
	 * @param aBuffer A buffer to store the data in.
	 * @param aBufLen Size of the buffer.
	 * @param aIP Remote IP address
	 * @return Number of bytes read, 0 if disconnected and -1 if the call would block.
	 * @throw SocketException On any failure.
	 */
	virtual int read(void* aBuffer, int aBufLen, string &aIP);
	/**
	 * Reads data until aBufLen bytes have been read or an error occurs.
	 * If the socket is closed, or the timeout is reached, the number of bytes read
	 * actually read is returned.
	 * On exception, an unspecified amount of bytes might have already been read.
	 */
	int readAll(void* aBuffer, int aBufLen, uint32_t timeout = 0);

	virtual std::pair<bool, bool> wait(int32_t millis, bool checkRead, bool checkWrite);

	typedef std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> addrinfo_p;
	static string resolve(const string& aDns, int af = AF_UNSPEC) noexcept;
	addrinfo_p resolveAddr(const string& name, const uint16_t& port, int family = AF_UNSPEC, int flags = 0);

	static uint64_t getTotalDown() { return stats.totalDown; }
	static uint64_t getTotalUp() { return stats.totalUp; }

	void setBlocking(bool block) noexcept;

	string getLocalIp(socket_t sock) noexcept;
	uint16_t getLocalPort() noexcept;

	/** Binds a socket to a certain local port and possibly IP. */
	virtual int16_t listen(const int16_t& port);
	/** Accept a socket.
	@return remote port */
	virtual void accept(const Socket& listeningSocket);

	int getSocketOptInt(int option);
	void setSocketOpt(int option, int value);

	virtual bool isSecure() const noexcept { return false; }
	virtual bool isTrusted() const noexcept { return false; }
	virtual string getCipherName() const noexcept { return Util::emptyString; }
	virtual ByteVector getKeyprint() const noexcept { return ByteVector(); }
	virtual bool verifyKeyprint(const string&, bool) noexcept { return true; };

	/** When socks settings are updated, this has to be called... */
	static void socksUpdated();

	bool isV6Valid() const noexcept;

	static int getLastError();
	static string getRemoteHost(const string& aIp);

	GETSET(string, ip, Ip);
	GETSET(string, localIp4, LocalIp4);
	GETSET(string, localIp6, LocalIp6);
	GETSET(bool, v4only, V4only);
	socket_t getSock() const;
protected:
	mutable SocketHandle sock4;
	mutable SocketHandle sock6;

	SocketType type;

	class Stats {
	public:
		uint64_t totalDown;
		uint64_t totalUp;
	};
	static Stats stats;

	static sockaddr_storage udpAddr;
	static socklen_t udpAddrLen;

private:
	void socksAuth(uint32_t timeout);
	socket_t setSock(socket_t s, int af);

	// Low level interface
	socket_t create(const addrinfo& ai);
	static string resolveName(const sockaddr* sa, socklen_t sa_len, int flags = NI_NUMERICHOST);
};

} // namespace dcpp

#endif // !defined(SOCKET_H)

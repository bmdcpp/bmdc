/* 
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

/**
 *						API Stuff (just to put some things down)
 *
 * API Terms:
 *	1. Node: User of the plugin API (both plugins and the host itself).
 *	2. Hook: Generic term for message based channel between two nodes. Hook can be either inbound
 *			 (called) our outboud (listened). Inbound hooks are referred as callbacks but their
 *			 mechanics are mostly identical.
 *				- Main hook: a special hook which notifies plugins about changes in the API and
 *				  its state (separate of the hooks system).
 *				- Default hook procedure: called after subscribers have been processed, if available. 
 *
 * Host node implementation:
 *	1§ Host is expected to implement all functions pointed directly by DCCore struct, with the
 *	   notable exception of "strconv" which should be implemented to be platform, useage and
 *	   environment relevant.
 *
 *	2§ set_hook and un_hook implementations must automatically create (if missing) and free hooks from
 *	   HookID range [1, HOOK_USER] respectively. The type of any automatically created hooks should be
 *	   HOOK_EVENT.
 *
 *	3§ Host node must create the callback CALLBACK_BASE, which is the only guaranteed callback between
 *	   host node and (all of) the plugin nodes (this callback should be a hook of type HOOK_CALLBACK).
 *
 *	4§ Host is expected to implement at least one of the "common" outbound hooks, rest are optional.
 *
 *	5§ Hooks the host node chooses implement are expected to be complete or complemented, to a satisfactory
 *	   degree, by plugin for the host.
 *
 *	6§ Host is required to implement only the callback events relevant to the implemented outbound hooks.
 *	   Alternatively a plugin may complement (complete) the hosts supply of these by the means of
 *	   callback overloading.
 *
 *	7§ Any completely platform specific callbacks or hooks are considered completely optional and
 *	   plugins can be expected to handle this accordingly.
 *
 * Plugin node implementation:
 *	1§ Plugin is required to export function pluginInit which must return pointer to a hook procedure
 *	   that can handle main hook events (basic DCHOOK). Unless plugin explictly sets the common data
 *	   when subscribing to a hook or a callback all hook procedures are of the simple variant.
 *
 *	2§ Most hooks are simple blocking type, however, even an event blocked by one node will get sent
 *	   to any remaining nodes (sending process is linear, based on subscription order) though they can't
 *	   unblock the event.
 *
 *	3§ Callback hooks are overloadable and differ slightly from above. They suppport both terminating the
 *	   sending of the current event to remaining nodes as well as changing the blocking state on the fly.
 *	   This allows callbacks to be overloaded by plugins. Example: host node uses default hook procedure
 *	   to provide callbacks. Plugin subscribes to the hook and catches the event it needs then terminates
 *	   so the default hook procedure won't get processed (callbacks can be implemented by all nodes, see below).
 *
 *	4§ Plugins can also intercommunicate and depend on each other because everyone can create and call hooks freely
 *	   this also allows plugins to complement the hosts implementations of the API (as noted under Host node
 *	   implementation). However, plugin intercom is something largely independant of the host and the API itself.
 *	   To put it simply plugins have to "know each other", the API only allows for basic checking of dependencies
 *	   based on plugin GUIDs.
 *
 * Misc implementation
 *	§1 Implementations that add additional messages or types to any of the groups of constants must always aim to
 *	   preserve the existing constants and their values so that backwards compatibility is maintained.
 *
 *	§2 Any constants should not exceed the absolute minium size requirements for an integer (ie. 16bit, [-32767, 32767])
 *	   even though on most architectures today integer can be larger.
 */

#ifndef DCPLUSPLUS_DCPP_PLUGIN_DEFS_H
#define DCPLUSPLUS_DCPP_PLUGIN_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Version of the plugin api (must change every time the API has changed) */
#define DCAPI_VER				0.50

/* The earliest version of the API that this version is backwards compatible with */
#define DCAPI_COMPATIBLE_VER	0.50

#ifdef _WIN32
	#define DCAPI __stdcall
	#define DCEXP __declspec(dllexport)
	#ifdef DCAPI_HOST
	#define DCIMP __declspec(dllexport)
	#else
	#define DCIMP __declspec(dllimport)
	#endif
#else
	#define STDCALL
	#define DCAPI STDCALL
	#define DCEXP __attribute__ ((visibility("default")))
	#define DCIMP __attribute__ ((visibility("default")))
#endif

#ifndef DCAPI_HOST
/* current STLPort GIT implements this */
/*# if _MSC_VER <= 1500 && (!defined(_STLPORT_VERSION) || (_STLPORT_VERSION < 0x600))
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
# else*/
#  include <stdint.h>
//# endif
#endif

/* Hook types */
typedef enum tagHookType {
	HOOK_ID = 0,									/* Used by the message register functions, to register new hooks */
	HOOK_CALLBACK,									/* Callback (inbound) hook */
	HOOK_EVENT										/* Regular (outbound) hook */
} HookType;

/* Hook IDs */
typedef enum tagHookID {
	/* Mandatory callback hook */
	CALLBACK_BASE = 0,

	/* Common hooks (none manadatory, however, one required) */
	HOOK_PROTOCOL = 500,
	HOOK_CHAT,
	HOOK_HUBS,
	HOOK_TIMER,
	HOOK_QUEUE,
	HOOK_UI,

	/* Plugin created hooks and callbacks (HOOK_USER + n) */
	HOOK_USER = 1000
} HookID;

typedef enum tagCallbackEvent {
	/* Generic callback events */
	DBG_MESSAGE = 0,
	LOG_MESSAGE,
	GET_PATHS,

	/* Protocol callback events */
	PROTOCOL_SEND_UDP = 500,
	PROTOCOL_HUB_EMULATE_CMD,
	PROTOCOL_HUB_SEND_CMD,
	PROTOCOL_CONN_SEND_CMD,
	PROTOCOL_CONN_TERMINATE,

	/* Hub callback events */
	HUBS_CREATE_HUB = 1000,
	HUBS_DESTROY_HUB,
	HUBS_FIND_HUB,
	HUBS_SEND_CHAT,
	HUBS_SEND_PM,
	HUBS_SEND_LOCAL,

	/* Queue callback events */
	QUEUE_ADD_DL = 1500,
	QUEUE_REMOVE_DL,
	QUEUE_SET_PRIORITY,

	/* [2000, 2499] Reserved for UI callback events */ 

	/* Plugin created callback events (CALLBACK_USER + n) */
	CALLBACK_USER = 2500
} CallbackEvent;

typedef enum tagHookEvent {
	/* Main hook events (returned by pluginInit) */
	ON_INSTALL = 0,									/* Replaces ON_LOAD for the very first loading of the plugin */
	ON_UNINSTALL,									/* Replaces ON_UNLOAD when plugin is being uninstalled */
	ON_LOAD,										/* Sent after successful call to pluginInit */
	ON_UNLOAD,										/* Sent right before plugin is unloaded (no params) */
	ON_CONFIGURE,									/* Sent when user wants to configure the plugin (obj: obj: impl. dependant or NULL) */

	/* Chat hook events (HOOK_CHAT) */
	CHAT_IN = 500,									/* Incoming chat from hub (obj: ClientData) */
	CHAT_OUT,										/* Outgoing chat (obj: ClientData) */
	CHAT_PM_IN,										/* Incoming private message (obj: UserData) */
	CHAT_PM_OUT,									/* Outgoing private message (obj: UserData) */

	/* Timer hook events (HOOK_TIMER) */
	TIMER_SECOND = 1000,							/* Timer event fired once per second (tick value) */
	TIMER_MINUTE,									/* Timer event fired once per minute (tick value) */

	/* Hubs hook events (HOOK_HUBS) */
	HUB_OFFLINE = 1500,								/* Hub has just gone offline (obj: ClientData) */
	HUB_ONLINE,										/* (New) hub has just gone online (obj: ClientData) */

	/* Connections hook events (HOOK_PROTOCOL) */
	HUB_IN = 2000,									/* Incoming protocol messages from hub (obj: ClientData) */
	HUB_OUT,										/* Outgoing protocol message to hub (obj: ClientData) */
	CONN_IN,										/* Incoming client<->client protocol message (obj: ConnectionData) */
	CONN_OUT,										/* Outgoing client<->client protocol message (obj: ConnectionData) */

	/* Queue hook events (HOOK_QUEUE) */
	QUEUE_ADD = 2500,								/* (New) item has been added to download queue (obj: QueueData) */
	QUEUE_MOVE,										/* Download queue item has been moved to new location (obj: QueueData) */
	QUEUE_REMOVE,									/* Item has just been removed from download queue (obj: QueueData) */
	QUEUE_FINISHED,									/* Item has just finished downloading (obj: QueueData) */

	/* UI hook events (HOOK_UI) */
	UI_CREATED = 3000,								/* Host node UI has been created (if any, obj: impl. dependant) */
	UI_CHAT_DISPLAY,								/* Chat messages before displayed in chat (obj: StringData) */
	UI_PROCESS_CHAT_CMD,							/* Client side commands in chat (obj: CommandData) */

	/* Plugin created hook events (EVENT_USER + n) */
	EVENT_USER = 3500
} HookEvent;

/* Conversion functions */
typedef enum tagConversionType {
	CONV_TO_UTF8 = 0,								/* Convert string to UTF-8 */
	CONV_FROM_UTF8,									/* Reverse of CONV_TO_UTF8 */
	CONV_UTF8_TO_WIDE,								/* Convert UTF-8 string to wide character string */
	CONV_WIDE_TO_UTF8,								/* Reverse of CONV_UTF8_TO_WIDE */
	CONV_TO_BASE32,									/* Convert unsigned short data (uint8_t*, array) to string */
	CONV_FROM_BASE32								/* Reverse of CONV_TO_BASE32 */
} ConversionType;

typedef enum tagConfigType {
	CFG_TYPE_REMOVE = -1,							/* Config value will be removed */
	CFG_TYPE_STRING,								/* Config value is string */
	CFG_TYPE_INT,									/* Config value is 32bit integer */
	CFG_TYPE_INT64									/* Config value is 64bit integer */
} ConfigType;

typedef enum tagProtocolType {
	PROTOCOL_ADC = 0,								/* Protocol used ís ADC */
	PROTOCOL_NMDC,									/* Protocol used is NMDC */
	PROTOCOL_DHT									/* DHT node (not used, reserved) */
} ProtocolType;

typedef enum tagPathType {
	PATH_GLOBAL_CONFIG = 0,							/* Global configuration */
	PATH_USER_CONFIG,								/* Per-user configuration (queue, favorites, ...) */
	PATH_USER_LOCAL,								/* Per-user local data (cache, temp files, ...)	*/					
	PATH_RESOURCES,									/* Various resources (help files etc) */
	PATH_LOCALE										/* Translations */
} PathType;

typedef enum tagMsgType {
	MSG_CLIENT = 0,									/* General text style */
	MSG_STATUS,										/* Message in status bar */
	MSG_SYSTEM,										/* Message with system message format */
	MSG_CHEAT										/* Message with cheat message format */
} MsgType;

typedef enum tagQueuePrio {
	PRIO_DEFAULT = -1,
	PRIO_PAUSED = 0,
	PRIO_LOWEST,
	PRIO_LOW,
	PRIO_NORMAL,
	PRIO_HIGH,
	PRIO_HIGHEST
} QueuePrio;

/* Types */
typedef void *hookHandle, *dcptr_t, *subsHandle;
typedef enum tagDCBool { dcFalse = 0, dcTrue } dcBool;

/* Workaround for other bool defs */
#define Bool dcBool
#define True dcTrue
#define False dcFalse

/* Hook function prototypes */
typedef Bool (DCAPI * DCHOOK)		(uint32_t eventId, dcptr_t pData);
typedef Bool (DCAPI* DCHOOKEX)		(uint32_t eventId, dcptr_t pData, Bool* bBreak);
typedef Bool (DCAPI* DCHOOKCOMMON)	(uint32_t eventId, dcptr_t pData, void* pCommon);
typedef Bool (DCAPI* DCHOOKCOMMONEX)(uint32_t eventId, dcptr_t pData, void* pCommon, Bool* bBreak);

/* Config Value (for get_cfg/set_cfg) */
typedef struct tagConfigValue {
	ConfigType type;								/* Indicates which type value holds */
	union {
		const char* str;
		int32_t int32;
		int64_t int64;
	} value;
} ConfigValue, *ConfigValuePtr;

/* String Data (for substitutions) */
typedef struct tagStringData {
	dcptr_t object;									/* Any related object (internal, may be omitted) */
	const char* in;									/* Incoming string */
	char* out;										/* Resulting new string (allocated with DCCore::memalloc) */
} StringData, *StringDataPtr;

/* Client side chat commands */
typedef struct tagCommandData {
	dcptr_t object;									/* UserData or ClientData based on isPrivate */
	const char* command;							/* Command name */
	const char* params;								/* Command parameters passed */
	Bool isPrivate;									/* Used in a private context (private messages) */
} CommandData, *CommandDataPtr;

/* Users */
typedef struct tagUserData {
	const char* data;								/* Data sent/received */
	const char* hubHint;							/* Contains hub url to find the user from */
	const uint8_t* cid;								/* User CID (raw data, size: 192 / 8) */
	dcptr_t object;									/* The source/destination for the data */
	ProtocolType protocol;							/* The protocol used */
	Bool isOp;										/* Whether user has a key or not */
	union {
		char nick[36];								/* Users nick (only valid in NMDC) */
		uint32_t sid;								/* Users SID (only valid in ADC) */
	} uid;
} UserData, *UserDataPtr;

/* Hubs (clients) */
typedef struct tagClientData {
	const char* data;								/* Data sent/received */
	const char* url;								/* Hub url address */
	const char* ip;									/* Hub ip address */
	dcptr_t object;									/* The source/destination for the data */
	uint16_t port;									/* Hub port */
	ProtocolType protocol;							/* The protocol used */
	Bool isOp;										/* Whether we have a key on this hub or not */
	Bool isSecure;									/* True for TLS encrypted connections */
} ClientData, *ClientDataPtr;

/* Client<->client connections */
typedef struct tagConnectionData {
	const char* data;								/* The data sent/received */
	const char* ip;									/* The ip address (remote) for this connection */
	dcptr_t object;									/* The source/destination for the data */
	uint16_t port;									/* The port for this connection */
	ProtocolType protocol;							/* The protocol used */
	Bool isOp;										/* Whether user has a key or not */
	Bool isSecure;									/* True for TLS encrypted connections */
} ConnectionData, *ConnectionDataPtr;

/* Queue items and files */
typedef struct tagQueueData {
	const char* file;								/* File name */
	const char* target;								/* The *final* location for the file */
	const char* location;							/* The *current* location for the file (may be same as target) */
	const uint8_t* hash;							/* TTH hash of the file (raw data, size: 192 / 8) */
	dcptr_t object;									/* The source/destination for the data */
	uint64_t size;									/* File size (bytes) */
	Bool isFileList;								/* FileList download */
} QueueData, *QueueDataPtr;

/* Plugin meta data */
typedef struct tagMetaData { 
	const char* name;								/* Name of the plugin */
	const char* author;								/* Name/Nick of the plugin author */
	const char* description;						/* *Short* description of plugin functionality (may be multiple lines) */
	const char* web;								/* Authors website if any */
	const char* guid;								/* Plugins unique GUID */
	const char** dependencies;						/* Array of plugin dependencies */
	uint32_t numDependencies;						/* Number of plugin GUIDs in dependencies array */
	double version;									/* Plugin version */
	double apiVersion;								/* API version the plugin was compiled against */
	double compatibleVersion;						/* Earliest API version the plugin can be used with */
} MetaData, *MetaDataPtr;

/* Interaction layer */
typedef struct tagDCCore {
	/* Core API version */
	double apiVersion;

	/* Hook creation */
	hookHandle	(DCAPI *create_hook)		(HookID hookId, HookType hookType, DCHOOK defProc);
	void		(DCAPI *destroy_hook)		(hookHandle hHook);

	/* Hook interaction */
	subsHandle	(DCAPI *set_hook)			(HookID hookId, DCHOOK hookProc, void* pCommon);
	Bool		(DCAPI *call_hook)			(HookID hookId, uint32_t eventId, dcptr_t pData);
	size_t		(DCAPI *un_hook)			(subsHandle hHook);

	/* Message regitster */
	uint32_t	(DCAPI *register_message)	(HookType type, const char* name);
	uint32_t	(DCAPI *register_range)		(HookType type, const char* name, uint32_t count);
	uint32_t	(DCAPI *seek_message)		(const char* name);

	/* Settings management */
	void		(DCAPI *set_cfg)			(const char* guid, const char* setting, ConfigValuePtr val);
	Bool		(DCAPI *get_cfg)			(const char* guid, const char* setting, ConfigValuePtr val);

	/* General */
	void*		(DCAPI *memalloc)			(void* ptr, size_t bytes);
	size_t		(DCAPI *strconv)			(ConversionType type, void* dst, void* src, size_t len);
} DCCore, *DCCorePtr;

/* For callback function arguments (after long thinking and with much regret) */
typedef struct tagTextDataCond {
	const char* data;
	uint32_t cond;
	char* res;
} TextDataCond, *TextDataCondPtr;

typedef struct tagTextArgsRes {
	dcptr_t object;
	const char* data;
	dcptr_t res;
} TextArgsRes, *TextArgsResPtr;

typedef struct tagTextArgsCond {
	dcptr_t object;
	const char* data;
	uint32_t cond;
} TextArgsCond, *TextArgsCondPtr;

typedef struct tagQueueArgs {
	const char* target;
	int64_t size;
	const char* hash;
	QueueDataPtr res;
} QueueArgs, *QueueArgsPtr;

#ifndef DCAPI_HOST
/* And then some macros so no-one has to figure out what goes where above */
#define BASE_DBG_MESSAGE(core, aMsg)		{ core->call_hook(CALLBACK_BASE, DBG_MESSAGE, (dcptr_t)aMsg); }
#define BASE_LOG_MESSAGE(core, aMsg)		{ core->call_hook(CALLBACK_BASE, LOG_MESSAGE, (dcptr_t)aMsg); }
#define BASE_DESTROY_HUB(core, client)		{ core->call_hook(CALLBACK_BASE, HUBS_DESTROY_HUB, client); }
#define BASE_QUEUE_REMOVE_DL(core, aTarget)	{ core->call_hook(CALLBACK_BASE, QUEUE_REMOVE_DL, (dcptr_t)aTarget); }

#define BASE_GET_PATHS(core, type, aData) \
{ \
	TextDataCond args; \
	memset(&args, 0, sizeof(TextDataCond)); \
	args.cond = type; \
	core->call_hook(CALLBACK_BASE, GET_PATHS, &args); \
	aData = args.res; \
}

#define BASE_SEND_UDP(core, aIP, aPort, aData) \
{ \
	TextDataCond args; \
	memset(&args, 0, sizeof(TextDataCond)); \
	args.data = aIP; \
	args.cond = aPort; \
	args.res = (char*)aData; \
	core->call_hook(CALLBACK_BASE, PROTOCOL_SEND_UDP, &args); \
}

#define BASE_HUB_EMULATE_CMD(core, client, cmd) \
{ \
	TextArgsRes args; \
	memset(&args, 0, sizeof(TextArgsRes)); \
	args.object = client; \
	args.data = cmd; \
	core->call_hook(CALLBACK_BASE, PROTOCOL_HUB_EMULATE_CMD, &args); \
}

#define BASE_HUB_SEND_CMD(core, client, cmd) \
{ \
	TextArgsRes args; \
	memset(&args, 0, sizeof(TextArgsRes)); \
	args.object = client; \
	args.data = cmd; \
	core->call_hook(CALLBACK_BASE, PROTOCOL_HUB_SEND_CMD, &args); \
}

#define BASE_CONN_SEND_CMD(core, uc, cmd) \
{ \
	TextArgsRes args; \
	memset(&args, 0, sizeof(TextArgsRes)); \
	args.object = uc; \
	args.data = cmd; \
	core->call_hook(CALLBACK_BASE, PROTOCOL_CONN_SEND_CMD, &args); \
}

#define BASE_CONN_TERMINATE(core, uc, graceless) \
{ \
	TextArgsCond args; \
	memset(&args, 0, sizeof(TextArgsCond)); \
	args.object = uc; \
	args.cond = graceless; \
	core->call_hook(CALLBACK_BASE, PROTOCOL_CONN_TERMINATE, &args); \
}

#define BASE_CREATE_HUB(core, aUrl, aData) \
{ \
	TextArgsRes args; \
	memset(&args, 0, sizeof(TextArgsRes)); \
	args.data = aUrl; \
	args.res = aData; \
	core->call_hook(CALLBACK_BASE, HUBS_CREATE_HUB, &args); \
}

#define BASE_FIND_HUB(core, aUrl, aData) \
{ \
	TextArgsRes args; \
	memset(&args, 0, sizeof(TextArgsRes)); \
	args.data = aUrl; \
	args.res = aData; \
	core->call_hook(CALLBACK_BASE, HUBS_FIND_HUB, &args); \
}

#define BASE_HUB_SEND_CHAT(core, client, aMsg, third_person) \
{ \
	TextArgsCond args; \
	memset(&args, 0, sizeof(TextArgsCond)); \
	args.object = client; \
	args.data = aMsg; \
	args.cond = third_person; \
	core->call_hook(CALLBACK_BASE, HUBS_SEND_CHAT, &args); \
}

#define BASE_HUB_SEND_PM(core, ou, aMsg, third_person) \
{ \
	TextArgsCond args; \
	memset(&args, 0, sizeof(TextArgsCond)); \
	args.object = ou; \
	args.data = aMsg; \
	args.cond = third_person; \
	core->call_hook(CALLBACK_BASE, HUBS_SEND_PM, &args); \
}

#define BASE_HUB_SEND_LOCAL(core, type, client, aMsg) \
{ \
	TextArgsCond args; \
	memset(&args, 0, sizeof(TextArgsCond)); \
	args.object = client; \
	args.data = aMsg; \
	args.cond = type; \
	core->call_hook(CALLBACK_BASE, HUBS_SEND_LOCAL, &args); \
}

#define BASE_QUEUE_ADD_DL(core, aTarget, aSize, aHash, aFlags, aData) \
{ \
	QueueArgs args; \
	memset(&args, 0, sizeof(QueueArgs)); \
	args.target = aTarget; \
	args.size = aSize; \
	args.hash = aHash; \
	args.res = aData; \
	core->call_hook(CALLBACK_BASE, QUEUE_ADD_DL, &args); \
}

#define BASE_QUEUE_SET_PRIO(core, qi, aPrio) \
{ \
	TextArgsCond args; \
	memset(&args, 0, sizeof(TextArgsCond)); \
	args.object = qi; \
	args.cond = aPrio; \
	core->call_hook(CALLBACK_BASE, QUEUE_SET_PRIO, &args); \
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* !defined(DCPLUSPLUS_DCPP_PLUGIN_DEFS_H) */

/**
 * @file
 * $Id: PluginDefs.h 715 2010-09-09 12:13:53Z crise $
 */

#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import commands
import string
import re
import fileinput
import sys
from sys import platform as _platform


try:
	from bzrlib import branch
except ImportError:
	print "bzrlib not installed"

EnsureSConsVersion(0, 98, 1)

PACKAGE = 'bmdc'
CORE_PACKAGE = 'libdcpp'
LIB_UPNP = 'libminiupnpc'
LIB_NATPMP = 'libnatpmp'
BUILD_PATH = '#/build/'
BUILD_LOCALE_PATH = BUILD_PATH + 'locale/'
LIB_IS_UPNP = True
LIB_IS_NATPMP = True
LIB_IS_GEO = False
LIB_IS_TAR = False
LIB_HAVE_XATTR = False
# For Idle Detection, Enabled by defualt
LIB_HAVE_XSS = False
NEW_SETTING = False
# , '-Werror' ,'-Wfatal-errors'
#'-fno-stack-protector',
# #,'-fpermissive' ],
#,'-Weffc++'
#'-L/usr/local/lib','-L/usr/lib',
#'-ldl',
BUILD_FLAGS = {#'-Wno-unused-parameter','-Wno-unused-value',
	'common'  : ['-I#','-D_GNU_SOURCE', '-D_LARGEFILE_SOURCE', '-D_FILE_OFFSET_BITS=64', '-D_REENTRANT','-pipe','-DUSE_STACKTRACE'],
	'debug'   : ['-O1','-g', '-ggdb', '-Wall','-Wextra','-D_DEBUG'],#'-fpermissive' ,'-Wpadded'
	'release' : ['-O3', '-fomit-frame-pointer', '-DNDEBUG']
}

# ----------------------------------------------------------------------
# Function definitions
# ----------------------------------------------------------------------

def check_pkg_config(context,name):
	context.Message('Checking for %s... ' % name)
	ret = commands.getoutput('\'%s\' --version' % name)
	retval = 0
	try:
		retval = 1
	except ValueError:
		print "No pkg-config found!"

	context.Result(retval)
	return retval

def check_pkg(context, name):
	context.Message('Checking for %s... ' % name)
	ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
	context.Result(ret)
	return ret

def check_cxx_version(context, name, major, minor):
	context.Message('Checking for %s >= %d.%d...' % (name, major, minor))
	ret = commands.getoutput('%s -dumpversion' % name)
	
	retval = 0
	try:
		if ((string.atoi(ret[0]) == major and string.atoi(ret[2]) >= minor)
		or (string.atoi(ret[0]) > major)):
			retval = 1
	except ValueError:
		print "No C++ compiler found!"

	context.Result(retval)
	return retval
#
# Recursively installs all files within the source folder to target. Optionally,
# a filter function can be provided to prevent installation of certain files.
def recursive_install(env, source, target, filter = None):
	nodes = env.Glob(os.path.join(source, '*'))
	target = os.path.join(target, os.path.basename(source))

	for node in nodes:
		if node.isdir():
			env.RecursiveInstall(str(node), target, filter)
		elif filter == None or filter(node.name):
			env.Alias('install', env.Install(target, node))

def generate_message_catalogs(env):
	mo_path = os.path.join(BUILD_LOCALE_PATH, '%s', 'LC_MESSAGES', env['package'] + '.mo')
	po_files = env.Glob('po/*.po', strings = True)

	for po_file in po_files:
		basename = os.path.basename(po_file)
		lang = os.path.splitext(basename)[0]
		mo_file = mo_path % lang
		env.MoBuild(source = po_file, target = mo_file)

	return None


def check_bzr_revision(context,env):
	context.Message("Checking bzr revision...")
	revision = ''

	try:
		b = branch.Branch.open('.')
		revision = str(b.revno())
	except:
		print "failed"

	context.env['BZR_REVISION'] = revision
	env.Append( CPPDEFINES = ('-DBZR_REVISION=') + revision)
	env.Append( CPPDEFINES = ('-DBZR_REVISION_STRING=\'"') + revision+('\"\''))
	context.Result(revision)
	return revision

def replaceAll(env,file,searchExp,replaceExp):
    for line in fileinput.input(file, inplace=1):
        if searchExp in line:
            line = line.replace(searchExp,replaceExp)
        sys.stdout.write(line)

# ----------------------------------------------------------------------
# Command-line options
# ----------------------------------------------------------------------

# Parameters are only sticky from scons -> scons install, otherwise they're cleared.
if 'install' in COMMAND_LINE_TARGETS:
	vars = Variables('build/sconf/scache.conf')
else:
	vars = Variables()

vars.AddVariables(
	BoolVariable('debug', 'Compile the program with debug information', 0),
	BoolVariable('release', 'Compile the program with optimizations', 0),
	BoolVariable('profile', 'Compile the program with profiling information', 0),
	BoolVariable('libnotify', 'Enable notifications through libnotify', 1),
	BoolVariable('libtar', 'Enable Backup&Export with libtar', 1),
	BoolVariable('libappindicator', 'Enable AppIndicator Support', 0),
	BoolVariable('libxattr', 'Enable xattr support for storing calculated Hash in extended attributes of file',1),
	BoolVariable('libXss', 'Enable libxss support for AutoAway on idle feat',1),
	BoolVariable('newSettings', 'Use new Settings dialog UI',0),
	BoolVariable('useStatusIcon', 'Use Status Icon',1),
	PathVariable('PREFIX', 'Compile the program with PREFIX as the root for installation', '/usr/local/', PathVariable.PathIsDir),
	('FAKE_ROOT', 'Make scons install the program under a fake root', '')
)

# ----------------------------------------------------------------------
# Initialization
# ----------------------------------------------------------------------

env = Environment(ENV = os.environ, variables = vars, package = PACKAGE)

env['mode'] = 'debug' if env.get('debug') else 'release'
env['build_path'] = BUILD_PATH + env['mode'] + '/'

if os.environ.has_key('CXX'):
	env['CXX'] = os.environ['CXX']
	if(os.environ['CXX'] == 'clang++'):
		env.Append( CPPPATH ='/usr/include/')
		env.Append( CXXFLAGS ='-Wno-overloaded-virtual')
else:
	print 'CXX env variable is not set, attempting to use g++'
	env['CXX'] = 'g++'

if os.environ.has_key('CC'):
	env['CC'] = os.environ['CC']

if os.environ.has_key('CXXFLAGS'):
	env['CPPFLAGS'] = env['CXXFLAGS'] = os.environ['CXXFLAGS'].split()

if os.environ.has_key('LDFLAGS'):
	env['LINKFLAGS'] = os.environ['LDFLAGS'].split()

if os.environ.has_key('CFLAGS'):
	env['CFLAGS'] = os.environ['CFLAGS'].split()

if os.environ.has_key('CPPPATH'):
	env['CPPPATH'] = os.environ['CPPPATH'].split()	

env['PKG_CONFIG'] = 'pkg-config'	
if os.environ.has_key('PKG_CONFIG'):
	env['PKG_CONFIG'] = os.environ['PKG_CONFIG']		

env['CPPDEFINES'] = [] # Initialize as a list so Append doesn't concat strings

env.SConsignFile('build/sconf/.sconsign')
vars.Save('build/sconf/scache.conf', env)
Help(vars.GenerateHelpText(env))

pot_args = ['xgettext', '--default-domain=$PACKAGE', '--package-name=$PACKAGE',
		'--msgid-bugs-address=https://sourceforge.net/projects/freedcppmv/',
		'--copyright-holder=BMDC++ Team', '--add-comments=TRANSLATORS',
		'--keyword=_', '--keyword=N_', '--keyword=C_:1c,2', '--keyword=F_',
		'--keyword=P_:1,2', '--from-code=UTF-8', '--foreign-user',
		'--no-wrap', '--boost', '--sort-output', '--language=$LANGUAGE',
		'--output=$TARGET', '$SOURCES']
pot_build = Builder(action = Action([pot_args], 'Extracting messages to $TARGET from $SOURCES'))
env.Append(BUILDERS = {'PotBuild' : pot_build})

merge_pot_args = ['msgcat', '$SOURCES', '--output-file=$TARGET']
merge_pot_builder = Builder(action = Action([merge_pot_args], 'Merging pot files $SOURCES to $TARGET'))
env.Append(BUILDERS = {'MergePotFiles' : merge_pot_builder})

mo_args = ['msgfmt', '-c', '-o', '$TARGET', '$SOURCE']
mo_build = Builder(action = Action([mo_args], 'Compiling message catalog $TARGET from $SOURCES'))
env.Append(BUILDERS = {'MoBuild' : mo_build})

env.AddMethod(generate_message_catalogs, 'GenerateMessageCatalogs')
env.AddMethod(recursive_install, 'RecursiveInstall')

env.AddMethod(replaceAll,'ReplaceAll')

conf = env.Configure(
	custom_tests =
	{
		'CheckPKGConfig' : check_pkg_config,
		'CheckPKG' : check_pkg,
		'CheckCXXVersion' : check_cxx_version,
		'CheckBZRRevision' : check_bzr_revision
	},
	conf_dir = 'build/sconf',
	log_file = 'build/sconf/config.log')

# ----------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------

if not 'install' in COMMAND_LINE_TARGETS:
	if not conf.CheckCXXVersion(env['CXX'], 4, 1): 
		print 'Compiler version check failed. g++ 4.6 or later is needed'
		Exit(1)
	elif env['CXX'] == 'clang++':	
		print 'Use clang compiler'
		env.Append(CXXFLAGS = ['-I/usr/include/','-Wno-overloaded-virtual','-pthread'])
		env.Append(CFLAGS = '-I/usr/include/')
		env.Append( CPPPATH ='/usr/include/')
		env.Append( LIBS = 'pthread')
		env.Append( LINKFLAGS = '-lpthread')

	if not conf.CheckPKGConfig(env['PKG_CONFIG']):
		print '\tpkg-config not found.'
		Exit(1)

	if not conf.CheckPKG('gtk+-3.0 >= 3.00'):
		print '\tgtk+ >= 3.4 not found.'
		print '\tNote: You might have the lib but not the headers'
		Exit(1)

	if not conf.CheckHeader('time.h'):
		 Exit(1)

	if not conf.CheckHeader('signal.h'):
		Exit(1)

	if not conf.CheckHeader('unistd.h'):
		Exit(1)

	if not conf.CheckLibWithHeader('pthread', 'pthread.h', 'c'):
		print '\tpthread library not found'
		print '\tNote: You might have the lib but not the headers'
		Exit(1)

	if not conf.CheckLibWithHeader('z', 'zlib.h', 'c'):
		print '\tz library (gzip/z compression) not found'
		print '\tNote: You might have the lib but not the headers'
		Exit(1)

	if not conf.CheckLibWithHeader('bz2', 'bzlib.h', 'c'):
		print '\tbz2 library (bz2 compression) not found'
		print '\tNote: You might have the lib but not the headers'
		Exit(1)

	# This needs to be before ssl check on *BSD systems
	if not conf.CheckLib('crypto'):
		print '\tcrypto library not found'
		print '\tNote: This library may be a part of libssl on your system'
		Exit(1)

	if not conf.CheckLibWithHeader('ssl', 'openssl/ssl.h', 'c'):
		print '\tOpenSSL library (libssl) not found'
		print '\tNote: You might have the lib but not the headers'
		Exit(1)
	#Fedora dont have EC
	if not conf.CheckHeader('openssl/ec.h'):
		print '\tOpenSSL Don`t have an EC Extesions'
		conf.env.Append(CPPDEFINES = ('DHAVE_EC_CRYPTO'))

	if not conf.CheckHeader('iconv.h'):
		Exit(1)
	elif conf.CheckLibWithHeader('iconv', 'iconv.h', 'c', 'iconv(0, (const char **)0, 0, (char**)0, 0);'):
		conf.env.Append(CPPDEFINES = ('ICONV_CONST', 'const'))

	if conf.CheckHeader(['sys/types.h', 'sys/socket.h', 'ifaddrs.h', 'net/if.h']):
		conf.env.Append(CPPDEFINES = 'HAVE_IFADDRS_H')

	#assumes we had also headers..
	if conf.env.get('libxattr'):
		if conf.CheckLib('attr'):
			conf.env.Append(CPPDEFINES = 'USE_XATTR')
			LIB_HAVE_XATTR = True

	# TODO: Implement a plugin system so libnotify doesn't have compile-time dependencies
	if conf.env.get('libnotify'):
			if not conf.CheckPKG('libnotify >= 0.4.1'):
				print '\tlibnotify >= 0.4.1 not found, disabling notifications.'
				print '\tNote: You might have the lib but not the headers'
			else:
				conf.env.Append(CPPDEFINES = 'HAVE_NOTIFY')
				conf.env.ParseConfig('pkg-config --libs --cflags libnotify')
				if conf.CheckPKG('libnotify >= 0.7'):
					conf.env.Append(CPPDEFINES = 'HAVE_LIBNOTIFY_0_7')

	# Sound
	conf.env['HAVE_CANBERRA_LIB'] = 0
	if not conf.CheckPKG('libcanberra'):
		print '\tlibcanberra not found.'
		print '\tNote: You might have the lib but not the headers'
		Exit(1)
	else:
		conf.env['HAVE_CANBERRA_LIB'] = 1

	# Check for MiniUPnPc
	if not conf.CheckLib('libminiupnpc'):
		LIB_IS_UPNP = False
	# Check for natpmp
	if not conf.CheckLib('libnatpmp'):
		LIB_IS_NATPMP = False

	# GeoIp
	if conf.CheckHeader('GeoIP.h'):
		print 'Found GeoIP headers'
		conf.env.Append(CPPDEFINES = 'HAVE_GEOIPLIB')
		LIB_IS_GEO = True
	else:
		print 'Dont Found GeoIP headers or libs'
		Exit(1)

	# libtar for Backup/Restore man...
	if conf.env.get('libtar'):
		if conf.CheckHeader('libtar.h'):
			print 'Found Libtar\n'
			conf.env.Append(CPPDEFINES = 'HAVE_LIBTAR')
			LIB_IS_TAR = True
		else:
			print 'Dont Found libtar headers'
			LIB_IS_TAR = False

	# Support of appindicator # Very Experimental!
	if conf.env.get('libappindicator'):
		if conf.CheckPKG('appindicator3-0.1'):
			print 'Found appindicator3'
			conf.env.Append(CPPDEFINES = 'HAVE_APPINDCATOR')
			conf.env.ParseConfig('pkg-config --libs --cflags appindicator3-0.1')
	
	if conf.env.get('libXss'):
		if conf.CheckLibWithHeader('libXss','X11/extensions/scrnsaver.h' ,'c'):
			print 'Found Xss'
			conf.env.Append(CPPDEFINES = 'HAVE_XSSLIB')
			LIB_HAVE_XSS = True
	
	if conf.env.get('newSettings'):
		conf.env.Append(CPPDEFINES = 'USE_NEW_SETTINGS')
		NEW_SETTING = True	
	
	if conf.env.get('useStatusIcon'	):
		conf.env.Append(CPPDEFINES = 'USE_STATUSICON')
		
	conf.CheckBZRRevision(env)
	env = conf.Finish()

# ----------------------------------------------------------------------
# Compile and link flags
# ----------------------------------------------------------------------
	#_platform = 'win32'#flag for cross enable compile
	env.MergeFlags(BUILD_FLAGS['common'])
	env.MergeFlags(BUILD_FLAGS[env['mode']])

	env.Append(CXXFLAGS = '-std=c++11')
	env.Append(LIBS = ['pcre'])
	env.Append(LINKFLAGS = ['-lpcre'])

	env.Append(CPPDEFINES = ['STATICLIB'])
	env.Append(CPPPATH = '#/natpmp')
	env.Append(LIBS = 'natpmp')
	env.Append(CPPPATH = '#/miniupnp')
	env.Append(LIBS = 'miniupnpc')

	if LIB_HAVE_XATTR:
		env.Append(LIBS='attr')
		env.Append(LINKFLAGS='-lattr')
		
	if LIB_HAVE_XSS:
		env.Append(LIBS='Xss')
		env.Append(LINKFLAGS='-lXss')	

	env.ParseConfig('pkg-config --libs gtk+-3.0')

	env.Append(LIBPATH = env['build_path'] + CORE_PACKAGE)
	env.Prepend(LIBS = 'dcpp')

	if os.sys.platform == 'linux2':
		env.Append(LINKFLAGS = '-Wl,--as-needed')
		if not _platform == 'win32':
			env.Append(LIBS='dl')
		

	if os.name == 'mac' or os.sys.platform == 'darwin':
		conf.env.Append(CPPDEFINES = ('ICONV_CONST', ''))

	if os.sys.platform == 'sunos5':
		conf.env.Append(CPPDEFINES = ('ICONV_CONST', 'const'))
		env.Append(LIBS = ['socket', 'nsl'])
		
	if _platform == 'win32':
		env.Append(LIBS = ['wsock32','iphlpapi','ws2_32'])
		#env.Append(LINKFLAGS= '-Wl,-subsystem ')
		#env.Append(LDFLAGS = '-L/usr/i686-w64-mingw32/lib/')		

	if LIB_IS_GEO:
		env.Append(LINKFLAGS = '-lGeoIP')
		env.Append(LIBS = 'GeoIP')

	if LIB_IS_TAR:
		env.Append(LINKFLAGS = '-ltar')
		env.Append(LIBS = 'tar')

	if env.get('profile'):
		env.Append(CXXFLAGS = '-pg')
		env.Append(LINKFLAGS= '-pg')

	if not LIB_IS_UPNP:
		env.Append(LIBPATH = [BUILD_PATH + LIB_UPNP])
		env.Prepend(LIBS = [LIB_UPNP])

	if not LIB_IS_NATPMP:
		env.Append(LIBPATH = [BUILD_PATH + LIB_NATPMP])
		env.Prepend(LIBS = [LIB_NATPMP])

	if env.get('PREFIX'):
		data_dir = '\'\"%sshare\"\'' % env['PREFIX']
		env.Append(CPPDEFINES = ('_DATADIR', data_dir))

# ----------------------------------------------------------------------
# Build
# ----------------------------------------------------------------------
	# Build the miniupnpc library
	if not LIB_IS_UPNP:
		mini_env = env.Clone(package = LIB_UPNP)
		upnp = SConscript(dirs = 'miniupnpc', variant_dir = BUILD_PATH + LIB_UPNP, duplicate = 0, exports = {'env': mini_env})
	if not LIB_IS_NATPMP:
		natpmp_env = env.Clone(package = LIB_NATPMP)
		pmp = SConscript(dirs = 'natpmp', variant_dir = BUILD_PATH + LIB_NATPMP, duplicate = 0, exports = { 'env': natpmp_env })

	# Build the dcpp library
	dcpp_env = env.Clone(package = CORE_PACKAGE)
	libdcpp = SConscript(dirs = 'dcpp', variant_dir = env['build_path'] + CORE_PACKAGE, duplicate = 0, exports = {'env': dcpp_env})

	# Build the GUI
	ui_env = env.Clone()
	glade_pot_file = SConscript(dirs = 'ui', variant_dir = env['build_path'] + 'ui', duplicate = 0, exports = {'env': ui_env})
	if NEW_SETTING:
		settings_files = SConscript(dirs = 'settings', variant_dir = env['build_path']+'settings', duplicate = 0, exports = { 'env': ui_env})
	(linux_pot_file, obj_files) = SConscript(dirs = 'linux', variant_dir = env['build_path'] + 'gui', duplicate = 0, exports = {'env': ui_env})

	# Create the executable
	if not LIB_IS_UPNP and not LIB_IS_NATPMP and NEW_SETTING:
		env.Program(target = PACKAGE, source = [libdcpp, upnp, pmp, settings_files, obj_files])
	elif not LIB_IS_UPNP and NEW_SETTING:
		env.Program(target = PACKAGE, source = [libdcpp, upnp,settings_files, obj_files])
	elif not LIB_IS_NATPMP and NEW_SETTING:
		env.Program(target = PACKAGE, source = [libdcpp, pmp,settings_files, obj_files])
	elif NEW_SETTING:
		env.Program(target = PACKAGE, source = [libdcpp,settings_files, obj_files])
	elif not NEW_SETTING and not LIB_IS_UPNP and not LIB_IS_NATPMP:
		env.Program(target = PACKAGE, source = [libdcpp,obj_files])	
	elif not NEW_SETTING and not LIB_IS_UPNP:
		env.Program(target = PACKAGE, source = [libdcpp,upnp,obj_files])
	elif not NEW_SETTING and not LIB_IS_NATPMP:
		env.Program(target = PACKAGE, source = [libdcpp,pmp,obj_files])
	elif not NEW_SETTING:
		env.Program(target = PACKAGE, source = [libdcpp,obj_files])	
	else:
		env.Program(target = PACKAGE, source = [libdcpp,obj_files])		

	# i18n
	env.MergePotFiles(source = [glade_pot_file, linux_pot_file], target = 'po/%s.pot' % PACKAGE)
	env.GenerateMessageCatalogs()

	# Build source files followed by everything else
	Default(PACKAGE, '.')

# ----------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------

else:

	glade_files = env.Glob('ui/*.ui')
	text_files = env.Glob('*.txt')
	prefix = env['FAKE_ROOT'] + env['PREFIX']
	shell_files = env.Glob('extensions/Scripts/*.sh')
	py_files = env.Glob('extensions/Scripts/*.py')
	country_files = env.Glob('country/*.png')
	info_image_files = env.Glob('info/*.png')
	desktop_file = os.path.join('data', PACKAGE + '.desktop')

	env.ReplaceAll(desktop_file,"/usr/share/",prefix+"share/")
	
	
	app_icon_filter = lambda icon: os.path.splitext(icon)[0] == PACKAGE
	regular_icon_filter = lambda icon: os.path.splitext(icon)[0] != PACKAGE

	env.RecursiveInstall('icons/hicolor', os.path.join(prefix, 'share', 'icons'), app_icon_filter)
	env.RecursiveInstall('icons/hicolor', os.path.join(prefix, 'share', PACKAGE, 'icons'), regular_icon_filter)
	env.RecursiveInstall(BUILD_LOCALE_PATH, os.path.join(prefix, 'share', 'locale'))
	env.RecursiveInstall('emoticons', os.path.join(prefix, 'share', PACKAGE))

	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', PACKAGE, 'ui'), source = glade_files))
	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', 'doc', PACKAGE), source = text_files))
	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', 'applications'), source = desktop_file))
	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', PACKAGE, 'extensions/Scripts'), source = shell_files))
	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', PACKAGE, 'extensions/Scripts'), source = py_files))
	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', PACKAGE, 'country'), source = country_files))
 	env.Alias('install', env.Install(dir = os.path.join(prefix, 'share', PACKAGE, 'info'), source = info_image_files))
	env.Alias('install', env.Install(dir = os.path.join(prefix, 'bin'), source = PACKAGE))


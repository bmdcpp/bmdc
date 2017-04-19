set(DEPENDENCIES
	natpmp
	dcpp
)

set(SOURCES
	AboutConfig.cc 
AboutConfigFav.cc 
adlsearch.cc 
bacon-message-connection.cc 
bookentry.cc 
cmddebug.cc 
detectiontab.cc 
dialogentry.cc 
diskinfo.cc 
downloadqueue.cc 
emoticons.cc 
emoticonsdialog.cc 
entry.cc 
exportimport.cc 
FavoriteHubDialog.cc 
favoritehubs.cc 
favoriteusers.cc 
finishedtransfers.cc 
freespace.cc 
fsusage.cc 
genres.cc 
hashdialog.cc 
hub.cc 
ignoremenu.cc 
IgnoreTempManager.cc 
mainwindow.cc 
notepad.cc 
notify.cc 
previewmenu.cc 
privatemessage.cc 
publichubs.cc 
recenthub.cc 
search.cc 
SearchEntry.cc 
searchspy.cc 
settingsdialog.cc 
settingsmanager.cc 
sharebrowser.cc 
ShellCommand.cc 
sound.cc 
stacktrace.cc 
System.cc 
transfers.cc 
treeview.cc 
uploadqueue.cc 
UserCommandMenu.cc 
wulfor.cc 
wulformanager.cc 
WulforUtil.cc 

)

set(HEADERS
	AboutConfigFav.hh 
AboutConfig.hh 
adlsearch.hh 
bacon-message-connection.hh 
bookentry.hh 
cmddebug.hh 
detectiontab.hh 
dialogentry.hh 
diskinfo.hh 
downloadqueue.hh 
emoticonsdialog.hh 
emoticons.hh 
entry.hh 
exportimport.hh 
FavoriteHubDialog.hh 
favoritehubs.hh 
favoriteusers.hh 
finishedtransfers.hh 
func.hh 
gtk-fixies.hh 
hashdialog.hh 
hub.hh 
ignoremenu.hh 
IgnoreTempManager.hh 
mainwindow.hh 
message.hh 
notepad.hh 
notify.hh 
previewmenu.hh 
privatemessage.hh 
publichubs.hh 
recenthub.hh 
SearchEntry.hh 
search.hh 
searchspy.hh 
settingsdialog.hh 
settingsmanager.hh 
sharebrowser.hh 
ShellCommand.hh 
sound.hh 
Splash.hh 
stacktrace.hh 
System.hh 
transfers.hh 
treeview.hh 
uploadqueue.hh 
UserCommandMenu.hh 
version.hh 
wulformanager.hh 
WulforUtil.hh 

)

add_executable(bmdc 
	${CMAKE_SOURCE_DIR}/natpmp
	${SOURCES}
	${HEADERS}
)
add_dependencies(bmdc
	${DEPENDENCIES}
)

target_compile_definitions(bmdc PRIVATE
-D_DATADIR=\"${SHARE_INSTALL_PREFIX}\"
)

target_compile_options(bmdc PUBLIC
	${CANBERRA_CFLAGS}
	${GTK_PLATFORM_INCLUDE_DIRS}
	${MINIUPNP_CFLAGS}
	${OPENSSL_CFLAGS}
	${GEOIP_CFLAGS}
)

target_include_directories(bmdc PUBLIC
	${CMAKE_BINARY_DIR}
	${CMAKE_BINARY_DIR}/src
	${CMAKE_SOURCE_DIR}/dcpp
	${CMAKE_CURRENT_BINARY_DIR}
	${CANBERRA_INCLUDE_DIRS}
	${GTK_PLATFORM_INCLUDE_DIRS}
	${MINIUPNP_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIRS}
	${GEOIP_INCLUDE_DIRS}
)

target_link_libraries(bmdc
	${DEPENDENCIES}
	${CANBERRA_LDFLAGS}
	${GTK_PLATFORM_LDFLAGS}
	${MINIUPNP_LIBRARY}
	${GEOIP_LIBRARIES}
	${PCRE_LIBRARY}
	${OPENSSL_LIBRARIES}
	${BZIP2_LIBRARIES}
	${ZLIB_LIBRARIES}
	natpmp
)
set_property (TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}")

install(TARGETS bmdc
	DESTINATION ${BIN_INSTALL_DIR}
)

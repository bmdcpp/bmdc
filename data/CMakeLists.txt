set(filedeps)
set(desktopdir ${SHARE_INSTALL_PREFIX}/applications)

install(FILES ${CMAKE_SOURCE_DIR}/data/bmdc.desktop
	DESTINATION ${desktopdir}
)

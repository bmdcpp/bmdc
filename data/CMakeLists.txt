set(filedeps)
set(desktopdir ${SHARE_INSTALL_PREFIX}/applications)

#list(APPEND filedeps ${CMAKE_CURRENT_BINARY_DIR}/bmdc.desktop)

install(FILES ${CMAKE_SOURCE_DIR}/data/bmdc.desktop
	DESTINATION ${desktopdir}
)

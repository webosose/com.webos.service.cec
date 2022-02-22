include(FindPkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})

pkg_check_modules(LS2 REQUIRED luna-service2++)
include_directories(${LS2_INCLUDE_DIRS})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})

pkg_check_modules(PMLOGLIB_CPP REQUIRED PmLogLibCpp)
include_directories(${PMLOGLIB_CPP_INCLUDE_DIRS})

add_executable(${CMAKE_PROJECT_NAME} Main.cpp
                Logger.cpp
		CecAPI.cpp
                )

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${GLIB2_CFLAGS_OTHER})
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${LS2_CFLAGS_OTHER})
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${PBNJSON_CPP_CFLAGS_OTHER})
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC ${PMLOGLIB_CPP_CFLAGS_OTHER})

target_link_libraries(${CMAKE_PROJECT_NAME} ${GLIB2_LDFLAGS} ${LS2_LDFLAGS} ${PBNJSON_CPP_LDFLAGS} ${PMLOGLIB_CPP_LDFLAGS})

install(TARGETS ${CMAKE_PROJECT_NAME}
        DESTINATION sbin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE)

webos_build_system_bus_files()
webos_build_configured_file(files/systemd/com.webos.service.cec.service BASE_LIBDIR systemd/system)

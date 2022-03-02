# Copyright (c) 2022 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})

pkg_check_modules(LS2 REQUIRED luna-service2++)
include_directories(${LS2_INCLUDE_DIRS})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})

if (USE_PMLOG)
    pkg_check_modules(PMLOGLIB REQUIRED PmLogLibCpp)
    include_directories(${PMLOGLIB_INCLUDE_DIRS})

    webos_add_compiler_flags(ALL ${PMLOGLIB_CFLAGS_OTHER} -DUSE_PMLOG)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories( ${CMAKE_SOURCE_DIR}/include)

file(GLOB CEC_SRC
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${CEC_SRC})

set(CEC_LIBS
    ${LS2_LDFLAGS}
    ${GLIB2_LDFLAGS}
    ${PMLOGLIB_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
    -lpthread
    -Wl,--no-undefined
    )

target_link_libraries(${CMAKE_PROJECT_NAME} ${CEC_LIBS})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})

webos_build_system_bus_files()
webos_build_configured_file(files/systemd/com.webos.service.cec.service BASE_LIBDIR systemd/system)

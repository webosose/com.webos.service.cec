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
# Author(s)    : Manjuraehmad Momin
# Email ID.    : manjuraehmad.momin@lge.com


cmake_minimum_required (VERSION 3.0)

project (cec VERSION 1.0.0 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 11)
# for making available config.h for other source codes
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h"
    @ONLY )
include_directories(include)

include(webOS/webOS)

webos_modules_init(1 0 0 QUALIFIER RC4)
webos_component(1 0 0)

add_subdirectory(src)

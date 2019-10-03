############################################################################
# Copyright (c) 2009-2019 Brian Waters
# Copyright (c) 2019 Sprint
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
############################################################################

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libepc, 1.0)
dnl AC_PROG_CC
AC_PROG_CXX

dnl AC_CANONICAL_SYSTEM

dnl Initialize automake
AM_INIT_AUTOMAKE

dnl this allows us specify individual liking flags for each target
AM_PROG_CC_C_O 

dnl Initialize Libtool
dnl LT_INIT

dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
dnl AC_PROG_LIBTOOL

AC_PROG_RANLIB

AC_CONFIG_FILES(Makefile
                src/Makefile
                include/Makefile
		exampleProgram/Makefile)

#AC_CONFIG_COMMANDS([submodules],[git submodule update --init --recursive])
AC_CONFIG_COMMANDS([submodules],[git submodule update --init])
AC_CONFIG_COMMANDS([rapidjson],[
    if [[ ! -d modules/rapidjson/build ]]; then
        mkdir -p modules/rapidjson/build
        pushd modules/rapidjson/build
        cmake ..
        make
        sudo make install
        popd
    fi
])
AC_CONFIG_COMMANDS([spdlog],[
    if [[ ! -d modules/spdlog/build ]]; then
	mkdir -p modules/spdlog/build
	pushd modules/spdlog/build
	cmake ..
        make
	sudo make install
	popd
    fi
])
AC_CONFIG_COMMANDS([c-ares],[
    if [[ ! -f modules/c-ares/configure ]]; then
        pushd modules/c-ares
        ./buildconf
        ./configure
        make
        sudo make install
        popd
    fi
])
AC_CONFIG_COMMANDS([pistache],[
    if [[ ! -d modules/pistache/build ]]; then
        mkdir -p modules/pistache/build
        pushd modules/pistache/build
        rm -rf *
        cmake ..
        make
        sudo make install
        popd
    fi
])
AC_CONFIG_COMMANDS([freeDiameter],[
    if [[ ! -d modules/freeDiameter/build ]]; then
        mkdir -p modules/freeDiameter/build
        pushd modules/freeDiameter/build
        cmake ..
        awk '{if (/^DISABLE_SCTP/) gsub(/OFF/, "ON"); print}' CMakeCache.txt > tmp && mv tmp CMakeCache.txt
        make
        sudo make install
        popd
    fi
])
AC_CONFIG_COMMANDS([bzip2],[
    if [[ ! -f modules/bzip2/libbz2.a ]]; then
        pushd modules/bzip2
        make
        sudo make install
        popd
    fi
])

AC_OUTPUT

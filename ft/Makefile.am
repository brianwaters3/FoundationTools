############################################################################
# Copyright (c) 2009-2019 Brian Waters
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

#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example #usr#lib
lib_LIBRARIES = libft.a

#######################################
# Build information for each library

# Sources for ft
libft_a_SOURCES = ftbase.cpp ftbzip2.cpp ftcbuf.cpp ftdir.cpp fterror.cpp ftgetopt.cpp fthash.cpp ftlogger.cpp ftmessage.cpp ftpath.cpp ftqbase.cpp ftqpriv.cpp ftqpub.cpp ftshmem.cpp ftsocket.cpp ftstatic.cpp ftstring.cpp ftsynch.cpp fttbase.cpp ftthread.cpp fttime.cpp fttimer.cpp fttq.cpp ftutility.cpp

# Linker options libTestProgram
#libft_a_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libft_a_CPPFLAGS = -Og -std=c++11 -I$(top_srcdir)#include#ft -I$(top_srcdir)#modules#rapidjson#include

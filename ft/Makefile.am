#########################################################################
## (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. ##
#########################################################################

#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example #usr#lib
lib_LIBRARIES = libft.a

#######################################
# Build information for each library

# Sources for ft
libft_a_SOURCES = ftbase.cpp ftbzip2.cpp ftcbuf.cpp ftdir.cpp fterror.cpp fthash.cpp ftlogger.cpp ftmessage.cpp ftpath.cpp ftqbase.cpp ftqpriv.cpp ftqpub.cpp ftshmem.cpp ftsocket.cpp ftstatic.cpp ftstring.cpp ftsynch.cpp fttbase.cpp ftthread.cpp fttime.cpp fttimer.cpp fttq.cpp ftutility.cpp

# Linker options libTestProgram
#libft_a_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libft_a_CPPFLAGS = -I$(top_srcdir)#include#ft


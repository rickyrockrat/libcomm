# this is example-file: src/bin/Makefile.am

# additional include paths necessary to compile the C++ programs
#AM_CXXFLAGS = -I$(top_srcdir)/src @AM_CXXFLAGS@

###############################################################################
# THE PROGRAMS TO BUILD
###############################################################################

# the program to build (the names of the final binaries)
bin_PROGRAMS = libcomm_test

# list of sources for the 'helloWorld' binary
libcomm_test_SOURCES =    \
                        test_libcomm.cpp \
                        test_libcomm_testautoser.h \
                        test_libcomm_testautoser.cpp

# the additional libraries needed to link helloWorld
libcomm_test_LDADD =  $(top_builddir)/src/libcomm.la $(AM_LDFLAGS)

###############################################################################

# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
#AM_CXXFLAGS = -I$(top_srcdir)/src @AM_CXXFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

# the library names to build (note we are building static libs only)
lib_LTLIBRARIES = libcomm.la

# where to install the headers on the system
libcomm_ladir = $(includedir)/libcomm

# the list of header files that belong to the library (to be installed later)
libcomm_la_HEADERS =    \
                       string_serializable.h \
                       vector_serializable.h \
                       buffer_serializable.h\
                       map_serializable.h \
                       simple_serializable.h \
                       auto_serializable.h \
                       null_placeholder.h




# the sources to add to the library and to add to the source distribution
libcomm_la_SOURCES =   \
                      $(libcomm_la_HEADERS) \
                      string_serializable.cpp \
                      vector_serializable.cpp \
                      buffer_serializable.cpp\
                      map_serializable.cpp \
                      simple_serializable.cpp \
                      auto_serializable.cpp \
                      null_placeholder.cpp

###############################################################################


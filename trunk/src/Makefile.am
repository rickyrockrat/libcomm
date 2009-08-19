# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
#AM_CXXFLAGS = -I$(top_srcdir)/src @AM_CXXFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

SUBDIRS = structs

# the library names to build (note we are building static libs only)
lib_LTLIBRARIES = libcomm.la

# where to install the headers on the system
libcomm_ladir = $(includedir)/libcomm

libcomm_structs_ladir = $(includedir)/libcomm/structs

# the list of header files that belong to the library (to be installed later)
libcomm_la_HEADERS =    \
                       exception.h \
                       types_utils.h \
                       serializable.h \
                       net_message.h \
                       net_address.h \
                       net_socket.h \
                       tcp_socket.h \
                       udp_socket.h \
                       file.h\
                       stream.h\
                       input_stream.h\
                       output_stream.h\
                       serialization_manager.h \
                       thread.h \
                       mutex.h \
                       condition.h \
                       timer.h \
                       logger.h \
                       participant.h \
                       config_loader.h \
                       libcomm_structs.h \
                       libcomm.h



libcomm_structs_la_HEADERS =    \
                       structs/string_serializable.h \
                       structs/vector_serializable.h \
                       structs/buffer_serializable.h\
                       structs/map_serializable.h \
                       structs/simple_serializable.h \
                       structs/auto_serializable.h \
                       structs/null_placeholder.h

# the sources to add to the library and to add to the source distribution
libcomm_la_SOURCES =   \
                      $(libcomm_a_HEADERS) \
                      $(libcomm_structs_a_HEADERS) \
                      exception.cpp \
                      types_utils.cpp \
                      serializable.cpp \
                      net_message.cpp \
                      net_address.cpp \
                      net_socket.cpp \
                      tcp_socket.cpp \
                      udp_socket.cpp \
                      file.cpp\
                      stream.cpp\
                      input_stream.cpp\
                      output_stream.cpp\
                      serialization_manager.cpp \
                      thread.cpp \
                      mutex.cpp \
                      condition.cpp \
                      timer.cpp \
                      logger.cpp \
                      participant.cpp \
                      config_loader.cpp \
                      libcomm_structs.cpp \
                      libcomm.cpp \
                      structs/string_serializable.cpp \
                      structs/vector_serializable.cpp \
                      structs/buffer_serializable.cpp\
                      structs/map_serializable.cpp \
                      structs/simple_serializable.cpp \
                      structs/auto_serializable.cpp \
                      structs/null_placeholder.cpp

###############################################################################


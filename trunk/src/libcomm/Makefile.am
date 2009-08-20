AM_CXXFLAGS = -I$(top_srcdir)/src @AM_CXXFLAGS@

SUBDIRS = structs

lib_LTLIBRARIES = libcomm.la

libcomm_ladir = $(includedir)/libcomm

libcomm_structs_ladir = $(includedir)/libcomm/structs

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
                       structs/set_serializable.h\
                       structs/multiset_serializable.h\
                       structs/map_serializable.h \
                       structs/multimap_serializable.h \
                       structs/simple_serializable.h \
                       structs/auto_serializable.h \
                       structs/null_placeholder.h

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
                      structs/set_serializable.cpp \
                      structs/multiset_serializable.cpp \
                      structs/map_serializable.cpp \
                      structs/multimap_serializable.cpp \
                      structs/simple_serializable.cpp \
                      structs/auto_serializable.cpp \
                      structs/null_placeholder.cpp

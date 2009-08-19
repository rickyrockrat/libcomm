AM_CXXFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = myftp.cpp\
             ping.cpp

bin_PROGRAMS = libcomm_test

libcomm_test_SOURCES =    \
                        test_libcomm.cpp \
                        test_libcomm_testautoser.h \
                        test_libcomm_testautoser.cpp

libcomm_test_LDADD =  $(top_builddir)/src/libcomm/libcomm.la $(AM_LDFLAGS)

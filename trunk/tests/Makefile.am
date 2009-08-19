EXTRA_DIST = myftp.cpp ping.cpp
bin_PROGRAMS = libcomm_test

libcomm_test_SOURCES =    \
                        test_libcomm.cpp \
                        test_libcomm_testautoser.h \
                        test_libcomm_testautoser.cpp

libcomm_test_LDADD =  $(top_builddir)/src/libcomm.la $(AM_LDFLAGS)

include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libValve.la
libValve_la_SOURCES=Valve.cpp
libValve_la_LIBADD= -lpthread ../json/libjson.la ../common/libcommon.la

lib_LTLIBRARIES+=libgenerator.la
libgenerator_la_SOURCES=TaskGenerator.cpp HotelGenerator.cpp FlightGenerator.cpp TrainGenerator.cpp
libgenerator_la_LIBADD=../common/libcommon.la libValve.la -L/usr/lib64/mysql/ -lmysqlclient -lgomp

bin_PROGRAMS+=generator
generator_SOURCES=main.cpp
generator_LDADD=-lgenerator libgenerator.la

AM_CPPFLAGS+=-DSS_DOCID_BITS=256

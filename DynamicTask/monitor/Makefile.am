include $(top_srcdir)/common.mk

bin_PROGRAMS=
lib_LIBRARIES=
lib_LTLIBRARIES=

lib_LTLIBRARIES+=libmonitor.la
libmonitor_la_SOURCES=FlightMonitor.cpp RoomMonitor.cpp TrainMonitor.cpp UseMysql.cpp BaseMonitor.cpp 
libmonitor_la_LIBADD=../key/libkey.la ../json/libjson.la ../common/libcommon.la -L/usr/lib64/mysql/ -lmysqlclient

bin_PROGRAMS+=monitor
monitor_SOURCES=main.cpp
monitor_LDADD=-lmonitor libmonitor.la

AM_CPPFLAGS+=-DSS_DOCID_BITS=256

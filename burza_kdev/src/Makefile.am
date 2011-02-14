bin_PROGRAMS = burza_kdev

burza_kdev_SOURCES = bdialogs.cpp burza.cpp burza_kdev.cc convfile.cpp \
	convfncs.cpp log.cpp main_window.cc main_window_glade.cc

noinst_HEADERS = burza.h convfile.h convfncs.h log.h main_window.hh \
	main_window_glade.hh

AM_CXXFLAGS = @CXXFLAGS@ @GTKMM_CFLAGS@ 

burza_kdev_LDADD = @LIBS@ @GTKMM_LIBS@ 

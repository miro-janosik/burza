bin_PROGRAMS = burza_kdev

burza_kdev_SOURCES = bdialogs.cpp burza.cpp burza_kdev.cc convfile.cpp \
	convfncs.cpp excelwrap.cpp log.cpp main_window.cc main_window_glade.cc odswrap.cpp

noinst_HEADERS = bdialogs.h burza.h convfile.h convfncs.h excelwrap.h log.h \
	main_window.hh main_window_glade.hh odswrap.h table.h

AM_CXXFLAGS = @CXXFLAGS@ @GTKMM_CFLAGS@ 

burza_kdev_LDADD = @LIBS@ @GTKMM_LIBS@ 

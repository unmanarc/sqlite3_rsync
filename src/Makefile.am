bin_PROGRAMS=sqlite3_rsync
sqlite3_rsync_SOURCES=ts/tsheap.cpp ts/tsuint32.cpp helpers/helper_system.cpp helpers/helper_string.cpp helpers/helper_nodes.cpp db/dbquery.cpp db/dbqueryconstructor.cpp db/sqlitedbconnector.cpp main.cpp sqlite3syncclient.cpp xsockets/xsocket_tcp.cpp xsockets/xsocket.cpp xsockets/xsocketinterface.cpp sqlite3syncserver.cpp

if DEBUG
AM_CFLAGS = -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = -O2
AM_CXXFLAGS = -O2
endif
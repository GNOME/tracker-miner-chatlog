libexec_PROGRAMS = tracker-miner-chatlog

tracker_miner_chatlog_SOURCES =	\
	entity.c		\
	entity.h		\
	main.c

tracker_miner_chatlog_CPPFLAGS =	\
    -DG_LOG_DOMAIN=\"Tmc\"	\
    -I$(top_srcdir)/src		\
    $(DEPS_CFLAGS)

tracker_miner_chatlog_LDADD =	\
    $(DEPS_LIBS)

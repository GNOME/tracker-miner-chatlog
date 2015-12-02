libexec_PROGRAMS = tracker-miner-chatlog

tracker_miner_chatlog_SOURCES =	\
	contact.c		\
	contact.h		\
	entity.c		\
	entity.h		\
	room.c			\
	room.h			\
	main.c

tracker_miner_chatlog_CPPFLAGS =	\
    -DG_LOG_DOMAIN=\"Tmc\"	\
    -I$(top_srcdir)/src		\
    $(DEPS_CFLAGS)

tracker_miner_chatlog_LDADD =	\
    $(DEPS_LIBS)

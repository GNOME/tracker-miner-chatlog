libexec_PROGRAMS = tracker-miner-chatlog

tracker_miner_chatlog_SOURCES =	\
	client-factory.c	\
	client-factory.h	\
	contact.c		\
	contact.h		\
	conversation.c		\
	conversation.h		\
	entity.c		\
	entity.h		\
	entity-pool.c		\
	entity-pool.h		\
	helpers.c		\
	helpers.h		\
	room.c			\
	room.h			\
	text-channel.c		\
	text-channel.h		\
	text-event.c		\
	text-event.h		\
	main.c

tracker_miner_chatlog_CPPFLAGS =	\
    -DG_LOG_DOMAIN=\"Tmc\"	\
    -I$(top_srcdir)/src		\
    $(DEPS_CFLAGS)

tracker_miner_chatlog_LDADD =	\
    $(DEPS_LIBS)

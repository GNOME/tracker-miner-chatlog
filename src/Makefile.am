libexec_PROGRAMS = tracker-miner-chatlog

if HAVE_MIGRATION
migration_sources =		\
	logger-dumper.c		\
	logger-dumper.h
endif

tracker_miner_chatlog_SOURCES =	\
	$(migration_sources)	\
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
	miner.c			\
	miner.h			\
	observer.c		\
	observer.h		\
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
    $(LOGGER_CFLAGS)		\
    $(DEPS_CFLAGS)

tracker_miner_chatlog_LDADD =	\
    $(LOGGER_LIBS)		\
    $(DEPS_LIBS)

# DBus service
service_in_files = \
	org.freedesktop.Tracker1.Miner.ChatLog.service.in

%.service: %.service.in
	@sed -e "s|@libexecdir[@]|${libexecdir}|" $< > $@

servicedir = $(datadir)/dbus-1/services
service_DATA = \
	org.freedesktop.Tracker1.Miner.ChatLog.service

# Miner description
desktop_in_in_files = \
	tracker-miner-chatlog.desktop.in.in
desktop_in_files = \
	tracker-miner-chatlog.desktop.in

@INTLTOOL_DESKTOP_RULE@

%.desktop.in: %.desktop.in.in
	@sed -e "s|@datadir[@]|$(datadir)|" $< > $@

miner_descdir = $(datadir)/tracker/miners
miner_desc_DATA = \
	tracker-miner-chatlog.desktop


EXTRA_DIST = \
	$(service_in_files) \
	$(desktop_in_in_files)

CLEANFILES = \
	$(service_DATA) \
	$(desktop_in_files) \
	$(miner_desc_DATA)


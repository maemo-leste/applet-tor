AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS  = -I m4

SUBDIRS = \
	status-applet \
	data

install-exec-hook:
	find $(DESTDIR) -type f -name \*-tor.la -delete

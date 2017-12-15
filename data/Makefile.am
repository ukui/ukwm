desktopfiles_in_files = \
	ukwm.desktop.in
desktopfilesdir = $(datadir)/applications
desktopfiles_DATA = $(desktopfiles_in_files:.desktop.in=.desktop)

%.desktop:%.desktop.in
	$(AM_V_GET) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

xmldir = $(GNOME_KEYBINDINGS_KEYSDIR)
xml_DATA = \
        50-ukwm-navigation.xml \
        50-ukwm-system.xml \
        50-ukwm-windows.xml

gschema_in_files = \
	org.ukui.ukwm.gschema.xml.in	\
	org.ukui.ukwm.wayland.gschema.xml.in
gsettings_SCHEMAS = $(gschema_in_files:.xml.in=.xml)

%.gschema.xml: %.gschema.xml.in Makefile
	$(AM_V_GEN) sed -e 's|@GETTEXT_DOMAIN[@]|$(GETTEXT_PACKAGE)|g' \
	$< > $@ || rm $@

@GSETTINGS_RULES@

convertdir = $(datadir)/GConf/gsettings
convert_DATA = ukwm-schemas.convert

CLEANFILES = \
	$(desktopfiles_DATA) \
	$(gsettings_SCHEMAS)

EXTRA_DIST = \
	$(convert_DATA) \
	$(desktopfiles_in_files) \
	$(gschema_in_files) \
	$(xml_DATA)

/*
 * Copyright (c) 2020-2021 Ivan Jelincic <parazyd@dyne.org>
 * Copyright (c) 2015      Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This file is part of tor-network-applet
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dbus/dbus-glib-lowlevel.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <libhildondesktop/libhildondesktop.h>
#include <libosso.h>

/* Use this for debugging */
#include <syslog.h>
#define status_debug(...) syslog(1, __VA_ARGS__)

#define STATUS_APPLET_TOR_TYPE (status_applet_tor_get_type())
#define STATUS_APPLET_TOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		STATUS_APPLET_TOR_TYPE, StatusAppletTor))

#define SETTINGS_RESPONSE -69

#define GC_TOR  "/system/maemo/tor"
#define GC_CONF GC_TOR"/configs"

typedef struct _StatusAppletTor StatusAppletTor;
typedef struct _StatusAppletTorClass StatusAppletTorClass;
typedef struct _StatusAppletTorPrivate StatusAppletTorPrivate;

typedef enum {
	TOR_NOT_CONNECTED = 0,
	TOR_CONNECTING = 1,
	TOR_CONNECTED = 2,
} TorConnState;

struct _StatusAppletTor {
	HDStatusMenuItem parent;
	StatusAppletTorPrivate *priv;
};

struct _StatusAppletTorClass {
	HDStatusMenuItemClass parent;
};

struct _StatusAppletTorPrivate {
	osso_context_t *osso;
	DBusConnection *dbus;
	GtkWidget *menu_button;
	TorConnState connection_state;
	gboolean provider_connected;
	gboolean tor_running;
	GtkWidget *settings_dialog;
	GtkWidget *tor_chkbtn;
	GtkWidget *config_btn;
};

HD_DEFINE_PLUGIN_MODULE_WITH_PRIVATE(StatusAppletTor, status_applet_tor,
				     HD_TYPE_STATUS_MENU_ITEM);
#define GET_PRIVATE(x) status_applet_tor_get_instance_private(x)

static void save_settings(StatusAppletTor * self)
{
	/* TODO: Here make a note in gconf on which configuration is selected.
	 * e.g.
	 * Default -> /system/maemo/tor/active_config
	 * or
	 * Foobar -> /system/maemo/tor/active_config
	 */
	(void)self;
}

static void start_tor_daemon(StatusAppletTor * self)
{
	/* TODO: Generate torrc from gconf and start via fork&exec
	 * This way we don't need privilege escalation. */
	(void)self;
}

static void execute_cp_plugin(GtkWidget * btn, StatusAppletTor * self)
{
	StatusAppletTorPrivate *p = GET_PRIVATE(self);
	GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(self));
	gtk_widget_hide(toplevel);

	if (osso_cp_plugin_execute(p->osso, "control-applet-tor.so", self, TRUE)
	    == OSSO_ERROR) {
		hildon_banner_show_information(NULL, NULL,
					       "Failed to show Tor settings");
	}
}

static void status_menu_clicked_cb(GtkWidget * btn, StatusAppletTor * self)
{
	StatusAppletTorPrivate *p = GET_PRIVATE(self);
	GtkWidget *toplevel = gtk_widget_get_toplevel(btn);
	GtkSizeGroup *size_group;
	GtkWidget *touch_selector;

	gtk_widget_hide(toplevel);

	p->settings_dialog = hildon_dialog_new_with_buttons("Tor",
							    GTK_WINDOW
							    (toplevel),
							    GTK_DIALOG_MODAL |
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    "Settings",
							    SETTINGS_RESPONSE,
							    GTK_STOCK_SAVE,
							    GTK_RESPONSE_ACCEPT,
							    NULL);

	p->tor_chkbtn =
	    hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT |
				    HILDON_SIZE_AUTO_WIDTH);
	gtk_button_set_label(GTK_BUTTON(p->tor_chkbtn), "Enable Tor daemon");
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(p->tor_chkbtn),
				       p->tor_running);

	/* TODO: Connect with saved configurations from control panel */
	size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	touch_selector = hildon_touch_selector_new_text();
	/* TODO: hildon_touch_selector_get_current_text */
	p->config_btn = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT, 0);
	hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(p->config_btn),
					  HILDON_TOUCH_SELECTOR
					  (touch_selector));
	hildon_button_set_title(HILDON_BUTTON(p->config_btn),
				"Current configuration");
	hildon_button_set_alignment(HILDON_BUTTON(p->config_btn), 0.0, 0.5, 1.0,
				    1.0);
	/* TODO: for loop list gconf configurations */
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(touch_selector),
					  "Default");
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(touch_selector),
					  "Custom");
	hildon_button_add_title_size_group(HILDON_BUTTON(p->config_btn),
					   size_group);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(p->settings_dialog)->vbox),
			   p->tor_chkbtn, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(p->settings_dialog)->vbox),
			   p->config_btn, TRUE, TRUE, 0);

	/* Make the buttons insensitive when provider is connected.
	 * At this time the daemon is controlled by icd and not this applet.
	 */
	if (p->provider_connected) {
		gtk_widget_set_sensitive(p->tor_chkbtn, FALSE);
		gtk_widget_set_sensitive(p->config_btn, FALSE);
	}

	gtk_widget_show_all(p->settings_dialog);
	switch (gtk_dialog_run(GTK_DIALOG(p->settings_dialog))) {
	case GTK_RESPONSE_ACCEPT:
		save_settings(self);
		start_tor_daemon(self);
		break;
	case SETTINGS_RESPONSE:
		execute_cp_plugin(btn, self);
	default:
		break;
	}

	gtk_widget_destroy(p->settings_dialog);
}

static void status_applet_tor_set_icons(StatusAppletTor * self)
{
	StatusAppletTorPrivate *p = GET_PRIVATE(self);
	GtkIconTheme *theme = gtk_icon_theme_get_default();
	GdkPixbuf *menu_pixbuf = NULL;
	GdkPixbuf *status_pixbuf = NULL;

	switch (p->connection_state) {
	case TOR_NOT_CONNECTED:
		menu_pixbuf =
		    gtk_icon_theme_load_icon(theme, "statusarea_tor_disabled",
					     48, 0, NULL);
		hildon_button_set_value(HILDON_BUTTON(p->menu_button),
					"Disconnected");
		break;
	case TOR_CONNECTING:
		/* TODO: blink, use network-liveness from Tor control socket */
		break;
	case TOR_CONNECTED:
		menu_pixbuf =
		    gtk_icon_theme_load_icon(theme, "statusarea_tor_enabled",
					     48, 0, NULL);
		status_pixbuf =
		    gtk_icon_theme_load_icon(theme, "statusarea_tor_connected",
					     18, 0, NULL);
		hildon_button_set_value(HILDON_BUTTON(p->menu_button),
					"Connected");
		break;
	default:
		g_critical("%s: Invalid connection_state", G_STRLOC);
		break;
	};

	if (menu_pixbuf) {
		hildon_button_set_image(HILDON_BUTTON(p->menu_button),
					gtk_image_new_from_pixbuf(menu_pixbuf));
		hildon_button_set_image_position(HILDON_BUTTON(p->menu_button),
						 0);
		g_object_unref(menu_pixbuf);
	}

	/* The icon is hidden when the pixbuf is NULL */
	hd_status_plugin_item_set_status_area_icon(HD_STATUS_PLUGIN_ITEM(self),
						   status_pixbuf);

	if (status_pixbuf) {
		g_object_unref(status_pixbuf);
	}
}

static int handle_running(gpointer obj, DBusMessage * msg)
{
	/* Either show or hide status icon */
	StatusAppletTorPrivate *p = GET_PRIVATE(obj);
	int status;

	dbus_message_get_args(msg, NULL, DBUS_TYPE_BYTE,
			      &status, DBUS_TYPE_INVALID);

	p->provider_connected = TRUE;
	p->tor_running = TRUE;

	return 0;
}

static int on_icd_signal(DBusConnection * dbus, DBusMessage * msg, gpointer obj)
{
	(void)dbus;

	if (dbus_message_is_signal
	    (msg, "org.maemo.TorProvider.Running", "Running"))
		return handle_running(obj, msg);

	return 1;
}

static void setup_dbus_matching(StatusAppletTor * self)
{
	StatusAppletTorPrivate *p = GET_PRIVATE(self);
	DBusError err;

	dbus_error_init(&err);

	p->dbus =
	    hd_status_plugin_item_get_dbus_connection(HD_STATUS_PLUGIN_ITEM
						      (self), DBUS_BUS_SYSTEM,
						      &err);

	if (dbus_error_is_set(&err)) {
		status_debug("tor-sb: Error getting dbus system bus: %s",
			     err.message);
		dbus_error_free(&err);
		return;
	}

	dbus_connection_setup_with_g_main(p->dbus, NULL);

	dbus_bus_add_match(p->dbus,
			   "type='signal',interface='org.maemo.TorProvider.Running',member='Running'",
			   &err);

	if (dbus_error_is_set(&err)) {
		status_debug("tor-sb: Failed to add match: %s", err.message);
		dbus_error_free(&err);
		return;
	}

	if (!dbus_connection_add_filter
	    (p->dbus, (DBusHandleMessageFunction) on_icd_signal, self, NULL)) {
		status_debug("tor-sb: Failed to add dbus filter");
		return;
	}
}

static void status_applet_tor_init(StatusAppletTor * self)
{
	StatusAppletTor *sa = STATUS_APPLET_TOR(self);
	StatusAppletTorPrivate *p = GET_PRIVATE(sa);
	DBusError err;

	p->osso = osso_initialize("tor-sb", VERSION, FALSE, NULL);

	dbus_error_init(&err);

	p->connection_state = TOR_NOT_CONNECTED;
	p->tor_running = FALSE;
	p->provider_connected = FALSE;

	/* Dbus setup for icd provider */
	setup_dbus_matching(self);

	/* Gtk items */
	p->menu_button =
	    hildon_button_new_with_text(HILDON_SIZE_FINGER_HEIGHT,
					HILDON_BUTTON_ARRANGEMENT_VERTICAL,
					"Tor", NULL);
	hildon_button_set_alignment(HILDON_BUTTON(p->menu_button),
				    0.0, 0.5, 0.0, 0.0);
	hildon_button_set_style(HILDON_BUTTON(p->menu_button),
				HILDON_BUTTON_STYLE_PICKER);

	status_applet_tor_set_icons(sa);

	g_signal_connect(p->menu_button, "clicked",
			 G_CALLBACK(status_menu_clicked_cb), self);

	gtk_container_add(GTK_CONTAINER(sa), p->menu_button);
	gtk_widget_show_all(GTK_WIDGET(sa));
}

static void status_applet_tor_class_init(StatusAppletTorClass * klass)
{
	(void)klass;
}

static void status_applet_tor_class_finalize(StatusAppletTorClass * klass)
{
	(void)klass;
}

StatusAppletTor *status_applet_tor_new(void)
{
	return g_object_new(STATUS_APPLET_TOR_TYPE, NULL);
}

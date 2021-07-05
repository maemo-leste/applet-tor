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

#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <libhildondesktop/libhildondesktop.h>

#define STATUS_APPLET_TOR_TYPE (status_applet_tor_get_type())
#define STATUS_APPLET_TOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		STATUS_APPLET_TOR_TYPE, StatusAppletTor))

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
	GtkWidget *menu_button;
	TorConnState connection_state;
	gboolean provider_connected;
	gboolean tor_running;
	GtkWidget *settings_dialog;
	GtkWidget *tor_chkbutton;
};

HD_DEFINE_PLUGIN_MODULE_WITH_PRIVATE(StatusAppletTor, status_applet_tor,
				     HD_TYPE_STATUS_MENU_ITEM);
#define GET_PRIVATE(x) status_applet_tor_get_instance_private(x)

static void save_settings(StatusAppletTor * self)
{
	(void)self;
}

static void start_tor_daemon(StatusAppletTor * self)
{
	(void)self;
}

static void status_menu_clicked_cb(GtkWidget * btn, StatusAppletTor * self)
{
	StatusAppletTorPrivate *p = GET_PRIVATE(self);
	GtkWidget *toplevel = gtk_widget_get_toplevel(btn);
	GtkSizeGroup *size_group;
	GtkWidget *config_btn, *touch_selector;

	gtk_widget_hide(toplevel);

	p->settings_dialog = hildon_dialog_new_with_buttons("Tor",
							    GTK_WINDOW
							    (toplevel),
							    GTK_DIALOG_MODAL |
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_STOCK_SAVE,
							    GTK_RESPONSE_ACCEPT,
							    NULL);

	/* TODO: Disable (make inactive and checked ) when connected to provider */
	p->tor_chkbutton =
	    hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT |
				    HILDON_SIZE_AUTO_WIDTH);
	gtk_button_set_label(GTK_BUTTON(p->tor_chkbutton), "Enable Tor daemon");
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(p->tor_chkbutton),
				       p->tor_running);

	/* TODO: Connect with saved configurations from control panel */
	size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	touch_selector = hildon_touch_selector_new_text();
	/* TODO: hildon_touch_selector_get_current_text */
	config_btn = hildon_picker_button_new(HILDON_SIZE_FINGER_HEIGHT, 0);
	hildon_picker_button_set_selector(HILDON_PICKER_BUTTON(config_btn),
					  HILDON_TOUCH_SELECTOR
					  (touch_selector));
	hildon_button_set_title(HILDON_BUTTON(config_btn),
				"Current configuration");
	hildon_button_set_alignment(HILDON_BUTTON(config_btn), 0.0, 0.5, 1.0,
				    1.0);
	/* TODO: for loop list gconf configurations */
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(touch_selector),
					  "Default");
	hildon_touch_selector_append_text(HILDON_TOUCH_SELECTOR(touch_selector),
					  "Custom");
	hildon_button_add_title_size_group(HILDON_BUTTON(config_btn),
					   size_group);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(p->settings_dialog)->vbox),
			   p->tor_chkbutton, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(p->settings_dialog)->vbox),
			   config_btn, TRUE, TRUE, 0);

	gtk_widget_show_all(p->settings_dialog);
	switch (gtk_dialog_run(GTK_DIALOG(p->settings_dialog))) {
	case GTK_RESPONSE_ACCEPT:
		save_settings(self);
		start_tor_daemon(self);
		break;
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
		// TODO: blink
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
	// The icon is hidden when the pixbuf is NULL
	hd_status_plugin_item_set_status_area_icon(HD_STATUS_PLUGIN_ITEM(self),
						   status_pixbuf);

	if (status_pixbuf) {
		g_object_unref(status_pixbuf);
	}
}

static void status_applet_tor_init(StatusAppletTor * self)
{
	StatusAppletTor *sa = STATUS_APPLET_TOR(self);
	StatusAppletTorPrivate *p = GET_PRIVATE(sa);

	p->connection_state = TOR_NOT_CONNECTED;
	p->tor_running = FALSE;
	p->provider_connected = FALSE;

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

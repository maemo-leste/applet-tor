/*
 * Copyright (c) 2021 Ivan J. <parazyd@dyne.org>
 *
 * This file is part of tor-applet
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
#ifndef __WIZARD_H__
#define __WIZARD_H__

enum wizard_button {
	WIZARD_BUTTON_FINISH = 0,
	WIZARD_BUTTON_PREVIOUS = 1,
	WIZARD_BUTTON_NEXT = 2,
	WIZARD_BUTTON_CLOSE = 3,
	WIZARD_BUTTON_ADVANCED = 4
};

struct wizard_data {
	GtkWidget *assistant;

	const gchar *config_name;
	GtkWidget *name_entry;

	GtkWidget *transproxy_chk;

	gboolean has_bridges;
	GtkWidget *br_chk;
	gint bridges_page;
	gchar *bridges_data;
	GtkWidget *bridges_textview;

	gboolean has_hs;
	GtkWidget *hs_chk;
	gint hs_page;
	gchar *hs_data;
	GtkWidget *hs_textview;

	gboolean has_adv;
	GtkWidget *ad_chk;
	GtkWidget *ports_wrn;
	gint adv_page;

	GtkWidget *socksport_entry;
	guint socksport;

	GtkWidget *controlport_entry;
	guint controlport;

	GtkWidget *dnsport_entry;
	guint dnsport;

	GtkWidget *transport_entry;
	guint transport;

};

void start_new_wizard(gpointer config_data);

#endif

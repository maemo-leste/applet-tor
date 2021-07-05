/*
 * Copyright (c) 2020-2021 Ivan Jelincic <parazyd@dyne.org>
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
#include <hildon-cp-plugin/hildon-cp-plugin-interface.h>

#define CONFIGS_RESPONSE -69

static GtkWidget *new_main_dialog(GtkWindow * parent)
{
	GtkWidget *dialog;

	dialog =
	    gtk_dialog_new_with_buttons("Tor Settings", parent, 0, "Configs",
					CONFIGS_RESPONSE, "Save",
					GTK_RESPONSE_ACCEPT, NULL);

	return dialog;
}

osso_return_t execute(osso_context_t * osso, gpointer data, gboolean user_act)
{
	(void)osso;
	(void)user_act;
	GtkWidget *maindialog;

	maindialog = new_main_dialog(GTK_WINDOW(data));

	gtk_widget_show_all(maindialog);

	gtk_dialog_run(GTK_DIALOG(maindialog));

	if (maindialog)
		gtk_widget_destroy(maindialog);

	return OSSO_OK;
}

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

enum {
	CONFIG_NEW,
	CONFIG_EDIT,
	CONFIG_DELETE,
	CONFIG_DONE,
};

static GtkWidget *new_main_dialog(GtkWindow * parent)
{
	GtkWidget *dialog, *tree_view, *scrolled_window;
	GtkTreeSelection *selection;
	GtkListStore *list_store;

	dialog =
	    gtk_dialog_new_with_buttons("Tor Configurations", parent, 0,
					"New", CONFIG_NEW,
					"Edit", CONFIG_EDIT,
					"Delete", CONFIG_DELETE,
					"Done", CONFIG_DONE, NULL);

	list_store = gtk_list_store_new(6, GDK_TYPE_PIXBUF, G_TYPE_STRING,
					G_TYPE_STRING, G_TYPE_STRING,
					G_TYPE_STRING, G_TYPE_STRING);

	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	g_object_unref(list_store);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);
	gtk_widget_set_size_request(scrolled_window, 478, 240);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), scrolled_window,
			   TRUE, TRUE, 0);

	return dialog;
}

osso_return_t execute(osso_context_t * osso, gpointer data, gboolean user_act)
{
	(void)osso;
	(void)user_act;
	gboolean config_done = FALSE;
	GtkWidget *maindialog;

	maindialog = new_main_dialog(GTK_WINDOW(data));

	gtk_widget_show_all(maindialog);

	do {
		/* TODO: Work on selected item in TreeView */
		switch (gtk_dialog_run(GTK_DIALOG(maindialog))) {
		case CONFIG_NEW:
			break;
		case CONFIG_EDIT:
			break;
		case CONFIG_DELETE:
			break;
		case CONFIG_DONE:
		default:
			config_done = TRUE;
			break;
		}

	} while (!config_done);

	gtk_widget_destroy(maindialog);

	return OSSO_OK;
}

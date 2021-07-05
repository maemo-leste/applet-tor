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
#include <gconf/gconf-client.h>
#include <hildon/hildon.h>
#include <hildon-cp-plugin/hildon-cp-plugin-interface.h>

enum {
	CONFIG_NEW,
	CONFIG_EDIT,
	CONFIG_DELETE,
	CONFIG_DONE,
};

enum {
	LIST_ITEM = 0,
	N_COLUMNS
};

#define GC_TOR        "/system/maemo/tor"
#define GC_TOR_ACTIVE GC_TOR"/active_config"

static void add_to_treeview(GtkWidget * tv, const gchar * str)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tv)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);
}

static void fill_treeview_from_gconf(GtkWidget * tv)
{
	GConfClient *gconf;
	GSList *configs, *iter;

	gconf = gconf_client_get_default();
	configs = gconf_client_all_dirs(gconf, GC_TOR, NULL);
	g_object_unref(gconf);

	for (iter = configs; iter; iter = iter->next) {
		add_to_treeview(tv, g_path_get_basename(iter->data));
		g_free(iter->data);
	}

	g_slist_free(iter);
	g_slist_free(configs);
}

static GtkWidget *new_main_dialog(GtkWindow * parent)
{
	GtkWidget *dialog, *tv;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	dialog =
	    gtk_dialog_new_with_buttons("Tor Configurations", parent, 0,
					"New", CONFIG_NEW,
					"Edit", CONFIG_EDIT,
					"Delete", CONFIG_DELETE,
					"Done", CONFIG_DONE, NULL);

	tv = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tv), FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), tv, TRUE, TRUE,
			   0);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Items",
							  renderer, "text",
							  LIST_ITEM, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tv), column);
	store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tv), GTK_TREE_MODEL(store));

	g_object_unref(store);

	fill_treeview_from_gconf(tv);

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

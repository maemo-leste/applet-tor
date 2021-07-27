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

#include "configuration.h"
#include "wizard.h"

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

GtkWidget *cfg_tree;

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
	GtkWidget *dialog;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	dialog =
	    gtk_dialog_new_with_buttons("Tor Configurations", parent, 0,
					"New", CONFIG_NEW,
					"Edit", CONFIG_EDIT,
					"Delete", CONFIG_DELETE,
					"Done", CONFIG_DONE, NULL);

	cfg_tree = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(cfg_tree), FALSE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), cfg_tree, TRUE,
			   TRUE, 0);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Items",
							  renderer, "text",
							  LIST_ITEM, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(cfg_tree), column);
	store = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(cfg_tree), GTK_TREE_MODEL(store));

	g_object_unref(store);

	fill_treeview_from_gconf(cfg_tree);

	return dialog;
}

static void delete_config(GtkTreeView * tv)
{
	GtkTreeSelection *sel = gtk_tree_view_get_selection(tv);
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *gconf_cfg, *sel_cfg;
	GConfClient *gconf;

	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;

	gtk_tree_model_get(model, &iter, 0, &sel_cfg, -1);

	if (!strcmp(sel_cfg, "Default")) {
		g_free(sel_cfg);
		return;
	}

	/* TODO: Yes/No dialog? */

	gconf_cfg = g_strjoin("/", GC_TOR, sel_cfg, NULL);

	gconf = gconf_client_get_default();
	gconf_client_recursive_unset(gconf, gconf_cfg, 0, NULL);
	gconf_client_remove_dir(gconf, gconf_cfg, NULL);

	g_free(sel_cfg);
	g_free(gconf_cfg);
	g_object_unref(gconf);
}

osso_return_t execute(osso_context_t * osso, gpointer data, gboolean user_act)
{
	(void)osso;
	(void)user_act;
	gboolean config_done = FALSE;
	GtkWidget *maindialog;

	do {
		/* TODO: Write a better way to refresh the tree view */
		maindialog = new_main_dialog(GTK_WINDOW(data));
		gtk_widget_show_all(maindialog);

		/* TODO: Work on selected item in TreeView */
		switch (gtk_dialog_run(GTK_DIALOG(maindialog))) {
		case CONFIG_NEW:
			gtk_widget_hide(maindialog);
			start_new_wizard(NULL);
			break;
		case CONFIG_EDIT:
			break;
		case CONFIG_DELETE:
			delete_config(GTK_TREE_VIEW(cfg_tree));
			break;
		case CONFIG_DONE:
		default:
			config_done = TRUE;
			break;
		}

		gtk_widget_destroy(maindialog);
	} while (!config_done);

	return OSSO_OK;
}

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
#include <hildon/hildon.h>
#include <hildon-cp-plugin/hildon-cp-plugin-interface.h>

#include "configuration.h"
#include "wizard.h"

static void on_assistant_close_cancel(GtkWidget * widget, gpointer data)
{
	(void)widget;
	GtkWidget **assistant = (GtkWidget **) data;
	gtk_widget_destroy(*assistant);
	*assistant = NULL;
	gtk_main_quit();
}

static void validate_bridges_textview_cb(GtkWidget * widget, gpointer data)
{
	(void)widget;
	struct wizard_data *w_data = data;
	GtkWidget *current_page;

	current_page =
	    gtk_assistant_get_nth_page(GTK_ASSISTANT(w_data->assistant),
				       w_data->bridges_page);

	gtk_assistant_set_page_complete(GTK_ASSISTANT(w_data->assistant),
					current_page, TRUE);
}

static gint new_wizard_bridges_page(struct wizard_data *w_data)
{
	GtkWidget *info, *vbox, *validate_btn;
	gint rv;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

	info = gtk_label_new("Here you can input/paste and validate bridges.\n"
			     "See https://bridges.torproject.org for more info.");

	// TODO: Make page not completed when text view changes
	w_data->bridges_textview = gtk_text_view_new();

	validate_btn = gtk_button_new_with_label("Validate");
	g_signal_connect(G_OBJECT(validate_btn), "clicked",
			 G_CALLBACK(validate_bridges_textview_cb), w_data);

	gtk_box_pack_start(GTK_BOX(vbox), info, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->bridges_textview, TRUE, TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox), validate_btn, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	rv = gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(w_data->assistant),
				     vbox, "Tor Bridges");

	return rv;
}

static gint new_wizard_hs_page(struct wizard_data *w_data)
{
	GtkWidget *info, *vbox, *validate_btn;
	gint rv;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

	info = gtk_label_new("Here you can configure Hidden Services for\n"
			     "this configuration.");

	validate_btn = gtk_button_new_with_label("Validate");
	/*
	   g_signal_connect(G_OBJECT(validate_btn), "clicked",
	   G_CALLBACK(validate_hidden_services_cb), w_data);
	 */

	gtk_box_pack_start(GTK_BOX(vbox), info, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), validate_btn, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	rv = gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(w_data->assistant),
				     vbox, "Tor Hidden Services");

	return rv;
}

static gint new_wizard_adv_page(struct wizard_data *w_data)
{
	GtkWidget *vbox;
	GtkWidget *s_hbox, *socksport_entry, *socksport_lbl;
	GtkWidget *c_hbox, *controlport_entry, *controlport_lbl;
	GtkWidget *d_hbox, *dnsport_entry, *dnsport_lbl;
	GtkWidget *t_hbox, *transport_entry, *transport_lbl;
	gint rv;

	vbox = gtk_vbox_new(TRUE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

	s_hbox = gtk_hbox_new(FALSE, 2);
	socksport_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(socksport_entry), "9050");
	socksport_lbl = gtk_label_new("SOCKSPort: ");
	//socksport_wrn = gtk_label_new("<- Invalid port number!");
	gtk_box_pack_start(GTK_BOX(s_hbox), socksport_lbl, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(s_hbox), socksport_entry, FALSE, FALSE, 0);
	//gtk_box_pack_start(GTK_BOX(s_hbox), socksport_wrn, FALSE, FALSE, 0);

	c_hbox = gtk_hbox_new(FALSE, 2);
	controlport_entry = gtk_entry_new();
	controlport_lbl = gtk_label_new("ControlPort: ");
	gtk_entry_set_text(GTK_ENTRY(controlport_entry), "9051");
	gtk_box_pack_start(GTK_BOX(c_hbox), controlport_lbl, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(c_hbox), controlport_entry, FALSE, FALSE, 0);

	d_hbox = gtk_hbox_new(FALSE, 2);
	dnsport_entry = gtk_entry_new();
	dnsport_lbl = gtk_label_new("DNSPort: ");
	gtk_entry_set_text(GTK_ENTRY(dnsport_entry), "7353");
	gtk_box_pack_start(GTK_BOX(d_hbox), dnsport_lbl, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(d_hbox), dnsport_entry, FALSE, FALSE, 0);

	t_hbox = gtk_hbox_new(FALSE, 2);
	transport_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(transport_entry), "9100");
	transport_lbl = gtk_label_new("TransPort: ");
	gtk_box_pack_start(GTK_BOX(t_hbox), transport_lbl, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(t_hbox), transport_entry, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), s_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), c_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), d_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_hbox, TRUE, TRUE, 0);
	gtk_widget_show_all(vbox);

	rv = gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(w_data->assistant),
				     vbox, "Advanced Settings");

	return rv;
}

static void on_br_chk_btn_toggled(GtkWidget * widget, gpointer data)
{
	struct wizard_data *w_data = data;
	GtkWidget *page;
	gboolean active;

	/* We assume page should be 0, as the checkbox is there. */
	page = gtk_assistant_get_nth_page(GTK_ASSISTANT(w_data->assistant), 0);

	g_object_get(G_OBJECT(w_data->br_chk), "active", &active, NULL);

	if (active) {
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_INTRO);
		w_data->bridges_page = new_wizard_bridges_page(w_data);
		w_data->has_bridges = TRUE;
		return;
	}

	if (w_data->hs_page > 0 || w_data->adv_page > 0)
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_INTRO);
	else
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_CONFIRM);

	w_data->bridges_page = -1;
	w_data->has_bridges = FALSE;

	(void)widget;
}

static void on_hs_chk_btn_toggled(GtkWidget * widget, gpointer data)
{
	struct wizard_data *w_data = data;
	GtkWidget *page;
	gboolean active;

	/* We assume page should be 0, as the checkbox is there. */
	page = gtk_assistant_get_nth_page(GTK_ASSISTANT(w_data->assistant), 0);

	g_object_get(G_OBJECT(w_data->hs_chk), "active", &active, NULL);

	if (active) {
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_INTRO);
		w_data->hs_page = new_wizard_hs_page(w_data);
		w_data->has_hs = TRUE;
		return;
	}

	if (w_data->bridges_page > 0 || w_data->adv_page > 0)
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_INTRO);
	else
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_CONFIRM);
	//w_data->intro_vbox, GTK_ASSISTANT_PAGE_CONFIRM);

	w_data->hs_page = -1;
	w_data->has_hs = FALSE;

	(void)widget;
}

static void on_ad_chk_btn_toggled(GtkWidget * widget, gpointer data)
{
	struct wizard_data *w_data = data;
	GtkWidget *page;
	gboolean active;

	/* We assume page should be 0, as the checkbox is there. */
	page = gtk_assistant_get_nth_page(GTK_ASSISTANT(w_data->assistant), 0);

	g_object_get(G_OBJECT(w_data->ad_chk), "active", &active, NULL);

	g_message("adv_page: %d", w_data->adv_page);

	if (active) {
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_INTRO);
		w_data->adv_page = new_wizard_adv_page(w_data);
		w_data->has_adv = TRUE;
		return;
	}

	if (w_data->bridges_page > 0 || w_data->hs_page > 0)
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_INTRO);
	else
		gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
					    page, GTK_ASSISTANT_PAGE_CONFIRM);

	w_data->adv_page = -1;
	w_data->has_adv = FALSE;

	(void)widget;
}

static void on_conf_name_entry_changed(GtkWidget * widget, gpointer data)
{
	GtkAssistant *assistant = GTK_ASSISTANT(data);
	GtkWidget *current_page;
	gint page_number;
	const gchar *text;

	page_number = gtk_assistant_get_current_page(assistant);
	current_page = gtk_assistant_get_nth_page(assistant, page_number);
	text = gtk_entry_get_text(GTK_ENTRY(widget));

	if (text && *text)
		gtk_assistant_set_page_complete(assistant, current_page, TRUE);
	else
		gtk_assistant_set_page_complete(assistant, current_page, FALSE);
}

static void new_wizard_main_page(struct wizard_data *w_data)
{
	GtkWidget *vbox, *box, *info, *name_label;

	vbox = gtk_vbox_new(FALSE, 5);
	box = gtk_hbox_new(FALSE, 12);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_container_set_border_width(GTK_CONTAINER(box), 12);

	info =
	    gtk_label_new
	    ("Here you can make a new Tor configuration. If you don't want to edit\n"
	     "any specifics, then just write a name for your configuration and\n"
	     "finish the wizard.");

	name_label = gtk_label_new("Configuration name:");
	w_data->name_entry = gtk_entry_new();
	g_signal_connect(G_OBJECT(w_data->name_entry), "changed",
			 G_CALLBACK(on_conf_name_entry_changed),
			 w_data->assistant);

	w_data->br_chk = gtk_check_button_new_with_label("Use Bridges");
	w_data->hs_chk = gtk_check_button_new_with_label("Use Hidden Services");
	w_data->ad_chk = gtk_check_button_new_with_label("Advanced Settings");

	g_signal_connect(G_OBJECT(w_data->br_chk), "toggled",
			 G_CALLBACK(on_br_chk_btn_toggled), w_data);
	g_signal_connect(G_OBJECT(w_data->hs_chk), "toggled",
			 G_CALLBACK(on_hs_chk_btn_toggled), w_data);
	g_signal_connect(G_OBJECT(w_data->ad_chk), "toggled",
			 G_CALLBACK(on_ad_chk_btn_toggled), w_data);

	gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), w_data->name_entry, TRUE, TRUE, 0);
	gtk_widget_show_all(box);

	gtk_box_pack_start(GTK_BOX(vbox), info, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->br_chk, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->hs_chk, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->ad_chk, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT(w_data->assistant),
				     vbox, "Tor Configuration Wizard");
	gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
				    vbox, GTK_ASSISTANT_PAGE_CONFIRM);
}

static gint find_next_wizard_page(gint current_page, gpointer data)
{
	struct wizard_data *w_data = data;

	if (current_page == 0) {
		if (w_data->bridges_page > 0)
			return w_data->bridges_page;

		if (w_data->hs_page > 0)
			return w_data->hs_page;

		if (w_data->adv_page > 0)
			return w_data->adv_page;
	}

	if (current_page == w_data->bridges_page) {
		if (w_data->hs_page > 0)
			return w_data->hs_page;

		if (w_data->adv_page > 0)
			return w_data->adv_page;
	}

	if (current_page == w_data->hs_page) {
		if (w_data->adv_page > 0)
			return w_data->adv_page;
	}

	return -1;
}

static void on_assistant_apply(GtkWidget * widget, gpointer data)
{
	(void)widget;
	(void)data;
	g_message("apply");
}

static void on_assistant_prepare(GtkWidget * assistant, GtkWidget * page,
				 gpointer data)
{
	struct wizard_data *w_data = data;
	gint page_number =
	    gtk_assistant_get_current_page(GTK_ASSISTANT(assistant));

	if (page_number == w_data->bridges_page) {
		if (w_data->hs_page > 0 || w_data->adv_page > 0)
			gtk_assistant_set_page_type(GTK_ASSISTANT(assistant),
						    page,
						    GTK_ASSISTANT_PAGE_CONTENT);
		else
			gtk_assistant_set_page_type(GTK_ASSISTANT(assistant),
						    page,
						    GTK_ASSISTANT_PAGE_CONFIRM);
		return;
	}

	if (page_number == w_data->hs_page) {
		if (w_data->adv_page > 0)
			gtk_assistant_set_page_type(GTK_ASSISTANT(assistant),
						    page,
						    GTK_ASSISTANT_PAGE_CONTENT);
		else
			gtk_assistant_set_page_type(GTK_ASSISTANT(assistant),
						    page,
						    GTK_ASSISTANT_PAGE_CONFIRM);
		return;
	}

	if (page_number == w_data->adv_page) {
		gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), page,
					    GTK_ASSISTANT_PAGE_CONFIRM);
		return;
	}
}

void start_new_wizard(gpointer config_data)
{
	struct wizard_data *w_data;

	if (config_data == NULL) {
		w_data = g_new0(struct wizard_data, 1);
		w_data->has_bridges = FALSE;
		w_data->has_hs = FALSE;
		w_data->has_adv = FALSE;
		w_data->bridges_page = -1;
		w_data->hs_page = -1;
		w_data->adv_page = -1;
	} else {
		w_data = config_data;
	}

	w_data->assistant = gtk_assistant_new();
	gtk_window_set_title(GTK_WINDOW(w_data->assistant),
			     config_data ? "New Tor configuration" :
			     "Edit Tor configuration");

	gtk_assistant_set_forward_page_func(GTK_ASSISTANT(w_data->assistant),
					    find_next_wizard_page, w_data,
					    NULL);

	new_wizard_main_page(w_data);

	if (w_data->has_bridges)
		new_wizard_bridges_page(w_data);

	if (w_data->has_hs)
		new_wizard_hs_page(w_data);

	if (w_data->has_adv)
		new_wizard_adv_page(w_data);

	g_signal_connect(G_OBJECT(w_data->assistant), "cancel",
			 G_CALLBACK(on_assistant_close_cancel),
			 &w_data->assistant);

	g_signal_connect(G_OBJECT(w_data->assistant), "close",
			 G_CALLBACK(on_assistant_close_cancel),
			 &w_data->assistant);

	g_signal_connect(G_OBJECT(w_data->assistant), "prepare",
			 G_CALLBACK(on_assistant_prepare), w_data);

	g_signal_connect(G_OBJECT(w_data->assistant), "apply",
			 G_CALLBACK(on_assistant_apply), w_data);

	gtk_widget_show_all(w_data->assistant);
	gtk_main();
	g_free(w_data);
}

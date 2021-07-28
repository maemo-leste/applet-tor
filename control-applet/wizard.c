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
#include <errno.h>

#include <gconf/gconf-client.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>

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

static gchar *gen_bridges_torrc(const gchar * entries)
{
	return g_strdup_printf("%s\n%s\n", BRIDGES_CFG, entries);
}

static gint find_next_wizard_page(gint cur_page, gpointer data)
{
	struct wizard_data *w_data = data;

	if (cur_page == 0) {
		if (w_data->bridges_page > 0)
			return w_data->bridges_page;

		if (w_data->hs_page > 0)
			return w_data->hs_page;

		if (w_data->adv_page > 0)
			return w_data->adv_page;
	}

	if (cur_page == w_data->bridges_page) {
		if (w_data->hs_page > 0)
			return w_data->hs_page;

		if (w_data->adv_page > 0)
			return w_data->adv_page;
	}

	if (cur_page == w_data->hs_page) {
		if (w_data->adv_page > 0)
			return w_data->adv_page;
	}

	return -1;
}

/*
static void gconf_set_string(GConfClient * gconf, gchar * key, gchar * string)
{
	GConfValue *v = gconf_value_new(GCONF_VALUE_STRING);
	gconf_value_set_string(v, string);
	gconf_client_set(gconf, key, v, NULL);
	gconf_value_free(v);
}
*/

static void gconf_set_int(GConfClient * gconf, gchar * key, gint x)
{
	GConfValue *v;

	v = gconf_value_new(GCONF_VALUE_INT);
	gconf_value_set_int(v, x);
	gconf_client_set(gconf, key, v, NULL);
	gconf_value_free(v);
}

static void on_assistant_apply(GtkWidget * widget, gpointer data)
{
	struct wizard_data *w_data = data;
	GConfClient *gconf = gconf_client_get_default();
	gchar *gconf_socksport, *gconf_controlport, *gconf_dnsport,
	    *gconf_transport;

	w_data->config_name = gtk_entry_get_text(GTK_ENTRY(w_data->name_entry));
	gchar *confname = g_strjoin("/", GC_TOR, w_data->config_name, NULL);

	gconf_client_add_dir(gconf, confname, GCONF_CLIENT_PRELOAD_NONE, NULL);

	if (w_data->socksport_entry) {
		/* Just one being not NULL means all are initialized, so we only check
		 * for socksport_entry. These GtkWidgets are initialized when advanced
		 * settings are enabled from the first page.
		 */
		w_data->socksport =
		    atoi(gtk_entry_get_text
			 (GTK_ENTRY(w_data->socksport_entry)));

		w_data->controlport =
		    atoi(gtk_entry_get_text
			 (GTK_ENTRY(w_data->controlport_entry)));

		w_data->dnsport =
		    atoi(gtk_entry_get_text(GTK_ENTRY(w_data->dnsport_entry)));

		w_data->transport =
		    atoi(gtk_entry_get_text
			 (GTK_ENTRY(w_data->transport_entry)));
	}

	gconf_socksport = g_strjoin("/", confname, GC_CFG_SOCKSPORT, NULL);
	gconf_set_int(gconf, gconf_socksport, w_data->socksport);
	g_free(gconf_socksport);

	gconf_controlport = g_strjoin("/", confname, GC_CFG_CONTROLPORT, NULL);
	gconf_set_int(gconf, gconf_controlport, w_data->controlport);
	g_free(gconf_controlport);

	gconf_dnsport = g_strjoin("/", confname, GC_CFG_DNSPORT, NULL);
	gconf_set_int(gconf, gconf_dnsport, w_data->dnsport);
	g_free(gconf_dnsport);

	/* TODO: if transproxy_chk is active, note the port. Perhaps the
	 * availability of TransPort in gconf should signal that this config
	 * is used for (or has) transparent proxying. Consider not writing
	 * this value unless the checkbox is active?
	 */
	gconf_transport = g_strjoin("/", confname, GC_CFG_TRANSPORT, NULL);
	gconf_set_int(gconf, gconf_transport, w_data->transport);
	g_free(gconf_transport);

	/* TODO: Bridges, hidden services, datadirs */

	g_object_unref(gconf);
	g_free(confname);
}

static void on_assistant_prepare(GtkWidget * assistant, GtkWidget * page,
				 gpointer data)
{
	struct wizard_data *w_data = data;

	gint page_number =
	    gtk_assistant_get_current_page(GTK_ASSISTANT(assistant));

	if (page_number == w_data->bridges_page
	    || page_number == w_data->hs_page) {
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

	if (page_number == w_data->adv_page) {
		gtk_assistant_set_page_type(GTK_ASSISTANT(assistant), page,
					    GTK_ASSISTANT_PAGE_CONFIRM);
		return;
	}
}

static void on_ports_entry_changed(GtkWidget * widget, gpointer data)
{
	struct wizard_data *w_data = data;
	GtkAssistant *assistant = GTK_ASSISTANT(w_data->assistant);
	gint page_number;
	GtkWidget *cur_page;
	const gchar *text;
	int port;

	page_number = gtk_assistant_get_current_page(assistant);
	cur_page = gtk_assistant_get_nth_page(assistant, page_number);

	text = gtk_entry_get_text(GTK_ENTRY(widget));

	if (strlen(text) < 4)
		goto invalid;

	for (int i = 0; i < strlen(text); i++)
		if (!g_ascii_isdigit(text[i]))
			goto invalid;

	/* >= 1024 are unprivileged ports so we allow only them. */
	port = atoi(text);
	if (port < 1024 || port > 65535)
		goto invalid;

	gtk_label_set_text(GTK_LABEL(w_data->ports_wrn),
			   "Choose Tor daemon ports");
	gtk_assistant_set_page_complete(assistant, cur_page, TRUE);
	return;

 invalid:
	gtk_label_set_text(GTK_LABEL(w_data->ports_wrn),
			   "Invalid port numbers!");
	gtk_assistant_set_page_complete(assistant, cur_page, FALSE);
	return;
}

static gint new_wizard_adv_page(struct wizard_data *w_data)
{
	GtkWidget *vbox;
	GtkWidget *s_hbox, *socksport_label;
	GtkWidget *c_hbox, *controlport_label;
	GtkWidget *d_hbox, *dnsport_label;
	GtkWidget *t_hbox, *transport_label;
	gint rv;

	vbox = gtk_vbox_new(TRUE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

	s_hbox = gtk_hbox_new(FALSE, 2);
	socksport_label = gtk_label_new("SocksPort: ");
	w_data->socksport_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w_data->socksport_entry),
			   g_strdup_printf("%d", w_data->socksport));

	gtk_box_pack_start(GTK_BOX(s_hbox), socksport_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(s_hbox), w_data->socksport_entry, FALSE,
			   FALSE, 0);

	c_hbox = gtk_hbox_new(FALSE, 2);
	controlport_label = gtk_label_new("ControlPort: ");
	w_data->controlport_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w_data->controlport_entry),
			   g_strdup_printf("%d", w_data->controlport));

	gtk_box_pack_start(GTK_BOX(c_hbox), controlport_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(c_hbox), w_data->controlport_entry, FALSE,
			   FALSE, 0);

	d_hbox = gtk_hbox_new(FALSE, 2);
	dnsport_label = gtk_label_new("DNSPort: ");
	w_data->dnsport_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w_data->dnsport_entry),
			   g_strdup_printf("%d", w_data->dnsport));

	gtk_box_pack_start(GTK_BOX(d_hbox), dnsport_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(d_hbox), w_data->dnsport_entry, FALSE, FALSE,
			   0);

	t_hbox = gtk_hbox_new(FALSE, 2);
	transport_label = gtk_label_new("TransPort: ");
	w_data->transport_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(w_data->transport_entry),
			   g_strdup_printf("%d", w_data->transport));

	gtk_box_pack_start(GTK_BOX(t_hbox), transport_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(t_hbox), w_data->transport_entry, FALSE,
			   FALSE, 0);

	w_data->ports_wrn = gtk_label_new("Choose Tor daemon ports");
	gtk_box_pack_start(GTK_BOX(vbox), w_data->ports_wrn, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), s_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), c_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), d_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), t_hbox, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(w_data->socksport_entry), "changed",
			 G_CALLBACK(on_ports_entry_changed), w_data);
	g_signal_connect(G_OBJECT(w_data->controlport_entry), "changed",
			 G_CALLBACK(on_ports_entry_changed), w_data);
	g_signal_connect(G_OBJECT(w_data->dnsport_entry), "changed",
			 G_CALLBACK(on_ports_entry_changed), w_data);
	g_signal_connect(G_OBJECT(w_data->transport_entry), "changed",
			 G_CALLBACK(on_ports_entry_changed), w_data);

	gtk_widget_show_all(vbox);
	rv = gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT
				     (w_data->assistant),
				     vbox, "Advanced Settings");

	return rv;
}

static void validate_hiddenservices_cb(GtkWidget * widget, gpointer data)
{
	(void)widget;
	(void)data;

	return;
}

static gint new_wizard_hs_page(struct wizard_data *w_data)
{
	/*
	   HiddenServiceDir /var/lib/tor/my_website/
	   HiddenServicePort 80 127.0.0.1:80

	 */
	GtkWidget *info, *vbox, *validate_btn;
	gint rv;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

	info =
	    gtk_label_new("Here you can configure Hidden Services for\n"
			  "this configuration");

	validate_btn = gtk_button_new_with_label("Validate");

	g_signal_connect(G_OBJECT(validate_btn), "clicked",
			 G_CALLBACK(validate_hiddenservices_cb), w_data);

	gtk_box_pack_start(GTK_BOX(vbox), info, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), validate_btn, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	rv = gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT
				     (w_data->assistant), vbox,
				     "Tor Hidden Services");
	return rv;
}

static void validate_bridges_cb(GtkWidget * widget, gpointer data)
{
	(void)widget;
	struct wizard_data *w_data = data;
	GtkAssistant *assistant = GTK_ASSISTANT(w_data->assistant);
	GtkWidget *cur_page;
	GtkTextIter start, end;
	GtkTextBuffer *buf;
	gchar *text, *tmpfile, *cmd = NULL, *bridges_torrc = NULL;
	gchar **split, brbuf[8192];
	gboolean valid = FALSE;
	gint fd;

	cur_page = gtk_assistant_get_nth_page(assistant, w_data->bridges_page);
	buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w_data->bridges_textview));

	gtk_text_buffer_get_bounds(buf, &start, &end);
	text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);

	/* Lazy check */
	if (strlen(text) < 5) {
		g_free(text);
		return;
	}

	split = g_strsplit(text, "\n", -1);

	int i = 0, l = 0;
	while (split[i] != NULL) {
		l += g_sprintf(brbuf + l, "bridge %s\n", split[i]);
		i++;
	}
	g_strfreev(split);

	bridges_torrc = gen_bridges_torrc(brbuf);

	tmpfile = g_strdup("/tmp/torrc-XXXXXX");

	fd = g_mkstemp(tmpfile);
	if (fd == -1) {
		g_warning("mkstemp: %s", g_strerror(errno));
		hildon_banner_show_information(NULL, NULL,
					       "Internal error creating tempfile");
		goto out;
	}

	switch (write(fd, bridges_torrc, strlen(bridges_torrc))) {
	case -1:
		g_warning("write: %s", g_strerror(errno));
		hildon_banner_show_information(NULL, NULL,
					       "Internal error writing tempfile");
		close(fd);
		goto out;
		break;
	default:
		close(fd);
		break;
	}

	cmd = g_strdup_printf("/usr/bin/tor --verify-config -f %s", tmpfile);
	switch (system(cmd)) {
	case 0:
		valid = TRUE;
		break;
	default:
		g_warning("Bridges are invalid");
		hildon_banner_show_information(NULL, NULL,
					       "Invalid bridge configuration");
		break;
	}

 out:
	g_unlink(tmpfile);
	g_free(tmpfile);
	g_free(bridges_torrc);
	g_free(text);
	g_free(cmd);
	gtk_assistant_set_page_complete(assistant, cur_page, valid);
}

static gint new_wizard_bridges_page(struct wizard_data *w_data)
{
	GtkWidget *info, *vbox, *validate_btn;
	gint rv;

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);

	info =
	    gtk_label_new
	    ("Here you can input/paste and validate bridges.\n"
	     "See https://bridges.torproject.org for more info.");

	w_data->bridges_textview = gtk_text_view_new();

	validate_btn = gtk_button_new_with_label("Validate");

	g_signal_connect(G_OBJECT(validate_btn), "clicked",
			 G_CALLBACK(validate_bridges_cb), w_data);

	gtk_box_pack_start(GTK_BOX(vbox), info, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->bridges_textview,
			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), validate_btn, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);

	rv = gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT
				     (w_data->assistant), vbox, "Tor Bridges");
	return rv;
}

static void on_br_chk_btn_toggled(GtkWidget * widget, gpointer data)
{
	(void)widget;
	struct wizard_data *w_data = data;
	GtkWidget *page;
	gboolean active;

	/* We assume page should be 0, as the checkbox is there */
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
}

static void on_hs_chk_btn_toggled(GtkWidget * widget, gpointer data)
{
	(void)widget;
	struct wizard_data *w_data = data;
	GtkWidget *page;
	gboolean active;

	/* We assume page should be 0, as the checkbox is there */
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

	w_data->hs_page = -1;
	w_data->has_hs = FALSE;
}

static void on_ad_chk_btn_toggled(GtkWidget * widget, gpointer data)
{
	(void)widget;
	struct wizard_data *w_data = data;
	GtkWidget *page;
	gboolean active;

	/* We assume page should be 0, as the checkbox is there */
	page = gtk_assistant_get_nth_page(GTK_ASSISTANT(w_data->assistant), 0);
	g_object_get(G_OBJECT(w_data->ad_chk), "active", &active, NULL);

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
}

static void on_conf_name_entry_changed(GtkWidget * widget, gpointer data)
{
	struct wizard_data *w_data = data;
	GtkAssistant *assistant = GTK_ASSISTANT(w_data->assistant);
	GtkWidget *cur_page;
	gint page_number;
	const gchar *text;
	gboolean valid = TRUE;

	page_number = gtk_assistant_get_current_page(assistant);
	cur_page = gtk_assistant_get_nth_page(assistant, page_number);
	text = gtk_entry_get_text(GTK_ENTRY(widget));

	/* TODO: Also check for name clash if we're not editing an existing cfg */
	if (text && *text) {
		/* For sanity's sake, we'll only allow alphanumeric names */
		for (int i = 0; i < strlen(text); i++) {
			if (!g_ascii_isalnum(text[i])) {
				valid = FALSE;
				break;
			}
		}
	} else {
		valid = FALSE;
	}

	gtk_assistant_set_page_complete(assistant, cur_page, valid);
}

static void new_wizard_main_page(struct wizard_data *w_data)
{
	GtkWidget *vbox, *box, *info, *name_label;

	vbox = gtk_vbox_new(FALSE, 5);
	box = gtk_hbox_new(FALSE, 12);

	gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
	gtk_container_set_border_width(GTK_CONTAINER(box), 12);

	info = gtk_label_new("This wizard will help you configure Tor");

	name_label = gtk_label_new("Configuration name:");

	w_data->name_entry = gtk_entry_new();

	g_signal_connect(G_OBJECT(w_data->name_entry), "changed",
			 G_CALLBACK(on_conf_name_entry_changed), w_data);

	w_data->transproxy_chk =
	    gtk_check_button_new_with_label("Enable Transparent Proxying");
	w_data->br_chk = gtk_check_button_new_with_label("Use Bridges");
	w_data->hs_chk = gtk_check_button_new_with_label("Use Hidden Services");
	w_data->ad_chk = gtk_check_button_new_with_label("Advanced Settings");
	/* TODO: Implement */
	gtk_widget_set_sensitive(w_data->hs_chk, FALSE);

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
	gtk_box_pack_start(GTK_BOX(vbox), w_data->transproxy_chk, FALSE, FALSE,
			   0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->br_chk, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->hs_chk, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), w_data->ad_chk, FALSE, FALSE, 0);

	gtk_widget_show_all(vbox);

	gtk_assistant_append_page(GTK_ASSISTANT(w_data->assistant), vbox);
	gtk_assistant_set_page_title(GTK_ASSISTANT
				     (w_data->assistant), vbox,
				     "Tor Configuration Wizard");
	gtk_assistant_set_page_type(GTK_ASSISTANT(w_data->assistant),
				    vbox, GTK_ASSISTANT_PAGE_CONFIRM);
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
		w_data->bridges_data = NULL;
		w_data->hs_data = NULL;
		w_data->socksport_entry = NULL;
		w_data->socksport = 9050;
		w_data->controlport_entry = NULL;
		w_data->controlport = 9051;
		w_data->dnsport_entry = NULL;
		w_data->dnsport = 5353;
		w_data->transport_entry = NULL;
		w_data->transport = 9040;
	} else {
		w_data = config_data;
	}

	w_data->assistant = gtk_assistant_new();

	gtk_window_set_title(GTK_WINDOW(w_data->assistant),
			     "Tor configuration");

	gtk_assistant_set_forward_page_func(GTK_ASSISTANT
					    (w_data->assistant),
					    find_next_wizard_page,
					    w_data, NULL);

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

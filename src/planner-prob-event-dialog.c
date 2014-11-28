/*
 * planner-prob-event-dialog.c
 *
 *  Created on: Nov 14, 2013
 *      Author: root
 */

#include <config.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> /* for atoi */
#include <glib/gi18n.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <libplanner/mrp-object.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-private.h>
#include "libplanner/mrp-paths.h"
/*#include "planner-cell-renderer-list.h"
#include "planner-assignment-model.h"
#include "planner-predecessor-model.h"
#include "planner-task-cmd.h"
#include "planner-format.h"
#include "planner-popup-button.h"
#include "planner-task-date-widget.h"*/
#include "planner-task-dialog.h"

typedef struct {
	PlannerWindow *main_window;
	GtkWidget     *dialog;

	GtkWidget     *a_checkbutton;
	GtkWidget     *b_checkbutton;
	GtkWidget     *c_checkbutton;
	GtkWidget     *d_checkbutton;
	GtkWidget     *e_checkbutton;
} DialogData;


static void            prob_event_dialog_close_clicked_cb               (GtkWidget               *w,
								   DialogData              *data);
static void            prob_event_dialog_parent_destroy_cb              (GtkWidget *parent,
			                       GtkWidget *dialog);
static void            prob_event_dialog_check_button_toggled_cb        (GtkWidget               *w,
								   DialogData              *data);
static void            prob_event_dialog_setup_widgets                  (DialogData *data,
			                       GladeXML   *glade);

#define DIALOG_GET_DATA(d) g_object_get_data ((GObject*)d, "data")


static void
prob_event_dialog_close_clicked_cb (GtkWidget *w, DialogData *data)
{
	gtk_widget_destroy (data->dialog);
}

static void
prob_event_dialog_check_button_toggled_cb (GtkWidget *w, DialogData *data)
{


}

static void
prob_event_dialog_parent_destroy_cb (GtkWidget *parent,
			       GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
}

GtkWidget *
planner_prob_event_dialog_new (PlannerWindow *window)
{
	DialogData   *data;
	GladeXML     *glade;
	GtkWidget    *dialog;
	GtkWidget    *w;
	gchar        *filename;

	filename = mrp_paths_get_glade_dir ("prob-event-dialog.glade");
	glade = glade_xml_new (filename, NULL, NULL);
	g_free (filename);

	if (!glade) {
		g_warning ("Could not create probability event dialog.");
		return NULL;
	}

	dialog = glade_xml_get_widget (glade, "prob_event_dialog");

	data = g_new0 (DialogData, 1);

	data->main_window = window;
	data->dialog = dialog;

	g_signal_connect_object (window,
				 "destroy",
				 G_CALLBACK (prob_event_dialog_parent_destroy_cb),
				 dialog,
				 0);

	g_object_set_data_full (G_OBJECT (dialog),
				"data", data,
				g_free);

	prob_event_dialog_setup_widgets (data, glade);

	return dialog;
}

static void
prob_event_dialog_setup_widgets (DialogData *data,
			   GladeXML   *glade)
{
	GtkWidget    *w;

	//get close_button in glade file,connect a "clicked" signal
	w = glade_xml_get_widget (glade, "close_button");
	g_signal_connect (w,
			  "clicked",
			  G_CALLBACK (prob_event_dialog_close_clicked_cb),
			  data);

	//get every x_checkbutton in glade file, connect a "toggled" signal each
	data->a_checkbutton = glade_xml_get_widget (glade, "a_checkbutton");
	g_signal_connect (data->a_checkbutton,
			  "toggled",
			  G_CALLBACK (prob_event_dialog_check_button_toggled_cb),
			  data);

	data->b_checkbutton = glade_xml_get_widget (glade, "b_checkbutton");
	g_signal_connect (data->b_checkbutton,
			  "toggled",
			  G_CALLBACK (prob_event_dialog_check_button_toggled_cb),
			  data);

	data->c_checkbutton = glade_xml_get_widget (glade, "c_checkbutton");
	g_signal_connect (data->c_checkbutton,
			  "toggled",
			  G_CALLBACK (prob_event_dialog_check_button_toggled_cb),
			  data);

	data->d_checkbutton = glade_xml_get_widget (glade, "d_checkbutton");
	g_signal_connect (data->d_checkbutton,
			  "toggled",
			  G_CALLBACK (prob_event_dialog_check_button_toggled_cb),
			  data);

	data->e_checkbutton = glade_xml_get_widget (glade, "e_checkbutton");
	g_signal_connect (data->e_checkbutton,
			  "toggled",
			  G_CALLBACK (prob_event_dialog_check_button_toggled_cb),
			  data);


}





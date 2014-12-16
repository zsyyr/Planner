/*
 * planner-show-view.c
 *
 *  Created on: Nov 11, 2013
 *      Author: root
 */

#include <config.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "libplanner/mrp-task.h"
#include "libplanner/mrp-paths.h"
#include "planner-show-view.h"
#include "planner-cell-renderer-date.h"
#include "planner-task-dialog.h"
#include "planner-resource-dialog.h"
#include "planner-gantt-model.h"
#include "planner-gantt-chart.h"
#include "planner-gantt-print.h"
#include "planner-conf.h"
#include "planner-prob-event-dialog.h"
#include "planner-simulat.h"
#include "planner-format.h"
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomecanvas/gnome-canvas-line.h>
#include <glade/glade.h>

struct _PlannerShowViewPriv {
	GtkWidget             *paned; //separately shows pert view and dynamic gantt view
	GtkWidget             *tree;
	GtkWidget             *pertlayout;  //shows pert graph
	GtkWidget             *gantt; //shows dynamic gantt graph
	PlannerGanttPrintData *print_data;

	GtkWidget             *currenttaskentry;
	GtkWidget             *currentdelayentry;

	GtkWidget             *currentdurationentry;
	GtkWidget             *lastdurationentry;

	GtkWidget             *deleteresourceentry;

	GtkWidget             *randomtogglebutton;
	GtkWidget             *deleteresourcetogglebutton;
	GtkWidget             *ganttcharttogglebutton;
	GtkWidget             *manulsettogglebutton;

	GtkWidget				 *nextstepbutton;
	GtkWidget				 *deleteresourcebutton;
	GtkWidget				 *displaytotaldurationbutton;
	GtkWidget			    *progressbar;
	GtkUIManager          *ui_manager;
	GtkActionGroup *actions; //stores accident events .etc

	mrptime lasttotalduration;
	mrptime firsttotalduration;

	guint                  merged_id;
	gulong                 expose_id;
};

//*******************
//*******************
//*******************
//adds methods' statements here
static void          show_view_finalize                  (GObject           *object);
static void          show_view_row_expanded              (GtkTreeView *tree_view,
			                  GtkTreeIter *iter,
			                  GtkTreePath *path,
			                  gpointer     data);//used for task row double clicked sensitive or expanded
static void          show_view_row_collapsed              (GtkTreeView *tree_view,
			                  GtkTreeIter *iter,
			                  GtkTreePath *path,
			                  gpointer     data);
static GtkWidget *   show_view_create_widget             (PlannerShowView  *view);


static void          show_view_edit_task_cb              (GtkAction         *action,
							   gpointer           data);
static void          show_view_add_prob_events_cb        (GtkAction *action,
			                   gpointer   data);
static gint
show_view_next_task_cb(GtkWidget         *button,
		                PlannerShowView   *view);


static void          show_view_activate                  (PlannerView       *view);
static void          show_view_deactivate                (PlannerView       *view);
static void          show_view_setup                     (PlannerView       *view,
							   PlannerWindow     *main_window);
static const gchar  *show_view_get_label                 (PlannerView       *view);
static const gchar  *show_view_get_menu_label            (PlannerView       *view);
static const gchar  *show_view_get_icon                  (PlannerView       *view);
static const gchar  *show_view_get_name                  (PlannerView       *view);
static GtkWidget    *show_view_get_widget                (PlannerView       *view);
static void          show_view_zoom_to_fit_cb            (GtkAction         *action,
							   gpointer           data);
static void          show_view_zoom_in_cb                (GtkAction         *action,
							   gpointer           data);
static void          show_view_zoom_out_cb               (GtkAction         *action,
							   gpointer           data);
static void          show_view_highlight_critical_cb     (GtkAction         *action,
							   gpointer           data);
static void          show_view_show_guidelines_cb	  (GtkAction         *action,
							   gpointer           data);
static void          show_view_nonstandard_days_cb       (GtkAction         *action,
							   gpointer           data);
static gboolean      show_view_expose_cb                 (GtkWidget         *widget,
							   GdkEventExpose    *event,
							   gpointer           user_data);
static void          show_view_update_row_height         (PlannerShowView *view);
static void          show_view_update_zoom_sensitivity   (PlannerShowView  *view);
static void          show_view_selection_changed_cb      (PlannerTaskTree   *tree,
							   PlannerShowView  *view);
static void          show_view_gantt_status_updated_cb   (PlannerGanttChart *gantt,
							   const gchar       *message,
							   PlannerShowView  *view);
static void          show_view_gantt_resource_clicked_cb (PlannerGanttChart *chart,
							   MrpResource       *resource,
							   PlannerShowView  *view);
static void          show_view_update_ui                 (PlannerShowView  *view);
static void          show_view_print_init                (PlannerView       *view,
							   PlannerPrintJob   *job);
static void          show_view_print                     (PlannerView       *view,
							   gint               page_nr);
static gint          show_view_print_get_n_pages         (PlannerView       *view);
static void          show_view_print_cleanup             (PlannerView       *view);
static gint			show_view_delete_resource_cb(GtkWidget         *button,  PlannerShowView   *view);
GList 					*getSortedTaskListByView(PlannerShowView   *view);

void				   select_deleteresource(GtkWidget *togglebutton,gpointer *view);
void				   select_random_task(GtkWidget *togglebutton,gpointer *view);
gboolean             timeout_callback(gpointer data);
void					displayTotalDuration(PlannerShowView *view);
//global variables
static PlannerViewClass *parent_class = NULL;

static gdouble progressbarvalue = 0;
static gint prosses = 0;
static gint tasklistlength = 1;

static gint currenttasknumber = -1;

static gint randomresourcenumber = 0;

static gint randomtasknumber = 0;

static GtkWidget *lastdurationentry;

static mrptime 			    lastduration;
static mrptime				firstduration;

 GList *firsttasklist = NULL;

 extern gint layoutexp;

static void
task_view_pert_chart_cb		(GtkAction       *action,
	     gpointer         data)
{

	GtkWidget    *layout;
	MrpTask    *root;
	GList *task_list;
	GList *l;
	MrpTaskManager *task_manager;
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;
	MrpProject *project;

	pertnodes = NULL;
	layoutexp = 0;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;
	layout = priv->pertlayout;
	project = planner_window_get_project(PLANNER_VIEW(view)->main_window);
	task_manager = imrp_project_get_task_manager(project);
	root = mrp_task_manager_get_root (task_manager);
	task_list = mrp_task_manager_get_all_tasks(task_manager);
	



	l = planner_pertchart_nodes_creat(task_list);
	task_manager_build_dependency_graph_for_node (task_manager);
	l = setPertNodesPosition();
	avoidCrossingNode();
	avoidCrossingLine();
	removeEmptyColumn();
	getposition(l);
	getArrowPosition(pertnodes);

	drawtask(layout,project);

}

static void
task_view_pert_chart_on_load	(PlannerShowView *view)
{

	GtkWidget    *layout;
	MrpTask    *root;
	GList *task_list;
	GList *l;
	MrpTaskManager *task_manager;
	//PlannerShowView     *view;
	PlannerShowViewPriv *priv;
	MrpProject *project;
	pertnodes = NULL;

	priv = view->priv;
	layout = priv->pertlayout;
	project = planner_window_get_project(PLANNER_VIEW(view)->main_window);
	task_manager = imrp_project_get_task_manager(project);
	root = mrp_task_manager_get_root (task_manager);
	task_list = mrp_task_manager_get_all_tasks(task_manager);

//	g_print("______________________22222222222222222222_______________________");
	l = planner_pertchart_nodes_creat(task_list);
	task_manager_build_dependency_graph_for_node (task_manager);
	l = setPertNodesPosition();
	avoidCrossingNode();
	avoidCrossingLine();
	removeEmptyColumn();
	getposition(l);
	getArrowPosition(pertnodes);

	drawtask(layout,project);


}

static const GtkActionEntry entries[] = {
		{ "EditTask",          NULL,                              N_("_Edit Task Properties..."),
		  "<shift><control>e", NULL,
		  G_CALLBACK (show_view_edit_task_cb) },
		{ "AddProbEvents",   "planner-stock-auto-assignment",   N_("_Add Probability Events..."),
		  "<shift><control>a", N_("Add probability events in project"),
		  G_CALLBACK (show_view_add_prob_events_cb) },

		  { "PertChart",   "planner-stock-pert-chart",   N_("_Add Probability Events..."),
		  "<shift><control>a", N_("Add probability events in project"),
		   G_CALLBACK (task_view_pert_chart_cb) },

		{ "ZoomToFit",       GTK_STOCK_ZOOM_FIT,                N_("Zoom To _Fit"),
	      NULL,                N_("Zoom to fit the entire project"),
	      G_CALLBACK (show_view_zoom_to_fit_cb) },
	    { "ZoomIn",          GTK_STOCK_ZOOM_IN,                 N_("_Zoom In"),
	      "<control>plus",     N_("Zoom in"),
	      G_CALLBACK (show_view_zoom_in_cb) },
	    { "ZoomOut",         GTK_STOCK_ZOOM_OUT,                N_("Zoom _Out"),
	      "<control>minus",    N_("Zoom out"),
	      G_CALLBACK (show_view_zoom_out_cb) },
};

static const GtkToggleActionEntry toggle_entries[] = {
	{ "HighlightCriticalTasks", NULL, N_("_Highlight Critical Tasks"),
	  NULL, NULL,
	  G_CALLBACK (show_view_highlight_critical_cb), FALSE },
	{ "ShowGuideLines", NULL, N_("_Show Guide Lines"),
	  NULL, NULL,
	  G_CALLBACK (show_view_show_guidelines_cb), FALSE },
	{ "NonstandardDays", NULL, N_("_Nonstandard Days"),
	  NULL, NULL,
	  G_CALLBACK (show_view_nonstandard_days_cb), FALSE }
};



G_DEFINE_TYPE (PlannerShowView, planner_show_view, PLANNER_TYPE_VIEW);


//methods' definitions below

static gboolean
show_view_chart_scroll_event (GtkWidget * gki, GdkEventScroll * event,
		PlannerShowView *view)
{
	gboolean dontpropagate = FALSE;
	gboolean can_in, can_out;
	PlannerShowViewPriv *priv;

	if (event->state & GDK_CONTROL_MASK) {
		priv = view->priv;
		planner_gantt_chart_can_zoom (PLANNER_GANTT_CHART (priv->gantt),
				&can_in, &can_out);
		switch (event->direction) {
			case GDK_SCROLL_UP: {
				dontpropagate = TRUE;
				if (can_in)
					show_view_zoom_in_cb  (NULL, view);
				break;
			}
			case GDK_SCROLL_DOWN:
				dontpropagate = TRUE;
				if (can_out)
					show_view_zoom_out_cb  (NULL, view);
			        break;
			default:
				break;
		}
	}

	return dontpropagate;
}

static void
planner_show_view_class_init (PlannerShowViewClass *klass)
{
	GObjectClass     *o_class;
	PlannerViewClass *view_class;

	parent_class = g_type_class_peek_parent (klass);

	o_class = (GObjectClass *) klass;
	view_class = PLANNER_VIEW_CLASS (klass);

	o_class->finalize = show_view_finalize;

	view_class->setup = show_view_setup;
	view_class->get_label = show_view_get_label;
	view_class->get_menu_label = show_view_get_menu_label;
	view_class->get_icon = show_view_get_icon;
	view_class->get_name = show_view_get_name;
	view_class->get_widget = show_view_get_widget;
	view_class->activate = show_view_activate;
	view_class->deactivate = show_view_deactivate;
	view_class->print_init = show_view_print_init;
	view_class->print_get_n_pages = show_view_print_get_n_pages;
	view_class->print = show_view_print;
	view_class->print_cleanup = show_view_print_cleanup;
}

static void
planner_show_view_init (PlannerShowView *view)
{
	view->priv = g_new0 (PlannerShowViewPriv, 1);
}

static void
show_view_finalize (GObject *object)
{
	PlannerShowView *view;

	view = PLANNER_SHOW_VIEW (object);

	if (PLANNER_VIEW (view)->activated) {
		show_view_deactivate (PLANNER_VIEW (view));
	}

	g_free (view->priv);

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		(*G_OBJECT_CLASS (parent_class)->finalize) (object);
	}
}

static void
show_view_activate (PlannerView *view)
{
	PlannerShowViewPriv *priv;
	gboolean              show_critical, show_nostd_days, show_guidelines;
	gchar                *filename;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	priv->actions = gtk_action_group_new ("ShowView");
	gtk_action_group_set_translation_domain (priv->actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (priv->actions,
				      entries,
				      G_N_ELEMENTS (entries),
				      view);
	gtk_action_group_add_toggle_actions (priv->actions,
					     toggle_entries,
					     G_N_ELEMENTS (toggle_entries),
					     view);

	gtk_ui_manager_insert_action_group (priv->ui_manager, priv->actions, 0);
	filename = mrp_paths_get_ui_dir ("show-view.ui");
	priv->merged_id = gtk_ui_manager_add_ui_from_file (priv->ui_manager,
							   filename,
							   NULL);
	g_free (filename);
	gtk_ui_manager_ensure_update (priv->ui_manager);

	/* Set the initial UI state. */
	show_critical = planner_gantt_chart_get_highlight_critical_tasks (
		PLANNER_GANTT_CHART (priv->gantt));

	show_nostd_days = planner_gantt_chart_get_nonstandard_days (
		PLANNER_GANTT_CHART (priv->gantt));

	show_guidelines = planner_gantt_chart_get_show_guidelines (
			PLANNER_GANTT_CHART (priv->gantt));

	planner_task_tree_set_highlight_critical (PLANNER_TASK_TREE (priv->tree),
						  show_critical);

	planner_task_tree_set_nonstandard_days (PLANNER_TASK_TREE (priv->tree),
						  show_nostd_days);

	gtk_toggle_action_set_active (
		GTK_TOGGLE_ACTION (gtk_action_group_get_action (priv->actions, "HighlightCriticalTasks")),
		show_critical);

	gtk_toggle_action_set_active (
		GTK_TOGGLE_ACTION (gtk_action_group_get_action (priv->actions, "NonstandardDays")),
		show_nostd_days);

	gtk_toggle_action_set_active (
		GTK_TOGGLE_ACTION (gtk_action_group_get_action (priv->actions, "ShowGuideLines")),
		show_guidelines);

	show_view_selection_changed_cb (PLANNER_TASK_TREE (priv->tree),
					 PLANNER_SHOW_VIEW (view));
	show_view_update_zoom_sensitivity (PLANNER_SHOW_VIEW (view));

	gtk_widget_grab_focus (priv->gantt);
}

static void
show_view_deactivate (PlannerView *view)
{
	PlannerShowViewPriv *priv;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	gtk_ui_manager_remove_ui (priv->ui_manager, priv->merged_id);
	gtk_ui_manager_remove_action_group (priv->ui_manager, priv->actions);
	g_object_unref (priv->actions);
	priv->actions = NULL;
}

static void
show_view_setup (PlannerView *view, PlannerWindow *main_window)
{
	PlannerShowViewPriv *priv;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	priv->ui_manager = planner_window_get_ui_manager (main_window);
}

static const gchar *
show_view_get_label (PlannerView *view)
{
	/* i18n: Label used for the sidebar. Please try to make it short and use
	 * a linebreak if necessary/possible.
	 */
	//return _("Show");
	return _("动态推演");
}

static const gchar *
show_view_get_menu_label (PlannerView *view)
{
	return _("_Show Chart");
}

static const gchar *
show_view_get_icon (PlannerView *view)
{
	static gchar *filename = NULL;

	if (!filename) {
		filename = mrp_paths_get_image_dir ("24_simulate.png");
	}

	return filename;
}

static const gchar *
show_view_get_name (PlannerView *view)
{
	return "show_view";
}

static GtkWidget *
show_view_get_widget (PlannerView *view)
{
	PlannerShowViewPriv *priv;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	if (priv->paned == NULL) {
		priv->paned = show_view_create_widget (PLANNER_SHOW_VIEW (view));
		gtk_widget_show_all (priv->paned);
	}

	return priv->paned;
}

static void
show_view_print_init (PlannerView     *view,
		       PlannerPrintJob *job)
{
	PlannerShowViewPriv *priv;
	gdouble          zoom;
	gboolean         show_critical;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	zoom = planner_gantt_chart_get_zoom (PLANNER_GANTT_CHART (priv->gantt));

	show_critical = planner_gantt_chart_get_highlight_critical_tasks (
		PLANNER_GANTT_CHART (priv->gantt));

	//priv->print_data = planner_gantt_print_data_new (view, job,
							 //GTK_TREE_VIEW (priv->tree),
							 //zoom, show_critical);
}

static void
show_view_print (PlannerView *view, gint page_nr)

{
	PlannerShowViewPriv *priv;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	planner_gantt_print_do (priv->print_data, page_nr);
}

static gint
show_view_print_get_n_pages (PlannerView *view)
{
	PlannerShowViewPriv *priv;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	return planner_gantt_print_get_n_pages (priv->print_data);
}

static void
show_view_print_cleanup (PlannerView *view)

{
	PlannerShowViewPriv *priv;

	priv = PLANNER_SHOW_VIEW (view)->priv;

	planner_gantt_print_data_free (priv->print_data);
	priv->print_data = NULL;
}

static void
show_view_project_loaded_cb (MrpProject  *project,
			      PlannerShowView *view)
{
	GtkTreeModel *model;
	GList *task_list;
	MrpTaskManager *task_manager;

	//g_free(pertnodes);//pretend more load action that this list add unexcepted tasks
	//g_free(firsttasklist);//the same as above

	firsttasklist = NULL;
	project = planner_window_get_project(PLANNER_VIEW (view) ->main_window);
	task_manager = imrp_project_get_task_manager(project);
	task_list = mrp_task_manager_get_all_tasks(task_manager);
	firsttasklist = sortTasklistsByFinishTime(task_list);
	tasklistlength = g_list_length(task_list);
	prosses = tasklistlength;
	model = GTK_TREE_MODEL (planner_gantt_model_new (project));

	planner_task_tree_set_model(PLANNER_TASK_TREE (view->priv->tree),
			PLANNER_GANTT_MODEL (model) );

	planner_gantt_chart_set_model(PLANNER_GANTT_CHART (view->priv->gantt),
			model);

	view->priv->expose_id = g_signal_connect_after (view->priv->gantt,
			"expose_event",
			G_CALLBACK (show_view_expose_cb),
			view);

	//task_view_pert_chart_on_load(view);

	g_object_unref(model);
}


static GtkWidget *
show_view_create_widget (PlannerShowView *view)
{
	PlannerShowViewPriv  *priv;
	GtkWidget        *pertlayout;
	GtkWidget        *tree;
	GtkWidget        *sw;
	GtkWidget        *frame;
	GtkWidget        *layout;
	GtkWidget        *vpaned;
	GtkAdjustment    *hadj, *vadj;
	GtkTreeModel     *model;
	MrpTaskManager   *task_manager;
	GtkLabel         *label;
	GtkEntry         *entry;
	GtkButton        *button;
	GtkWidget        *hbox,*vbox,*vbox2;
	GtkWidget		   *progressbar;

	GtkWidget *randomtogglebutton;
	GtkWidget *deleteresourcetogglebutton;
	GtkWidget *ganttcharttogglebutton;
	GtkWidget *manulsettogglebutton;
	GtkWidget *table;

	MrpProject *project;
	GtkTreeSelection *selection;
	GList *task_list;

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);

	task_manager = imrp_project_get_task_manager(project);
	priv = view->priv;

	g_signal_connect (project,
			  "loaded",
			  G_CALLBACK (show_view_project_loaded_cb),
			  view);

	model = GTK_TREE_MODEL (planner_gantt_model_new (project));

	tree = planner_task_tree_new (PLANNER_VIEW (view)->main_window,
				      PLANNER_GANTT_MODEL (model),
				      FALSE,
				      TRUE,

				      COL_WBS, _("WBS"),
				      COL_NAME, _("Name"),
				      COL_START, _("Start"),
				      COL_FINISH, _("Finish"),
				      COL_WORK, _("Work"),
				      COL_DURATION, _("Duration"),
				      COL_SLACK, _("Slack"),
				      COL_COST, _("Cost"),
				      COL_ASSIGNED_TO, _("Assigned to"),

				      COL_COMPLETE, _("% Complete"),
				      -1);
	priv->tree = tree;


	priv->gantt = planner_gantt_chart_new_with_model (model);
	g_object_set (priv->gantt,
				  "header_height", 50,
			      NULL);
	planner_gantt_chart_set_view (PLANNER_GANTT_CHART (priv->gantt),
				      PLANNER_TASK_TREE (tree));

	//*********************
	//events sensitive on gantt widget
	gtk_widget_set_events (GTK_WIDGET (priv->gantt), GDK_SCROLL_MASK);

	g_signal_connect (priv->gantt, "scroll-event",
                    G_CALLBACK (show_view_chart_scroll_event), view);

	g_object_unref (model);

	g_signal_connect (priv->gantt,
			  "status_updated",
			  G_CALLBACK (show_view_gantt_status_updated_cb),
			  view);

	g_signal_connect (priv->gantt,
			  "resource_clicked",
			  G_CALLBACK (show_view_gantt_resource_clicked_cb),
			  view);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	//g_signal_connect (tree,
			  //"style_set",
			  //G_CALLBACK (show_view_tree_style_set_cb),
			  //view);

	//*********************
	//interface design
	//GtkVPaned
	//  GtkFrame -> GtkScrollWindow ->GtkLayout (pert view showed here)A
	//  GtkHBox
	//    GtkVBox -> GtkHBox (show run-in-time data showed here)B
	//                 GtkLabel
	//                 GtkEntry
	//    GtkScrollWindow  (gantt view showed here)C

	//interface design in A
	//GtkScrollWindow, let its scrollbar displayed if needed, put the pert view inside
	layout = gtk_layout_new(NULL,NULL);
	priv->pertlayout = layout;

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (sw),
			        priv->pertlayout);

	//GtkFrame, put the GtkScrollWindow in the GtkFrame
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), sw);

	//GtkVPaned, add the GtkFrame in the GtkVPaned
	vpaned = gtk_vpaned_new ();
	gtk_paned_add1 (GTK_PANED (vpaned), frame);

	//interface design in B
    //GtkVBox, holds labels and entry and buttons
    vbox = gtk_vbox_new(FALSE, 0);
    vbox2 = gtk_vbox_new(FALSE,0);
    progressbar = gtk_progress_bar_new();
	//GtkLabel, "The Current Task Is: "
    label = gtk_label_new("当前执行审计任务: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);

    //GtkEntry, holds task name
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry), FALSE, FALSE, 0);
    priv->currenttaskentry = entry;

    //GtkLabel, "The Current Delay Is:: "
    label = gtk_label_new("当前审计延迟时间: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);

    //GtkEntry, holds task delay
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry), FALSE, FALSE, 0);
    priv->currentdelayentry = entry;

    //GtkLabel, "The Current duration is "
    label = gtk_label_new("当前审计总时间: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);

    //GtkEntry, holds total duration
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry), FALSE, FALSE, 0);
    priv->currentdurationentry = entry;

    //GtkLabel, "The Duration has changed :: "
    label = gtk_label_new("延迟后审计总时间: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);

    //GtkEntry, The Duration has changed to
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry), FALSE, FALSE, 0);
    priv->lastdurationentry = entry;

    lastdurationentry = entry;

	//GtkLabel, "delete resource"
	label = gtk_label_new("调整的审计人员: ");
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label), FALSE, FALSE, 0);

	//GtkEntry, delete resource
	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry), FALSE, FALSE, 0);
	priv->deleteresourceentry = entry;



	GtkWidget *labelChild;
	PangoFontDescription *font1;
	short fontSize = 8;


    //GtkButton, delete resource
    button = gtk_button_new_with_label("重大事项显示");
    GdkColor color;
       color.red = 50000;
       color.green = 20000;
       color.blue = 15000;
       gtk_widget_modify_bg(button, GTK_STATE_INSENSITIVE, &color);

      // font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       //pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
      // labelChild = gtk_bin_get_child(GTK_BIN(button));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了

    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (button),	FALSE,FALSE, 0);
    g_signal_connect (button,
       			  "clicked",
       			  G_CALLBACK (show_view_delete_resource_cb),
       			 view);
    priv->deleteresourcebutton = button;

    //total duration display
	button = gtk_button_new_with_label("审计总时间无延迟");
	GdkColor color1;
	color1.red = 50000;
	color1.green = 20000;
	color1.blue = 15000;
	gtk_widget_modify_bg(button, GTK_STATE_INSENSITIVE, &color1);

	 // font1 = pango_font_description_from_string("Sans");//"Sans"字体名
	     //  pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
	     //  labelChild = gtk_bin_get_child(GTK_BIN(button));//取出GtkButton里的label
	       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (button), FALSE,FALSE, 0);
	g_signal_connect(button, "clicked",
			G_CALLBACK (show_view_delete_resource_cb), view);
	priv->displaytotaldurationbutton = button;

    //GtkButton, go to next task
    button = gtk_button_new_with_label("执行下一审计任务");
    GdkColor color2;
    color2.red = 50000;
    color2.green = 10000;
    color2.blue = 10000;
    gtk_widget_modify_bg(button, GTK_STATE_INSENSITIVE, &color2);

   // font1 = pango_font_description_from_string("Sans");//"Sans"字体名
        // pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
        // labelChild = gtk_bin_get_child(GTK_BIN(button));//取出GtkButton里的label
         gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (button), FALSE,FALSE, 0);
    g_signal_connect (button,
    			  "clicked",
    			  G_CALLBACK (show_view_next_task_cb),
    			 view);
    priv->nextstepbutton = button;


    table = gtk_table_new(2, 2, FALSE);
    manulsettogglebutton = gtk_toggle_button_new_with_label("手动设置审计任务延迟");
    gtk_widget_set_size_request(manulsettogglebutton,5,7);



    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
    pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
    labelChild = gtk_bin_get_child(GTK_BIN( manulsettogglebutton));//取出GtkButton里的label
    gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了






    randomtogglebutton = gtk_toggle_button_new_with_label("随机设置审计任务延迟");

    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
       labelChild = gtk_bin_get_child(GTK_BIN( randomtogglebutton));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了

    deleteresourcetogglebutton = gtk_toggle_button_new_with_label("参审人员随机调整");
    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
       labelChild = gtk_bin_get_child(GTK_BIN(  deleteresourcetogglebutton));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了

    ganttcharttogglebutton = gtk_toggle_button_new_with_label("干特图路径选择");
    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
       labelChild = gtk_bin_get_child(GTK_BIN( ganttcharttogglebutton));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了


	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 2);
	gtk_table_attach_defaults(GTK_TABLE(table), manulsettogglebutton, 0, 1, 0,1);
	gtk_table_attach_defaults(GTK_TABLE(table), randomtogglebutton, 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), deleteresourcetogglebutton, 1,
			2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), ganttcharttogglebutton, 1, 2, 1,
			2);
	gtk_signal_connect(GTK_OBJECT(randomtogglebutton), "toggle",
				GTK_SIGNAL_FUNC(select_random_task), view);
	gtk_signal_connect(GTK_OBJECT(deleteresourcetogglebutton), "toggle",
			GTK_SIGNAL_FUNC(select_deleteresource), view);
	priv->manulsettogglebutton = manulsettogglebutton;
	priv->randomtogglebutton = randomtogglebutton;
	priv->deleteresourcetogglebutton = deleteresourcetogglebutton;
	priv->ganttcharttogglebutton = ganttcharttogglebutton;
	//gtk_table_attach_defaults(GTK_TABLE(table), labelprobability,1,2,2,3);
//   		   gtk_table_attach_defaults(GTK_TABLE(table), togglebutton,0,2,3,4);
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (table), FALSE, FALSE, 0);
	//GtkButton, auto optimization
	button = gtk_button_new_with_label("Auto Optimization");



    sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (sw),
				    priv->gantt);
	gtk_box_pack_start(GTK_BOX (vbox2), GTK_WIDGET (sw), TRUE, TRUE,0);
	gtk_widget_set_size_request(progressbar,150,36);
	gtk_box_pack_start(GTK_BOX (vbox2), GTK_WIDGET (progressbar), FALSE, FALSE, 0);
	priv->progressbar = progressbar;
	//put the GtkHBox in the GtkFrame, put the GtkVBox and GtkScrollWindow in the GtkHBox
	hbox = gtk_hbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX (hbox), GTK_WIDGET (vbox), FALSE,TRUE, 0);
	gtk_box_pack_end(GTK_BOX (hbox), GTK_WIDGET (vbox2), TRUE, TRUE, 10);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (hbox));

	//add the GtkFrame in the GtkVPaned
	gtk_paned_add2 (GTK_PANED (vpaned), frame);

	gtk_paned_set_position (GTK_PANED (vpaned), 250);

	g_signal_connect (tree,
			  "row_expanded",
			  G_CALLBACK (show_view_row_expanded),
			  priv->gantt);

	g_signal_connect (tree,
			  "row_collapsed",
			  G_CALLBACK (show_view_row_collapsed),
			  priv->gantt);

	gtk_tree_view_expand_all (GTK_TREE_VIEW (tree));





	return vpaned;

}

//*******************
//*******************
//*******************
//Command callbacks

static void
show_view_row_expanded (GtkTreeView *tree_view,
			 GtkTreeIter *iter,
			 GtkTreePath *path,
			 gpointer     data)
{
	PlannerGanttChart *gantt = data;

	planner_gantt_chart_expand_row (gantt, path);
}

static void
show_view_row_collapsed (GtkTreeView *tree_view,
			  GtkTreeIter *iter,
			  GtkTreePath *path,
			  gpointer     data)
{
	PlannerGanttChart *gantt = data;

	planner_gantt_chart_collapse_row (gantt, path);
}

static void show_view_edit_task_cb(GtkAction *action, gpointer data) {
	PlannerShowView *view;

	view = PLANNER_SHOW_VIEW (data);

	planner_task_tree_edit_task(PLANNER_TASK_TREE (view->priv->tree),
			PLANNER_TASK_DIALOG_PAGE_GENERAL);
}

static void show_view_add_prob_events_cb(GtkAction *action, gpointer data) {
	PlannerShowView *view;
	GtkWidget *dialog;

	view = PLANNER_SHOW_VIEW (data);

	dialog = planner_prob_event_dialog_new(view->parent.main_window);
	gtk_widget_show(dialog);
}

static void
show_view_zoom_to_fit_cb (GtkAction *action,
			   gpointer   data)
{
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;

	planner_gantt_chart_zoom_to_fit (PLANNER_GANTT_CHART (priv->gantt));

	show_view_update_zoom_sensitivity (view);
}

static void
show_view_zoom_in_cb (GtkAction *action,
		       gpointer   data)
{
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;

	planner_gantt_chart_zoom_in (PLANNER_GANTT_CHART (priv->gantt));

	show_view_update_zoom_sensitivity (view);
}

static void
show_view_zoom_out_cb (GtkAction *action,
			gpointer   data)
{
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;

	planner_gantt_chart_zoom_out (PLANNER_GANTT_CHART (priv->gantt));

	show_view_update_zoom_sensitivity (view);
}

static void
show_view_highlight_critical_cb (GtkAction *action,
				  gpointer   data)
{
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;
	gboolean         state;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	planner_gantt_chart_set_highlight_critical_tasks (
		PLANNER_GANTT_CHART (priv->gantt),
		state);

	//planner_task_tree_set_highlight_critical (
                //PLANNER_TASK_TREE (priv->tree), state);
}

static void
show_view_nonstandard_days_cb (GtkAction *action,
				  gpointer   data)
{
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;
	gboolean         state;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	planner_gantt_chart_set_nonstandard_days (
		PLANNER_GANTT_CHART (priv->gantt),
		state);
	planner_task_tree_set_nonstandard_days (
		PLANNER_TASK_TREE (priv->tree), state);
	//show_view_update_row_height (view);

}

static void
show_view_show_guidelines_cb (GtkAction *action,
				  gpointer   data)
{
	PlannerShowView     *view;
	PlannerShowViewPriv *priv;
	gboolean              state;

	view = PLANNER_SHOW_VIEW (data);
	priv = view->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION(action));

	planner_gantt_chart_set_show_guidelines (
		PLANNER_GANTT_CHART (priv->gantt),
		state);
}

static void
show_view_update_zoom_sensitivity (PlannerShowView *view)
{
	gboolean in, out;

	planner_gantt_chart_can_zoom (PLANNER_GANTT_CHART (view->priv->gantt),
				      &in,
				      &out);

	g_object_set (gtk_action_group_get_action (GTK_ACTION_GROUP(view->priv->actions),
						   "ZoomIn"),
		      "sensitive", in,
		      NULL);

	g_object_set (gtk_action_group_get_action (GTK_ACTION_GROUP(view->priv->actions),
						   "ZoomOut"),
		      "sensitive", out,
		      NULL);
}

static void
show_view_selection_changed_cb (PlannerTaskTree *tree, PlannerShowView *view)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	show_view_update_ui (view);
}

static void
show_view_gantt_status_updated_cb (PlannerGanttChart *gantt,
				    const gchar       *message,
				    PlannerShowView  *view)
{
	planner_window_set_status (PLANNER_VIEW (view)->main_window, message);
}

static void show_view_gantt_resource_clicked_cb(PlannerGanttChart *chart,
		MrpResource *resource,
		PlannerShowView *view)
{
	GtkWidget *dialog;

	dialog = planner_resource_dialog_new(PLANNER_VIEW (view) ->main_window,
			resource);
	gtk_widget_show(dialog);
}







//*******************
//*******************
//*******************
//Other methods

PlannerView *
planner_show_view_new (void)
{
	PlannerView *view;

	view = g_object_new(PLANNER_TYPE_SHOW_VIEW, NULL );

	return view;
}


static gboolean
show_view_expose_cb (GtkWidget      *widget,
		      GdkEventExpose *event,
		      gpointer        data)
{
	PlannerShowView     *view = PLANNER_SHOW_VIEW (data);

	show_view_update_row_height (view);

	g_signal_handler_disconnect (view->priv->gantt,
				     view->priv->expose_id);

	return FALSE;
}

static void
show_view_update_ui (PlannerShowView *view)
{
	PlannerShowViewPriv *priv;
	GList                *list, *l;
	gboolean              value;
	gboolean              rel_value = FALSE;
	gboolean              link_value = FALSE;
	gint                  count_value = 0;

	if (!PLANNER_VIEW (view)->activated) {
		return;
	}

	priv = view->priv;

	list = planner_task_tree_get_selected_tasks(
			PLANNER_TASK_TREE (priv->tree) );

	for (l = list; l; l = l->next) {
		if (mrp_task_has_relation(MRP_TASK (l->data) )) {
			rel_value = TRUE;
			break;
		}
	}

	for (l = list; l; l = l->next) {
		count_value++;
	}

	value = (list != NULL );
	link_value = (count_value >= 2);

	g_list_free(list);
}

static void
show_view_update_row_height (PlannerShowView *view)
{
	GtkWidget *gantt = view->priv->gantt;
	gint header_height = 20;

	g_object_set(gantt, "header_height", header_height, NULL );
}

//*******************
//*******************
//*******************
//go to next task methods

GList *
getSortedTaskListByView(PlannerShowView   *view)
{
	GList *task_list;
	GList *sortedtasks;
	MrpTaskManager *task_manager;
	MrpProject *project;

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);
	task_manager = imrp_project_get_task_manager(project);
	task_list = mrp_task_manager_get_all_tasks(task_manager);
	sortedtasks = sortTasklistsByFinishTime(task_list);
	return sortedtasks;
}

static guint  timertag;
static guint  timer;

static void
show_view_simul_task_delay_cb    (PlannerShowView   *view)
{
	GList *tasklist;
	MrpProject *project;
	gint probability;
	MrpTask *currenttask;
	gchar *currenttaskname;
	gint delayedtime;
	gint chengeddurationtime;
	gchar *str1;
	gchar *str2;
	PlannerShowViewPriv *priv;
	mrptime duration;
	mrptime delayedduration;
	currenttasknumber++;
	priv = view->priv;
//	tasklist = getSortedTaskListByView(view);
//	currenttask = g_list_nth_data(tasklist,currenttasknumber);
	currenttask = g_list_nth_data(firsttasklist,currenttasknumber);
	//setDelayedDuration(currenttask);
	probability = mrp_task_get_probability(currenttask);
	currenttaskname = mrp_task_get_name(currenttask);
	duration = mrp_task_get_duration(currenttask);
	g_printf("%s, %d",currenttaskname,probability);

	gtk_entry_set_text (GTK_ENTRY (priv->currenttaskentry), currenttaskname);

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);

//random task delay
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->randomtogglebutton))
			&& ((5 == currenttasknumber) || (1 == currenttasknumber)))
//		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->randomtogglebutton))
//					&& (randomtasknumber == currenttasknumber) && (randomtasknumber+3 == currenttasknumber))

	{
		//srand((int) time(0));
		//MrpTask *randomtask = getRandomTask(firsttasklist);

		delayedduration = duration*1.5;
		g_object_set (currenttask, "sched",1,NULL);
		g_object_set (currenttask, "duration", delayedduration, NULL);

		gtk_widget_set_sensitive(priv->deleteresourcebutton, FALSE);
		gchar *a = g_strconcat(currenttaskname, "当前审计任务被随机设置延迟!", NULL );
		gtk_button_set_label(priv->deleteresourcebutton, a);
	}

	//manual set task delay
	GdkColor color1;
	if(probability)
	{
		if (probability >= 30 && probability <= 60) {
			color1.red = 50000;
			color1.green = 20000;
			color1.blue = 15000;
			gtk_widget_modify_bg(priv->deleteresourcebutton, GTK_STATE_INSENSITIVE,	&color1);
			gtk_widget_modify_fg(priv->currentdelayentry, GTK_STATE_NORMAL,	&color1);
			gtk_widget_set_sensitive(priv->deleteresourcebutton, FALSE);
			gchar *a = g_strconcat(currenttaskname, "当前审计任务被手动设置延迟!延迟小于60%", NULL );
			gtk_button_set_label(priv->deleteresourcebutton, a);
		}
		else{
			if (probability > 60) {
				color1.red = 55000;
				color1.green = 5000;
				color1.blue = 5000;
				gtk_widget_modify_bg(priv->deleteresourcebutton, GTK_STATE_INSENSITIVE, &color1);
				gtk_widget_modify_bg(priv->currentdelayentry, GTK_STATE_NORMAL, &color1);
				gtk_widget_set_sensitive(priv->deleteresourcebutton, FALSE);
				gchar *a = g_strconcat(currenttaskname, "当前审计任务被手动设置延迟!延迟大于60%",
						NULL );
				gtk_button_set_label(priv->deleteresourcebutton, a);
			}
			else{
				color1.red = 5000;
				color1.green = 35000;
				color1.blue = 55000;
				gtk_widget_modify_bg(priv->deleteresourcebutton,	GTK_STATE_INSENSITIVE, &color1);

				gtk_widget_set_sensitive(priv->deleteresourcebutton, FALSE);
				gchar *a = g_strconcat(currenttaskname, "当前审计任务被手动设置延迟!延迟小于30%",NULL );
				gtk_button_set_label(priv->deleteresourcebutton, a);
			}

		}

		delayedduration = duration*(probability*0.01+1);
		g_object_set (currenttask, "sched",1,NULL);
		g_object_set (currenttask, "duration", delayedduration, NULL);
	}

	//set task delayed time
		delayedtime =(delayedduration - duration) / (60 * 60 * 8);
		if(delayedtime >= 0){
		 str1 = planner_format_float(delayedtime, 2, FALSE);
		 gtk_entry_set_text(GTK_ENTRY (priv->currentdelayentry), str1);
		}
		else{
			str1 = planner_format_float(0, 2, FALSE);
			gtk_entry_set_text(GTK_ENTRY (priv->currentdelayentry), str1);
		}


	//delete resource
//	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->deleteresourcetogglebutton))
//			&& (randomresourcenumber == currenttasknumber))
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->deleteresourcetogglebutton))
				&& (2 == currenttasknumber))
	{
		MrpResource *delresource = getDelRandomResourceFromTask(currenttask);

		gchar *s1 = mrp_task_get_name(currenttask);
		gchar *s2 = mrp_resource_get_name(delresource);
		g_printf("随机选取审计任务为 %s,退出该任务的审计人员为 %s\n", s1, s2);
		gtk_entry_set_text (GTK_ENTRY (priv->deleteresourceentry), s2);
		MrpAssignment *assignment = mrp_task_get_assignment(currenttask,delresource);
		mrp_object_removed(MRP_OBJECT (assignment));
		currenttaskname = mrp_task_get_name(currenttask);

		gtk_widget_set_sensitive(priv->deleteresourcebutton,FALSE);
		gchar *a = g_strconcat("审计任务",s1,"的",s2,"退出该任务执行!",NULL);
		gtk_button_set_label(priv->deleteresourcebutton, a);
	}


	//caculat the duration
	chengeddurationtime = totalduration(project);

	priv->lasttotalduration = chengeddurationtime;
	str2 = planner_format_float(chengeddurationtime, 2, FALSE);
	gtk_entry_set_text(GTK_ENTRY (priv->lastdurationentry), str2);
	displayTotalDuration(view);
	g_source_remove (timertag);
}

static void
show_view_timeout_remove_cb()
{
	g_printf("----show_view_simul_cb returns, and timeout removed!\n");
}



static gint
show_view_next_task_cb(GtkWidget         *button,
		                PlannerShowView   *view)
{
	GList *task_list;
	GList *sortedtasks;
	GList *l;
	MrpTaskManager *task_manager;
	PlannerShowViewPriv *priv;
	MrpProject *project;
	MrpTask *currenttask;
	gchar *str;
	gint currentdurationtime;
	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);
	task_manager = imrp_project_get_task_manager(project);
	task_list = mrp_task_manager_get_all_tasks(task_manager);
	priv = view->priv;

	//root = mrp_task_manager_get_root (task_manager);
	//task_list = mrp_task_manager_get_all_tasks(task_manager);
	//sortedtasks = sortTasklistsByFinishTime(task_list);
	tasklistlength--;

	if(!gtk_widget_get_sensitive(priv->deleteresourcebutton))
		gtk_widget_set_sensitive(priv->deleteresourcebutton,TRUE);

	if (tasklistlength < 0) {
		gtk_button_set_label(priv->nextstepbutton, "整体审计计划模拟推演完毕");
		gtk_widget_set_sensitive(priv->nextstepbutton, FALSE);
		return 0;
	}

	currentdurationtime = totalduration(project);

	priv->firsttotalduration = currentdurationtime;
	//g_print("%d",totalduration(project));
	str = planner_format_float(currentdurationtime, 2, FALSE);
	gtk_entry_set_text(GTK_ENTRY (priv->currentdurationentry), str);

	timertag = g_timeout_add_full(G_PRIORITY_HIGH, 1000,
			show_view_simul_task_delay_cb, view, show_view_timeout_remove_cb);

	timer = gtk_timeout_add(1000, timeout_callback, priv->progressbar);

	return 1;
}

static void
show_view_simul_delete_resource_cb    (PlannerShowView   *view)
{
	GList *tasklist;
	MrpProject *project;

	MrpTask *randomtask;
	gchar *currenttaskname;
	gint delayedtime;
	gint chengeddurationtime;
	gchar *str1;
	gchar *str2;
	PlannerShowViewPriv *priv;
	srand((int) time(0));

	currenttasknumber++;
	priv = view->priv;
	tasklist = getSortedTaskListByView(view);
	//randomtask = getRandomTask(tasklist);
	//deleteRandomResourceFromTask(tasklist);
	randomtask = getRandomTask(tasklist);

	MrpResource *delresource = getDelRandomResourceFromTask(randomtask);

	gchar *s1 = mrp_task_get_name(randomtask);
	gchar *s2 = mrp_resource_get_name(delresource);
	g_printf("the random task is %s,the deleted resource is %s\n",s1,s2);

	MrpAssignment *assignment = mrp_task_get_assignment (randomtask,delresource);
	mrp_object_removed (MRP_OBJECT (assignment));


	if(gtk_toggle_button_get_active(priv->ganttcharttogglebutton)){

		firstduration = totalduration(project);

		priv->firsttotalduration = firstduration;
		gchar *str2 = planner_format_float(firstduration, 2, FALSE);
		gtk_entry_set_text(GTK_ENTRY (priv->currentdurationentry), str2);
	}

	//currenttask = g_list_nth_data(firsttasklist,currenttasknumber);
	//setDelayedDuration(currenttask);
/*
	currenttaskname = mrp_task_get_name(randomtask);
	gtk_entry_set_text (GTK_ENTRY (priv->currenttaskentry), currenttaskname);

	delayedtime = mrp_task_get_duration(randomtask)/(60*60*8*2);
	str1 = planner_format_float (delayedtime, 2, FALSE);
	gtk_entry_set_text (GTK_ENTRY (priv->currentdelayentry), str1);

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);
	chengeddurationtime = totalduration(project);
	str2 = planner_format_float (chengeddurationtime, 2, FALSE);
	gtk_entry_set_text (GTK_ENTRY (priv->lastdurationentry), str2);

	gchar *a = g_strconcat(s1,"'s",s2,"resource has been deleted!",NULL);
	gtk_button_set_label(priv->deleteresourcebutton, a);

//	can_go_to_next_task = FALSE;
	g_source_remove (timertag);*/
}

gboolean timeout_callback(gpointer data) {
	//gdouble value;
	GString *text;
	//value = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(data) );
	gdouble interval = 1.0/prosses;
	progressbarvalue += interval;
//	if (value > 1.0) {
//		value = 0.0;
//	}
    gdouble val;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data), progressbarvalue);

	text = g_string_new(gtk_progress_bar_get_text(GTK_PROGRESS_BAR(data) ));
	if(progressbarvalue > 0.99)
		val = 1.0;
	else
		val = progressbarvalue;
	g_string_sprintf(text, "%d %%", (int) (val * 100));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(data), text->str);
	g_source_remove (timer);
	return TRUE;
}

static gint
show_view_delete_resource_cb(GtkWidget *button,PlannerShowView *view)
{
	GList *task_list;
	GList *sortedtasks;
	GList *l;
	MrpTaskManager *task_manager;
	PlannerShowViewPriv *priv;
	MrpProject *project;
	MrpTask *currenttask;
	gchar *str;
	gint currentdurationtime;
	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);
	task_manager = imrp_project_get_task_manager(project);
	task_list = mrp_task_manager_get_all_tasks(task_manager);
	priv = view->priv;

	//root = mrp_task_manager_get_root (task_manager);
	//task_list = mrp_task_manager_get_all_tasks(task_manager);
	//sortedtasks = sortTasklistsByFinishTime(task_list);
	tasklistlength--;

	if (!tasklistlength) {
		gtk_button_set_label(priv->deleteresourcebutton,"All tasks have finished");
		gtk_widget_set_sensitive(priv->deleteresourcebutton, FALSE);
		return 0;
	}

	currentdurationtime = totalduration(project);
	//g_print("%d",totalduration(project));
	str = planner_format_float (currentdurationtime, 2, FALSE);
	gtk_entry_set_text (GTK_ENTRY (priv->currentdurationentry), str);

	timertag = g_timeout_add_full(G_PRIORITY_HIGH, 2000,
			show_view_simul_delete_resource_cb, view,
			show_view_timeout_remove_cb);

	timer = gtk_timeout_add(1500, timeout_callback, priv->progressbar);

	return 1;
}

void
select_deleteresource(GtkWidget *togglebutton,gpointer *view)
{
	GList *tasklist = getSortedTaskListByView(view);
	//srand((int) time(0));
	randomresourcenumber = getRandomNumber(tasklist);

}

void
select_random_task(GtkWidget *togglebutton,gpointer *view)
{
	GList *tasklist = getSortedTaskListByView(view);
	//srand((int) time(0));
	randomtasknumber = getRandomNumber(tasklist);
}

void
button_set_message(GtkWidget *button,gchar *message)
{
	gboolean sensitive;
	sensitive = gtk_widget_get_sensitive(button);
	GdkColor color2;
	color2.red = 40000;
	color2.green = 50000;
	color2.blue = 30000;
	gtk_widget_modify_bg(button, GTK_STATE_INSENSITIVE, &color2);
	//if()
	gtk_widget_set_sensitive(button,FALSE);
}

void
displaylastduration(lastduration){
	gchar *str = planner_format_float(lastduration, 2, FALSE);
	gtk_entry_set_text(GTK_ENTRY (lastdurationentry), str);
}

void
displayTotalDuration(PlannerShowView *view)
{
	PlannerShowViewPriv *priv;
	gfloat miner;
	priv = view->priv;
	miner = (priv->lasttotalduration - priv->firsttotalduration);
g_printf("*********************%f,%f,%f\n",priv->lasttotalduration ,priv->firsttotalduration,miner);
	if(priv->lasttotalduration != priv->firsttotalduration){
		gtk_widget_set_sensitive(priv->displaytotaldurationbutton, FALSE);
		gchar *str2 = planner_format_float(miner, 2, FALSE);
		gchar *a = g_strconcat("总工期延长",str2,"天！",NULL );
		gtk_button_set_label(priv->displaytotaldurationbutton, a);
	}
}

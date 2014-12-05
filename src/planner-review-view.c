/*
 * planner-review-view.c
 *
 *  Created on: Apr 9, 2014
 *      Author: zms
 */


/*
 * planner-review-view.c
 *
 *  Created on: Nov 11, 2013
 *      Author: root
 */
#include <gio/gio.h>
#include <config.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "libplanner/mrp-task.h"
#include "libplanner/mrp-paths.h"
#include "planner-review-view.h"
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
#include "libplanner/mrp-qualification.h"
struct _PlannerReviewViewPriv {
	GtkWidget             *paned; //separately reviews pert view and dynamic gantt view
	GtkWidget             *tree;
	GtkWidget             *pertlayout;  //reviews pert graph
	GtkWidget             *gantt; //reviews dynamic gantt graph
	PlannerGanttPrintData *print_data;

	GtkWidget             *currenttaskentry;
	GtkWidget             *optitime;

	GtkWidget             *currentresource;
	GtkWidget             *optiresource;

	GtkWidget             *deleteresourceentry;

	GtkWidget             *randomtogglebutton;
	GtkWidget             *deleteresourcetogglebutton;
	GtkWidget             *ganttcharttogglebutton;
	GtkWidget             *setoptimizebutton;

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

struct WriteTaskInfo{
	gchar *taskname;
	gint taskid;

};
//*******************
//*******************
//*******************
//adds methods' statements here
static void          review_view_finalize                  (GObject           *object);
static void          review_view_row_expanded              (GtkTreeView *tree_view,
			                  GtkTreeIter *iter,
			                  GtkTreePath *path,
			                  gpointer     data);//used for task row double clicked sensitive or expanded
static void          review_view_row_collapsed              (GtkTreeView *tree_view,
			                  GtkTreeIter *iter,
			                  GtkTreePath *path,
			                  gpointer     data);
static GtkWidget *   review_view_create_widget             (PlannerReviewView  *view);


static void          review_view_edit_task_cb              (GtkAction         *action,
							   gpointer           data);
static void          review_view_add_prob_events_cb        (GtkAction *action,
			                   gpointer   data);



static void          review_view_activate                  (PlannerView       *view);
static void          review_view_deactivate                (PlannerView       *view);
static void          review_view_setup                     (PlannerView       *view,
							   PlannerWindow     *main_window);
static const gchar  *review_view_get_label                 (PlannerView       *view);
static const gchar  *review_view_get_menu_label            (PlannerView       *view);
static const gchar  *review_view_get_icon                  (PlannerView       *view);
static const gchar  *review_view_get_name                  (PlannerView       *view);
static GtkWidget    *review_view_get_widget                (PlannerView       *view);
static void          review_view_zoom_to_fit_cb            (GtkAction         *action,
							   gpointer           data);
static void          review_view_zoom_in_cb                (GtkAction         *action,
							   gpointer           data);
static void          review_view_zoom_out_cb               (GtkAction         *action,
							   gpointer           data);
static void          review_view_highlight_critical_cb     (GtkAction         *action,
							   gpointer           data);
static void          review_view_review_guidelines_cb	  (GtkAction         *action,
							   gpointer           data);
static void          review_view_nonstandard_days_cb       (GtkAction         *action,
							   gpointer           data);
static gboolean      review_view_expose_cb                 (GtkWidget         *widget,
							   GdkEventExpose    *event,
							   gpointer           user_data);
static void          review_view_update_row_height         (PlannerReviewView *view);
static void          review_view_update_zoom_sensitivity   (PlannerReviewView  *view);
static void          review_view_selection_changed_cb      (PlannerTaskTree   *tree,
							   PlannerReviewView  *view);
static void          review_view_gantt_status_updated_cb   (PlannerGanttChart *gantt,
							   const gchar       *message,
							   PlannerReviewView  *view);
static void          review_view_gantt_resource_clicked_cb (PlannerGanttChart *chart,
							   MrpResource       *resource,
							   PlannerReviewView  *view);
static void          review_view_update_ui                 (PlannerReviewView  *view);
static void          review_view_print_init                (PlannerView       *view,
							   PlannerPrintJob   *job);
static void          review_view_print                     (PlannerView       *view,
							   gint               page_nr);
static gint          review_view_print_get_n_pages         (PlannerView       *view);
static void          review_view_print_cleanup             (PlannerView       *view);

static void 			get_copy_project_cb(GtkWidget         *button,
        PlannerReviewView   *view);
static void 			read_file_cb(GtkWidget *button,PlannerReviewView *view);
//global variables
static PlannerViewClass *parent_class = NULL;



static gint tasklistlength = 1;




static GtkWidget *lastdurationentry;



extern GList *firsttasklist;

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
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;
	MrpProject *project;

	pertnodes = NULL;
	layoutexp = 0;

	view = PLANNER_REVIEW_VIEW (data);
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
task_view_pert_chart_on_load	(PlannerReviewView *view)
{

	GtkWidget    *layout;
	MrpTask    *root;
	GList *task_list;
	GList *l;
	MrpTaskManager *task_manager;
	//PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;
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
		  G_CALLBACK (review_view_edit_task_cb) },
		{ "AddProbEvents",   "planner-stock-auto-assignment",   N_("_Add Probability Events..."),
		  "<shift><control>a", N_("Add probability events in project"),
		  G_CALLBACK (review_view_add_prob_events_cb) },

		  { "PertChart",   "planner-stock-pert-chart",   N_("_Add Probability Events..."),
		  "<shift><control>a", N_("Add probability events in project"),
		   G_CALLBACK (task_view_pert_chart_cb) },

		{ "ZoomToFit",       GTK_STOCK_ZOOM_FIT,                N_("Zoom To _Fit"),
	      NULL,                N_("Zoom to fit the entire project"),
	      G_CALLBACK (review_view_zoom_to_fit_cb) },
	    { "ZoomIn",          GTK_STOCK_ZOOM_IN,                 N_("_Zoom In"),
	      "<control>plus",     N_("Zoom in"),
	      G_CALLBACK (review_view_zoom_in_cb) },
	    { "ZoomOut",         GTK_STOCK_ZOOM_OUT,                N_("Zoom _Out"),
	      "<control>minus",    N_("Zoom out"),
	      G_CALLBACK (review_view_zoom_out_cb) },
};

static const GtkToggleActionEntry toggle_entries[] = {
	{ "HighlightCriticalTasks", NULL, N_("_Highlight Critical Tasks"),
	  NULL, NULL,
	  G_CALLBACK (review_view_highlight_critical_cb), FALSE },
	{ "ReviewGuideLines", NULL, N_("_Review Guide Lines"),
	  NULL, NULL,
	  G_CALLBACK (review_view_review_guidelines_cb), FALSE },
	{ "NonstandardDays", NULL, N_("_Nonstandard Days"),
	  NULL, NULL,
	  G_CALLBACK (review_view_nonstandard_days_cb), FALSE }
};



G_DEFINE_TYPE (PlannerReviewView, planner_review_view, PLANNER_TYPE_VIEW);


//methods' definitions below

static gboolean
review_view_chart_scroll_event (GtkWidget * gki, GdkEventScroll * event,
		PlannerReviewView *view)
{
	gboolean dontpropagate = FALSE;
	gboolean can_in, can_out;
	PlannerReviewViewPriv *priv;

	if (event->state & GDK_CONTROL_MASK) {
		priv = view->priv;
		planner_gantt_chart_can_zoom (PLANNER_GANTT_CHART (priv->gantt),
				&can_in, &can_out);
		switch (event->direction) {
			case GDK_SCROLL_UP: {
				dontpropagate = TRUE;
				if (can_in)
					review_view_zoom_in_cb  (NULL, view);
				break;
			}
			case GDK_SCROLL_DOWN:
				dontpropagate = TRUE;
				if (can_out)
					review_view_zoom_out_cb  (NULL, view);
			        break;
			default:
				break;
		}
	}

	return dontpropagate;
}

static void
planner_review_view_class_init (PlannerReviewViewClass *klass)
{
	GObjectClass     *o_class;
	PlannerViewClass *view_class;

	parent_class = g_type_class_peek_parent (klass);

	o_class = (GObjectClass *) klass;
	view_class = PLANNER_VIEW_CLASS (klass);

	o_class->finalize = review_view_finalize;

	view_class->setup = review_view_setup;
	view_class->get_label = review_view_get_label;
	view_class->get_menu_label = review_view_get_menu_label;
	view_class->get_icon = review_view_get_icon;
	view_class->get_name = review_view_get_name;
	view_class->get_widget = review_view_get_widget;
	view_class->activate = review_view_activate;
	view_class->deactivate = review_view_deactivate;
	view_class->print_init = review_view_print_init;
	view_class->print_get_n_pages = review_view_print_get_n_pages;
	view_class->print = review_view_print;
	view_class->print_cleanup = review_view_print_cleanup;
}

static void
planner_review_view_init (PlannerReviewView *view)
{
	view->priv = g_new0 (PlannerReviewViewPriv, 1);
}

static void
review_view_finalize (GObject *object)
{
	PlannerReviewView *view;

	view = PLANNER_REVIEW_VIEW (object);

	if (PLANNER_VIEW (view)->activated) {
		review_view_deactivate (PLANNER_VIEW (view));
	}

	g_free (view->priv);

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		(*G_OBJECT_CLASS (parent_class)->finalize) (object);
	}
}

static void
review_view_activate (PlannerView *view)
{
	PlannerReviewViewPriv *priv;
	gboolean              review_critical, review_nostd_days, review_guidelines;
	gchar                *filename;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	priv->actions = gtk_action_group_new ("ReviewView");
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
	filename = mrp_paths_get_ui_dir ("review-view.ui");
	priv->merged_id = gtk_ui_manager_add_ui_from_file (priv->ui_manager,
							   filename,
							   NULL);
	g_free (filename);
	gtk_ui_manager_ensure_update (priv->ui_manager);

	/* Set the initial UI state. */
	review_critical = planner_gantt_chart_get_highlight_critical_tasks (
		PLANNER_GANTT_CHART (priv->gantt));

	review_nostd_days = planner_gantt_chart_get_nonstandard_days (
		PLANNER_GANTT_CHART (priv->gantt));

	review_guidelines = planner_gantt_chart_get_show_guidelines (
			PLANNER_GANTT_CHART (priv->gantt));

	planner_task_tree_set_highlight_critical (PLANNER_TASK_TREE (priv->tree),
						  review_critical);

	planner_task_tree_set_nonstandard_days (PLANNER_TASK_TREE (priv->tree),
						  review_nostd_days);

	gtk_toggle_action_set_active (
		GTK_TOGGLE_ACTION (gtk_action_group_get_action (priv->actions, "HighlightCriticalTasks")),
		review_critical);

	gtk_toggle_action_set_active (
		GTK_TOGGLE_ACTION (gtk_action_group_get_action (priv->actions, "NonstandardDays")),
		review_nostd_days);

	gtk_toggle_action_set_active (
		GTK_TOGGLE_ACTION (gtk_action_group_get_action (priv->actions, "ReviewGuideLines")),
		review_guidelines);

	review_view_selection_changed_cb (PLANNER_TASK_TREE (priv->tree),
					 PLANNER_REVIEW_VIEW (view));
	review_view_update_zoom_sensitivity (PLANNER_REVIEW_VIEW (view));

	gtk_widget_grab_focus (priv->gantt);
}

static void
review_view_deactivate (PlannerView *view)
{
	PlannerReviewViewPriv *priv;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	gtk_ui_manager_remove_ui (priv->ui_manager, priv->merged_id);
	gtk_ui_manager_remove_action_group (priv->ui_manager, priv->actions);
	g_object_unref (priv->actions);
	priv->actions = NULL;
}

static void
review_view_setup (PlannerView *view, PlannerWindow *main_window)
{
	PlannerReviewViewPriv *priv;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	priv->ui_manager = planner_window_get_ui_manager (main_window);
}

static const gchar *
review_view_get_label (PlannerView *view)
{
	/* i18n: Label used for the sidebar. Please try to make it short and use
	 * a linebreak if necessary/possible.
	 */
	return _("智能反馈");
}

static const gchar *
review_view_get_menu_label (PlannerView *view)
{
	return _("_Review Chart");
}

static const gchar *
review_view_get_icon (PlannerView *view)
{
	static gchar *filename = NULL;

	if (!filename) {
		filename = mrp_paths_get_image_dir ("24_review.png");
	}

	return filename;
}

static const gchar *
review_view_get_name (PlannerView *view)
{
	return "review_view";
}

static GtkWidget *
review_view_get_widget (PlannerView *view)
{
	PlannerReviewViewPriv *priv;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	if (priv->paned == NULL) {
		priv->paned = review_view_create_widget (PLANNER_REVIEW_VIEW (view));
		gtk_widget_show_all (priv->paned);
	}

	return priv->paned;
}

static void
review_view_print_init (PlannerView     *view,
		       PlannerPrintJob *job)
{
	PlannerReviewViewPriv *priv;
	gdouble          zoom;
	gboolean         review_critical;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	zoom = planner_gantt_chart_get_zoom (PLANNER_GANTT_CHART (priv->gantt));

	review_critical = planner_gantt_chart_get_highlight_critical_tasks (
		PLANNER_GANTT_CHART (priv->gantt));

	//priv->print_data = planner_gantt_print_data_new (view, job,
							 //GTK_TREE_VIEW (priv->tree),
							 //zoom, review_critical);
}

static void
review_view_print (PlannerView *view, gint page_nr)

{
	PlannerReviewViewPriv *priv;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	planner_gantt_print_do (priv->print_data, page_nr);
}

static gint
review_view_print_get_n_pages (PlannerView *view)
{
	PlannerReviewViewPriv *priv;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	return planner_gantt_print_get_n_pages (priv->print_data);
}

static void
review_view_print_cleanup (PlannerView *view)

{
	PlannerReviewViewPriv *priv;

	priv = PLANNER_REVIEW_VIEW (view)->priv;

	planner_gantt_print_data_free (priv->print_data);
	priv->print_data = NULL;
}

static void
review_view_project_loaded_cb (MrpProject  *project,
			      PlannerReviewView *view)
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

	model = GTK_TREE_MODEL (planner_gantt_model_new (project));

	planner_task_tree_set_model(PLANNER_TASK_TREE (view->priv->tree),
			PLANNER_GANTT_MODEL (model) );

	planner_gantt_chart_set_model(PLANNER_GANTT_CHART (view->priv->gantt),
			model);

	view->priv->expose_id = g_signal_connect_after (view->priv->gantt,
			"expose_event",
			G_CALLBACK (review_view_expose_cb),
			view);

	//task_view_pert_chart_on_load(view);

	g_object_unref(model);
}


static GtkWidget *
review_view_create_widget (PlannerReviewView *view)
{
	PlannerReviewViewPriv  *priv;
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
	GtkWidget *setoptimizebutton;
	GtkWidget *table;

	MrpProject *project;
	GtkTreeSelection *selection;
	GList *task_list;

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);

	task_manager = imrp_project_get_task_manager(project);
	priv = view->priv;

	g_signal_connect (project,
			  "loaded",
			  G_CALLBACK (review_view_project_loaded_cb),
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
                    G_CALLBACK (review_view_chart_scroll_event), view);

	g_object_unref (model);

	g_signal_connect (priv->gantt,
			  "status_updated",
			  G_CALLBACK (review_view_gantt_status_updated_cb),
			  view);

	g_signal_connect (priv->gantt,
			  "resource_clicked",
			  G_CALLBACK (review_view_gantt_resource_clicked_cb),
			  view);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	//g_signal_connect (tree,
			  //"style_set",
			  //G_CALLBACK (review_view_tree_style_set_cb),
			  //view);

	//*********************
	//interface design
	//GtkVPaned
	//  GtkFrame -> GtkScrollWindow ->GtkLayout (pert view reviewed here)A
	//  GtkHBox
	//    GtkVBox -> GtkHBox (review run-in-time data reviewed here)B
	//                 GtkLabel
	//                 GtkEntry
	//    GtkScrollWindow  (gantt view reviewed here)C

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
    gtk_widget_set_size_request(progressbar,50,100);
    vbox2 = gtk_vbox_new(FALSE,0);
    progressbar = gtk_progress_bar_new();
	//GtkLabel, "The Current Task Is: "
    label = gtk_label_new("当前审计方案总时间");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label), TRUE, TRUE, 0);

    //GtkEntry, holds task name
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry),  TRUE, TRUE, 0);
    priv->currenttaskentry = entry;

    //GtkLabel, "The Current Delay Is:: "
    label = gtk_label_new("审计方案优化后总时间: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label),  TRUE, TRUE, 0);

    //GtkEntry, holds task delay
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry),  TRUE, TRUE, 0);
    priv->optitime = entry;

    //GtkLabel, "The Current duration is "
    label = gtk_label_new("当前审计方案动用审计兵力总数: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label),  TRUE, TRUE, 0);

    //GtkEntry, holds total duration
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry),  TRUE, TRUE, 0);
    priv->currentresource = entry;


    //GtkLabel, "The Duration has changed :: "
    label = gtk_label_new("审计方案优化后兵力总数: ");
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label),  TRUE, TRUE, 0);

    //GtkEntry, The Duration has changed to
    entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry),  TRUE, TRUE, 0);
    priv->optiresource = entry;

    lastdurationentry = entry;

	//GtkLabel, "delete resource"
	/*label = gtk_label_new("调整的审计人员: ");
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label),  TRUE, TRUE, 0);

	//GtkEntry, delete resource
	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (entry),  TRUE, TRUE, 0);
	priv->deleteresourceentry = entry;

    */

	GtkWidget *labelChild;
	PangoFontDescription *font1;
	short fontSize = 8;

	label = gtk_label_new("是否对整体方案进行优化 ");
	    gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (label),  TRUE, TRUE, 0);
    //GtkButton, delete resource
    /*button = gtk_button_new_with_label("是否对整体方案进行优化");
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
       			  G_CALLBACK (review_view_delete_resource_cb),
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
			G_CALLBACK (review_view_delete_resource_cb), view);
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
    			  G_CALLBACK (review_view_next_task_cb),
    			 view);
    priv->nextstepbutton = button;

*/
    table = gtk_table_new(1, 1, TRUE);
    setoptimizebutton = gtk_toggle_button_new_with_label("是");
    gtk_widget_set_size_request(setoptimizebutton,5,7);

    g_signal_connect (setoptimizebutton,
        			  "clicked",
        			  G_CALLBACK (get_copy_project_cb),
        			 view);
    deleteresourcetogglebutton = gtk_toggle_button_new_with_label("否");
    g_signal_connect (deleteresourcetogglebutton ,
            			  "clicked",
            			  G_CALLBACK (read_file_cb),
            			 view);

    /*font1 = pango_font_description_from_string("Sans");//"Sans"字体名
    pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
    labelChild = gtk_bin_get_child(GTK_BIN( manulsettogglebutton));//取出GtkButton里的label
    gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了






    randomtogglebutton = gtk_toggle_button_new_with_label("随机设置审计任务延迟");

    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
       labelChild = gtk_bin_get_child(GTK_BIN( randomtogglebutton));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了


    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
       labelChild = gtk_bin_get_child(GTK_BIN(  deleteresourcetogglebutton));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了

    ganttcharttogglebutton = gtk_toggle_button_new_with_label("干特图路径选择");
    font1 = pango_font_description_from_string("Sans");//"Sans"字体名
       pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
       labelChild = gtk_bin_get_child(GTK_BIN( ganttcharttogglebutton));//取出GtkButton里的label
       gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了
*/

	gtk_table_set_row_spacings(GTK_TABLE(table), 1);
	gtk_table_set_col_spacings(GTK_TABLE(table), 2);
	gtk_table_attach_defaults(GTK_TABLE(table), setoptimizebutton, 0, 1, 0,1);
	gtk_table_attach_defaults(GTK_TABLE(table), deleteresourcetogglebutton , 1, 2, 0, 1);

	priv->setoptimizebutton= setoptimizebutton;
	priv->randomtogglebutton = randomtogglebutton;
	priv->deleteresourcetogglebutton = deleteresourcetogglebutton;
	priv->ganttcharttogglebutton = ganttcharttogglebutton;
	//gtk_table_attach_defaults(GTK_TABLE(table), labelprobability,1,2,2,3);
//   		   gtk_table_attach_defaults(GTK_TABLE(table), togglebutton,0,2,3,4);
	gtk_box_pack_start(GTK_BOX (vbox), GTK_WIDGET (table),  TRUE, TRUE, 0);
	//GtkButton, auto optimization
	button = gtk_button_new_with_label("Auto Optimization");



    sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (sw),
				    priv->gantt);
	gtk_box_pack_start(GTK_BOX (vbox2), GTK_WIDGET (sw), TRUE, TRUE,0);
	gtk_widget_set_size_request(progressbar,150,36);
	//gtk_box_pack_start(GTK_BOX (vbox2), GTK_WIDGET (progressbar), FALSE, FALSE, 0);
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
			  G_CALLBACK (review_view_row_expanded),
			  priv->gantt);

	g_signal_connect (tree,
			  "row_collapsed",
			  G_CALLBACK (review_view_row_collapsed),
			  priv->gantt);

	gtk_tree_view_expand_all (GTK_TREE_VIEW (tree));





	return vpaned;

}

//*******************
//*******************
//*******************
//Command callbacks

static void
review_view_row_expanded (GtkTreeView *tree_view,
			 GtkTreeIter *iter,
			 GtkTreePath *path,
			 gpointer     data)
{
	PlannerGanttChart *gantt = data;

	planner_gantt_chart_expand_row (gantt, path);
}

static void
review_view_row_collapsed (GtkTreeView *tree_view,
			  GtkTreeIter *iter,
			  GtkTreePath *path,
			  gpointer     data)
{
	PlannerGanttChart *gantt = data;

	planner_gantt_chart_collapse_row (gantt, path);
}

static void review_view_edit_task_cb(GtkAction *action, gpointer data) {
	PlannerReviewView *view;

	view = PLANNER_REVIEW_VIEW (data);

	planner_task_tree_edit_task(PLANNER_TASK_TREE (view->priv->tree),
			PLANNER_TASK_DIALOG_PAGE_GENERAL);
}

static void review_view_add_prob_events_cb(GtkAction *action, gpointer data) {
	PlannerReviewView *view;
	GtkWidget *dialog;

	view = PLANNER_REVIEW_VIEW (data);

	dialog = planner_prob_event_dialog_new(view->parent.main_window);
	gtk_widget_show(dialog);
}

static void
review_view_zoom_to_fit_cb (GtkAction *action,
			   gpointer   data)
{
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;

	view = PLANNER_REVIEW_VIEW (data);
	priv = view->priv;

	planner_gantt_chart_zoom_to_fit (PLANNER_GANTT_CHART (priv->gantt));

	review_view_update_zoom_sensitivity (view);
}

static void
review_view_zoom_in_cb (GtkAction *action,
		       gpointer   data)
{
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;

	view = PLANNER_REVIEW_VIEW (data);
	priv = view->priv;

	planner_gantt_chart_zoom_in (PLANNER_GANTT_CHART (priv->gantt));

	review_view_update_zoom_sensitivity (view);
}

static void
review_view_zoom_out_cb (GtkAction *action,
			gpointer   data)
{
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;

	view = PLANNER_REVIEW_VIEW (data);
	priv = view->priv;

	planner_gantt_chart_zoom_out (PLANNER_GANTT_CHART (priv->gantt));

	review_view_update_zoom_sensitivity (view);
}

static void
review_view_highlight_critical_cb (GtkAction *action,
				  gpointer   data)
{
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;
	gboolean         state;

	view = PLANNER_REVIEW_VIEW (data);
	priv = view->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	planner_gantt_chart_set_highlight_critical_tasks (
		PLANNER_GANTT_CHART (priv->gantt),
		state);

	//planner_task_tree_set_highlight_critical (
                //PLANNER_TASK_TREE (priv->tree), state);
}

static void
review_view_nonstandard_days_cb (GtkAction *action,
				  gpointer   data)
{
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;
	gboolean         state;

	view = PLANNER_REVIEW_VIEW (data);
	priv = view->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	planner_gantt_chart_set_nonstandard_days (
		PLANNER_GANTT_CHART (priv->gantt),
		state);
	planner_task_tree_set_nonstandard_days (
		PLANNER_TASK_TREE (priv->tree), state);
	//review_view_update_row_height (view);

}

static void
review_view_review_guidelines_cb (GtkAction *action,
				  gpointer   data)
{
	PlannerReviewView     *view;
	PlannerReviewViewPriv *priv;
	gboolean              state;

	view = PLANNER_REVIEW_VIEW (data);
	priv = view->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION(action));

	planner_gantt_chart_set_show_guidelines (
		PLANNER_GANTT_CHART (priv->gantt),
		state);
}

static void
review_view_update_zoom_sensitivity (PlannerReviewView *view)
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
review_view_selection_changed_cb (PlannerTaskTree *tree, PlannerReviewView *view)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	review_view_update_ui (view);
}

static void
review_view_gantt_status_updated_cb (PlannerGanttChart *gantt,
				    const gchar       *message,
				    PlannerReviewView  *view)
{
	planner_window_set_status (PLANNER_VIEW (view)->main_window, message);
}

static void review_view_gantt_resource_clicked_cb(PlannerGanttChart *chart,
		MrpResource *resource,
		PlannerReviewView *view)
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
planner_review_view_new (void)
{
	PlannerView *view;

	view = g_object_new(PLANNER_TYPE_REVIEW_VIEW, NULL );

	return view;
}


static gboolean
review_view_expose_cb (GtkWidget      *widget,
		      GdkEventExpose *event,
		      gpointer        data)
{
	PlannerReviewView     *view = PLANNER_REVIEW_VIEW (data);

	review_view_update_row_height (view);

	g_signal_handler_disconnect (view->priv->gantt,
				     view->priv->expose_id);

	return FALSE;
}

static void
review_view_update_ui (PlannerReviewView *view)
{
	PlannerReviewViewPriv *priv;
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
review_view_update_row_height (PlannerReviewView *view)
{
	GtkWidget *gantt = view->priv->gantt;
	gint header_height = 20;

	g_object_set(gantt, "header_height", header_height, NULL );
}

//*******************
//*******************
//*******************
//go to next task methods
void display_array(GArray *array,int len,const char *prompt)
{
	int i = 0;
	printf("%s:\n",prompt);
	for(i = 0;i<len;i++)
	{
		g_printf("%d\n",g_array_index(array,int,i));
	}
}

void print_int_list(GList *list,const char *prompt)
{
	GList *l;
	printf("%s:\n",prompt);
	gint i = 0;
	for(l = list;l;l = l->next)
	{
		g_printf("%d",l->data);
		g_printf(" ");
	}
		g_printf("\n");
}

GList * get_all_task_ids(MrpProject *project)
{
	GList *task_ids = NULL;
	GList *all_tasks;
	GList *l = NULL;
	all_tasks = mrp_project_get_all_tasks(project);
	if(all_tasks)
	{
		for(l=all_tasks;l;l=l->next)
		{
			gint i = mrp_task_get_id(l->data);
			task_ids = g_list_append(task_ids,i);
		}
	}
	return task_ids;
}

GList * get_all_resources_qualification_ids(MrpProject *project)
{
	GList *resources_qualif_ids = NULL;
	GList *all_resources;
	GList *l = NULL;
	all_resources = mrp_project_get_resources(project);
	gint i = 0;
	gint rec_qualif_num[100];
	gchar *n = NULL;
	GList *qualifications = NULL;
	GList *recource_sum = NULL;
	MrpQualification *qualification = NULL;
	qualifications = mrp_project_get_qualifications(project);
	gint qualif_num = g_list_length(qualifications);
	gint ln;
	for(i = 0;i < 100;i++){
		rec_qualif_num[i] = 0;
	}
	if(all_resources)
	{
		for(l=all_resources;l;l=l->next)
		{
			mrp_object_get(MRP_OBJECT (l->data),"qualification",&qualification,NULL);
			if(qualification)
			{
				i = mrp_qualification_get_id(qualification);
				rec_qualif_num[i-1]++;
				n = mrp_qualification_get_name(qualification);
			}
			//this list is the id each resource's qualif has
			resources_qualif_ids = g_list_append(resources_qualif_ids,i);
			gchar *a = mrp_resource_get_name(l->data);

			g_printf("the %s resource's qualification id is %d,name is %s\n",a,i,n);
		}
		for(ln = 0;ln < qualif_num;ln++)
		{
			recource_sum = g_list_append(recource_sum,rec_qualif_num[ln]);
			g_printf("the qualification sum is %d\n",rec_qualif_num[ln]);
		}
		//this list is the qualifications how many resources used
		return recource_sum;
	}else
		return NULL;
}

GArray * get_all_task_qualification_ids(MrpProject *project)
{
	//GList *task_qualif_ids = NULL;
	GArray *task_qualif_ids = NULL;
	GList *all_tasks;
	GList *l = NULL;
	all_tasks = mrp_project_get_all_tasks(project);
	gint i = 0;
	gchar *n = NULL;
	MrpQualification *qualification = NULL;
	task_qualif_ids = g_array_new(FALSE,TRUE,sizeof(int));
	if(all_tasks)
	{
		for(l=all_tasks;l;l=l->next)
		{
			mrp_object_get(MRP_OBJECT (l->data),"qualification",&qualification,NULL);
			if(qualification)
			{
				i = mrp_qualification_get_id(qualification);
				n = mrp_qualification_get_name(qualification);
			}
			//task_qualif_ids = g_list_append(task_qualif_ids,i);
			task_qualif_ids = g_array_append_val(task_qualif_ids,i);
			gchar *a = mrp_task_get_name(l->data);

			g_printf("the %s task's qualification id is %d,name is %s\n",a,i,n);
			g_printf("%d\n",task_qualif_ids->len);
		}
	}

	return task_qualif_ids;
}

void print_all_task_names(MrpProject *project)
{
	GList *all_tasks;
	GList *l;
	gint i = 0;
	all_tasks = mrp_project_get_all_tasks(project);
	for(l=all_tasks;l;l=l->next)
			{
				g_printf("%s  ",mrp_task_get_name(l->data));
				g_printf("\n");
			}
}

void print_all_qualification_names(MrpProject *project)
{
	GList *all_qualifs;
	GList *l;
	gint i = 0;
	all_qualifs = mrp_project_get_qualifications(project);
	for(l=all_qualifs;l;l=l->next)
			{

				g_printf("%s, %d \n",mrp_qualification_get_name(l->data),mrp_qualification_get_id(l->data));
				g_printf("\n");
			}
}
void set_all_task_ids(MrpProject *project)
{
	GList *all_tasks;
	GList *l;
	gint i = 0;
	all_tasks = mrp_project_get_all_tasks(project);
	for(l=all_tasks;l;l=l->next)
		{
			i++;
			mrp_task_set_id(l->data,i);
		}
}

GList *get_all_tasks_duration(MrpProject *project)
{
	GList *all_tasks = NULL;
	GList *durations = NULL;
	GList *l;
	gint duration;
	all_tasks = mrp_project_get_all_tasks(project);
	for(l = all_tasks;l;l = l->next)
	{
		duration = mrp_task_get_duration(l->data);
		durations = g_list_append(durations,duration/(3600*8));
	}
	return durations;

}
gint writefile(gchar *path,GString *str)
{
	    g_type_init ();
	    GFile            *file;/* 文件抽象数据类型 */
	    GOutputStream    *fos; /* 用来写的 */
	    GError           *error = NULL;
	    file = g_file_new_for_path (path);
	    /* 获取输出流 */
	    fos = G_OUTPUT_STREAM (g_file_replace (file, NULL, FALSE,
	                                           G_FILE_CREATE_NONE,
	                                           NULL, &error));
	    if (!fos)
	    {
	        g_error ("%s", error->message);
	        g_error_free (error);
	        return 1;
	    }
	    /* 写入 */
	   g_output_stream_write_all (fos, str->str,str->len, NULL, NULL, NULL);
	    g_output_stream_flush (fos, NULL, NULL);


	    g_output_stream_close (fos, NULL, NULL);
	    g_object_unref (file);
	    return 0;
}

void write_duration(MrpProject *project)
{
	GList *durations;
	GList *l;
	gchar *path = "/home/zms/test/testwrite/time.txt";
	durations = get_all_tasks_duration(project);

	GString *str = g_string_new(NULL );
	gchar *c = g_malloc_n(10, sizeof(gchar));
	for (l = durations; l; l = l->next) {
		g_sprintf(c, "%d", l->data);
//	  	    	g_printf("%s\n",c);
		g_string_append(str, c);
		g_string_append(str, " ");
	}
	g_free(c);
	writefile(path, str);
	g_string_free(str, TRUE);
}

gboolean isinlist(GList *l,gint i){
	gboolean isin;
	for(;l;l = l->next){
		if(i == l->data)
		{
			isin = TRUE;
			return isin;
		}
	}
	isin = FALSE;
	return isin;
}

void write_resource_sum(MrpProject *project)
{
	GList *res_qualif_ids;
	GList *l = NULL;
	GList *ll =NULL;
	gint i;
	gint qualif_num = 0;
	gint rec_qualif_num[100];
	gchar *path = "/home/zms/test/testwrite/ResourceSum.txt";

	res_qualif_ids = get_all_resources_qualification_ids(project);
	GString *str = g_string_new(NULL );
	gchar *c = g_malloc_n(10, sizeof(gchar));
	GList *nl;
	for (nl = res_qualif_ids; nl; nl = nl->next) {
		g_sprintf(c, "%d", nl->data);
		//	  	    	g_printf("%s\n",c);
		g_string_append(str, c);
		g_string_append(str, " ");
	}
	g_string_append(str, "\n");
	g_free(c);


	writefile(path, str);
	g_string_free(str, TRUE);

}

void write_resource_odd(MrpProject *project)
{
	GArray *task_qualif_ids;
	GList *l = NULL;
	GList *ll =NULL;
	GList *qualifications = NULL;
	gint i;
	//gint qualif_num = 0;
	gint rec_qualif_num[100];
	gchar *path = "/home/zms/test/testwrite/ResourceOdd.txt";

	qualifications = mrp_project_get_qualifications(project);
	gint qualif_num = g_list_length(qualifications);
	task_qualif_ids = get_all_task_qualification_ids(project);
	GString *str = g_string_new(NULL );
	gchar *c = g_malloc_n(10, sizeof(gchar));
	gint nl;
	for (nl = 0; nl < task_qualif_ids->len; nl++) {
		gint a = g_array_index(task_qualif_ids,int,nl);
		for(i = 0;i < qualif_num;i++){
			if(i+1 == a)
				g_sprintf(c,"%d",1);
			else
				g_sprintf(c, "%d",0);
			//	  	    	g_printf("%s\n",c);
			g_string_append(str, c);
			g_string_append(str, " ");
		}
		g_string_append(str, "\n");
	}
	g_free(c);
	writefile(path, str);
	g_string_free(str, TRUE);

}


void write_precedence(MrpProject *project)
{
	GList *durations;
	GList *l = NULL;
	GList *ll =NULL;
	GList *presnumber = NULL;
	GList *pres = NULL;
	GList *alltasks;
	MrpTask *task;
	gint i;
	gint tasknumbers;
	alltasks = mrp_project_get_all_tasks(project);
	gchar *path = "/home/zms/test/testwrite/PrecedencRestrict.txt";
	durations = get_all_tasks_duration(project);
	tasknumbers = g_list_length(alltasks);
	GString *str = g_string_new(NULL );
	for (l = alltasks; l; l = l->next) {
		task = l->data;
//		pres = imrp_task_peek_predecessors(task);
		GList *pl;
		pres = NULL;
		for (pl = imrp_task_peek_predecessors(task); pl; pl = pl->next) {
			MrpTask *predecessor = mrp_relation_get_predecessor(pl->data);

			if (MRP_IS_TASK (predecessor)) {
				gchar *name = mrp_task_get_name(predecessor);
				g_print("%s\n", name);
				pres = g_list_append(pres,
						mrp_task_get_id(predecessor));
			}
		}
		for(ll = pres;ll;ll = ll->next)
					{
						g_printf("%d  ",ll->data);
					}
		presnumber = NULL;
		for (i = 0; i <= tasknumbers; i++) {
			ll = pres;
			if(isinlist(ll,i))
				presnumber = g_list_append(presnumber, 1);
			else
				presnumber = g_list_append(presnumber, 0);
		}
		presnumber = presnumber->next;
		for (ll = presnumber; ll; ll = ll->next) {
			g_printf("%d  ", ll->data);
		}
		g_printf("\n");

		gchar *c = g_malloc_n(10, sizeof(gchar));
		GList *nl;
		for (nl = presnumber; nl; nl = nl->next) {
			g_sprintf(c, "%d", nl->data);
			//	  	    	g_printf("%s\n",c);
			g_string_append(str, c);
			g_string_append(str, " ");
		}
		g_string_append(str, "\n");
		g_free(c);
	}

	writefile(path, str);
	g_string_free(str, TRUE);

}


gint readfile (gchar *path)
{

    g_type_init ();
    GArray *array;
    gint length = 0;
    gint number = 0;
    GFile            *file;/* 文件抽象数据类型 */
    GInputStream     *fis; /* 用来读的 */
    GError           *error = NULL;
    GDataInputStream *dis; /* 抽象输入流 */
    /* 创建file对象 */
    array = g_array_new(FALSE,TRUE,sizeof(int));
    file = g_file_new_for_path (path);

    /* 获取输入流 */
    fis = G_INPUT_STREAM (g_file_read (file, NULL, &error));
    if (!fis)
    {
        g_error ("%s", error->message);
        g_error_free (error);
        return 1;
    }

    /* 读取 */
    dis = g_data_input_stream_new (fis);
    gchar *line;
    while (TRUE)
    {
        line = g_data_input_stream_read_line (dis, NULL, NULL, &error);
//        gint32 number = g_data_input_stream_read_int32(dis,NULL,&error);
        if (error)
        {
            g_error ("%s", error->message);
            return 1;
        }
        if (!line)
            break;

        g_print ("%s\n", line);
        number = g_ascii_digit_value(*line);
        length++;
//        g_list_append(l,number);
        g_array_append_val(array,number);
//        g_print("%d\n",number);
        g_free (line);
    }
   display_array(array,length,"the int number");
    /* 清理 */
    g_input_stream_close (fis, NULL, NULL);
    g_object_unref (file);


    return 0;
}

static void read_file_cb(GtkWidget *button,PlannerReviewView *view)
{
	PlannerReviewViewPriv *priv;
	MrpProject *project;
	GList    *alltasks;
	GList    *l;
	GList    *ll;
	//gchar *filepath = "/home/zms/test/readfile";
	gchar *writefilepath = "/home/zms/test/testwrite";
	//readfile(filepath);
	set_all_task_ids(project);
	l = get_all_task_ids(project);
//	writefile(writefilepath);
	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);
	/*alltasks = mrp_project_get_all_tasks(project);
	set_all_task_ids(project);
	write_duration(project);
	write_precedence(project);
	MrpQualification *q = mrp_qualification_new();
	char *nn = mrp_qualification_get_name (q);
	gchar *a = "aaaaaaaa";
	g_object_set(q,"name",a,NULL);
//	 mrp_qualification_set_name(q,a);
	const gchar *n = NULL;

	mrp_object_get(q,"name",&n,NULL);
//	n = mrp_qualification_get_name(q);
	g_printf("%s\n",n);*/

	get_all_task_qualification_ids(project);
	print_all_qualification_names(project);
	//get_all_resources_qualification_ids(project);
	//write_resource_sum(project);
	write_resource_odd(project);
//		int i =0;
//		for(l = alltasks;l;l = l->next)
//		{
//			i++;
//			char *taskname = mrp_task_get_name(l->data);
//			if(taskname)
//				g_printf("%s\n",taskname);
//
//			else
//				g_print("no name %d\n",i);
//			mrp_task_set_id(l->data,i);
//			g_printf(mrp_task_get_id(l->data));
//			g_printf("\n");
//		}
//		for(l = alltasks;l;l = l->next)
//			{
//
//				//g_printf(mrp_task_get_name(l->data));
//				g_printf("%d\n",mrp_task_get_id(l->data));
//			}

//		system("/usr/local/MATLAB/R2010b/bin/matlab -nojvm -nodesktop -nodisplay -r ItemNoSource1 >test.out");
}

static void get_copy_project_cb(GtkWidget *button,PlannerReviewView *view)
{
	PlannerReviewViewPriv *priv;
	MrpProject *project;
	gchar      *old_uri;
	gchar      *opt_uri;
	gchar 		 *position;
	gchar		*test_uri;
	GError   *error = NULL;
	gboolean success;
	GList    *alltasks;
	GList    *l;
	gchar *filepath = "/home/zms/test/readfile";

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);

	old_uri = mrp_project_get_uri(project);
//	g_return_val_if_fail (MRP_IS_PROJECT (project), FALSE);
//	g_return_val_if_fail (old_uri != NULL && old_uri[0] != '\0', FALSE);

	position = strstr (old_uri, ".planner");
	if(position)
		opt_uri = g_strconcat(g_strndup(old_uri,position-old_uri),"_optimize.planner",NULL);
	else
		opt_uri = g_strconcat (old_uri, "_optimize.planner", NULL);
	g_printf(opt_uri);
	g_printf("\n");
	g_free(old_uri);
/*
	project_do_save (project, opt_uri,TRUE, &error);
	test_uri = "/home/zms/planner/examples/shenji.planner";
	mrp_project_load (project, test_uri, &error);*/

	print_all_task_names(project);
	set_all_task_ids(project);
	l = get_all_task_ids(project);
	print_int_list(l,"task ids");


}







/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2003-2005 Imendio AB
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <string.h>
#include <time.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libplanner/mrp-task.h>
#include "planner-task-view.h"
#include "libplanner/mrp-paths.h"
#include "planner-conf.h"
#include "planner-cell-renderer-date.h"
#include "planner-task-dialog.h"
#include "planner-property-dialog.h"
#include "planner-gantt-model.h"
#include "planner-task-tree.h"
#include "planner-table-print-sheet.h"
#include "planner-column-dialog.h"
#include "libplanner/mrp-task-manager.h"
#include "planner-gantt-row.h"
#include "planner-relation-arrow.h"
#include "libplanner/mrp-group-assignment.h"
//#include "libplanner/mrp-assignment.h"
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomecanvas/gnome-canvas-line.h>
#include <stdio.h>
#include <cairo.h>
#include "planner-show-view.h"
#include <math.h>

#include "planner-pert-chart.h"
//#include "planner-pertchartnode.h"


struct _PlannerTaskViewPriv {
	GtkWidget              *tree;
	GtkWidget              *frame;
	PlannerTablePrintSheet *print_sheet;
	GtkUIManager           *ui_manager;
	GtkActionGroup         *actions;
	guint                   merged_id;
};

static void          task_view_finalize                     (GObject         *object);
static void          task_view_activate                     (PlannerView     *view);
static void          task_view_deactivate                   (PlannerView     *view);
static void          task_view_setup                        (PlannerView     *view,
							     PlannerWindow   *main_window);
static const gchar  *task_view_get_label                    (PlannerView     *view);
static const gchar  *task_view_get_menu_label               (PlannerView     *view);
static const gchar  *task_view_get_icon                     (PlannerView     *view);
static const gchar  *task_view_get_name                     (PlannerView     *view);
static GtkWidget    *task_view_get_widget                   (PlannerView     *view);
static void          task_view_print_init                   (PlannerView     *view,
							     PlannerPrintJob *job);
static void          task_view_print                        (PlannerView     *view,
							     gint             page_nr);
static gint          task_view_print_get_n_pages            (PlannerView     *view);
static void          task_view_print_cleanup                (PlannerView     *view);
static void          task_view_tree_view_columns_changed_cb (GtkTreeView     *tree_view,
							     PlannerView     *view);
static void          task_view_tree_view_destroy_cb         (GtkTreeView     *tree_view,
							     PlannerView     *view);
static void          task_view_project_loaded_cb            (MrpProject      *project,
							     PlannerView     *view);
static void          task_view_insert_task_cb               (GtkAction       *action,
							     gpointer         data);
static void          task_view_insert_tasks_cb              (GtkAction       *action,
							     gpointer         data);
static void          task_view_remove_task_cb               (GtkAction       *action,
							     gpointer         data);
static void          task_view_edit_task_cb                 (GtkAction       *action,
							     gpointer         data);
static void          task_view_select_all_cb                (GtkAction       *action,
							     gpointer         data);
static void          task_view_unlink_task_cb               (GtkAction       *action,
							     gpointer         data);
static void          task_view_link_tasks_cb                (GtkAction       *action,
							     gpointer         data);
static void          task_view_indent_task_cb               (GtkAction       *action,
							     gpointer         data);
static void          task_view_move_task_up_cb              (GtkAction       *action,
							     gpointer         data);
static void          task_view_move_task_down_cb            (GtkAction       *action,
							     gpointer         data);
static void          task_view_unindent_task_cb             (GtkAction       *action,
							     gpointer         data);
static void          task_view_reset_constraint_cb          (GtkAction       *action,
							     gpointer         data);
static void          task_view_edit_custom_props_cb         (GtkAction       *action,
							     gpointer         data);
static void          task_view_highlight_critical_cb        (GtkAction       *action,
							     gpointer         data);
static void          task_view_nonstandard_days_cb          (GtkAction       *action,
							     gpointer         data);
static void          task_view_edit_columns_cb              (GtkAction       *action,
							     gpointer         data);
static void          task_view_arrange_resource_cb              (GtkAction       *action,
							     gpointer         data);
static void			 task_view_pert_chart_cb		(GtkAction       *action,
	     gpointer         data);
static void			 task_view_auto_assignment_cb 				(GtkAction       *action,
	     gpointer         data);
static void          task_view_selection_changed_cb         (PlannerTaskTree *tree,
							     PlannerView     *view);
static void          task_view_relations_changed_cb         (PlannerTaskTree *tree,
							     MrpTask         *task,
							     MrpRelation     *relation,
							     PlannerView     *view);
static void          task_view_update_ui                    (PlannerView     *view);
static void          task_view_save_columns                 (PlannerView     *view);
static void          task_view_load_columns                 (PlannerView     *view);


static PlannerViewClass *parent_class = NULL;

static const GtkActionEntry entries[] = {
	{ "InsertTask",      "planner-stock-insert-task",      N_("_Insert Task"),
	  "<Control>i",        N_("Insert a new task"),
          G_CALLBACK (task_view_insert_task_cb) },
	{ "InsertTasks",     "planner-stock-insert-task",      N_("In_sert Tasks..."),
	  NULL,                NULL,
	  G_CALLBACK (task_view_insert_tasks_cb) },
	{ "RemoveTask",      "planner-stock-remove-task",      N_("_Remove Task"),
	  "<Control>d",        N_("Remove the selected tasks"),
	  G_CALLBACK (task_view_remove_task_cb) },
	{ "EditTask",        NULL,                             N_("_Edit Task"),
	  "<Shift><Control>e", NULL,
	  G_CALLBACK (task_view_edit_task_cb) },
	{ "SelectAll",       NULL,                             N_("Select _All"),
	  "<Control>a",        N_("Select all tasks"),
	  G_CALLBACK (task_view_select_all_cb) },
	{ "UnlinkTask",      "planner-stock-unlink-task",      N_("_Unlink Task"),
	  NULL,                N_("Unlink the selected tasks"),
	  G_CALLBACK (task_view_unlink_task_cb) },
	{ "LinkTasks",       "planner-stock-link-task",        N_("_Link Tasks"),
	  NULL,                N_("Link the selected tasks"),
	  G_CALLBACK (task_view_link_tasks_cb) },
	{ "IndentTask",      "planner-stock-indent-task",      N_("I_ndent Task"),
	  "<Shift><Control>i", N_("Indent the selected tasks"),
	  G_CALLBACK (task_view_indent_task_cb) },
	{ "UnindentTask",    "planner-stock-unindent-task",    N_("Unin_dent Task"),
	  "<Shift><Control>u", N_("Unindent the selected tasks"),
	  G_CALLBACK (task_view_unindent_task_cb) },
	{ "MoveTaskUp",      "planner-stock-move-task-up",     N_("Move Task _Up"),
	  NULL,                N_("Move the selected tasks upwards"),
	  G_CALLBACK (task_view_move_task_up_cb) },
	{ "MoveTaskDown",    "planner-stock-move-task-down",   N_("Move Task Do_wn"),
	  NULL,                N_("Move the selected tasks downwards"),
	  G_CALLBACK (task_view_move_task_down_cb) },
	{ "ResetConstraint", "planner-stock-reset-constraint", N_("Reset _Constraint"),
          NULL,                NULL,
	  G_CALLBACK (task_view_reset_constraint_cb) },
	{ "EditCustomProps", GTK_STOCK_PROPERTIES,             N_("_Edit Custom Properties..."),
	  NULL,                NULL,
	  G_CALLBACK (task_view_edit_custom_props_cb) },
	{ "EditColumns",       NULL,                           N_("Edit _Visible Columns"),
	  NULL,                N_("Edit visible columns"),
	  G_CALLBACK (task_view_edit_columns_cb) },
	{ "ArrangeResource",	NULL,	N_("Arrange _Resource"),
	  NULL, NULL,G_CALLBACK (task_view_arrange_resource_cb) },

	{ "AutoAssignment",	"planner-stock-auto-assignment",	N_("Auto_Assignment"),
		  NULL, NULL,G_CALLBACK (task_view_auto_assignment_cb) },
	{ "PertChart",	"planner-stock-pert-chart",	N_("Pert_Chart"),
			  NULL, NULL,G_CALLBACK (task_view_pert_chart_cb) }
};

static const GtkToggleActionEntry toggle_entries[] = {
	{ "HighlightCriticalTasks", NULL, N_("_Highlight Critical Tasks"), NULL, NULL,
	  G_CALLBACK (task_view_highlight_critical_cb), FALSE },
	{ "NonstandardDays", NULL, N_("_Nonstandard Days"), NULL, NULL,
	  G_CALLBACK (task_view_nonstandard_days_cb), FALSE }
};

#define CRITICAL_PATH_KEY  "/views/task_view/highlight_critical_path"
#define NOSTDDAYS_PATH_KEY "/views/task_view/display_nonstandard_days"

G_DEFINE_TYPE (PlannerTaskView, planner_task_view, PLANNER_TYPE_VIEW);


static void
planner_task_view_class_init (PlannerTaskViewClass *klass)
{
	GObjectClass     *o_class;
	PlannerViewClass *view_class;

	parent_class = g_type_class_peek_parent (klass);

	o_class = (GObjectClass *) klass;
	view_class = PLANNER_VIEW_CLASS (klass);

	o_class->finalize = task_view_finalize;

	view_class->setup = task_view_setup;
	view_class->get_label = task_view_get_label;
	view_class->get_menu_label = task_view_get_menu_label;
	view_class->get_icon = task_view_get_icon;
	view_class->get_name = task_view_get_name;
	view_class->get_widget = task_view_get_widget;
	view_class->activate = task_view_activate;
	view_class->deactivate = task_view_deactivate;
	view_class->print_init = task_view_print_init;
	view_class->print_get_n_pages = task_view_print_get_n_pages;
	view_class->print = task_view_print;
	view_class->print_cleanup = task_view_print_cleanup;
}

static void
planner_task_view_init (PlannerTaskView *view)
{
	view->priv = g_new0 (PlannerTaskViewPriv, 1);
}

static void
task_view_finalize (GObject *object)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (object);

	if (PLANNER_VIEW (view)->activated) {
		task_view_deactivate (PLANNER_VIEW (view));
	}

	g_free (view->priv);

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		(*G_OBJECT_CLASS (parent_class)->finalize) (object);
	}
}

static void
task_view_activate (PlannerView *view)
{
	PlannerTaskViewPriv *priv;
	gboolean             show_critical;
	gboolean             show_nostd_days;
	gchar               *filename;

	priv = PLANNER_TASK_VIEW (view)->priv;

	priv->actions = gtk_action_group_new ("TaskView");
	gtk_action_group_set_translation_domain (priv->actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (priv->actions, entries,
				      G_N_ELEMENTS (entries),
				      view);
	gtk_action_group_add_toggle_actions (priv->actions, toggle_entries,
					     G_N_ELEMENTS (toggle_entries),
					     view);

	gtk_ui_manager_insert_action_group (priv->ui_manager, priv->actions, 0);
	filename = mrp_paths_get_ui_dir ("task-view.ui");
	priv->merged_id = gtk_ui_manager_add_ui_from_file (priv->ui_manager,
							   filename,
							   NULL);
	g_free (filename);
	gtk_ui_manager_ensure_update (priv->ui_manager);

	/* Set the initial UI state. */
	show_critical =   planner_conf_get_bool (CRITICAL_PATH_KEY, NULL);
	show_nostd_days = planner_conf_get_bool (NOSTDDAYS_PATH_KEY, NULL);
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

	task_view_selection_changed_cb (PLANNER_TASK_TREE (priv->tree), view);

	gtk_widget_grab_focus (priv->tree);
}

static void
task_view_deactivate (PlannerView *view)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;
	gtk_ui_manager_remove_ui (priv->ui_manager, priv->merged_id);
	gtk_ui_manager_remove_action_group (priv->ui_manager, priv->actions);
	g_object_unref (priv->actions);
	priv->actions = NULL;
}

static void
task_view_setup (PlannerView *view, PlannerWindow *main_window)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;

	priv->ui_manager = planner_window_get_ui_manager (main_window);
}

static const gchar *
task_view_get_label (PlannerView *view)
{
	return _("审计任务");
}

static const gchar *
task_view_get_menu_label (PlannerView *view)
{
	return _("_Tasks");
}

static const gchar *
task_view_get_icon (PlannerView *view)
{
	static gchar *filename = NULL;

	if (!filename) {
		filename = mrp_paths_get_image_dir ("tasks.png");
	}

	return filename;
}

static const gchar *
task_view_get_name (PlannerView *view)
{
	return "task_view";
}

static GtkWidget *
task_view_get_widget (PlannerView *view)
{
	PlannerTaskViewPriv *priv;
	MrpProject          *project;
	GtkWidget           *sw;
	PlannerGanttModel   *model;

	priv = PLANNER_TASK_VIEW (view)->priv;

	if (priv->tree == NULL) {
		project = planner_window_get_project (view->main_window);

		g_signal_connect (project,
				  "loaded",
				  G_CALLBACK (task_view_project_loaded_cb),
				  view);

		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);

		priv->frame = gtk_frame_new (NULL);
		gtk_frame_set_shadow_type (GTK_FRAME (priv->frame), GTK_SHADOW_IN);

		gtk_container_add (GTK_CONTAINER (priv->frame), sw);

		model = planner_gantt_model_new (project);

		priv->tree = planner_task_tree_new (view->main_window,
						    model,
						    TRUE,
						    FALSE,
						    /* i18n: WBS is sort for work breakdown structure, and is a
						     * project management term. You might want to leave it
						     * untranslated unless there is a localized term for it.
						     */
						    COL_WBS, _("序号"),
						    COL_NAME, _("Name"),
						    COL_START, _("Start"),
						    COL_FINISH, _("Finish"),
						    COL_WORK, _("Work"),
						    COL_DURATION, _("Duration"),
						    COL_SLACK, _("Slack"),
						    COL_COST, _("Cost"),
						    COL_ASSIGNED_TO, _("Assigned to"),
						    /* i18n: The string "% Complete" will be used in the header
						     * of a column containing values from 0 upto 100, indicating
						     * what part of a task has been completed.
						     * xgettext:no-c-format
						     */
						    COL_COMPLETE, _("任务完成度"),
						    COL_PROBABILITY,_("发生可能"),
						    -1);

		g_object_unref (model);

		task_view_load_columns (view);

		gtk_container_add (GTK_CONTAINER (sw), priv->tree);

		g_signal_connect (priv->tree,
				  "columns-changed",
				  G_CALLBACK (task_view_tree_view_columns_changed_cb),
				  view);

		g_signal_connect (priv->tree,
				  "destroy",
				  G_CALLBACK (task_view_tree_view_destroy_cb),
				  view);

		g_signal_connect (priv->tree,
				  "selection-changed",
				  G_CALLBACK (task_view_selection_changed_cb),
				  view);

		g_signal_connect (priv->tree,
				  "relation-added",
				  G_CALLBACK (task_view_relations_changed_cb),
				  view);

		g_signal_connect (priv->tree,
				  "relation-removed",
				  G_CALLBACK (task_view_relations_changed_cb),
				  view);

		gtk_widget_show (priv->tree);
		gtk_widget_show (sw);
		gtk_widget_show (priv->frame);
	}

	return priv->frame;
}

static void
task_view_tree_view_columns_changed_cb (GtkTreeView *tree_view,
					PlannerView *view)
{
	task_view_save_columns (view);
}

static void
task_view_tree_view_destroy_cb (GtkTreeView *tree_view,
				PlannerView *view)
{
	/* Block, we don't want to save the column configuration when they are
	 * removed by the destruction.
	 */
	g_signal_handlers_block_by_func (tree_view,
					 task_view_tree_view_columns_changed_cb,
					 view);
}

static void
task_view_project_loaded_cb (MrpProject  *project,
			     PlannerView *view)
{
	PlannerTaskViewPriv *priv;
	GtkTreeModel        *model;

	priv = PLANNER_TASK_VIEW (view)->priv;

	model = GTK_TREE_MODEL (planner_gantt_model_new (project));

	planner_task_tree_set_model (PLANNER_TASK_TREE (priv->tree),
				     PLANNER_GANTT_MODEL (model));

	g_object_unref (model);
}

/* Command callbacks. */

static void
task_view_insert_task_cb (GtkAction *action,
			  gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_insert_task (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_insert_tasks_cb (GtkAction *action,
			   gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_insert_tasks (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_remove_task_cb (GtkAction *action,
			  gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_remove_task (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_edit_task_cb (GtkAction *action,
			gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_edit_task (PLANNER_TASK_TREE (view->priv->tree),
				     PLANNER_TASK_DIALOG_PAGE_GENERAL);
}

static void
task_view_select_all_cb (GtkAction *action,
			 gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_select_all (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_unlink_task_cb (GtkAction *action,
			  gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_unlink_task (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_link_tasks_cb (GtkAction *action,
			 gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_link_tasks (PLANNER_TASK_TREE (view->priv->tree),
				      MRP_RELATION_FS);
}

static void
task_view_indent_task_cb (GtkAction *action,
			  gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_indent_task (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_move_task_up_cb (GtkAction *action,
			   gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_move_task_up (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_move_task_down_cb (GtkAction *action,
			     gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_move_task_down (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_unindent_task_cb (GtkAction *action,
			    gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_unindent_task (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_reset_constraint_cb (GtkAction *action,
			       gpointer   data)
{
	PlannerTaskView *view;

	view = PLANNER_TASK_VIEW (data);

	planner_task_tree_reset_constraint (PLANNER_TASK_TREE (view->priv->tree));
}

static void
task_view_edit_custom_props_cb (GtkAction *action,
				gpointer   data)
{
	PlannerTaskView *view;
	GtkWidget       *dialog;
	MrpProject      *project;

	view = PLANNER_TASK_VIEW (data);

	project = planner_window_get_project (PLANNER_VIEW (view)->main_window);

	dialog = planner_property_dialog_new (PLANNER_VIEW (view)->main_window,
					      project,
					      MRP_TYPE_TASK,
					      _("Edit custom task properties"));

	gtk_window_set_default_size (GTK_WINDOW (dialog), 500, 300);
	gtk_widget_show (dialog);
}

static void
task_view_highlight_critical_cb (GtkAction *action,
				 gpointer   data)
{
	PlannerTaskViewPriv *priv;
	gboolean             state;

	priv = PLANNER_TASK_VIEW (data)->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	planner_task_tree_set_highlight_critical (
		PLANNER_TASK_TREE (priv->tree),
		state);

	planner_conf_set_bool (CRITICAL_PATH_KEY, state, NULL);
}

static void
task_view_nonstandard_days_cb (GtkAction *action,
				 gpointer   data)
{
	PlannerTaskViewPriv *priv;
	gboolean             state;

	priv = PLANNER_TASK_VIEW (data)->priv;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

	planner_task_tree_set_nonstandard_days (
		PLANNER_TASK_TREE (priv->tree),
		state);


	planner_conf_set_bool (NOSTDDAYS_PATH_KEY, state, NULL);
}

static void
task_view_edit_columns_cb (GtkAction *action,
			   gpointer   data)
{
	PlannerTaskView     *view;
	PlannerTaskViewPriv *priv;

	view = PLANNER_TASK_VIEW (data);
	priv = view->priv;

	planner_column_dialog_show (PLANNER_VIEW (view)->main_window,
				    _("Edit Task Columns"),
				    GTK_TREE_VIEW (priv->tree));
}


/*
 * zmsadded for auto assignment resource to each task belonging to current project
 *

static void
task_view_auto_assignment_cb(GtkAction *action,
	            gpointer data)
{
	    PlannerTaskView     *view;
		PlannerTaskViewPriv *priv;
		MrpProject *project;
		MrpProjectPriv *ppriv;
		GList *task_list;
		GList *resource_list;
		GList *l;
		GList *ll;
		MrpResource *resource;
		MrpTaskManager *task_manager;
		gboolean  exclusive1;
		view = PLANNER_TASK_VIEW (data);
		priv = view->priv;
		project = planner_window_get_project(PLANNER_VIEW(view)->main_window);
	//	ppriv = project->priv;
		task_manager =imrp_project_get_task_manager(project);
		task_list = mrp_task_manager_get_all_tasks(task_manager);
		resource_list = mrp_project_get_resources(project);
		ll = resource_list;
		if(ll)
		{
			for(l=task_list;l;l=l->next)
			{
				resource = 	ll->data;
				g_object_get (G_OBJECT(resource), "exclusive", &exclusive1, NULL);

				if(ll->next)
						{
							ll = ll->next;
							if(exclusive1)
							{
								if(ll->next)
									ll = ll->next;
							}

						}else{
							printf("\t\t\t noResource!!\n");
							break;
						}
				mrp_resource_assign (resource,l->data,100);

			}
			//g_list_free(l);
		}
		//g_list_free(ll);
		//g_list_free(resource_list);
		//g_list_free(task_list);
		//g_free(project);
}
*/

static void
task_view_auto_assignment_cb(GtkAction *action,
	            gpointer data)
{
	PlannerTaskView     *view;
	PlannerTaskViewPriv *priv;
	MrpProject *project;
	MrpProjectPriv *ppriv;
	GList *l1 = NULL;
	GList *l2 = NULL;
	MrpResource *resource;
	MrpResource *resource2;
	MrpTaskManager *task_manager;
	gboolean  exclusive1;
	view = PLANNER_TASK_VIEW (data);
	priv = view->priv;
	project = planner_window_get_project(PLANNER_VIEW(view)->main_window);
	GList *resources = mrp_project_get_resources(project);
	GList *hadresources = NULL;
	GList *hadtasks = NULL;
	GList *hadtasks1 = NULL;
	GList *tasklist = mrp_project_get_all_tasks(project);
	GList *sortedtasks = sortTasklistsByStartTime(tasklist);
	gint sig = 0;
	g_printf("have sorted the tasks\n");
	mrptime s1 = 0,f1 = 0,s2 = 0,f2 = 0 ;
	l1=sortedtasks;
	MrpTask *task1 = l1->data;
	if(resources != NULL)
	{
		resource = resources->data;
		resources = resources->next;
		//resources = g_list_remove(resources,resource);
	}
//TODO:need else for no resource
	mrp_resource_assign (resource,task1,100);

	gchar *s11 = mrp_task_get_name(task1);
				gchar *s21 = mrp_resource_get_name(resource);
				g_printf("%s,%s\n",s11,s21);

	g_printf("have assigned the first task\n");
	hadtasks = g_list_append(hadtasks,task1);
	//hadresources = g_list_append(hadresources,resource);
	l1=l1->next;
	MrpTask *taskdelet = NULL;
	while(l1)
	{
		g_printf("in the loop\n");
		MrpTask *task2 = l1->data;
		s2 = mrp_task_get_start(task2);
		for(l2=hadtasks;l2;l2=l2->next)
		{
			f1 = mrp_task_get_finish(l2->data);
			g_printf("the task time %d,%d\n",f1,s2);
			if(f1 <= s2)
			{
				//hadtasks1 = g_list_copy(hadtasks);
				taskdelet = l2->data;

				gchar *s11 = mrp_task_get_name(task1);
								g_printf("%s\n",s11);

				hadtasks = g_list_remove(hadtasks,l2->data);
				sig = 1;
				break;
			}

		}
		//hadtasks = hadtasks1;
		if(sig)
		{
			//resource2 = hadresources->data;
			GList *l = mrp_task_get_assigned_resources(taskdelet);

			/*GList *l8 = l;
			for(;l8;l8=l8->data)
			{
				gchar *s5 = mrp_resource_get_note(l8->data);
				gchar *s6 = mrp_task_get_note(task2);
				g_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!11%s,%s\n",s5,s6);
				if(g_strcmp0(s5,s6))
				{
					break;
				}
			}*/


				resource2 = l->data;

							mrp_resource_assign (resource2,task2,100);
							hadtasks = g_list_append(hadtasks,task2);

							gchar *s11 = mrp_task_get_name(task2);
							gchar *s21 = mrp_resource_get_name(resource2);
							g_printf("%s,%s\n",s11,s21);
		}				//hadresources = g_list_remove(hadresources,resource2);

			/*else
			{
				GList *l9 = resources;
									for(;l9;l9=l9->data)
									{
										gchar *s5 = mrp_resource_get_note(l9->data);
										gchar *s6 = mrp_task_get_note(task2);
										g_printf("!!!!!!!!!!!!!!!!!!!!!!!!!11%s,%s\n",s5,s6);
										if(g_strcmp0(s5,s6))
										{
											resources = g_list_remove(resources,l9->data);
											//l9 = resources;
											break;
										}
									}
									resource2 = l9->data;
							mrp_resource_assign (resource2,task2,100);
							hadtasks = g_list_append(hadtasks,task2);

							gchar *s1 = mrp_task_get_name(task2);
										gchar *s2 = mrp_resource_get_name(resource2);
										g_printf("%s,%s\n",s11,s21);
							//hadresources = g_list_append(hadresources,resource2);
			}
		}*/
		else
//TODO:need else for no resource
		{
			resource2 = resources->data;
			resources = resources->next;
		/*	GList *l9 = resources;
					for(;l9;l9=l9->data)
					{
						gchar *s5 = mrp_resource_get_note(l9->data);
						gchar *s6 = mrp_task_get_note(task2);
						g_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s,%s\n",s5,s6);
						if(g_strcmp0(s5,s6))
						{
							resources = g_list_remove(resources,l9->data);
							//l9 = resources;
							break;
						}
					}
					resource2 = l9->data;*/
			mrp_resource_assign (resource2,task2,100);
			hadtasks = g_list_append(hadtasks,task2);

			gchar *s1 = mrp_task_get_name(task2);
						gchar *s2 = mrp_resource_get_name(resource2);
						g_printf("%s,%s\n",s11,s21);
			//hadresources = g_list_append(hadresources,resource2);
		}
		task1 = task2;
		l1=l1->next;
		sig = 0;
	}

	firsttasklist = sortTasklistsByFinishTime(tasklist);
	//durationOptimize(project);


}

static void
task_view_pert_chart_cb		(GtkAction       *action,
	     gpointer         data)
{
	gchar        *filename;
	GladeXML     *glade;
	GtkWidget    *windows;
	GtkWidget    *scrollwindows;
	GtkWidget    *layout;
	GtkAdjustment *horizontal, *vertical;
	GtkWidget	 *label;
	GtkWidget    *frame;
	MrpTask    *root;
	GList *task_list;
	GList *l;
	MrpTaskManager *task_manager;
	PlannerTaskView     *view;
	PlannerTaskViewPriv *priv;
	MrpProject *project;
	gint x,y;
	MrpTask *taskinpert;


	//g_printf("haha\n");

	view = PLANNER_TASK_VIEW (data);
	priv = view->priv;
	project = planner_window_get_project(PLANNER_VIEW(view)->main_window);
	task_manager = imrp_project_get_task_manager(project);
	root = mrp_task_manager_get_root (task_manager);

	windows = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (windows,"Pert Chart");
	gtk_window_set_default_size(windows,1000,800);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(windows),frame);
	gtk_widget_show(frame);
	//gtk_adjustment_new();
	//gtk_adjustment_new();
	scrollwindows = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrollwindows),GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
	gtk_container_set_border_width(GTK_CONTAINER(scrollwindows),20);
	gtk_container_add (GTK_CONTAINER (frame), scrollwindows);
	gtk_widget_show(scrollwindows);

	layout = gtk_layout_new(NULL,NULL);

	// GtkWidget *fixed;
	 GtkWidget *button;
	 GtkWidget *vbox;
	 GtkWidget *table;
	 GtkWidget *togglebutton;

	 GtkWidget *labelname;
	 GtkWidget *labelstart;
	 GtkWidget *labelfinished;
	 GtkWidget *labelduration;
	 GtkWidget *labelprobability;

	// button1=gtk_button_new_with_label("改变主窗口label文字333333");

	// button=gtk_button_new_with_label("改变主窗口label文字");
	// gtk_fixed_put(GTK_FIXED(fixed), button, 497, 250);
	 labelname=gtk_label_new("任务名称");
	 labelstart=gtk_label_new("开始时间");
	 labelfinished=gtk_label_new("结束时间");
	 labelduration=gtk_label_new("工期");
	 labelprobability=gtk_label_new("事件概率");
	 togglebutton = gtk_toggle_button_new_with_label("选择路径");
	 gtk_layout_put(layout, button, 497, 250);
	 gtk_widget_set_size_request(button, 80, 65);

	 gtk_container_add(GTK_CONTAINER(scrollwindows), layout);

	 table = gtk_table_new(4, 2, TRUE);
		   gtk_table_set_row_spacings(GTK_TABLE(table), 2);
		   gtk_table_set_col_spacings(GTK_TABLE(table), 2);
		   gtk_table_attach_defaults(GTK_TABLE(table), labelname,0,2,0,1);
		   gtk_table_attach_defaults(GTK_TABLE(table), labelstart,0,1,1,2);
		   gtk_table_attach_defaults(GTK_TABLE(table), labelfinished,1,2,1,2);
		   gtk_table_attach_defaults(GTK_TABLE(table), labelduration,0,1,2,3);
		   gtk_table_attach_defaults(GTK_TABLE(table), labelprobability,1,2,2,3);
		   gtk_table_attach_defaults(GTK_TABLE(table), togglebutton,0,2,3,4);

	task_list = mrp_task_manager_get_all_tasks(task_manager);
	g_printf("lll111\n");
	gint aa = g_list_length(task_list);
				g_printf("%d\n",aa);
	l = planner_pertchart_nodes_creat(task_list);

	g_printf("lll333\n");
	task_manager_build_dependency_graph_for_node (task_manager);
	g_printf("lll222\n");

	l = setPertNodesPosition();
	avoidCrossingNode();
	avoidCrossingLine();
	removeEmptyColumn();
	getposition(l);
	getArrowPosition(pertnodes);
	//g_printf("lll222\n");

	drawtask(layout,project);


	GList *l3 = getCriticalTasks(task_list);
	for(;l3;l3=l3->next)
		{
		gchar *s5 = mrp_task_get_name(l3->data);
		g_printf("the critical task is %s\n",s5);
		}


	g_printf("lll222\n");
	MrpProject *newproject = renewProject(project);
	l3 = mrp_project_get_resources(newproject);
	g_printf("%d\n",g_list_length(l3));
	for(;l3;l3=l3->next)
			{
			gchar *s3 = mrp_resource_get_name(l3->data);
			g_printf("the new project resources is %s\n",s3);
			}

	MrpTask *t3 = mrp_project_get_task_by_name(project,"G");

  // gchar *a = "asd";
	//g_object_set (t3, "name",a,"probability", 42, NULL);
   //mrp_task_set_probability(t3,45);
   gint prob = mrp_task_get_probability(t3);
	//gchar *na = mrp_task_get_name(t3);
	g_printf("h's probability is %d,\n",prob);

	gint duration = totalduration(project);
	g_printf("project's total duration is %d day\n",duration);
	//imrp_task_set_duration(t3,duration*60*60*24);
	//g_object_set (t3, "sched",1,NULL);
	//g_object_set (t3, "duration",duration*60*60*24,NULL);
	setDelayedDuration(t3);
	mrptime duration1 = mrp_task_get_duration(t3);
	g_printf("t3's new duration is %d day\n",duration1);
	//l3 = task_list;
	gint i;
	//srand((int) time(0));
	/*for(i=0;i<10;i++)
	{
		t3 = getRandomTask(task_list);
		gchar *na = mrp_task_get_name(t3);
	    g_printf("the %d's random task is %s\n",i,na);
	}

	*///deleteRandomResourceFromTask(task_list);
	PlannerPertchartNode *pertnode3 = getNodeByRowCol(pertnodes,2,1);

	gchar *s3 = mrp_task_get_name(planner_pertchart_node_get_task(pertnode3));
	g_printf("the 2th row 1th col node is %s\n",s3);
	l3 = getPertchartNodebrothers(pertnode3);
	for(;l3;l3=l3->next)
				{
				gchar *s3 = mrp_task_get_name(l3->data);
				g_printf("the G's brother is %s\n",s3);
				}

	l3 = getAllTasks(project);
	for(;l3;l3=l3->next)
					{
					gchar *s3 = mrp_task_get_name(l3->data);
					g_printf("the project's task is %s\n",s3);
					}



	//g_list_free(pertnodes);
	gtk_widget_show_all(windows);


}



static void
task_view_arrange_resource_cb (GtkAction *action,
			   gpointer   data)
{
	PlannerTaskView     *view;
	PlannerTaskViewPriv *priv;
	MrpProject *project;
	GList *group_list;
	GList *resource_list;
	GList *l;
	GList *ll;
	GList *group_resource_list;
	typedef struct{
			MrpGroup * group;
			GList *resource;
		}Group_resource;
	group_resource_list = NULL;
	view = PLANNER_TASK_VIEW (data);
	priv = view->priv;
//todo:aa
	project = planner_window_get_project(PLANNER_VIEW(view)->main_window);
	group_list = mrp_project_get_groups(project);
	resource_list = mrp_project_get_resources(project);

	for (l = group_list; l; l = l->next) {
		Group_resource* temp;
		temp = g_malloc0(sizeof(Group_resource));
		temp->group = MRP_GROUP (l->data);
		temp->resource = NULL;
		group_resource_list = g_list_append(group_resource_list,temp);
	}
	for (l = resource_list; l; l = l->next) {
		MrpResource * resource;
		MrpGroup *resource_group;
		resource = MRP_RESOURCE(l->data);
		mrp_object_get(MRP_OBJECT(resource),"group",&resource_group,NULL);
		if(resource_group == NULL) continue;
		for(ll = group_resource_list; ll; ll = ll ->next) {
			if(resource_group == ((Group_resource*)ll->data)->group) {
				((Group_resource*)ll->data)->resource =
						g_list_append(((Group_resource*)ll->data)->resource,resource);
			}
		}
	}
	printf("Arrange::\n");
	for (l = group_resource_list; l; l = l->next) {
		MrpGroup *group;
		GList *group_resources;
		group = ((Group_resource*)l->data)->group;
		printf("\t group:%s\n",mrp_group_get_name(group));
		group_resources = ((Group_resource*)l->data)->resource;
		for (ll = mrp_group_get_assignments(group); ll;
				ll = mrp_group_get_assignments(group)) {
			MrpGroupAssignment *groupAssign;
			groupAssign = MRP_GROUP_ASSIGNMENT(ll->data);
			printf("\t\t task:%s\n",
					mrp_task_get_name(mrp_group_assignment_get_task(groupAssign)));
			if(group_resources) {
				mrp_resource_assign (MRP_RESOURCE(group_resources->data)
						, mrp_group_assignment_get_task(groupAssign), 100);
				mrp_object_removed (MRP_OBJECT (groupAssign));
				group_resources = group_resources->next;
			} else {
				printf("\t\t\t noResource!!\n");
				break;
			}
		}
	}
	for (l = group_resource_list; l; l = l->next) {
		GList *tempRes;
		tempRes = ((Group_resource*)l->data)->resource;
		for(ll = tempRes;ll;ll = ll->next) {
			gchar* resName = mrp_resource_get_name(MRP_RESOURCE(ll->data));
			printf("resName:%s\n",resName);
		}
		printf("\n");
		g_list_free(tempRes);
		g_free(l->data);
	}
	g_list_free(group_resource_list);
}

static void
task_view_selection_changed_cb (PlannerTaskTree *tree, PlannerView *view)
{
	task_view_update_ui (view);
}

static void
task_view_relations_changed_cb (PlannerTaskTree  *tree,
				MrpTask     *task,
				MrpRelation *relation,
				PlannerView      *view)
{
	task_view_update_ui (view);
}


static void
task_view_print_init (PlannerView     *view,
		      PlannerPrintJob *job)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;

	priv->print_sheet = planner_table_print_sheet_new (PLANNER_VIEW (view), job,
							   GTK_TREE_VIEW (priv->tree));
}

static void
task_view_print (PlannerView *view,
		 gint         page_nr)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;

	planner_table_print_sheet_output (priv->print_sheet, page_nr);
}

static gint
task_view_print_get_n_pages (PlannerView *view)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;

	return planner_table_print_sheet_get_n_pages (priv->print_sheet);
}

static void
task_view_print_cleanup (PlannerView *view)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;

	planner_table_print_sheet_free (priv->print_sheet);
	priv->print_sheet = NULL;
}

static void
task_view_update_ui (PlannerView *view)
{
	PlannerTaskViewPriv *priv;
	GList           *list, *l;
	gboolean         value;
	gboolean         rel_value  = FALSE;
	gboolean         link_value = FALSE;
	gint	         count = 0;

	if (!view->activated) {
		return;
	}

	priv = PLANNER_TASK_VIEW (view)->priv;

	list = planner_task_tree_get_selected_tasks (PLANNER_TASK_TREE (priv->tree));

	for (l = list; l; l = l->next) {
		if (mrp_task_has_relation (MRP_TASK (l->data))) {
			rel_value = TRUE;
			break;
		}
	}

	for (l = list; l; l = l->next) {
		count++;
	}

	value = (list != NULL);
	link_value = (count >= 2);

	g_object_set (gtk_action_group_get_action (priv->actions, "EditTask"),
		      "sensitive", value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "RemoveTask"),
		      "sensitive", value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "UnlinkTask"),
		      "sensitive", rel_value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "LinkTasks"),
		      "sensitive", link_value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "IndentTask"),
		      "sensitive", value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "UnindentTask"),
		      "sensitive", value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "MoveTaskUp"),
		      "sensitive", value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "MoveTaskDown"),
		      "sensitive", value,
		      NULL);
	g_object_set (gtk_action_group_get_action (priv->actions, "ResetConstraint"),
		      "sensitive", value,
		      NULL);

	g_list_free (list);
}

static void
task_view_save_columns (PlannerView *view)
{
	PlannerTaskViewPriv *priv;

	priv = PLANNER_TASK_VIEW (view)->priv;

	planner_view_column_save_helper (view, GTK_TREE_VIEW (priv->tree));
}

static void
task_view_load_columns (PlannerView *view)
{
	PlannerTaskViewPriv *priv;
	GList               *columns, *l;
	GtkTreeViewColumn   *column;
	const gchar         *id;
	gint                 i;

	priv = PLANNER_TASK_VIEW (view)->priv;

	/* Load the columns. */
	planner_view_column_load_helper (view, GTK_TREE_VIEW (priv->tree));

	/* Make things a bit more robust by setting defaults if we don't get any
	 * visible columns. Should be done through a schema instead (but we'll
	 * keep this since a lot of people get bad installations when installing
	 * themselves).
	 */
	columns = gtk_tree_view_get_columns (GTK_TREE_VIEW (priv->tree));
	i = 0;
	for (l = columns; l; l = l->next) {
		if (gtk_tree_view_column_get_visible (l->data)) {
			i++;
		}
	}

	if (i == 0) {
		for (l = columns; l; l = l->next) {
			column = l->data;

			if (g_object_get_data (G_OBJECT (column), "custom")) {
				continue;
			}

			id = g_object_get_data (G_OBJECT (column), "id");

			g_print ("%s\n", id);

			if (!id) {
				continue;
			}


			if (strcmp (id, "wbs") == 0 ||
			    strcmp (id, "name") == 0 ||
			    strcmp (id, "start") == 0 ||
			    strcmp (id, "finish") == 0 ||
			    strcmp (id, "work") == 0 ||
			    strcmp (id, "duration") == 0 ||
			    strcmp (id, "duration") == 0 ||
			    strcmp (id, "slack") == 0 ||
			    strcmp (id, "cost") == 0 ||
			    strcmp (id, "complete") == 0 ||
			    strcmp (id, "probability") == 0 ||
			    strcmp (id, "assigned-to")) {
				gtk_tree_view_column_set_visible (column, TRUE);
			} else {
				gtk_tree_view_column_set_visible (column, FALSE);
			}
		}
	}

	g_list_free (columns);
}

PlannerView *
planner_task_view_new (void)
{
	PlannerView *view;

	view = g_object_new (PLANNER_TYPE_TASK_VIEW, NULL);

	return view;
}

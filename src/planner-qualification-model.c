/*
 * planner-qualification-model.c
 *
 *  Created on: 2014-11-19
 *      Author: zms
 */
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2002 Alvaro del Castillo <acs@barrapunto.com>
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
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "planner-qualification-model.h"

struct _PlannerQualificationModelPriv {
	MrpProject *project;
};

#define G_LIST(x) ((GList *) x)

static void     mqm_init                     (PlannerQualificationModel        *model);
static void     mqm_class_init               (PlannerQualificationModelClass   *klass);
static void     mqm_finalize                 (GObject             *object);

static gint     mqm_get_n_columns            (GtkTreeModel        *tree_model);
static GType    mqm_get_column_type          (GtkTreeModel        *tree_model,
					      gint                 index);

static void     mqm_get_value                (GtkTreeModel        *tree_model,
					      GtkTreeIter         *iter,
					      gint                 column,
					      GValue              *value);
static void     mqm_qualification_notify_cb          (MrpQualification            *qualification,
					      GParamSpec          *pspec,
					      PlannerQualificationModel        *model);
static void     mqm_qualification_added_cb           (MrpProject          *project,
					      MrpQualification            *resource,
					      PlannerQualificationModel        *model);

static void     mqm_qualification_removed_cb         (MrpProject          *project,
					      MrpQualification            *resource,
					      PlannerQualificationModel        *model);

static void     mqm_default_qualification_changed_cb (MrpProject          *project,
					      MrpQualification            *qualification,
					      PlannerQualificationModel        *model);

static PlannerListModelClass *parent_class = NULL;


GType
planner_qualification_model_get_type (void)
{
        static GType rm_type = 0;

        if (!rm_type) {
                static const GTypeInfo rm_info =
                        {
                                sizeof (PlannerQualificationModelClass),
                                NULL,		/* base_init */
                                NULL,		/* base_finalize */
                                (GClassInitFunc) mqm_class_init,
                                NULL,		/* class_finalize */
                                NULL,		/* class_data */
                                sizeof (PlannerQualificationModel),
                                0,
                                (GInstanceInitFunc) mqm_init,
                        };

                rm_type = g_type_register_static (PLANNER_TYPE_LIST_MODEL,
                                                  "PlannerQualificationModel",
                                                  &rm_info, 0);
        }

        return rm_type;
}

static void
mqm_class_init (PlannerQualificationModelClass *klass)
{
        GObjectClass     *object_class;
	PlannerListModelClass *lm_class;

        parent_class = g_type_class_peek_parent (klass);
        object_class = G_OBJECT_CLASS (klass);
	lm_class     = PLANNER_LIST_MODEL_CLASS (klass);

        object_class->finalize = mqm_finalize;

	lm_class->get_n_columns   = mqm_get_n_columns;
	lm_class->get_column_type = mqm_get_column_type;
	lm_class->get_value       = mqm_get_value;
}

static void
mqm_init (PlannerQualificationModel *model)
{
        PlannerQualificationModelPriv *priv;

        priv = g_new0 (PlannerQualificationModelPriv, 1);

	priv->project = NULL;

        model->priv = priv;
}


static void
mqm_finalize (GObject *object)
{
	PlannerQualificationModel *model = PLANNER_QUALIFICATION_MODEL (object);
	GList             *l, *qualifications;

	g_return_if_fail (model->priv != NULL);
	g_return_if_fail (MRP_IS_PROJECT (model->priv->project));

	qualifications = planner_list_model_get_data (PLANNER_LIST_MODEL (model));
	for (l = qualifications; l; l = l->next) {
		g_signal_handlers_disconnect_by_func (MRP_QUALIFICATION (l->data),
						      mqm_qualification_notify_cb,
						      model);
	}
	g_signal_handlers_disconnect_by_func (model->priv->project,
					      mqm_qualification_added_cb,
					      model);
	g_signal_handlers_disconnect_by_func (model->priv->project,
					      mqm_qualification_removed_cb,
					      model);
	g_signal_handlers_disconnect_by_func (model->priv->project,
					      mqm_default_qualification_changed_cb,
					      model);

	g_object_unref (model->priv->project);
	g_free (model->priv);
	model->priv = NULL;

        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static gint
mqm_get_n_columns (GtkTreeModel *tree_model)
{
        return NUMBER_OF_QUALIFICATION_COLS;
}

static GType
mqm_get_column_type (GtkTreeModel *tree_model,
		      gint          column)
{
        switch (column) {
        case QUALIFICATION_COL_NAME:
        case QUALIFICATION_COL_NOTE:
                return G_TYPE_STRING;
        case QUALIFICATION_COL:
        		  return MRP_TYPE_QUALIFICATION;
	default:
		return G_TYPE_INVALID;
        }
}

static void
mqm_get_value (GtkTreeModel *tree_model,
		GtkTreeIter  *iter,
		gint          column,
		GValue       *value)
{
	gchar *str = NULL;
	MrpQualification *qualification, *default_qualification;
	PlannerQualificationModelPriv *priv;
	gboolean is_default;

	g_return_if_fail(PLANNER_IS_QUALIFICATION_MODEL (tree_model));
	g_return_if_fail(iter != NULL);

	priv = PLANNER_QUALIFICATION_MODEL (tree_model) ->priv;
	qualification = MRP_QUALIFICATION (planner_list_model_get_object (
					PLANNER_LIST_MODEL (tree_model), iter));

	switch (column) {
	case QUALIFICATION_COL_NAME:
		mrp_object_get(qualification, "name", &str, NULL );
		g_value_init(value, G_TYPE_STRING );
		g_value_set_string(value, str);
		g_free(str);
		break;
	case QUALIFICATION_COL_NOTE:
		mrp_object_get(qualification, "note", &str, NULL );
		g_value_init(value, G_TYPE_STRING );
		g_value_set_string(value, str);
		g_free(str);

		break;
	case QUALIFICATION_COL:
		g_value_init(value, MRP_TYPE_QUALIFICATION);
		g_value_set_object(value, qualification);
		break;

	default:
		g_assert_not_reached ()
		;
        }
}

static void
mqm_qualification_notify_cb (MrpQualification *qualification, GParamSpec *pspec, PlannerQualificationModel *model)
{
	GtkTreeModel *tree_model;
	GtkTreePath  *path;
	GtkTreeIter   iter;

	g_return_if_fail (PLANNER_IS_QUALIFICATION_MODEL (model));
	g_return_if_fail (MRP_IS_QUALIFICATION (qualification));

	tree_model = GTK_TREE_MODEL (model);

	path = planner_list_model_get_path (PLANNER_LIST_MODEL (model),
				       MRP_OBJECT (qualification));

	if (path) {
		gtk_tree_model_get_iter (tree_model, &iter, path);
		gtk_tree_model_row_changed (GTK_TREE_MODEL (model),
					    path, &iter);

		gtk_tree_path_free (path);
	}
}

static void
mqm_qualification_added_cb (MrpProject   *project,
		    MrpQualification     *qualification,
		    PlannerQualificationModel *model)
{
	g_return_if_fail (PLANNER_IS_QUALIFICATION_MODEL (model));
	g_return_if_fail (MRP_IS_QUALIFICATION (qualification));

	planner_list_model_append (PLANNER_LIST_MODEL (model), MRP_OBJECT (qualification));

	g_signal_connect (qualification, "notify",
			  G_CALLBACK (mqm_qualification_notify_cb),
			  model);
}

static void
mqm_qualification_removed_cb (MrpProject   *project,
		      MrpQualification     *qualification,
		      PlannerQualificationModel *model)
{
	g_return_if_fail (PLANNER_IS_QUALIFICATION_MODEL (model));
	g_return_if_fail (MRP_IS_QUALIFICATION (qualification));

	g_signal_handlers_disconnect_by_func (qualification,
					      mqm_qualification_notify_cb,
					      model);

	planner_list_model_remove (PLANNER_LIST_MODEL (model), MRP_OBJECT (qualification));
}

static void
mqm_default_qualification_changed_cb (MrpProject   *project,
			      MrpQualification     *qualification,
			      PlannerQualificationModel *model)
{
	GtkTreePath *path;
	GtkTreeIter  iter;
	gint         i;
	GList       *qualifications;

	g_return_if_fail (PLANNER_IS_QUALIFICATION_MODEL (model));

	if (qualification == NULL)
		return;

	qualifications = planner_list_model_get_data (PLANNER_LIST_MODEL (model));

	i = g_list_index (qualifications, qualification);

	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, i);

	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);

	gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);

	gtk_tree_path_free (path);
}

PlannerQualificationModel *
planner_qualification_model_new (MrpProject *project)
{
        PlannerQualificationModel     *model;
        PlannerQualificationModelPriv *priv;
	GList                 *qualifications;

        model = g_object_new (PLANNER_TYPE_QUALIFICATION_MODEL, NULL);

        priv = model->priv;

        qualifications = mrp_project_get_qualifications (project);
	planner_list_model_set_data (PLANNER_LIST_MODEL (model), qualifications);

	priv->project = g_object_ref (project);

	g_signal_connect_object (project,
				 "qualification_added",
				 G_CALLBACK (mqm_qualification_added_cb),
				 model, 0);

	g_signal_connect_object (project,
				 "qualification_removed",
				 G_CALLBACK (mqm_qualification_removed_cb),
				 model, 0);
	mrp_object_set(qualifications, "name", "aaa", NULL );

        return model;
}



/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2001-2003 CodeFactory AB
 * Copyright (C) 2001-2003 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "string.h"
#include <glib/gi18n.h>
#include "mrp-group.h"
#include "mrp-group-assignment.h"
#include "mrp-private.h"

struct _MrpGroupPriv {
	gchar *name;
	gchar *manager_name;
	gchar *manager_phone;
	gchar *manager_email;
	GList           *assignments;
};

/* Properties */
enum {
        PROP_0,
        PROP_NAME,
        PROP_MANAGER_NAME,
        PROP_MANAGER_PHONE,
	PROP_MANAGER_EMAIL
};


static void group_class_init   (MrpGroupClass      *klass);
static void group_init         (MrpGroup           *group);
static void group_finalize     (GObject            *object);
static void group_set_property (GObject            *object,
				guint               prop_id,
				const GValue       *value,
				GParamSpec         *pspec);
static void group_get_property (GObject            *object,
				guint               prop_id,
				GValue             *value,
				GParamSpec         *pspec);

static void group_remove_assignment_foreach (MrpGroupAssignment *assignment,
		MrpGroup   *group);


static MrpObjectClass *parent_class;


GType
mrp_group_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (MrpGroupClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) group_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (MrpGroup),
			0,              /* n_preallocs */
			(GInstanceInitFunc) group_init,
		};

		object_type = g_type_register_static (MRP_TYPE_OBJECT,
                                                      "MrpGroup",
                                                      &object_info, 0);
	}

	return object_type;
}

static void
group_class_init (MrpGroupClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        parent_class = MRP_OBJECT_CLASS (g_type_class_peek_parent (klass));

        object_class->finalize     = group_finalize;
        object_class->set_property = group_set_property;
        object_class->get_property = group_get_property;

        g_object_class_install_property (object_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "Name",
                                                              "Name of the group",
                                                              "empty",
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_MANAGER_NAME,
                                         g_param_spec_string ("manager_name",
							      "Manager Name",
							      "The name of the group manager",
							      "empty",
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_MANAGER_PHONE,
                                         g_param_spec_string ("manager_phone",
							      "Manager Phone",
							      "The phone number of the group manager",
							      "empty",
							      G_PARAM_READWRITE));

        g_object_class_install_property (object_class,
                                         PROP_MANAGER_EMAIL,
                                         g_param_spec_string ("manager_email",
							      "Manager Email",
							      "The email address of the group manager",
							      "empty",
							      G_PARAM_READWRITE));

}


static void
group_init (MrpGroup *group)
{
        MrpGroupPriv *priv;

        priv = g_new0 (MrpGroupPriv, 1);

	priv->name          = g_strdup ("");
	priv->manager_name  = g_strdup ("");
	priv->manager_phone = g_strdup ("");
	priv->manager_email = g_strdup ("");
	priv->assignments = NULL;

        group->priv = priv;
}

static void
group_finalize (GObject *object)
{
        MrpGroup     *group = MRP_GROUP (object);
        MrpGroupPriv *priv;

        priv = group->priv;

        g_free (priv->name);
        priv->name = NULL;

        g_free (priv->manager_name);
        priv->manager_name = NULL;

        g_free (priv->manager_phone);
        priv->manager_phone = NULL;

        g_free (priv->manager_email);
        priv->manager_email = NULL;

        g_list_foreach (priv->assignments,
        			(GFunc) group_remove_assignment_foreach,
        			group);

        	g_list_free (priv->assignments);
        	priv->assignments = NULL;

        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static void
group_set_property (GObject        *object,
		    guint           prop_id,
		    const GValue   *value,
		    GParamSpec     *pspec)
{
	MrpGroup     *group;
	MrpGroupPriv *priv;
	gboolean      changed = FALSE;
	const gchar  *str;

	g_return_if_fail (MRP_IS_GROUP (object));

	group = MRP_GROUP (object);
	priv  = group->priv;

	switch (prop_id) {
	case PROP_NAME:
		str = g_value_get_string (value);

		if (!priv->name || strcmp (priv->name, str)) {
			g_free (priv->name);
			priv->name = g_strdup (str);
			changed = TRUE;
		}

		break;
	case PROP_MANAGER_NAME:
		str = g_value_get_string (value);

		if (!priv->manager_name || strcmp (priv->manager_name, str)) {
			g_free (priv->manager_name);
			priv->manager_name = g_strdup (str);
			changed = TRUE;
		}

		break;
	case PROP_MANAGER_PHONE:
		str = g_value_get_string (value);

		if (!priv->manager_phone || strcmp (priv->manager_phone, str)) {
			g_free (priv->manager_phone);
			priv->manager_phone = g_strdup (str);
			changed = TRUE;
		}

		break;
	case PROP_MANAGER_EMAIL:
		str = g_value_get_string (value);

		if (!priv->manager_email || strcmp (priv->manager_email, str)) {
			g_free (priv->manager_email);
			priv->manager_email = g_strdup (str);
			changed = TRUE;
		}
		break;
	default:
		break;
	}

	if (changed) {
		mrp_object_changed (MRP_OBJECT (object));
	}
}

static void
group_get_property (GObject      *object,
		    guint         prop_id,
		    GValue       *value,
		    GParamSpec   *pspec)
{
	MrpGroup     *group;
	MrpGroupPriv *priv;

	g_return_if_fail (MRP_IS_GROUP (object));

	group = MRP_GROUP (object);
	priv  = group->priv;

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_MANAGER_NAME:
		g_value_set_string (value, priv->manager_name);
		break;
	case PROP_MANAGER_PHONE:
		g_value_set_string (value, priv->manager_phone);
		break;
	case PROP_MANAGER_EMAIL:
		g_value_set_string (value, priv->manager_email);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * mrp_group_new:
 *
 * Creates a new end group.
 *
 * Return value: the newly created group.
 **/
MrpGroup *
mrp_group_new (void)
{
        MrpGroup *group;

        group = g_object_new (MRP_TYPE_GROUP,
			      "name", "",
			      NULL);

        return group;
}

/**
 * mrp_group_get_name:
 * @group: an #MrpGroup
 *
 * Retrives the name of @group.
 *
 * Return value: the name
 **/
const gchar *
mrp_group_get_name (MrpGroup *group)
{
	g_return_val_if_fail (MRP_IS_GROUP (group), NULL);

	return group->priv->name;
}

/**
 * mrp_group_set_name:
 * @group: an #MrpGroup
 * @name: new name of @group
 *
 * Sets the name of @group.
 **/
void
mrp_group_set_name (MrpGroup *group, const gchar *name)
{
	g_return_if_fail (MRP_IS_GROUP (group));
	g_return_if_fail (name != NULL);

	mrp_object_set (MRP_OBJECT (group), "name", name, NULL);
}
static void
group_assignment_removed_cb (MrpGroupAssignment *assignment,
				MrpGroup *group)
{
	MrpGroupPriv *priv;
	MrpTask         *task;

	g_return_if_fail (MRP_IS_GROUP (group));
	g_return_if_fail (MRP_IS_GROUP_ASSIGNMENT (assignment));

	priv = group->priv;

	task = mrp_group_assignment_get_task (assignment);

	if (!task) {
		g_warning ("Task not found in group's assignment list");
		return;
	}
	printf("group_assignment_removed_cb!\n");
	priv->assignments = g_list_remove (priv->assignments, assignment);

	//g_signal_emit (resource, signals[ASSIGNMENT_REMOVED],
	//	       0,
	//	       assignment);

	g_object_unref (assignment);

	mrp_object_changed (MRP_OBJECT (group));
}
/**
 * mrp_group_add_assignment:
 * @resource: an #MrpResource
 * @assignment: an #MrpAssignment
 *
 * Adds an assignment to @resource. This increases the reference count of
 * @assignment.
 *
 **/
void
imrp_group_add_assignment (MrpGroup *group, MrpGroupAssignment *assignment)
{
	MrpGroupPriv *priv;
	GList *l;

	g_return_if_fail (MRP_IS_GROUP (group));
	g_return_if_fail (MRP_IS_GROUP_ASSIGNMENT (assignment));

	priv = group->priv;

	priv->assignments = g_list_prepend (priv->assignments,
					    g_object_ref (assignment));

	printf("from imrp_group_add_assignment of %s::\n\t print assignments:\n",priv->name);
	for(l = priv->assignments;l;l=l->next) {
		printf("\t\t\t task:%s\n",
							mrp_task_get_name(mrp_group_assignment_get_task(
									MRP_GROUP_ASSIGNMENT(l->data))));
	}

	g_signal_connect (G_OBJECT (assignment),
			  "removed",
			  G_CALLBACK (group_assignment_removed_cb),
			  group);

	//g_signal_emit (group, signals[ASSIGNMENT_ADDED], 0, assignment);

	mrp_object_changed (MRP_OBJECT (group));
}
/**
 * mrp_group_assign:
 * @group: an #group
 * @task: an #MrpTask
 * @units: the amount of units of assignment
 *
 * Assigns @group to @task by the given amount of @units. A value of 100
 * units corresponds to fulltime assignment.
 *
 **/
void
mrp_group_assign (MrpGroup *group,
		     MrpTask     *task,
		     gint         units)
{
	MrpGroupAssignment   *assignment;

	g_return_if_fail (MRP_IS_GROUP (group));
	g_return_if_fail (MRP_IS_TASK (task));

	assignment = g_object_new (MRP_TYPE_GROUP_ASSIGNMENT,
				   "group", group,
				   "task", task,
				   "units", units,
				   NULL);

	imrp_group_add_assignment (group, assignment);//todo:add here
	imrp_task_add_group_assignment (task, assignment);//todo:add at task.c
	//todo:add signal of task
	//todo:add signal "removed" connect of group_assignment

	g_object_unref (assignment);
}

/**
 * mrp_group_get_assignments:
 * @group: an #MrpGroup.
 *
 * Retrieves the assignments that this group has. If caller needs to
 * manipulate the returned list, a copy of it needs to be made.
 *
 * Return value: The assignments of @group. It should not be freed.
 **/
GList *
mrp_group_get_assignments (MrpGroup *group)
{
	g_return_val_if_fail (MRP_IS_GROUP (group), NULL);

	return group->priv->assignments;
}

static void
group_remove_assignment_foreach (MrpGroupAssignment *assignment,
				    MrpGroup   *group)
{
	g_return_if_fail (MRP_IS_GROUP_ASSIGNMENT (assignment));

	g_signal_handlers_disconnect_by_func (MRP_OBJECT (assignment),
					      group_assignment_removed_cb,
					      group);

	g_object_unref (assignment);

	mrp_object_removed (MRP_OBJECT (assignment));
}


#include <config.h>

#include <glib/gi18n.h>
#include "mrp-task.h"
#include "mrp-resource.h"
#include "mrp-marshal.h"
#include "mrp-group-assignment.h"

struct _MrpGroupAssignmentPriv {
	MrpTask     *task;
	MrpGroup *group;

	gint         units;
};

/* Properties */
enum {
        PROP_0,
        PROP_TASK,
        PROP_GROUP,
        PROP_UNITS
};


static void group_assignment_class_init        (MrpGroupAssignmentClass *klass);
static void group_assignment_init              (MrpGroupAssignment      *assignment);
static void group_assignment_finalize          (GObject            *object);
static void group_assignment_set_property      (GObject            *object,
					  guint               prop_id,
					  const GValue       *value,
					  GParamSpec         *pspec);
static void group_assignment_get_property      (GObject            *object,
					  guint               prop_id,
					  GValue             *value,
					  GParamSpec         *pspec);

static MrpObjectClass *parent_class;

GType
mrp_group_assignment_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (MrpGroupAssignmentClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) group_assignment_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (MrpGroupAssignment),
			0,              /* n_preallocs */
			(GInstanceInitFunc) group_assignment_init,
		};

		type = g_type_register_static (MRP_TYPE_OBJECT,
					       "MrpGroupAssignment",
					       &info, 0);
	}

	return type;
}

static void
group_assignment_class_init (MrpGroupAssignmentClass *klass)
{
        GObjectClass   *object_class     = G_OBJECT_CLASS (klass);

        parent_class = MRP_OBJECT_CLASS (g_type_class_peek_parent (klass));

        object_class->finalize     = group_assignment_finalize;
        object_class->set_property = group_assignment_set_property;
        object_class->get_property = group_assignment_get_property;

	/* Properties */
        g_object_class_install_property (object_class,
                                         PROP_TASK,
                                         g_param_spec_object ("task",
							      "Task",
							      "The task",
							      MRP_TYPE_TASK,
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
        								PROP_GROUP,
                                         g_param_spec_object ("group",
							      "Group",
							      "The group that is assigned to the task",
							      MRP_TYPE_GROUP,
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_UNITS,
                                         g_param_spec_int ("units",
							   "Units",
							   "Number of units assignment",
							   -1,
							   G_MAXINT,
							   0,
							   G_PARAM_READWRITE));
}

static void
group_assignment_init (MrpGroupAssignment *assignment)
{
        MrpGroupAssignmentPriv *priv;

        priv = g_new0 (MrpGroupAssignmentPriv, 1);

        assignment->priv = priv;
}

static void
group_assignment_finalize (GObject *object)
{
        MrpGroupAssignment     *assignment = MRP_GROUP_ASSIGNMENT (object);
        MrpGroupAssignmentPriv *priv;

        priv = assignment->priv;

	if (priv->task) {
		g_object_unref (priv->task);
		priv->task = NULL;
	}

	if (priv->group) {
		g_object_unref (priv->group);
		priv->group = NULL;
	}

        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static void
group_assignment_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	MrpGroupAssignment     *assignment;
	MrpGroupAssignmentPriv *priv;

	assignment = MRP_GROUP_ASSIGNMENT (object);
	priv       = assignment->priv;

	/* FIXME: See bug #138368 about this. The assignment doesn't have a
	 * project pointer so we can't emit changed on it. We cheat for now and
	 * use the resource/task in those cases.
	 */

	switch (prop_id) {
	case PROP_TASK:
		if (priv->task) {
			g_object_unref (priv->task);
		}
		priv->task = g_object_ref (g_value_get_object (value));
		mrp_object_changed (MRP_OBJECT (priv->task));
		break;

	case PROP_GROUP:
		if (priv->group) {
			g_object_unref (priv->group);
		}
		priv->group = g_object_ref (g_value_get_object (value));
		mrp_object_changed (MRP_OBJECT (priv->group));
		break;

	case PROP_UNITS:
		priv->units = g_value_get_int (value);
		mrp_object_changed (MRP_OBJECT (priv->group));
		break;

	default:
		break;
	}
}

static void
group_assignment_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	MrpGroupAssignment     *assignment;
	MrpGroupAssignmentPriv *priv;

	assignment = MRP_GROUP_ASSIGNMENT (object);
	priv       = assignment->priv;

	switch (prop_id) {
	case PROP_TASK:
		g_value_set_object (value, priv->task);
		break;
	case PROP_GROUP:
		g_value_set_object (value, priv->group);
		break;
	case PROP_UNITS:
		g_value_set_int (value, priv->units);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/**
 * mrp_group_assignment_new:
 *
 * Creates a new, empty, group assignment. You most often don't want to create an
 * assignment explicitly like this, but using mrp_resource_assign() instead.
 *
 * Return value: Newly created group assignment.
 **/
MrpGroupAssignment *
mrp_group_assignment_new (void)
{
        MrpGroupAssignment *assignment;

        assignment = g_object_new (MRP_TYPE_GROUP_ASSIGNMENT, NULL);

        return assignment;
}

/**
 * mrp_assignment_get_task:
 * @assignment: an #MrpAssignment
 *
 * Retrieves the #MrpTask associated with @assignment.
 *
 * Return value: the task associated with the assignment object. The reference
 * count of the task is not increased.
 **/
MrpTask *
mrp_group_assignment_get_task (MrpGroupAssignment *assignment)
{
	g_return_val_if_fail (MRP_IS_GROUP_ASSIGNMENT (assignment), NULL);

	return assignment->priv->task;
}

/**
 * mrp_assignment_get_resource:
 * @assignment: an #MrpAssignment
 *
 * Retrieves the #MrpResource associated with @assignment.
 *
 * Return value: the resource associated with the assignment object. The reference
 * count of the resource is not increased.
 **/
MrpGroup *
mrp_group_assignment_get_group (MrpGroupAssignment *assignment)
{
	g_return_val_if_fail (MRP_IS_GROUP_ASSIGNMENT (assignment), NULL);

	return assignment->priv->group;
}

/**
 * mrp_assignment_get_units:
 * @assignment: an #MrpAssignment
 *
 * Retrieves the number of units that the resource is assigned with to the
 * task. 100 means 100%, etc.
 *
 * Return value: number of units of the assignment.
 **/
gint
mrp_group_assignment_get_units (MrpGroupAssignment *assignment)
{
	g_return_val_if_fail (MRP_IS_GROUP_ASSIGNMENT (assignment), -1);

	return assignment->priv->units;
}

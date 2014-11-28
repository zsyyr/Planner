/*
 * planner-pertchartnode.c
 *
 *  Created on: Oct 14, 2013
 *      Author: zms
 */

#include <config.h>

#include <glib/gi18n.h>
#include "libplanner/mrp-task.h"
#include "libplanner/mrp-resource.h"
#include "planner-pertchartnode.h"

GList *pertnodes = NULL;

struct _PlannerPertchartNodePriv {
	MrpTask     *task;
	MrpResource *resource;
	PertchartNodeType type;
	gint row;
	gint col;
	gboolean seted;
};

/* Properties */
enum {
        PROP_0,
        PROP_TASK,
        PROP_RESOURCE,
        PROP_TYPE,
        PROP_ROW,
        PROP_COL,
        PROP_SETED
};
GType planner_pertchart_node_type_get_type(void);
MrpTaskGraphNode * pertchart_node_get_graphnode(PlannerPertchartNode *);
static void plannerpertchartnode_class_init        (PlannerPertchartNodeClass *klass);
static void plannerpertchartnode_init              (PlannerPertchartNode      *pertnode);
static void plannerpertchartnode_finalize          (GObject            *object);
static void plannerpertchartnode_set_property      (GObject            *object,
					  guint               prop_id,
					  const GValue       *value,
					  GParamSpec         *pspec);
static void plannerpertchartnode_get_property      (GObject            *object,
					  guint               prop_id,
					  GValue             *value,
					  GParamSpec         *pspec);

static MrpObjectClass *parent_class;

GType
planner_pertchart_node_type_get_type(void)
{
	static GType etype = 0;
		if (etype == 0) {
			static const GEnumValue values[] = {
				{ PLANNER_PERTCHART_NODE_REAL, "PLANNER_PERTCHART_NODE_REAL", "real" },
				{ PLANNER_PERTCHART_NODE_VIRTUAL, "PLANNER_PERTCHART_NODE_VIRTUAL", "virtual" },
				{ 0, NULL, NULL }
			};
			etype = g_enum_register_static ("PertchartNodeType", values);
		}
		return etype;
}

GType
planner_pertchart_node_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (PlannerPertchartNodeClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) plannerpertchartnode_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (PlannerPertchartNode ),
			0,              /* n_preallocs */
			(GInstanceInitFunc) plannerpertchartnode_init,
		};

		type = g_type_register_static (MRP_TYPE_OBJECT,
					       "PlannerPertchartNode",
					       &info, 0);
	}

	return type;
}

static void
plannerpertchartnode_class_init (PlannerPertchartNodeClass *klass)
{
        GObjectClass   *object_class     = G_OBJECT_CLASS (klass);

        parent_class = MRP_OBJECT_CLASS (g_type_class_peek_parent (klass));

        object_class->finalize     = plannerpertchartnode_finalize;
        object_class->set_property = plannerpertchartnode_set_property;
        object_class->get_property = plannerpertchartnode_get_property;

	/* Properties */
        g_object_class_install_property (object_class,
                                         PROP_TASK,
                                         g_param_spec_object ("task",
							      "Task",
							      "The task",
							      MRP_TYPE_TASK,
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_RESOURCE,
                                         g_param_spec_object ("resource",
							      "Resource",
							      "The resource that is assigned to the task",
							      MRP_TYPE_RESOURCE,
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_TYPE,
                                         g_param_spec_enum ("type",
                                         "type",
                                         "real or virtual of the node",
                                         PLANNER_PERTCHARTNODE_TYPE,
                                         PLANNER_PERTCHART_NODE_REAL,
                                         G_PARAM_READWRITE));
       g_object_class_install_property (object_class,
                                         PROP_ROW,
                                         g_param_spec_int ("row",
                                         "the number of row",
                                         "the row number of the situation of node",
                                         G_MININT,
                                         G_MAXINT,
                                         -1,
                                         G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_COL,
                                         g_param_spec_int ("col",
              							 "the number of col",
              							 "the col number of the situation of node",
              							 G_MININT,
              							 G_MAXINT,
              							 -1,
              							 G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_SETED,
                                         g_param_spec_boolean ("seted",
                    				     "the node has been seted",
                    					 "the flag that whether the node has been seted",
                    					 FALSE,
                    					 G_PARAM_READWRITE));

}

static void
plannerpertchartnode_init (PlannerPertchartNode *pertnode)
{
		PlannerPertchartNodePriv *priv;

        priv = g_new0 (PlannerPertchartNodePriv, 1);

        //pertnode->type = PLANNER_PERTCHART_NODE_REAL;

        pertnode->priv = priv;
}

static void
plannerpertchartnode_finalize (GObject *object)
{
		PlannerPertchartNode *pertnode = PLANNER_PERTCHART_NODE(object);
		PlannerPertchartNodePriv *priv;

        priv = pertnode->priv;

	if (priv->task) {
		g_object_unref (priv->task);
		priv->task = NULL;
	}

	if (priv->resource) {
		g_object_unref (priv->resource);
		priv->resource = NULL;
	}

        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static void
plannerpertchartnode_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	PlannerPertchartNode     *pertnode;
	PlannerPertchartNodePriv *priv;

	pertnode = PLANNER_PERTCHART_NODE (object);
	priv     = pertnode->priv;



	switch (prop_id) {
	case PROP_TASK:
		if (priv->task) {
			g_object_unref (priv->task);
		}
		priv->task = g_object_ref (g_value_get_object (value));
		mrp_object_changed (MRP_OBJECT (priv->task));
		break;

	case PROP_RESOURCE:
		if (priv->resource) {
			g_object_unref (priv->resource);
		}
		priv->resource = g_object_ref (g_value_get_object (value));
		mrp_object_changed (MRP_OBJECT (priv->resource));
		break;

	case PROP_TYPE:
			priv->type = g_value_get_enum (value);
			mrp_object_changed (MRP_OBJECT (priv->type));
			break;

	case PROP_ROW:
		priv->row = g_value_get_int (value);
		//mrp_object_changed (MRP_OBJECT (priv->row));
		break;

	case PROP_COL:
			priv->col = g_value_get_int (value);
			//mrp_object_changed (MRP_OBJECT (priv->col));
			break;
	case PROP_SETED:
				priv->seted = g_value_get_boolean (value);
				//mrp_object_changed (MRP_OBJECT (priv->seted));
				break;
	default:
		break;
	}
}

static void
plannerpertchartnode_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
		PlannerPertchartNode     *pertnode;
		PlannerPertchartNodePriv *priv;

		pertnode = PLANNER_PERTCHART_NODE (object);
		priv     = pertnode->priv;

	switch (prop_id) {
	case PROP_TASK:
		g_value_set_object (value, priv->task);
		break;
	case PROP_RESOURCE:
		g_value_set_object (value, priv->resource);
		break;
	case PROP_TYPE:
		g_value_set_enum (value, priv->type);
		break;
	case PROP_ROW:
			g_value_set_int (value, priv->col);
			break;
	case PROP_COL:
			g_value_set_int (value, priv->col);
			break;
	case PROP_SETED:
		    g_value_set_boolean (value, priv->seted);
			break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}





MrpTask *
planner_pertchart_node_get_task (PlannerPertchartNode *pertnode)
{
	g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), NULL);

	return pertnode->priv->task;
}


MrpResource *
planner_pertchart_node_get_resource (PlannerPertchartNode *pertnode)
{
	g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), NULL);

	return pertnode->priv->resource;
}


PertchartNodeType
planner_pertchart_node_get_nodetype (PlannerPertchartNode *pertnode)
{
	g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), 0);
	return pertnode->priv->type;
}
/*
PlannerPertchartNode *
planner_pertchart_node_set_type (PlannerPertchartNode *pertnode,GValue value)
{
	g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), NULL);

    pertnode->type =g_value_get_int (value) ;
    return NULL;
}
*/
gint
planner_pertchart_node_get_row (PlannerPertchartNode *pertnode)
{
	//g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), NULL);

	return pertnode->priv->row;
}

gint
planner_pertchart_node_get_col (PlannerPertchartNode *pertnode)
{
	//g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), NULL);

	return pertnode->priv->col;
}

gboolean
planner_pertchart_node_get_seted (PlannerPertchartNode *pertnode)
{
	g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertnode), NULL);

	return pertnode->priv->seted;
}

MrpTaskGraphNode *
pertchart_node_get_graphnode(PlannerPertchartNode *pertchartnode)
{
	MrpTask *task;

	g_return_val_if_fail (PLANNER_IS_PERTCHART_NODE (pertchartnode), NULL);
	task = planner_pertchart_node_get_task (pertchartnode);
	return imrp_task_get_graph_node(task);
}

PlannerPertchartNode *
planner_pertchart_node_new()
{
	PlannerPertchartNode *pertnode;
	pertnode = g_object_new(PLANNER_TYPE_PERTCHART_NODE,NULL);
	return pertnode;
}

GList *
planner_pertchart_nodes_creat(GList *task_list)
{
	GList *l;
	GList *l1;
	GList *l2;

	GList *newpertnodes = NULL;
	pertnodes = newpertnodes;
	MrpTaskVirtualType virtualtype;
	PertchartNodeType perttype;

	g_printf("-----------in planner_pertchart_nodes_creat function\n");
	gint aa = g_list_length(task_list);
				g_printf("%d\n",aa);
	for(l2 = task_list;l2;l2=l2->next)
	{
		//g_printf("llllllllll333333333333333333333333333333333lllllllllllllllll\n");
		pertnodes = g_list_append(pertnodes,g_object_ref(planner_pertchart_node_new()));
	}
	l1 = pertnodes;
	for(l = task_list;l;l=l->next)
	{
		PlannerPertchartNode *pertnode;
		pertnode = l1->data;
		virtualtype = mrp_task_get_task_virtual_type(l->data);

		/*if(type == MRP_TASK_TYPE_NORMAL || type == MRP_TASK_TYPE_MILESTONE)
			perttype = PLANNER_PERTCHART_NODE_REAL;
		else
			perttype = PLANNER_PERTCHART_NODE_VIRTUAL;
*/
		g_object_set(pertnode,"task",l->data,NULL);
		//pertnode
		g_printf("lllllllllllllllllllllllllllllllllllllllllllll\n");
		//g_object_set(pertnode,"type",perttype,NULL);
		//g_object_set(pertnode,"type",type,NULL);
		pertnode->priv->type = virtualtype;
		l1 = l1->next;
	}

	return pertnodes;
}

gint
tasksCompareByName(MrpTask *task1,MrpTask *task2)
{
	gchar *s1 = mrp_task_get_name(task1);
	gchar *s2 = mrp_task_get_name(task2);
	return g_strcmp0(s1,s2);
}

GList *
tasksToPertnodes(GList *tasks)
{
	GList *nodesuccessors = NULL;
	GList *l1 = NULL;
	GList *l2 = NULL;
	g_printf("-----------in tasksToPertnodes function\n");
	g_printf("%d",g_list_length(tasks));
	for(l1=tasks;l1;l1=l1->next)
	{
		//g_printf("-----------------in the frist loop\n");
		gchar *s1 = mrp_task_get_name(MRP_TASK(l1->data));
		g_printf("%s",s1);
		g_printf("\n");
		for(l2 = pertnodes;l2;l2=l2->next)
		{
			MrpTask *task;
			task = planner_pertchart_node_get_task (l2->data);
			gchar *s2 = mrp_task_get_name(task);
			/*g_printf("%s",s2);
			g_printf("\n");
			g_printf("%d",g_strcmp0(s1,s2));
			g_printf("\n");*/
			if(!g_strcmp0(s1,s2))
			{
				nodesuccessors = g_list_append(nodesuccessors,l2->data);
			}
		}
	}
	g_printf("the successors list number is%d",g_list_length(nodesuccessors));
	g_printf("\n");
	g_printf("-----------out tasksToPertnodes function\n");
	return nodesuccessors;
}

GList *
getPertchartNodebrothers(PlannerPertchartNode *pertchartnode)
{

	GList *l = NULL;
	GList *pertnodebrothers = NULL;
	MrpTask *successor;
	MrpTask *task = planner_pertchart_node_get_task(pertchartnode);
	GList *predecessors = imrp_task_peek_predecessors(task);
	MrpTask *predecessor = mrp_relation_get_predecessor(predecessors->data);
	GList *successors = imrp_task_peek_successors(predecessor);
	//MrpTask *successor = successors->data;
	for(l=successors;l;l=l->next)
	{
		successor = mrp_relation_get_successor(l->data);
		if(!tasksCompareByName(task,successor))
			continue;
		else
			pertnodebrothers = g_list_prepend(pertnodebrothers,successor);
	}
	return pertnodebrothers;
}

PlannerPertchartNode *
getNodeByRowCol(GList *pertchartnodes,gint row,gint col)
{
	GList *l1 = NULL;
	gint prow = 0;
	gint pcol = 0;
	g_printf("-----------in getNodeByRowCol function\n");
	for(l1=pertchartnodes;l1;l1=l1->next)
	{
		pcol = planner_pertchart_node_get_col(l1->data);
		prow = planner_pertchart_node_get_row(l1->data);
		if(pcol == col && prow == row)
		{
			g_printf("the node has find\n");
			return l1->data;
		}
	}
	g_printf("not find\n");
	g_printf("-----------out getNodeByRowCol function\n");
	return NULL;
}



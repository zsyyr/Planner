/*
 * planner-pertchartnode.h
 *
 *  Created on: Oct 14, 2013
 *      Author: zms
 */

#ifndef _PLANNER_PERTCHARTNODE_H_
#define _PLANNER_PERTCHARTNODE_H_

#include <gtk/gtk.h>
#include <glib-object.h>
#include <config.h>
#include <gtk/gtk.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <stdio.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-resource.h>
#include <libplanner/mrp-task.h>
#include <libplanner/mrp-calendar.h>
#include <libplanner/mrp-private.h>
#include <glib/gi18n.h>

#define PLANNER_TYPE_PERTCHART_NODE            (planner_pertchart_node_get_type ())
#define PLANNER_PERTCHART_NODE(obj)            (GTK_CHECK_CAST ((obj), PLANNER_TYPE_PERTCHART_NODE, PlannerPertchartNode))
#define PLANNER_PERTCHART_NODE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), PLANNER_TYPE_PERTCHART_NODE, PlannerPertchartNodeClass))
#define PLANNER_IS_PERTCHART_NODE(obj)         (GTK_CHECK_TYPE ((obj), PLANNER_TYPE_PERTCHART_NODE))
#define PLANNER_IS_PERTCHART_NODE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), PLANNER_TYPE_PERTCHART_NODE))
#define PLANNER_PERTCHART_NODE_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), PLANNER_TYPE_PERTCHART_NODE, PlannerPertchartNodeClass))
#define PLANNER_PERTCHARTNODE_TYPE				(planner_pertchart_node_type_get_type())

typedef struct _PlannerPertchartNode      PlannerPertchartNode;
typedef struct _PlannerPertchartNodeClass PlannerPertchartNodeClass;
typedef struct _PlannerPertchartNodePriv  PlannerPertchartNodePriv;

typedef enum{
	PLANNER_PERTCHART_NODE_REAL = 0,
	PLANNER_PERTCHART_NODE_VIRTUAL = 1
}PertchartNodeType;

struct _PlannerPertchartNode {
	MrpObject  parent;

	PlannerPertchartNodePriv  *priv;
};

struct _PlannerPertchartNodeClass {
	MrpObjectClass parent_class;
};

extern GList *pertnodes;

MrpTaskGraphNode * pertchart_node_get_graphnode(PlannerPertchartNode *);
GType planner_pertchart_node_get_type (void);
PlannerPertchartNode *planner_pertchart_node_new (void);
MrpTask *planner_pertchart_node_get_task (PlannerPertchartNode *pertnode);
MrpResource *planner_pertchart_node_get_resource (PlannerPertchartNode *pertnode);
PertchartNodeType planner_pertchart_node_get_nodetype (PlannerPertchartNode *pertnode);
gint planner_pertchart_node_get_row (PlannerPertchartNode *pertnode);
gint planner_pertchart_node_get_col (PlannerPertchartNode *pertnode);
GList *planner_pertchart_nodes_creat(GList *task_list);
GList *tasksToPertnodes(GList *tasks);
//GList *

#endif /* PLANNER_PERTCHARTNODE_H_ */

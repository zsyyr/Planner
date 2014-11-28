/*
 * planner-show-view.h
 *
 *  Created on: Nov 11, 2013
 *      Author: root
 */

#ifndef PLANNER_SHOW_VIEW_H_
#define PLANNER_SHOW_VIEW_H_

#include <gtk/gtk.h>
#include "planner-view.h"
#include "planner-pert-chart.h"

#define PLANNER_TYPE_SHOW_VIEW	           (planner_show_view_get_type ())
#define PLANNER_SHOW_VIEW(obj)	           (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_SHOW_VIEW, PlannerShowView))
#define PLANNER_SHOW_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PLANNER_TYPE_SHOW_VIEW, PlannerShowViewClass))
#define PLANNER_IS_SHOW_VIEW(obj)	   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_SHOW_VIEW))
#define PLANNER_IS_SHOW_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLANNER_TYPE_SHOW_VIEW))
#define PLANNER_SHOW_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLANNER_TYPE_SHOW_VIEW, PlannerShowViewClass))

typedef struct _PlannerShowView       PlannerShowView;
typedef struct _PlannerShowViewClass  PlannerShowViewClass;
typedef struct _PlannerShowViewPriv   PlannerShowViewPriv;

struct _PlannerShowView {
	PlannerView         parent;
	PlannerShowViewPriv *priv;
};

struct _PlannerShowViewClass {
	PlannerViewClass parent_class;
};

GType        planner_show_view_get_type     (void) G_GNUC_CONST;
PlannerView *planner_show_view_new          (void);

extern GList *firsttasklist;
#endif /* PLANNER_SHOW_VIEW_H_ */

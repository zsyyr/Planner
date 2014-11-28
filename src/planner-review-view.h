/*
 * planner-review-view.h
 *
 *  Created on: Apr 9, 2014
 *      Author: zms
 */

#ifndef PLANNER_REVIEW_VIEW_H_
#define PLANNER_REVIEW_VIEW_H_

#include <gtk/gtk.h>
#include "planner-view.h"
#include "planner-pert-chart.h"

#define PLANNER_TYPE_REVIEW_VIEW	           (planner_review_view_get_type ())
#define PLANNER_REVIEW_VIEW(obj)	           (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_REVIEW_VIEW, PlannerReviewView))
#define PLANNER_REVIEW_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PLANNER_TYPE_REVIEW_VIEW, PlannerReviewViewClass))
#define PLANNER_IS_REVIEW_VIEW(obj)	   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_REVIEW_VIEW))
#define PLANNER_IS_REVIEW_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLANNER_TYPE_REVIEW_VIEW))
#define PLANNER_REVIEW_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLANNER_TYPE_REVIEW_VIEW, PlannerReviewViewClass))

typedef struct _PlannerReviewView       PlannerReviewView;
typedef struct _PlannerReviewViewClass  PlannerReviewViewClass;
typedef struct _PlannerReviewViewPriv   PlannerReviewViewPriv;

struct _PlannerReviewView {
	PlannerView         parent;
	PlannerReviewViewPriv *priv;
};

struct _PlannerReviewViewClass {
	PlannerViewClass parent_class;
};

GType        planner_review_view_get_type     (void) G_GNUC_CONST;
PlannerView *planner_review_view_new          (void);

extern GList *firsttasklist;


#endif /* PLANNER_REVIEW_VIEW_H_ */

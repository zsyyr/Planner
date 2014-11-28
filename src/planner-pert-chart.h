/*
 * planner-pert-chart.h

 *
 *  Created on: May 21, 2013
 *      Author: zms
 */

#ifndef _PLANNER_PERT_CHART_H_
#define _PLANNER_PERT_CHART_H_


#include <glib-object.h>

#include <gtk/gtk.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <stdio.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-resource.h>
#include <libplanner/mrp-task.h>

#include <libplanner/mrp-private.h>


#define MAXLEN12 200
#define X_GAP 10
#define Y_GAP 20
#define XWIDTH12 30
#define YHIGH12 30
#define WIDTH12 95
#define HIGH12 60

#include <glib/gi18n.h>
#include <libgnomecanvas/gnome-canvas.h>

#include "planner-pertchartnode.h"



typedef struct {
  gint x;
  gint y;
  gint w;
  gint h;
  gchar *name;
  gint type;
}Chart;

typedef struct{
  gint x1;
  gint y1;
  gint x2;
  gint y2;
  gint x3;
  gint y3;
  gint x4;
  gint y4;
}Arrow;

void drawbutton(GtkWidget *layout,gchar *tname,gint x,gint y,gint type,MrpProject *project);
gboolean getposition(GList *tasklist);

GList *setPertNodesPosition();




#endif /* PLANNER_PERT_CHART_H_ */

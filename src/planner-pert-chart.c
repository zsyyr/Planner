/*
 * planner-pert-chart.c
 *
 *  Created on: May 21, 2013
 *      Author: zms
 */
#include <config.h>
#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-resource.h>
#include <libplanner/mrp-task.h>
#include <libplanner/mrp-calendar.h>
#include <glib/gi18n.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>
#include <libgnomecanvas/gnome-canvas-line.h>
#include <libplanner/mrp-relation.h>
#include "planner-marshal.h"
#include "planner-format.h"
#include "planner-gantt-row.h"
#include "planner-gantt-chart.h"
#include "planner-canvas-line.h"
#include "eel-canvas-rect.h"
#include "dummy-canvas-item.h"
#include "planner-scale-utils.h"
#include "planner-task-tree.h"
#include "planner-task-popup.h"
#include "planner-task-cmd.h"
#include "planner-pert-chart.h"



gint nbCols;
gint nbRows;
Chart pertchart[MAXLEN12];
Arrow arrow[MAXLEN12];
gint click = 0;
gint Lin = 0;
GList *buttons;
gint layoutexp = 0;
//void drawbutton(GtkWidget *layout,gchar *tname,gint x,gint y);
void redrawGertToPert(GtkWidget *widget, gpointer data);
gboolean getposition(GList *tasklist);
GtkWidget *findWidgetByName(GList *buttonlist, gchar *name);
static gboolean isZeroPosition(PlannerPertchartNode *pertchartnode);
static gboolean on_expose_event(GtkWidget * widget, GdkEventExpose * event, gpointer *task);

 void
 reDrawdd(GtkWidget *widget,gpointer data)
 {
	 //click &= click;
	 GtkWidget *layout;
	 if(click)
		 click = 0;
	 else
		 click = 1;
	 if(click)
		 {
			 GdkColor color;
			 color.red = 0;
			 color.green = 0;
			 color.blue = 2000;
			 gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &color);
		 }
	 else
	 {
	 		 GdkColor color;
	 		 color.red = 0;
	 		 color.green = 2000;
	 		 color.blue = 0;
	 		 gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &color);
	 	 }
	 click++;
	// gtk_widget_set_redraw_on_allocate (widget,TRUE);
	 layout = gtk_widget_get_parent(widget);
	 gtk_widget_show (layout);
 }





 void
  reDrawd(GtkButton *widget,gpointer project)
 {
	 GtkWidget *layout;
	 layout = gtk_widget_get_parent(widget);
	 GtkWidget *brotherbutton;

	 GtkWidget    *windows;
	 GtkWidget    *scrollwindows;
	 scrollwindows = gtk_widget_get_parent(layout);
	 windows = gtk_widget_get_parent(scrollwindows);

	 GdkColor color2;
	 color2.red = 10000;
	 color2.green = 50000;
	 color2.blue = 10000;

	 gtk_widget_modify_bg(widget, GTK_STATE_INSENSITIVE, &color2);
	 gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &color2);

	 gtk_widget_set_sensitive(widget,FALSE);
	 gchar *buttonname = gtk_widget_get_name(widget);

	 g_printf("the clicked button is %s\n",buttonname);

	 GList *l = pertnodes;
	 GList *l1;
	 PlannerPertchartNode *clickednode;
	 GList *string = NULL;
	 for(;l;l=l->next)
	 {
		 gchar *pertnodename = mrp_task_get_name(planner_pertchart_node_get_task(l->data));
		 if(!g_strcmp0(buttonname,pertnodename))
		 {
			 clickednode = l->data;
			 break;
		 }
	 }

	 GList *brothers = getPertchartNodebrothers(clickednode);
	 for(l1=brothers;l1;l1=l1->next)
	 {
		 gchar *name = mrp_task_get_name(l1->data);

		 brotherbutton = findWidgetByName(buttons,name);


		 GdkColor color;
		 color.red = 50000;
		 color.green = 10000;
		 color.blue = 10000;
		 gtk_widget_modify_bg(brotherbutton, GTK_STATE_INSENSITIVE, &color);
        gtk_widget_set_sensitive(brotherbutton,FALSE);
		 gtk_button_set_label(brotherbutton, "deleted");
		 mrp_project_remove_task (project,l1->data);
	 }

	 mrptime lastduration = totalduration(project);
	 displaylastduration(lastduration);
 }
 GtkWidget *labelChild;
 PangoFontDescription *font1;
 short fontSize = 10;
 void drawbutton(GtkWidget *layout,gchar *tname,gint x,gint y,gint type,MrpProject *project)
 {
	 GtkWidget *button;

	 gboolean sensitive;
	 sensitive = type;
	 gchar *buttonname = tname;

	 button = gtk_button_new_with_label (buttonname);
	 gtk_layout_put(layout,button,x,y);
	 //gtk_widget_set_size_request(button, WIDTH12,HIGH12);
	 gtk_widget_set_size_request(button, 95,50);
	 gtk_widget_set_name (button,tname);


	  font1 = pango_font_description_from_string("Sans");//"Sans"字体名
	  pango_font_description_set_size(font1, fontSize * PANGO_SCALE);//设置字体大小
	  labelChild = gtk_bin_get_child(GTK_BIN( button));//取出GtkButton里的label
	  pango_font_description_set_weight(font1,700);
	  gtk_widget_modify_font(GTK_WIDGET(labelChild), font1);//设置label的字体， 这样这个GtkButton上面显示的字体就变了


	 gtk_widget_set_sensitive(button,sensitive);

	 if(sensitive)
	 {
		 GdkColor color1;
		 color1.red = 0;
		 color1.green = 50100;
		 color1.blue = 50000;
		 gtk_widget_modify_bg(button, GTK_STATE_NORMAL, &color1);
	 }

	 buttons = g_list_prepend(buttons,button);

	 gtk_widget_show (button);
	 g_signal_connect (G_OBJECT (button), "clicked",
	 	           G_CALLBACK (reDrawd),
	 	           project);
 }



 GtkWidget *
 findWidgetByName(GList *buttonlist,gchar *name)
 {
	 gchar *buttonname = NULL;
	 GList *l = NULL;
	 for(l=buttonlist;l;l=l->next)
	 {
		 buttonname = gtk_widget_get_name(l->data);
		 if(!g_strcmp0(name,buttonname))
		 {
			 return l->data;
		 }
	 } return NULL;
 }

 void
 redrawGertToPert(GtkWidget *widget,gpointer data)
 {
	 GtkWidget *layout;

	 GtkWidget *scrollwindows;
	 layout = gtk_widget_get_parent(widget);
	 g_printf("redraw the pert node,name is %s\n",data);
	 //gtk_widget_hide(layout);
	 scrollwindows = gtk_widget_get_parent(layout);
	 gtk_container_remove(scrollwindows,layout);

	 GtkWidget *newlayout;
	 GtkWidget *button;
	 newlayout = gtk_layout_new(NULL,NULL);

	 button=gtk_button_new_with_label("改变主窗口label文字");
	 gtk_layout_put(newlayout, button, 497, 250);
	 gtk_widget_set_size_request(button, 80, 65);
	 gtk_container_add(GTK_CONTAINER(scrollwindows), newlayout);
	 gtk_widget_show(newlayout);
	 gtk_widget_show_all(scrollwindows);
 }

 static gboolean
  on_expose_event (GtkWidget * widget, GdkEventExpose * event, gpointer *project)
  {
  cairo_t *cr;
  GdkWindow *gdkw;
  gint i;
  gdkw = gtk_layout_get_bin_window (GTK_LAYOUT(widget));
  cr = gdk_cairo_create (gdkw);
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, 1);


  if(!layoutexp)
  {
  for(i=0;i<=MAXLEN12;i++)
    {
  	  if(pertchart[i].name != NULL)
        {
  		  drawbutton(widget,pertchart[i].name,pertchart[i].x,pertchart[i].y,pertchart[i].type,project);
         }
     }
  for(i=0;i<Lin;i++)
   {
 		   cr = gdk_cairo_create (gdkw);
 		   cairo_set_source_rgb (cr, 0, 0, 0);
           cairo_set_line_width (cr, 1);
           cairo_move_to (cr, arrow[i].x1,arrow[i].y1);
           cairo_line_to (cr, arrow[i].x2,arrow[i].y2);
           cairo_move_to (cr, arrow[i].x2,arrow[i].y2);
           cairo_line_to (cr, arrow[i].x3,arrow[i].y3);
           cairo_move_to (cr, arrow[i].x3,arrow[i].y3);
           cairo_line_to (cr, arrow[i].x4,arrow[i].y4);

           cairo_stroke_preserve (cr);
           cairo_set_source_rgb (cr, 1, 1, 1);
           cairo_fill (cr);
           cairo_destroy (cr);
    }
  }
  else
  {
	  for(i=0;i<Lin;i++)
	     {
	   		   cr = gdk_cairo_create (gdkw);
	   		   cairo_set_source_rgb (cr, 0, 0, 0);
	             cairo_set_line_width (cr, 1);
	             cairo_move_to (cr, arrow[i].x1,arrow[i].y1);
	             cairo_line_to (cr, arrow[i].x2,arrow[i].y2);
	             cairo_move_to (cr, arrow[i].x2,arrow[i].y2);
	             cairo_line_to (cr, arrow[i].x3,arrow[i].y3);
	             cairo_move_to (cr, arrow[i].x3,arrow[i].y3);
	             cairo_line_to (cr, arrow[i].x4,arrow[i].y4);

	             cairo_stroke_preserve (cr);
	             cairo_set_source_rgb (cr, 1, 1, 1);
	             cairo_fill (cr);
	             cairo_destroy (cr);}
  }
  layoutexp++;
  return FALSE;
  }


 void
 drawtask(GtkWidget *layout,MrpProject *project)
 {
	 g_signal_connect (layout, "expose-event",G_CALLBACK (on_expose_event), project);

 }

 gboolean
 getposition(GList *pertchart_node_list)
 {
	 g_printf("------------in getposition function\n");
	 gboolean v;
	 GList *l;
	 gint i = 0;
	 gint a;
	 gint b;
	 MrpTask *task;
	 gint type;
	 for(l = pertnodes;l;l=l->next)
	 	{
		    task = planner_pertchart_node_get_task (l->data);
		    a = planner_pertchart_node_get_row(l->data);
		    b = planner_pertchart_node_get_col(l->data);
		    type = planner_pertchart_node_get_nodetype(l->data);
	 		pertchart[i].x = 95*b + 10*b;
	 		pertchart[i].y = 50*a + 30*a;
	 		pertchart[i].w = 95;
	 		pertchart[i].h = 50;
	 		pertchart[i].type = type;
	 		//g_printf("lll333\n");
	 		pertchart[i].name = mrp_task_get_name (task);
	 		i++;
	 	}
	 return v;
 }

Chart *
 getPositionByTaskName(GList *pertchartnodes,gchar *taskname)
 {
	 MrpTask *t;
	 GList *l = NULL;
	 gint i;
	 gint n = g_list_length(pertchartnodes);
	 for(i=0;i<n;i++)
	 {
		gchar *s = pertchart[i].name;
		if (g_str_equal(s, taskname))
			return &pertchart[i];
	 }
	 return NULL;
 }




static gboolean
isZeroPosition(PlannerPertchartNode *pertchartnode)
{
	GList l;
	MrpTaskGraphNode *graphnode;
	g_printf("------------in iszero function\n");
	graphnode = pertchart_node_get_graphnode(pertchartnode);

	if(graphnode->prev == NULL)
	{
		g_printf("------------out iszero function\n");
		return TRUE;
	}
	else
	{
		g_printf("------------out iszero function\n");
		return FALSE;
	}
}

static GList *
getPertNodeInColumn(gint col)
{
	GList *nodeincolumns = NULL;
	GList *l= NULL;
	gint colnumber;
	g_printf("-----------in getPertNodeInColumn function\n");
	for(l=pertnodes;l;l=l->next)
	{
		colnumber = planner_pertchart_node_get_col(PLANNER_PERTCHART_NODE(l->data));
		if(colnumber == col)
		{
			PlannerPertchartNode *nodeincolumn;
			nodeincolumns = g_list_prepend(nodeincolumns,l->data);
		}
	}
	g_printf("-----------out getPertNodeInColumn function\n");
	return nodeincolumns;
}

static GList *
getPertNodeSuccessors(PlannerPertchartNode *pertchartnode)
{
	MrpTaskGraphNode *graphnode;
	GList *nodesuccessors;
	GList *l;
	g_printf("-----------in getPertNodeSuccessors function\n");

	graphnode = pertchart_node_get_graphnode(pertchartnode);
	nodesuccessors = graphnode->next;
	nodesuccessors = tasksToPertnodes(graphnode->next);
	g_printf("-----------out getPertNodeSuccessors function\n");
	return nodesuccessors;
}

static GList *
getPertNodeAncestor(PlannerPertchartNode *pertchartnode)
{
	MrpTaskGraphNode *graphnode;
	GList *nodeancestors;
	GList *l;
	graphnode = pertchart_node_get_graphnode(pertchartnode);
	nodeancestors = tasksToPertnodes(graphnode->prev);
	return nodeancestors;
}

static GList *
getPertNodeSuccessorsInOneCol(gint col)
{
	GList *l = NULL;
	GList *ll = NULL;
	GList *successorsincol = NULL;
	GList *nodesuccessors = NULL;

	g_printf("-----------in getPertNodeSuccessorsInOneCol function\n");

	l = getPertNodeInColumn(col);
	if(l == NULL)
		return NULL;
	for(;l;l=l->next)
	{
		nodesuccessors = getPertNodeSuccessors(PLANNER_PERTCHART_NODE(l->data));
		for(ll=nodesuccessors;ll;ll=ll->next)
		{
			successorsincol = g_list_append(successorsincol,PLANNER_PERTCHART_NODE(ll->data));
		}
	}
	g_printf("-----------out getPertNodeSuccessorsInOneCol function\n");

	return successorsincol;
}

static gboolean
isOccupied(int row,int col)
{
	GList *l = NULL;
	gint noderow;
	g_printf("-----------in isOccupied function\n");
	l = getPertNodeInColumn(col);

	for(;l;l=l->next)
		{
		 	noderow = planner_pertchart_node_get_row(PLANNER_PERTCHART_NODE(l->data));

		 	if(noderow == row)
		 		 return TRUE;

		}
	g_printf("-----------out isOccupied function\n");
	return FALSE;
}

static void
addNode(int col,PlannerPertchartNode *pertchartnode)
{

	g_printf("-----------in add function\n");
	//GList *l = pertnodes;
	pertnodes = g_list_remove(pertnodes,pertchartnode);
	if(nbCols-1 < col)
	{
		nbCols = col + 1;
	}
	int row = 0;
	//g_printf("%d",isOccupied(row,col));

	while(isOccupied(row,col))
	{
		row++;
	}

	g_object_set(G_OBJECT(pertchartnode),"col",col,"row",row,"seted",TRUE,NULL);
	pertnodes = g_list_prepend(pertnodes,pertchartnode);
}

void
removeNode(PlannerPertchartNode *pertchartnode)
{
	GList *l;
	g_printf("-----------in remove function\n");
	pertnodes = g_list_remove(pertnodes,pertchartnode);
	gint nodecol = planner_pertchart_node_get_col(pertchartnode);
	gint noderow = planner_pertchart_node_get_row(pertchartnode);
	gint rownub;
	if(nodecol == -1)
		return;
	l = getPertNodeInColumn(nodecol);
	for(;l;l= l->next)
	{
		rownub = planner_pertchart_node_get_row(l->data);
		if(rownub > noderow)
			{
				rownub--;
				g_object_set(l->data,"row",rownub,NULL);
			}
	}

	if(nodecol == nbCols-1)
	{
		l = getPertNodeInColumn(nbCols-1);
		while(l != NULL)
		{
			nbCols--;
			l = getPertNodeInColumn(nbCols-1);
		}
	}
	g_object_set(pertchartnode,"col",-1,"row",-1,"seted",FALSE,NULL);
	g_printf("-----------out remove function\n");
}

/*static gboolean isCrossingNode(PlannerPertchartNode *pertchartnode)
{


}*/



GList *
setPertNodesPosition()
{
	GList *l;
	GList *ll;
	GList *nodesuccessors;

	nbCols = 0;
	GList *l3;
	for(l3 = pertnodes;l3;l3=l3->next)
			{
		        g_object_set(G_OBJECT(l3->data),"col",-1,"row",-1,"seted",FALSE,NULL);
			}

	for(l3 = pertnodes;l3;l3=l3->next)
		{
			gint a = planner_pertchart_node_get_col (l3->data);
			gint b = planner_pertchart_node_get_row (l3->data);

		}

	l=pertnodes;
	while(l)
	{

	   gboolean set = planner_pertchart_node_get_seted(l->data);

       if(!set && isZeroPosition(l->data))
       {
    		   addNode(0,l->data);
    		   l=pertnodes;
    	}


       l = l->next;
	}
	gint nodecol = 0;
	nodesuccessors = getPertNodeSuccessorsInOneCol(nodecol);
	while(nodesuccessors != NULL)
	{
		for(ll=nodesuccessors;ll;ll=ll->next)
		{
			gboolean set = planner_pertchart_node_get_seted(ll->data);
			if(set)
			{
				removeNode(ll->data);
			}
			g_printf("the next col of nodesuccessors is%d",nodecol+1);
			addNode(nodecol + 1,ll->data);
		}
		nodecol++;

		nodesuccessors = getPertNodeSuccessorsInOneCol(nodecol);
	}

	return pertnodes;
}

gboolean
isCrossingNode(PlannerPertchartNode *pertchartnode)
{
	g_printf("-----------in isCrossingNode function\n");
	GList *l = NULL;
	GList *ancestors = getPertNodeAncestor(pertchartnode);
	gint ancol = 0;
	gint nodecol = 0;
	gint noderow = 0;
	gint col = 0;
	nodecol = planner_pertchart_node_get_col(pertchartnode);
	noderow = planner_pertchart_node_get_row(pertchartnode);
	for(l = ancestors;l;l=l->next)
	{
		PlannerPertchartNode *ancestor = l->data;
		ancol = planner_pertchart_node_get_col(ancestor);
		if(ancol < nodecol-1)
			for(col=nodecol-1;col>ancol;col--)
			{
				if(isOccupied(noderow,col))
					return TRUE;
			}
	}
	g_printf("-----------out isCrossingNode function\n");
	return FALSE;
}

void
moveDown(PlannerPertchartNode *pertchartnode)
{
	gint row = planner_pertchart_node_get_row(pertchartnode);
	gint col = planner_pertchart_node_get_col(pertchartnode);
	while(isOccupied(++row,col));
	g_object_set(pertchartnode,"row",row,NULL);
}

void
moveRight(PlannerPertchartNode *pertchartnode)
{
	GList *l = NULL;
	l = getPertNodeSuccessors(pertchartnode);
	gint row = planner_pertchart_node_get_row(pertchartnode);
	gint col = planner_pertchart_node_get_col(pertchartnode);
	for(;l;l=l->next)
	{
		moveRight(l->data);
	}
	gint newcol = col + 1;
	if(isOccupied(row,newcol))
	{
		moveRight(getNodeByRowCol(pertnodes,row,newcol));
	}
	g_object_set(pertchartnode,"col",newcol,NULL);
	if(newcol == nbCols)
	{
		nbCols++;
	}
}

gboolean
isCrossingArrow(PlannerPertchartNode *pertchartnode)
{
	int maxUp = 10000,maxDown = -1;
	GList *l1 = NULL;
	GList *l2 = NULL;
	GList *l3 = NULL;
	GList *l4 = NULL;
	l1 = getPertNodeSuccessors(pertchartnode);
	for(;l1;l1=l1->next)
	{
		gint row = planner_pertchart_node_get_row(l1->data);
		if(row < maxUp)
			maxUp = row;
		if(row > maxDown)
			maxDown = row;
	}

	gint col = planner_pertchart_node_get_col(pertchartnode);
	l2 = getPertNodeInColumn(col);
	l2 = g_list_remove(l2,pertchartnode);


	for(;l2;l2=l2->next)
	{
		l3 = getPertNodeSuccessors(l2->data);
		for(;l3;l3=l3->next)
		{
			gint row1 = planner_pertchart_node_get_row(pertchartnode);
			gint col1 = planner_pertchart_node_get_col(pertchartnode);
			gint row2 = planner_pertchart_node_get_row(l2->data);
			gint col2 = planner_pertchart_node_get_col(l2->data);
			gint row3 = planner_pertchart_node_get_row(l3->data);
			gint col3 = planner_pertchart_node_get_col(l3->data);
			gint sig = 0;
			for(l4=getPertNodeSuccessors(pertchartnode);l4;l4=l4->next)
			{
				gchar *name1 = mrp_task_get_name(MRP_TASK(planner_pertchart_node_get_task(l4->data)));
				gchar *name2 = mrp_task_get_name(MRP_TASK(planner_pertchart_node_get_task(l3->data)));
				if(!g_strcmp0(name1,name2))
					sig = 1;
			}

			if(maxUp < row1)
			{
				if(row3 <= row1 && !sig)
					return TRUE;
			}
			if(maxDown > row1)
			{
				if(row3 >= row1 && !sig)
					return TRUE;
			}
		}
	}
	return FALSE;
}

void
avoidCrossingNode()
{
	g_printf("-----------in avoidCrossingNode function\n");
	GList *l = NULL;
	if(nbCols == 0)
		return;

	int col = nbCols-1;
	while(col > 0)
	{
		gboolean hasmoved = FALSE;
		for(l = getPertNodeInColumn(col);l;l=l->next)
		{
			while(isCrossingNode(l->data))
			{
				moveDown(l->data);
				hasmoved = TRUE;
			}
		}

		if(hasmoved && col < nbCols-1)
			col++;
		else
			col--;

	}
	g_printf("-----------out avoidCrossingNode function\n");
}

void
avoidCrossingLine()
{
	gboolean restart;
	GList *l1 = NULL;
	GList *l2 = NULL;
	gint col = 0;
	g_printf("-----------in avoidCrossingLine function\n");
	do
	{
		restart = FALSE;
		for(col=0;col<nbCols && restart==FALSE;col++)
		{
			l1 = getPertNodeInColumn(col);
			if(g_list_length(l1) > 1)
			{
				for(;l1;l1=l1->next)
				{
					if(isCrossingArrow(l1->data))
					{
						moveRight(l1->data);
						avoidCrossingNode();
						restart = TRUE;
						break;
					}
				}
			}
		}
	}while(restart);
	g_printf("-----------out avoidCrossingLine function\n");
}

void
removeEmptyColumn()
{
	GList *l = NULL;
	gint c = 0;
	gint col;
	gint nodecol = 0;
	for(col=nbCols-1;col>=0;col--)
	{
		if(g_list_length(getPertNodeInColumn(col)) == 0)
		{
			if(col != nbCols-1)
			{
				for(c = col+1;c<nbCols;c++)
				{
					for(l=getPertNodeInColumn(c);l;l=l->next)
					{
						nodecol = planner_pertchart_node_get_col(l->data);
						nodecol--;
						g_object_set(l->data,"col",nodecol,NULL);
					}
				}
			}
			nbCols--;
		}
	}
}

void
 getArrowPosition(GList *pertchartnodes)
 {
 	GList *l1 = NULL;
 	GList *l2 = NULL;
 	gint i = 0;
 	Chart *anposition = NULL;
 	Chart *suposition = NULL;

 	g_printf("-----------@#$@#$in getArrowPosition function\n");

 	gint j;
 	for(j=0;j<200;j++)
 	{
 		arrow[j].x1 = 0;
 	}

 	for(l1=pertchartnodes;l1;l1=l1->next)
 	{
 		gchar *ancestorname = mrp_task_get_name(MRP_TASK(planner_pertchart_node_get_task(l1->data)));
 		anposition = getPositionByTaskName(pertnodes,ancestorname);
 		if(anposition)
 		{
 			l2 = getPertNodeSuccessors(l1->data);
 			for(;l2;l2=l2->next)
 			{
 				g_printf("*******************************in the 2th loop*******************************\n");
 				gchar *successorname = mrp_task_get_name(MRP_TASK(planner_pertchart_node_get_task(l2->data)));
 				suposition = getPositionByTaskName(pertnodes,successorname);
 				if(suposition)
 				{
 					arrow[i].x1 = anposition->x + anposition->w;
 					arrow[i].y1 = anposition->y + anposition->h/2;
 					arrow[i].x2 = anposition->x + anposition->w + X_GAP/2;
 					arrow[i].y2 = anposition->y + anposition->h/2;
 					arrow[i].x3 = anposition->x + anposition->w + X_GAP/2;
 					arrow[i].y3 = suposition->y + suposition->h/2;
 					arrow[i].x4 = suposition->x;
 					arrow[i].y4 = suposition->y + suposition->h/2;
 					i++;
 				}
 				else
 				{
 		 			g_printf("do not find the position\n");
 		 			//break;
 		 		}
 			}
 		}
 		else
 		{
 			g_printf("do not find the position\n");
 			//break;
 		}
 		//i++;
 	}
 	Lin = i;
 	g_printf("%d\n",i);
 	g_printf("-----------out getArrowPosition function\n");
 }

//void







/*
 * planner-simulat.c
 *
 *  Created on: 2013-11-14
 *      Author: zms
 */

#include <config.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-resource.h>
#include <libplanner/mrp-task.h>
#include <libplanner/mrp-calendar.h>
#include "libplanner/mrp-assignment.h"
#include "planner-marshal.h"
#include "planner-format.h"
#include "planner-gantt-row.h"
#include "planner-gantt-chart.h"
#include "planner-task-tree.h"
#include "planner-task-popup.h"
#include "planner-task-cmd.h"
#include "planner-pert-chart.h"
#include "planner-pertchartnode.h"
#include "planner-simulat.h"
#include <stdio.h>
#include <cairo.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
/*
 * simulation type
 */
enum {
	RESOURCEDELETE,
	EMERGENCYTASK,
	TASKDELAYED,
	ALLDELAYED,
	PROBABILITY //if virtual=0,prob should bigger than 1,and means delay.
};

gint
randNumber()
{
	//srand((int) time(0));
	return rand();
}


GList *
getAllTasks(MrpProject * project)
{
	MrpTaskManager *task_manager = imrp_project_get_task_manager(project);
	return  mrp_task_manager_get_all_tasks(task_manager);
}

gint
getRandomNumber(GList *tasklist)
{
	gint modular = g_list_length(tasklist);
	if(modular)
	{
		return randNumber()%modular;
	}
	else
		return 0;
}

MrpTask *
getRandomTask(GList *tasklist)
{
	//GList *tasklist = getAllTasks(project);
	gint modular = g_list_length(tasklist);
	gint tasknumber;
	if(modular)
	{
		tasknumber = randNumber()%modular;
		return g_list_nth_data(tasklist,tasknumber);
	}
	else
		return NULL;
}

MrpResource *
getRandomResource(GList *resourcelist)
{
	gint modular = g_list_length(resourcelist);
	gint resourcenumber;
	if(modular)
	{
		resourcenumber = randNumber()%modular;
		return g_list_nth_data(resourcelist,resourcenumber);
	}
	else
		return NULL;
}

MrpResource *
getDelRandomResourceFromTask(MrpTask *randomtask)
{
	GList *assignmentresources = mrp_task_get_assigned_resources(randomtask);
	return getRandomResource(assignmentresources);
}

void
deleteRandomResourceFromTask(GList *tasklist)
{
	MrpTask *randomtask = getRandomTask(tasklist);
	MrpResource *delresource = getDelRandomResourceFromTask(randomtask);

	gchar *s1 = mrp_task_get_name(randomtask);
	gchar *s2 = mrp_resource_get_name(delresource);
	g_printf("the random task is %s,the deleted resource is %s\n",s1,s2);

	MrpAssignment *assignment = mrp_task_get_assignment (randomtask,delresource);
	mrp_object_removed (MRP_OBJECT (assignment));
}

void
setDelayedDuration(MrpTask *task)
{
	mrptime delayedduration;
	mrptime duration = mrp_task_get_duration(task);
	gint probability = mrp_task_get_probability(task);
	if(probability)
		delayedduration = duration*(probability*0.01+1);
	else
		delayedduration = duration*1.5;
	//imrp_task_set_work(task,delayedduration);
	g_object_set (task, "sched",1,NULL);
	g_object_set (task, "duration", delayedduration, NULL);
}


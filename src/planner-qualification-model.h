/*
 * planner-qualification-model.h
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

#ifndef __PLANNER_QUALIFICATION_MODEL_H__
#define __PLANNER_QUALIFICATION_MODEL_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-qualification.h>
#include "planner-list-model.h"

#define PLANNER_TYPE_QUALIFICATION_MODEL	          (planner_qualification_model_get_type ())
#define PLANNER_QUALIFICATION_MODEL(obj)	          (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_QUALIFICATION_MODEL, PlannerQualificationModel))
#define PLANNER_QUALIFICATION_MODEL_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), PLANNER_TYPE_QUALIFICATION_MODEL, PlannerQualificationModelClass))
#define PLANNER_IS_QUALIFICATION_MODEL(obj)	          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_QUALIFICATION_MODEL))
#define PLANNER_IS_QUALIFICATION_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), PLANNER_TYPE_QUALIFICATION_MODEL))
#define PLANNER_QUALIFICATION_MODEL_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), PLANNER_TYPE_QUALIFICATION_MODEL, PlannerQualificationModelClass))

typedef struct _PlannerQualificationModel       PlannerQualificationModel;
typedef struct _PlannerQualificationModelClass  PlannerQualificationModelClass;
typedef struct _PlannerQualificationModelPriv   PlannerQualificationModelPriv;

struct _PlannerQualificationModel
{
        PlannerListModel       parent;

        PlannerQualificationModelPriv *priv;
};

struct _PlannerQualificationModelClass
{
	PlannerListModelClass  parent_class;
};

enum {
        QUALIFICATION_COL_NAME,
        QUALIFICATION_COL_NOTE,
        QUALIFICATION_COL,
        NUMBER_OF_QUALIFICATION_COLS
};


GType           planner_qualification_model_get_type        (void) G_GNUC_CONST;
PlannerQualificationModel *  planner_qualification_model_new             (MrpProject      *project);

#endif /* __PLANNER_QUALIFICATION_MODEL_H__ */

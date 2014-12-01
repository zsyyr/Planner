/*
 * mrp-qualification.h
 *
 *  Created on: 2014-11-17
 *      Author: zms
 */

#ifndef MRP_QUALIFICATION_H_
#define MRP_QUALIFICATION_H_

#include <libplanner/mrp-object.h>
#include <libplanner/mrp-types.h>
#define MRP_TYPE_QUALIFICATION         (mrp_qualification_get_type ())
#define MRP_QUALIFICATION(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), MRP_TYPE_QUALIFICATION, MrpQualification))
#define MRP_QUALIFICATION_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), MRP_TYPE_QUALIFICATION, MrpQualificationClass))
#define MRP_IS_QUALIFICATION(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), MRP_TYPE_QUALIFICATION))
#define MRP_IS_QUALIFICATION_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), MRP_TYPE_QUALIFICATION))
#define MRP_QUALIFICATION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), MRP_TYPE_QUALIFICATION, MrpQualificationClass))

typedef struct _MrpQualificationClass MrpQualificationClass;
typedef struct _MrpQualificationPriv  MrpQualificationPriv;

struct _MrpQualification {
        MrpObject     parent;
        MrpQualificationPriv *priv;
};

struct _MrpQualificationClass {
        MrpObjectClass parent_class;
};

GType         mrp_qualification_get_type     (void) G_GNUC_CONST;

MrpQualification *    mrp_qualification_new          (void);
const gchar * mrp_qualification_get_name     (MrpQualification    *qualification);
void          mrp_qualification_set_name     (MrpQualification    *qualification,
				       gchar *name);
void mrp_qualification_assign (MrpQualification *qualification,MrpTask     *task,
		     gint         units);
GList *mrp_qualification_get_assignments (MrpQualification *qualification);

#endif /* MRP_QUALIFICATION_H_ */

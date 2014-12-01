/*
 * mrp-qualification.c
 *
 *  Created on: 2014-11-17
 *      Author: zms
 */

#include <config.h>

#include <glib/gi18n.h>
#include "mrp-task.h"
#include "mrp-resource.h"
#include "mrp-marshal.h"
#include "mrp-qualification.h"

struct _MrpQualificationPriv {
	MrpProject 		*project;
	gchar 				*name;
	gchar 				*note;
};

/* Properties */
enum {
        PROP_0,
        PROP_PROJECT,
        PROP_NAME,
        PROP_NOTE
};


static void qualification_class_init        (MrpQualificationClass *klass);
static void qualification_init              (MrpQualification      *qualification);
static void qualification_finalize          (GObject            *object);
static void qualification_set_property      (GObject            *object,
					  guint               prop_id,
					  const GValue       *value,
					  GParamSpec         *pspec);
static void qualification_get_property      (GObject            *object,
					  guint               prop_id,
					  GValue             *value,
					  GParamSpec         *pspec);

static MrpObjectClass *parent_class;

GType
mrp_qualification_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (MrpQualificationClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) qualification_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (MrpQualification),
			0,              /* n_preallocs */
			(GInstanceInitFunc) qualification_init,
		};

		type = g_type_register_static (MRP_TYPE_OBJECT,
					       "MrpQualification",
					       &info, 0);
	}

	return type;
}

static void
qualification_class_init (MrpQualificationClass *klass)
{
        GObjectClass   *object_class     = G_OBJECT_CLASS (klass);

        parent_class = MRP_OBJECT_CLASS (g_type_class_peek_parent (klass));

        object_class->finalize     = qualification_finalize;
        object_class->set_property = qualification_set_property;
        object_class->get_property = qualification_get_property;

	/* Properties */
        g_object_class_install_property (object_class,
                                         PROP_PROJECT,
                                         g_param_spec_object ("project",
							      "Project",
							      "The project",
							      MRP_TYPE_PROJECT,
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
        									  PROP_NAME,
                                         g_param_spec_string ("name",
							      "Qname",
							      "The qnme",
							      "empty",
							      G_PARAM_READWRITE));
        g_object_class_install_property(object_class,
        							PROP_NOTE,
        							g_param_spec_string("note",
        							"Qnote",
        							"The note",
        							"empty",
        							G_PARAM_READWRITE));
}

static void
qualification_init (MrpQualification *qualification)
{
        MrpQualificationPriv *priv;
        priv->name  = g_strdup ("");
        priv->note = g_strdup ("");
        priv->project = NULL;
        priv = g_new0 (MrpQualificationPriv, 1);

        qualification->priv = priv;
}

static void
qualification_finalize (GObject *object)
{
        MrpQualification     *qualification = MRP_QUALIFICATION (object);
        MrpQualificationPriv *priv;

        priv = qualification->priv;

	if (priv->project) {
		g_object_unref (priv->project);
		priv->project = NULL;
	}

        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static void
qualification_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	MrpQualification     *qualification;
	MrpQualificationPriv *priv;
	gboolean      changed = FALSE;
	const gchar *str;
	qualification = MRP_QUALIFICATION (object);
	priv          =   qualification->priv;

	switch (prop_id) {
	case PROP_PROJECT:
		if (priv->project) {
			g_object_unref (priv->project);
		}
		priv->project = g_object_ref (g_value_get_object (value));
		mrp_object_changed (MRP_OBJECT (priv->project));
		break;

	case PROP_NAME:
		str = g_value_get_string (value);

				if (!priv->name || strcmp (priv->name, str)) {
					g_free (priv->name);
					priv->name = g_strdup (str);
					changed = TRUE;
				}

				break;

	case PROP_NOTE:
		str = g_value_get_string (value);

				if (!priv->note || strcmp (priv->note, str)) {
					g_free (priv->note);
					priv->note = g_strdup (str);
					changed = TRUE;
				}

				break;

	default:
		break;
	}
	if (changed) {
		mrp_object_changed (MRP_OBJECT (object));
	}
}

static void
qualification_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	MrpQualification     *qualification;
	MrpQualificationPriv *priv;

	qualification = MRP_QUALIFICATION (object);
	priv       = qualification->priv;

	switch (prop_id) {
	case PROP_PROJECT:
		g_value_set_object (value, priv->project);
		break;
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_NOTE:
		g_value_set_string (value, priv->note);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


MrpQualification *
mrp_qualification_new (void)
{
        MrpQualification *qualification;

        qualification = g_object_new (MRP_TYPE_QUALIFICATION, NULL);

        return qualification;
}


MrpProject *
mrp_qualification_get_project (MrpQualification *qualification)
{
	g_return_val_if_fail (MRP_IS_QUALIFICATION (qualification), NULL);

	return qualification->priv->project;
}


const gchar *
mrp_qualification_get_name (MrpQualification *qualification)
{
	g_return_val_if_fail (MRP_IS_QUALIFICATION (qualification), NULL);

	return qualification->priv->name;
}


gchar *
mrp_qualification_get_note (MrpQualification *qualification)
{
	g_return_val_if_fail (MRP_IS_QUALIFICATION (qualification), -1);

	return qualification->priv->note;
}

void mrp_qualification_set_name(MrpQualification *qualif,gchar *name){
	g_return_if_fail (MRP_IS_QUALIFICATION (qualif));
		g_return_if_fail (name != NULL);
		qualif->priv->name =  g_strdup(name);

}

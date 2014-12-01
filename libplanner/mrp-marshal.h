
#ifndef __mrp_marshal_MARSHAL_H__
#define __mrp_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:OBJECT (./mrp-marshal.list:1) */
#define mrp_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT

/* VOID:OBJECT,OBJECT,OBJECT,BOOLEAN (./mrp-marshal.list:2) */
extern void mrp_marshal_VOID__OBJECT_OBJECT_OBJECT_BOOLEAN (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

/* VOID:POINTER (./mrp-marshal.list:3) */
#define mrp_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:POINTER,POINTER (./mrp-marshal.list:4) */
extern void mrp_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

/* VOID:OBJECT,INT (./mrp-marshal.list:5) */
extern void mrp_marshal_VOID__OBJECT_INT (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* VOID:VOID (./mrp-marshal.list:6) */
#define mrp_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:BOOLEAN (./mrp-marshal.list:7) */
#define mrp_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

/* VOID:LONG,POINTER (./mrp-marshal.list:8) */
extern void mrp_marshal_VOID__LONG_POINTER (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

G_END_DECLS

#endif /* __mrp_marshal_MARSHAL_H__ */



#ifndef __planner_marshal_MARSHAL_H__
#define __planner_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:VOID (./planner-marshal.list:1) */
#define planner_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:BOOLEAN (./planner-marshal.list:2) */
#define planner_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

/* VOID:BOOLEAN,STRING (./planner-marshal.list:3) */
extern void planner_marshal_VOID__BOOLEAN_STRING (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* VOID:POINTER (./planner-marshal.list:4) */
#define planner_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:OBJECT (./planner-marshal.list:5) */
#define planner_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT

/* VOID:STRING (./planner-marshal.list:6) */
#define planner_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:OBJECT,OBJECT (./planner-marshal.list:7) */
extern void planner_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

/* VOID:INT,INT,INT,INT (./planner-marshal.list:8) */
extern void planner_marshal_VOID__INT_INT_INT_INT (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);

/* VOID:STRING,INT,INT,INT,INT (./planner-marshal.list:9) */
extern void planner_marshal_VOID__STRING_INT_INT_INT_INT (GClosure     *closure,
                                                          GValue       *return_value,
                                                          guint         n_param_values,
                                                          const GValue *param_values,
                                                          gpointer      invocation_hint,
                                                          gpointer      marshal_data);

/* VOID:DOUBLE,DOUBLE,DOUBLE,DOUBLE (./planner-marshal.list:10) */
extern void planner_marshal_VOID__DOUBLE_DOUBLE_DOUBLE_DOUBLE (GClosure     *closure,
                                                               GValue       *return_value,
                                                               guint         n_param_values,
                                                               const GValue *param_values,
                                                               gpointer      invocation_hint,
                                                               gpointer      marshal_data);

/* VOID:OBJECT,BOOLEAN (./planner-marshal.list:11) */
extern void planner_marshal_VOID__OBJECT_BOOLEAN (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* OBJECT:VOID (./planner-marshal.list:12) */
extern void planner_marshal_OBJECT__VOID (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

G_END_DECLS

#endif /* __planner_marshal_MARSHAL_H__ */


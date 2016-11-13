/*
 * peas-plugin-loader-rbffi.c
 * This file is part of libpeas
 *
 * Copyright (C) 2016 - Thomas Martitz
 *
 * libpeas is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libpeas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "peas-plugin-loader-rbffi.h"

#include "libpeas/peas-plugin-info-priv.h"

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <ruby/ruby.h>
#include <ruby/thread.h>

typedef struct {
  VALUE module;
} PeasRbffiData;

typedef struct {
  VALUE loader_module;
  gint  loaded_plugins;
  char dummy_gvl_frame[32];
} PeasPluginLoaderRbffiPrivate;


G_DEFINE_TYPE_WITH_PRIVATE (PeasPluginLoaderRbffi,
                            peas_plugin_loader_rbffi,
                            PEAS_TYPE_PLUGIN_LOADER)

#define GET_PRIV(o) \
  (peas_plugin_loader_rbffi_get_instance_private (o))

static GQuark quark_extension_type = 0;

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_PLUGIN_LOADER,
                                              PEAS_TYPE_PLUGIN_LOADER_RBFFI);
}

static void
peas_rbffi_handle_exception(void)
{
  VALUE exc, ary, bt, str;

  exc = rb_errinfo();
  rb_set_errinfo(Qnil);

  ary = rb_ary_new();
  rb_ary_push(ary, rb_funcall(exc, rb_intern("message"), 0));
  bt = rb_funcall(exc, rb_intern("backtrace"), 0);
  if (!NIL_P(bt))
    rb_ary_concat(ary, bt);

  str = rb_funcall(ary, rb_intern("join"), 1, rb_str_new_cstr("\n\t"));
  g_error("Ruby exception!\n%s\n", StringValueCStr(str));
}

static gpointer
_call_ruby (gpointer data)
{
  GPtrArray *args = (GPtrArray *) data;
  VALUE receiver = (VALUE) args->pdata[0];
  gchar *method  = args->pdata[1];

  return (gpointer) rb_funcallv (receiver,
                                 rb_intern(method),
                                 args->len-2,
                                 (const VALUE *) &args->pdata[2]);
}


static VALUE
peas_rbffi_call_ruby_gvl (PeasPluginLoaderRbffi *rbloader,
                          const gchar *method,
                          const gchar *args_fmt,
                          ...)
{
  PeasPluginLoaderRbffiPrivate *priv = GET_PRIV (rbloader);
  VALUE arg, result;
  va_list va_args;
  GPtrArray *args;
  gint ch;
  const gchar *arg_s;
  gint arg_i;
  gsize arg_z;
  int state;

  args = g_ptr_array_sized_new(2);

  g_ptr_array_add(args, (gpointer) priv->loader_module);
  g_ptr_array_add(args, (gpointer) method);

  va_start (va_args, args_fmt);

  ch = *args_fmt++;
  while (ch)
    {
      switch (ch)
        {
        case 's':
          arg_s = va_arg (va_args, const gchar *);
          arg   = rb_str_new_cstr (arg_s);
          break;
        case 'i':
          arg_i = va_arg (va_args, gint);
          arg   = INT2NUM (arg_i);
          break;
        case 'z':
          arg_z = va_arg (va_args, gsize);
          arg   = SIZET2NUM (arg_z);
          break;
        case 'v':
          arg   = va_arg (va_args, VALUE);
          break;
        case '0':
          continue;
        default:
          g_assert_not_reached();
        }
        g_ptr_array_add (args, (gpointer) arg);
        ch = *args_fmt++;
    }

  va_end (va_args);

  result = rb_protect ((rb_block_call_func_t) _call_ruby, (VALUE) args, &state);

  if (state)
    {
      peas_rbffi_handle_exception();
      /* result == Qnil */
    }

  g_ptr_array_free(args, TRUE);

  return result;
}

static GType
find_extension_type (PeasPluginLoaderRbffi *rbloader,
                     PeasRbffiData         *plugin,
                     GType                  exten_type)
{
  VALUE result;

  result = peas_rbffi_call_ruby_gvl (rbloader, "find_extension_type",
                                     "vz", plugin->module, exten_type);

  return NIL_P(result) ? G_TYPE_INVALID : NUM2SIZET(result);
}

static gboolean
peas_plugin_loader_rbffi_provides_extension (PeasPluginLoader *loader,
                                             PeasPluginInfo   *info,
                                             GType             exten_type)
{
  GType type;

  type = find_extension_type(PEAS_PLUGIN_LOADER_RBFFI (loader),
                             info->loader_data, exten_type);

  return type != G_TYPE_INVALID;
}

static PeasExtension *
peas_plugin_loader_rbffi_create_extension (PeasPluginLoader *loader,
                                           PeasPluginInfo   *info,
                                           GType             exten_type,
                                           guint             n_parameters,
                                           GParameter       *parameters)
{
  PeasPluginLoaderRbffi *rbloader = PEAS_PLUGIN_LOADER_RBFFI (loader);
  GObject *object = NULL;

#if 0
  GType the_type;
  GParameter *exten_parameters;
  /* Add plugin-info to construct properties in the same
   * way as the C loader does.See peas-plugin-loader-c.c */
  exten_parameters = g_newa (GParameter, n_parameters + 1);
  memcpy (exten_parameters, parameters, sizeof (GParameter) * n_parameters);
  exten_parameters[n_parameters].name = g_intern_static_string ("plugin_info");
  memset (&exten_parameters[n_parameters].value, 0, sizeof (GValue));
  g_value_init (&exten_parameters[n_parameters].value, PEAS_TYPE_PLUGIN_INFO);
  g_value_set_boxed (&exten_parameters[n_parameters].value, info);

  the_type = find_extension_type (rbloader, info->loader_data, exten_type);
  if (the_type == G_TYPE_INVALID)
    goto out;

  object = g_object_newv (the_type, n_parameters + 1,
                                               exten_parameters);
  g_value_unset (&exten_parameters[n_parameters].value);
#else
  VALUE result;
  PeasRbffiData *plugin = info->loader_data;

  result = peas_rbffi_call_ruby_gvl(rbloader, "create_extension",
                                    "vzz", plugin->module, exten_type,
                                    g_boxed_copy(PEAS_TYPE_PLUGIN_INFO, info));

  if (NIL_P(result))
    goto out;

  object = (gpointer) NUM2SIZET(rb_funcall(rb_funcall(result, rb_intern("to_ptr"), 0), rb_intern("address"), 0));
#endif

  if (!object)
    goto out;


  /* We have to remember which interface we are instantiating
   * for the deprecated peas_extension_get_extension_type().
   */
  g_object_set_qdata (object, quark_extension_type,
                      GSIZE_TO_POINTER (exten_type));

out:
  return object;
}

static gboolean
peas_plugin_loader_rbffi_load (PeasPluginLoader *loader,
                               PeasPluginInfo   *info)
{
  PeasPluginLoaderRbffi *rbloader = PEAS_PLUGIN_LOADER_RBFFI (loader);
  //~ PeasPluginLoaderRbffiPrivate *priv = GET_PRIV (pyloader);
  const gchar *module_dir, *module_name;
  VALUE result;

  module_dir = peas_plugin_info_get_module_dir (info);
  module_name = peas_plugin_info_get_module_name (info);

  result = peas_rbffi_call_ruby_gvl (rbloader, "load", "sss",
                                     info->filename,
                                     module_dir, module_name);

  if (!NIL_P(result))
    {
      PeasRbffiData *data = g_new(PeasRbffiData, 1);
      data->module = result;
      info->loader_data = data;
      rb_gc_register_address(&data->module);

      return TRUE;
    }

  return FALSE;
}

static void
peas_plugin_loader_rbffi_unload (PeasPluginLoader *loader,
                                  PeasPluginInfo   *info)
{
  /* Note: We can't truly unload a module. We can only allow GC
   * to collect it by dropping our references to it but I don't think
   * Ruby ever collects loaded modules, as they are always referenced
   * in Module.constants */
  PeasRbffiData *data = info->loader_data;

  rb_gc_unregister_address(&data->module);

  g_free(data);
  info->loader_data = NULL;
}

static void
peas_plugin_loader_rbffi_garbage_collect (PeasPluginLoader *loader)
{
  /* rb_gc_enable/_disable return the old state (negated). We use this
   * to disable GC again when it was previously disabled. We have
   * to enable temporarily otherwise rb_gc_start() does nothing. */
  VALUE disabled = rb_gc_enable();

  rb_gc_start();

  if (disabled == Qtrue)
    rb_gc_disable();
}


static gboolean
peas_plugin_loader_rbffi_initialize (PeasPluginLoader *loader)
{
  PeasPluginLoaderRbffi *rbloader = PEAS_PLUGIN_LOADER_RBFFI (loader);
  PeasPluginLoaderRbffiPrivate *priv = GET_PRIV (rbloader);
  GBytes *internal_script;
  const gchar *scriptp;
  int state = 0;

  ruby_init();
  ruby_init_loadpath();

  internal_script = g_resources_lookup_data ("/org/gnome/libpeas/loaders/rbffi/"
                                             "internal.rb",
                                             G_RESOURCE_LOOKUP_FLAGS_NONE,
                                             NULL);
  if (internal_script == NULL)
    goto error;

  scriptp = g_bytes_get_data (internal_script, NULL);

  priv->loader_module = rb_eval_string_protect(scriptp, &state);
  if (state != 0)
    {
      peas_rbffi_handle_exception();
      return FALSE;
    }

  rb_gc_register_address(&priv->loader_module);

  {
    void (*rbffi_frame_push) (void* frame);
    GModule *mod = g_module_open (NULL, 0);
    if (g_module_symbol (mod, "rbffi_frame_push", (gpointer *) &rbffi_frame_push))
      rbffi_frame_push (&priv->dummy_gvl_frame);
    else
      g_warning("Unable to workaround ffi gvl issue\n");
    g_module_close (mod);
  }

  return TRUE;

error:
  g_warning("ruby initialization failed\n");
  return FALSE;
}

static void
peas_plugin_loader_rbffi_init (PeasPluginLoaderRbffi *pyloader)
{
}

static void
peas_plugin_loader_rbffi_finalize (GObject *object)
{
  PeasPluginLoaderRbffi *rbyloader = PEAS_PLUGIN_LOADER_RBFFI (object);
  PeasPluginLoaderRbffiPrivate *priv = GET_PRIV (rbyloader);

  {
    void (*rbffi_frame_pop) (void* frame);
    GModule *mod = g_module_open (NULL, 0);
    if (g_module_symbol (mod, "rbffi_frame_pop", (gpointer *) &rbffi_frame_pop))
      rbffi_frame_pop (&priv->dummy_gvl_frame);
    g_module_close (mod);
  }

  rb_gc_unregister_address (&priv->loader_module);

  ruby_finalize();

  G_OBJECT_CLASS (peas_plugin_loader_rbffi_parent_class)->finalize (object);
}

static void
peas_plugin_loader_rbffi_class_init (PeasPluginLoaderRbffiClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasPluginLoaderClass *loader_class = PEAS_PLUGIN_LOADER_CLASS (klass);

  quark_extension_type = g_quark_from_static_string ("peas-extension-type");

  object_class->finalize = peas_plugin_loader_rbffi_finalize;

  loader_class->initialize = peas_plugin_loader_rbffi_initialize;
  loader_class->load = peas_plugin_loader_rbffi_load;
  loader_class->unload = peas_plugin_loader_rbffi_unload;
  loader_class->create_extension = peas_plugin_loader_rbffi_create_extension;
  loader_class->provides_extension = peas_plugin_loader_rbffi_provides_extension;
  loader_class->garbage_collect = peas_plugin_loader_rbffi_garbage_collect;
}

/*
 * peas-plugin-loader-rbffi.h
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

#ifndef __PEAS_PLUGIN_LOADER_RBFFI_H__
#define __PEAS_PLUGIN_LOADER_RBFFI_H__

#include <libpeas/peas-plugin-loader.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

#define PEAS_TYPE_PLUGIN_LOADER_RBFFI             (peas_plugin_loader_rbffi_get_type ())
#define PEAS_PLUGIN_LOADER_RBFFI(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_PLUGIN_LOADER_RBFFI, PeasPluginLoaderRbffi))
#define PEAS_PLUGIN_LOADER_RBFFI_CONST(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), PEAS_TYPE_PLUGIN_LOADER_RBFFI, PeasPluginLoaderRbffi const))
#define PEAS_PLUGIN_LOADER_RBFFI_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), PEAS_TYPE_PLUGIN_LOADER_RBFFI, PeasPluginLoaderRbffiClass))
#define PEAS_IS_PLUGIN_LOADER_RBFFI(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PEAS_TYPE_PLUGIN_LOADER_RBFFI))
#define PEAS_IS_PLUGIN_LOADER_RBFFI_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), PEAS_TYPE_PLUGIN_LOADER_RBFFI))
#define PEAS_PLUGIN_LOADER_RBFFI_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), PEAS_TYPE_PLUGIN_LOADER_RBFFI, PeasPluginLoaderRbffiClass))

typedef struct _PeasPluginLoaderRbffi         PeasPluginLoaderRbffi;
typedef struct _PeasPluginLoaderRbffiClass    PeasPluginLoaderRbffiClass;

struct _PeasPluginLoaderRbffi {
  PeasPluginLoader parent;
};

struct _PeasPluginLoaderRbffiClass {
  PeasPluginLoaderClass parent_class;
};

GType                    peas_plugin_loader_rbffi_get_type  (void) G_GNUC_CONST;

/* All the loaders must implement this function */
G_MODULE_EXPORT void     peas_register_types                 (PeasObjectModule *module);

G_END_DECLS

#endif /* __PEAS_PLUGIN_LOADER_RBFFI_H__ */


/*
 * gpe-plugin.h
 * This file is part of libgpe
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GPE_PLUGIN_H__
#define __GPE_PLUGIN_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "gpe-plugin-info.h"

/* TODO: add a .h file that includes all the .h files normally needed to
 * develop a plugin */

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define GPE_TYPE_PLUGIN            (gpe_plugin_get_type())
#define GPE_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GPE_TYPE_PLUGIN, GPEPlugin))
#define GPE_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GPE_TYPE_PLUGIN, GPEPluginClass))
#define GPE_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GPE_TYPE_PLUGIN))
#define GPE_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPE_TYPE_PLUGIN))
#define GPE_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GPE_TYPE_PLUGIN, GPEPluginClass))

/**
 * GPEPlugin:
 *
 * Base class for plugins.
 */
typedef struct _GPEPlugin GPEPlugin;
typedef struct _GPEPluginPrivate GPEPluginPrivate;

struct _GPEPlugin
{
	GObject parent;
	GPEPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GPEPluginClass GPEPluginClass;

/**
 * GPEFunc:
 * @plugin: A #GPEPlugin;
 * @object: The target #GObject on which the handler is to be executed.
 *
 * The type of the handler methods of #GPEPlugin.
 */
typedef void (*GPEFunc) (GPEPlugin *plugin, GObject *object);


struct _GPEPluginClass
{
	GObjectClass parent_class;

	/* Virtual public methods */

	void 		(*activate)		(GPEPlugin *plugin,
						 GObject   *target_object);
	void 		(*deactivate)		(GPEPlugin *plugin,
						 GObject   *target_object);

	void 		(*update_ui)		(GPEPlugin *plugin,
						 GObject   *target_object);

	GtkWidget 	*(*create_configure_dialog)
						(GPEPlugin *plugin);

	/* Plugins should not override this, it's handled automatically by
	   the GPEPluginClass */
	gboolean 	(*is_configurable)
						(GPEPlugin *plugin);

	/* Padding for future expansion */
	void		(*_gpe_reserved1)	(void);
	void		(*_gpe_reserved2)	(void);
	void		(*_gpe_reserved3)	(void);
	void		(*_gpe_reserved4)	(void);
};

/*
 * Public methods
 */
GType 		 gpe_plugin_get_type 			(void) G_GNUC_CONST;

GPEPluginInfo	*gpe_plugin_get_info			(GPEPlugin *plugin);
gchar 		*gpe_plugin_get_data_dir		(GPEPlugin *plugin);

void 		 gpe_plugin_activate			(GPEPlugin *plugin,
							 GObject   *target_object);
void 		 gpe_plugin_deactivate			(GPEPlugin *plugin,
							 GObject   *target_object);

void 		 gpe_plugin_update_ui			(GPEPlugin *plugin,
							 GObject   *target_object);

gboolean	 gpe_plugin_is_configurable		(GPEPlugin *plugin);
GtkWidget	*gpe_plugin_create_configure_dialog	(GPEPlugin *plugin);

/**
 * GPE_REGISTER_TYPE_WITH_CODE(PARENT_TYPE, PluginName, plugin_name, CODE):
 * @PARENT_TYPE: The GPEPlugin subclass used as the parent type.
 * @PluginName: The name of the new plugin class, in CamelCase.
 * @plugin_name: The name of the new plugin class, in lower_case.
 * @CODE: Custom code that gets inserted in the *_get_type() function.
 *
 * A convenience macro for plugin implementations, which declares a subclass
 * of PARENT_TYPE (see G_DEFINE_DYNAMIC_TYPE_EXTENDED()) and a registration
 * function.  The resulting #GType is to be registered in a #GTypeModule.
 *
 * @CODE will be included in the resulting *_get_type() function, and will
 * usually consist of G_IMPLEMENT_INTERFACE() calls and eventually on
 * additional type registration
 */
#define GPE_REGISTER_TYPE_WITH_CODE(PARENT_TYPE, PluginName, plugin_name, CODE)	\
	G_DEFINE_DYNAMIC_TYPE_EXTENDED (PluginName,				\
					plugin_name,				\
					PARENT_TYPE,				\
					0,					\
					CODE)					\
										\
/* This is not very nice, but G_DEFINE_DYNAMIC wants it and our old macro	\
 * did not support it */							\
static void									\
plugin_name##_class_finalize (PluginName##Class *klass)				\
{										\
}										\
										\
G_MODULE_EXPORT GType								\
register_gpe_plugin (GTypeModule *type_module)					\
{										\
	plugin_name##_register_type (type_module);				\
										\
	return plugin_name##_get_type();					\
}

/**
 * GPE_REGISTER_TYPE(PARENT_TYPE, PluginName, plugin_name):
 * @PARENT_TYPE: The GPEPlugin subclass used as the parent type.
 * @PluginName: The name of the new plugin class, in CamelCase.
 * @plugin_name: The name of the new plugin class, in lower_case.
 *
 * A convenience macro for plugin implementations, which declares a subclass
 * of PARENT_TYPE (see G_DEFINE_DYNAMIC_TYPE_EXTENDED()) and a registration
 * function.  The resulting #GType is to be registered in a #GTypeModule.
 */
#define GPE_REGISTER_TYPE(PARENT_TYPE, PluginName, plugin_name)		\
	GPE_REGISTER_TYPE_WITH_CODE(PARENT_TYPE, PluginName, plugin_name, ;)

G_END_DECLS

#endif  /* __GPE_PLUGIN_H__ */

/*
 * peas-plugin-info.c
 * This file is part of libpeas
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2007 - Steve Frécinaux
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <glib.h>

#include "peas-i18n.h"
#include "peas-plugin-info-priv.h"

#ifdef G_OS_WIN32
#define OS_HELP_KEY "Help-Windows"
#elif defined(OS_OSX)
#define OS_HELP_KEY "Help-MacOS-X"
#else
#define OS_HELP_KEY "Help-GNOME"
#endif

/**
 * SECTION:peas-plugin-info
 * @short_description: Information about a plugin.
 *
 * A #PeasPluginInfo contains all the information available about a plugin.
 *
 * All this information comes from the related plugin info file, whose file
 * extension is ".plugin". Here is an example of such a plugin file, in the
 * #GKeyFile format:
 * |[
 * [Plugin]
 * Module=helloworld
 * Name=Hello World
 * Description=Displays "Hello World"
 * Authors=Steve Frécinaux &lt;code@istique.net&gt;
 * Copyright=Copyright © 2009-10 Steve Frécinaux
 * Website=http://live.gnome.org/Libpeas
 * Help=http://library.gnome.org/devel/libpeas/unstable/
 * ]|
 **/

G_DEFINE_BOXED_TYPE (PeasPluginInfo, peas_plugin_info,
                     _peas_plugin_info_ref,
                     _peas_plugin_info_unref)

PeasPluginInfo *
_peas_plugin_info_ref (PeasPluginInfo *info)
{
  g_atomic_int_inc (&info->refcount);
  return info;
}

void
_peas_plugin_info_unref (PeasPluginInfo *info)
{
  if (!g_atomic_int_dec_and_test (&info->refcount))
    return;

  g_free (info->module_dir);
  g_free (info->data_dir);
  if (info->schema_source != NULL)
    g_settings_schema_source_unref (info->schema_source);
  g_free (info->module_name);
  g_strfreev (info->dependencies);
  g_free (info->name);
  g_free (info->desc);
  g_free (info->icon_name);
  g_free (info->website);
  g_free (info->copyright);
  g_free (info->loader);
  g_free (info->version);
  g_free (info->help_uri);
  g_strfreev (info->authors);
  if (info->error != NULL)
    g_error_free (info->error);

  if (info->external_data != NULL)
    g_hash_table_unref (info->external_data);

  g_free (info);
}


GQuark
peas_plugin_info_error_quark (void)
{
  static volatile gsize quark = 0;

	if (g_once_init_enter (&quark))
		g_once_init_leave (&quark,
		                   g_quark_from_static_string ("peas-plugin-info-error"));

	return quark;
}

/*
 * _peas_plugin_info_new:
 * @filename: The filename where to read the plugin information.
 * @module_dir: The module directory.
 * @data_dir: The data directory.
 *
 * Creates a new #PeasPluginInfo from a file on the disk.
 *
 * Return value: a newly created #PeasPluginInfo.
 */
gboolean _peas_plugin_info_keyfile (PeasPluginInfo *info,
                                    const gchar    *filename,
                                    const gchar    *module_dir,
                                    const gchar    *data_dir,
                                    gpointer        user_data)
{
  GKeyFile *plugin_file = NULL;
  gchar *str;
  gchar **strv;
  gboolean b;
  GError *error = NULL;
  gchar **keys;
  gsize i;

  plugin_file = g_key_file_new ();
  if (!g_key_file_load_from_file (plugin_file, filename, G_KEY_FILE_NONE, NULL))
    {
      g_warning ("Bad plugin file: '%s'", filename);
      goto error;
    }

  /* Get module name */
  str = g_key_file_get_string (plugin_file, "Plugin", "Module", NULL);

  if ((str != NULL) && (*str != '\0'))
    {
      info->module_name = str;
    }
  else
    {
      g_warning ("Could not find 'Module' in '[Plugin]' section in '%s'", filename);
      goto error;
    }

  /* Get Name */
  str = g_key_file_get_locale_string (plugin_file, "Plugin",
                                      "Name", NULL, NULL);
  if (str)
    info->name = str;
  else
    {
      g_warning ("Could not find 'Name' in '[Plugin]' section in '%s'", filename);
      goto error;
    }

  /* Get the dependency list */
  info->dependencies = g_key_file_get_string_list (plugin_file,
                                                   "Plugin",
                                                   "Depends", NULL, NULL);
  if (info->dependencies == NULL)
    info->dependencies = g_new0 (gchar *, 1);

  /* Get the loader for this plugin */
  str = g_key_file_get_string (plugin_file, "Plugin", "Loader", NULL);

  if ((str != NULL) && (*str != '\0'))
    {
      info->loader = str;
    }
  else
    {
      /* default to the C loader */
      info->loader = g_strdup ("C");
    }

  /* Get Description */
  str = g_key_file_get_locale_string (plugin_file, "Plugin",
                                      "Description", NULL, NULL);
  if (str)
    info->desc = str;

  /* Get Icon */
  str = g_key_file_get_locale_string (plugin_file, "Plugin",
                                      "Icon", NULL, NULL);
  if (str)
    info->icon_name = str;

  /* Get Authors */
  info->authors = g_key_file_get_string_list (plugin_file, "Plugin",
                                              "Authors", NULL, NULL);
  if (info->authors == NULL)
    info->authors = g_new0 (gchar *, 1);

  /* Get Copyright */
  strv = g_key_file_get_string_list (plugin_file, "Plugin",
                                     "Copyright", NULL, NULL);
  if (strv)
    {
      info->copyright = g_strjoinv ("\n", strv);

      g_strfreev (strv);
    }

  /* Get Website */
  str = g_key_file_get_string (plugin_file, "Plugin", "Website", NULL);
  if (str)
    info->website = str;

  /* Get Version */
  str = g_key_file_get_string (plugin_file, "Plugin", "Version", NULL);
  if (str)
    info->version = str;

  /* Get Help URI */
  str = g_key_file_get_string (plugin_file, "Plugin", OS_HELP_KEY, NULL);
  if (str)
    info->help_uri = str;
  else
    {
      str = g_key_file_get_string (plugin_file, "Plugin", "Help", NULL);
      if (str)
        info->help_uri = str;
    }

  /* Get Builtin */
  b = g_key_file_get_boolean (plugin_file, "Plugin", "Builtin", &error);
  if (error != NULL)
    g_clear_error (&error);
  else
    info->builtin = b;

  /* Get Hidden */
  b = g_key_file_get_boolean (plugin_file, "Plugin", "Hidden", &error);
  if (error != NULL)
    g_clear_error (&error);
  else
    info->hidden = b;

  keys = g_key_file_get_keys (plugin_file, "Plugin", NULL, NULL);

  for (i = 0; keys[i] != NULL; ++i)
    {
      if (!g_str_has_prefix (keys[i], "X-"))
        continue;

      if (info->external_data == NULL)
        info->external_data = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                     (GDestroyNotify) g_free,
                                                     (GDestroyNotify) g_free);

      g_hash_table_insert (info->external_data,
                           g_strdup (keys[i] + 2),
                           g_key_file_get_string (plugin_file, "Plugin",
                                                  keys[i], NULL));
    }

  g_strfreev (keys);

  g_key_file_free (plugin_file);

  return TRUE;
  
error:
  g_free (info->module_name);
  g_free (info->name);
  g_key_file_free (plugin_file);

  return FALSE;
}

/*
 * _peas_plugin_info_new:
 * @filename: The filename where to read the plugin information.
 * @module_dir: The module directory.
 * @data_dir: The data directory.
 *
 * Creates a new #PeasPluginInfo from a file on the disk.
 *
 * Return value: a newly created #PeasPluginInfo.
 */
PeasPluginInfo *
_peas_plugin_info_new (const gchar *filename,
                       const gchar *module_dir,
                       const gchar *data_dir,
                       PeasPluginInfoProvider provider,
                       gpointer provider_data)
{
  PeasPluginInfo *info;

  info = g_new0 (PeasPluginInfo, 1);
  info->refcount = 1;

  g_return_val_if_fail (filename != NULL, NULL);

  if (!provider(info, filename, module_dir, data_dir, provider_data))
    {
      g_free(info);
      return NULL;
    }

  /* allow provder to override module and data dirs */
  if (!info->module_dir)
    info->module_dir = g_strdup (module_dir);
  if (!info->data_dir)
    info->data_dir = g_build_filename (data_dir, info->module_name, NULL);

  /* If we know nothing about the availability of the plugin,
     set it as available */
  info->available = TRUE;

  return info;
}

/**
 * peas_plugin_info_is_loaded:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is loaded.
 *
 * Returns: %TRUE if the plugin is loaded.
 */
gboolean
peas_plugin_info_is_loaded (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->available && info->loaded;
}

/**
 * peas_plugin_info_is_available:
 * @info: A #PeasPluginInfo.
 * @error: A #GError.
 *
 * Check if the plugin is available.
 *
 * A plugin is marked as not available when there is no loader available to
 * load it, or when there has been an error when trying to load it previously.
 * If not available then @error will be set.
 *
 * Returns: %TRUE if the plugin is available.
 */
gboolean
peas_plugin_info_is_available (const PeasPluginInfo  *info,
                               GError               **error)
{
  g_return_val_if_fail (info != NULL, FALSE);

  /* Uses g_propagate_error() so we get the right warning
   * in the case that *error != NULL
   */
  if (error != NULL && info->error != NULL)
    g_propagate_error (error, g_error_copy (info->error));

  return info->available != FALSE;
}

/**
 * peas_plugin_info_is_builtin:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is a builtin plugin.
 *
 * A builtin plugin is a plugin which cannot be enabled or disabled by the
 * user through a plugin manager (like #PeasGtkPluginManager). Loading or
 * unloading such plugins is the responsibility of the application alone.
 * Most applications will usually load those plugins immediately after
 * the initialization of the #PeasEngine.
 *
 * The relevant key in the plugin info file is "Builtin".
 *
 * Returns: %TRUE if the plugin is a builtin plugin, %FALSE
 * if not.
 **/
gboolean
peas_plugin_info_is_builtin (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, TRUE);

  return info->builtin;
}

/**
 * peas_plugin_info_is_hidden:
 * @info: A #PeasPluginInfo.
 *
 * Check if the plugin is a hidden plugin.
 *
 * A hidden plugin is a plugin which cannot be seen by a
 * user through a plugin manager (like #PeasGtkPluginManager). Loading and
 * unloading such plugins is the responsibility of the application alone or
 * through plugins that depend on them.
 *
 * The relevant key in the plugin info file is "Hidden".
 *
 * Returns: %TRUE if the plugin is a hidden plugin, %FALSE
 * if not.
 **/
gboolean
peas_plugin_info_is_hidden (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, FALSE);

  return info->hidden;
}

/**
 * peas_plugin_info_get_module_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the module name.
 *
 * The module name will be used to find the actual plugin. The way this value
 * will be used depends on the loader (i.e. on the language) of the plugin.
 * This value is also used to uniquely identify a particular plugin.
 *
 * The relevant key in the plugin info file is "Module".
 *
 * Returns: the module name.
 */
const gchar *
peas_plugin_info_get_module_name (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->module_name;
}

/**
 * peas_plugin_info_get_module_dir:
 * @info: A #PeasPluginInfo.
 *
 * Gets the module directory.
 *
 * The module directory is the directory where the plugin file was found. This
 * is not a value from the #GKeyFile, but rather a value provided by the
 * #PeasEngine.
 *
 * Returns: the module directory.
 */
const gchar *
peas_plugin_info_get_module_dir (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->module_dir;
}

/**
 * peas_plugin_info_get_data_dir:
 * @info: A #PeasPluginInfo.
 *
 * Gets the data dir of the plugin.
 *
 * The module data directory is the directory where a plugin should find its
 * runtime data. This is not a value read from the #GKeyFile, but rather a
 * value provided by the #PeasEngine, depending on where the plugin file was
 * found.
 *
 * Returns: the plugin's data dir.
 */
const gchar *
peas_plugin_info_get_data_dir (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->data_dir;
}

/**
 * peas_plugin_info_get_settings:
 * @info: A #PeasPluginInfo.
 * @schema_id: (allow-none): The schema id.
 *
 * Creates a new #GSettings for the given @schema_id and if
 * gschemas.compiled is not in the module directory an attempt
 * will be made to create it.
 *
 * Returns: (transfer full): a new #GSettings, or %NULL.
 */
GSettings *
peas_plugin_info_get_settings (const PeasPluginInfo *info,
                               const gchar          *schema_id)
{
  GSettingsSchema *schema;
  GSettings *settings;

  g_return_val_if_fail (info != NULL, NULL);

  if (info->schema_source == NULL)
    {
      GFile *module_dir_location;
      GFile *gschema_compiled;
      GSettingsSchemaSource *default_source;

      module_dir_location = g_file_new_for_path (info->module_dir);
      gschema_compiled = g_file_get_child (module_dir_location,
                                           "gschemas.compiled");

      if (!g_file_query_exists (gschema_compiled, NULL))
        {
          const gchar *argv[] = {
            "glib-compile-schemas",
            "--targetdir", info->module_dir,
            info->module_dir,
            NULL
          };

          g_spawn_sync (NULL, (gchar **) argv, NULL, G_SPAWN_SEARCH_PATH,
                        NULL, NULL, NULL, NULL, NULL, NULL);
        }

      g_object_unref (gschema_compiled);
      g_object_unref (module_dir_location);

      default_source = g_settings_schema_source_get_default ();
      ((PeasPluginInfo *) info)->schema_source =
            g_settings_schema_source_new_from_directory (info->module_dir,
                                                         default_source,
                                                         FALSE, NULL);

      /* glib-compile-schemas already outputted a message */
      if (info->schema_source == NULL)
        return NULL;
    }

  if (schema_id == NULL)
    schema_id = info->module_name;

  schema = g_settings_schema_source_lookup (info->schema_source, schema_id,
                                            FALSE);

  if (schema == NULL)
    return NULL;

  settings = g_settings_new_full (schema, NULL, NULL);

  g_settings_schema_unref (schema);

  return settings;
}

/**
 * peas_plugin_info_get_dependencies:
 * @info: A #PeasPluginInfo.
 *
 * Gets the dependencies of the plugin.
 *
 * The #PeasEngine will always ensure that the dependencies of a plugin are
 * loaded when the said plugin is loaded. It means that dependencies are
 * loaded before the plugin, and unloaded after it. Circular dependencies of
 * plugins lead to undefined loading order.
 *
 * The relevant key in the plugin info file is "Depends".
 *
 * Returns: (transfer none): the plugin's dependencies.
 */
const gchar **
peas_plugin_info_get_dependencies (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return (const gchar **) info->dependencies;
}

/**
 * peas_plugin_info_has_dependency:
 * @info: A #PeasPluginInfo.
 * @module_name: The name of the plugin to check.
 *
 * Check if the plugin depends on another plugin.
 *
 * Returns: whether the plugin depends on the plugin @module_name.
 */
gboolean
peas_plugin_info_has_dependency (const PeasPluginInfo *info,
                                 const gchar          *module_name)
{
  guint i;

  g_return_val_if_fail (info != NULL, FALSE);
  g_return_val_if_fail (module_name != NULL, FALSE);

  for (i = 0; info->dependencies[i] != NULL; i++)
    if (g_ascii_strcasecmp (module_name, info->dependencies[i]) == 0)
      return TRUE;

  return FALSE;
}


/**
 * peas_plugin_info_get_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the name of the plugin.
 *
 * The name of a plugin should be a nice short string to be presented in UIs.
 *
 * The relevant key in the plugin info file is "Name".
 *
 * Returns: the plugin's name.
 */
const gchar *
peas_plugin_info_get_name (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->name;
}

/**
 * peas_plugin_info_get_description:
 * @info: A #PeasPluginInfo.
 *
 * Gets the description of the plugin.
 *
 * The description of the plugin should be a string presenting the purpose of
 * the plugin. It will typically be presented in a plugin's about box.
 *
 * The relevant key in the plugin info file is "Description".
 *
 * Returns: the plugin's description.
 */
const gchar *
peas_plugin_info_get_description (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->desc;
}

/**
 * peas_plugin_info_get_icon_name:
 * @info: A #PeasPluginInfo.
 *
 * Gets the icon name of the plugin.
 *
 * The icon of the plugin will be presented in the plugin manager UI. If no
 * icon is specified, the default green puzzle icon will be used.
 *
 * The relevant key in the plugin info file is "Icon".
 *
 * Returns: the plugin's icon name.
 */
const gchar *
peas_plugin_info_get_icon_name (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  /* use the libpeas-plugin icon as a default if the plugin does not
     have its own */
  if (info->icon_name != NULL)
    return info->icon_name;
  else
    return "libpeas-plugin";
}

/**
 * peas_plugin_info_get_authors:
 * @info: A #PeasPluginInfo.
 *
 * Gets a %NULL-terminated array of strings with the authors of the plugin.
 *
 * The relevant key in the plugin info file is "Authors".
 *
 * Returns: (transfer none) (array zero-terminated=1): the plugin's author list.
 */
const gchar **
peas_plugin_info_get_authors (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, (const gchar **) NULL);

  return (const gchar **) info->authors;
}

/**
 * peas_plugin_info_get_website:
 * @info: A #PeasPluginInfo.
 *
 * Gets the website of the plugin.
 *
 * The relevant key in the plugin info file is "Website".
 *
 * Returns: the plugin's associated website.
 */
const gchar *
peas_plugin_info_get_website (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->website;
}

/**
 * peas_plugin_info_get_copyright:
 * @info: A #PeasPluginInfo.
 *
 * Gets the copyright of the plugin.
 *
 * The relevant key in the plugin info file is "Copyright".
 *
 * Returns: the plugin's copyright information.
 */
const gchar *
peas_plugin_info_get_copyright (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->copyright;
}

/**
 * peas_plugin_info_get_version:
 * @info: A #PeasPluginInfo.
 *
 * Gets the version of the plugin.
 *
 * The relevant key in the plugin info file is "Version".
 *
 * Returns: the plugin's version.
 */
const gchar *
peas_plugin_info_get_version (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->version;
}

/**
 * peas_plugin_info_get_help_uri:
 * @info: A #PeasPluginInfo.
 *
 * Gets the help URI of the plugin.
 *
 * The Help URI of a plugin will typically be presented by the plugin manager
 * as a "Help" button linking to the URI. It can either be a HTTP URL on some
 * website or a ghelp: URI if a Gnome help page is available for the plugin.
 *
 * The relevant key in the plugin info file is "Help". Other platform-specific
 * keys exist for platform-specific help files. Those are "Help-GNOME",
 * "Help-Windows" and "Help-MacOS-X".
 *
 * Returns: the plugin's help URI.
 */
const gchar *
peas_plugin_info_get_help_uri (const PeasPluginInfo *info)
{
  g_return_val_if_fail (info != NULL, NULL);

  return info->help_uri;
}

/**
 * peas_plugin_info_get_external_data:
 * @info: A #PeasPluginInfo.
 * @key: The key to lookup.
 *
 * Gets external data specified for the plugin.
 *
 * External data is specified in the plugin info file prefixed with X-. For
 * example, if a key/value pair X-Peas=1 is specified in the key file, you
 * can use "Peas" for @key to retrieve the value "1".
 *
 * Note: that you can omit the X- prefix when retrieving the value,
 * but not when specifying the value in the file.
 *
 * Returns: the external data, or %NULL if the external data could not be found.
 */
const gchar *
peas_plugin_info_get_external_data (const PeasPluginInfo *info,
                                    const gchar          *key)
{
  g_return_val_if_fail (info != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);

  if (info->external_data == NULL)
    return NULL;

  if (g_str_has_prefix (key, "X-"))
    key += 2;

  return g_hash_table_lookup (info->external_data, key);
}

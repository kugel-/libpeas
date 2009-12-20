/*
 * peas-dirs.c
 * This file is part of libpeas
 *
 * Copyright (C) 2008 Ignacio Casal Quinteiro
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef OS_OSX
#include <ige-mac-bundle.h>
#endif

#include "peas-dirs.h"

gchar *
peas_dirs_get_data_dir (void)
{
  gchar *data_dir;

#ifndef G_OS_WIN32
  data_dir = g_build_filename (DATADIR, "libpeas-2.0", NULL);
#elif defined (OS_OSX)
  IgeMacBundle *bundle = ige_mac_bundle_get_default ();

  if (ige_mac_bundle_get_is_app_bundle (bundle))
    {
      const gchar *bundle_data_dir = ige_mac_bundle_get_datadir (bundle);

      data_dir = g_build_filename (bundle_data_dir, "libpeas-2.0", NULL);
    }
  else
    {
      data_dir = g_build_filename (DATADIR, "libpeas-2.0", NULL);
    }
#else
  gchar *win32_dir;

  win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

  data_dir = g_build_filename (win32_dir, "share", "libpeas-2.0", NULL);
  g_free (win32_dir);
#endif

  return data_dir;
}

gchar *
peas_dirs_get_lib_dir (void)
{
  gchar *lib_dir;

#ifndef G_OS_WIN32
  lib_dir = g_build_filename (LIBDIR, "libpeas-2.0", NULL);
#elif defined (OS_OSX)
  IgeMacBundle *bundle = ige_mac_bundle_get_default ();

  if (ige_mac_bundle_get_is_app_bundle (bundle))
    {
      const gchar *path = ige_mac_bundle_get_resourcesdir (bundle);

      lib_dir = g_build_filename (path, "lib", "libpeas-2.0", NULL);
    }
  else
    {
      lib_dir = g_build_filename (LIBDIR, "libpeas-2.0", NULL);
    }
#else
  gchar *win32_dir;

  win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

  data_dir = g_build_filename (win32_dir, "lib", "libpeas-2.0", NULL);
  g_free (win32_dir);
#endif

  return lib_dir;
}

gchar *
peas_dirs_get_plugin_loaders_dir (void)
{
  gchar *lib_dir;
  gchar *loader_dir;

  lib_dir = peas_dirs_get_lib_dir ();
  loader_dir = g_build_filename (lib_dir, "loaders", NULL);

  g_free (lib_dir);

  return loader_dir;
}

gchar *
peas_dirs_get_locale_dir (void)
{
  gchar *locale_dir;

#ifdef G_OS_WIN32
  gchar *win32_dir;

  win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

  locale_dir = g_build_filename (win32_dir, "share", "locale", NULL);

  g_free (win32_dir);
#elif defined (OS_OSX)
  IgeMacBundle *bundle = ige_mac_bundle_get_default ();

  if (ige_mac_bundle_get_is_app_bundle (bundle))
    {
      locale_dir = g_strdup (ige_mac_bundle_get_localedir (bundle));
    }
  else
    {
      locale_dir = g_build_filename (DATADIR, "locale", NULL);
    }
#else
  locale_dir = g_build_filename (DATADIR, "locale", NULL);
#endif

  return locale_dir;
}


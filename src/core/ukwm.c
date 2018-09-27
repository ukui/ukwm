/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright 2011 Red Hat, Inc.
 * Copyright 2017 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdlib.h>

#include <meta/main.h>
#include <meta/util.h>
#include <glib/gi18n-lib.h>
#include "meta-plugin-manager.h"

#include <glib.h>
#include <X11/extensions/Xrandr.h>

static gboolean
print_version (const gchar    *option_name,
               const gchar    *value,
               gpointer        data,
               GError        **error)
{
  const int latest_year = 2011;

  g_print (_("ukwm %s\n"
             "Copyright © 2001-%d Havoc Pennington, Red Hat, Inc., and others\n"
             "This is free software; see the source for copying conditions.\n"
             "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"),
           VERSION, latest_year);
  exit (0);
}

static const char *plugin = "default";

GOptionEntry ukwm_options[] = {
  {
    "version", 0, G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
    print_version,
    N_("Print version"),
    NULL
  },
  {
    "ukwm-plugin", 0, 0, G_OPTION_ARG_STRING,
    &plugin,
    N_("Ukwm plugin to use"),
    "PLUGIN",
  },
  { NULL }
};

/* This is used for detect whether there is monitor connected.
 * if no monitor connected, and User autologin was set,
 * Ukwm won't work
 */

static int
detect_monitor_connect(void)
{
  int event_base, error_base;
  int major, minor;
  Display      *dpy;
  Window       root;
  int   screen = -1;

  dpy = XOpenDisplay (NULL);
  if(!dpy)
    exit(1);
  screen = DefaultScreen (dpy);
  if (screen >= ScreenCount (dpy))
    exit(1);
  root = RootWindow (dpy, screen);

  if (!XRRQueryExtension (dpy, &event_base, &error_base) ||
        !XRRQueryVersion (dpy, &major, &minor))
    {
      g_print("RandR extension missing\n");
      exit(1);
    }

  if(major > 1|| (major == 1 && minor >= 5))
    {
      XRRMonitorInfo *m;
      int             n;
      m = XRRGetMonitors(dpy, root, 1, &n);
      XCloseDisplay(dpy);
      return n;
    }
  else
    {
      g_print("RandR extension version is lower than 1.5\n");
      exit(1);
    }
}


int
main (int argc, char **argv)
{
  GOptionContext *ctx;
  GError *error = NULL;

  ctx = meta_get_option_context ();
  g_option_context_add_main_entries (ctx, ukwm_options, GETTEXT_PACKAGE);
  if (!g_option_context_parse (ctx, &argc, &argv, &error))
    {
      g_printerr ("ukwm: %s\n", error->message);
      exit (1);
    }
  g_option_context_free (ctx);

/*
 * Wait until monitor is connected
 */

  while(1)
    {
      if(detect_monitor_connect())
        break;
      sleep(5);
    }

  if (plugin)
    meta_plugin_manager_load (plugin);

  meta_init ();
  meta_register_with_session ();
  return meta_run ();
}

/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (c) 2008 Intel Corp.
 *
 * Author: Tomas Frydrych <tf@linux.intel.com>
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

#include <config.h>

#include <meta/meta-plugin.h>
#include <meta/window.h>
#include <meta/meta-background-group.h>
#include <meta/meta-background-actor.h>
#include <meta/util.h>
#include <glib/gi18n-lib.h>

#include <clutter/clutter.h>
#include <gmodule.h>
#include <string.h>

#include <meta/screen.h>
#include <meta/display.h>
#include <meta/meta-backend.h>
#include <core/window-private.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <backends/x11/meta-backend-x11.h>
#include <glib-2.0/glib.h>
#include <gdk/gdkpixbuf.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo.h>
#include <sys/time.h>
#include <sys/file.h>
#include <pthread.h>
#include <unistd.h>

#include <ukui_plugin.h>
#include <ukui_plugin_generated.h>

#define DESTROY_TIMEOUT   100
#define MINIMIZE_TIMEOUT  250
#define MAP_TIMEOUT       250
#define SWITCH_TIMEOUT    500

static UkwmPlugin *pSkeleton = NULL;
MetaPlugin * global_plugin;
MetaScreen * global_screen;
MetaDisplay * global_display;
GList * global_tab_list = NULL;
alt_tab_item ati_list[ATP_COUNT];

#define ACTOR_DATA_KEY "MCCP-Default-actor-data"
#define SCREEN_TILE_PREVIEW_DATA_KEY "MCCP-Default-screen-tile-preview-data"

#define META_TYPE_DEFAULT_PLUGIN            (meta_default_plugin_get_type ())
#define META_DEFAULT_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), META_TYPE_DEFAULT_PLUGIN, MetaDefaultPlugin))
#define META_DEFAULT_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  META_TYPE_DEFAULT_PLUGIN, MetaDefaultPluginClass))
#define META_IS_DEFAULT_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), META_DEFAULT_PLUGIN_TYPE))
#define META_IS_DEFAULT_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  META_TYPE_DEFAULT_PLUGIN))
#define META_DEFAULT_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  META_TYPE_DEFAULT_PLUGIN, MetaDefaultPluginClass))

#define META_DEFAULT_PLUGIN_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), META_TYPE_DEFAULT_PLUGIN, MetaDefaultPluginPrivate))

typedef struct _MetaDefaultPlugin        MetaDefaultPlugin;
typedef struct _MetaDefaultPluginClass   MetaDefaultPluginClass;
typedef struct _MetaDefaultPluginPrivate MetaDefaultPluginPrivate;

static int uid;
static char pid_file[PATH_MAX_LEN] = {0};
static char tab_list_image_file[PATH_MAX_LEN] = {0};
//static char workspace_image_file[PATH_MAX_LEN] = {0};

struct _MetaDefaultPlugin
{
  MetaPlugin parent;

  MetaDefaultPluginPrivate *priv;
};

struct _MetaDefaultPluginClass
{
  MetaPluginClass parent_class;
};

static GQuark actor_data_quark = 0;
static GQuark screen_tile_preview_data_quark = 0;

static void start      (MetaPlugin      *plugin);
static void minimize   (MetaPlugin      *plugin,
                        MetaWindowActor *actor);
static void map        (MetaPlugin      *plugin,
                        MetaWindowActor *actor);
static void destroy    (MetaPlugin      *plugin,
                        MetaWindowActor *actor);

static void switch_workspace (MetaPlugin          *plugin,
                              gint                 from,
                              gint                 to,
                              MetaMotionDirection  direction);

static void kill_window_effects   (MetaPlugin      *plugin,
                                   MetaWindowActor *actor);
static void kill_switch_workspace (MetaPlugin      *plugin);

static void show_tile_preview (MetaPlugin      *plugin,
                               MetaWindow      *window,
                               MetaRectangle   *tile_rect,
                               int              tile_monitor_number);
static void hide_tile_preview (MetaPlugin      *plugin);

static void confirm_display_change (MetaPlugin *plugin);

static const MetaPluginInfo * plugin_info (MetaPlugin *plugin);

META_PLUGIN_DECLARE(MetaDefaultPlugin, meta_default_plugin);

/*
 * Plugin private data that we store in the .plugin_private member.
 */
struct _MetaDefaultPluginPrivate
{
  /* Valid only when switch_workspace effect is in progress */
  ClutterTimeline       *tml_switch_workspace1;
  ClutterTimeline       *tml_switch_workspace2;
  ClutterActor          *desktop1;
  ClutterActor          *desktop2;

  ClutterActor          *background_group;

  MetaPluginInfo         info;
};

/*
 * Per actor private data we attach to each actor.
 */
typedef struct _ActorPrivate
{
  ClutterActor *orig_parent;

  ClutterTimeline *tml_minimize;
  ClutterTimeline *tml_destroy;
  ClutterTimeline *tml_map;
} ActorPrivate;

/* callback data for when animations complete */
typedef struct
{
  ClutterActor *actor;
  MetaPlugin *plugin;
} EffectCompleteData;


typedef struct _ScreenTilePreview
{
  ClutterActor   *actor;

  GdkRGBA        *preview_color;

  MetaRectangle   tile_rect;
} ScreenTilePreview;

static void
meta_default_plugin_dispose (GObject *object)
{
  /* MetaDefaultPluginPrivate *priv = META_DEFAULT_PLUGIN (object)->priv;
  */
  G_OBJECT_CLASS (meta_default_plugin_parent_class)->dispose (object);
}

static void
meta_default_plugin_finalize (GObject *object)
{
  G_OBJECT_CLASS (meta_default_plugin_parent_class)->finalize (object);
}

static void
meta_default_plugin_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
meta_default_plugin_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
meta_default_plugin_class_init (MetaDefaultPluginClass *klass)
{
  GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
  MetaPluginClass *plugin_class  = META_PLUGIN_CLASS (klass);

  gobject_class->finalize        = meta_default_plugin_finalize;
  gobject_class->dispose         = meta_default_plugin_dispose;
  gobject_class->set_property    = meta_default_plugin_set_property;
  gobject_class->get_property    = meta_default_plugin_get_property;

  plugin_class->start            = start;
  plugin_class->map              = map;
  plugin_class->minimize         = minimize;
  plugin_class->destroy          = destroy;
  plugin_class->switch_workspace = NULL;
  plugin_class->show_tile_preview = show_tile_preview;
  plugin_class->hide_tile_preview = hide_tile_preview;
  plugin_class->plugin_info      = plugin_info;
  plugin_class->kill_window_effects   = kill_window_effects;
  plugin_class->kill_switch_workspace = NULL;
  plugin_class->confirm_display_change = confirm_display_change;

  g_type_class_add_private (gobject_class, sizeof (MetaDefaultPluginPrivate));
}

static void
meta_default_plugin_init (MetaDefaultPlugin *self)
{
  MetaDefaultPluginPrivate *priv;

  self->priv = priv = META_DEFAULT_PLUGIN_GET_PRIVATE (self);

  priv->info.name        = "Default Effects";
  priv->info.version     = "0.1";
  priv->info.author      = "Intel Corp.";
  priv->info.license     = "GPL";
  priv->info.description = "This is an example of a plugin implementation.";
}

/*
 * Actor private data accessor
 */
static void
free_actor_private (gpointer data)
{
  if (G_LIKELY (data != NULL))
    g_slice_free (ActorPrivate, data);
}

static ActorPrivate *
get_actor_private (MetaWindowActor *actor)
{
  ActorPrivate *priv = g_object_get_qdata (G_OBJECT (actor), actor_data_quark);

  if (G_UNLIKELY (actor_data_quark == 0))
    actor_data_quark = g_quark_from_static_string (ACTOR_DATA_KEY);

  if (G_UNLIKELY (!priv))
    {
      priv = g_slice_new0 (ActorPrivate);

      g_object_set_qdata_full (G_OBJECT (actor),
                               actor_data_quark, priv,
                               free_actor_private);
    }

  return priv;
}

static ClutterTimeline *
actor_animate (ClutterActor         *actor,
               ClutterAnimationMode  mode,
               guint                 duration,
               const gchar          *first_property,
               ...)
{
  va_list args;
  ClutterTransition *transition;

  clutter_actor_save_easing_state (actor);
  clutter_actor_set_easing_mode (actor, mode);
  clutter_actor_set_easing_duration (actor, duration);

  va_start (args, first_property);
  g_object_set_valist (G_OBJECT (actor), first_property, args);
  va_end (args);

  transition = clutter_actor_get_transition (actor, first_property);

  clutter_actor_restore_easing_state (actor);

  return CLUTTER_TIMELINE (transition);
}

static void
on_switch_workspace_effect_complete (ClutterTimeline *timeline, gpointer data)
{
  MetaPlugin               *plugin  = META_PLUGIN (data);
  MetaDefaultPluginPrivate *priv = META_DEFAULT_PLUGIN (plugin)->priv;
  MetaScreen *screen = meta_plugin_get_screen (plugin);
  GList *l = meta_get_window_actors (screen);

  while (l)
    {
      ClutterActor *a = l->data;
      MetaWindowActor *window_actor = META_WINDOW_ACTOR (a);
      ActorPrivate *apriv = get_actor_private (window_actor);

      if (apriv->orig_parent)
        {
          g_object_ref (a);
          clutter_actor_remove_child (clutter_actor_get_parent (a), a);
          clutter_actor_add_child (apriv->orig_parent, a);
          g_object_unref (a);
          apriv->orig_parent = NULL;
        }

      l = l->next;
    }

  clutter_actor_destroy (priv->desktop1);
  clutter_actor_destroy (priv->desktop2);

  priv->tml_switch_workspace1 = NULL;
  priv->tml_switch_workspace2 = NULL;
  priv->desktop1 = NULL;
  priv->desktop2 = NULL;

  meta_plugin_switch_workspace_completed (plugin);
}

static void
on_monitors_changed (MetaScreen *screen,
                     MetaPlugin *plugin)
{
  MetaDefaultPlugin *self = META_DEFAULT_PLUGIN (plugin);
  int i, n;
  GRand *rand = g_rand_new_with_seed (123456);

  clutter_actor_destroy_all_children (self->priv->background_group);

  n = meta_screen_get_n_monitors (screen);
  for (i = 0; i < n; i++)
    {
      MetaRectangle rect;
      ClutterActor *background_actor;
      MetaBackground *background;
      ClutterColor color;

      meta_screen_get_monitor_geometry (screen, i, &rect);

      background_actor = meta_background_actor_new (screen, i);

      clutter_actor_set_position (background_actor, rect.x, rect.y);
      clutter_actor_set_size (background_actor, rect.width, rect.height);

      /* UKUI: set init color black as default*/
      clutter_color_init (&color, 0, 0, 0, 255);

      background = meta_background_new (screen);
      meta_background_set_color (background, &color);
      meta_background_actor_set_background (META_BACKGROUND_ACTOR (background_actor), background);
      g_object_unref (background);

      meta_background_actor_set_vignette (META_BACKGROUND_ACTOR (background_actor),
                                          TRUE,
                                          0.5,
                                          0.5);

      clutter_actor_add_child (self->priv->background_group, background_actor);
    }

  g_rand_free (rand);
}

static gboolean ukwm_plugin_get_alt_tab_list(MetaPlugin *object,
                                             GDBusMethodInvocation *invocation)
{
  int count = 0;

  MetaScreen * screen;
  MetaDisplay * display;
  MetaTabList type = META_TAB_LIST_NORMAL;

  GList * l = NULL;

  GVariant * gva;
  GVariantBuilder* _builder = g_variant_builder_new(G_VARIANT_TYPE("av"));

  screen = meta_plugin_get_screen(global_plugin);
  display = meta_screen_get_display(screen);

  if (global_tab_list != NULL)
    g_list_free(global_tab_list);

  global_tab_list = meta_display_get_tab_list (display, type, NULL);
  l = global_tab_list;

  int offset_x = 0;
  int offset_y = 0;

  while (l != NULL)
    {
      if (count >= ATP_COUNT)
        break;

      MetaWindow * win = l->data;

      int icon_width = cairo_image_surface_get_width(win->icon);
      int icon_height = cairo_image_surface_get_height(win->icon);

      ati_list[count].title_name = win->title;
      ati_list[count].xid = (unsigned long)win->xwindow;
      ati_list[count].width = icon_width;
      ati_list[count].height = icon_height;
      ati_list[count].x = offset_x;
      ati_list[count].y = offset_y;
      ati_list[count].icon = (void *)gdk_pixbuf_get_from_surface(win->icon, 0, 0,
                                                                 icon_width,
                                                                 icon_height);
      offset_x += ati_list[count].width;

      GVariant * _item = g_variant_new("(siiiii)",
                                       ati_list[count].title_name,
                                       ati_list[count].xid,
                                       ati_list[count].width,
                                       ati_list[count].height,
                                       ati_list[count].x,
                                       ati_list[count].y);

      g_variant_builder_add(_builder, "v", _item);
      l = l->next;
      count += 1;
    }
  gva = g_variant_builder_end(_builder);
  g_variant_builder_unref(_builder);

  int i = 0;
  int max_width = 0;
  int max_height = 0;
  for (i = 0; i < count; i++)
    {
      if (ati_list[i].x + ati_list[i].width > max_width)
        max_width = ati_list[i].x + ati_list[i].width;
      if (ati_list[i].y + ati_list[i].height > max_height)
        max_height = ati_list[i].y + ati_list[i].height;
    }

  GdkPixbuf * alt_tab_list_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8,
                                                   max_width, max_height);
  memset(gdk_pixbuf_get_pixels(alt_tab_list_pixbuf), 0, max_width * max_height * 4);
  for (i = 0; i < count; i++)
    {
      gdk_pixbuf_copy_area((GdkPixbuf *)ati_list[i].icon,
                           0, 0,
                           ati_list[i].width, ati_list[i].height,
                           alt_tab_list_pixbuf,
                           ati_list[i].x, ati_list[i].y);
      g_object_unref((GdkPixbuf *)ati_list[i].icon);
    }

  gdk_pixbuf_save(alt_tab_list_pixbuf, tab_list_image_file,
                  "png", NULL, NULL);
  chmod(tab_list_image_file, 0660);

  g_object_unref(alt_tab_list_pixbuf);

  ukwm_plugin_complete_get_alt_tab_list(object, invocation, count, gva);

  return true;
}

static gboolean ukwm_plugin_activate_window_by_tab_list_index(MetaPlugin *object,
                                                              GDBusMethodInvocation *invocation,
                                                              int index)
{
  MetaScreen * screen;
  MetaDisplay * display;
  MetaWindow * window;
  MetaTabList type = META_TAB_LIST_NORMAL;

  screen = meta_plugin_get_screen(global_plugin);
  display = meta_screen_get_display(screen);

  if (global_tab_list == NULL)
    {
      ukwm_plugin_complete_activate_window_by_tab_list_index(object, invocation);
      return true;
    }
  index = index % g_list_length(global_tab_list);
  window = g_list_nth_data (global_tab_list, index);

  GList * now_tab_list = meta_display_get_tab_list (display, type, NULL);
  GList * window_exist = g_list_find(now_tab_list, window);
  if (window_exist == NULL)
    {
      ukwm_plugin_complete_activate_window_by_tab_list_index(object, invocation);
      return true;
    }

  struct timeval tv1, tv2;
  //  struct sysinfo info;
  guint32 timestamp = 0;

  /*
  if (sysinfo(&info))
  {
        fprintf(stderr, "Failed to get sysinfo, errno:%u, reason:%s\n",
                                                                                  errno, strerror(errno));
                                                                                  timestamp = 0;
  }
  else
        timestamp = info.uptime * 1000;
*/

  if (window)
    meta_window_activate (window, timestamp);

  g_list_free(now_tab_list);
  g_list_free(global_tab_list);
  global_tab_list = NULL;

  ukwm_plugin_complete_activate_window_by_tab_list_index(object, invocation);
  return true;
}

static void bus_acquired_cb(GDBusConnection *connection,
                            const gchar *bus_name,
                            gpointer user_data)
{
  GError *pError = NULL;

  /** Second step: Try to get a connection to the given bus. */
  pSkeleton = ukwm_plugin_skeleton_new();

  /** Third step: Attach to dbus signals. */
  g_signal_connect(pSkeleton, "handle-get-alt-tab-list",
                   G_CALLBACK(ukwm_plugin_get_alt_tab_list), NULL);
  g_signal_connect(pSkeleton, "handle-activate-window-by-tab-list-index",
                   G_CALLBACK(ukwm_plugin_activate_window_by_tab_list_index), NULL);

  /** Fourth step: Export interface skeleton. */
  (void) g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(pSkeleton),
                                          connection,
                                          UKUI_PLUGIN_OBJECT_PATH,
                                          &pError);
  if(pError != NULL)
    {
      g_print("Error: Failed to export object. Reason: %s.\n", pError->message);
      g_error_free(pError);
    }

}

static void name_acquired_cb(GDBusConnection *connection,
                             const gchar *bus_name,
                             gpointer user_data)
{
  g_print("name_acquired_cb call, Acquired bus name: %s.\n", UKUI_PLUGIN_BUS_NAME);
}

static void name_lost_cb(GDBusConnection *connection,
                         const gchar *bus_name,
                         gpointer user_data)
{
  if(connection == NULL)
    {
      g_print("name_lost_cb call, Error: Failed to connect to dbus.\n");
    }
  else
    {
      g_print("name_lost_cb call, Error: Failed to obtain bus name: %s.\n", UKUI_PLUGIN_BUS_NAME);
    }
}

bool InitUkwmPluginDBusCommServer(void)
{
  bool bRet = TRUE;

  g_print("InitDBusCommunicationServer: Server started.\n");

  /** first step: connect to dbus */
  (void)g_bus_own_name(UKUI_PLUGIN_BUS,
                       UKUI_PLUGIN_BUS_NAME,
                       G_BUS_NAME_OWNER_FLAGS_REPLACE|G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT,
                       &bus_acquired_cb,
                       &name_acquired_cb,
                       &name_lost_cb,
                       NULL,
                       NULL);
  return bRet;
}

void ukui_window_switch_monitor(void)
{
  sleep(5);

  int lock_ret = -1;
  int pid_file_fd = open(pid_file, O_CREAT | O_TRUNC | O_RDWR, 0666);
  if (pid_file_fd < 0)
    {
      fprintf(stderr, "Can not open pid file[%s], %s\n",
              pid_file, strerror(pid_file_fd));
      return ;
    }

  while (1)
    {
      sleep(2);

      int pid_file_fd = open(pid_file, O_CREAT | O_TRUNC | O_RDWR, 0666);
      if (pid_file_fd < 0)
        {
          fprintf(stderr, "Can not open pid file[%s], %s\n",
                  pid_file, strerror(pid_file_fd));
          break;
        }
      else
        {
          int flags = fcntl(pid_file_fd, F_GETFD);
          fcntl(pid_file_fd, F_SETFD, flags | FD_CLOEXEC );
          lock_ret = flock(pid_file_fd, LOCK_EX | LOCK_NB);
          if (lock_ret == 0)
            {
              printf("ukui-window-switch is not running...\n");
              flock(pid_file_fd, LOCK_UN);
              pid_t kws_pid;
              kws_pid = fork();
              if (kws_pid == 0)
                {
                  char bin_file[PATH_MAX_LEN] = {0};
                  snprintf(bin_file, PATH_MAX_LEN, "/usr/bin/%s", PROGRAM_NAME);
                  if (access(bin_file, F_OK | R_OK | X_OK) == 0)
                    {
                      int err;
                      err = execlp(PROGRAM_NAME, PROGRAM_NAME, NULL);
                      fprintf(stderr, "Can not exec %s: %s\n",
                              PROGRAM_NAME, strerror(err));
                    }
                  exit(0);
                }
	      waitpid(-1, NULL, WNOHANG);
            }
          close(pid_file_fd);
        }
    }
}

static void
start (MetaPlugin *plugin)
{
  MetaDefaultPlugin *self = META_DEFAULT_PLUGIN (plugin);
  MetaScreen *screen = meta_plugin_get_screen (plugin);

  self->priv->background_group = meta_background_group_new ();
  clutter_actor_insert_child_below (meta_get_window_group_for_screen (screen),
                                    self->priv->background_group, NULL);

  g_signal_connect (screen, "monitors-changed",
                    G_CALLBACK (on_monitors_changed), plugin);
  on_monitors_changed (screen, plugin);

  clutter_actor_show (meta_get_stage_for_screen (screen));

  uid = getuid();
  snprintf(pid_file, PATH_MAX_LEN, "/run/user/%d/%s.pid", uid, PROGRAM_NAME);
  snprintf(tab_list_image_file, PATH_MAX_LEN, "/run/user/%d/%s",
           uid, TAB_LIST_IMAGE_FILE);

  global_plugin = plugin;
  InitUkwmPluginDBusCommServer();

  int err;
  pthread_t monitor_thread;

  err = pthread_create(&monitor_thread, NULL, ukui_window_switch_monitor, NULL);
  if (err != 0)
    fprintf(stderr, "Can't create ukui-window-switch monitor: %s\n",
            strerror(err));
}

static void
switch_workspace (MetaPlugin *plugin,
                  gint from, gint to,
                  MetaMotionDirection direction)
{
  MetaScreen *screen;
  MetaDefaultPluginPrivate *priv = META_DEFAULT_PLUGIN (plugin)->priv;
  GList        *l;
  ClutterActor *workspace0  = clutter_actor_new ();
  ClutterActor *workspace1  = clutter_actor_new ();
  ClutterActor *stage;
  int           screen_width, screen_height;

  screen = meta_plugin_get_screen (plugin);
  stage = meta_get_stage_for_screen (screen);

  meta_screen_get_size (screen,
                        &screen_width,
                        &screen_height);

  clutter_actor_set_pivot_point (workspace1, 1.0, 1.0);
  clutter_actor_set_position (workspace1,
                              screen_width,
                              screen_height);

  clutter_actor_set_scale (workspace1, 0.0, 0.0);

  clutter_actor_add_child (stage, workspace1);
  clutter_actor_add_child (stage, workspace0);

  if (from == to)
    {
      meta_plugin_switch_workspace_completed (plugin);
      return;
    }

  l = g_list_last (meta_get_window_actors (screen));

  while (l)
    {
      MetaWindowActor *window_actor = l->data;
      ActorPrivate    *apriv	    = get_actor_private (window_actor);
      ClutterActor    *actor	    = CLUTTER_ACTOR (window_actor);
      MetaWorkspace   *workspace;
      gint             win_workspace;

      workspace = meta_window_get_workspace (meta_window_actor_get_meta_window (window_actor));
      win_workspace = meta_workspace_index (workspace);

      if (win_workspace == to || win_workspace == from)
        {
          ClutterActor *parent = win_workspace == to ? workspace1 : workspace0;
          apriv->orig_parent = clutter_actor_get_parent (actor);

          g_object_ref (actor);
          clutter_actor_remove_child (clutter_actor_get_parent (actor), actor);
          clutter_actor_add_child (parent, actor);
          clutter_actor_show (actor);
          clutter_actor_set_child_below_sibling (parent, actor, NULL);
          g_object_unref (actor);
        }
      else if (win_workspace < 0)
        {
          /* Sticky window */
          apriv->orig_parent = NULL;
        }
      else
        {
          /* Window on some other desktop */
          clutter_actor_hide (actor);
          apriv->orig_parent = NULL;
        }

      l = l->prev;
    }

  priv->desktop1 = workspace0;
  priv->desktop2 = workspace1;

  priv->tml_switch_workspace1 = actor_animate (workspace0, CLUTTER_EASE_IN_SINE,
                                               SWITCH_TIMEOUT,
                                               "scale-x", 1.0,
                                               "scale-y", 1.0,
                                               NULL);
  g_signal_connect (priv->tml_switch_workspace1,
                    "completed",
                    G_CALLBACK (on_switch_workspace_effect_complete),
                    plugin);

  priv->tml_switch_workspace2 = actor_animate (workspace1, CLUTTER_EASE_IN_SINE,
                                               SWITCH_TIMEOUT,
                                               "scale-x", 0.0,
                                               "scale-y", 0.0,
                                               NULL);
}


/*
 * Minimize effect completion callback; this function restores actor state, and
 * calls the manager callback function.
 */
static void
on_minimize_effect_complete (ClutterTimeline *timeline, EffectCompleteData *data)
{
  /*
   * Must reverse the effect of the effect; must hide it first to ensure
   * that the restoration will not be visible.
   */
  MetaPlugin *plugin = data->plugin;
  ActorPrivate *apriv;
  MetaWindowActor *window_actor = META_WINDOW_ACTOR (data->actor);

  apriv = get_actor_private (META_WINDOW_ACTOR (data->actor));
  apriv->tml_minimize = NULL;

  clutter_actor_hide (data->actor);

  /* FIXME - we shouldn't assume the original scale, it should be saved
   * at the start of the effect */
  clutter_actor_set_scale (data->actor, 1.0, 1.0);

  /* Now notify the manager that we are done with this effect */
  meta_plugin_minimize_completed (plugin, window_actor);

  g_free (data);
}

/*
 * Simple minimize handler: it applies a scale effect (which must be reversed on
 * completion).
 */
static void
minimize (MetaPlugin *plugin, MetaWindowActor *window_actor)
{
  MetaWindowType type;
  MetaRectangle icon_geometry;
  MetaWindow *meta_window = meta_window_actor_get_meta_window (window_actor);
  ClutterActor *actor  = CLUTTER_ACTOR (window_actor);


  type = meta_window_get_window_type (meta_window);

  if (!meta_window_get_icon_geometry(meta_window, &icon_geometry))
    {
      icon_geometry.x = 0;
      icon_geometry.y = 0;
    }

  if (type == META_WINDOW_NORMAL)
    {
      EffectCompleteData *data = g_new0 (EffectCompleteData, 1);
      ActorPrivate *apriv = get_actor_private (window_actor);

      apriv->tml_minimize = actor_animate (actor,
                                           CLUTTER_EASE_IN_SINE,
                                           MINIMIZE_TIMEOUT,
                                           "scale-x", 0.0,
                                           "scale-y", 0.0,
                                           "x", (double)icon_geometry.x,
                                           "y", (double)icon_geometry.y,
                                           NULL);
      data->plugin = plugin;
      data->actor = actor;
      g_signal_connect (apriv->tml_minimize, "completed",
                        G_CALLBACK (on_minimize_effect_complete),
                        data);

    }
  else
    meta_plugin_minimize_completed (plugin, window_actor);
}

static void
on_map_effect_complete (ClutterTimeline *timeline, EffectCompleteData *data)
{
  /*
   * Must reverse the effect of the effect.
   */
  MetaPlugin *plugin = data->plugin;
  MetaWindowActor  *window_actor = META_WINDOW_ACTOR (data->actor);
  ActorPrivate  *apriv = get_actor_private (window_actor);

  apriv->tml_map = NULL;

  /* Now notify the manager that we are done with this effect */
  meta_plugin_map_completed (plugin, window_actor);

  g_free (data);
}

/*
 * Simple map handler: it applies a scale effect which must be reversed on
 * completion).
 */
static void
map (MetaPlugin *plugin, MetaWindowActor *window_actor)
{
  MetaWindowType type;
  ClutterActor *actor = CLUTTER_ACTOR (window_actor);
  MetaWindow *meta_window = meta_window_actor_get_meta_window (window_actor);

  type = meta_window_get_window_type (meta_window);

  if (type == META_WINDOW_NORMAL)
    {
      EffectCompleteData *data = g_new0 (EffectCompleteData, 1);
      ActorPrivate *apriv = get_actor_private (window_actor);

      clutter_actor_set_pivot_point (actor, 0.5, 0.5);
      clutter_actor_set_opacity (actor, 0);
      clutter_actor_set_scale (actor, 0.5, 0.5);
      clutter_actor_show (actor);

      apriv->tml_map = actor_animate (actor,
                                      CLUTTER_EASE_OUT_QUAD,
                                      MAP_TIMEOUT,
                                      "opacity", 255,
                                      "scale-x", 1.0,
                                      "scale-y", 1.0,
                                      NULL);
      data->actor = actor;
      data->plugin = plugin;
      g_signal_connect (apriv->tml_map, "completed",
                        G_CALLBACK (on_map_effect_complete),
                        data);
    }
  else
    meta_plugin_map_completed (plugin, window_actor);
}

/*
 * Destroy effect completion callback; this is a simple effect that requires no
 * further action than notifying the manager that the effect is completed.
 */
static void
on_destroy_effect_complete (ClutterTimeline *timeline, EffectCompleteData *data)
{
  MetaPlugin *plugin = data->plugin;
  MetaWindowActor *window_actor = META_WINDOW_ACTOR (data->actor);
  ActorPrivate *apriv = get_actor_private (window_actor);

  apriv->tml_destroy = NULL;

  meta_plugin_destroy_completed (plugin, window_actor);
}

/*
 * Simple TV-out like effect.
 */
static void
destroy (MetaPlugin *plugin, MetaWindowActor *window_actor)
{
  MetaWindowType type;
  ClutterActor *actor = CLUTTER_ACTOR (window_actor);
  MetaWindow *meta_window = meta_window_actor_get_meta_window (window_actor);

  type = meta_window_get_window_type (meta_window);

  if (type == META_WINDOW_NORMAL)
    {
      EffectCompleteData *data = g_new0 (EffectCompleteData, 1);
      ActorPrivate *apriv = get_actor_private (window_actor);

      apriv->tml_destroy = actor_animate (actor,
                                          CLUTTER_EASE_OUT_QUAD,
                                          DESTROY_TIMEOUT,
                                          "opacity", 0,
                                          "scale-x", 0.8,
                                          "scale-y", 0.8,
                                          NULL);
      data->plugin = plugin;
      data->actor = actor;
      g_signal_connect (apriv->tml_destroy, "completed",
                        G_CALLBACK (on_destroy_effect_complete),
                        data);
    }
  else
    meta_plugin_destroy_completed (plugin, window_actor);
}

/*
 * Tile preview private data accessor
 */
static void
free_screen_tile_preview (gpointer data)
{
  ScreenTilePreview *preview = data;

  if (G_LIKELY (preview != NULL)) {
      clutter_actor_destroy (preview->actor);
      g_slice_free (ScreenTilePreview, preview);
    }
}

static ScreenTilePreview *
get_screen_tile_preview (MetaScreen *screen)
{
  ScreenTilePreview *preview = g_object_get_qdata (G_OBJECT (screen), screen_tile_preview_data_quark);

  if (G_UNLIKELY (screen_tile_preview_data_quark == 0))
    screen_tile_preview_data_quark = g_quark_from_static_string (SCREEN_TILE_PREVIEW_DATA_KEY);

  if (G_UNLIKELY (!preview))
    {
      preview = g_slice_new0 (ScreenTilePreview);

      preview->actor = clutter_actor_new ();
      clutter_actor_set_background_color (preview->actor, CLUTTER_COLOR_Blue);
      clutter_actor_set_opacity (preview->actor, 100);

      clutter_actor_add_child (meta_get_window_group_for_screen (screen), preview->actor);
      g_object_set_qdata_full (G_OBJECT (screen),
                               screen_tile_preview_data_quark, preview,
                               free_screen_tile_preview);
    }

  return preview;
}

static void
show_tile_preview (MetaPlugin    *plugin,
                   MetaWindow    *window,
                   MetaRectangle *tile_rect,
                   int            tile_monitor_number)
{
  MetaScreen *screen = meta_plugin_get_screen (plugin);
  ScreenTilePreview *preview = get_screen_tile_preview (screen);
  ClutterActor *window_actor;

  if (clutter_actor_is_visible (preview->actor)
      && preview->tile_rect.x == tile_rect->x
      && preview->tile_rect.y == tile_rect->y
      && preview->tile_rect.width == tile_rect->width
      && preview->tile_rect.height == tile_rect->height)
    return; /* nothing to do */

  clutter_actor_set_position (preview->actor, tile_rect->x, tile_rect->y);
  clutter_actor_set_size (preview->actor, tile_rect->width, tile_rect->height);

  clutter_actor_show (preview->actor);

  window_actor = CLUTTER_ACTOR (meta_window_get_compositor_private (window));
  clutter_actor_set_child_below_sibling (clutter_actor_get_parent (preview->actor),
                                         preview->actor,
                                         window_actor);

  preview->tile_rect = *tile_rect;
}

static void
hide_tile_preview (MetaPlugin *plugin)
{
  MetaScreen *screen = meta_plugin_get_screen (plugin);
  ScreenTilePreview *preview = get_screen_tile_preview (screen);

  clutter_actor_hide (preview->actor);
}

static void
kill_switch_workspace (MetaPlugin     *plugin)
{
  MetaDefaultPluginPrivate *priv = META_DEFAULT_PLUGIN (plugin)->priv;

  if (priv->tml_switch_workspace1)
    {
      clutter_timeline_stop (priv->tml_switch_workspace1);
      clutter_timeline_stop (priv->tml_switch_workspace2);
      g_signal_emit_by_name (priv->tml_switch_workspace1, "completed", NULL);
    }
}

static void
kill_window_effects (MetaPlugin      *plugin,
                     MetaWindowActor *window_actor)
{
  ActorPrivate *apriv;

  apriv = get_actor_private (window_actor);

  if (apriv->tml_minimize)
    {
      clutter_timeline_stop (apriv->tml_minimize);
      g_signal_emit_by_name (apriv->tml_minimize, "completed", NULL);
    }

  if (apriv->tml_map)
    {
      clutter_timeline_stop (apriv->tml_map);
      g_signal_emit_by_name (apriv->tml_map, "completed", NULL);
    }

  if (apriv->tml_destroy)
    {
      clutter_timeline_stop (apriv->tml_destroy);
      g_signal_emit_by_name (apriv->tml_destroy, "completed", NULL);
    }
}

static const MetaPluginInfo *
plugin_info (MetaPlugin *plugin)
{
  MetaDefaultPluginPrivate *priv = META_DEFAULT_PLUGIN (plugin)->priv;

  return &priv->info;
}

static void
on_dialog_closed (GPid     pid,
                  gint     status,
                  gpointer user_data)
{
  MetaPlugin *plugin = user_data;
  gboolean ok;

  ok = g_spawn_check_exit_status (status, NULL);
  meta_plugin_complete_display_change (plugin, ok);
}

static void
confirm_display_change (MetaPlugin *plugin)
{
  GPid pid;

  pid = meta_show_dialog ("--question",
                          "Does the display look OK?",
                          "20",
                          NULL,
                          "_Keep This Configuration",
                          "_Restore Previous Configuration",
                          "preferences-desktop-display",
                          0,
                          NULL, NULL);

  g_child_watch_add (pid, on_dialog_closed, plugin);
}

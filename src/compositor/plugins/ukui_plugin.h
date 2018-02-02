#ifndef UKUI_PLUGIN_H
#define UKUI_PLUGIN_H

#include <gio/gio.h>

#define UKUI_PLUGIN_BUS         G_BUS_TYPE_SESSION
#define UKUI_PLUGIN_BUS_NAME    "org.ukui.ukwm.UkwmPlugin"
#define UKUI_PLUGIN_OBJECT_PATH "/org/ukui/ukwm/UkwmPlugin"

#define PREVIEW_WIDTH     168
#define PREVIEW_HEIGHT    128
#define SPACE_WIDTH       8
#define SPACE_HEIGHT      8
#define THUMBNAIL_WIDTH   (PREVIEW_WIDTH - SPACE_WIDTH)
#define THUMBNAIL_HEIGHT  (PREVIEW_HEIGHT - SPACE_HEIGHT)
#define ICON_WIDTH        48
#define ICON_HEIGHT       48

#define ATP_COUNT 128

typedef struct _alt_tab_item {
  char * title_name;
  unsigned long xid;
  int x;
  int y;
  int width;
  int height;
  void * icon;
} alt_tab_item;

#define PATH_MAX_LEN    1024
#define PID_STRING_LEN  64

#define TAB_LIST_IMAGE_FILE   "ukwm-tab-list.image"
#define WORKSPACE_IMAGE_FILE  "ukwm-workspace.image"
#define PROGRAM_NAME          "ukui-window-switch"

#endif // UKUI_PLUGIN_H

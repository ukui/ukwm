#ifndef UKUI_PLUGIN_H
#define UKUI_PLUGIN_H

#include <gio/gio.h>

#define UKUI_PLUGIN_BUS               G_BUS_TYPE_SESSION
#define UKUI_PLUGIN_BUS_NAME          "org.ukui.ukwm.UkwmPlugin"
#define UKUI_PLUGIN_OBJECT_PATH       "/org/ukui/ukwm/UkwmPlugin"

#define PREVIEW_WIDTH   168
#define PREVIEW_HEIGHT  128
#define SPACE_WIDTH             8
#define SPACE_HEIGHT    8
#define THUMBNAIL_WIDTH         (PREVIEW_WIDTH - SPACE_WIDTH)
#define THUMBNAIL_HEIGHT        (PREVIEW_HEIGHT - SPACE_HEIGHT)
#define ICON_WIDTH      48
#define ICON_HEIGHT     48

#define ATP_COUNT 100

#undef SHARED_MEMORY
#ifdef SHARED_MEMORY
       #define ATP_SHM_KEY             1064
       #define ATP_SHM_COUNT   ATP_COUNT
       #define ATP_SHM_BUF_SIZE        1920 * 1080 * 8
       #define ATP_SHM_SIZE    (sizeof(atp_shm_area))
#endif

typedef struct _alt_tab_item {
       char * title_name;
       int x;
       int y;
       int width;
       int height;
       void * priv;
} alt_tab_item;

#ifdef SHARED_MEMORY
       typedef struct _atp_shm_area {
               unsigned long len;
               unsigned char buffer[ATP_SHM_BUF_SIZE];
       } atp_shm_area;
#endif

#define PATH_MAX_LEN    		1024
#define PID_STRING_LEN  		64

#define TAB_LIST_IMAGE_FILE             "ukwm-tab-list.image"
#define WORKSPACE_IMAGE_FILE    	"ukwm-workspace.image"
#define PROGRAM_NAME 			"ukui-window-switch"

#endif // UKUI_PLUGIN_H

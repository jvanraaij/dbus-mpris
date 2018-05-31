// Simple dumb executable to dump the current playing track from audacious (or other MPRIS2 player).
// Basically the same as `audtool current-song` except this was a pointless excuse to figure out DBus.
#include <stdio.h>
#include <stdbool.h>
#include <gio/gio.h>

#define MPRIS_BUSNAME "org.mpris.MediaPlayer2.audacious"
#define MPRIS_OBJECT "/org/mpris/MediaPlayer2"
#define MPRIS_INTERFACE "org.mpris.MediaPlayer2.Player"

#define STR_UNKNOWNTITLE "Unknown Title"
#define STR_UNKNOWNARTIST "Unknown Artist"

GDBusProxy *mpris_player = NULL;
GMainLoop *loop;
bool dodump = true;

static void clean() {
    if(mpris_player) {
        g_object_unref(mpris_player);
        mpris_player = NULL;
    }
}

static void bus_found(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data) {
    GError *err = NULL;
    mpris_player = g_dbus_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, NULL, name, MPRIS_OBJECT, MPRIS_INTERFACE, NULL, &err);
    if(mpris_player) {
        GVariant *metadata = g_dbus_proxy_get_cached_property(mpris_player, "Metadata");
        if(metadata == NULL) {
            g_printerr("Failed to get metadata.\n");
            g_main_loop_quit(loop);
            return;
        }

        GVariant *titleV = g_variant_lookup_value(metadata, "xesam:title", G_VARIANT_TYPE_STRING);
        GVariant *artistVA = g_variant_lookup_value(metadata, "xesam:artist", G_VARIANT_TYPE_STRING_ARRAY);

        gchar *title = NULL;
        gchar *artist = NULL;

        if(titleV != NULL) {
            title = g_variant_dup_string(titleV, NULL);
            g_variant_unref(titleV);
        }

        if(artistVA != NULL) {
            gsize artistsc;
            const gchar **artists = g_variant_get_strv(artistVA, &artistsc);
            artist = artistsc ? g_strdup(artists[0]) : NULL;
            g_free(artists);
            g_variant_unref(artistVA);
        }


        if(dodump) {
            g_print("%s - %s\n", artist ? artist : STR_UNKNOWNARTIST, title ? title : STR_UNKNOWNTITLE);
            dodump = false;
        }

        g_free(title);
        g_free(artist);

        g_variant_unref(metadata);
        g_main_loop_quit(loop);
    }
    else {
        g_print("ERR: %s\n", err->message);
    }
}

static void bus_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data) {
    g_main_loop_quit(loop);
    if(dodump) {
        g_print("[No media player running]\n");
        dodump = false;
    }

    clean();
}

int main() {
    guint watch = g_bus_watch_name(G_BUS_TYPE_SESSION, MPRIS_BUSNAME, G_BUS_NAME_WATCHER_FLAGS_NONE, bus_found, bus_vanished, NULL, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // The documentation would lead me to believe this calls bus_vanished(), but it doesn't.
    g_bus_unwatch_name(watch);
    clean();

    return 0;
}
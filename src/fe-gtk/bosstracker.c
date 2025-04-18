// bosstracker.c
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <stdio.h>
#include <libgen.h>
#include <windows.h>

#define GIST_URL "https://gist.githubusercontent.com/LeoPiro/1cf96c4e39510561979a3d12728b2d5c/raw/boss_tracker.txt"

static GtkWidget *window;
static GtkWidget *tracker_box;

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
        return 0;

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static void apply_red_bar_style(void) {
    gtk_rc_parse_string(
        "style \"red-bar\"\n"
        "{\n"
        "  fg[PRELIGHT] = { 1.0, 0.0, 0.0 }\n"
        "  fg[NORMAL]   = { 1.0, 0.0, 0.0 }\n"
        "  bg[NORMAL]   = { 1.0, 0.0, 0.0 }\n"
        "  base[NORMAL] = { 1.0, 0.0, 0.0 }\n"
        "}\n"
        "widget \"*.red-bar\" style \"red-bar\""
    );
}

static void load_boss_data(void) {
    printf("[DEBUG] load_boss_data called\n");
    fflush(stdout);

    GList *children = gtk_container_get_children(GTK_CONTAINER(tracker_box));
    GList *iter;
    for (iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };

    curl = curl_easy_init();
    if (curl) {
        char full_url[512];
        snprintf(full_url, sizeof(full_url), "%s?nocache=%lld", GIST_URL, (long long)time(NULL));
        curl_easy_setopt(curl, CURLOPT_URL, full_url);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "HexChatBossTracker/1.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        char *last_slash = strrchr(exe_path, '\\');
        if (last_slash) *(last_slash + 1) = '\0';

        char ca_path[MAX_PATH];
        snprintf(ca_path, sizeof(ca_path), "%sca-bundle.crt", exe_path);
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fflush(stderr);
        }

        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("[DEBUG] HTTP Response Code: %ld\n", response_code);
        printf("[DEBUG] Raw response:\n%s\n", chunk.memory);
        fflush(stdout);

        if (chunk.size == 0 || chunk.memory[0] == '\0') {
            printf("[DEBUG] Empty response from gist, using fallback.\n");
            free(chunk.memory);
            chunk.memory = strdup("Shadow,80\nKraken,100\nBehemoth,45\n");
            chunk.size = strlen(chunk.memory);
        }

        if (res == CURLE_OK) {
            char *line = strtok(chunk.memory, "\n");
            while (line) {
                char *comma = strchr(line, ',');
                if (comma) {
                    *comma = '\0';
                    const char *name = line;
                    int percent = atoi(comma + 1);

                    GtkWidget *outer_hbox = gtk_hbox_new(FALSE, 0);
                    GtkWidget *spacer = gtk_label_new("   ");
                    gtk_box_pack_start(GTK_BOX(outer_hbox), spacer, FALSE, FALSE, 0);

                    GtkWidget *hbox = gtk_hbox_new(FALSE, 4);

                    GtkWidget *label = gtk_label_new(name);
                    gtk_widget_set_size_request(label, 120, -1);
                    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
                    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

                    GtkWidget *progress = gtk_progress_bar_new();
                    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), percent / 100.0);
                    gtk_widget_set_size_request(progress, 200, 16);

                    if (percent > 75)
                        gtk_widget_set_name(progress, "red-bar");

                    gtk_box_pack_start(GTK_BOX(hbox), progress, FALSE, FALSE, 0);
                    gtk_box_pack_start(GTK_BOX(outer_hbox), hbox, FALSE, FALSE, 0);
                    gtk_box_pack_start(GTK_BOX(tracker_box), outer_hbox, FALSE, FALSE, 2);
                }
                line = strtok(NULL, "\n");
            }

            gtk_widget_queue_draw(tracker_box);
            gtk_container_foreach(GTK_CONTAINER(tracker_box), (GtkCallback)gtk_widget_show_all, NULL);
        }

        curl_easy_cleanup(curl);
    }

    free(chunk.memory);
}

static gboolean refresh_boss_data(gpointer user_data) {
    load_boss_data();
    return TRUE;
}

void open_bosstracker_window(GtkWidget *widget, gpointer data) {
    printf("[DEBUG] open_bosstracker_window called\n");
    fflush(stdout);

    if (window && GTK_IS_WINDOW(window)) {
        gtk_window_present(GTK_WINDOW(window));
        return;
    }

    apply_red_bar_style();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Boss Tracker");
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 550);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

    tracker_box = gtk_vbox_new(FALSE, 4);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tracker_box);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(window), scroll);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroyed), &window);

    load_boss_data();
    g_timeout_add_seconds(30, refresh_boss_data, NULL);

    gtk_widget_show_all(window);
}

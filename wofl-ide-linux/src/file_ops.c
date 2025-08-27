#include "file_ops.h"
#include "gap_buffer.h"

bool load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    gb_free(&g_app.buf);
    gb_init(&g_app.buf);
    
    char buffer[4096];
    size_t read_size;
    while ((read_size = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        gb_insert(&g_app.buf, buffer, read_size);
    }
    
    fclose(f);
    strcpy(g_app.file_path, filename);
    
    const char *name = strrchr(filename, '/');
    strcpy(g_app.file_name, name ? name + 1 : filename);
    
    g_app.buf.dirty = false;
    return true;
}

bool gb_save_to_file(GapBuffer *gb, const char *path, EolMode eol) {
    FILE *f = fopen(path, "w");
    if (!f) return false;

    const size_t len = gb_length(gb);
    for (size_t i = 0; i < len; i++) {
        char ch = gb_char_at(gb, i);
        if (ch == '\n' && eol == EOL_CRLF) {
            fputc('\r', f);
        }
        fputc(ch, f);
    }

    fclose(f);
    gb->dirty = false;
    return true;
}

#include <sys/wait.h>

bool open_file_dialog(char *selected_path, size_t max_len) {
    // Use zenity for file dialog if available
    FILE *fp = popen("which zenity", "r");
    if (!fp) return false;
    
    char path[256];
    bool has_zenity = (fgets(path, sizeof(path), fp) != NULL);
    pclose(fp);
    
    if (has_zenity) {
        // Use zenity file dialog
        fp = popen("zenity --file-selection --title=\"Open File\"", "r");
        if (fp) {
            if (fgets(selected_path, max_len, fp)) {
                // Remove newline
                size_t len = strlen(selected_path);
                if (len > 0 && selected_path[len-1] == '\n') {
                    selected_path[len-1] = '\0';
                }
                pclose(fp);
                return true;
            }
            pclose(fp);
        }
    }
    
    // Fallback: simple console input
    printf("Enter file path to open: ");
    fflush(stdout);
    if (fgets(selected_path, max_len, stdin)) {
        size_t len = strlen(selected_path);
        if (len > 0 && selected_path[len-1] == '\n') {
            selected_path[len-1] = '\0';
        }
        return selected_path[0] != '\0';
    }
    
    return false;
}

bool save_file_dialog(char *selected_path, size_t max_len) {
    FILE *fp = popen("which zenity", "r");
    if (!fp) return false;
    
    char path[256];
    bool has_zenity = (fgets(path, sizeof(path), fp) != NULL);
    pclose(fp);
    
    if (has_zenity) {
        fp = popen("zenity --file-selection --save --title=\"Save File\"", "r");
        if (fp) {
            if (fgets(selected_path, max_len, fp)) {
                size_t len = strlen(selected_path);
                if (len > 0 && selected_path[len-1] == '\n') {
                    selected_path[len-1] = '\0';
                }
                pclose(fp);
                return true;
            }
            pclose(fp);
        }
    }
    
    // Fallback: console input
    printf("Enter file path to save: ");
    fflush(stdout);
    if (fgets(selected_path, max_len, stdin)) {
        size_t len = strlen(selected_path);
        if (len > 0 && selected_path[len-1] == '\n') {
            selected_path[len-1] = '\0';
        }
        return selected_path[0] != '\0';
    }
    
    return false;
}

void save_file() {
    if (!g_app.file_path[0]) {
        strcpy(g_app.file_path, "untitled.txt");
        strcpy(g_app.file_name, "untitled.txt");
    }
    
    if (gb_save_to_file(&g_app.buf, g_app.file_path, EOL_LF)) {
        printf("Saved: %s\n", g_app.file_path);
        g_app.buf.dirty = false;
    } else {
        printf("Save failed: %s\n", g_app.file_path);
    }
}
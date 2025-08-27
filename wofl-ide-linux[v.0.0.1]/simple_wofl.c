// ==================== simple_wofl.c ====================
// Simple line-based terminal editor for Linux

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 256
#define MAX_PATH 512

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
    int current_line;
    char filename[MAX_PATH];
    int modified;
} Editor;

static Editor editor = {0};
static struct termios orig_termios;

// Terminal handling
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char read_key() {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

void wait_for_key() {
    printf("Press any key to continue...");
    fflush(stdout);
    char c;
    read(STDIN_FILENO, &c, 1);
    printf("\n");
}

char get_yes_no(const char* prompt) {
    printf("%s (y/n): ", prompt);
    fflush(stdout);
    char c;
    do {
        read(STDIN_FILENO, &c, 1);
        c = tolower(c);
    } while (c != 'y' && c != 'n');
    printf("%c\n", c);
    return c;
}

// File operations
void load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("New file: %s\n", filename);
        editor.line_count = 1;
        strcpy(editor.lines[0], "");
        strncpy(editor.filename, filename, MAX_PATH - 1);
        return;
    }
    
    editor.line_count = 0;
    while (editor.line_count < MAX_LINES && 
           fgets(editor.lines[editor.line_count], MAX_LINE_LEN, f)) {
        // Remove newline
        int len = strlen(editor.lines[editor.line_count]);
        if (len > 0 && editor.lines[editor.line_count][len-1] == '\n') {
            editor.lines[editor.line_count][len-1] = '\0';
        }
        editor.line_count++;
    }
    
    if (editor.line_count == 0) {
        editor.line_count = 1;
        strcpy(editor.lines[0], "");
    }
    
    fclose(f);
    strncpy(editor.filename, filename, MAX_PATH - 1);
    printf("Loaded %d lines from %s\n", editor.line_count, filename);
    wait_for_key();
}

void save_file() {
    FILE *f = fopen(editor.filename, "w");
    if (!f) {
        printf("Error: Cannot save %s\n", editor.filename);
        wait_for_key();
        return;
    }
    
    for (int i = 0; i < editor.line_count; i++) {
        fprintf(f, "%s\n", editor.lines[i]);
    }
    
    fclose(f);
    editor.modified = 0;
    printf("Saved %d lines to %s\n", editor.line_count, editor.filename);
}

// Editor operations
void insert_line() {
    if (editor.line_count >= MAX_LINES - 1) {
        printf("Error: Too many lines\n");
        wait_for_key();
        return;
    }
    
    // Shift lines down
    for (int i = editor.line_count; i > editor.current_line + 1; i--) {
        strcpy(editor.lines[i], editor.lines[i-1]);
    }
    
    strcpy(editor.lines[editor.current_line + 1], "");
    editor.line_count++;
    editor.current_line++;
    editor.modified = 1;
}

void delete_line() {
    if (editor.line_count <= 1) return;
    
    // Shift lines up
    for (int i = editor.current_line; i < editor.line_count - 1; i++) {
        strcpy(editor.lines[i], editor.lines[i+1]);
    }
    
    editor.line_count--;
    if (editor.current_line >= editor.line_count) {
        editor.current_line = editor.line_count - 1;
    }
    editor.modified = 1;
}

// Display
void show_status() {
    printf("\n=== WOFL Simple Editor ===\n");
    printf("File: %s%s | Line %d/%d\n", 
           editor.filename, editor.modified ? "*" : "", 
           editor.current_line + 1, editor.line_count);
    printf("Commands: (i)nsert (d)elete (e)dit (s)ave (r)un (q)uit (h)elp\n");
}

void show_lines() {
    printf("\n");
    int start = editor.current_line > 5 ? editor.current_line - 5 : 0;
    int end = start + 10;
    if (end > editor.line_count) end = editor.line_count;
    
    for (int i = start; i < end; i++) {
        char marker = (i == editor.current_line) ? '>' : ' ';
        printf("%c %3d: %s\n", marker, i + 1, editor.lines[i]);
    }
}

void show_help() {
    printf("\nWOFL Simple Editor Commands:\n");
    printf("  i - Insert new line after current\n");
    printf("  d - Delete current line\n");
    printf("  e - Edit current line\n");
    printf("  j - Move to next line\n");
    printf("  k - Move to previous line\n");
    printf("  s - Save file\n");
    printf("  r - Run file (Python/Shell)\n");
    printf("  q - Quit\n");
    printf("  h - Show this help\n");
}

void edit_line() {
    printf("Edit line %d: ", editor.current_line + 1);
    printf("Current: %s\n", editor.lines[editor.current_line]);
    printf("New: ");
    fflush(stdout);
    
    // Use regular input for line editing
    disable_raw_mode();
    if (fgets(editor.lines[editor.current_line], MAX_LINE_LEN, stdin)) {
        // Remove newline
        int len = strlen(editor.lines[editor.current_line]);
        if (len > 0 && editor.lines[editor.current_line][len-1] == '\n') {
            editor.lines[editor.current_line][len-1] = '\0';
        }
        editor.modified = 1;
    }
    enable_raw_mode();
}

void run_file() {
    if (!editor.filename[0]) {
        printf("No file loaded\n");
        wait_for_key();
        return;
    }
    
    if (editor.modified) {
        char c = get_yes_no("Save file first?");
        if (c == 'y') {
            save_file();
        }
    }
    
    char command[MAX_PATH + 50];
    const char *ext = strrchr(editor.filename, '.');
    
    if (ext && strcmp(ext, ".py") == 0) {
        snprintf(command, sizeof(command), "python3 '%s'", editor.filename);
    } else if (ext && strcmp(ext, ".sh") == 0) {
        snprintf(command, sizeof(command), "bash '%s'", editor.filename);
    } else {
        snprintf(command, sizeof(command), "'./%s'", editor.filename);
    }
    
    printf("\nRunning: %s\n", command);
    printf("=== OUTPUT ===\n");
    fflush(stdout);
    
    disable_raw_mode();
    int result = system(command);
    printf("\n=== EXIT CODE: %d ===\n", result);
    printf("Press ENTER to continue...");
    fflush(stdout);
    
    // Clear any remaining input
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    enable_raw_mode();
}

// Main loop
int main(int argc, char *argv[]) {
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        strcpy(editor.filename, "untitled.txt");
        editor.line_count = 1;
        strcpy(editor.lines[0], "");
    }
    
    enable_raw_mode();
    
    while (1) {
        printf("\033[2J\033[H"); // Clear screen
        show_status();
        show_lines();
        
        char c = read_key();
        
        switch (c) {
            case 'q':
            case 'Q':
                if (editor.modified) {
                    char save = get_yes_no("File modified. Save?");
                    if (save == 'y') {
                        save_file();
                    }
                }
                printf("\nGoodbye!\n");
                return 0;
                
            case 'i':
            case 'I':
                insert_line();
                break;
                
            case 'd':
            case 'D':
                delete_line();
                break;
                
            case 'e':
            case 'E':
                edit_line();
                break;
                
            case 'j':
            case 'J':
                if (editor.current_line < editor.line_count - 1) {
                    editor.current_line++;
                }
                break;
                
            case 'k':
            case 'K':
                if (editor.current_line > 0) {
                    editor.current_line--;
                }
                break;
                
            case 's':
            case 'S':
                save_file();
                wait_for_key();
                break;
                
            case 'r':
            case 'R':
                run_file();
                break;
                
            case 'h':
            case 'H':
                show_help();
                wait_for_key();
                break;
                
            default:
                // Ignore unknown keys
                break;
        }
    }
    
    return 0;
}
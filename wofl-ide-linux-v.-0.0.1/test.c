cat > test.c << 'EOF'
#include <ncurses.h>
int main() {
    initscr();
    printw("Hello ncurses! Press any key...");
    refresh();
    getch();
    endwin();
    return 0;
}
EOF
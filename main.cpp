#include <string.h>
#include <ncurses.h>
#include <vector>
// create a Window class wrapping ncurses window
class Window {
private:
    WINDOW *win;
    int w, h;
    int startx, starty;
    struct Cell {
        int x, y;
    };
    std::vector<Cell> cells = {
        {0, 0}, {13, 0}, {26, 0},
        {0, 6}, {13, 6}, {26, 6},
        {0, 12}, {13, 12}, {26, 12}
    };
public:
// constructor for Window class that takes window height and width and x and y coordinates
    Window(int height, int width, int starty, int startx): w(width), h(height), startx(startx), starty(starty) {
        // create a new window
        win = newwin(height, width, starty, startx);
        // set the background color to blue
        wbkgd(win, COLOR_PAIR(1));
        // refresh the window
        wrefresh(win);
        refresh();
        renderBorder();
        attron(COLOR_PAIR(3));
        //draw horizontal line
        mvhline(starty + height, startx + 1, 0, width);
        // draw vertical line
        mvvline(starty + 1, startx + width, 0, height);
        attroff(COLOR_PAIR(3));
        wrefresh(win);
        move(0, 0);
        wattron(win, COLOR_PAIR(1));
        mvwhline(win, height / 3, 1, 0, width - 2);
        mvwhline(win, (height * 2) / 3, 1, 0, width - 2);
        mvwvline(win, 1, width / 3, 0, height - 2);
        mvwvline(win, 1, (width * 2) / 3, 0, height - 2);
        wattroff(win, COLOR_PAIR(1));
        wrefresh(win);
    }
    // destructor for Window class
    ~Window() {
        delwin(win);
    }
    // render cross at x and y coordinates
    void renderCross(int x, int y) {
        wattron(win, COLOR_PAIR(5));
        mvwprintw(win, y + 1, x + 2, "XX");
        mvwprintw(win, y + 1, x + 2 + 8, "XX");
        mvwprintw(win, y + 2, x + 4, "XX");
        mvwprintw(win, y + 2, x + 4 + 6 - 2, "XX");
        mvwprintw(win, y + 3, x + 6, "XX");
        mvwprintw(win, y + 4, x + 4, "XX");
        mvwprintw(win, y + 4, x + 4 + 6 - 2, "XX");
        mvwprintw(win, y + 5, x + 2, "XX");
        mvwprintw(win, y + 5, x + 2 + 8, "XX");
        wattroff(win, COLOR_PAIR(5));
        wrefresh(win);
    }
    // render circle at x and y coordinates
    void renderCircle(int x, int y) {
        wattron(win, COLOR_PAIR(5));
        mvwprintw(win, y + 1, x + 6, "00");
        mvwprintw(win, y + 2, x + 4, "0");
        mvwprintw(win, y + 2, x + 9, "0");
        mvwprintw(win, y + 3, x + 3, "0");
        mvwprintw(win, y + 3, x + 10, "0");
        mvwprintw(win, y + 4, x + 4, "0");
        mvwprintw(win, y + 4, x + 9, "0");
        mvwprintw(win, y + 5, x + 6, "00");

        wattroff(win, COLOR_PAIR(5));
        wrefresh(win);
    }
    // window click function that takes x and y coordinates
    void click(int x, int y) {
        if (x <= startx && x >= startx + w && y <= starty && y >= starty + h) {
            return;
        }
        x -= startx;
        y -= starty;
    }
    // a function to render window border
    void renderBorder() {
        box(win, 0, 0);
        char const* title = " TicTacToe ";
        wattron(win, COLOR_PAIR(4));
        mvwprintw(win, 0, 1 + (w - 2 - strlen(title)) / 2, "%s", title);
        wattroff(win, COLOR_PAIR(4));
        wrefresh(win);
    }
    // function to print a formated string to the window
    void print(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        wmove(win, 1, 1);
        // clear the previous line
        wclrtoeol(win);
        // print the string to the window
        vw_printw(win, fmt, args);
        va_end(args);
        // refresh the window
        wrefresh(win);
        renderBorder();
        move(0, 0);
    }
    // key input function
    int input() const {
        // return the key input
        return wgetch(win);
    }
    // function to enable keypad
    void enable_keypad() const {
        keypad(win, TRUE);
    }
};

// main function
int main() {
    // initialize ncurses
    initscr();
    cbreak();
    // set the color pairs
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_WHITE);
    // init color pair with background color white and foreground color blue
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    init_pair(3, COLOR_BLACK, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_WHITE);
    init_pair(5, COLOR_BLACK, COLOR_WHITE);
    // set the background color of stdsrc to blue
    bkgd(COLOR_PAIR(2));
    // get terminal size
    int height, width;
    getmaxyx(stdscr, height, width);
    // create a new window with height 20 and width 40 and centered on the screen
    Window win(19, 40, (height - 20) / 2, (width - 40) / 2);
    // enable keypad
    win.enable_keypad();
    // enable mouse events
    mousemask(ALL_MOUSE_EVENTS, NULL);
    // render cross at x and y coordinates
    win.renderCross(0, 0);
    // render circle at x and y coordinates
    win.renderCircle(13, 0);
    while (true) {
        int ch = getch();
        switch (ch) {
            case KEY_MOUSE:
                MEVENT event;
                if (getmouse(&event) == OK) {
                    win.click(event.x, event.y);
                }
                break;
            case 27:
                endwin();
                return 0;
        }
    }
}
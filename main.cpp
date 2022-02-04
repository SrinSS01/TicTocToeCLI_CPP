#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <vector>

class Window {
protected:
    const int width;
    const int height;
    WINDOW* window;
private:
    char const* const title;
public:
    Window(
        int x,
        int y, 
        int width, 
        int height, 
        const char* title
    ): width(width), height(height), title(title) {
        window = newwin(height, width, y, x);
        // set window background color to blue on white
        wbkgd(window, COLOR_PAIR(1));
        wrefresh(window);
        refresh();
        renderBorder();
        // enable attribute color black on black
        attron(COLOR_PAIR(3));
        mvhline(y + height, x + 1, 0, width);
        mvvline(y + 1, x + width, 0, height);
        attroff(COLOR_PAIR(3));
        refresh();
    }

    virtual ~Window() {
        delwin(window);
    }

    // renderBorder
    void renderBorder() const {
        box(window, 0, 0);
        // enable attribute color red on white
        wattron(window, COLOR_PAIR(4));
        mvwprintw(window, 0, 1 + (width - 2 - strlen(title)) / 2, " %s ", title);
        wattroff(window, COLOR_PAIR(4));
        wrefresh(window);
    }

    // render text at x, y position and the text is formatted with variable args
    template <typename... T>
    void renderText(int x, int y, const char* format, T... args) const {
        // enable attribute color black on white
        wattron(window, COLOR_PAIR(5));
        mvwprintw(window, y, x, format, args...);
        wattroff(window, COLOR_PAIR(5));
        wrefresh(window);
    }

    // clear the window
    virtual void clear() const {
        wclear(window);
        renderBorder();
        wrefresh(window);
    }
};

// create a Board class extending Window
class Board: public Window {
public:
    Board(
        int x,
        int y, 
        int width, 
        int height
    ): Window(x, y, width, height, "TicTacToe") {
        renderBoard();
    }

    // renderBoard
    void renderBoard() const {
        mvwhline(window, height / 3, 1, 0, width - 2);
        mvwhline(window, (height * 2) / 3, 1, 0, width - 2);
        mvwvline(window, 1, width / 3, 0, height - 2);
        mvwvline(window, 1, (width * 2) / 3, 0, height - 2);
        // loop row from 0 to 12 with step 6 and column from 0 to 26 with step 13
        unsigned int k = 8u;
        // enable color red on white
        wattron(window, COLOR_PAIR(4));
        for (int row = 0; row <= 12; row += 6) {
            for (int column = 0; column <= 26; column += 13) {
                mvwaddch(window, row + 5, column + 12, 48u + k--);
            }
        }
        wattroff(window, COLOR_PAIR(4));
        wrefresh(window);
    }

    // renderCross
    void renderCross(int x, int y) const {
        wattron(window, COLOR_PAIR(5));
        mvwprintw(window, y + 1, x + 2, "XX");
        mvwprintw(window, y + 1, x + 2 + 8, "XX");
        mvwprintw(window, y + 2, x + 4, "XX");
        mvwprintw(window, y + 2, x + 4 + 6 - 2, "XX");
        mvwprintw(window, y + 3, x + 6, "XX");
        mvwprintw(window, y + 4, x + 4, "XX");
        mvwprintw(window, y + 4, x + 4 + 6 - 2, "XX");
        mvwprintw(window, y + 5, x + 2, "XX");
        mvwprintw(window, y + 5, x + 2 + 8, "XX");
        wattroff(window, COLOR_PAIR(5));
        wrefresh(window);
    }

    // renderCircle
    void renderCircle(int x, int y) const {
        wattron(window, COLOR_PAIR(5));
        mvwprintw(window, y + 1, x + 6, "00");
        mvwprintw(window, y + 2, x + 4, "0");
        mvwprintw(window, y + 2, x + 9, "0");
        mvwprintw(window, y + 3, x + 3, "0");
        mvwprintw(window, y + 3, x + 10, "0");
        mvwprintw(window, y + 4, x + 4, "0");
        mvwprintw(window, y + 4, x + 9, "0");
        mvwprintw(window, y + 5, x + 6, "00");
        wattroff(window, COLOR_PAIR(5));
        wrefresh(window);
    }

    // override clear method
    void clear() const override {
        Window::clear();
        renderBoard();
    }
};

struct Coordinate {
    int x;
    int y;
};

class TicTacToeEngine {
private:
    char player {'x'};
    const int LAST_INDEX {1 << 8};
    const int COLUMN_0 {4 | (4 << 3) | (4 << 3 << 3)};
    const int COLUMN_1 {2 | (2 << 3) | (2 << 3 << 3)};
    const int COLUMN_2 {1 | (1 << 3) | (1 << 3 << 3)};
    const int ROW_0 {7};
    const int ROW_1 {7 << 3};
    const int ROW_2 {7 << 3 << 3};
    const int DIAGONAL_RIGHT {84};
    const int DIAGONAL_LEFT {273};
    const int DRAW_BITMASK {ROW_0 | ROW_1 | ROW_2};
    int xBoard = 0;
    int oBoard = 0;
    mutable int won = false;
    mutable int draw = false;
    const std::vector<int> win_combinaton = {
        ROW_0, ROW_1, ROW_2,
        COLUMN_2, COLUMN_1, COLUMN_0,
        DIAGONAL_RIGHT, DIAGONAL_LEFT
    };

public:
    bool place(int pos) {
        if (won | draw) return false;
        auto cell = 1 << pos;
        if (cell > LAST_INDEX || cell < 0 || (xBoard | oBoard & cell) == cell) return false;
        if (player == 'x') {
            xBoard = xBoard | cell;
        } else {
            oBoard = oBoard | cell;
        }
        return true;
    }
    bool checkWin() const {
        return (player == 'x')? 
            std::find_if(win_combinaton.begin(), win_combinaton.end(), [this](int combination) {
                return (xBoard & combination) == combination;
            }) != win_combinaton.end()
            :
            std::find_if(win_combinaton.begin(), win_combinaton.end(), [this](int combination) {
                return (oBoard & combination) == combination;
            }) != win_combinaton.end();
    }
    bool isWin() const {
        if (checkWin()) {
            won = true;
            return true;
        }
        return false;
    }
    void swap() {
        player = (player == 'x')? 'o' : 'x';
    }
    bool isDraw() const {
        if ((xBoard | oBoard) == DRAW_BITMASK) {
            draw = true;
            return true;
        }
        return false;
    }
    // get player
    char getPlayer() const {
        return player;
    }
    // reset
    void reset() {
        xBoard = 0;
        oBoard = 0;
        won = false;
        draw = false;
        player = 'x';
    }
};

void displayPlayer(int x, int y, char player) {
    // enable color red on white
    attron(COLOR_PAIR(4));
    // print player
    auto text_length = 17;
    mvprintw(y - 2, x + (20 - text_length) / 2, "Current player: %c", player);
    attroff(COLOR_PAIR(4));
}

bool win(char player) {
    // get terminal size
    auto maxY = getmaxy(stdscr);
    auto maxX = getmaxx(stdscr);
    auto width = 20;
    auto height = 5;
    // create a new Window instance and center it with title winner!!!
    auto win = Window((maxX - width) / 2, (maxY - height) / 2, width, height, "Winner!!!");
    auto message_length = 14;
    win.renderText((width - message_length) / 2, (height / 2) - 1, "Player %c wins!", player);
    win.renderText((width - message_length) / 2, (height / 2)  + 1, "continue? (y/n)");
    // wait for user to press enter or escape
    while (true) {
        // check if user pressed y or n
        switch (getch()) {
            case 121: {
                return true;
            }
            case 110: {
                return false;
            }
        }
    }
}
bool draw() {
    // get terminal size
    auto maxY = getmaxy(stdscr);
    auto maxX = getmaxx(stdscr);
    auto width = 20;
    auto height = 5;
    // create a new Window instance and center it with title draw!!!
    auto win = Window((maxX - width) / 2, (maxY - height) / 2, width, height, "Draw!!!");
    auto message = "It's a draw :(";
    win.renderText((width - strlen(message)) / 2, (height / 2) - 1, message);
    win.renderText((width - strlen(message)) / 2, (height / 2)  + 1, "continue? (y/n)");
    while (true) {
        // check if user pressed y or n
        switch (getch()) {
            case 121: {
                return true;
            }
            case 110: {
                return false;
            }
        }
    }
}

// main function
int main() {
    // initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(false);
    // check if color is supported
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }
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
    auto startX = (width - 40) / 2;
    auto startY = (height - 19) / 2;
    // create a new Board with height 19 and width 40 and centered on the screen
    Board board(startX, startY, 40, 19);
    Coordinate coords[] = {
        {0, 0}, {13, 0}, {26, 0},
        {0, 6}, {13, 6}, {26, 6},
        {0, 12}, {13, 12}, {26, 12}
    };
    // create a new TicTacToeEngine
    TicTacToeEngine engine;
    // display the current player
    displayPlayer(startX, startY, engine.getPlayer());
    // create a game loop and switch through the key, run until the user presses escape
    while (true) {
        auto key = getch();
        switch (key) {
            case 48 ... 56: {
                auto pos = 8 - (key - 48);
                auto [x, y] = coords[pos];
                auto player = engine.getPlayer();
                if (engine.place(pos)) {
                    // check if player is x or o, if x then drawCross, if o then drawCircle
                    if (player == 'x') board.renderCross(x, y);
                    else board.renderCircle(x, y);
                    if (engine.isWin()) {
                        if (win(player)) {
                            board.clear();
                            engine.reset();
                        } else {
                            // exit the game
                            endwin();
                            return 0;
                        }
                    } else if (engine.isDraw()) {
                        if (draw()) {
                            board.clear();
                            engine.reset();
                        } else {
                            // exit the game
                            endwin();
                            return 0;
                        };
                    } else {
                        engine.swap();
                    }
                    displayPlayer(startX, startY, engine.getPlayer());
                }
            } break;
            case 27: {
                // exit the game
                endwin();
                return 0;
            }
        }
    }
}
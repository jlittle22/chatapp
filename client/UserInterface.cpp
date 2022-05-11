#include "UserInterface.h"

string promptPrefix;
int linesWritten;


UserInterface::UserInterface() {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    raw();
}

UserInterface::~UserInterface() {
    endwin();
}

void UserInterface::display(string content) {
    printw("%d: %s\n", linesWritten, content.c_str());
    linesWritten += 1;
    refresh();
}

string UserInterface::prompt(string content) {
    savePosition();
    moveTo(80, 0);
    printf("%s", content.c_str());
    string res = readLine();
    clearLastLine();
    restorePosition();
    return res;
}

string UserInterface::prompt() {
    savePosition();
    moveTo(80, 0);
    printf("%s", promptPrefix.c_str());
    string res = readLine();
    clearLastLine();
    restorePosition();
    return res;
}

string UserInterface::readLine() {
    char input[UI_INPUT_BUFFER_SIZE];
    fgets(input, UI_INPUT_BUFFER_SIZE, stdin);
    return string(input, strlen(input) - 1);
}

void UserInterface::savePosition() {
    printf("\033[u");
}

void UserInterface::restorePosition() {
    printf("\033[u");
}

void UserInterface::clearLastLine() {
    printf("\033[1A \033[2K");
}

void UserInterface::moveTo(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

void UserInterface::clearScreen() {
    printf("\033[2J\033[H");
}
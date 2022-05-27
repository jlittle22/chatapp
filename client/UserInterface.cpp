#include "UserInterface.h"


#define UI_USER_INPUT_HEIGHT 1
#define UI_HEADER_HEIGHT 1

#define ASCI_ENTER 10
#define ASCI_BACKSPACE 127

mutex messageHistoryMutex;


UserInterface::UserInterface() {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    this->promptFormat = (char*) "%s %s";
    this->messageHistory = vector<string>();
    setDimensions();
    this->scrollOffset = 0;
    // for (int i = 0; i < this->chatViewHeight; i++) {
    //     this->messageHistory.push_back(to_string(i));
    // }
    render();
}

void UserInterface::setDimensions() {
    getmaxyx(stdscr, this->windowHeight, this->windowWidth);
    this->chatViewHeight = this->windowHeight - UI_HEADER_HEIGHT - UI_USER_INPUT_HEIGHT;
}

UserInterface::~UserInterface() {
    endwin();
}

void UserInterface::display(string content) {
    messageHistoryMutex.lock();
    this->messageHistory.push_back(content);
    messageHistoryMutex.unlock();
    render();

    
}

void UserInterface::render() {
    prerender();
    printHeader();
    printMessageRange();
    mvprintw(this->windowHeight - 1, 0, this->promptPrefix.c_str(), this->scrollOffset);
    refresh();
}

string UserInterface::prompt(string prefix) {
    string currentMessage;
    int ch;
    while((ch = getch()) != KEY_F(1)) {
        
        switch(ch) {	
			case KEY_UP:
                if (this->scrollOffset > 0) {
                    this->scrollOffset--;
                    render();
                }
                break;
			case KEY_DOWN:
                if (this->scrollOffset + this->chatViewHeight < messageHistory.size()) {
                    this->scrollOffset++;
                    render();
                }
                break;
            case ASCI_BACKSPACE:
                if (currentMessage.length() != 0) {
                    currentMessage.pop_back();
                }
                break;
            case ASCI_ENTER:
                if (currentMessage.length() >= 1) {
                    printPrompt(prefix, "");
                    messageHistory.push_back(currentMessage);
                    return currentMessage;
                }
                break;
            case KEY_RESIZE:
                setDimensions();
                render();
                break;
            default:
                currentMessage.push_back(ch);
                break;
		}
        printPrompt(prefix, currentMessage);

        refresh();
    }
    return currentMessage;
}

string UserInterface::prompt() {
    return prompt(">");
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

void UserInterface::printHeader() {
    attron(A_BOLD | A_UNDERLINE);
    printw("Header\n");
    attroff(A_BOLD | A_UNDERLINE);
}

void UserInterface::prerender() {
    clear();
    move(0,0);
}

void UserInterface::printMessageRange() {
    messageHistoryMutex.lock();
    for (int i = 0; i < this->chatViewHeight && i < messageHistory.size(); i++) {
        printw("%d: %s\n", i + scrollOffset, messageHistory[i + this->scrollOffset].c_str());
    }
    messageHistoryMutex.unlock();
}

void UserInterface::printPrompt(string prefix, string currentMessage) {
    move(this->windowHeight - 1, promptPrefix.length());
    clrtoeol();
    printw(this->promptFormat, prefix.c_str(), currentMessage.c_str());
}
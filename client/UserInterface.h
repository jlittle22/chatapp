#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <string>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <vector>
#include <mutex>
#define UI_INPUT_BUFFER_SIZE 256

using namespace std;

class UserInterface {
    public:
        UserInterface();
        
        ~UserInterface();
        string prompt(string);
        string prompt();
        void display(string);  
    private:
        UserInterface(UserInterface*);
        vector<string> messageHistory;
        string promptPrefix;
        char* promptFormat;
        int windowHeight;
        int windowWidth;
        int chatViewHeight;
        int scrollOffset;
        void render();
        string readLine();
        void clearLastLine();
        void savePosition();
        void restorePosition();
        void moveTo(int, int);
        void clearScreen();
        void printHeader();
        void prerender();
        void printMessageRange();
        void setDimensions();
        void printPrompt(string, string);
};


#endif
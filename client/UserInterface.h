#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <string>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

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
        string readLine();
        void clearLastLine();
        void savePosition();
        void restorePosition();
        void moveTo(int, int);
        void clearScreen();
};


#endif
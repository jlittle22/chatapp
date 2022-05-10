#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <string>
#include <stdio.h>
#include <string.h>

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
};


#endif
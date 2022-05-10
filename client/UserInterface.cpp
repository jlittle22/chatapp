#include "UserInterface.h"

string promptPrefix; 

UserInterface::UserInterface() {
    // do nothing
    promptPrefix = "> ";
}

UserInterface::~UserInterface() {
    // do nothing
}

void UserInterface::display(string content) {
    printf("%s\n", content.c_str());
}

string UserInterface::prompt(string content) {
    printf("%s", content.c_str());
    return readLine();
}

string UserInterface::prompt() {
    printf("%s", promptPrefix);
    return readLine();
}

string UserInterface::readLine() {
    char input[UI_INPUT_BUFFER_SIZE];
    fgets(input, UI_INPUT_BUFFER_SIZE, stdin);
    return string(input, UI_INPUT_BUFFER_SIZE);
}
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>

#include "UserInterface.h"

using namespace std;

int main(int argc, char* argv[]) {
    UserInterface ui = UserInterface();
    
    ui.display("Connected");
    ui.display("Hello and Welcome to ChatApp");
    getch();
    return 0;
}
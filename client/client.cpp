#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <network.h>
#include "UserInterface.h"

using namespace std;

int main(int argc, char* argv[]) {
    
    ClientNetworkInterface cni = ClientNetworkInterface("localhost");
    UserInterface ui = UserInterface();
    
    ui.display("Connected");
    ui.display("Hello and welcome to ChatApp");
    
    string res = ui.prompt("Username: ");
    ui.display("Hello " + res);
    string msg = ui.prompt();
    ui.display(msg);
    return 0;
}
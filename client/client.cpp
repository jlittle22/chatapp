#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "clientNetworkInterface.h"
#include "UserInterface.h"

using namespace std;

typedef struct threadPackage_t {
    ClientNetworkInterface* cni;
    UserInterface* ui;
} ThreadPackage;

void* handleMessaging(void*);

int main(int argc, char* argv[]) {
    ClientNetworkInterface cni = ClientNetworkInterface("chat.johnsnlittle.com");
    UserInterface ui = UserInterface();
    ThreadPackage threadPackage = ThreadPackage();
    NetworkFormatter nformat = NetworkFormatter();
    threadPackage.ui = &ui;
    threadPackage.cni = &cni;
    pthread_t remoteMessageListener;
    pthread_create(&remoteMessageListener, NULL, handleMessaging, (void*) &threadPackage);

    // todo - better end condition
    while(1) {
        string res = ui.prompt();
        nformat.setData(res);
        nformat.setOpcode(C2S_CHAT_SENT);
        string fmessage = nformat.networkForm();
        cni.sendMessage(fmessage);
    }

    // wait for completion
    pthread_join(remoteMessageListener, NULL);

    // ui.prompt();
    return 0;
}

void* handleMessaging(void* in) {
    ThreadPackage* tp = (ThreadPackage*) in;
    ClientNetworkInterface* cni = tp->cni;
    UserInterface* ui = tp->ui;
    // todo make better end condition
    while(1) {
        string res = cni->readNextMessage();
        ui->display(res);
    }
    return NULL;
}
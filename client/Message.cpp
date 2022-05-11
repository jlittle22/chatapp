#include "Message.h"

using namespace std;

string content; 
int length;
timeval time;
Message::Message(string _content) {
    content = _content;
    length = content.length();
}

Message::~Message() {
    // do nothing
}

string Message::getContent() {
    return content;
}

int Message::getLength() {
    return length;
}
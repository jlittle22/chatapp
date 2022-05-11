#include "MessageHistory.h"



MessageHistory::MessageHistory() {
    this->count = 0;
}

MessageHistory::~MessageHistory() {}

void MessageHistory::push(Message* msgptr) {
    if (this->count == 0) {
        HistoryNode tmp(msgptr, NULL);
        newest = &tmp;
        oldest = newest;
    } else {
        HistoryNode tmp(msgptr, newest);
        newest = &tmp;
    }
}


HistoryNode::HistoryNode(Message* _msg, HistoryNode* _next) {
    this->next = _next;
    this->msg  = _msg;
}

HistoryNode::HistoryNode() {}

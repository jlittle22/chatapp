#ifndef MESSAGE_HISTORY_H
#define MESSAGE_HISTORY_H

#include "Message.h"

class MessageHistory {
    public:
        MessageHistory();
        ~MessageHistory();
        void push(Message*);
        HistoryNode* getSubHistory(int);
        HistoryNode* oldest; 
        HistoryNode* newest;
        int count;
};

class HistoryNode {
    public:
        HistoryNode();
        HistoryNode(Message*, HistoryNode*);
        ~HistoryNode();
        HistoryNode* next;
        Message* msg;
};



#endif
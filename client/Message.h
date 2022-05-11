#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>



class Message {
    
    public:
        Message(string);
        ~Message();
        string getContent();
        int getLength();
};

#endif 

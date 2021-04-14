//
//  utils.cpp
//  server
//
//  Created by XIAOLIANG WU on 4/11/21.
//

#include "utils.hpp"
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>



#define LEN_BYTE_LEN 4



void int_to_chars(u_long n, char* res){
    for (int i=0; i<LEN_BYTE_LEN; i++){
        res[i] = (n >> ((LEN_BYTE_LEN - 1 - i)*8)) & 255;
    }
}


uint chars_to_int(char* raw_data){
    uint a = 0;
    for (int i=0; i < LEN_BYTE_LEN; i++) {
        a += (unsigned char)(raw_data[i]) << ((LEN_BYTE_LEN - 1 - i)*8);
    }
    
    return a;
}

int MAX_MSG_LEN=100000;

void send_msg_with_length(int socket, std::string message) {
    u_long message_length = message.length();
    char data[MAX_MSG_LEN];
    int_to_chars(message_length, data);
    for (int i=0; i < message.length(); i++) {
        data[i+LEN_BYTE_LEN] = message[i];
    }
    
    send(socket, data, message_length + LEN_BYTE_LEN, 0);
}


std::string recv_msg_with_length(int socket) {
    char buffer[MAX_MSG_LEN];
    read(socket, buffer, LEN_BYTE_LEN);
    int msg_len = chars_to_int(buffer);

    if (msg_len <= 0 || msg_len >= MAX_MSG_LEN){
        perror("illegal msg length");
        return "";
    }

    read(socket, buffer, msg_len);
    char res[msg_len];
    strncpy(res, buffer, msg_len);

    std::string res_str;
    for (int i=0; i<msg_len; i++) {
        res_str += res[i];
    }

    return res_str;
}


int rand_int(int low, int high){
    return low + (std::rand()/(RAND_MAX / (high - low + 1)));
}

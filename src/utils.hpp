//
//  utils.hpp
//  server
//
//  Created by XIAOLIANG WU on 4/11/21.
//

#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>
#include <iostream>

void int_to_chars(u_long n, char* res);
uint chars_to_int(char* raw_data);
void send_msg_with_length(int socket, std::string message);
std::string recv_msg_with_length(int socket);
int rand_int(int low, int high);

#endif /* utils_hpp */

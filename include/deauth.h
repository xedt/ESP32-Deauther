#ifndef DEAUTH_H
#define DEAUTH_H

#include <Arduino.h>
#include <vector> 

void start_deauth(std::vector<int> wifi_number = {-1}, int attack_type = 0, uint16_t reason = 2);
void stop_deauth();

extern int eliminated_connections;
extern int deauth_type;

#endif
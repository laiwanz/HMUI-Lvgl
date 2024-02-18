#ifndef SLAVEMODE_2023_12_26
#define SLAVEMODE_2023_12_26

#include <iostream>
#include <string>

int loadMasterResource();

void* runMasterCheckSlavePing(void* arg);
int redisMasterCloseFunc(void* lpParameter);
int redisMasterRecvFunc(const std::string& sChannel, const std::string& sMessage, void* lpParameter);

void* runConnectToMasterHmi(void* arg);
int redisSlaveCloseFunc(void* lpParameter);
int redisSlaveRecvFunc(const std::string& sChannel, const std::string& sMessage, void* lpParameter);

#endif // !SLAVEMODE_2023_12_26

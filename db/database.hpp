#pragma once
#include <string>

int  sendquery(const char* query, int min = 0, int sub = 0, int response = 1,
               std::string divisor = " ");
int  updateData();
int  verifyConnection(Screen &currentScreen);

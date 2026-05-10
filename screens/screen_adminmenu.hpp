#pragma once
#include "../globals.hpp"

void screenAdminmenuUpdate(Screen& currentScreen,
                           bool& restartExplorar, bool& restartResultados, bool& restartTerminal,
                           bool& invalidIp, bool& inputEmpty, bool& invalidCredentials,
                           bool& successfulPdfCreation, bool& fromAdmin, bool& rqst, bool& correctstudent,
                           std::string& explorarFinalOutput, std::string& strresultados,
                           std::string& votes, std::string& outputTerm, std::string& modeInput,
                           std::string& outResultsMode, int& opcSize);
void screenAdminmenuDraw(Screen &currentScreen,
                         bool &invalidCredentials, bool &inputEmpty, bool &invalidIp,
                         bool &adminAuthenticated_local, bool& successfulPdfCreation,
                         const std::string &explorarFinalOutput, std::string& modeInput,
                         std::string &outResultsMode);

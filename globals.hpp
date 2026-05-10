#pragma once

// globals.hpp debe ser el PRIMER include en todo .cpp que use MySQL + raylib juntos.
// platform/compat.hpp maneja el conflicto de símbolos entre windows.h y raylib.h.

#include "platform/compat.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <setjmp.h>
#include <memory>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std::string_literals;

// ── Screen enum ─────────────────────────────────────────────────────────────
typedef enum Screen { LOGO = 0, CONFIGURATION, ADMINMENU, MAINMENU, CONFIRMATION, VOTATION, ENDING } Screen;

// ── Configuration ────────────────────────────────────────────────────────────
extern std::vector<char*> configurations;
extern char** admPassword;
extern char** password;
extern char** labName;
extern char** database;
extern char** server;
extern char** user;
extern char** port;
extern char** nameColumnPartidosNombre;
extern char** nameColumnNuloPartido;
extern char** nameColumnVotosNombre;
extern char** nameColumnVotoNombre;
extern char** nameTableEstudiantes;
extern char** nameTablePartidos;
extern char** pathProgramFont;
extern char** pathLogsFont;
extern char** pathPdfFont;
extern char** informeName;

// ── MySQL ────────────────────────────────────────────────────────────────────
extern MYSQL_RES* res;
extern MYSQL_ROW  row;
extern MYSQL*     auth;
extern MYSQL*     conn;

// ── Colors ───────────────────────────────────────────────────────────────────
extern Color DORADO_BORDE;
extern Color VOCAVERDE;
extern Color VOCAVERDESUAVE;
extern Color VOCAAMARILLO;
extern Color VOCAAMARILLOSUAVE;
extern Color VOCADORADO;
extern Color VOCADORADOSUAVE;
extern Color NEGRO;
extern Color GRIS;
extern Color BLANCO;
extern Color COLORTEXTO;

// ── File / query state ───────────────────────────────────────────────────────
extern std::ofstream configFile;
extern std::vector<std::vector<std::string>> vecRows;
extern std::vector<std::string> logCommands;
extern std::vector<std::string> vecColumns;
extern std::vector<std::string> namepartidos;
extern std::vector<std::string> nametables;
extern std::vector<std::string> lastLogs;
extern std::vector<Color*>      colorsVec;
extern std::vector<double>      percentages;
extern std::string oldTableSelected;
extern std::string partidoSelected;
extern std::string backupRetString;
extern std::string configSelected;
extern std::string pdfErrorString;
extern std::string tableSelected;
extern std::string adminSelected;
extern std::string butnames[7];
extern std::string oldSelected;
extern std::string outQuery;
extern std::string oldbar;

// ── Raylib / UI state ────────────────────────────────────────────────────────
extern Font  fontTtf;
extern Font  monoTtf;
extern float explorarSquare[4];
extern float mediumFontSize;
extern float littleFontSize;
extern float configPanel[4];
extern float adminPanel[4];
extern float outSquare[4];
extern float screenHeight;
extern float screenWidth;
extern float fontSize;

extern bool adminAuthenticated;
extern bool backupReleased;
extern bool pdfRandomError;
extern bool pdfFontError;
extern bool successReset;
extern bool alphaIsZero;
extern bool errorReset;
extern bool alphaIsFull;
extern bool tabRestart;
extern bool nullOption;
extern bool darkMode;
extern bool showBeam;
extern bool pdfError;

extern int statusCodeUpdating;
extern int intentosRestantes;
extern int statusCodeConfig;
extern int columnSelected;
extern int framesCounter;
extern int stopBackspace;
extern int maxBarLenght;
extern int quanstudents;
extern int quanpartidos;
extern int quancolumns;
extern int quantables;
extern int stopArrows;
extern int maxLenName;
extern int nlSpacing;
extern int stopCtrls;
extern int inputpos;
extern int opcSize;
extern int logpos;
extern int tabCnt;
extern int value;
extern int beam;
extern int add;

// ── jmp_buf for libhpdf error handling ───────────────────────────────────────
extern jmp_buf env;

// ── UI object vectors and raw pointer aliases ────────────────────────────────
#include "ui/widgets.hpp"

extern std::vector<std::unique_ptr<nxyxys>> adminObj;
extern std::vector<sqlobject*> partidosVec;
extern std::vector<sqlobject*> opcionesAct;
extern std::vector<button*>    configbuttons;
extern std::vector<sqlobject*> tablesVec;
extern std::vector<inputBar*>  termBars;
extern std::vector<inputBar*>  extraBars;
extern std::vector<inputBar*>  pathBars;
extern std::vector<column*>    columnsVec;
extern std::vector<button*>    adminButtons;

extern sqlobject* opcSelectedPtr;
extern sqlobject* sinVotarPtr;
extern inputBar*  barAdminTerminalPtr;
extern inputBar*  admPasswordBarPtr;
extern inputBar*  adminTerminalPtr;
extern inputBar*  terminalBarPtr;
extern inputBar*  labNameBarPtr;
extern inputBar*  cedulaBarPtr;
extern inputBar*  actBarPtr;
extern button*    cambiarFrontendPtr;
extern button*    enterConfigPtr;
extern button*    saveConfigPtr;
extern button*    resetDataPtr;
extern button*    continuarPtr;
extern button*    opcionActPtr;
extern button*    resTogglePtr;
extern button*    exitAdminPtr;
extern button*    regresarPtr;
extern button*    informePtr;
extern button*    refreshPtr;
extern button*    backupPtr;
extern button*    cedulaPtr;
extern button*    votarPtr;

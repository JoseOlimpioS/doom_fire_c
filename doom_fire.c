/**
 * @file doom_fire.c
 *
 * @brief Translation of doom fire algorith from javascript to ANSI C
 *
 * Compilation: gcc -o doom_fire.out doom_fire.c -Wall -Wextra -ansi -pedantic -D_XOPEN_SOURCE=501
 *
 * @author Jose Eduardo Olimpio Silva <joseeduardoolimpios@gmail.com>
 * @author Gustavo Bacagine <gustavo.bacagine@protonmail.com>
 *
 * TODO: Fix refresh on terminal fire renderization
 *
 * @date Feb 2026
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>
#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>

  HANDLE gpConsole;
  CONSOLE_SCREEN_BUFFER_INFO gstCSBI;

#else
  #include <unistd.h>
  #include <sys/ioctl.h>
#endif

#ifndef _WIN32
typedef enum boolean { FALSE, TRUE } boolean;
#endif

/* Command Line */
#define NO_ARGUMENT no_argument
#define REQUIRED_ARGUMENT required_argument
#define OPTIONAL_ARGUMENT optional_argument

typedef enum ENUM_CMDDATATYPES {
  CMDTYPE_BOOL,
  CMDTYPE_CHAR,
  CMDTYPE_INT,
  CMDTYPE_DOUBLE,
  CMDTYPE_STRING,
  CMDTYPE_NULL
} ENUM_CMDDATATYPES, *PENUM_CMDDATATYPES;

typedef struct STRUCT_COMLINE_OPTIONS {
  const char* kpszLong;
  const char kchShort;
  int has_arg;
  int iDataType;
  const char* kpszArgExemple;
  boolean bSet;
  const char* kpszDefaultData;
  void* pData;
  int iDataSize;
  const char* kpszHelp;
} STRUCT_COMLINE_OPTIONS, *PSTRUCT_COMLINE_OPTIONS;

const char* gkpszShortOpts = "hv";

STRUCT_COMLINE_OPTIONS astCmdOpt[] = {
  { "help", 'h', NO_ARGUMENT, CMDTYPE_NULL, "",
    FALSE, "", NULL, 0,
    "Show this message and exit."
  },
  { "version", 'v', NO_ARGUMENT, CMDTYPE_NULL, "",
    FALSE, "", NULL, 0,
    "Show the version and exit."
  },
  { NULL, 0, NO_ARGUMENT, CMDTYPE_NULL, NULL,
    FALSE, NULL, NULL, 0,
    NULL
  }
};

/**
 * @brief Parse command line
 *
 * @param argc Command line argument counter
 * @param argv Command line argument vector
 * @param astCmdOpt Command line options
 *
 * @return TRUE if OK
 * @return FALSE if have an error
 */
boolean bParseCommandLine(int argc, char* argv[], STRUCT_COMLINE_OPTIONS astCmdOpt[]);
boolean bParseCommandLine(int argc, char* argv[], STRUCT_COMLINE_OPTIONS astCmdOpt[]) {
  int ii = 0;
  int iArraySize = 0;
  struct option* astGnuCmdOpt = NULL;
  int iGnuIdx = 0;
  boolean bRsl = TRUE;
  int opt = 0;

  opterr = FALSE;

  for ( ii = 0; astCmdOpt[ii].kpszLong; ii++ ) iArraySize++;

  astGnuCmdOpt = (struct option*) malloc(iArraySize * sizeof(struct option));
  if ( !astGnuCmdOpt ) return FALSE;

  for ( ii = 0; astCmdOpt[ii].kpszLong; ii++ ) {
    astGnuCmdOpt[ii].name = astCmdOpt[ii].kpszLong;
    astGnuCmdOpt[ii].val = astCmdOpt[ii].kchShort;
    astGnuCmdOpt[ii].has_arg = astCmdOpt[ii].has_arg;
    astGnuCmdOpt[ii].flag = NULL;
    /* Set default values */
    switch ( astCmdOpt[ii].iDataType ) {
      case CMDTYPE_BOOL:
      case CMDTYPE_INT: {
        *(int*)astCmdOpt[ii].pData = atoi(astCmdOpt[ii].kpszDefaultData);
        break;
      }
      case CMDTYPE_CHAR: {
        *(char*)astCmdOpt[ii].pData = astCmdOpt[ii].kpszDefaultData[0];
        break;
      }
      case CMDTYPE_DOUBLE: {
        *(double*)astCmdOpt[ii].pData = atof(astCmdOpt[ii].kpszDefaultData);
        break;
      }
      case CMDTYPE_STRING: {
        snprintf((char*)astCmdOpt[ii].pData, astCmdOpt[ii].iDataSize, "%s", astCmdOpt[ii].kpszDefaultData);
        break;
      }
      default: break;
    }
  }

  while ( (opt = getopt_long(argc, argv, gkpszShortOpts, astGnuCmdOpt, &iGnuIdx)) != -1 ) {
    if ( opt == '?' || opt == ':' ) {
      bRsl = FALSE;
      break;
    }
    for ( ii = 0; astCmdOpt[ii].kpszLong; ii++ ) {
      if ( !(
        ( opt && opt == astCmdOpt[ii].kchShort) ||
        ( opt == 0 && !strcmp(astGnuCmdOpt[iGnuIdx].name, astCmdOpt[ii].kpszLong))
      ) ) {
        continue;
      }
      astCmdOpt[ii].bSet = TRUE;
      if ( astCmdOpt[ii].has_arg == REQUIRED_ARGUMENT ) {
        if ( !optarg ) {
          bRsl = FALSE;
          break;
        }
        switch ( astCmdOpt[ii].iDataType ) {
          case CMDTYPE_BOOL:
          case CMDTYPE_INT: {
            *(int*)astCmdOpt[ii].pData = atoi(optarg);
            break;
          }
          case CMDTYPE_CHAR: {
            *(char*)astCmdOpt[ii].pData = optarg[0];
            break;
          }
          case CMDTYPE_DOUBLE: {
            *(double*)astCmdOpt[ii].pData = atof(optarg);
            break;
          }
          case CMDTYPE_STRING: {
            snprintf((char*)astCmdOpt[ii].pData, astCmdOpt[ii].iDataSize, "%s", optarg);
            break;
          }
          default: {
            break;
          }
        }
      }
    }
    if ( !bRsl ) break;
  }

  free(astGnuCmdOpt);
  astGnuCmdOpt = NULL;

  return bRsl;
}

/**
 * @var gkpszArgv0
 * @brief Receive the content of argv[0]
 */
const char* gkpszArgv0 = NULL;

/**
 * @var gkpszProgramName
 * @brief Receive the name of binary
 */
const char* gkpszProgramName = NULL;

/**
 * @var giFireWidth
 * @brief ...
 */
int giFireWidth = 0;

/**
 * @var giFireHeight
 * @brief ...
 */
int giFireHeight = 0;

/**
 * @struct STRUCT_COLOR
 * @brief ...
 */
typedef struct STRUCT_COLOR {
  int* aiData;
} STRUCT_COLOR;

typedef STRUCT_COLOR STRUCT_COLOUR;

/**
 * @struct STRUCT_PIXEL
 * @brief ...
 */
typedef struct STRUCT_PIXEL {
  unsigned char uchR; /*< RED   */
  unsigned char uchG; /*< GREEN */
  unsigned char uchB; /*< BLUE  */
} STRUCT_PIXEL;

/**
 * @var gastFirePal
 * @brief ...
 */
STRUCT_PIXEL gastFirePal[37];

/**
 * @var gaiFirePixels
 * @brief ...
 */
int* gaiFirePixels;

/**
 * @var gaiFireBuffer
 * @brief ...
 */
int* gaiFireBuffer;

/**
 * @var gstColor
 * @brief ...
 */
STRUCT_COLOR gstColor;

/**
 * @var gkaiFireRGB
 * @brief ...
 */
static const int gkaiFireRGB[] = {
  0x07,0x07,0x07,0x1F,0x07,0x07,0x2F,0x0F,0x07,0x47,0x0F,0x07,0x57,0x17,0x07,0x67,
  0x1F,0x07,0x77,0x1F,0x07,0x8F,0x27,0x07,0x9F,0x2F,0x07,0xAF,0x3F,0x07,0xBF,0x47,
  0x07,0xC7,0x47,0x07,0xDF,0x4F,0x07,0xDF,0x57,0x07,0xDF,0x57,0x07,0xD7,0x5F,0x07,
  0xD7,0x5F,0x07,0xD7,0x67,0x0F,0xCF,0x6F,0x0F,0xCF,0x77,0x0F,0xCF,0x7F,0x0F,0xCF,
  0x87,0x17,0xC7,0x87,0x17,0xC7,0x8F,0x17,0xC7,0x97,0x1F,0xBF,0x9F,0x1F,0xBF,0x9F,
  0x1F,0xBF,0xA7,0x27,0xBF,0xA7,0x27,0xBF,0xAF,0x2F,0xB7,0xAF,0x2F,0xB7,0xB7,0x2F,
  0xB7,0xB7,0x37,0xCF,0xCF,0x6F,0xDF,0xDF,0x9F,0xEF,0xEF,0xC7,0xFF,0xFF,0xFF
};

/**
 * @brief ...
 *
 * @param iX p_iX:...
 * @param iY p_iY:...
 * @param stPixel p_stPixel:...
 */
void vDrawPixel(int iX, int iY, STRUCT_PIXEL stPixel);

/**
 * @brief ...
 *
 */
void vStart(void);

/**
 * @brief ...
 *
 */
void vEnd(void);

/**
 * @brief ...
 *
 * @param iPixel p_iPixel:...
 * @param iCurSrc p_iCurSrc:...
 * @param iCount p_iCount:...
 * @param iSrcOffset p_iSrcOffset:...
 * @param iRand p_iRand:...
 * @param iWidth p_iWidth:...
 * @return int
 */
int iSpreadFire(int iPixel, int iCurSrc, int iCount, int iSrcOffset, int iRand, int iWidth);

/**
 * @brief ...
 *
 */
void vDoFire(void);

/**
 * @brief ...
 *
 */
void vTick(void);

/* ============================================= */
/* WINDOWS-SPECIFIC FUNCTIONS (INSERTED HERE)    */
/* ============================================= */
#ifdef _WIN32
void vClearScreen(void) {
    COORD stCoord = {0, 0};
    DWORD uiWritten;
    FillConsoleOutputCharacter(gpConsole, ' ', 
        gstCSBI.dwSize.X * gstCSBI.dwSize.Y, stCoord, &uiWritten);
    FillConsoleOutputAttribute(gpConsole, gstCSBI.wAttributes,
        gstCSBI.dwSize.X * gstCSBI.dwSize.Y, stCoord, &uiWritten);
    SetConsoleCursorPosition(gpConsole, stCoord);
}

void vHideCursor(void) {
    CONSOLE_CURSOR_INFO stCursorInfo;
    GetConsoleCursorInfo(gpConsole, &stCursorInfo);
    stCursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(gpConsole, &stCursorInfo);
}

void vSetColor(int iR, int iG, int iB) {
    // Windows Console has limited colors, map RGB to nearest console color
    int iColor = 0;
    if (iR > 128) iColor |= FOREGROUND_RED;
    if (iG > 128) iColor |= FOREGROUND_GREEN;
    if (iB > 128) iColor |= FOREGROUND_BLUE;
    if (iR > 128 && iG > 128 && iB > 128) iColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    SetConsoleTextAttribute(gpConsole, iColor);
}

void vDrawPixelWin(int iX, int iY, STRUCT_PIXEL stPixel) {
    COORD stCoord = {iX, iY};
    SetConsoleCursorPosition(gpConsole, stCoord);
    vSetColor(stPixel.uchR, stPixel.uchG, stPixel.uchB);
    printf("█");
}
#endif
/* ============================================= */

void vDrawPixel(int iX, int iY, STRUCT_PIXEL stPixel) {
  if (iX < 0 || iX >= giFireWidth || iY < 0 || iY >= giFireHeight) return;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 0] = stPixel.uchR;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 1] = stPixel.uchG;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 2] = stPixel.uchB;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 3] = 255;
}

void vStart(void) {
  int ii = 0;
  #ifdef _WIN32
    /* Initialize Windows console handle */
    gpConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(gpConsole, &gstCSBI);
    giFireWidth = gstCSBI.srWindow.Right - gstCSBI.srWindow.Left + 1;
    giFireHeight = gstCSBI.srWindow.Bottom - gstCSBI.srWindow.Top + 1;
  #else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    giFireWidth = w.ws_col;
    giFireHeight = w.ws_row;
  #endif
  gstColor.aiData = (int*)malloc(sizeof(int) * giFireWidth * giFireHeight * 4);
  if ( !gstColor.aiData ) {
    fprintf(stderr, "Erro ao alocar gstColor.aiData!");
    exit(EXIT_FAILURE);
  }
  gaiFirePixels = (int*)malloc(sizeof(int) * giFireWidth * giFireHeight);
  if ( !gaiFirePixels ) {
    fprintf(stderr, "Erro ao alocar gaiFirePixels!");
    exit(EXIT_FAILURE);
  }
  gaiFireBuffer = (int*)malloc(sizeof(int) * giFireWidth * giFireHeight);
  if ( !gaiFireBuffer ) {
    fprintf(stderr, "Erro ao alocar gaiFireBuffer!");
    exit(EXIT_FAILURE);
  }
  for ( ii = 0; ii < 37; ii++ ) {
    gastFirePal[ii].uchR = gkaiFireRGB[ii * 3 + 0]; /* 16 * i, */
    gastFirePal[ii].uchG = gkaiFireRGB[ii * 3 + 1]; /* 16 * i, */
    gastFirePal[ii].uchB = gkaiFireRGB[ii * 3 + 2]; /* 16 * i  */
  }
  for ( ii = 0; ii < giFireWidth*giFireHeight; ii++ ) {
    gaiFirePixels[ii] = 0;
  }
  for ( ii = 0; ii < giFireWidth; ii++ ) {
    gaiFirePixels[(giFireHeight-1)*giFireWidth + ii] = 26;
  }
}

void vEnd(void) {
  if ( gstColor.aiData ) {
    free(gstColor.aiData);
    gstColor.aiData = NULL;
  }
  if ( gaiFirePixels ) {
    free(gaiFirePixels);
    gaiFirePixels = NULL;
  }
  if ( gaiFireBuffer ) {
    free(gaiFireBuffer);
    gaiFireBuffer = NULL;
  }
}

int iSpreadFire(int iPixel, int iCurSrc, int iCount, int iSrcOffset, int iRand, int iWidth) {
  if( iPixel != 0 ) {
    int iRandIdx = (rand()) % 255;
    int iTempSrc;
    int iShift = (iCount - (iRandIdx & 3)) + 1;
    if ( iShift < 0 ) {
      iShift += iWidth;
    }
    iRand = ((iRand+2) & 255);
    iTempSrc = iCurSrc + (iShift % iWidth);
    gaiFirePixels[iTempSrc - iWidth] = iPixel - ((iRandIdx & 2));
  }
  else {
    gaiFirePixels[iSrcOffset - iWidth] = 0;
  }
  return iRand;
}

void vDoFire(void) {
  int  iCounter = 0;
  int  iCurSrc = 0;
  int  iSrcOffset = 0;
  int  iRand = 0;
  int  iStep = 0;
  int  iPixel = 0;

  iRand = (rand()) % 255;
  iCurSrc = giFireWidth;

  do {
    iSrcOffset = (iCurSrc + iCounter);
    iPixel = gaiFirePixels[iSrcOffset];
    iStep = 2;

    iRand = iSpreadFire(iPixel, iCurSrc, iCounter, iSrcOffset, iRand, giFireWidth);

    iCurSrc += giFireWidth;
    iSrcOffset += giFireWidth;

    do {
      iPixel = gaiFirePixels[iSrcOffset];
      iStep += 2;

      iRand = iSpreadFire(iPixel, iCurSrc, iCounter, iSrcOffset, iRand, giFireWidth);

      if ((iSrcOffset + giFireWidth) >= (giFireWidth * giFireHeight)) break;

      iPixel = gaiFirePixels[iSrcOffset + giFireWidth];
      iCurSrc += giFireWidth;
      iSrcOffset += giFireWidth;

      iRand = iSpreadFire(iPixel, iCurSrc, iCounter, iSrcOffset, iRand, giFireWidth);

      iCurSrc += giFireWidth;
      iSrcOffset += giFireWidth;
    } while(iStep < giFireHeight);

    iCounter++;
    iCurSrc -= ((giFireWidth*giFireHeight)-giFireWidth);
  } while(iCounter < giFireWidth);
}

void vTick(void) {
  int h = 0;
  int w = 0;

  #ifdef _WIN32
    /* Clear screen and move cursor to top using Windows API */
    vClearScreen();
  #else
    printf("\033[H"); /* Move cursor para topo */
    printf("\033[J");   /* limpa da posição atual até o final */
  #endif
  
  vDoFire();
  for ( h = 0; h < giFireHeight; h++ ) {
    for ( w = 0; w < giFireWidth; w++ ) {
      int iRIdx = 0;
      int iGIdx = 0;
      int iBIdx = 0;
      int p = gaiFirePixels[h * giFireWidth + w];

    #ifdef _WIN32
      /* Use Windows-specific drawing function */
      vDrawPixelWin(w, h, gastFirePal[p]);
    #else
      iRIdx = ((giFireWidth * h) + w) * 4 + 0;
      iGIdx = ((giFireWidth * h) + w) * 4 + 1;
      iBIdx = ((giFireWidth * h) + w) * 4 + 2;
      vDrawPixel(w, h, gastFirePal[p]);
      printf(
        "\033[38;2;%d;%d;%dm█",
        gstColor.aiData[iRIdx],
        gstColor.aiData[iGIdx],
        gstColor.aiData[iBIdx]
      );
    #endif
    }

    #ifdef _WIN32
      printf("\n");
    #else
     printf("\033[0m\n");
    #endif
  }

  #ifndef _WIN32
    printf("\033[0m");
  #endif

  fflush(stdout);
}

static void vShowUsage(void);
static void vShowUsage(void) {
  int ii = 0;
  printf("Usage: %s [options]\n", gkpszProgramName ? gkpszProgramName : "doom_fire");
  printf("\nOptions:\n");
  for ( ii = 0; astCmdOpt[ii].kpszLong; ii++ ) {
    const char* pszArgExample = NULL;
    int iHasExample = 0;
    if (!astCmdOpt[ii].kpszLong) break;
    pszArgExample = astCmdOpt[ii].kpszArgExemple;
    iHasExample = (pszArgExample && pszArgExample[0] != '\0');
    if ( iHasExample ) {
      printf("  --%s=%s", astCmdOpt[ii].kpszLong, pszArgExample);
    }
    else {
      printf("  --%s", astCmdOpt[ii].kpszLong);
    }
    if ( astCmdOpt[ii].kchShort != 0 ) {
      if ( iHasExample ) {
        printf(", -%c%s", astCmdOpt[ii].kchShort, pszArgExample);
      }
      else {
        printf(", -%c", astCmdOpt[ii].kchShort);
      }
    }
    printf("\n    %s\n", astCmdOpt[ii].kpszHelp);
    if ( astCmdOpt[ii+1].kpszLong ) {
      printf("\n");
    }
  }
}

static void vShowVersion(void);
static void vShowVersion(void) {
  printf("%s\n", gkpszProgramName);
  printf("Jose Eduardo & Gustavo Bacagine (C) 2026 [%s %s]\n", __DATE__, __TIME__);
}

int main(int argc, char* argv[]) {
  memset(gastFirePal  , 0x00, sizeof(gastFirePal  ));
  memset(&gstColor    , 0x00, sizeof(gstColor     ));

  gkpszArgv0 = argv[0];
  gkpszProgramName = basename(argv[0]);

  if ( !bParseCommandLine(argc, argv, astCmdOpt) ) {
    return -1;
  }

  if ( astCmdOpt[0].bSet ) {
    vShowUsage();
    return 0;
  }

  if ( astCmdOpt[1].bSet ) {
    vShowVersion();
    return 0;
  }
  
  srand(time(NULL));
#ifdef _WIN32
  /* Initialize Windows console and hide cursor */
    gpConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    vHideCursor();
    vClearScreen();
#else
  printf("\033[2J");  /* limpa tela */
  printf("\033[?25l"); /* esconde cursor */
#endif
  vStart();
  while ( 1 ) {
    vTick();
#ifdef _WIN32
    Sleep(30000/1000);
#else
    usleep(30000);
#endif
  }
  vEnd();

  return 0;
}


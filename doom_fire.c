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
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

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
 *
 * @brief ...
 */
typedef struct STRUCT_PIXEL {
  int iR; /*< RED   */
  int iG; /*< GREEN */
  int iB; /*< BLUE  */
} STRUCT_PIXEL;

/**
 * @brief ...
 *
 */
STRUCT_PIXEL gastFirePal[37];

/**
 * @brief ...
 *
 */
int* gaiFirePixels;

/**
 * @brief ...
 *
 */
int* gaiFireBuffer;

/**
 * @brief ...
 *
 */
STRUCT_COLOR gstColor;

/**
 * @brief ...
 *
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

void vDrawPixel(int iX, int iY, STRUCT_PIXEL stPixel) {
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 0] = stPixel.iR;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 1] = stPixel.iG;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 2] = stPixel.iB;
  gstColor.aiData[((giFireWidth * iY) + iX) * 4 + 3] = 255;
}

void vStart(void) {
  int ii = 0;
  struct winsize w;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  giFireWidth = w.ws_row;
  giFireHeight = w.ws_col;

  gstColor.aiData = (int*)malloc(sizeof(int) * giFireWidth * giFireHeight * 4);
  gaiFirePixels = (int*)malloc(sizeof(int) * giFireWidth * giFireHeight);
  gaiFireBuffer = (int*)malloc(sizeof(int) * giFireWidth * giFireHeight);

  for ( ii = 0; ii < 37; ii++ ) {
    gastFirePal[ii].iR = gkaiFireRGB[ii * 3 + 0]; /* 16 * i, */
    gastFirePal[ii].iG = gkaiFireRGB[ii * 3 + 1]; /* 16 * i, */
    gastFirePal[ii].iB = gkaiFireRGB[ii * 3 + 2]; /* 16 * i  */
  }

  for ( ii = 0; ii < giFireWidth*giFireHeight; ii++ ) {
    gaiFirePixels[ii] = 0;
  }

  for ( ii = 0; ii < giFireWidth; ii++ ) {
    gaiFirePixels[(giFireHeight-1)*giFireWidth + ii] = 36;
  }
}

void vEnd(void) {
  free(gstColor.aiData);
  free(gaiFirePixels);
  free(gaiFireBuffer);
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
    gaiFirePixels[iTempSrc - iWidth] = iPixel - ((iRandIdx & 1));
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
  printf("\033[H"); /* Move cursor para topo */
  printf("\033[J");   /* limpa da posição atual até o final */
  vDoFire();
  for ( h = 0; h < giFireHeight; h++ ) {
    for ( w = 0; w < giFireWidth; w++ ) {
      int iRIdx = 0;
      int iGIdx = 0;
      int iBIdx = 0;
      int p = gaiFirePixels[h * giFireWidth + w];
      vDrawPixel(w, h, gastFirePal[p]);
      iRIdx = ((giFireWidth * h) + w) * 4 + 0;
      iGIdx = ((giFireWidth * h) + w) * 4 + 1;
      iBIdx = ((giFireWidth * h) + w) * 4 + 2;
      printf(
        "\033[38;2;%d;%d;%dm█",
        gstColor.aiData[iRIdx],
        gstColor.aiData[iGIdx],
        gstColor.aiData[iBIdx]
      );
    }
    printf("\033[0m\n");
  }
  printf("\033[0m");
}

int main(void) {
  memset(gastFirePal  , 0x00, sizeof(gastFirePal  ));
  memset(&gstColor    , 0x00, sizeof(gstColor     ));
  srand(time(NULL));
  printf("\033[2J");  /* limpa tela */
  printf("\033[?25l"); /* esconde cursor */
  vStart();
  while ( 1 ) {
    vTick();
    usleep(30000);
  }
  vEnd();
  return 0;
}


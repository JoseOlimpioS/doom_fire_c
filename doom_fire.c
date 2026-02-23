#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>


#define FIRE_WIDTH 64
#define FIRE_HEIGHT 128

typedef struct STRUCT_COLOR {
  int aiData[FIRE_WIDTH * FIRE_HEIGHT * 4]; //chatgpt suggested to dynamically allocated a larger buffer. Before was 32
} STRUCT_COLOR;

typedef STRUCT_COLOR STRUCT_COLOUR;

typedef struct STRUCT_PIXEL {
  int iR;
  int iG;
  int iB;
} STRUCT_PIXEL;


//var stage;
STRUCT_PIXEL gastFirePal[37];
int gaiFirePixels[FIRE_WIDTH * FIRE_HEIGHT];
int gaiFireBuffer[FIRE_WIDTH * FIRE_HEIGHT];
// var container = null;
STRUCT_COLOR gstColor;
//var canvas;
// var bmp;

int gaiFireRGB[] = {
  0x07,0x07,0x07,0x1F,0x07,0x07,0x2F,0x0F,0x07,0x47,0x0F,0x07,0x57,0x17,0x07,0x67,
  0x1F,0x07,0x77,0x1F,0x07,0x8F,0x27,0x07,0x9F,0x2F,0x07,0xAF,0x3F,0x07,0xBF,0x47,
  0x07,0xC7,0x47,0x07,0xDF,0x4F,0x07,0xDF,0x57,0x07,0xDF,0x57,0x07,0xD7,0x5F,0x07,
  0xD7,0x5F,0x07,0xD7,0x67,0x0F,0xCF,0x6F,0x0F,0xCF,0x77,0x0F,0xCF,0x7F,0x0F,0xCF,
  0x87,0x17,0xC7,0x87,0x17,0xC7,0x8F,0x17,0xC7,0x97,0x1F,0xBF,0x9F,0x1F,0xBF,0x9F,
  0x1F,0xBF,0xA7,0x27,0xBF,0xA7,0x27,0xBF,0xAF,0x2F,0xB7,0xAF,0x2F,0xB7,0xB7,0x2F,
  0xB7,0xB7,0x37,0xCF,0xCF,0x6F,0xDF,0xDF,0x9F,0xEF,0xEF,0xC7,0xFF,0xFF,0xFF
};

void vDrawPixel(int iX,int iY,STRUCT_PIXEL stPixel) {
    gstColor.aiData[((FIRE_WIDTH * iY) + iX) * 4 + 0] = stPixel.iR;
    gstColor.aiData[((FIRE_WIDTH * iY) + iX) * 4 + 1] = stPixel.iG;
    gstColor.aiData[((FIRE_WIDTH * iY) + iX) * 4 + 2] = stPixel.iB;
    gstColor.aiData[((FIRE_WIDTH * iY) + iX) * 4 + 3] = 255;
}

void  vStart() {
//  document.body.style.backgroundColor="#000000";
//  stage = new createjs.Stage("mainCanvas");

//  createjs.Ticker.addEventListener("tick", tick);
//  createjs.Ticker.setFPS(CJS_TICKER_FPS);

//  container = new createjs.Container();

  for (int ii = 0; ii < 37; ii++) {
    gastFirePal[ii].iR = gaiFireRGB[ii * 3 + 0]; //16 * i,
    gastFirePal[ii].iG = gaiFireRGB[ii * 3 + 1]; //16 * i,
    gastFirePal[ii].iB = gaiFireRGB[ii * 3 + 2];  //16 * i
  }

//  stage.addChild(container);

  for (int ii = 0; ii < FIRE_WIDTH*FIRE_HEIGHT; ii++) {
    gaiFirePixels[ii] = 0;
  }

  for(int ii = 0; ii < FIRE_WIDTH; ii++) {
    gaiFirePixels[(FIRE_HEIGHT-1)*FIRE_WIDTH + ii] = 36;
  }

//  container.scaleX = 2;
//  container.scaleY = 2;

//  canvas = document.createElement("canvas");
//  bmp = new createjs.Bitmap(canvas);

//  container.addChild(bmp);
//  stage.update();
}

int iSpreadFire(int iPixel,int iCurSrc,int iCount,int iSrcOffset,int iRand,int iWidth) {
  srand(time(NULL));
  if(iPixel != 0) {
    int iRandIdx = (rand() % 255) & 255;
    int iTempSrc;

    iRand = ((iRand+2) & 255);
    iTempSrc = (iCurSrc + (((iCount - (iRandIdx & 3)) + 1) & (FIRE_WIDTH - 1)));
    gaiFirePixels[iTempSrc - FIRE_WIDTH] = iPixel - ((iRandIdx & 1));
  }
  else {
    gaiFirePixels[iSrcOffset - FIRE_WIDTH] = 0;
  }

  return iRand;
}

void vDoFire() {
  int  iCounter = 0;
  int  iCurSrc = 0;
  int  iSrcOffset = 0;
  int  iRand = 0;
  int  iStep = 0;
  int  iPixel = 0;
  int  ii = 0;

  iRand = (rand() * 255) & 255;
  iCurSrc = FIRE_WIDTH;

  do {
    iSrcOffset = (iCurSrc + iCounter);
    iPixel = gaiFirePixels[iSrcOffset];
    iStep = 2;

    iRand = iSpreadFire(iPixel, iCurSrc, iCounter, iSrcOffset, iRand, FIRE_WIDTH);

    iCurSrc += FIRE_WIDTH;
    iSrcOffset += FIRE_WIDTH;

    do {
      iPixel = gaiFirePixels[iSrcOffset];
      iStep += 2;

      iRand = iSpreadFire(iPixel, iCurSrc, iCounter, iSrcOffset, iRand, FIRE_WIDTH);

      iPixel = gaiFirePixels[iSrcOffset + FIRE_WIDTH];
      iCurSrc += FIRE_WIDTH;
      iSrcOffset += FIRE_WIDTH;

      iRand = iSpreadFire(iPixel, iCurSrc, iCounter, iSrcOffset, iRand, FIRE_WIDTH);

      iCurSrc += FIRE_WIDTH;
      iSrcOffset += FIRE_WIDTH;

    } while(iStep < FIRE_HEIGHT);

    iCounter++;
    iCurSrc -= ((FIRE_WIDTH*FIRE_HEIGHT)-FIRE_WIDTH);

  } while(iCounter < FIRE_WIDTH);
}
//TODO rever assim que possível


/*function iTick(event) {
    doFire();

    color = canvas.getContext("2d").getImageData(0, 0, FIRE_WIDTH, FIRE_HEIGHT);

    for(var h = 0; h < FIRE_HEIGHT; h++) {
        for(var w = 0; w < FIRE_WIDTH; w++) {
            var p = firePixels[h * FIRE_WIDTH + w];
            drawPixel(w, h, firePal[p]);
        }
    }

    canvas.getContext("2d").putImageData(color, 0, 0);
    stage.update();
}*/ 

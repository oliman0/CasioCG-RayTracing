#ifndef GL_H
#define GL_H

#include <fxcg/display.h>
#include "./fpmath.h"

unsigned short colourFromDec(vec3 col);

void setPixel(unsigned x,unsigned y,unsigned short col);

unsigned short getPixel(unsigned x,unsigned y);

vec3 getPixelAsVec(unsigned x,unsigned y);

void clearBuffer();

#endif
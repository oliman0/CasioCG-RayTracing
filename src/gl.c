#include "./gl.h"

unsigned short colourFromDec(vec3 col) {
    int ri = (int)(FIXTOF(col.x) * 31);
    int gi = (int)(FIXTOF(col.y) * 63);
    int bi = (int)(FIXTOF(col.z) * 31);

    return ((ri << 11) | (gi << 5) | bi);
}

void setPixel(unsigned x,unsigned y,unsigned short col){
    unsigned short*s=(unsigned short*)GetVRAMAddress();
    s+=(y*384)+x;
    *s=col;
}

unsigned short getPixel(unsigned x,unsigned y){
    unsigned short*s=(unsigned short*)GetVRAMAddress();
    s+=(y*384)+x;
    return *s;
}

vec3 getPixelAsVec(unsigned x,unsigned y){
    unsigned short*s=(unsigned short*)GetVRAMAddress();
    s+=(y*384)+x;
    unsigned short col = *s;

    vec3 out = (vec3){0, 0, 0};
    out.x = ITOFIX(col >> 11);
    out.y = ITOFIX((col >> 5) - (out.x << 6));
    out.z = ITOFIX(col - ((out.x << 11) | (out.y << 5)));

    return out;
}

void clearBuffer() {
    unsigned short*p=(unsigned short*)GetVRAMAddress() - 1;
    int i;
    for (i = 0; i < LCD_HEIGHT_PX * LCD_WIDTH_PX; i++) {
        *p++ = 0xFFFF;
    }
}
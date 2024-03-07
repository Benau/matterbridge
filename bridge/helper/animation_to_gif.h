#ifndef ANIMATION_TO_GIF_H
#define ANIMATION_TO_GIF_H
#include <stdint.h>
#include <stdlib.h>

typedef int (*writeCallback)(size_t, const uint8_t*, void*);
int animationToGif(unsigned char *videoData, int videoSize, writeCallback cb);

#endif

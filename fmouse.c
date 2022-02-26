#include "unistd.h"
#define CNFG_IMPLEMENTATION
#include "monospace_pixels.h"
#include "os_generic.h"
#include "permutations.h"
#include "rawdraw_sf.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#define KEYSYM(name) XStringToKeysym(name)
#define ATLEAST_ZERO(x) x > 0 ? x : 0

short screenX, screenY, screenId = -1, gridOffsetX = 0, gridOffsetY = 0,
                        gridResPxRatio = 32, gridParts, lineThickness = 2,
                        drawGrid = 1, maxGridX, maxGridY, fontScale = -1,
                        mouseButton = 1;
char choice[3] = "  ";

void MouseClick() {
  if (!CNFGDisplay)
    CNFGDisplay = XOpenDisplay(NULL);
  XTestFakeButtonEvent(CNFGDisplay, mouseButton, false, CurrentTime);
  XFlush(CNFGDisplay);
  usleep(1000 * 10);
  XTestFakeButtonEvent(CNFGDisplay, mouseButton, true, CurrentTime);
  XFlush(CNFGDisplay);
  usleep(1000 * 10);
  XTestFakeButtonEvent(CNFGDisplay, mouseButton, false, CurrentTime);
  XFlush(CNFGDisplay);
}

void DrawCharacter(short xOffset, short yOffset, char ch, short scale) {
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      if (MonospacePixels[8 * 8 * ch + y * 8 + x])
        CNFGTackRectangle(xOffset + (x * scale), yOffset + (y * scale),
                          xOffset + (x * scale) + scale,
                          yOffset + (y * scale) + scale);
    }
  }
}

void DrawCharacterOverlaying(short xOffset, short yOffset, char ch, short scale,
                             uint32_t color) {
  CNFGDrawToTransparencyMode(1);
  DrawCharacter(xOffset, yOffset, ch, scale);
  CNFGDrawToTransparencyMode(0);
  short moreOffset = 0;
  short scaleOffset = 0;
  if (scale == 8) {
    CNFGColor(0x000000ff);
    DrawCharacter(xOffset, yOffset, ch, scale);
    moreOffset = 2;
    scaleOffset = -1;
  }
  CNFGColor(color);
  DrawCharacter(xOffset + moreOffset, yOffset + moreOffset, ch,
                scale + scaleOffset);
}

void DrawStringOverlaying(short xOffset, short yOffset, char *str, short scale,
                          uint32_t colors[]) {
  int offsetToCenter = (strlen(str) * 8 * scale) / 2;
  int idx = 0;
  while (*str != '\0') {
    DrawCharacterOverlaying(ATLEAST_ZERO(xOffset - offsetToCenter),
                            ATLEAST_ZERO(yOffset - (offsetToCenter * 0.5)),
                            *str, scale, colors[idx]);
    xOffset += scale * 8;
    str++;
    idx++;
  }
}

int main(int argc, char** argv) {
  while (argc > 1) {
    char *arg = argv[--argc];
    if (strcmp("--right-click", arg) == 0 || strcmp("-r", arg) == 0) {
      mouseButton = 3;
    }
    if (strcmp("--help", arg) == 0 || strcmp("-h", arg) == 0) {
      printf("Usage: fmouse [options]\n");
      printf("Options:\n");
      printf("\t--right-click, -r\tSimulate a right click instead of the default left\n");
      printf("\t--help, -h\t\t\tShow this help message\n");
      return 0;
    }
  }
  CNFGBGColor = 0xffffffff;
  CNFGPrepareForTransparency();
  CNFGSetupFullscreen("fmouse", screenId);
  while (1) {
    CNFGHandleInput();
    CNFGForceInputFocus();

    CNFGClearFrame();
    CNFGGetDimensions(&screenX, &screenY);
    if (!maxGridX || !maxGridY) {
      maxGridX = screenX;
      maxGridY = screenY;
    }
    CNFGClearTransparencyLevel();
    CNFGDrawToTransparencyMode(1);
    int xPart = screenX / gridResPxRatio;
    int yPart = screenX / gridResPxRatio;
    if (fontScale == -1) {
      fontScale = xPart / (8 * 1.8);
    }
    if (fontScale <= 0) {
      CNFGTearDown();
      MouseClick();
      exit(0);
    }
    if (drawGrid) {
      int lastX, lastY;
      for (int x = gridOffsetX + xPart; x < maxGridX; x += xPart) {
        CNFGTackRectangle(x - (lineThickness / 2), 0, x + (lineThickness / 2),
                          screenY);
      }
      for (int y = gridOffsetY + yPart; y < maxGridY; y += yPart) {
        CNFGTackRectangle(0, y - (lineThickness / 2), screenX,
                          y + (lineThickness / 2));
      }
      char stopGridContents = 0;
      int permutationIdx = 0;
      for (int x = gridOffsetX; x < maxGridX; x += xPart) {
        for (int y = gridOffsetY; y < maxGridY; y += yPart) {
          if (!stopGridContents) {
            uint32_t colors[] = {0xe65100ff, 0xe65100ff};
            char *name = permutations[permutationIdx++];
            if (*choice == *name) {
              colors[0] = 0xffeb3bff;
            }
            if (strncmp(choice, name, 2) == 0) {
              XWarpPointer(CNFGDisplay, None, CNFGWindow, 0, 0, 0, 0,
                           x + xPart / 2, y + yPart / 2);
              XFlush(CNFGDisplay);
              gridOffsetX = x;
              gridOffsetY = y;
              maxGridX = x + xPart;
              maxGridY = y + yPart;
              gridResPxRatio *= 4;
              fontScale = -1;
              choice[0] = ' ';
              choice[1] = ' ';
              stopGridContents = 1;
            }
            if (*choice == *name || *choice == ' ') {
              DrawStringOverlaying(x + xPart / 2, y + yPart / 2, name,
                                   fontScale, colors);
            }
          }
        }
      }
    } else {
      CNFGTackRectangle(gridOffsetX, gridOffsetY, gridOffsetX + xPart,
                        gridOffsetY + yPart);
    }

    CNFGDrawToTransparencyMode(0);
    CNFGSwapBuffers();
  }

  return 0;
}

void HandleKey(int keycode, int down) {
  if (down && keycode == KEYSYM("Escape")) {
    CNFGTearDown();
    exit(0);
  } else if (down && keycode <= 122 && keycode >= 97) {
    if (*choice != ' ')
      choice[1] = keycode;
    else
      *choice = keycode;
  } else if (!down && keycode == ' ') {
    CNFGTearDown();
    usleep(10000);
    MouseClick();
    exit(0);
  }
}
void HandleButton(int x, int y, int button, int bDown) {}
void HandleMotion(int x, int y, int mask) {}
void HandleDestroy() {}

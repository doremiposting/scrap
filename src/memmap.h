#ifndef MEMMAP_H
#define MEMMAP_H

#include <stdint.h>

#define VM_MEMMAP_VERSION 1

/* Engine -> Game Region */
#define VM_ADDR_VERSION 0x0000
#define VM_ADDR_DELTA_MS 0x0004
#define VM_ADDR_INPUT 0x0008
#define VM_ADDR_MOUSE_X 0x000C
#define VM_ADDR_MOUSE_Y 0x0010
#define VM_ADDR_SCREEN_W 0x0014
#define VM_ADDR_SCREEN_H 0x0018
/* 0x001C thru 0x00FF reserved */

#define VM_INPUT_UP (1 << 0)
#define VM_INPUT_DOWN (1 << 1)
#define VM_INPUT_LEFT (1 << 2)
#define VM_INPUT_RIGHT (1 << 3)
#define VM_INPUT_A (1 << 4)
#define VM_INPUT_B (1 << 5)
#define VM_INPUT_START (1 << 6)
#define VM_INPUT_SELECT (1 << 7)
/* bits 8 thru 31 reserved */

/* Game -> Engine Region */
#define VM_ADDR_CMD_COUNT 0x0100
#define VM_ADDR_CMD_BUF 0x0104

/* Each command is STRIDE bytes wide
   byte 0-3: type
   byte 4-7: arg0 (int32)
   byte 8-11: arg1
   byte 12-15: arg2
   byte 16-19: arg3 
   Max commands per frame: (0x04FF - CMD_BUF + 1) / STRIDE = 51 
*/
#define VM_CMD_STRIDE 20
#define VM_CMD_MAX 51

#define VM_CMD_DRAW_SPRITE 1
#define VM_CMD_PLAY_SOUND 2
/* VM_CMD_DRAW_RECT, VM_CMD_SET_CAMERA, VM_CMD_STOP_SOUND, and on. */

/* Game Scratch Section */
#define VM_ADDR_SCRATCH 0x0500

#endif /* MEMMAP_H */

#pragma once

#include "stm32746g_discovery_lcd.h"

#define CELL_SIZE 						16
#define HALF_CELL_SIZE 					(CELL_SIZE / 2)
#define BOARD_WIDTH 					30 
#define BOARD_HEIGHT 					17

#define SOKOBAN_MAP_WALL 				'*'
#define SOKOBAN_MAP_PLAYER_ON_TARGET 	'+'
#define SOKOBAN_MAP_PLAYER 				'p'
#define SOKOBAN_MAP_TARGET 				'x'
#define SOKOBAN_MAP_STONE 				'o'
#define SOKOBAN_MAP_STONE_ON_TARGET 	'd'
#define SOKOBAN_MAP_EMPTY 				' '


#define SOKOBAN_BACKGROUND_COLOR 		LCD_COLOR_BROWN
#define SOKOBAN_WALL_COLOR 				LCD_COLOR_DARKGRAY
#define SOKOBAN_PLAYER_COLOR 			LCD_COLOR_RED
#define SOKOBAN_STONE_COLOR 			LCD_COLOR_BLUE
#define SOKOBAN_DONE_COLOR 				LCD_COLOR_DARKGREEN
#define SOKOBAN_TARGET_COLOR 			LCD_COLOR_DARKMAGENTA


void sokoban_init_board(void);
void sokoban_move_player(uint32_t delta_x, uint32_t delta_y);
void sokoban_spacebar_handler(void);



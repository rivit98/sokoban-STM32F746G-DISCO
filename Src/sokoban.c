#include <sokoban.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "term_io.h"

typedef struct{
	uint32_t x;
	uint32_t y;
} sokoban_point_t;

sokoban_point_t player;

int sokoban_current_level = 0;
uint32_t total_levels = 0;
uint32_t sokoban_target_num = 0;
char *sokoban_current_level_data;
bool in_game = false;

//one level stored as string of BOARD_WIDTH * BOARD_HEIGHT characters
char *sokoban_levels[] = {
	"                                                                       *****                         *   *                         *   *                         *   ******                    *  xo    *                    *       p*                    *        *                    *  xo    *                    *   ******                    *   ******                    *   ******                    **********                                                                                                   ",

	"                                                                       ***                           *x*                           * ****                    *****o ox*                    *x   op***                    ******o*                           * *                           *x*                           ***                                                                                                                                                                                                   ",
	"                                    *******                       * *   *                       * poo *                       * o   *                       * xxx *                       *******                                                                                                                                                                                                                                                                                                                             ",
	"                                    ******                        *   x*                        *o** **                       *   p *                       * * o *                       *x  ***                       *****                                                                                                                                                                                                                                                                                                 "
};

#define LCD_LAYER_FG 1
#define LCD_LAYER_BG 0

static char sokoban_get_cell_value_by_pos(sokoban_point_t pt);
static void sokoban_draw_board(char *data_level);
static void check_game_end(void);

static uint32_t cell_idx_to_x(int cell_idx){
	return cell_idx / BOARD_WIDTH;
}

static uint32_t cell_idx_to_y(int cell_idx){
	return cell_idx % BOARD_WIDTH;
}

static uint32_t sokoban_x_y_to_idx(sokoban_point_t pt){
	return pt.x * BOARD_WIDTH + pt.y;
}

static void sokoban_idx_to_x_y(uint32_t idx, sokoban_point_t *pt){
	pt->x = cell_idx_to_x(idx);
	pt->y = cell_idx_to_y(idx);
}

static char sokoban_get_cell_value_by_pos(sokoban_point_t pt){
	return sokoban_current_level_data[sokoban_x_y_to_idx(pt)];
}

static char *sokoban_get_cell_by_pos(sokoban_point_t pt){
	return &sokoban_current_level_data[sokoban_x_y_to_idx(pt)];
}

static uint32_t char_occurences(const char *s, char c){
	uint32_t cnt = 0;
	for(int i = 0; s[i]; i++){
		cnt += (s[i] == c);
	}

	return cnt;
}

void sokoban_clear_level(){
	free(sokoban_current_level_data);
	sokoban_current_level_data = NULL;
}

void sokoban_init_board(){
	char *data_level = sokoban_levels[sokoban_current_level];

	sokoban_draw_board(data_level);
	sokoban_current_level_data = strdup(data_level);

	char *pl_idx = strchr(sokoban_current_level_data, SOKOBAN_MAP_PLAYER);
	uint32_t offset = pl_idx - sokoban_current_level_data;

	sokoban_idx_to_x_y(offset, &player);
	sokoban_target_num = char_occurences(sokoban_current_level_data, SOKOBAN_MAP_TARGET);
	total_levels = sizeof(sokoban_levels) / sizeof(char *);

	xprintf("Level %d/%ld loaded! %ld targets\n", sokoban_current_level, total_levels, sokoban_target_num);

	in_game = true;
}

static void sokoban_draw_board(char *data_level)
{
	BSP_LCD_SelectLayer(LCD_LAYER_BG);
	BSP_LCD_Clear(SOKOBAN_BACKGROUND_COLOR);
	BSP_LCD_SelectLayer(LCD_LAYER_FG);
	BSP_LCD_Clear(SOKOBAN_BACKGROUND_COLOR);

	uint32_t cell_index = 0;
	char *c = data_level;
	while (*c){
		uint32_t y = cell_idx_to_x(cell_index) * CELL_SIZE;
		uint32_t x = cell_idx_to_y(cell_index) * CELL_SIZE;

		switch (*c){
		case SOKOBAN_MAP_WALL: //wall
			BSP_LCD_SetTextColor(SOKOBAN_WALL_COLOR);
			BSP_LCD_FillRect(x, y, CELL_SIZE, CELL_SIZE);
			break;

		case SOKOBAN_MAP_PLAYER_ON_TARGET: //player standing on target field
			BSP_LCD_SetTextColor(SOKOBAN_TARGET_COLOR);
			BSP_LCD_FillRect(x, y, CELL_SIZE, CELL_SIZE);

			//fallthrough to draw player

		case SOKOBAN_MAP_PLAYER: //player
			BSP_LCD_SetTextColor(SOKOBAN_PLAYER_COLOR);
			BSP_LCD_FillCircle(x + HALF_CELL_SIZE, y + HALF_CELL_SIZE, HALF_CELL_SIZE - 1);
			break;

		case SOKOBAN_MAP_TARGET: //target field
			BSP_LCD_SetTextColor(SOKOBAN_TARGET_COLOR);
			BSP_LCD_FillRect(x, y, CELL_SIZE, CELL_SIZE);
			break;
		
		case SOKOBAN_MAP_STONE: //stone
			BSP_LCD_SetTextColor(SOKOBAN_STONE_COLOR);
			BSP_LCD_FillCircle(x + HALF_CELL_SIZE, y + HALF_CELL_SIZE, HALF_CELL_SIZE - 3);
			break;

		case SOKOBAN_MAP_STONE_ON_TARGET:
			BSP_LCD_SetTextColor(SOKOBAN_TARGET_COLOR);
			BSP_LCD_FillRect(x, y, CELL_SIZE, CELL_SIZE);

			BSP_LCD_SetTextColor(SOKOBAN_DONE_COLOR);
			BSP_LCD_FillCircle(x + HALF_CELL_SIZE, y + HALF_CELL_SIZE, HALF_CELL_SIZE - 3);
			break;
		}

		c++;
		cell_index++;
	}
}

static void update_game_data(sokoban_point_t old, sokoban_point_t new, bool redraw){
	char *old_data = sokoban_get_cell_by_pos(old);
	char *new_data = sokoban_get_cell_by_pos(new);

	if(*old_data == SOKOBAN_MAP_PLAYER){
		if(*new_data == SOKOBAN_MAP_EMPTY){
			*old_data = SOKOBAN_MAP_EMPTY;
			*new_data = SOKOBAN_MAP_PLAYER;
		}else if(*new_data == SOKOBAN_MAP_TARGET){
			*old_data = SOKOBAN_MAP_EMPTY;
			*new_data = SOKOBAN_MAP_PLAYER_ON_TARGET;
		}
	}else if(*old_data == SOKOBAN_MAP_PLAYER_ON_TARGET){
		if(*new_data == SOKOBAN_MAP_EMPTY){
			*old_data = SOKOBAN_MAP_TARGET;
			*new_data = SOKOBAN_MAP_PLAYER;
		}else if(*new_data == SOKOBAN_MAP_TARGET){
			*old_data = SOKOBAN_MAP_TARGET;
			*new_data = SOKOBAN_MAP_PLAYER_ON_TARGET;
		}
	}else if(*old_data == SOKOBAN_MAP_STONE){
		if(*new_data == SOKOBAN_MAP_EMPTY){
			*old_data = SOKOBAN_MAP_EMPTY;
			*new_data = SOKOBAN_MAP_STONE;
		}else if(*new_data == SOKOBAN_MAP_TARGET){
			*old_data = SOKOBAN_MAP_EMPTY;
			*new_data = SOKOBAN_MAP_STONE_ON_TARGET;
		}
	}else if(*old_data == SOKOBAN_MAP_STONE_ON_TARGET){
		if(*new_data == SOKOBAN_MAP_EMPTY){
			*old_data = SOKOBAN_MAP_TARGET;
			*new_data = SOKOBAN_MAP_STONE;
		}else if(*new_data == SOKOBAN_MAP_TARGET){
			*old_data = SOKOBAN_MAP_TARGET;
			*new_data = SOKOBAN_MAP_STONE_ON_TARGET;
		}
	}

	if(redraw){
		sokoban_draw_board(sokoban_current_level_data);
	}
}

void sokoban_move_player(uint32_t delta_x, uint32_t delta_y)
{
	if(!in_game){
		return;
	}

	sokoban_point_t old_player_pos = {player.x, player.y};
	sokoban_point_t new_player_pos = {player.x + delta_x, player.y + delta_y};

	char new_player_cell = sokoban_get_cell_value_by_pos(new_player_pos);
	
	if (new_player_cell == SOKOBAN_MAP_WALL || 
		new_player_pos.x < 0 || new_player_pos.x >= BOARD_HEIGHT || 
		new_player_pos.y < 0 || new_player_pos.y >= BOARD_WIDTH
		){
		return;
	}

	if(new_player_cell == SOKOBAN_MAP_STONE || new_player_cell == SOKOBAN_MAP_STONE_ON_TARGET){ //push stone
		sokoban_point_t new_stone_pos = {new_player_pos.x + delta_x, new_player_pos.y + delta_y};
		char new_stone_cell = sokoban_get_cell_value_by_pos(new_stone_pos);
		if(new_stone_cell == SOKOBAN_MAP_WALL || new_stone_cell == SOKOBAN_MAP_STONE){
			return;
		}

		update_game_data(new_player_pos, new_stone_pos, false);
	}

	player = new_player_pos;

	update_game_data(old_player_pos, new_player_pos, true);

	check_game_end();
}

void sokoban_spacebar_handler(void){
	if(in_game){ //reset level
		sokoban_init_board();
	}else{ //next level
		sokoban_init_board();
	}
}

static void sokoban_clear_both_screens(){
	BSP_LCD_SelectLayer(LCD_LAYER_BG);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SelectLayer(LCD_LAYER_FG);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
}

static void sokoban_new_game_splashscreen(){
	sokoban_clear_both_screens();

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Level finished", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t *)"Space to continue!", CENTER_MODE);
}

static void sokoban_end_game_splashscreen(){
	sokoban_clear_both_screens();

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAt(0, 100, (uint8_t *)"Game finished", CENTER_MODE);
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t *)"Space to restart game!", CENTER_MODE);
}

static void check_game_end(void){
	uint32_t valid_stones = char_occurences(sokoban_current_level_data, SOKOBAN_MAP_STONE_ON_TARGET);

	if(valid_stones != sokoban_target_num){
		return;
	}

	in_game = false;

	osDelay(800);
	sokoban_clear_level();
	
	sokoban_current_level += 1;
	if(sokoban_current_level >= total_levels){
		sokoban_current_level = 0;
		sokoban_end_game_splashscreen();
	}else{
		sokoban_new_game_splashscreen();
	}
}

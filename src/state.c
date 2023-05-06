#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Task 1 */
game_state_t* create_default_state() {
  game_state_t* state = (game_state_t *) malloc(sizeof(game_state_t));
  state->num_rows = 18;

  state->board = (char **) malloc(state->num_rows * sizeof(char *));
  for(unsigned int i = 0; i < state->num_rows; i++) {
    state->board[i] = (char *) malloc(21 * sizeof(char));
    state->board[i][20] = '\0';
    for(unsigned int j = 0; j < 20; j++) {
      if(i == 0 || i == (state->num_rows - 1) || j == 0 || j == 19) {
        set_board_at(state, i, j, '#');
      }
      else if(i == 2 && j >= 2 && j <= 4) {
        if(j == 2)
          set_board_at(state, i, j, 'd');
        else if(j == 4)
          set_board_at(state, i, j, 'D');
        else 
          set_board_at(state, i, j, '>');
      }
      else {
        set_board_at(state, i, j, ' ');
      }
    }
  }
  set_board_at(state, 2, 9, '*');

  state->num_snakes = 1;
  state->snakes = (snake_t *) malloc(state->num_snakes * sizeof(snake_t));
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].live = true;

  return state;
}

/* Task 2 */
void free_state(game_state_t* state) {
  if(state) {
    if(state->snakes) {
      free(state->snakes);
      state->snakes = NULL;
    }

    for(unsigned int i = 0; i < state->num_rows; i++) {
      if(state->board[i]) {
        free(state->board[i]);
        state->board[i] = NULL;
      }
    }

    if(state->board) {
      free(state->board);
      state->board = NULL;
    }

    free(state);
    state = NULL;
  }

  return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  for(unsigned int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s\n", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  if(c == 'w' || c == 'a' || c == 's' || c == 'd') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  if(c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
    return true;
  }
  return false;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  if(is_tail(c) || c == '^' || c == '<' || c == 'v' || c == '>' || is_head(c)) {
    return true;
  }
  return false;
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  if(c == '^')
    return 'w';
  else if(c == '<') 
    return 'a';
  else if(c == 'v')
    return 's';
  else if(c == '>')
    return 'd';
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  if(c == 'W')
    return '^';
  else if(c == 'A')
    return '<';
  else if(c == 'S')
    return 'v';
  else if(c == 'D')
    return '>';
  return '?';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if(c == 'v' || c == 's' || c == 'S')
    return cur_row + 1;
  else if(c == '^' || c == 'w' || c == 'W')
    return cur_row - 1;
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if(c == '>' || c == 'd' || c == 'D')
    return cur_col + 1;
  else if(c == '<' || c == 'a' || c == 'A')
    return cur_col - 1;
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  if(state != NULL && snum < state->num_snakes) {
    if(state->snakes[snum].live == false)
      return '?';

    unsigned int x = state->snakes[snum].head_row;
    unsigned int y = state->snakes[snum].head_col;
    char head = get_board_at(state, x, y);

    unsigned int new_x = get_next_row(x, head);
    unsigned int new_y = get_next_col(y, head);

    if(new_x < state->num_rows && new_y < strlen(state->board[0]))
      return get_board_at(state, new_x, new_y);
    else 
      return '?';
  }

  return '?';
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  if(state != NULL && snum < state->num_snakes) {
    if(state->snakes[snum].live == false)
      return;

    unsigned int x = state->snakes[snum].head_row;
    unsigned int y = state->snakes[snum].head_col;
    char head = get_board_at(state, x, y);
    unsigned int new_x = get_next_row(x, head);
    unsigned int new_y = get_next_col(y, head);

    set_board_at(state, new_x, new_y, head);
    set_board_at(state, x, y, head_to_body(head));
    state->snakes[snum].head_row = new_x;
    state->snakes[snum].head_col = new_y;
  }

  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  if(state != NULL && snum < state->num_snakes) {
    if(state->snakes[snum].live == false)
      return;

    unsigned int x = state->snakes[snum].tail_row;
    unsigned int y = state->snakes[snum].tail_col;
    char tail = get_board_at(state, x, y);
    unsigned int new_x = get_next_row(x, tail);
    unsigned int new_y = get_next_col(y, tail);
    char new_tail = body_to_tail(get_board_at(state, new_x, new_y));

    set_board_at(state, new_x, new_y, new_tail);
    set_board_at(state, x, y, ' ');
    state->snakes[snum].tail_row = new_x;
    state->snakes[snum].tail_col = new_y;
  }  
  
  return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  if(state != NULL) {
    for(unsigned int i = 0; i < state->num_snakes; i++) {
      if(state->snakes[i].live == false)
        continue;

      char next = next_square(state, i);
      if(next == '*') {
        update_head(state, i);
        add_food(state);
      }
      else if(next == '#' || is_snake(next)) {
        state->snakes[i].live = false;
        set_board_at(state, state->snakes[i].head_row, state->snakes[i].head_col, 'x');
      }
      else {
        update_head(state, i);
        update_tail(state, i);
      }
    }
  }

  return;
}

/* Task 5 */
game_state_t* load_board(char* filename) {
  FILE* file = fopen(filename, "r");

  game_state_t* state = (game_state_t *) malloc(sizeof(game_state_t));

  unsigned int num_rows = 0;
  if(file != NULL) {   
    char ch;
    while((ch = (char) fgetc(file)) != EOF) {
      if(ch == '\n') {
        num_rows++;
      }
    }
  }
  fclose(file);

  state->num_rows = num_rows;
  state->board = (char **) malloc(num_rows * sizeof(char *));

  file = fopen(filename, "r");
  if(file != NULL) {
    unsigned int num_cols = 0;
    unsigned int indx = 0;
    char ch;
    while((ch = (char) fgetc(file)) != EOF) {
      if(ch == '\n') {
        // add 1 to the num_cols for null char 
        state->board[indx++] = (char *) malloc((num_cols + 1) * sizeof(char));
        num_cols = 0;
        continue;
      }

      num_cols++;
    }
  }
  fclose(file);

  file = fopen(filename, "r");
  if(file != NULL) {
    unsigned int row = 0;
    unsigned int col = 0; 
    char ch;
    while((ch = (char) fgetc(file)) != EOF) {   
      if(ch == '\n') {
        set_board_at(state, row, col, '\0');
        col = 0;
        row++;
        continue;
      }
      set_board_at(state, row, col++, ch);
    }
  }
  fclose(file);

  state->num_snakes = 0;
  state->snakes = NULL;

  return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
  unsigned int x = state->snakes[snum].tail_row;
  unsigned int y = state->snakes[snum].tail_col;

  while(!is_head(get_board_at(state, x, y))) {
    char ch = get_board_at(state, x, y);
    unsigned int new_x = get_next_row(x, ch);
    unsigned int new_y = get_next_col(y, ch);

    x = new_x;
    y = new_y;
  }

  state->snakes[snum].head_row = x;
  state->snakes[snum].head_col = y;
  return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  if(state) {
    // cnt the number of snakes on the board
    unsigned int cnt = 0;
    for(unsigned int i = 0; i < state->num_rows; i++) {
      for(unsigned int j = 0; j < strlen(state->board[i]); j++) {
        if(is_tail(get_board_at(state, i, j))) {
          cnt++;
        }
      }
    }
    state->snakes = (snake_t *) malloc(cnt * sizeof(snake_t));
    state->num_snakes = cnt;

    // find head and tail of each snake
    unsigned int snum = 0;
    for(unsigned int i = 0; i < state->num_rows; i++) {
      for(unsigned int j = 0; j < strlen(state->board[i]); j++) {
        if(is_tail(get_board_at(state, i, j))) {
          state->snakes[snum].tail_row = i;
          state->snakes[snum].tail_col = j;
          find_head(state, snum);
          snum++;
        }
      }
    }
  }
  return state;
}

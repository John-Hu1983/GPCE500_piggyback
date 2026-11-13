#ifndef _GAME_2048_H__
#define _GAME_2048_H__

typedef struct
{
    u8 grid[4][4]; /* Stores log2(value): 0 = empty, 1 = 2, 2 = 4, ..., 11 = 2048 */
    u16 score;
    bool over;
} G2048_State;

void game_2048_init(G2048_State *g);
void game_2048_on_key(G2048_State *g, BTN_Id key);

#endif /* _GAME_2048_H__ */

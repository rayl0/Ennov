#if !defined(ENNOV_H)
#include "ennov_platform.h"

#ifdef __cplusplus
extern "C" {
#endif 

// TODO(Rajat): Services that the game provide to the platform layer

void GameUpdateAndRender(game_input* Input, game_memory* Memory); 


#ifdef __cplusplus    
}
#endif

#define ENNOV_H
#endif 

//
// Created by PETROS on 09/11/2024.
//

#ifndef INPUT_HANDLING_H
#define INPUT_HANDLING_H
#include <SDL3/SDL_scancode.h>


struct Camera;


void KeyDown(SDL_Scancode key, Camera& camera);


#endif //INPUT_HANDLING_H

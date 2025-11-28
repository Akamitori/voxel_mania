//
// Created by PETROS on 09/11/2024.
//

#include "InputHandling.h"
#include "Camera.h"
#include "Renderer.h"

//TODO we can probably remove the camera and request it from the renderer directly
//TODO we could also probably have a camera attached to a separate object with transform hierarchies but lets ignore that for now
void KeyDown(const SDL_Scancode key, Camera &camera) {
    bool camera_changed = true;

    switch (key) {
        case SDL_SCANCODE_W: {
            MoveCameraZ(camera, 1);
            break;
        }
        case SDL_SCANCODE_S: {
            MoveCameraZ(camera, -1);
            break;
        }
        case SDL_SCANCODE_D: {
            MoveCameraX(camera, 1);
            break;
        }
        case SDL_SCANCODE_A: {
            MoveCameraX(camera, -1);
            break;
        }
        case SDL_SCANCODE_SPACE: {
            MoveCameraY(camera, -1);
            break;
        }
        case SDL_SCANCODE_LSHIFT: {
            MoveCameraY(camera, 1);
            break;
        }
        case SDL_SCANCODE_RIGHT: {
            RotateCamera(camera, 1, 0);
            break;
        }
        case SDL_SCANCODE_LEFT: {
            RotateCamera(camera, -1, 0);
            break;
        }
        case SDL_SCANCODE_UP: {
            RotateCamera(camera, 0, -1);
            break;
        }
        case SDL_SCANCODE_DOWN: {
            RotateCamera(camera, 0, 1);
            break;
        }
        default: {
            camera_changed = false;
            break;
        }
    }


    

    if (camera_changed) {
        Renderer_CameraUpdate();
    }
}

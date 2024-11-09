//
// Created by PETROS on 09/11/2024.
//

#include "InputHandling.h"
#include "AppData.h"
#include "Camera.h"

void KeyDown(SDL_Scancode key, AppData& appData) {
    bool camera_changed = true;

    switch (key) {
        case SDL_SCANCODE_W: {
            MoveCameraZ(appData.camera, 1);
            break;
        }
        case SDL_SCANCODE_S: {
            MoveCameraZ(appData.camera, -1);
            break;
        }
        case SDL_SCANCODE_D: {
            MoveCameraX(appData.camera, 1);
            break;
        }
        case SDL_SCANCODE_A: {
            MoveCameraX(appData.camera, -1);
            break;
        }
        case SDL_SCANCODE_SPACE: {
            MoveCameraY(appData.camera, 1);
            break;
        }
        case SDL_SCANCODE_LSHIFT: {
            MoveCameraY(appData.camera, -1);
            break;
        }
        case SDL_SCANCODE_RIGHT: {
            RotateCamera(appData.camera, 1, 0);
            break;
        }
        case SDL_SCANCODE_LEFT: {
            RotateCamera(appData.camera, -1, 0);
            break;
        }
        case SDL_SCANCODE_UP: {
            RotateCamera(appData.camera, 0, 1);
            break;
        }
        case SDL_SCANCODE_DOWN: {
            RotateCamera(appData.camera, 0, -1);
            break;
        }
        default: {
            camera_changed = false;
            break;
        }
    }

    if (camera_changed) {
        appData.camera_matrix = CameraLookAtMatrix(appData.camera);
        appData.view_dirty = true;
    }
}

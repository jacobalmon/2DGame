#include <raylib.h>
#include <iostream>
#include "Samurai.h"
#include "Goblin.h"

int main() {
    InitWindow(800, 600, "2D Game");

    // Creating Samurai at (300, 450) on the same level
    Samurai samurai((Vector2) {300, 450});
    samurai.loadTextures();

    // Creating Goblin at (500, 450) on the same level
    Goblin goblin((Vector2) {500, 450});
    goblin.loadTextures();

    // Set FPS at 60.
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update.
        samurai.move();
        samurai.applyVelocity();
        samurai.updateAnimation();

        goblin.move();
        goblin.applyVelocity();
        goblin.updateAnimation();

        // Drawing.
        BeginDrawing();
        ClearBackground(GREEN);

        // Draw ground/platform
        DrawRectangle(0, 500, 800, 100, DARKBROWN);
        
        samurai.draw();
        goblin.draw();
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}


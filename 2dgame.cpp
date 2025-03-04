#include <raylib.h>
#include <iostream>
#include "Samurai.h"
#include "Goblin.h"
#include "Werewolf.h"
#include "Wizard.h"
#include "Demon.h"

int main() {
    InitWindow(800, 600, "2D Game");

    // Initialize audio device before loading music
    InitAudioDevice(); 

    // Load background music, Note this is a placeholder music file, this is the final boss music.
    Music backgroundMusic = LoadMusicStream("music/Lady Maria of the Astral Clocktower.mp3");  // Replace with your music file path.

    PlayMusicStream(backgroundMusic);  // Start playing the music
    SetMusicVolume(backgroundMusic, 0.5f);  // Adjust the volume if necessary (0.0 to 1.0)

    // Load floor and wall textures
    Texture2D floorTexture = LoadTexture("assets/tiles/floor_tile.png");
    Texture2D wallTexture = LoadTexture("assets/tiles/wall_tile.png");
    float tileSize = 64.0f;  // Size of each tile
    int numTilesX = (800 / tileSize) + 1;  // Number of tiles needed horizontally
    float floorY = 480.0f;  // Y position of the floor
    int numTilesY = ((600 - floorY) / tileSize) + 1;  // Number of tiles needed vertically from floor to bottom
    int numWallTilesY = (floorY / tileSize);  // Number of tiles needed vertically for walls

    // Creating Samurai.
    Samurai samurai((Vector2) {400, 300});
    samurai.loadTextures();
    samurai.loadSounds();

    // Creating Goblin.
    Goblin goblin((Vector2) {400, 300});
    goblin.loadTextures();

    // Creating Werewolf.
    Werewolf werewolf((Vector2) {400, 300});
    werewolf.loadTextures();

    // Creating Wizard.
    Wizard wizard((Vector2) {400, 300});
    wizard.loadTextures();

    // Creating Demon.
    Demon demon((Vector2) {400, 300});

    // Set FPS at 60.
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update background music stream
        UpdateMusicStream(backgroundMusic);  // Ensure music keeps playing

        // Update game objects
        samurai.updateSamurai();

        goblin.move();
        goblin.applyVelocity();
        goblin.updateAnimation();

        werewolf.move();
        werewolf.applyVelocity();
        werewolf.updateAnimation();

        wizard.move();
        wizard.applyVelocity();
        wizard.updateAnimation();

        demon.move();
        demon.applyVelocity();
        demon.updateAnimation();

        // Drawing.
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw wall tiles (from top to floor)
        for (int y = 0; y < numWallTilesY; y++) {
            for (int x = 0; x < numTilesX; x++) {
                DrawTexture(wallTexture, x * tileSize, y * tileSize, WHITE);
            }
        }

        // Draw floor tiles (only from y = 480 to bottom)
        for (int y = 0; y < numTilesY; y++) {
            for (int x = 0; x < numTilesX; x++) {
                float tileY = floorY + (y * tileSize);
                if (tileY >= floorY) {  // Only draw tiles at or below floorY
                    DrawTexture(floorTexture, x * tileSize, tileY, WHITE);
                }
            }
        }

        // Draw characters and their health bars
        samurai.draw();
        samurai.drawHealthBar();
        goblin.draw();
        goblin.drawHealthBar();
        werewolf.draw();
        werewolf.drawHealthBar();
        wizard.draw();
        wizard.drawHealthBar();
        demon.draw();
        demon.drawHealthBar();
        EndDrawing();
    }

    // Unload resources
    UnloadTexture(floorTexture);
    UnloadTexture(wallTexture);
    UnloadMusicStream(backgroundMusic);  // Free the music resources
    CloseAudioDevice();  // Close the audio device
    CloseWindow();  // Close the window
    return 0;
}
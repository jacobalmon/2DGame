#include <raylib.h>
#include <raytmx.h>
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

    // Creating Samurai.
    Samurai samurai((Vector2) {400, 300});
    samurai.loadTextures();
    samurai.loadSounds();

    // Creating Goblin.
    Goblin goblin((Vector2) {400, 300});
    goblin.loadTextures();
    goblin.loadSounds();

    // Creating Werewolf.
    Werewolf werewolf((Vector2 {400, 300}));
    werewolf.loadTextures();
    werewolf.loadSounds();

    // Creating Wizard.
    Wizard wizard((Vector2 {400, 300}));
    wizard.loadTextures();
    wizard.loadSounds();

    // Creating Demon.
    Demon demon((Vector2 {400, 300}));
    demon.loadSounds();

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
        ClearBackground(GREEN);
        samurai.draw();
        samurai.drawHealthBar();
        goblin.draw();
        werewolf.draw();
        wizard.draw();
        demon.draw();
        EndDrawing();
    }

    // Unload music and close window
    UnloadMusicStream(backgroundMusic);  // Free the music resources
    CloseAudioDevice();  // Close the audio device
    CloseWindow();  // Close the window
    return 0;
}
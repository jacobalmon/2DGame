#define RAYTMX_IMPLEMENTATION
#include <raylib.h>
#include "raytmx.h"
// #include <raytmx.h> // <== This is the original.
#include <iostream>
#include "Samurai.h"
#include "Goblin.h"
#include "Werewolf.h"
#include "Wizard.h"
#include "Demon.h"

// This is where you put a bunch of png in layer. Top to bottom.
static std::vector<std::string> layerPaths = {
    "assets/Backgrounds/Test_Background_layers/Layer_0011_0.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0010_1.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0009_2.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0008_3.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0007_Lights.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0006_4.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0005_5.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0004_Lights.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0003_6.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0002_7.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0001_8.png",
    "assets/Backgrounds/Test_Background_layers/Layer_0000_9.png"
};

int main() 
{
    InitWindow(1600, 1000, "2D Game"); // <== Change the size to fit your screen

    // Pause State Variable
    bool isPaused = false;  // Game starts unpaused

    // Load TMX level
    const char* tmxFile = "resources/Room2.tmx";
    TmxMap* level = LoadTMX(tmxFile);
    if (level == nullptr) {
        TraceLog(LOG_ERROR, "Could not load level: %s", tmxFile);
        return EXIT_FAILURE;
    }

    // Load all layers into a vector of Texture2D
    std::vector<Texture2D> backgroundLayers;
    backgroundLayers.reserve(layerPaths.size());
    for (const auto &path : layerPaths)
    {
        backgroundLayers.push_back(LoadTexture(path.c_str()));
    }
    
    // Initialize audio device before loading music
    InitAudioDevice(); 
    // Load background music, Note this is a placeholder music file, this is the final boss music.
    Music backgroundMusic = LoadMusicStream("music/Lady Maria of the Astral Clocktower.mp3");  // Replace with your music file path.
    PlayMusicStream(backgroundMusic);  // Start playing the music
    SetMusicVolume(backgroundMusic, 0.0f);  // Adjust the volume if necessary (0.0 to 1.0)

    // Creating Samurai.
    Samurai samurai((Vector2) {400, 6850});
    samurai.loadTextures();
    samurai.loadSounds();

    // Creating Goblin.
    Goblin goblin((Vector2) {400, 300});
    goblin.loadTextures();

    // Creating Werewolf.
    Werewolf werewolf((Vector2 {400, 300}));
    werewolf.loadTextures();

    // Creating Wizard.
    Wizard wizard((Vector2 {400, 300}));
    wizard.loadTextures();

    // Creating Demon.
    Demon demon((Vector2 {400, 300}));

    // Set up the camera.
    Camera2D camera = {0};
    camera.target = (Vector2){400, 300};
    camera.offset = (Vector2){800, 400};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f; // Zooming in or out

    

     // Set FPS at 60.
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) 
    {
        if (IsKeyPressed(KEY_P)) 
        {
            isPaused = !isPaused;
        }
        // Update background music stream
        UpdateMusicStream(backgroundMusic);  // Ensure music keeps playing

        if (!isPaused) 
        {
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

            camera.target = samurai.getPosition(); // Track the samurai
        }

        // Drawing.
        BeginDrawing();
            ClearBackground(GREEN);
            
            int offset = 50;
            for (auto &layer : backgroundLayers) // each layer in a loop
            {
                DrawTexturePro
                (
                    layer,
                    Rectangle{ 0, 0, (float)layer.width, (float)layer.height },
                    Rectangle{ 0, -50, (float)GetScreenWidth(), (float)GetScreenHeight() },
                    Vector2{ 0, 0 },
                    0.0f,
                    WHITE
                );
                
                DrawTexturePro( // No green screen
                layer,
                Rectangle{ 0, layer.height - offset, (float)layer.width, (float)offset },
                Rectangle{ 0, (float)GetScreenHeight() - offset, (float)GetScreenWidth(), (float)offset },
                Vector2{ 0, 0 },
                0.0f,
                WHITE
                );
            }
            
            BeginMode2D(camera); // Camera lock in
                DrawTMX(level, &camera, 0, 0, WHITE); // level
                samurai.draw();
                goblin.draw();
                werewolf.draw();
                wizard.draw();
                demon.draw();
        
            EndMode2D();
        
        samurai.drawHealthBar();
        
        // Draw Pause Screen
        if (isPaused) 
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
            DrawText("PAUSED", GetScreenWidth()/2 - 50, GetScreenHeight()/2 - 10, 30, WHITE);
            DrawText("Press 'P' to resume", GetScreenWidth()/2 - 100, GetScreenHeight()/2 + 30, 20, WHITE);
        }
        
        EndDrawing();
    }

    for (auto &layer : backgroundLayers)
    {
        UnloadTexture(layer);
    }
    
    // Unload music and close window
    UnloadTMX(level);
    UnloadMusicStream(backgroundMusic);  // Free the music resources
    CloseAudioDevice();  // Close the audio device
    CloseWindow();  // Close the window
    return 0;
}
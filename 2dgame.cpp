#include "raylib.h"
#include "raymath.h"
#include "CollisionSystem.h"
#include "Samurai.h"
#include <iostream>
#include <vector>
#include <stdlib.h> // For exit()
#include <unistd.h> // For getcwd()
#include <limits.h> // For PATH_MAX
#include <cstdio>
#include "StartScreen.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Define RAYTMX_IMPLEMENTATION to include the implementation of the library
#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

// Define the global variable for collision box visibility
bool showCollisionBoxes = false;

// Global audio variables
Music backgroundMusic = { 0 };
Sound demonChantSound = { 0 };
float masterVolume = 0.7f;

// Define a scale factor for the demon (visual only)
float demonScaleFactor = 1.3f;

// Global background texture
Texture2D backgroundTexture = { 0 };
Camera2D camera = { 0 };

// Global TMX map variable
TmxMap* tmxMap = NULL;

// Global map variables
int mapWidth = 128;  // Default map width in tiles
int mapHeight = 32;  // Default map height in tiles
const int mapTileSize = 16;

// Key variables
Texture2D keyTexture = { 0 };
Vector2 keyPosition = { 0, 0 };
float keyRotation = 0.0f;
float keyFrameTime = 0.0f;
int keyCurrentFrame = 0;
const int KEY_FRAME_COUNT = 17;  // Number of frames in the sprite sheet
const float KEY_FRAME_DURATION = 0.1f;  // Duration for each frame in seconds
const int KEY_FRAME_WIDTH = 32;  // Width of each frame in pixels
const int KEY_FRAME_HEIGHT = 32; // Height of each frame in pixels
Rectangle keyCollisionRect = { 0 }; // Collision rectangle for the key
Sound keySound = { 0 }; // Sound to play when key is collected
bool keyCollected = false; // Flag to track if key has been collected

enum GameState {START_SCREEN, MAIN_GAME, EXIT};

// Helper function to check collision between two collision boxes
bool checkCharacterCollision(const CollisionBox& box1, const CollisionBox& box2) {
    if (box1.active && box2.active) {
        return CheckCollisionRecs(box1.rect, box2.rect);
    }
    return false;
}

// Helper function to handle attack collision and damage
void handleAttackCollision(CollisionBox* attackBox, CollisionBox* hurtBox, int& health, int damage) {
    if (attackBox && hurtBox && attackBox->active && hurtBox->active && 
        checkCharacterCollision(*attackBox, *hurtBox)) {
        health -= damage;
        if (health < 0) health = 0;
        
        // Deactivate attack box to prevent multiple hits from the same attack
        attackBox->active = false;
    }
}

// Custom exit function that bypasses normal cleanup
void safeExit() {
    // Unload audio resources
    if (backgroundMusic.ctxData != NULL) {
        StopMusicStream(backgroundMusic);
        UnloadMusicStream(backgroundMusic);
    }
    
    if (demonChantSound.stream.buffer) {
        UnloadSound(demonChantSound);
    }
    
    if (keySound.stream.buffer) {
        UnloadSound(keySound);
    }
    
    // Unload the background texture
    if (backgroundTexture.id != 0) {
        UnloadTexture(backgroundTexture);
    }
    
    // Unload the TMX map
    if (tmxMap != NULL) {
        UnloadTMX(tmxMap);
        tmxMap = NULL;
    }
    
    // Clean up Raylib
    CloseAudioDevice();
    CloseWindow();
    
    // Exit directly without going through normal cleanup
    _Exit(0); // Use _Exit instead of exit to bypass any atexit handlers
}

// Function to draw collision boxes for debugging
void drawCollisionBox(const CollisionBox& box) {
    if (!box.active) return;
    
    Color color;
    switch (box.type) {
        case BODY:
            color = BLUE;
            break;
        case ATTACK:
            color = RED;
            break;
        case HURTBOX:
            color = GREEN;
            break;
        default:
            color = YELLOW;
            break;
    }
    
    // Draw the collision box as a rectangle outline
    DrawRectangleLines(box.rect.x, box.rect.y, box.rect.width, box.rect.height, color);
    
    // Draw a small label to identify the box type
    const char* label = "";
    switch (box.type) {
        case BODY: label = "BODY"; break;
        case ATTACK: label = "ATTACK"; break;
        case HURTBOX: label = "HURT"; break;
        default: label = "UNKNOWN"; break;
    }
    
    DrawText(label, box.rect.x, box.rect.y - 15, 10, color);
}

// Function to check if a file exists
bool fileExists(const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

int main() {
    // Print current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
    
    // Set up error handling
    SetTraceLogLevel(LOG_WARNING);
    
    // Initialize window
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "2D Game");

    // Define floor level to match where the non-zero tiles (floor tiles) are in Room1.tmx
    // This value is used for all characters to ensure consistent vertical positioning
    const float floorLevel = 380.0f; // Exact floor level matching the non-zero floor tiles in TMX
    const float floorHeight = 50.0f; // Height of the floor rectangle if needed
    
    // Update constant values in other files to match our floor level
    // These are compile-time constants, but we're making this note for clarity
    // GROUND_LEVEL and GROUND_LEVEL_WIZARD should be equal to floorLevel for consistency
    
    // Initialize audio device before loading music
    InitAudioDevice();

    // Load background music
    backgroundMusic = LoadMusicStream("music/Lady Maria of the Astral Clocktower.mp3");
    PlayMusicStream(backgroundMusic);  // Start playing the music
    SetMusicVolume(backgroundMusic, 0.5f * masterVolume);
    
    // Load demon chant sound
    demonChantSound = LoadSound("sounds/misc/demon-chant-latin-14489.mp3");
    SetSoundVolume(demonChantSound, 0.6f * masterVolume);
    
    // Load key sound
    keySound = LoadSound("sounds/key/keysound.mp3");
    
    SetTargetFPS(60);

    // Load background texture
    // Try to load from maps directory first
    backgroundTexture = LoadTexture("maps/64 x 64 Blue Dungeon Sprite Sheet.png");
    if (backgroundTexture.id == 0) {
        // If failed, try assets directory
        backgroundTexture = LoadTexture("assets/background/dungeon_background.png");
        if (backgroundTexture.id == 0) {
            printf("Failed to load background texture! Creating a default one.\n");
            // Create a default background texture with a simple pattern
            Image checkerImage = GenImageChecked(800, 600, 64, 64, DARKGRAY, GRAY);
            backgroundTexture = LoadTextureFromImage(checkerImage);
            UnloadImage(checkerImage);
        }
    }
    
    // Initialize camera
    camera.target = (Vector2){ screenWidth/2.0f, screenHeight/2.0f }; // Set initial target to center of screen
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;  // Zoom in for better visibility
  
    // Keep the array of map paths for fallback loading
    const char* mapPaths[] = {
        "maps/Room1.tmx",
        "maps/Room3and4.tmx",
        "maps/Dungeon.tmx"
    };
    int numMapPaths = sizeof(mapPaths) / sizeof(mapPaths[0]);
    
    // Try to load any available map
    tmxMap = NULL;
    bool mapLoaded = false;
    for (int i = 0; i < numMapPaths && !mapLoaded; i++) {
        tmxMap = LoadTMX(mapPaths[i]);
        if (tmxMap != NULL) {
            mapLoaded = true;
            printf("Successfully loaded TMX map: %s\n", mapPaths[i]);
        } else {
            printf("Failed to load TMX map: %s\n", mapPaths[i]);
        }
    }
    
    if (!mapLoaded) {
        printf("Failed to load any TMX map. Exiting...\n");
        exit(1);
    }
    
    // Keep the tileset checking code but remove debug prints
    const char* requiredTilesets[] = {
        "maps/16 x16 Purple Dungeon Sprite Sheet copy.png"
    };
    int numRequiredTilesets = sizeof(requiredTilesets) / sizeof(requiredTilesets[0]);
    
    // Ensure needed tileset files exist
    for (int i = 0; i < numRequiredTilesets; i++) {
        if (!fileExists(requiredTilesets[i])) {
            // Keep the TSX file creation code
            FILE* file = fopen(requiredTilesets[i], "w");
            if (file) {
                fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
                fprintf(file, "<tileset version=\"1.10\" tiledversion=\"1.10.2\" name=\"16 x16 Purple Dungeon Sprite Sheet\" tilewidth=\"16\" tileh\n");
                fprintf(file, "eight=\"16\" tilecount=\"60\" columns=\"6\">\n");
                fprintf(file, " <image source=\"16 x16 Purple Dungeon Sprite Sheet.png\" width=\"96\" height=\"160\"/>\n");
                fprintf(file, "</tileset>\n");
                fclose(file);
            } else {
                printf("Failed to create tileset file: %s\n", requiredTilesets[i]);
            }
        }
    }
    
    // Calculate the map dimensions in pixels based on the loaded TMX map
    // Try to get actual dimensions from the loaded TMX map
    if (tmxMap != NULL && tmxMap->fileName != NULL) {
        // Get map dimensions from the loaded TMX file
        mapWidth = tmxMap->width;
        mapHeight = tmxMap->height;
    } else {
        printf("Using default map dimensions: %d x %d tiles\n", mapWidth, mapHeight);
    }
    
    const float mapWidthPixels = mapWidth * mapTileSize;
    const float mapHeightPixels = mapHeight * mapTileSize;
    
    // Load key texture
    keyTexture = LoadTexture("assets/gameObjects/key/key.png");
    if (keyTexture.id == 0) {
        printf("Failed to load key texture!\n");
        CloseWindow();
        return -1;
    }
    // Place the key further in the map to encourage exploration
    keyPosition = { 1200, floorLevel - keyTexture.height };

    // Initialize characters using stack allocation - all characters now use the same floorLevel
    Samurai samurai(100, floorLevel, floorLevel);
    
    // Initialize dash sound volume to match master volume
    samurai.setDashSoundVolume(0.8f * masterVolume);
    
    // Position camera at initial player position
    Rectangle initialPlayerRect = samurai.getRect();
    camera.target = (Vector2){ initialPlayerRect.x, initialPlayerRect.y };

    StartScreen startScreen;
    GameState gameState = START_SCREEN;

    // Game loop
    while (!WindowShouldClose()) {

        // Handle game state updates based on current game state
        switch(gameState) {
            case START_SCREEN: {
                startScreen.Update();

                if (startScreen.ShouldStartGame()) {
                    gameState = MAIN_GAME;  // Transition to main game
                }
                if (startScreen.ShouldExitGame()) {
                    gameState = EXIT;  // Exit the game
                }

                BeginDrawing();
                startScreen.Draw();  // Draw the start screen
                EndDrawing();
                break;
            }

            case MAIN_GAME: {
                // Update music stream
                UpdateMusicStream(backgroundMusic);
                
                // Toggle music with M key
                if (IsKeyPressed(KEY_M)) {
                    if (IsMusicStreamPlaying(backgroundMusic)) {
                        PauseMusicStream(backgroundMusic);
                    } else {
                        ResumeMusicStream(backgroundMusic);
                    }
                }

                // Volume control with + and - keys
                if (IsKeyPressed(KEY_EQUAL)) { // + key
                    masterVolume += 0.1f;
                    if (masterVolume > 1.0f) masterVolume = 1.0f;
                    SetMusicVolume(backgroundMusic, 0.5f * masterVolume);
                    SetSoundVolume(demonChantSound, 0.6f * masterVolume);
                    samurai.setDashSoundVolume(0.8f * masterVolume);
                }

                if (IsKeyPressed(KEY_MINUS)) { // - key
                    masterVolume -= 0.1f;
                    if (masterVolume < 0.0f) masterVolume = 0.0f;
                    SetMusicVolume(backgroundMusic, 0.5f * masterVolume);
                    SetSoundVolume(demonChantSound, 0.6f * masterVolume);
                    samurai.setDashSoundVolume(0.8f * masterVolume);
                }

                // Toggle collision box visibility with F1 key
                if (IsKeyPressed(KEY_F1)) {
                    showCollisionBoxes = !showCollisionBoxes;
                    printf("Collision boxes visibility: %s\n", showCollisionBoxes ? "ON" : "OFF");
                }

                // Get frame time for updates
                float deltaTime = GetFrameTime();

                // Update samurai character
                samurai.updateSamurai();

                // Get samurai position for collision detection
                Vector2 samuraiPos = {0, 0};
                CollisionBox* samuraiBody = samurai.getCollisionBox(BODY);
                if (samuraiBody && samuraiBody->active) {
                    samuraiPos.x = samuraiBody->rect.x + samuraiBody->rect.width / 2;
                    samuraiPos.y = samuraiBody->rect.y + samuraiBody->rect.height / 2;

                // Check for key collection
                if (samuraiBody && samuraiBody->active) {
                    if (CheckCollisionRecs(samuraiBody->rect, keyCollisionRect) && !keyCollected) {
                        // Play key collection sound
                        PlaySound(keySound);
                        keyCollected = true;
                    }
                }

                // Check for collisions between Samurai's attack and enemies
                CollisionBox* samuraiAttack = samurai.getCollisionBox(ATTACK);

                // Check for enemy attacks hitting Samurai
                CollisionBox* samuraiHurtbox = samurai.getCollisionBox(HURTBOX);

                // Update animations and collision rectangles
                keyFrameTime += deltaTime;
                if (keyFrameTime >= KEY_FRAME_DURATION) {
                    keyFrameTime = 0.0f;
                    keyCurrentFrame = (keyCurrentFrame + 1) % KEY_FRAME_COUNT;
                }

                // Update key collision rectangle
                keyCollisionRect = (Rectangle){
                    keyPosition.x + (KEY_FRAME_WIDTH),              // Center horizontally on the scaled key
                    keyPosition.y + (KEY_FRAME_HEIGHT),             // Center vertically on the scaled key
                    (float)KEY_FRAME_WIDTH,                         // Keep original collision size
                    (float)KEY_FRAME_HEIGHT
                };

                // Update TMX animations
                if (tmxMap != NULL && tmxMap->fileName != NULL) {
                    AnimateTMX(tmxMap);
                }

                // Update camera to follow player, ensuring it stays within map boundaries
                Rectangle samuraiRect = samurai.getRect();
                camera.target = (Vector2){ samuraiRect.x, samuraiRect.y };

                float halfScreenWidth = screenWidth / (2.0f * camera.zoom);
                float halfScreenHeight = screenHeight / (2.0f * camera.zoom);

                // Ensure camera doesn't go out of bounds
                if (camera.target.x < halfScreenWidth) camera.target.x = halfScreenWidth;
                if (camera.target.x > mapWidthPixels - halfScreenWidth) camera.target.x = mapWidthPixels - halfScreenWidth;
                if (camera.target.y < halfScreenHeight) camera.target.y = halfScreenHeight;
                if (camera.target.y > mapHeightPixels - halfScreenHeight) camera.target.y = mapHeightPixels - halfScreenHeight;

                // Camera zoom controls
                camera.zoom += ((float)GetMouseWheelMove() * 0.1f);
                if (camera.zoom < 0.5f) camera.zoom = 0.5f;
                if (camera.zoom > 3.0f) camera.zoom = 3.0f;

                // Begin drawing
                BeginDrawing();
                ClearBackground(RAYWHITE);

                // 2D camera mode for proper drawing
                BeginMode2D(camera);

                // Draw background
                if (tmxMap != NULL && tmxMap->fileName != NULL) {
                    DrawTMX(tmxMap, &camera, 0, 0, WHITE);
                } else {
                    float tileWidth = backgroundTexture.width;
                    float tileHeight = backgroundTexture.height;
                    int tilesX = (int)((screenWidth / camera.zoom) / tileWidth) + 2;
                    int tilesY = (int)((screenHeight / camera.zoom) / tileHeight) + 2;
                    float startX = (int)(camera.target.x / 2 - (tilesX * tileWidth) / 2);
                    float startY = (int)(camera.target.y / 2 - (tilesY * tileHeight) / 2);

                    for (int y = 0; y < tilesY; y++) {
                        for (int x = 0; x < tilesX; x++) {
                            DrawTexture(backgroundTexture, startX + x * tileWidth, startY + y * tileHeight, WHITE);
                        }
                    }
                }

                // Draw characters and key
                samurai.draw();
                if (!keyCollected) {
                    Rectangle keySource = {
                        (float)(keyCurrentFrame * KEY_FRAME_WIDTH), 0.0f, 
                        (float)KEY_FRAME_WIDTH, (float)KEY_FRAME_HEIGHT
                    };
                    DrawTexturePro(keyTexture, keySource, (Rectangle){ keyPosition.x, keyPosition.y, (float)KEY_FRAME_WIDTH, (float)KEY_FRAME_HEIGHT }, (Vector2){0, 0}, keyRotation, WHITE);
                } else {
                    CollisionBox* samuraiBody = samurai.getCollisionBox(BODY);
                    if (samuraiBody && samuraiBody->active) {
                        Rectangle keySource = { (float)(keyCurrentFrame * KEY_FRAME_WIDTH), 0.0f, (float)KEY_FRAME_WIDTH, (float)KEY_FRAME_HEIGHT };
                        DrawTexturePro(keyTexture, keySource, (Rectangle){ samuraiBody->rect.x + samuraiBody->rect.width/2 - KEY_FRAME_WIDTH/2, samuraiBody->rect.y - KEY_FRAME_HEIGHT - 10, (float)KEY_FRAME_WIDTH, (float)KEY_FRAME_HEIGHT }, (Vector2){0, 0}, keyRotation, WHITE);
                    }
                }

                // Draw collision boxes if enabled
                if (showCollisionBoxes) {
                    if (!keyCollected) {
                        DrawRectangleLines(keyCollisionRect.x, keyCollisionRect.y, keyCollisionRect.width, keyCollisionRect.height, PURPLE);
                    }
                    drawCollisionBox(*samurai.getCollisionBox(BODY));
                    drawCollisionBox(*samurai.getCollisionBox(ATTACK));
                    drawCollisionBox(*samurai.getCollisionBox(HURTBOX));
                }

                // End camera mode and finalize drawing
                EndMode2D();

                // UI controls instructions
                int instructionsY = screenHeight - 210;
                int lineHeight = 25;

                DrawText("GAME CONTROLS:", 10, instructionsY, 20, DARKGRAY);
                DrawText("W or Up: Jump ", 10, instructionsY + lineHeight, 20, DARKGRAY);
                DrawText("A/D or Left/Right: Move", 10, instructionsY + lineHeight*2, 20, DARKGRAY);
                DrawText("Space: Attack", 10, instructionsY + lineHeight*3, 20, DARKGRAY);
                DrawText("Double-tap A/D: Dash", 10, instructionsY + lineHeight*4, 20, DARKGRAY);
                DrawText("F1: Toggle collision boxes", 10, instructionsY + lineHeight*5, 20, DARKGRAY);
                DrawText("M: Toggle music", 10, instructionsY + lineHeight*6, 20, DARKGRAY);
                DrawText("Mouse Wheel: Zoom", 10, instructionsY + lineHeight*7, 20, DARKGRAY);

                EndDrawing();

                break;
            }

            case EXIT: {
                // Handle exit (save state, cleanup, etc.)
                safeExit();
                break;
            }
        }
    }

    // Unload key texture
    UnloadTexture(keyTexture);

    // Unload sounds
    UnloadSound(keySound);

    // This code should never be reached because we call safeExit() when WindowShouldClose() is true
    return 0;
}
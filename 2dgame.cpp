#include "raylib.h"
#include "raymath.h"
#include "CollisionSystem.h"
#include "Samurai.h"
#include "Demon.h"
#include <iostream>
#include <vector>
#include <stdlib.h> // For exit()
#include <unistd.h> // For getcwd()
#include <limits.h> // For PATH_MAX
#include <cstdio>
#include "StartScreen.h"

#include <functional>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Define RAYTMX_IMPLEMENTATION to include the implementation of the library
#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

// Define the global variable for collision box visibility
bool showCollisionBoxes = true;

// Global audio variables
Music backgroundMusic = { 0 };
Music menuMusic = { 0 };
float masterVolume = 0.7f;

// Global background texture
Texture2D backgroundTexture = { 0 };
Camera2D camera = { 0 };

TmxMap* map = NULL;

enum GameState {START_SCREEN, MAIN_GAME, EXIT};
bool isPaused = false;
bool isComplete = false;
bool gameover = false;

float debugTimer = 0.0f;
float debugInterval = 2.0f; 

// Fade transition on map switch
bool isTransitioning = false;
float transitionAlpha = 0.0f;
bool transitionFadeIn = false;

std::function<void()> transitionAction;
void startTransition(std::function<void()> action) {
    isTransitioning = true;
    transitionAlpha = 0.0f;
    transitionFadeIn = false;
    transitionAction = action;
}

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
    
    // Unload the background texture
    if (backgroundTexture.id != 0) {
        UnloadTexture(backgroundTexture);
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

void loadLevel() {
    map = LoadTMX("maps/LevelDesign.tmx");
    if (!map) {
        printf("Failed to Load TMX File.\n");
        exit (1);
    } else {
        printf("Loaded TMX File.");
    }

    // Loop through tilesets (assuming map->tilesets is a pointer to an array of TmxTileset)
    for (int i = 0; i < map->tilesetsLength; i++) {
        TmxTileset* tileset = &map->tilesets[i];  // Accessing tileset by pointer

        // Check if the image source is valid (not empty)
        if (tileset->image.source[0] != '\0') {  // Using image.source to check validity
            Texture2D tilesetTexture = LoadTexture("maps/16 x16 Purple Dungeon Sprite Sheet.png");
            if (tilesetTexture.id == 0) {
                std::cout << "Error loading tileset image" << std::endl;
            }
        }
    }
}

void renderLevel() {
    if (map) {
        DrawTMX(map, &camera, 0, 0, WHITE);
    }
}

void checkTileCollisions(TmxMap* map, Samurai& player) {
    for (unsigned int i = 0; i < map->layersLength; i++) {
      TraceLog(LOG_DEBUG, "current layer is %d: %s", i, map->layers[i].name);
      if (strcmp(map->layers[i].name, "Object Layer 1") == 0 && map->layers[i].type == LAYER_TYPE_OBJECT_GROUP) 
      {
          TmxObject col;
          if (CheckCollisionTMXObjectGroupRec(map->layers[i].exact.objectGroup, player.getRect(), &col))
          {
            TraceLog(LOG_DEBUG, "We've made contact!");
            
            Vector2 newVel = player.getVelocity();
            newVel.y = 0;
            player.setVelocity(newVel);

            Rectangle newRect = player.getRect();
            newRect.y = (col.aabb.y - newRect.height);
            player.setRect(newRect);

            if (player.isJumping()) {
                player.land();
            }
          }
      }
    }
    return;
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
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "2D Game");

    // Define floor level to match where the non-zero tiles (floor tiles) are in Room1.tmx
    // This value is used for all characters to ensure consistent vertical positioning
    const float floorLevel = 10000.0f; // Exact floor level matching the non-zero floor tiles in TMX
    const float floorHeight = 50.0f; // Height of the floor rectangle if needed
    
    // Loading the Background.
    Texture2D background = LoadTexture("maps/Dungeon_brick_wall_purple.png.png");

    // Background Scale Factors.
    float scalebgx = (float)screenWidth / (float)background.width;
    float scalebgy = (float)screenHeight / (float)background.height;
    float scalebg = (scalebgx < scalebgy) ? scalebgx : scalebgy;
    scalebg /= 4.5f;

    // Positions for Background Positions.
    float bgposX = ((screenWidth - background.width * scalebg) / 2) - 600;
    float bgposY = ((screenHeight - background.height * scalebg) / 2) - 210;
    
    // Horizontal and Vertical Sliders for Background.
    int scaledW = background.width * scalebg;
    int scaledH = background.height * scalebg;
    int tilesX = (screenWidth / scaledW) + 50;
    int tilesY = (screenHeight / scaledH) + 15;
    
    // Initialize audio device before loading music
    InitAudioDevice();

    // Load Music.
    backgroundMusic = LoadMusicStream("music/Lady Maria of the Astral Clocktower.mp3");
    menuMusic = LoadMusicStream("music/Soul Of Cinder.mp3");
    
    SetTargetFPS(60);
    
    // Initialize camera
    camera.target = (Vector2){ 100, 0 };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 3.3f;  // Zoom in for better visibility.

    // Initialize characters using stack allocation - all characters now use the same floorLevel
    Samurai samurai(400, 2223, floorLevel);
    
    // Initialize dash sound volume to match master volume
    samurai.setDashSoundVolume(0.8f * masterVolume);

    StartScreen startScreen;
    GameState gameState = START_SCREEN;

    loadLevel();

    // Start playing menu music initially
    PlayMusicStream(menuMusic);
    SetMusicVolume(menuMusic, 0.5f * masterVolume);
    bool isPlayingMenuMusic = true;

    // Check which map are we in
    bool mapSwitchedToRoom2 = false;

    bool mapSwitchedToRoom3 = false;
    bool mapSwitchedToRoom4 = false;

    
    // Create a demon for Room2
    Demon* demon = nullptr;

    // Game loop
    while (!WindowShouldClose()) {
        // Update currently playing music
        UpdateMusicStream(isPlayingMenuMusic ? menuMusic : backgroundMusic);

        // Handle game state updates based on current game state
        switch(gameState) {
            case START_SCREEN: {
                if (!isPlayingMenuMusic) {
                    PlayMusicStream(menuMusic);
                    isPlayingMenuMusic = true;
                }

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
                if (isPlayingMenuMusic) {
                    PlayMusicStream(backgroundMusic);
                    isPlayingMenuMusic = false;
                }

                if (IsKeyPressed(KEY_P)) {
                    isPaused = !isPaused;
                }
                
                // Toggle music with M key
                if (IsKeyPressed(KEY_M)) {
                    if (IsMusicStreamPlaying(backgroundMusic)) {
                        PauseMusicStream(backgroundMusic);
                    } else {
                        ResumeMusicStream(backgroundMusic);
                    }
                }

                // Toggle collision box visibility with F1 key
                if (IsKeyPressed(KEY_F1)) {
                    showCollisionBoxes = !showCollisionBoxes;
                    printf("Collision boxes visibility: %s\n", showCollisionBoxes ? "ON" : "OFF");
                }

                // Get frame time for updates
                float deltaTime = GetFrameTime();

                if (!isPaused && !isComplete) {
                    // Update samurai character
                    samurai.updateSamurai();
                }

                samurai.deathBarrier();

                // Get samurai position for collision detection
                Vector2 samuraiPos = {0, 0};
                CollisionBox* samuraiBody = samurai.getCollisionBox(BODY);
                if (samuraiBody && samuraiBody->active) {
                    samuraiPos.x = samuraiBody->rect.x + samuraiBody->rect.width / 2;
                    samuraiPos.y = samuraiBody->rect.y + samuraiBody->rect.height / 2;
                }
                
                // Check for collisions between Samurai's attack and enemies
                CollisionBox* samuraiAttack = samurai.getCollisionBox(ATTACK);
                
                // Check for enemy attacks hitting Samurai
                CollisionBox* samuraiHurtbox = samurai.getCollisionBox(HURTBOX);

                checkTileCollisions(map, samurai);

                // Update camera to follow player, ensuring it stays within map boundaries
                Rectangle samuraiRect = samurai.getRect();
                
                // NOTE: Maybe delete this?
                float samuraiCenterX = samuraiRect.x + samuraiRect.width / 2;
                float samuraiCenterY = samuraiRect.y + samuraiRect.height / 2;
                
                if (!samurai.checkDeath()) {
                    camera.target = (Vector2){ samuraiPos.x, samuraiPos.y };
                    // Add some camera boundary checks to avoid the camera going out of bounds:
                    float halfScreenWidth = screenWidth / (2.0f * camera.zoom);
                    float halfScreenHeight = screenHeight / (2.0f * camera.zoom);
    
                    // Clamp X and Y positions with easing towards boundaries
                    camera.target.x = Lerp(camera.target.x, Clamp(camera.target.x, halfScreenWidth, map->width * map->tileWidth - halfScreenWidth), 0.1f);
                    camera.target.y = Lerp(camera.target.y, Clamp(camera.target.y, halfScreenHeight, map->height * map->tileHeight - halfScreenHeight), 0.1f);
    
                    // Ensure camera doesn't go out of bounds
                    if (camera.target.x < halfScreenWidth) camera.target.x = halfScreenWidth + 100;
                    if (camera.target.y < halfScreenHeight) camera.target.y = halfScreenHeight;
    
                    // Camera zoom controls
                    if (camera.target.x < halfScreenWidth) camera.target.x = halfScreenWidth;
                    if (camera.target.x > map->width * 16 - halfScreenWidth) camera.target.x = map->width * 16 - halfScreenWidth;
                    if (camera.target.y < halfScreenHeight) camera.target.y = halfScreenHeight;
                    if (camera.target.y > map->height * 16 - halfScreenHeight) camera.target.y = map->height * 16 - halfScreenHeight;
                } else {
                    // Stop moving the camera when the player is dead
                    camera.target = camera.target; // Keeps the camera locked in place
                }
                
                // Switching map :o
                // Using a wider range check for the portal to make it easier to trigger
                if (!mapSwitchedToRoom2 && 
                    samuraiRect.x > 900 && samuraiRect.x < 970 && 
                    samuraiRect.y > 1500 && samuraiRect.y < 1570) 
                {
                    // Debug output to confirm portal detection
                    printf("Portal to Room2 detected! Player position: %.2f, %.2f\n", samuraiRect.x, samuraiRect.y);
                    startTransition([&]() 
                    {
                        mapSwitchedToRoom2 = true;

                        if (map) 
                        {
                            UnloadTMX(map);
                        }

                    map = LoadTMX("maps/Room2.tmx");
                    if (!map) 
                    {
                        std::cerr << "Failed to load Room2.tmx!" << std::endl;
                    }

                    Rectangle newPos = samurai.getRect();
                    newPos.x = 700;  // Moved further away from the portal (was 550)
                    newPos.y = 2222;
                    samurai.setRect(newPos);

                    camera.target = { newPos.x, newPos.y };
                    
                    // Create demon in Room2
                    if (demon == nullptr) {
                        Vector2 demonPos = { 1000.0f, 2165.0f }; // Position the demon in Room2 at same ground level as samurai
                        demon = new Demon(demonPos, 50.0f, 500);
                        std::cout << "Demon spawned in Room2" << std::endl;
                    }
                    });
                }

                // Using a wider range check for the return portal to make it easier to trigger
                if (mapSwitchedToRoom2 && 
                    samuraiRect.x > 520 && samuraiRect.x < 580 && 
                    samuraiRect.y > 2180 && samuraiRect.y < 2240) 
                {
                    printf("Portal back to LevelDesign detected! Player position: %.2f, %.2f\n", samuraiRect.x, samuraiRect.y);
                    startTransition([&]() 
                    {
                        mapSwitchedToRoom3 = false; // Exiting Room 3
                        UnloadTMX(map); // Unload current map (Room3)
                        map = LoadTMX("maps/LevelDesign.tmx"); // Load the main level
                        if (!map) {
                            printf("Failed to Load TMX File: LevelDesign.tmx\n");
                            safeExit(); // Exit if loading fails
                        } else {
                            printf("Loaded TMX File: LevelDesign.tmx\n");
                        }


                    map = LoadTMX("maps/LevelDesign.tmx");
                    if (!map) 
                    {
                        std::cerr << "Failed to load LevelDesign.tmx!" << std::endl;
                    }

                    Rectangle newPos = samurai.getRect();
                    newPos.x = 500;
                    newPos.y = 2222;
                    samurai.setRect(newPos);

                    camera.target = { newPos.x, newPos.y };
                    
                    // Clean up demon when leaving Room2
                    if (demon != nullptr) {
                        delete demon;
                        demon = nullptr;
                        std::cout << "Demon removed when leaving Room2" << std::endl;
                    }
                    });  

                }
                // Portal to Room 3
                else if (!mapSwitchedToRoom3 && 
                         samuraiRect.x > 2500 && samuraiRect.x < 2510 && 
                         samuraiRect.y > 2185 && samuraiRect.y < 2195) // Check Y coordinate around 2190
                {
                    printf("Portal to Room3 detected! Player position: %.2f, %.2f\n", samuraiRect.x, samuraiRect.y);
                    startTransition([&]() 
                    {
                        mapSwitchedToRoom3 = true; // Set flag for Room 3
                        mapSwitchedToRoom2 = false; // Reset Room 2 flag if needed

                        if (map) 
                        {
                            UnloadTMX(map);
                        }
                        map = LoadTMX("maps/Room3.tmx"); // Load Room 3
                        if (!map) {
                            fprintf(stderr, "Failed to load Room3.tmx\n");
                            // Handle error appropriately, maybe exit or load a default map
                        } else {
                            printf("Loaded Room3.tmx successfully.\n");
                        }
                        
                        Rectangle newPos = samurai.getRect();
                        newPos.x = 100; // Reset player position for Room3 (adjust as needed)
                        newPos.y = 2223;
                        samurai.setRect(newPos);
                        
                        // Reset or remove enemies from previous room if necessary
                        if (demon) {
                            delete demon;
                            demon = nullptr;
                            printf("Demon deleted when entering Room3.\n");
                        }
                        // Add logic for enemies specific to Room 3 here if needed
                    });
                }
                // Portal back from Room 3 to LevelDesign
                else if (mapSwitchedToRoom3 && 
                         samuraiRect.x > 1550 && samuraiRect.x < 1555 && 
                         samuraiRect.y > 2170 && samuraiRect.y < 2180)
                {
                    printf("Portal back to LevelDesign detected! Player position: %.2f, %.2f\n", samuraiRect.x, samuraiRect.y);
                    startTransition([&]() 
                    {
                        mapSwitchedToRoom3 = false; // Exiting Room 3
                        UnloadTMX(map); // Unload current map (Room3)
                        map = LoadTMX("maps/LevelDesign.tmx"); // Load the main level
                        if (!map) {
                            printf("Failed to Load TMX File: LevelDesign.tmx\n");
                            safeExit(); // Exit if loading fails
                        } else {
                            printf("Loaded TMX File: LevelDesign.tmx\n");
                        }
                        Rectangle newPos = samurai.getRect();
                        newPos.x = 400;
                        newPos.y = 2223;
                        samurai.setRect(newPos); // Reset samurai position to a known point in LevelDesign
                        // Immediately update camera target to prevent visual jump
                        camera.target = (Vector2){ samurai.getRect().x + samurai.getRect().width / 2, samurai.getRect().y + samurai.getRect().height / 2 };
                    });
                }
                
                // Using a wider range check for the return portal to make it easier to trigger
                if (mapSwitchedToRoom2 && 
                    samuraiRect.x > 520 && samuraiRect.x < 580 && 
                    samuraiRect.y > 2180 && samuraiRect.y < 2240) 
                {
                    // Debug output to confirm return portal detection
                    printf("Return portal detected! Player position: %.2f, %.2f\n", samuraiRect.x, samuraiRect.y);
                    startTransition([&]() 
                    {
                        mapSwitchedToRoom2 = false;

                        if (map) 
                        {
                            UnloadTMX(map);
                        }

                    map = LoadTMX("maps/LevelDesign.tmx");
                    if (!map) 
                    {
                        std::cerr << "Failed to load LevelDesign.tmx!" << std::endl;
                    }

                    Rectangle newPos = samurai.getRect();
                    newPos.x = 500;
                    newPos.y = 2222;
                    samurai.setRect(newPos);

                    camera.target = { newPos.x, newPos.y };

                    });  
                }
                // Room 4 ==> Main Room 
                if (mapSwitchedToRoom4 && samuraiRect.x >= 3075 && samuraiRect.x <= 3085 && samuraiRect.y >= 2200.00 && samuraiRect.y <= 2210.00) 
                {
                    startTransition([&]() 
                    {
                        mapSwitchedToRoom4 = false;

                        if (map) 
                        {
                            UnloadTMX(map);
                        }

                    map = LoadTMX("maps/LevelDesign.tmx");
                    if (!map) 
                    {
                        std::cerr << "Failed to load LevelDesign.tmx!" << std::endl;
                    }

                    Rectangle newPos = samurai.getRect();
                    newPos.x = 9385;
                    newPos.y = 2062.25;
                    samurai.setRect(newPos);

                    camera.target = { newPos.x, newPos.y };
                    });  
                }

                
                if(samurai.checkDeath()) {
                    gameover = true;
                }
                
                if(samurai.getRect().x >= 18880) {
                    isComplete = true;
                }
                
                // Begin drawing
                BeginDrawing();
                ClearBackground(BLACK);
                
                // 2D camera mode for proper drawing
                BeginMode2D(camera);
                
                // Draw Background.
                for (int x = 0; x < tilesX; x++) {
                    for (int y = 0; y < tilesY; y++) {
                        float posX = x * scaledW;
                        float posY = y * scaledH;
                        
                        DrawTextureEx(background, Vector2{posX + bgposX, posY + bgposY}, 0.0f, scalebg, GRAY);
                    }
                }
                
                renderLevel();
                
                // Draw Samurai.
                samurai.draw();
                
                // Update and draw demon if in Room2
                if (mapSwitchedToRoom2 && demon != nullptr) 
                {
                    
                    // Update demon AI behavior
                    if (!demon->isDead && !isPaused) 
                    {
                        demon->update(deltaTime, samuraiPos);
                    }
                    // Update demon animation
                    demon->updateAnimation();
                    demon->updateCollisionBoxes();
                    
                    // Draw the demon
                    demon->draw();
                    
                    // Check for collision between Samurai's attack and demon
                    CollisionBox* samuraiAttack = samurai.getCollisionBox(ATTACK);
                    if (samuraiAttack && samuraiAttack->active) {
                        for (auto& box : demon->collisionBoxes) {
                            if (box.type == HURTBOX && box.active) {
                                if (checkCharacterCollision(*samuraiAttack, box)) {
                                    demon->takeDamage(25); // Samurai deals 25 damage
                                    //samuraiAttack->active = false; // Prevent multiple hits
                                    break;
                                }
                            }
                        }
                    }
                    
                    // Check for collision between demon's attack and Samurai
                    CollisionBox* samuraiHurtbox = samurai.getCollisionBox(HURTBOX);
                    for (auto& box : demon->collisionBoxes) {
                        if (box.type == ATTACK && box.active) {
                            if (samuraiHurtbox && samuraiHurtbox->active) {
                                if (checkCharacterCollision(box, *samuraiHurtbox)) {
                                    // Check if samurai is blocking to reduce damage
                                    if (samurai.isBlocking()) {
                                        // Apply damage reduction when blocking (half damage)
                                        int reducedDamage = static_cast<int>(15 * samurai.getBlockDamageReduction());
                                        samurai.takeDamage(reducedDamage);
                                        std::cout << "Blocked attack! Reduced damage: " << reducedDamage << std::endl;
                                    } else {
                                        samurai.takeDamage(15); // Full damage when not blocking
                                    }
                                    box.active = false; // Prevent multiple hits
                                    break;
                                }
                            }
                        }
                    }
                }

                debugTimer += deltaTime;
                if (debugTimer >= debugInterval) 
                {
                    std::cout << "X: " << samurai.getRect().x << std::endl;
                    std::cout << "Y: " << samurai.getRect().y << std::endl;
                    debugTimer = 0.0f;
                }

                // End camera mode and finalize drawing
                EndMode2D();

                // UI controls instructions
                int instructionsY = screenHeight - 1050;
                int lineHeight = 25;

                DrawText("GAME CONTROLS:", 10, instructionsY, 20, WHITE);
                DrawText("W or Up: Jump ", 10, instructionsY + lineHeight, 20, WHITE);
                DrawText("A/D or Left/Right: Move", 10, instructionsY + lineHeight*2, 20, WHITE);
                DrawText("Space: Attack", 10, instructionsY + lineHeight*3, 20, WHITE);
                DrawText("Double-tap A/D: Dash", 10, instructionsY + lineHeight*4, 20, WHITE);
                DrawText("M: Toggle music", 10, instructionsY + lineHeight*5, 20, WHITE);
                DrawText("P: Pause", 10, instructionsY + lineHeight*6, 20, WHITE);
                
                if (isPaused) {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
                    DrawText("PAUSED", GetScreenWidth()/2 - 50, GetScreenHeight()/2 - 10, 30, WHITE);
                    DrawText("Press 'P' to resume", GetScreenWidth()/2 - 100, GetScreenHeight()/2 + 30, 20, WHITE);

                    // Define the exit button area
                    Rectangle exitButton = { static_cast<float>(GetScreenWidth()) / 2 - 75, 
                        static_cast<float>(GetScreenHeight()) / 2 + 60, 
                        150.0f, 40.0f };


                    // Draw the exit button
                    DrawRectangleRec(exitButton, DARKGRAY);
                    DrawText("Exit", GetScreenWidth()/2 - 20, GetScreenHeight()/2 + 70, 20, WHITE);

                    // Check if the exit button is clicked
                    if (CheckCollisionPointRec(GetMousePosition(), exitButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        safeExit();
                    }

                    samurai.pauseSounds();
                } else if(isComplete) {
                    // Draw completion screen with improved UI layout
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f)); // Darker background for emphasis

                    // Centered text
                    const int centerX = GetScreenWidth() / 2;
                    const int centerY = GetScreenHeight() / 2;

                    DrawText("GAME COMPLETED!", centerX - MeasureText("GAME COMPLETED!", 40) / 2, centerY - 100, 40, GOLD);
                    DrawText("Congratulations!", centerX - MeasureText("Congratulations!", 30) / 2, centerY - 50, 30, WHITE);
                    DrawText("Press 'E' to exit", centerX - MeasureText("Press 'E' to exit", 20) / 2, centerY + 20, 20, LIGHTGRAY);

                    // Define the exit button
                    Rectangle exitButton = { static_cast<float>(centerX - 100), 
                                            static_cast<float>(centerY + 60), 
                                            200.0f, 50.0f };

                    // Draw the exit button with a hover effect
                    Color exitButtonColor = CheckCollisionPointRec(GetMousePosition(), exitButton) ? LIGHTGRAY : DARKGRAY;
                    DrawRectangleRec(exitButton, exitButtonColor);
                    DrawText("Exit", centerX - MeasureText("Exit", 25) / 2, centerY + 75, 25, WHITE);

                    // Check if the exit button is clicked
                    if (CheckCollisionPointRec(GetMousePosition(), exitButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (startScreen.ShouldStartGame()) {
                            safeExit();
                        }
                    }

                    // Handle key press for exiting
                    if (IsKeyPressed(KEY_E)) {
                        if (startScreen.ShouldStartGame()) {
                            safeExit();
                        }
                    }
                } else if(gameover) {
                    // Draw Game Over screen
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.8f)); // Dark background overlay

                    // Center coordinates
                    const int centerX = GetScreenWidth() / 2;
                    const int centerY = GetScreenHeight() / 2;

                    // Draw Game Over text
                    DrawText("GAME OVER", centerX - MeasureText("GAME OVER", 50) / 2, centerY - 100, 50, RED);
                    DrawText("Better luck next time!", centerX - MeasureText("Better luck next time!", 25) / 2, centerY - 50, 25, WHITE);

                    // Define the Exit button
                    Rectangle exitButton = { static_cast<float>(centerX - 100), 
                                            static_cast<float>(centerY + 30), 
                                            200.0f, 50.0f };

                    // Draw the Exit button with hover effect
                    Color exitButtonColor = CheckCollisionPointRec(GetMousePosition(), exitButton) ? LIGHTGRAY : DARKGRAY;
                    DrawRectangleRec(exitButton, exitButtonColor);
                    DrawText("Exit", centerX - MeasureText("Exit", 25) / 2, centerY + 45, 25, WHITE);

                    // Check if the Exit button is clicked
                    if (CheckCollisionPointRec(GetMousePosition(), exitButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        safeExit(); // Close the game window
                    }

                    // Handle key press for exiting
                    if (IsKeyPressed(KEY_E)) {
                        safeExit();
                    }
                }
                else 
                {
                    samurai.resumeSound();
                }
                
                if (isTransitioning) 
                {
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, transitionAlpha));
                    if (!transitionFadeIn) 
                    {
                        transitionAlpha += 0.02f;
                        if (transitionAlpha >= 1.0f) 
                        {
                            transitionAlpha = 1.0f;

                            if (transitionAction) 
                            {
                                transitionAction();  // run the map change
                            }

                            transitionFadeIn = true;
                        }
                    } 
                    else 
                    {
                        transitionAlpha -= 0.02f;
                        if (transitionAlpha <= 0.0f) 
                        {
                            transitionAlpha = 0.0f;
                            isTransitioning = false;
                        }
                    }
                }
                
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

    // This code should never be reached because we call safeExit() when WindowShouldClose() is true
    return 0;
}

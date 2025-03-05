#include "raylib.h"
#include "Character.h"
#include "Werewolf.h"
#include "Wizard.h"
#include "Demon.h"
#include <iostream>

// Game states
enum GameState {
    TITLE,
    PLAYING,
    GAME_OVER
};

// Global variables
bool showCollisionBoxes = false; // Toggle for collision visualization

int main() {
    // Initialize window
    InitWindow(800, 600, "Samurai vs Goblin");
    
    // Initialize audio device before loading music
    InitAudioDevice(); 

    // Load background music
    Music backgroundMusic = LoadMusicStream("music/Lady Maria of the Astral Clocktower.mp3");
    PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.5f);

    // Load floor and wall textures
    Texture2D floorTexture = LoadTexture("assets/tiles/floor_tile.png");
    Texture2D wallTexture = LoadTexture("assets/tiles/wall_tile.png");
    float tileSize = 64.0f;  // Size of each tile
    int numTilesX = (800 / tileSize) + 1;  // Number of tiles needed horizontally
    float floorY = 480.0f;  // Y position of the floor
    int numTilesY = ((600 - floorY) / tileSize) + 1;  // Number of tiles needed vertically from floor to bottom
    int numWallTilesY = (floorY / tileSize);  // Number of tiles needed vertically for walls

    // Creating characters
    Samurai samurai((Vector2) {300, floorY - 128});
    samurai.loadTextures();
    samurai.loadSounds();

    Goblin goblin((Vector2) {500, floorY - 64});
    goblin.loadTextures();

    // Creating Werewolf
    Werewolf werewolf((Vector2) {600, floorY - 128});
    werewolf.loadTextures();

    // Creating Wizard
    Wizard wizard((Vector2) {200, floorY - 128});
    wizard.loadTextures();

    // Creating Demon
    Demon demon((Vector2) {400, floorY - 128});
    demon.loadAnimations();

    GameState gameState = TITLE;
    float gameOverTimer = 0;
    const float RESTART_DELAY = 3.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Update background music stream
        UpdateMusicStream(backgroundMusic);

        switch(gameState) {
            case TITLE:
                if (IsKeyPressed(KEY_ENTER)) {
                    gameState = PLAYING;
                }
                break;
                
            case PLAYING:
                // Toggle collision box visualization with F1 key
                if (IsKeyPressed(KEY_F1)) {
                    showCollisionBoxes = !showCollisionBoxes;
                }
                
                // Update character states and movement
                if (samurai.isAlive() && goblin.isAlive()) {
                    samurai.move();
                    goblin.move();
                    werewolf.move();
                    wizard.move();
                    demon.move();

                    // Store previous positions
                    Vector2 prevSamuraiPos = { samurai.rect.x, samurai.rect.y };
                    Vector2 prevGoblinPos = { goblin.rect.x, goblin.rect.y };

                    // Apply velocity
                    samurai.applyVelocity();
                    goblin.applyVelocity();
                    werewolf.applyVelocity();
                    wizard.applyVelocity();
                    demon.applyVelocity();
                    
                    // Update combat states
                    samurai.updateCombatState(deltaTime);
                    goblin.updateCombatState(deltaTime);

                    // Handle collisions and combat
                    if(checkCharacterCollision(samurai.rect, goblin.rect)) {
                        // Handle attack-based collisions first
                        if (samurai.state == ATTACK && !goblin.isInvulnerable) {
                            goblin.takeDamage(samurai.attackDamage);
                            goblin.rect.x = prevGoblinPos.x;
                            goblin.rect.y = prevGoblinPos.y;
                            goblin.velocity.x = samurai.direction * 200.0f; // Knockback
                        }
                        else if((goblin.state == ATTACK_CLUB || goblin.state == ATTACK_STOMP || goblin.state == ATTACK_AOE) 
                                && !samurai.isInvulnerable) {
                            samurai.takeDamage(goblin.attackDamage);
                            samurai.rect.x = prevSamuraiPos.x;
                            samurai.rect.y = prevSamuraiPos.y;
                            samurai.velocity.x = goblin.direction * 200.0f; // Knockback
                        }
                        else { // Normal collision resolution
                            float overlapX = (samurai.rect.width + goblin.rect.width) / 2.0f - 
                                fabsf((samurai.rect.x + samurai.rect.width/2.0f) - (goblin.rect.x + goblin.rect.width/2.0f));
                            
                            float overlapY = (samurai.rect.height + goblin.rect.height) / 2.0f - 
                                fabsf((samurai.rect.y + samurai.rect.height/2.0f) - (goblin.rect.y + goblin.rect.height/2.0f));

                            bool horizontalCollision = overlapX < overlapY;
                            
                            if (horizontalCollision) {
                                if (samurai.rect.x < goblin.rect.x) {
                                    samurai.rect.x -= overlapX / 2.0f;
                                    goblin.rect.x += overlapX / 2.0f;
                                } else {
                                    samurai.rect.x += overlapX / 2.0f;
                                    goblin.rect.x -= overlapX / 2.0f;
                                }
                            } else {
                                if (samurai.rect.y < goblin.rect.y) {
                                    samurai.rect.y -= overlapY / 2.0f;
                                    goblin.rect.y += overlapY / 2.0f;
                                } else {
                                    samurai.rect.y += overlapY / 2.0f;
                                    goblin.rect.y -= overlapY / 2.0f;
                                }
                            }
                        }
                    }
                }

                // Check for game over
                if (!samurai.isAlive() || !goblin.isAlive()) {
                    gameState = GAME_OVER;
                    gameOverTimer = RESTART_DELAY;
                }
                break;
                
            case GAME_OVER:
                gameOverTimer -= deltaTime;
                if (gameOverTimer <= 0 && IsKeyPressed(KEY_R)) {
                    // Reset game
                    samurai = Samurai((Vector2){300, floorY - 128});
                    goblin = Goblin((Vector2){500, floorY - 64});
                    samurai.loadTextures();
                    goblin.loadTextures();
                    gameState = PLAYING;
                }
                break;
        }

        // Drawing
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

        switch(gameState) {
            case TITLE:
                DrawText("Samurai vs Goblin", 250, 200, 40, WHITE);
                DrawText("Controls:", 300, 280, 20, WHITE);
                DrawText("A/D - Move Left/Right", 300, 310, 20, WHITE);
                DrawText("W - Jump", 300, 340, 20, WHITE);
                DrawText("SPACE - Attack", 300, 370, 20, WHITE);
                DrawText("F1 - Toggle Collision Boxes", 300, 400, 20, WHITE);
                DrawText("Press ENTER to start", 270, 430, 30, YELLOW);
                break;
                
            case PLAYING:
            case GAME_OVER:
                // Draw characters
                samurai.draw();
                goblin.draw();
                werewolf.draw();
                wizard.draw();
                demon.draw();
                
                // Draw health bars
                samurai.drawHealthBar();
                goblin.drawHealthBar();
                werewolf.drawHealthBar();
                wizard.drawHealthBar();
                demon.drawHealthBar();
                
                // Draw hit feedback
                if (samurai.isInvulnerable) {
                    DrawText("!", (int)samurai.rect.x + samurai.rect.width/2, (int)samurai.rect.y - 40, 40, RED);
                }
                if (goblin.isInvulnerable) {
                    DrawText("!", (int)goblin.rect.x + goblin.rect.width/2, (int)goblin.rect.y - 40, 40, RED);
                }
                
                // Update animations
                samurai.updateAnimation();
                goblin.updateAnimation();
                werewolf.updateAnimation();
                wizard.updateAnimation();
                demon.updateAnimation();
                
                if (gameState == GAME_OVER) {
                    const char* message = !samurai.isAlive() ? "GAME OVER - Goblin Wins!" : "VICTORY - Samurai Wins!";
                    DrawText(message, 250, 250, 30, !samurai.isAlive() ? RED : GREEN);
                    if (gameOverTimer <= 0) {
                        DrawText("Press R to restart", 280, 300, 25, YELLOW);
                    }
                }
                
                // Display collision box toggle status
                if (gameState == PLAYING) {
                    DrawText(TextFormat("Collision Boxes: %s (F1)", showCollisionBoxes ? "ON" : "OFF"), 
                             10, 10, 20, WHITE);
                }
                break;
        }

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
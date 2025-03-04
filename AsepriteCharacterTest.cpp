#include "raylib.h"
#include "GoblinAseprite.h"
#include "SamuraiAseprite.h"
#include "GameConstants.h"
#include <iostream>

// Game states
enum GameState {
    TITLE,
    PLAYING,
    GAME_OVER
};

// Function to check character collision
bool checkCharacterCollision(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

int main() {
    // Initialize window
    InitWindow(800, 600, "Samurai vs Goblin - Aseprite Edition");
    
    // Define floor parameters
    Rectangle floor = { 0, 500, 800, 100 };
    const float GROUND_LEVEL = floor.y;

    // Creating characters
    SamuraiAseprite samurai((Vector2) {300, GROUND_LEVEL - 128});
    GoblinAseprite goblin((Vector2) {500, GROUND_LEVEL - 100});
    
    // Load animations
    bool samuraiLoaded = samurai.loadAnimations("assets/Samurai/exported/samurai_sheet.png", 
                                               "assets/Samurai/exported/samurai_sheet.json");
    bool goblinLoaded = goblin.loadAnimations("assets/Goblin/exported/goblin_sheet.png", 
                                             "assets/Goblin/exported/goblin_sheet.json");

    GameState gameState = TITLE;
    float gameOverTimer = 0;
    const float RESTART_DELAY = 3.0f;
    
    // Debug mode flag
    bool debugMode = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Toggle debug mode with F1
        if (IsKeyPressed(KEY_F1)) {
            debugMode = !debugMode;
            samurai.setDebugBoxes(debugMode);
            goblin.setDebugBoxes(debugMode);
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw floor
        DrawRectangleRec(floor, DARKGRAY);
        
        switch(gameState) {
            case TITLE:
                // Display title screen
                DrawText("SAMURAI VS GOBLIN", 200, 150, 40, MAROON);
                DrawText("Aseprite Animation Edition", 250, 200, 20, DARKGRAY);
                
                // Display animation status
                if (!samuraiLoaded || !goblinLoaded) {
                    DrawText("WARNING: Some animations could not be loaded!", 150, 250, 20, RED);
                    DrawText("Make sure to export your Aseprite animations first:", 150, 280, 18, RED);
                    DrawText("1. Run ./export_aseprite.sh assets/Samurai/Idle.aseprite", 160, 310, 16, DARKGRAY);
                    DrawText("2. Run ./export_aseprite.sh assets/Goblin/Hobgoblin Idle/Hobgoblin Idle 1.aseprite", 160, 330, 16, DARKGRAY);
                    DrawText("3. Check the exported/ directories for the output files", 160, 350, 16, DARKGRAY);
                }
                
                DrawText("Press ENTER to start", 300, 400, 20, DARKGRAY);
                
                if (IsKeyPressed(KEY_ENTER)) {
                    gameState = PLAYING;
                }
                break;
                
            case PLAYING:
                // Update character states and movement
                if (samurai.isAlive() && goblin.isAlive()) {
                    // Update characters
                    samurai.update(deltaTime);
                    goblin.update(deltaTime);

                    // Store previous positions
                    Rectangle prevSamuraiRect = samurai.getRect();
                    Rectangle prevGoblinRect = goblin.getRect();

                    // Apply velocity
                    samurai.applyVelocity(deltaTime);
                    goblin.applyVelocity(deltaTime);
                    
                    // Handle movement collisions using collision boxes
                    if(checkCharacterCollision(samurai.getCollisionBox(), goblin.getCollisionBox())) {
                        // Basic collision resolution
                        samurai.setPosition((Vector2){prevSamuraiRect.x, prevSamuraiRect.y});
                        goblin.setPosition((Vector2){prevGoblinRect.x, prevGoblinRect.y});
                    }
                    
                    // Handle attack collisions using hit boxes
                    // Samurai attacking Goblin
                    if (samurai.getState() == ATTACK_SAMURAI && 
                        !goblin.getIsInvulnerable() && 
                        samurai.getHitBox().width > 0 && 
                        checkCharacterCollision(samurai.getHitBox(), goblin.getCollisionBox())) {
                        
                        goblin.takeDamage(samurai.getAttackDamage());
                        goblin.setVelocity((Vector2){(float)samurai.getDirection() * 200.0f, -100.0f}); // Knockback with slight upward
                    }
                    
                    // Goblin attacking Samurai
                    if ((goblin.getState() == ATTACK_CLUB_GOBLIN || 
                         goblin.getState() == ATTACK_STOMP_GOBLIN || 
                         goblin.getState() == ATTACK_AOE_GOBLIN) && 
                        !samurai.getIsInvulnerable() && 
                        goblin.getHitBox().width > 0 && 
                        checkCharacterCollision(goblin.getHitBox(), samurai.getCollisionBox())) {
                        
                        // Check if Samurai is parrying
                        if (samurai.getState() == PARRY_SAMURAI) {
                            // Parry successful - reflect damage back to goblin
                            goblin.takeDamage(goblin.getAttackDamage() / 2);
                            goblin.setVelocity((Vector2){(float)samurai.getDirection() * 300.0f, -150.0f}); // Stronger knockback
                        } else {
                            // Normal hit
                            samurai.takeDamage(goblin.getAttackDamage());
                            samurai.setVelocity((Vector2){(float)goblin.getDirection() * 200.0f, -100.0f}); // Knockback with slight upward
                        }
                    }
                    
                    // Check if game is over
                    if (!samurai.isAlive() || !goblin.isAlive()) {
                        gameState = GAME_OVER;
                        gameOverTimer = RESTART_DELAY;
                    }
                }
                
                // Draw characters
                samurai.draw();
                goblin.draw();
                
                // Draw controls
                DrawText("Controls: A/D - Move, W - Jump, SPACE - Attack, E - Parry", 10, 10, 20, DARKGRAY);
                
                // Draw debug mode status
                if (debugMode) {
                    DrawText("DEBUG MODE ON (F1 to toggle)", 10, 40, 20, RED);
                    DrawText("Green: Collision Box, Red: Hit Box, Blue: Character Rect", 10, 70, 16, DARKGRAY);
                } else {
                    DrawText("Press F1 for debug mode", 10, 40, 16, DARKGRAY);
                }
                break;
                
            case GAME_OVER:
                // Update game over timer
                gameOverTimer -= deltaTime;
                
                // Draw characters
                samurai.draw();
                goblin.draw();
                
                // Display game over message
                const char* message = !samurai.isAlive() ? "GAME OVER - Goblin Wins!" : "VICTORY - Samurai Wins!";
                DrawText(message, 250, 250, 30, !samurai.isAlive() ? RED : GREEN);
                
                if (gameOverTimer <= 0) {
                    DrawText("Press R to restart", 280, 300, 25, YELLOW);
                    
                    if (IsKeyPressed(KEY_R)) {
                        // Reset characters
                        samurai = SamuraiAseprite((Vector2) {300, GROUND_LEVEL - 128});
                        goblin = GoblinAseprite((Vector2) {500, GROUND_LEVEL - 100});
                        
                        // Reload animations
                        samuraiLoaded = samurai.loadAnimations("assets/Samurai/exported/samurai_sheet.png", 
                                                              "assets/Samurai/exported/samurai_sheet.json");
                        goblinLoaded = goblin.loadAnimations("assets/Goblin/exported/goblin_sheet.png", 
                                                            "assets/Goblin/exported/goblin_sheet.json");
                        
                        // Preserve debug mode setting
                        samurai.setDebugBoxes(debugMode);
                        goblin.setDebugBoxes(debugMode);
                        
                        gameState = PLAYING;
                    }
                }
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
} 
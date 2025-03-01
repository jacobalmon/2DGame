#include "raylib.h"
#include "Character.h"
#include <iostream>

// Game states
enum GameState {
    TITLE,
    PLAYING,
    GAME_OVER
};

int main() {
    // Initialize window
    InitWindow(800, 600, "Samurai vs Goblin");
    
    // Define floor parameters
    Rectangle floor = { 0, 500, 800, 100 };
    const float GROUND_LEVEL = floor.y;

    // Creating characters
    Samurai samurai((Vector2) {300, GROUND_LEVEL - 128});
    samurai.loadTextures();

    Goblin goblin((Vector2) {500, GROUND_LEVEL - 100});
    goblin.loadTextures();

    GameState gameState = TITLE;
    float gameOverTimer = 0;
    const float RESTART_DELAY = 3.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        switch(gameState) {
            case TITLE:
                if (IsKeyPressed(KEY_ENTER)) {
                    gameState = PLAYING;
                }
                break;
                
            case PLAYING:
                // Update character states and movement
                if (samurai.isAlive() && goblin.isAlive()) {
                    samurai.move();
                    goblin.move();

                    // Store previous positions
                    Vector2 prevSamuraiPos = { samurai.rect.x, samurai.rect.y };
                    Vector2 prevGoblinPos = { goblin.rect.x, goblin.rect.y };

                    // Apply velocity
                    samurai.applyVelocity();
                    goblin.applyVelocity();
                    
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
                    samurai = Samurai((Vector2){300, GROUND_LEVEL - 128});
                    goblin = Goblin((Vector2){500, GROUND_LEVEL - 100});
                    samurai.loadTextures();
                    goblin.loadTextures();
                    gameState = PLAYING;
                }
                break;
        }

        // Drawing
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // Draw the floor
        DrawRectangleRec(floor, DARKGREEN);

        switch(gameState) {
            case TITLE:
                DrawText("Samurai vs Goblin", 250, 200, 40, WHITE);
                DrawText("Controls:", 300, 280, 20, WHITE);
                DrawText("A/D - Move Left/Right", 300, 310, 20, WHITE);
                DrawText("W - Jump", 300, 340, 20, WHITE);
                DrawText("SPACE - Attack", 300, 370, 20, WHITE);
                DrawText("Press ENTER to start", 270, 430, 30, YELLOW);
                break;
                
            case PLAYING:
            case GAME_OVER:
                // Draw characters
                samurai.draw();
                goblin.draw();
                
                // Draw health bars
                samurai.drawHealthBar();
                goblin.drawHealthBar();
                
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
                
                if (gameState == GAME_OVER) {
                    const char* message = !samurai.isAlive() ? "GAME OVER - Goblin Wins!" : "VICTORY - Samurai Wins!";
                    DrawText(message, 250, 250, 30, !samurai.isAlive() ? RED : GREEN);
                    if (gameOverTimer <= 0) {
                        DrawText("Press R to restart", 280, 300, 25, YELLOW);
                    }
                }
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
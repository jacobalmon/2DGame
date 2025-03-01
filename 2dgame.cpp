#include "raylib.h"
#include "Character.h"
#include <iostream>

int main() {
    // Initialize window
    InitWindow(800, 600, "2D Game");

    // Define floor parameters
    Rectangle floor = { 0, 500, 800, 100 };  // x, y, width, height
    const float GROUND_LEVEL = floor.y;  // Ground level for characters

    // Creating Samurai at ground level
    Samurai samurai((Vector2) {300, GROUND_LEVEL - 128});  // Adjust Y position based on character height
    samurai.loadTextures();

    // Creating Goblin at ground level
    Goblin goblin((Vector2) {500, GROUND_LEVEL - 100});  // Adjust Y position based on character height
    goblin.loadTextures();

    // Set FPS at 60
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Update character states and movement
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

        // Begin drawing
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // Draw the floor
        DrawRectangleRec(floor, DARKGREEN);

        // Handle collisions and combat
        if(checkCharacterCollision(samurai.rect, goblin.rect))
        {
            // Handle attack-based collisions first
            if (samurai.state == ATTACK && !goblin.isInvulnerable)
            {
                goblin.takeDamage(samurai.attackDamage);
                goblin.rect.x = prevGoblinPos.x;
                goblin.rect.y = prevGoblinPos.y;
                goblin.velocity.x = samurai.direction * 200.0f; // Knockback
            }
            else if((goblin.state == ATTACK_CLUB || goblin.state == ATTACK_STOMP || goblin.state == ATTACK_AOE) 
                    && !samurai.isInvulnerable)
            {
                samurai.takeDamage(goblin.attackDamage);
                samurai.rect.x = prevSamuraiPos.x;
                samurai.rect.y = prevSamuraiPos.y;
                samurai.velocity.x = goblin.direction * 200.0f; // Knockback
            }
            else // Normal collision resolution
            {
                // Calculate horizontal overlap
                float overlapX = (samurai.rect.width + goblin.rect.width) / 2.0f - 
                    fabsf((samurai.rect.x + samurai.rect.width/2.0f) - (goblin.rect.x + goblin.rect.width/2.0f));
                
                // Calculate vertical overlap
                float overlapY = (samurai.rect.height + goblin.rect.height) / 2.0f - 
                    fabsf((samurai.rect.y + samurai.rect.height/2.0f) - (goblin.rect.y + goblin.rect.height/2.0f));

                // Determine collision direction
                bool horizontalCollision = overlapX < overlapY;
                
                if (horizontalCollision)
                {
                    // Resolve horizontal collision
                    if (samurai.rect.x < goblin.rect.x) 
                    {
                        samurai.rect.x -= overlapX / 2.0f;
                        goblin.rect.x += overlapX / 2.0f;
                    } 
                    else 
                    {
                        samurai.rect.x += overlapX / 2.0f;
                        goblin.rect.x -= overlapX / 2.0f;
                    }
                }
                else
                {
                    // Resolve vertical collision
                    if (samurai.rect.y < goblin.rect.y) 
                    {
                        samurai.rect.y -= overlapY / 2.0f;
                        goblin.rect.y += overlapY / 2.0f;
                    } 
                    else 
                    {
                        samurai.rect.y += overlapY / 2.0f;
                        goblin.rect.y -= overlapY / 2.0f;
                    }
                }
            }
        }

        // Draw characters
        samurai.draw();
        goblin.draw();
        
        // Draw health bars
        samurai.drawHealthBar();
        goblin.drawHealthBar();
        
        // Draw hit feedback
        if (samurai.isInvulnerable) {
            DrawText("INVULNERABLE!", (int)samurai.rect.x, (int)samurai.rect.y - 40, 20, RED);
        }
        if (goblin.isInvulnerable) {
            DrawText("INVULNERABLE!", (int)goblin.rect.x, (int)goblin.rect.y - 40, 20, RED);
        }
        
        // Check for defeat
        if (!samurai.isAlive()) {
            DrawText("GAME OVER - Goblin Wins!", 300, 250, 30, RED);
        }
        if (!goblin.isAlive()) {
            DrawText("VICTORY - Samurai Wins!", 300, 250, 30, GREEN);
        }

        samurai.updateAnimation();
        goblin.updateAnimation();

        EndDrawing();
    }

    // De-Initialization
    CloseWindow();
    return 0;
} 
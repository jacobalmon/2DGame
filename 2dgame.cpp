#include <raylib.h>
#include <iostream>
#include "Samurai.h"
#include "Goblin.h"

bool checkCharacterCollision(const Rectangle& rect1, const Rectangle& rect2)
{
  return CheckCollisionRecs(rect1,rect2);
}
int main() 
{
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

    // Set FPS at 60.
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        samurai.move();
        goblin.move();

        //previous positions
        Vector2 prevSamuraiPos = { samurai.rect.x, samurai.rect.y };
        Vector2 prevGoblinPos = { goblin.rect.x, goblin.rect.y };

        //apply velocity
        samurai.applyVelocity();
        goblin.applyVelocity();

        // Begin drawing
        BeginDrawing();
        ClearBackground(SKYBLUE);  // Sky blue background

        // Draw the floor
        DrawRectangleRec(floor, DARKGREEN);

        // Draw the characters
        samurai.draw();  // Draw samurai
        goblin.draw();   // Draw goblin

        if(checkCharacterCollision(samurai.rect,goblin.rect))
        {
            // Handle attack-based collisions first
            if (samurai.state == ATTACK)
            {
                goblin.rect.x = prevGoblinPos.x;
                goblin.rect.y = prevGoblinPos.y;
                goblin.velocity.x = samurai.direction * 200.0f; // Knockback in samurai's facing direction
            }
            else if(goblin.state == ATTACK_CLUB || goblin.state == ATTACK_STOMP || goblin.state == ATTACK_AOE)
            {
                samurai.rect.x = prevSamuraiPos.x;
                samurai.rect.y = prevSamuraiPos.y;
                samurai.velocity.x = goblin.direction * 200.0f; // Knockback in goblin's facing direction
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
                        
                        // Reset horizontal velocities
                        if (samurai.velocity.x > 0) samurai.velocity.x = 0;
                        if (goblin.velocity.x < 0) goblin.velocity.x = 0;
                    } 
                    else 
                    {
                        samurai.rect.x += overlapX / 2.0f;
                        goblin.rect.x -= overlapX / 2.0f;
                        
                        // Reset horizontal velocities
                        if (samurai.velocity.x < 0) samurai.velocity.x = 0;
                        if (goblin.velocity.x > 0) goblin.velocity.x = 0;
                    }
                }
                else
                {
                    // Resolve vertical collision
                    if (samurai.rect.y < goblin.rect.y) 
                    {
                        samurai.rect.y -= overlapY / 2.0f;
                        goblin.rect.y += overlapY / 2.0f;
                        
                        // Reset vertical velocities
                        if (samurai.velocity.y > 0) samurai.velocity.y = 0;
                        if (goblin.velocity.y < 0) goblin.velocity.y = 0;
                    } 
                    else 
                    {
                        samurai.rect.y += overlapY / 2.0f;
                        goblin.rect.y -= overlapY / 2.0f;
                        
                        // Reset vertical velocities
                        if (samurai.velocity.y < 0) samurai.velocity.y = 0;
                        if (goblin.velocity.y > 0) goblin.velocity.y = 0;
                    }
                }
            }
        }
        
        samurai.updateAnimation();
        goblin.updateAnimation();

        EndDrawing();  // End drawing and display the frame
    }
    

    CloseWindow();
    return 0;
}


#pragma once

#include "raylib.h"
#include "CollisionSystem.h"
#include <vector>
#include <cmath>   // For sinf
#include <cstdlib> // For system
#include <unistd.h> // For getcwd()
#include <limits.h> // For PATH_MAX
#include <cstdio>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Define PI if not already defined
#ifndef PI
#define PI 3.14159265358979323846f
#endif

// Direction of the Character.
enum Direction {
    LEFT = -1,
    RIGHT = 1
};

// States for the Character. (Note: not all will be used).
enum CurrentState {
    DEAD_STATE = 0,
    ATTACK_STATE = 1,
    HURT_STATE = 2,
    IDLE_STATE = 3,
    JUMP_STATE = 4,
    RUN_STATE = 5
};

// Animation types.
enum AnimationType {
    LOOP,    // Animation loops continuously.
    ONESHOT  // Animation plays once and stops at the last frame.
};

// Animation data structure.
struct Animation {
    int firstFrame;    // First frame of the animation.
    int lastFrame;     // Last frame of the animation.
    int currentFrame;  // Current frame being displayed.
    float timer;       // Timer for frame changes.
    float frameTime;   // Time per frame.
    float speed;       // Animation speed.
    AnimationType type; // Type of animation (loop or oneshot).
};

class Samurai {
private:
    Rectangle rect; // Character's rectangle for position and size.
    Vector2 velocity; // Velocity of the character for movement.
    Direction direction; // Current facing direction of the character.
    CurrentState state; // Current state of the character (e.g., idle, attack, etc.).
    std::vector<Animation> animations; // List of animations for different states.
    std::vector<Texture2D> sprites; // List of textures for each state.
    float groundLevel; // The Y-coordinate of the ground level.

    // Define Health Variables
    int maxHealth = 100; // Maximum health value.
    int currentHealth = 100; // Current health of the samurai.
    bool wasInAir = false; // Flag to indicate if the character was in the air.
    
    // Double dash variables
    float dashSpeed = 15.0f; // Speed multiplier when dashing
    float dashDuration = 0.3f; // How long the dash lasts in seconds
    float dashTimer = 0.0f; // Timer to track current dash duration
    float dashCooldown = 0.5f; // Cooldown period between dashes in seconds
    float dashCooldownTimer = 0.0f; // Timer to track cooldown
    float lastAKeyPressTime = 0.0f; // Time when A key was last pressed
    float lastDKeyPressTime = 0.0f; // Time when D key was last pressed
    float doubleTapTimeThreshold = 0.3f; // Maximum time between taps to count as double tap
    bool canDash = true; // Flag to determine if dash is available
    float dashSoundVolume = 0.8f; // Volume for dash sound (0.0 to 1.0)
    
    // Invincibility frames variables
    bool isInvincible = false; // Flag to indicate if the character is currently invincible
    float invincibilityTimer = 0.0f; // Timer to track invincibility duration
    const float invincibilityDuration = 1.5f; // 1.5 seconds of invincibility after taking damage

    // Define Sound Varaibles.
    Sound attackSound;
    Sound jumpSound;
    Sound hurtSound;
    Sound runSound;
    Sound deadSound;
    Sound landSound;
    Sound dashSound;

    // Collision boxes for different purposes
    std::vector<CollisionBox> collisionBoxes;

    // Helper method to update the animation frame.
    void updateAnimation(float deltaTime) {
        // Safety check for valid state
        if (state < 0 || state >= sprites.size()) {
            state = IDLE_STATE;
        }
        
        Animation &anim = animations[state];
        anim.timer += deltaTime;
        
        // Handle special case for HURT_STATE
        if (state == HURT_STATE && anim.currentFrame >= anim.lastFrame) {
            state = IDLE_STATE;
            animations[state].currentFrame = 0;
            anim.timer = 0;
        }
        
        // Update animation frame based on timer
        if (anim.timer >= anim.frameTime) {
            anim.timer = 0;
            
            // For JUMP_STATE, don't increment frame if we're at the last frame
            if (state == JUMP_STATE && anim.currentFrame >= anim.lastFrame) {
                // Keep the last frame until landing
            } else {
                // Increment frame
                anim.currentFrame++;
                
                // Handle frame looping or state transitions
                if (anim.currentFrame > anim.lastFrame) {
                    if (anim.type == LOOP) {
                        anim.currentFrame = 0;
                    } else if (anim.type == ONESHOT) {
                        anim.currentFrame = anim.lastFrame;
                        
                        // For ATTACK_STATE, transition back to IDLE_STATE when animation completes
                        if (state == ATTACK_STATE) {
                            state = IDLE_STATE;
                            animations[state].currentFrame = 0;
                        }
                    }
                }
            }
        }
    }

    // Helper method to get the current animation frame rectangle.
    Rectangle getAnimationFrame() const {
        // Safety check for valid state
        if (state < 0 || state >= sprites.size() || state >= animations.size()) {
            return Rectangle{0, 0, 128, 128}; // Return a default frame
        }
        
        // Get the sprite dimensions
        int spriteWidth = sprites[state].width;
        
        // Safety check for valid sprite
        if (spriteWidth <= 0) {
            return Rectangle{0, 0, 128, 128}; // Return a default frame
        }
        
        // Calculate the number of frames in the sprite sheet
        int framesPerRow = spriteWidth / 128;
        
        // Safety check for valid frames calculation
        if (framesPerRow <= 0) {
            return Rectangle{0, 0, 128, 128}; // Return a default frame
        }
        
        // Get the current frame from the animation
        int currentFrame = animations[state].currentFrame;
        
        // Safety check for valid frame
        if (currentFrame < 0) {
            currentFrame = 0; // Reset to first frame
        }
        
        // Ensure current frame doesn't exceed the last frame
        if (currentFrame > animations[state].lastFrame) {
            currentFrame = animations[state].lastFrame;
        }
        
        // Calculate the position of the frame in the sprite sheet
        int frameX = (currentFrame % framesPerRow) * 128;
        int frameY = (currentFrame / framesPerRow) * 128;
        
        return Rectangle{(float)frameX, (float)frameY, 128, 128};
    }

    // Helper method to handle movement input.
    void move(float deltaTime) {
        // Update dash cooldown timer
        if (!canDash) {
            dashCooldownTimer -= deltaTime;
            if (dashCooldownTimer <= 0.0f) {
                canDash = true;
                dashCooldownTimer = 0.0f;
            }
        }
        
        // Update dash timer if currently dashing
        if (isDashing) {
            dashTimer -= deltaTime;
            if (dashTimer <= 0.0f) {
                isDashing = false;
                dashTimer = 0.0f;
            }
        }
        
        // Reset velocity if not jumping.
        if (rect.y >= groundLevel) {
            velocity.y = 0;
            rect.y = groundLevel;  // Ensure character is on ground.
            
            // Only transition from JUMP_STATE to IDLE_STATE when landing
            if (state == JUMP_STATE) {
                state = IDLE_STATE;
                animations[state].currentFrame = 0;  // Reset animation frame
                if (wasInAir) {
                    if (landSound.frameCount > 0) {
                        PlaySound(landSound);
                    }
                    wasInAir = false;
                }
            }
        }

        // Check for jump input.
        if (IsKeyPressed(KEY_W) && state != ATTACK_STATE) {
            StopSound(runSound);
            if (rect.y >= groundLevel) {
                velocity.y = -15.0f;  // Apply upward velocity.
                if (jumpSound.frameCount > 0) {
                    PlaySound(jumpSound);
                }
                wasInAir = true;
                
                if (state != HURT_STATE && state != DEAD_STATE) {
                    state = JUMP_STATE;
                    animations[state].currentFrame = 0;
                }
            }
        }        

        // Apply gravity.
        if (rect.y < groundLevel) {
            velocity.y += 0.5f;  // Gravity effect.
            
            // Maintain JUMP_STATE while in the air unless in special states
            if (state != JUMP_STATE && state != ATTACK_STATE && state != HURT_STATE && state != DEAD_STATE) {
                state = JUMP_STATE;
                animations[state].currentFrame = 0;  // Reset animation frame
            }
        }
        
        // Current time for double tap detection
        float currentTime = GetTime();
        
        // Handle left/right movement with double tap dash
        if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && state != ATTACK_STATE) {
            PlaySound(runSound);
            if (canDash && (currentTime - lastAKeyPressTime) <= doubleTapTimeThreshold) {
                isDashing = true;
                dashTimer = dashDuration;
                canDash = false;
                dashCooldownTimer = dashCooldown;
                direction = LEFT;
                playDashSound();
                StopSound(runSound);
            }
            lastAKeyPressTime = currentTime;
        }        
        
        if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && state != ATTACK_STATE) {
            PlaySound(runSound);
            if (canDash && (currentTime - lastDKeyPressTime) <= doubleTapTimeThreshold) {
                isDashing = true;
                dashTimer = dashDuration;
                canDash = false;
                dashCooldownTimer = dashCooldown;
                direction = RIGHT;
                playDashSound();
                StopSound(runSound);
            }
            lastDKeyPressTime = currentTime;
        }        
        
        // Handle movement based on key press and dash state
        if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && !isDashing && state != ATTACK_STATE) {
            velocity.x = -5.0f;  // Move left normally
            direction = LEFT;
            
            if (state != JUMP_STATE && state != HURT_STATE && state != DEAD_STATE) {
                state = RUN_STATE;
            }
        } else if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && !isDashing && state != ATTACK_STATE) {
            velocity.x = 5.0f;  // Move right normally
            direction = RIGHT;
            
            if (state != JUMP_STATE && state != HURT_STATE && state != DEAD_STATE) {
                state = RUN_STATE;
            }
        } else if (!isDashing && state != ATTACK_STATE) { // Only stop movement if not dashing
            velocity.x = 0;
            if (state == RUN_STATE) {
                state = IDLE_STATE;
                StopSound(runSound);
            }
        }

        // Apply dash movement
        if (isDashing) {
            velocity.x = (direction == RIGHT) ? dashSpeed : -dashSpeed;
        }

        if (wasInAir) {
            StopSound(runSound);
        }

        // Check for attack input.
        if (IsKeyPressed(KEY_SPACE) && state != ATTACK_STATE && state != HURT_STATE && state != DEAD_STATE) {
            velocity.x = 0;
            state = ATTACK_STATE;  // Set to attack state.
            animations[state].currentFrame = 0;  // Reset animation frame.
            if (attackSound.frameCount > 0) {
                PlaySound(attackSound);
                StopSound(runSound);
            }
        }

        // Apply velocity to position.
        applyVelocity();
    }

    // Helper method to apply velocity to position.
    void applyVelocity() {
        rect.x += velocity.x;  // Update horizontal position.
        rect.y += velocity.y;  // Update vertical position.

        // Ensure character stays within map bounds instead of screen bounds
        // Map dimensions should be passed from outside instead of hardcoded
        extern int mapWidth;  // Use the external map width from 2dgame.cpp
        const float mapWidthPixels = mapWidth * 16;  // Convert tiles to pixels
        
        if (rect.x < 0) rect.x = 0;
        if (rect.x > mapWidthPixels - rect.width) rect.x = mapWidthPixels - rect.width;
    }

    // Helper method to handle taking damage.
    void heal(int healingAmount) {
        currentHealth += healingAmount;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;  // Cap health at maximum.
        }
    }

    // Helper method to check for healing input.
    void checkForHealing() {
        if (IsKeyPressed(KEY_H)) {
            heal(10);  // Heal by 10 points.
        }
    }

    // Helper method to check for damage input (for testing).
    void checkForDamage() {
        if (IsKeyPressed(KEY_K)) {
            takeDamage(10);  // Take 10 damage.
        }
    }

    // Helper method to update the collision boxes of the character.
    void updateCollisionBoxes() {
        // Only update collision boxes if they exist.
        if (!collisionBoxes.empty()) {
            
            // Update the body collision box
            if (collisionBoxes.size() > 0) {
                // Base body box
                float bodyOffsetX = 15.0f * SPRITE_SCALE;
                float bodyOffsetY = 15.0f * SPRITE_SCALE;
                float bodyWidth = rect.width - (30.0f * SPRITE_SCALE);
                float bodyHeight = rect.height - (15.0f * SPRITE_SCALE);
                
                // If dashing, extend the collision box in the direction of movement
                if (isDashing) {
                    // Extend the box in the direction of dash
                    if (direction == RIGHT) {
                        bodyWidth += 10.0f * SPRITE_SCALE;
                    } else {
                        bodyOffsetX -= 10.0f * SPRITE_SCALE;
                        bodyWidth += 10.0f * SPRITE_SCALE;
                    }
                }
                
                collisionBoxes[0].rect = {rect.x + bodyOffsetX, rect.y + bodyOffsetY, bodyWidth, bodyHeight};
                collisionBoxes[0].active = (state != DEAD_STATE);
            }
            
            // Update attack collision box
            if (collisionBoxes.size() > 1) {
                float attackOffsetX = (direction == RIGHT) ? rect.width - (16.0f * SPRITE_SCALE) : 0;
                float attackOffsetY = 24.0f * SPRITE_SCALE;
                float attackSize = 32.0f * SPRITE_SCALE;
                
                collisionBoxes[1].rect = {rect.x + attackOffsetX, rect.y + attackOffsetY, attackSize, attackSize};
                collisionBoxes[1].active = (state == ATTACK_STATE && animations[state].currentFrame >= 2 && animations[state].currentFrame <= 4);
                
                // Update hurtbox collision box
                if (collisionBoxes.size() > 2) {
                    float hurtboxOffsetX = 20.0f * SPRITE_SCALE;
                    float hurtboxOffsetY = 20.0f * SPRITE_SCALE;
                    float hurtboxWidth = rect.width - (40.0f * SPRITE_SCALE);
                    float hurtboxHeight = rect.height - (24.0f * SPRITE_SCALE);
                    
                    collisionBoxes[2].rect = {rect.x + hurtboxOffsetX, rect.y + hurtboxOffsetY, hurtboxWidth, hurtboxHeight};
                    collisionBoxes[2].active = (state != DEAD_STATE);
                }
            }
        }
    }

    // Helper function to play the dash sound with proper volume
    void playDashSound() {
        if (dashSound.frameCount > 0) {
            // Play the sound (volume already set through setSoundVolumes)
            PlaySound(dashSound);
            
            // Debug output
            printf("Playing dash sound (volume: %.2f)\n", dashSoundVolume);
        } else {
            printf("Dash sound not loaded properly!\n");
        }
    }

    // Helper function to set volume for all sound effects
    void setSoundVolumes(float volume) {
        // Clamp volume between 0.0 and 1.0
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        
        // Store the dash sound volume for future reference
        dashSoundVolume = volume;
        
        // Set volume for all sound effects
        if (attackSound.frameCount > 0) SetSoundVolume(attackSound, volume);
        if (jumpSound.frameCount > 0) SetSoundVolume(jumpSound, volume);
        if (hurtSound.frameCount > 0) SetSoundVolume(hurtSound, volume);
        if (runSound.frameCount > 0) SetSoundVolume(runSound, volume);
        if (deadSound.frameCount > 0) SetSoundVolume(deadSound, volume);
        if (landSound.frameCount > 0) SetSoundVolume(landSound, volume);
        if (dashSound.frameCount > 0) SetSoundVolume(dashSound, volume);
    }

    void loadTextures() {
        sprites.resize(6);

        // Load textures for different states (idle, attack, etc.).
        sprites[DEAD_STATE] = LoadTexture("assets/Samurai/Dead.png");
        sprites[ATTACK_STATE] = LoadTexture("assets/Samurai/Attack_1.png");
        sprites[HURT_STATE] = LoadTexture("assets/Samurai/Hurt.png");
        sprites[IDLE_STATE] = LoadTexture("assets/Samurai/Idle.png");
        sprites[JUMP_STATE] = LoadTexture("assets/Samurai/Jump.png");
        sprites[RUN_STATE] = LoadTexture("assets/Samurai/Run.png");
    }

public:
    bool isDead = false; // Flag to indicate if the samurai is dead.
    bool showCollisionBoxes = false; // Flag to enable/disable collision box drawing
    bool isDashing = false; // Flag to indicate if the character is currently dashing
    
    // Accessor for dash sound volume (kept for compatibility)
    void setDashSoundVolume(float volume) {
        // Set volume for all sounds to maintain consistency
        setSoundVolumes(volume);
    }
    
    float getDashSoundVolume() const {
        return dashSoundVolume;
    }

    // Constructor initializing the Samurai's properties and animations
    Samurai(float x, float y, float groundLevel) {
        rect = (Rectangle){x, y, 64.0f * SPRITE_SCALE, 64.0f * SPRITE_SCALE}; // Scale the sprite size
        velocity = (Vector2){0.0f, 0.0f}; // Initialize velocity.
        direction = RIGHT; // Default facing direction.
        state = IDLE_STATE; // Start in idle state.
        this->groundLevel = groundLevel; // Set initial ground level.
        currentHealth = maxHealth = 100;
        isDead = false;
        wasInAir = false;

        // Load textures for each state.
        loadTextures();

        // Initialize animations for each state.
        animations = {
            {0, 2, 0, 0, 0.1f, 0.1f, ONESHOT},    // DEAD_STATE - Changed to use all 3 frames (0, 1, 2)
            {0, 5, 0, 0, 0.1f, 0.1f, ONESHOT},  // ATTACK_STATE
            {0, 1, 0, 0, 0.1f, 0.1f, ONESHOT},  // HURT_STATE
            {0, 5, 0, 0, 0.1f, 0.1f, LOOP},     // IDLE_STATE
            {0, 11, 0, 0, 0.1f, 0.1f, ONESHOT}, // JUMP_STATE - 12 frames (0-11) based on 1536/128 = 12
            {0, 7, 0, 0, 0.1f, 0.1f, LOOP}      // RUN_STATE
        };

        // Load sound effects with error checking
        attackSound = LoadSound("sounds/samurai/sword-sound-2-36274.wav");
        jumpSound = LoadSound("sounds/samurai/female-jump.wav");
        hurtSound = LoadSound("sounds/samurai/female-hurt-2-94301.wav");
        runSound = LoadSound("sounds/samurai/running-on-concrete-268478.wav");
        deadSound = LoadSound("sounds/samurai/female-death.wav");
        landSound = LoadSound("sounds/samurai/land2-43790.wav");
        dashSound = LoadSound("sounds/samurai/whoosh (phaser).wav");

        // Initialize collision boxes with scaled dimensions
        float bodyOffsetX = 16.0f * SPRITE_SCALE;
        float bodyOffsetY = 16.0f * SPRITE_SCALE;
        float bodyWidth = rect.width - (32.0f * SPRITE_SCALE);
        float bodyHeight = rect.height - (16.0f * SPRITE_SCALE);
        
        float attackOffsetX = rect.width - (16.0f * SPRITE_SCALE);
        float attackOffsetY = 24.0f * SPRITE_SCALE;
        float attackSize = 32.0f * SPRITE_SCALE;
        
        float hurtboxOffsetX = 20.0f * SPRITE_SCALE;
        float hurtboxOffsetY = 20.0f * SPRITE_SCALE;
        float hurtboxWidth = rect.width - (40.0f * SPRITE_SCALE);
        float hurtboxHeight = rect.height - (24.0f * SPRITE_SCALE);
        
        collisionBoxes = {
            CollisionBox({rect.x + bodyOffsetX, rect.y + bodyOffsetY, bodyWidth, bodyHeight}, BODY),
            CollisionBox({rect.x + attackOffsetX, rect.y + attackOffsetY, attackSize, attackSize}, ATTACK, false),
            CollisionBox({rect.x + hurtboxOffsetX, rect.y + hurtboxOffsetY, hurtboxWidth, hurtboxHeight}, HURTBOX)
        };

        // Initialize dash variables
        isDashing = false;
        dashTimer = 0.0f;
        canDash = true;
        dashCooldownTimer = 0.0f;
        dashSoundVolume = 0.8f; // Set default volume to 80%
    }

    // Destructor to clean up resources
    ~Samurai() {
        for (auto& sprite : sprites) {
            UnloadTexture(sprite); // Unload textures from memory when done.
        }
        
        // Unload sounds - make sure they're valid before unloading
        if (attackSound.frameCount > 0) UnloadSound(attackSound);
        if (jumpSound.frameCount > 0) UnloadSound(jumpSound);
        if (hurtSound.frameCount > 0) UnloadSound(hurtSound);
        if (runSound.frameCount > 0) UnloadSound(runSound);
        if (deadSound.frameCount > 0) UnloadSound(deadSound);
        if (landSound.frameCount > 0) UnloadSound(landSound);
        if (dashSound.frameCount > 0) UnloadSound(dashSound);
    }

    // Draw the character.
    void draw() const {
        if (sprites.size() <= state) {
            return; // Safety check
        }
        
        // Get the frame width for rendering
        float frameWidth = sprites[state].width / (animations[state].lastFrame + 1);
        
        // Calculate source rectangle for the current frame
        Rectangle source = {
            animations[state].currentFrame * frameWidth,
            0.0f,
            direction == RIGHT ? frameWidth : -frameWidth,
            (float)sprites[state].height
        };
        
        // Calculate destination rectangle for rendering
        Rectangle dest = {
            rect.x,
            rect.y,
            rect.width,
            rect.height
        };
        
        // Draw dash trail effect when dashing
        if (isDashing) {
            // Draw a few fading copies of the character behind the main sprite
            for (int i = 1; i <= 3; i++) {
                float offsetX = (direction == RIGHT) ? -i * 10.0f : i * 10.0f;
                Rectangle trailDest = {
                    rect.x + offsetX,
                    rect.y,
                    rect.width,
                    rect.height
                };
                
                // Calculate alpha based on distance (further = more transparent)
                float alpha = 0.7f - (i * 0.2f);
                Color trailTint = {255, 255, 255, (unsigned char)(alpha * 255)};
                
                DrawTexturePro(sprites[state], source, trailDest, (Vector2){0, 0}, 0.0f, trailTint);
            }
        }
        
        // Apply visual effect for invincibility frames
        Color tint = WHITE;
        if (isInvincible) {
            // Flash the character by alternating transparency
            float flashFrequency = 10.0f; // Higher value = faster flashing
            if (fmodf(invincibilityTimer * flashFrequency, 1.0f) > 0.5f) {
                tint = (Color){255, 255, 255, 128}; // Half transparent
            }
        }
        
        // Draw the sprite
        DrawTexturePro(sprites[state], source, dest, (Vector2){0, 0}, 0.0f, tint);
        
        // Draw collision boxes for debugging
        if (showCollisionBoxes) {
            for (const auto& box : collisionBoxes) {
                if (box.active) {
                    Color color;
                    switch (box.type) {
                        case BODY: color = BLUE; break;
                        case ATTACK: color = RED; break;
                        case HURTBOX: color = GREEN; break;
                    }
                    DrawRectangleLines(box.rect.x, box.rect.y, box.rect.width, box.rect.height, color);
                }
            }
        }
        
        // Draw health bar above the character instead of top left corner
        float healthBarWidth = rect.width;
        float healthBarHeight = 5.0f;
        float healthPercentage = (float)currentHealth / maxHealth;
        
        // Draw health bar background (red)
        DrawRectangle(
            rect.x, 
            rect.y - healthBarHeight - 5, 
            healthBarWidth, 
            healthBarHeight, 
            RED
        );
        
        // Draw current health (green)
        DrawRectangle(
            rect.x, 
            rect.y - healthBarHeight - 5, 
            healthBarWidth * healthPercentage, 
            healthBarHeight, 
            GREEN
        );
    }

    // Update the Samurai's state and position
    void updateSamurai() {
        float deltaTime = GetFrameTime();
        
        // Always update animation regardless of whether the samurai is dead or alive
        updateAnimation(deltaTime);
        
        // Update invincibility timer if active
        if (isInvincible) {
            invincibilityTimer -= deltaTime;
            if (invincibilityTimer <= 0.0f) {
                isInvincible = false;
                invincibilityTimer = 0.0f;
            }
        }
        
        // Only handle movement if the samurai is alive
        if (!isDead) {
            move(deltaTime);
        }
        
        // Always update collision boxes
        updateCollisionBoxes();
        
        // Always check for healing and damage
        checkForHealing();
        checkForDamage();
    }

    // Get the Samurai's rectangle for collision detection
    Rectangle getRect() const {
        return rect;
    }

    // Get the Samurai's current health
    int getHealth() const {
        return currentHealth;
    }

    // Get a collision box of a specific type
    CollisionBox* getCollisionBox(CollisionBoxType type) {
        for (auto& box : collisionBoxes) {
            if (box.type == type) {
                return &box;
            }
        }
        return nullptr;
    }

    // Take damage when hit by an enemy
    void takeDamage(int damage) {
        // Skip damage if currently invincible
        if (isInvincible) return;
            
        if (!isDead && state != HURT_STATE) {
            currentHealth -= damage;
            if (currentHealth <= 0) {
                currentHealth = 0;
                state = DEAD_STATE;
                isDead = true;
                animations[state].currentFrame = 0;  // Reset death animation to start from the beginning
                animations[state].timer = 0;  // Reset timer for smooth animation
                if (deadSound.frameCount > 0) {
                    PlaySound(deadSound);
                    StopSound(runSound);
                }
            } else {
                state = HURT_STATE;
                animations[state].currentFrame = 0;  // Reset hurt animation.
                if (hurtSound.frameCount > 0) {
                    PlaySound(hurtSound);
                    StopSound(runSound);
                }
                
                // Activate invincibility frames
                isInvincible = true;
                invincibilityTimer = invincibilityDuration;
            }
        }
    }
};
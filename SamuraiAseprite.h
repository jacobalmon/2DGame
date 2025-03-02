#ifndef SAMURAI_ASEPRITE_H
#define SAMURAI_ASEPRITE_H

#include "AsepriteLoader.h"
#include "raylib.h"
#include "GameConstants.h"
#include <string>
#include <map>
#include <iostream>

// Direction of the Samurai
enum DirectionSamuraiAseprite {
    LEFT_SAMURAI = -1,
    RIGHT_SAMURAI = 1
};

// States for the Samurai
enum CurrentStateSamuraiAseprite {
    DEAD_SAMURAI = 0,
    ATTACK_SAMURAI = 1,
    HURT_SAMURAI = 2,
    IDLE_SAMURAI = 3,
    JUMP_SAMURAI = 4,
    RUN_SAMURAI = 5,
    PARRY_SAMURAI = 6,
    WALK_SAMURAI = 7
};

class SamuraiAseprite {
private:
    AsepriteAnimation animations;
    Rectangle rect;                // Main character rectangle (for positioning)
    Rectangle collisionBox;        // Collision box for movement/physics
    Rectangle hitBox;              // Hit box for attacks
    Vector2 velocity;
    DirectionSamuraiAseprite direction;
    CurrentStateSamuraiAseprite state;
    float groundLevel;
    bool loaded;
    
    // Health and combat properties
    int maxHealth;
    int currentHealth;
    int attackDamage;
    bool isInvulnerable;
    float invulnerabilityTimer;
    float hitStunDuration;
    bool isHit;
    
    // Animation state tracking
    std::string currentAnimationTag;
    float attackCooldown;
    float attackTimer;
    
    // Jump properties
    float jumpVelocity;
    float jumpSpeed;
    bool isJumping;
    float gravity;
    
    // Debug visualization
    bool showDebugBoxes;

public:
    SamuraiAseprite(Vector2 position) : 
        rect({ position.x, position.y, 64.0f, 64.0f }),
        collisionBox({ position.x + 20.0f, position.y + 16.0f, 24.0f, 48.0f }),  // Smaller than visual size
        hitBox({ position.x, position.y, 64.0f, 64.0f }),                        // Default same as rect
        velocity({ 0.0f, 0.0f }),
        direction(RIGHT_SAMURAI),
        state(IDLE_SAMURAI),
        groundLevel(position.y),
        loaded(false),
        maxHealth(100),
        currentHealth(maxHealth),
        attackDamage(25),
        isInvulnerable(false),
        invulnerabilityTimer(0),
        hitStunDuration(0.5f),
        isHit(false),
        currentAnimationTag("idle"),
        attackCooldown(0.5f),
        attackTimer(0),
        jumpVelocity(0),
        jumpSpeed(-600.0f),
        isJumping(false),
        gravity(1500.0f),
        showDebugBoxes(false) {
    }

    ~SamuraiAseprite() {
        animations.Unload();
    }

    // Load samurai animations from PNG and JSON files
    bool loadAnimations(const std::string& pngPath, const std::string& jsonPath) {
        if (!animations.LoadFromFiles(pngPath.c_str(), jsonPath.c_str())) {
            std::cerr << "Failed to load samurai animations from: " << pngPath << std::endl;
            return false;
        }
        
        // Get available animation tags
        std::vector<std::string> tags = animations.GetAllTags();
        if (tags.empty()) {
            std::cerr << "No animation tags found in: " << jsonPath << std::endl;
            return false;
        }
        
        // Try to play the idle animation by default
        bool foundIdle = false;
        for (const auto& tag : tags) {
            if (tag == "idle" || tag == "IDLE") {
                animations.Play(tag);
                currentAnimationTag = tag;
                foundIdle = true;
                break;
            }
        }
        
        // If no idle animation, just play the first one
        if (!foundIdle) {
            animations.Play(tags[0]);
            currentAnimationTag = tags[0];
        }
        
        loaded = true;
        return true;
    }

    // Update samurai state and animations based on input
    void update(float deltaTime) {
        if (!loaded) return;
        
        // Update invulnerability timer
        if (isInvulnerable) {
            invulnerabilityTimer -= deltaTime;
            if (invulnerabilityTimer <= 0) {
                isInvulnerable = false;
            }
        }
        
        // Update hit stun
        if (isHit) {
            hitStunDuration -= deltaTime;
            if (hitStunDuration <= 0) {
                isHit = false;
                hitStunDuration = 0.5f;
                state = IDLE_SAMURAI;
                playAnimation("idle");
            }
        }
        
        // Update attack cooldown
        if (attackTimer > 0) {
            attackTimer -= deltaTime;
        }
        
        // Handle input for character movement and actions
        if (!isHit && state != DEAD_SAMURAI) {
            move();
        }
        
        // Apply physics
        applyVelocity(deltaTime);
        
        // Update animation
        animations.Update(deltaTime);
        
        // If attack animation finished, return to idle
        if (state == ATTACK_SAMURAI && !animations.IsPlaying()) {
            state = IDLE_SAMURAI;
            playAnimation("idle");
        }
        
        // If parry animation finished, return to idle
        if (state == PARRY_SAMURAI && !animations.IsPlaying()) {
            state = IDLE_SAMURAI;
            playAnimation("idle");
        }
        
        // Update collision and hit boxes based on current animation and state
        updateBoxes();
    }

    // Update collision and hit boxes based on current animation and state
    void updateBoxes() {
        // Update collision box position to follow the character
        collisionBox.x = rect.x + 20.0f;
        collisionBox.y = rect.y + 16.0f;
        
        // Update hit box based on current state
        if (state == ATTACK_SAMURAI) {
            // Extend hit box in the direction the samurai is facing for sword attack
            if (direction == RIGHT_SAMURAI) {
                hitBox = { rect.x + 40.0f, rect.y + 10.0f, 48.0f, 40.0f };
            } else {
                hitBox = { rect.x - 24.0f, rect.y + 10.0f, 48.0f, 40.0f };
            }
        } else if (state == PARRY_SAMURAI) {
            // Parry box is slightly larger than the collision box
            if (direction == RIGHT_SAMURAI) {
                hitBox = { rect.x + 10.0f, rect.y, 44.0f, 64.0f };
            } else {
                hitBox = { rect.x + 10.0f, rect.y, 44.0f, 64.0f };
            }
        } else {
            // Default hit box (no attack)
            hitBox = { 0, 0, 0, 0 };
        }
    }

    // Handle movement input
    void move() {
        float moveSpeed = 300.0f;
        velocity.x = 0.0f;
        
        if (IsKeyDown(KEY_A)) {
            velocity.x = -moveSpeed;
            direction = LEFT_SAMURAI;
            if (rect.y >= groundLevel && state != ATTACK_SAMURAI && state != PARRY_SAMURAI) {
                state = RUN_SAMURAI;
                playAnimation("run");
            }
        } else if (IsKeyDown(KEY_D)) {
            velocity.x = moveSpeed;
            direction = RIGHT_SAMURAI;
            if (rect.y >= groundLevel && state != ATTACK_SAMURAI && state != PARRY_SAMURAI) {
                state = RUN_SAMURAI;
                playAnimation("run");
            }
        } else {
            if (rect.y >= groundLevel && state != ATTACK_SAMURAI && state != PARRY_SAMURAI && state != JUMP_SAMURAI) {
                state = IDLE_SAMURAI;
                playAnimation("idle");
            }
        }
        
        // Attack
        if (IsKeyPressed(KEY_SPACE) && state != ATTACK_SAMURAI && attackTimer <= 0) {
            state = ATTACK_SAMURAI;
            playAnimation("attack");
            attackTimer = attackCooldown;
        }
        
        // Jump
        if (IsKeyPressed(KEY_W) && rect.y >= groundLevel && state != JUMP_SAMURAI) {
            state = JUMP_SAMURAI;
            playAnimation("jump");
            velocity.y = jumpSpeed;
            isJumping = true;
        }
        
        // Parry/Shield
        if (IsKeyPressed(KEY_E) && state != PARRY_SAMURAI) {
            state = PARRY_SAMURAI;
            playAnimation("parry");
        }
    }

    // Apply physics to the character
    void applyVelocity(float deltaTime) {
        rect.x += velocity.x * deltaTime;
        
        // Apply gravity if in the air
        if (rect.y < groundLevel) {
            velocity.y += gravity * deltaTime;
            rect.y += velocity.y * deltaTime;
            
            // Keep jump animation active while in air
            if (state != HURT_SAMURAI && state != DEAD_SAMURAI) {
                state = JUMP_SAMURAI;
            }
        } else {
            // Land on ground
            if (isJumping) {
                isJumping = false;
                if (state != HURT_SAMURAI && state != DEAD_SAMURAI && state != ATTACK_SAMURAI && state != PARRY_SAMURAI) {
                    state = IDLE_SAMURAI;
                    playAnimation("idle");
                }
            }
            
            velocity.y = 0.0f;
            rect.y = groundLevel;
        }
        
        // Update collision and hit boxes
        updateBoxes();
    }

    // Draw the samurai
    void draw() {
        if (!loaded) return;
        
        // Draw with appropriate flipping based on direction
        animations.DrawEx(rect.x, rect.y, direction == LEFT_SAMURAI, CHARACTER_SCALE);
        
        // Draw health bar
        drawHealthBar();
        
        // Draw debug boxes if enabled
        if (showDebugBoxes) {
            // Draw collision box (green)
            DrawRectangleLinesEx(collisionBox, 1.0f, GREEN);
            
            // Draw hit box (red) if in attack state
            if (state == ATTACK_SAMURAI || state == PARRY_SAMURAI) {
                DrawRectangleLinesEx(hitBox, 1.0f, RED);
            }
            
            // Draw character rect (blue)
            DrawRectangleLinesEx(rect, 1.0f, BLUE);
        }
    }
    
    // Draw health bar above the samurai
    void drawHealthBar() {
        float healthBarWidth = 50.0f;
        float healthBarHeight = 5.0f;
        float healthPercentage = (float)currentHealth / maxHealth;
        
        // Background (red)
        DrawRectangle(
            rect.x + rect.width / 2 - healthBarWidth / 2,
            rect.y - 15,
            healthBarWidth,
            healthBarHeight,
            RED
        );
        
        // Foreground (green)
        DrawRectangle(
            rect.x + rect.width / 2 - healthBarWidth / 2,
            rect.y - 15,
            healthBarWidth * healthPercentage,
            healthBarHeight,
            GREEN
        );
    }
    
    // Play a specific animation
    void playAnimation(const std::string& animName) {
        if (currentAnimationTag == animName) return;
        
        if (animations.Play(animName)) {
            currentAnimationTag = animName;
        }
    }
    
    // Take damage
    void takeDamage(int damage) {
        if (!isInvulnerable && !isHit && currentHealth > 0 && state != PARRY_SAMURAI) {
            currentHealth -= damage;
            if (currentHealth <= 0) {
                currentHealth = 0;
                state = DEAD_SAMURAI;
                playAnimation("dead");
            } else {
                state = HURT_SAMURAI;
                playAnimation("hurt");
                isHit = true;
                isInvulnerable = true;
                invulnerabilityTimer = 1.0f;
            }
        }
    }
    
    // Check if samurai is alive
    bool isAlive() const {
        return currentHealth > 0;
    }
    
    // Get samurai rectangle for collision detection
    Rectangle getRect() const {
        return rect;
    }
    
    // Get collision box for movement/physics
    Rectangle getCollisionBox() const {
        return collisionBox;
    }
    
    // Get hit box for attacks
    Rectangle getHitBox() const {
        return hitBox;
    }
    
    // Get current state
    CurrentStateSamuraiAseprite getState() const {
        return state;
    }
    
    // Get direction
    DirectionSamuraiAseprite getDirection() const {
        return direction;
    }
    
    // Get attack damage
    int getAttackDamage() const {
        return attackDamage;
    }
    
    // Check if samurai is invulnerable
    bool getIsInvulnerable() const {
        return isInvulnerable;
    }
    
    // Set position
    void setPosition(Vector2 position) {
        rect.x = position.x;
        rect.y = position.y;
        groundLevel = position.y;
    }
    
    // Set velocity (for knockback)
    void setVelocity(Vector2 newVelocity) {
        velocity = newVelocity;
    }
    
    // Toggle debug box visualization
    void toggleDebugBoxes() {
        showDebugBoxes = !showDebugBoxes;
    }
    
    // Set debug box visualization
    void setDebugBoxes(bool show) {
        showDebugBoxes = show;
    }
};

#endif // SAMURAI_ASEPRITE_H 
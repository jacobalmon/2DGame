#ifndef GOBLIN_ASEPRITE_H
#define GOBLIN_ASEPRITE_H

#include "AsepriteLoader.h"
#include "raylib.h"
#include "GameConstants.h"
#include <string>
#include <map>
#include <iostream>

// Direction of the Goblin
enum DirectionGoblinAseprite {
    LEFT_GOBLIN = -1,
    RIGHT_GOBLIN = 1
};

// States for the Goblin
enum CurrentStateGoblinAseprite {
    DEAD_GOBLIN = 0,
    ATTACK_CLUB_GOBLIN = 1,
    ATTACK_STOMP_GOBLIN = 2,
    ATTACK_AOE_GOBLIN = 3,
    HURT_GOBLIN = 4,
    IDLE_GOBLIN = 5,
    WALK_GOBLIN = 6
};

class GoblinAseprite {
private:
    AsepriteAnimation animations;
    Rectangle rect;                // Main character rectangle (for positioning)
    Rectangle collisionBox;        // Collision box for movement/physics
    Rectangle hitBox;              // Hit box for attacks
    Vector2 velocity;
    DirectionGoblinAseprite direction;
    CurrentStateGoblinAseprite state;
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
    
    // Debug visualization
    bool showDebugBoxes;

public:
    GoblinAseprite(Vector2 position) : 
        rect({ position.x, position.y, 64.0f, 64.0f }),
        collisionBox({ position.x + 16.0f, position.y + 16.0f, 32.0f, 48.0f }),  // Smaller than visual size
        hitBox({ position.x, position.y, 64.0f, 64.0f }),                        // Default same as rect
        velocity({ 0.0f, 0.0f }),
        direction(RIGHT_GOBLIN),
        state(IDLE_GOBLIN),
        groundLevel(position.y),
        loaded(false),
        maxHealth(100),
        currentHealth(maxHealth),
        attackDamage(15),
        isInvulnerable(false),
        invulnerabilityTimer(0),
        hitStunDuration(0.5f),
        isHit(false),
        currentAnimationTag("idle"),
        attackCooldown(1.0f),
        attackTimer(0),
        showDebugBoxes(false) {
    }

    ~GoblinAseprite() {
        animations.Unload();
    }

    // Load goblin animations from PNG and JSON files
    bool loadAnimations(const std::string& pngPath, const std::string& jsonPath) {
        if (!animations.LoadFromFiles(pngPath.c_str(), jsonPath.c_str())) {
            std::cerr << "Failed to load goblin animations from: " << pngPath << std::endl;
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

    // Update goblin state and animations
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
                state = IDLE_GOBLIN;
                playAnimation("idle");
            }
        }
        
        // Update attack cooldown
        if (attackTimer > 0) {
            attackTimer -= deltaTime;
        }
        
        // Simple AI for the goblin
        if (!isHit && state != DEAD_GOBLIN) {
            // Random chance to change state
            if (GetRandomValue(0, 100) < 2) {
                int randomState = GetRandomValue(0, 10);
                
                if (randomState < 5) {
                    state = IDLE_GOBLIN;
                    playAnimation("idle");
                } else if (randomState < 8) {
                    state = WALK_GOBLIN;
                    playAnimation("walk");
                    // Random direction
                    direction = GetRandomValue(0, 1) ? RIGHT_GOBLIN : LEFT_GOBLIN;
                    velocity.x = 100.0f * (float)direction;
                } else if (attackTimer <= 0) {
                    // Random attack type
                    int attackType = GetRandomValue(0, 2);
                    if (attackType == 0) {
                        state = ATTACK_CLUB_GOBLIN;
                        playAnimation("attack_club");
                    } else if (attackType == 1) {
                        state = ATTACK_STOMP_GOBLIN;
                        playAnimation("attack_stomp");
                    } else {
                        state = ATTACK_AOE_GOBLIN;
                        playAnimation("attack_aoe");
                    }
                    attackTimer = attackCooldown;
                }
            }
            
            // Move if walking
            if (state == WALK_GOBLIN) {
                rect.x += velocity.x * deltaTime;
            } else {
                velocity.x = 0;
            }
        }
        
        // Update animation
        animations.Update(deltaTime);
        
        // If attack animation finished, return to idle
        if ((state == ATTACK_CLUB_GOBLIN || state == ATTACK_STOMP_GOBLIN || state == ATTACK_AOE_GOBLIN) 
            && !animations.IsPlaying()) {
            state = IDLE_GOBLIN;
            playAnimation("idle");
        }
        
        // Update collision and hit boxes based on current animation and state
        updateBoxes();
    }

    // Update collision and hit boxes based on current animation and state
    void updateBoxes() {
        // Update collision box position to follow the character
        collisionBox.x = rect.x + 16.0f;
        collisionBox.y = rect.y + 16.0f;
        
        // Update hit box based on current state
        if (state == ATTACK_CLUB_GOBLIN) {
            // Extend hit box in the direction the goblin is facing for club attack
            if (direction == RIGHT_GOBLIN) {
                hitBox = { rect.x + 32.0f, rect.y, 64.0f, 64.0f };
            } else {
                hitBox = { rect.x - 32.0f, rect.y, 64.0f, 64.0f };
            }
        } else if (state == ATTACK_STOMP_GOBLIN) {
            // Extend hit box downward for stomp attack
            hitBox = { rect.x, rect.y + 32.0f, 64.0f, 32.0f };
        } else if (state == ATTACK_AOE_GOBLIN) {
            // Larger hit box for AOE attack
            hitBox = { rect.x - 16.0f, rect.y - 16.0f, 96.0f, 96.0f };
        } else {
            // Default hit box (no attack)
            hitBox = { 0, 0, 0, 0 };
        }
    }

    // Draw the goblin
    void draw() {
        if (!loaded) return;
        
        // Draw with appropriate flipping based on direction
        animations.DrawEx(rect.x, rect.y, direction == LEFT_GOBLIN, CHARACTER_SCALE);
        
        // Draw health bar
        drawHealthBar();
        
        // Draw debug boxes if enabled
        if (showDebugBoxes) {
            // Draw collision box (green)
            DrawRectangleLinesEx(collisionBox, 1.0f, GREEN);
            
            // Draw hit box (red) if in attack state
            if (state == ATTACK_CLUB_GOBLIN || state == ATTACK_STOMP_GOBLIN || state == ATTACK_AOE_GOBLIN) {
                DrawRectangleLinesEx(hitBox, 1.0f, RED);
            }
            
            // Draw character rect (blue)
            DrawRectangleLinesEx(rect, 1.0f, BLUE);
        }
    }
    
    // Draw health bar above the goblin
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
        if (!isInvulnerable && !isHit && currentHealth > 0) {
            currentHealth -= damage;
            if (currentHealth <= 0) {
                currentHealth = 0;
                state = DEAD_GOBLIN;
                playAnimation("dead");
            } else {
                state = HURT_GOBLIN;
                playAnimation("hurt");
                isHit = true;
                isInvulnerable = true;
                invulnerabilityTimer = 1.0f;
            }
        }
    }
    
    // Check if goblin is alive
    bool isAlive() const {
        return currentHealth > 0;
    }
    
    // Get goblin rectangle for collision detection
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
    CurrentStateGoblinAseprite getState() const {
        return state;
    }
    
    // Get direction
    DirectionGoblinAseprite getDirection() const {
        return direction;
    }
    
    // Get attack damage
    int getAttackDamage() const {
        return attackDamage;
    }
    
    // Check if goblin is invulnerable
    bool getIsInvulnerable() const {
        return isInvulnerable;
    }
    
    // Set position
    void setPosition(Vector2 position) {
        rect.x = position.x;
        rect.y = position.y;
    }
    
    // Apply velocity (for knockback)
    void applyVelocity(float deltaTime) {
        rect.x += velocity.x * deltaTime;
        
        // Apply gravity if in the air
        if (rect.y < groundLevel) {
            velocity.y += 500.0f * deltaTime;
            rect.y += velocity.y * deltaTime;
        } else {
            velocity.y = 0.0f;
            rect.y = groundLevel;
        }
        
        // Update collision and hit boxes
        updateBoxes();
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

#endif // GOBLIN_ASEPRITE_H 
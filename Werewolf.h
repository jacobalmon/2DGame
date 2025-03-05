#ifndef WEREWOLF_H
#define WEREWOLF_H

#include "raylib.h"
#include "Character.h"
#include <vector>

// Reference to the global collision box toggle
extern bool showCollisionBoxes;

enum DirectionWolf {
    LEFT_WOLF = -1,
    RIGHT_WOLF = 1
};

enum CurrentStateWolf {
    DEAD_WOLF = 0,
    ATTACK_SWIPE,
    ATTACK_RUN,
    HURT_WOLF,
    IDLE_WOLF,
    JUMP_WOLF,
    RUN_WOLF,
    WALK_WOLF
};

enum AnimationTypeWolf {
    REPEATING_WOLF,
    ONESHOT_WOLF
};

struct AnimationWolf {
    int firstFrame, lastFrame, currentFrame, offset;
    float speed, timeLeft;
    AnimationTypeWolf type;
};

const float GRAVITY = 800.0f;
const float JUMP_FORCE = -500.0f;
const float GROUND_LEVEL = 480.0f - 128.0f;  // Adjusted to match floor level

class Werewolf : public Character {
private:
    std::vector<Texture2D> sprites;
    std::vector<AnimationWolf> animations;
    CurrentStateWolf state;
    DirectionWolf direction;
    bool isDead;
    bool isOnGround;
    bool hasFinishedAttack;

public:
    Werewolf(Vector2 position) : Character(position) {
        rect.width = 64;
        rect.height = 128;
        velocity = {0, 0};
        direction = RIGHT_WOLF;
        state = IDLE_WOLF;
        currentHealth = 150;
        maxHealth = 150;
        attackDamage = 20;
        isDead = false;
        isOnGround = true;
        hasFinishedAttack = true;
        
        // Initialize animations
        sprites.resize(8);
        animations.resize(8);
        
        // Place on floor
        placeOnFloor();
    }
    
    ~Werewolf() {
        for (auto& sprite : sprites) {
            UnloadTexture(sprite);
        }
    }
    
    void loadTextures() override {
        sprites[DEAD_WOLF] = LoadTexture("assets/Werewolf/Dead.png");
        sprites[ATTACK_SWIPE] = LoadTexture("assets/Werewolf/Attack_2.png");
        sprites[ATTACK_RUN] = LoadTexture("assets/Werewolf/Run+Attack.png");
        sprites[HURT_WOLF] = LoadTexture("assets/Werewolf/Hurt.png");
        sprites[IDLE_WOLF] = LoadTexture("assets/Werewolf/Idle.png");
        sprites[JUMP_WOLF] = LoadTexture("assets/Werewolf/Jump.png");
        sprites[RUN_WOLF] = LoadTexture("assets/Werewolf/Run.png");
        sprites[WALK_WOLF] = LoadTexture("assets/Werewolf/Walk.png");
        
        // Setup animations
        animations[IDLE_WOLF] = {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_WOLF};
        animations[WALK_WOLF] = {0, 10, 0, 0, 0.1f, 0.1f, REPEATING_WOLF};
        animations[JUMP_WOLF] = {0, 10, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF};
        animations[RUN_WOLF] = {0, 8, 0, 0, 0.1f, 0.1f, REPEATING_WOLF};
        animations[ATTACK_SWIPE] = {0, 3, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF};
        animations[ATTACK_RUN] = {0, 6, 0, 0, 0.1f, 0.1f, ONESHOT_WOLF};
        animations[HURT_WOLF] = {0, 1, 0, 0, 0.2f, 0.2f, ONESHOT_WOLF};
        animations[DEAD_WOLF] = {0, 1, 0, 0, 0.2f, 0.2f, ONESHOT_WOLF};
    }
    
    void updateAnimation() override {
        AnimationWolf& anim = animations[state];
        anim.timeLeft -= GetFrameTime();
        
        if (anim.timeLeft <= 0) {
            anim.timeLeft = anim.speed;
            
            if (anim.type == REPEATING_WOLF) {
                anim.currentFrame = (anim.currentFrame + 1) % (anim.lastFrame + 1);
            } else {
                if (anim.currentFrame < anim.lastFrame) {
                    anim.currentFrame++;
                } else {
                    // Animation finished
                    if (state == ATTACK_SWIPE || state == ATTACK_RUN) {
                        hasFinishedAttack = true;
                        state = IDLE_WOLF;
                    }
                }
            }
        }
    }
    
    Rectangle getAnimationFrame() const {
        const AnimationWolf& anim = animations[state];
        int frameWidth = sprites[state].width / (anim.lastFrame + 1);
        int frameHeight = sprites[state].height;

        return { (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight };
    }
    
    void draw() override {
        if (state == DEAD_WOLF && isDead) return;
        
        Rectangle frameRec = getAnimationFrame();
        
        // Calculate destination rectangle
        Rectangle destRec = {
            rect.x + (direction == LEFT_WOLF ? rect.width : 0),
            rect.y,
            frameRec.width * (direction == LEFT_WOLF ? -2.0f : 2.0f),
            frameRec.height * 2.0f
        };
        
        Color tint = isInvulnerable ? ColorAlpha(WHITE, 0.5f) : WHITE;
        
        // Draw the sprite
        DrawTexturePro(sprites[state], frameRec, destRec, Vector2{0, 0}, 0.0f, tint);
        
        // Draw debug boxes
        drawDebugBoxes();
    }

    void takeDamage(int damage) override {
        if (isDead) return;
        currentHealth -= damage;
        if (currentHealth <= 0) {
            currentHealth = 0;
            isDead = true;
            state = DEAD_WOLF;
        }
    }

    void heal(int amount) {
        if (isDead) return;
        currentHealth += amount;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;
        }
    }

    void move() override {
        if (!hasFinishedAttack) return;
    
        float moveSpeed = 300.0f;
        velocity.x = 0.0f;
    
        // Allow movement while jumping without changing state
        if (IsKeyDown(KEY_V)) {
            velocity.x = -moveSpeed;
            direction = LEFT_WOLF;
            if (isOnGround) state = WALK_WOLF;
        } 
        else if (IsKeyDown(KEY_B)) {
            velocity.x = moveSpeed;
            direction = RIGHT_WOLF;
            if (isOnGround) state = WALK_WOLF;
        } 
        else if (isOnGround) {
            state = IDLE_WOLF;
        }
    
        if (IsKeyPressed(KEY_G) && isOnGround) {
            velocity.y = JUMP_FORCE;
            state = JUMP_WOLF;
            isOnGround = false;
    
            // Restart jump animation when pressing jump
            animations[JUMP_WOLF].currentFrame = animations[JUMP_WOLF].firstFrame;
        }
    
        if (IsKeyPressed(KEY_KP_4) && hasFinishedAttack) {
            state = ATTACK_SWIPE;
            hasFinishedAttack = false;
            animations[ATTACK_SWIPE].currentFrame = animations[ATTACK_SWIPE].firstFrame;
        }
    
        if (IsKeyPressed(KEY_KP_5) && hasFinishedAttack) {
            state = ATTACK_RUN;
            hasFinishedAttack = false;
            animations[ATTACK_RUN].currentFrame = animations[ATTACK_RUN].firstFrame;
        }
    }
    
    void applyVelocity() override {
        if (isDead) return;
        
        rect.x += velocity.x * GetFrameTime();
        rect.y += velocity.y * GetFrameTime();
        
        // Apply gravity
        if (!isOnGround) {
            velocity.y += GRAVITY * GetFrameTime();
        }
        
        // Check floor collision
        if (rect.y + rect.height > GROUND_LEVEL + rect.height) {
            rect.y = GROUND_LEVEL;
            velocity.y = 0;
            isOnGround = true;
        }
        
        // Update collision boxes when position changes
        updateCollisionBoxes();
    }
    
    bool isAlive() override {
        return !isDead;
    }

    void updateCollisionBoxes() override {
        // Get the current texture dimensions and frames based on state
        Texture2D* currentTexture = &sprites[0]; // Default to idle
        int framesInTexture = 8; // Assuming 8 frames for simplicity
        
        switch(state) {
            case IDLE_WOLF: 
                currentTexture = &sprites[IDLE_WOLF]; 
                framesInTexture = 8;
                break;
            case WALK_WOLF: 
                currentTexture = &sprites[WALK_WOLF]; 
                framesInTexture = 8;
                break;
            case ATTACK_SWIPE: 
                currentTexture = &sprites[ATTACK_SWIPE]; 
                framesInTexture = 3;
                break;
            case HURT_WOLF: 
                currentTexture = &sprites[HURT_WOLF]; 
                framesInTexture = 1;
                break;
            case DEAD_WOLF: 
                currentTexture = &sprites[DEAD_WOLF]; 
                framesInTexture = 1;
                break;
            default: 
                currentTexture = &sprites[IDLE_WOLF];
                framesInTexture = 8;
        }
        
        if (currentTexture->id != 0) {
            // Calculate frame dimensions
            float frameWidth = (float)currentTexture->width / framesInTexture;
            float frameHeight = (float)currentTexture->height;
            
            // Calculate the actual position of the sprite
            float spriteX = rect.x + (direction < 0 ? rect.width : 0);
            float spriteY = rect.y - (frameHeight * 2.0f - rect.height);
            float spriteWidth = frameWidth * 2.0f;
            float spriteHeight = frameHeight * 2.0f;
            
            // Update collision box to match the sprite's actual body
            // Make the collision box about 40% of the sprite width and 70% of the sprite height
            // Position it to be centered horizontally and aligned with the bottom of the sprite
            collisionBox.width = spriteWidth * 0.4f;
            collisionBox.height = spriteHeight * 0.7f;
            
            // Adjust position based on direction
            if (direction > 0) {
                collisionBox.x = spriteX + (spriteWidth - collisionBox.width) / 2;
            } else {
                collisionBox.x = spriteX - spriteWidth + (spriteWidth - collisionBox.width) / 2;
            }
            
            collisionBox.y = spriteY + spriteHeight - collisionBox.height;
            
            // Update hit box based on state and direction
            if (state == ATTACK_SWIPE || state == ATTACK_RUN) {
                // Attack hit box extends in front of the werewolf
                if (direction > 0) {
                    hitBox.width = spriteWidth * 0.6f;
                    hitBox.height = spriteHeight * 0.5f;
                    hitBox.x = spriteX + spriteWidth * 0.4f;
                    hitBox.y = spriteY + spriteHeight * 0.3f;
                } else {
                    hitBox.width = spriteWidth * 0.6f;
                    hitBox.height = spriteHeight * 0.5f;
                    hitBox.x = spriteX - spriteWidth - hitBox.width * 0.5f;
                    hitBox.y = spriteY + spriteHeight * 0.3f;
                }
            } else {
                // No hit box for other states
                hitBox = { 0, 0, 0, 0 };
            }
        }
    }
};

#endif // WEREWOLF_H
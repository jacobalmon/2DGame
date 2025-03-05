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
        Rectangle source = getAnimationFrame();
        
        if (direction == LEFT_WOLF) {
            source.width = -source.width; // Flip horizontally
        }
        
        Rectangle dest = {
            rect.x,
            rect.y,
            source.width * 2.0f,
            source.height * 2.0f
        };
        
        DrawTexturePro(sprites[state], source, dest, Vector2{0, 0}, 0.0f, WHITE);
        
        // Draw collision box if enabled
        if (showCollisionBoxes) {
            DrawRectangleLinesEx(rect, 1, GREEN);
        }
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
        float deltaTime = GetFrameTime();

        velocity.y += GRAVITY * deltaTime;

        rect.x += velocity.x * deltaTime;
        rect.y += velocity.y * deltaTime;

        if (rect.y >= GROUND_LEVEL) {
            rect.y = GROUND_LEVEL;
            velocity.y = 0;
            isOnGround = true;
            if (state == JUMP_WOLF) {
                state = IDLE_WOLF;
            }
        }
    }
    
    bool isAlive() override {
        return !isDead;
    }
};

#endif // WEREWOLF_H
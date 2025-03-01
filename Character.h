#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"

// Character states
enum CharacterState {
    IDLE,
    WALK,
    JUMP,
    ATTACK,
    ATTACK_CLUB,
    ATTACK_STOMP,
    ATTACK_AOE,
    HURT,
    DEAD
};

class Character {
public:
    Rectangle rect;
    Vector2 velocity;
    float direction;
    CharacterState state;
    float animationTimer;
    int currentFrame;
    
    // Health and combat properties
    int maxHealth;
    int currentHealth;
    int attackDamage;
    bool isInvulnerable;
    float invulnerabilityTimer;
    float hitStunDuration;
    bool isHit;
    
    Character(Vector2 position) {
        rect = { position.x, position.y, 64, 64 };  // Default size
        velocity = { 0, 0 };
        direction = 1.0f;
        state = IDLE;
        animationTimer = 0;
        currentFrame = 0;
        
        // Initialize health and combat values
        maxHealth = 100;
        currentHealth = maxHealth;
        attackDamage = 20;
        isInvulnerable = false;
        invulnerabilityTimer = 0;
        hitStunDuration = 0.5f;
        isHit = false;
    }
    
    virtual void takeDamage(int damage) {
        if (!isInvulnerable && !isHit) {
            currentHealth -= damage;
            if (currentHealth < 0) currentHealth = 0;
            isHit = true;
            isInvulnerable = true;
            invulnerabilityTimer = 1.0f; // 1 second of invulnerability
        }
    }
    
    virtual void updateCombatState(float deltaTime) {
        if (isInvulnerable) {
            invulnerabilityTimer -= deltaTime;
            if (invulnerabilityTimer <= 0) {
                isInvulnerable = false;
                invulnerabilityTimer = 0;
            }
        }
        
        if (isHit) {
            hitStunDuration -= deltaTime;
            if (hitStunDuration <= 0) {
                isHit = false;
                hitStunDuration = 0.5f;
            }
        }
    }
    
    virtual bool isAlive() {
        return currentHealth > 0;
    }
    
    // Draw health bar above character
    virtual void drawHealthBar() {
        float healthBarWidth = 50;
        float healthBarHeight = 5;
        float healthPercentage = (float)currentHealth / maxHealth;
        
        // Background (red)
        DrawRectangle(
            rect.x + rect.width/2 - healthBarWidth/2,
            rect.y - 20,
            healthBarWidth,
            healthBarHeight,
            RED
        );
        
        // Foreground (green)
        DrawRectangle(
            rect.x + rect.width/2 - healthBarWidth/2,
            rect.y - 20,
            healthBarWidth * healthPercentage,
            healthBarHeight,
            GREEN
        );
    }

    virtual void move() = 0;
    virtual void applyVelocity() = 0;
    virtual void updateAnimation() = 0;
    virtual void draw() = 0;
    virtual void loadTextures() = 0;
};

class Samurai : public Character {
private:
    Texture2D idleTexture;
    Texture2D walkTexture;
    Texture2D attackTextures[3];  // For the three attack animations
    Texture2D hurtTexture;
    Texture2D deadTexture;
    Texture2D jumpTexture;
    Texture2D runTexture;
    Texture2D shieldTexture;
    
public:
    Samurai(Vector2 position) : Character(position) {
        rect.width = 128;  // Adjust size for samurai
        rect.height = 128;
        attackDamage = 25;  // Slightly stronger attack
    }
    
    void loadTextures() override {
        idleTexture = LoadTexture("assets/Samurai/Idle.png");
        walkTexture = LoadTexture("assets/Samurai/Walk.png");
        attackTextures[0] = LoadTexture("assets/Samurai/Attack_1.png");
        attackTextures[1] = LoadTexture("assets/Samurai/Attack_2.png");
        attackTextures[2] = LoadTexture("assets/Samurai/Attack_3.png");
        hurtTexture = LoadTexture("assets/Samurai/Hurt.png");
        deadTexture = LoadTexture("assets/Samurai/Dead.png");
        jumpTexture = LoadTexture("assets/Samurai/Jump.png");
        runTexture = LoadTexture("assets/Samurai/Run.png");
        shieldTexture = LoadTexture("assets/Samurai/Shield.png");
    }
    
    void move() override {
        if (IsKeyDown(KEY_D)) {
            velocity.x = 200.0f;
            direction = 1.0f;
            state = WALK;
        }
        else if (IsKeyDown(KEY_A)) {
            velocity.x = -200.0f;
            direction = -1.0f;
            state = WALK;
        }
        else {
            velocity.x = 0;
            state = IDLE;
        }
        
        if (IsKeyPressed(KEY_SPACE)) {
            state = ATTACK;
        }
    }
    
    void applyVelocity() override {
        rect.x += velocity.x * GetFrameTime();
    }
    
    void draw() override {
        // Calculate the frame rectangle for sprite animation
        Rectangle frameRec = { 0, 0, (float)idleTexture.width/6, (float)idleTexture.height };  // 6 frames per row
        frameRec.x = currentFrame * frameRec.width;
        
        // Flip horizontally if facing left
        Vector2 pos = { rect.x, rect.y };
        float scale = 2.0f;  // Scale up the sprites
        
        switch(state) {
            case IDLE:
                DrawTexturePro(idleTexture, 
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case WALK:
                DrawTexturePro(walkTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case ATTACK:
                DrawTexturePro(attackTextures[0],
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case HURT:
                DrawTexturePro(hurtTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case DEAD:
                DrawTexturePro(deadTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            default:
                DrawTexturePro(idleTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
    }
    
    void updateAnimation() override {
        animationTimer += GetFrameTime();
        if (animationTimer >= 0.1f) {  // Update animation every 0.1 seconds
            currentFrame++;
            if (currentFrame > 5) currentFrame = 0;  // 6 frames per animation
            animationTimer = 0.0f;
        }
    }
};

class Goblin : public Character {
private:
    Texture2D idleTexture;
    Texture2D walkTexture;
    Texture2D attackTextures[4];  // For all attack animations
    Texture2D dieTexture;
    Texture2D readyTexture;
    
public:
    Goblin(Vector2 position) : Character(position) {
        rect.width = 100;
        rect.height = 100;
        attackDamage = 15;
    }
    
    void loadTextures() override {
        idleTexture = LoadTexture("assets/Goblin/Hobgoblin Idle/sprite_sheet.png");
        walkTexture = LoadTexture("assets/Goblin/Hobgoblin Walk/sprite_sheet.png");
        attackTextures[0] = LoadTexture("assets/Goblin/Hobgoblin Attack 1 & 2/sprite_sheet.png");
        attackTextures[1] = LoadTexture("assets/Goblin/Hobgoblin Attack 1 & 2/sprite_sheet2.png");
        attackTextures[2] = LoadTexture("assets/Goblin/Hobgoblin Attack 3/sprite_sheet.png");
        attackTextures[3] = LoadTexture("assets/Goblin/Hobgoblin Attack 4/sprite_sheet.png");
        dieTexture = LoadTexture("assets/Goblin/Hobgoblin Die/sprite_sheet.png");
        readyTexture = LoadTexture("assets/Goblin/Hobgoblin Ready Up/sprite_sheet.png");
    }
    
    void move() override {
        // AI movement logic here
        // For now, just move back and forth
        static float timer = 0;
        timer += GetFrameTime();
        
        if (timer > 2.0f) {
            direction *= -1;
            timer = 0;
        }
        
        velocity.x = direction * 100.0f;
        state = WALK;
    }
    
    void applyVelocity() override {
        rect.x += velocity.x * GetFrameTime();
    }
    
    void draw() override {
        // Calculate the frame rectangle for sprite animation
        Rectangle frameRec = { 0, 0, (float)idleTexture.width/6, (float)idleTexture.height };  // 6 frames per row
        frameRec.x = currentFrame * frameRec.width;
        
        Vector2 pos = { rect.x, rect.y };
        float scale = 2.0f;  // Scale up the sprites
        
        switch(state) {
            case IDLE:
                DrawTexturePro(idleTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case WALK:
                DrawTexturePro(walkTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case ATTACK_CLUB:
                DrawTexturePro(attackTextures[0],
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case ATTACK_STOMP:
                DrawTexturePro(attackTextures[2],
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case ATTACK_AOE:
                DrawTexturePro(attackTextures[3],
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            case DEAD:
                DrawTexturePro(dieTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                break;
            default:
                DrawTexturePro(idleTexture,
                    frameRec,
                    (Rectangle){ pos.x, pos.y, frameRec.width * scale * direction, frameRec.height * scale },
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
    }
    
    void updateAnimation() override {
        animationTimer += GetFrameTime();
        if (animationTimer >= 0.1f) {  // Update animation every 0.1 seconds
            currentFrame++;
            if (currentFrame > 5) currentFrame = 0;  // 6 frames per animation
            animationTimer = 0.0f;
        }
    }
};

// Collision detection function
bool checkCharacterCollision(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

#endif // CHARACTER_H 
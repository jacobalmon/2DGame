#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include <cstdio>   // Added for sprintf

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

// Game constants
#define FLOOR_Y 500  // Define floor Y position as a constant

class Character {
public:
    Rectangle rect;
    Vector2 velocity;
    float direction;
    CharacterState state;
    float animationTimer;
    int currentFrame;
    int frameCount;  // Added to handle different sprite frame counts
    
    // Health and combat properties
    int maxHealth;
    int currentHealth;
    int attackDamage;
    bool isInvulnerable;
    float invulnerabilityTimer;
    float hitStunDuration;
    bool isHit;
    
    Character(Vector2 position) {
        rect = { position.x, position.y, 64, 64 };
        velocity = { 0, 0 };
        direction = 1.0f;
        state = IDLE;
        animationTimer = 0;
        currentFrame = 0;
        frameCount = 6;  // Default frame count
        
        maxHealth = 100;
        currentHealth = maxHealth;
        attackDamage = 20;
        isInvulnerable = false;
        invulnerabilityTimer = 0;
        hitStunDuration = 0.5f;
        isHit = false;
    }
    
    // Place character on the floor
    void placeOnFloor() {
        rect.y = FLOOR_Y - rect.height;
    }
    
    virtual void takeDamage(int damage) {
        if (!isInvulnerable && !isHit && currentHealth > 0) {
            currentHealth -= damage;
            if (currentHealth < 0) currentHealth = 0;
            isHit = true;
            isInvulnerable = true;
            invulnerabilityTimer = 1.0f;
            state = HURT;  // Change state to HURT when taking damage
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
                if (currentHealth <= 0) {
                    state = DEAD;
                } else if (state == HURT) {
                    state = IDLE;
                }
            }
        }
    }
    
    virtual bool isAlive() {
        return currentHealth > 0;
    }
    
    virtual void drawHealthBar() {
        if (!isAlive()) return;  // Don't draw health bar if dead
        
        float healthBarWidth = rect.width * 1.2f;
        float healthBarHeight = 8;
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
        
        // Health number
        char healthText[32];
        snprintf(healthText, sizeof(healthText), "%d/%d", currentHealth, maxHealth);  // Using snprintf instead of sprintf for safety
        DrawText(healthText, 
                rect.x + rect.width/2 - MeasureText(healthText, 16)/2,
                rect.y - 40, 16, WHITE);
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
    Texture2D attackTextures[3];
    Texture2D hurtTexture;
    Texture2D deadTexture;
    Texture2D jumpTexture;
    Texture2D runTexture;
    Texture2D shieldTexture;
    
    float jumpVelocity;
    float jumpSpeed;
    bool isJumping;
    float gravity;
    float attackCooldown;
    float attackTimer;
    
public:
    Samurai(Vector2 position) : Character(position) {
        rect.width = 128;
        rect.height = 128;
        attackDamage = 25;
        
        jumpVelocity = 0;
        jumpSpeed = -600.0f;  // Increased jump speed
        isJumping = false;
        gravity = 1500.0f;    // Increased gravity
        attackCooldown = 0.5f;
        attackTimer = 0;
        
        // Place on floor
        placeOnFloor();
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
        if (state == DEAD) {
            velocity.x = 0;
            return;
        }
        
        attackTimer -= GetFrameTime();
        
        // Horizontal movement
        if (IsKeyDown(KEY_D)) {
            velocity.x = 300.0f;  // Increased movement speed
            direction = 1.0f;
            state = isJumping ? JUMP : WALK;
        }
        else if (IsKeyDown(KEY_A)) {
            velocity.x = -300.0f;  // Increased movement speed
            direction = -1.0f;
            state = isJumping ? JUMP : WALK;
        }
        else {
            velocity.x = 0;
            state = isJumping ? JUMP : IDLE;
        }
        
        // Jump mechanics
        if (IsKeyPressed(KEY_W) && !isJumping) {
            jumpVelocity = jumpSpeed;
            isJumping = true;
            state = JUMP;
        }
        
        // Attack
        if (IsKeyPressed(KEY_SPACE) && attackTimer <= 0 && !isJumping) {
            state = ATTACK;
            attackTimer = attackCooldown;
        }
        
        // Apply gravity
        if (isJumping) {
            jumpVelocity += gravity * GetFrameTime();
            velocity.y = jumpVelocity;
        }
    }
    
    void applyVelocity() override {
        if (state == DEAD) return;
        
        rect.x += velocity.x * GetFrameTime();
        rect.y += velocity.y * GetFrameTime();
        
        // Screen bounds
        if (rect.x < 0) rect.x = 0;
        if (rect.x + rect.width > GetScreenWidth()) rect.x = GetScreenWidth() - rect.width;
        if (rect.y < 0) {
            rect.y = 0;
            jumpVelocity = 0;
        }
        if (rect.y + rect.height > FLOOR_Y) {
            rect.y = FLOOR_Y - rect.height;
            if (isJumping) {
                isJumping = false;
                velocity.y = 0;
                jumpVelocity = 0;
                state = velocity.x != 0 ? WALK : IDLE;
            }
        }
    }
    
    void draw() override {
        Rectangle frameRec = { 0, 0, (float)idleTexture.width/6, (float)idleTexture.height };
        frameRec.x = currentFrame * frameRec.width;
        
        Texture2D* currentTexture = &idleTexture;
        
        switch(state) {
            case IDLE:
                currentTexture = &idleTexture;
                break;
            case WALK:
                currentTexture = &walkTexture;
                break;
            case JUMP:
                currentTexture = &jumpTexture;
                break;
            case ATTACK:
                currentTexture = &attackTextures[0];
                break;
            case HURT:
                currentTexture = &hurtTexture;
                break;
            case DEAD:
                currentTexture = &deadTexture;
                break;
            default:
                currentTexture = &idleTexture;
        }
        
        // Recalculate frame width based on the current texture
        frameRec.width = (float)currentTexture->width/6;
        
        Color tint = isInvulnerable ? ColorAlpha(WHITE, 0.5f) : WHITE;
        
        // Calculate the destination rectangle - align bottom with floor
        Rectangle destRec = {
            rect.x + (direction < 0 ? rect.width : 0),  // Adjust X position when facing left
            rect.y - (frameRec.height * 2.0f - rect.height),  // Align bottom with character rect
            frameRec.width * 2.0f * (direction < 0 ? -1.0f : 1.0f),  // Flip width when facing left
            frameRec.height * 2.0f
        };
        
        DrawTexturePro(*currentTexture, frameRec, destRec, Vector2{0, 0}, 0.0f, tint);
        
        // Debug: Draw collision box
        DrawRectangleLinesEx(rect, 1, GREEN);
    }
    
    void updateAnimation() override {
        if (state == DEAD && currentFrame >= 5) return;  // Stay on last frame when dead
        
        animationTimer += GetFrameTime();
        if (animationTimer >= 0.1f) {
            currentFrame++;
            if (currentFrame > 5) currentFrame = 0;
            animationTimer = 0.0f;
        }
    }
};

class Goblin : public Character {
private:
    Texture2D idleTexture;
    Texture2D walkTexture;
    Texture2D attackTextures[4];
    Texture2D dieTexture;
    Texture2D readyTexture;
    bool texturesLoaded;
    float scale;
    float attackTimer;
    float attackCooldown;
    float aiTimer;
    float aiDecisionTime;
    
public:
    Goblin(Vector2 position) : Character(position) {
        rect.width = 64;
        rect.height = 64;
        attackDamage = 15;
        texturesLoaded = false;
        scale = 2.0f;
        attackTimer = 0;
        attackCooldown = 1.0f;
        aiTimer = 0;
        aiDecisionTime = 2.0f;
        
        idleTexture = { 0 };
        walkTexture = { 0 };
        for(int i = 0; i < 4; i++) {
            attackTextures[i] = { 0 };
        }
        dieTexture = { 0 };
        readyTexture = { 0 };
        
        // Place on floor
        placeOnFloor();
    }
    
    void loadTextures() override {
        TraceLog(LOG_INFO, "Loading Goblin textures in Character.h...");
        
        // Helper function to check if a file exists
        auto fileExists = [](const char* path) -> bool {
            FILE* file = fopen(path, "rb");
            if (file) {
                fclose(file);
                return true;
            }
            return false;
        };
        
        // Load idle texture
        const char* idlePath = "assets/Goblin/Hobgoblin Idle/GoblinK Idle.png";
        TraceLog(LOG_INFO, "Checking if idle texture exists: %s - %s", idlePath, fileExists(idlePath) ? "YES" : "NO");
        idleTexture = LoadTexture(idlePath);
        TraceLog(LOG_INFO, "Idle texture ID: %d, Width: %d, Height: %d", idleTexture.id, idleTexture.width, idleTexture.height);
        if (idleTexture.id == 0) {
            TraceLog(LOG_ERROR, "Failed to load Goblin idle texture from: %s", idlePath);
            return;
        }
        
        // Load walk texture
        const char* walkPath = "assets/Goblin/Hobgoblin Walk/Hobgoblin Walk.png";
        TraceLog(LOG_INFO, "Checking if walk texture exists: %s - %s", walkPath, fileExists(walkPath) ? "YES" : "NO");
        walkTexture = LoadTexture(walkPath);
        TraceLog(LOG_INFO, "Walk texture ID: %d, Width: %d, Height: %d", walkTexture.id, walkTexture.width, walkTexture.height);
        if (walkTexture.id == 0) {
            TraceLog(LOG_ERROR, "Failed to load Goblin walk texture from: %s", walkPath);
            return;
        }
        
        // Load attack textures
        const char* attackPaths[] = {
            "assets/Goblin/Hobgoblin Attack 1 & 2/Hobgoblin Attack 1 and 2.png",
            "assets/Goblin/Hobgoblin Attack 1 & 2/Hobgoblin Attack 1 and 2.png",
            "assets/Goblin/Hobgoblin Attack 3/Hobgoblin Attack 3.png",
            "assets/Goblin/Hobgoblin Attack 4/Hobgoblin Fourth Attack.png"
        };
        
        for (int i = 0; i < 4; i++) {
            TraceLog(LOG_INFO, "Checking if attack texture %d exists: %s - %s", i + 1, attackPaths[i], fileExists(attackPaths[i]) ? "YES" : "NO");
            attackTextures[i] = LoadTexture(attackPaths[i]);
            TraceLog(LOG_INFO, "Attack texture %d ID: %d, Width: %d, Height: %d", 
                    i + 1, attackTextures[i].id, attackTextures[i].width, attackTextures[i].height);
            if (attackTextures[i].id == 0) {
                TraceLog(LOG_ERROR, "Failed to load Goblin attack texture %d from: %s", i + 1, attackPaths[i]);
                return;
            }
        }
        
        // Load death and ready animations
        const char* diePath = "assets/Goblin/Hobgoblin Die/Hobgoblin Beheaded.png";
        TraceLog(LOG_INFO, "Checking if die texture exists: %s - %s", diePath, fileExists(diePath) ? "YES" : "NO");
        dieTexture = LoadTexture(diePath);
        TraceLog(LOG_INFO, "Die texture ID: %d, Width: %d, Height: %d", dieTexture.id, dieTexture.width, dieTexture.height);
        if (dieTexture.id == 0) {
            TraceLog(LOG_ERROR, "Failed to load Goblin die texture from: %s", diePath);
        }
        
        const char* readyPath = "assets/Goblin/Hobgoblin Ready Up/Hobgoblin Ready Up.png";
        TraceLog(LOG_INFO, "Checking if ready texture exists: %s - %s", readyPath, fileExists(readyPath) ? "YES" : "NO");
        readyTexture = LoadTexture(readyPath);
        TraceLog(LOG_INFO, "Ready texture ID: %d, Width: %d, Height: %d", readyTexture.id, readyTexture.width, readyTexture.height);
        if (readyTexture.id == 0) {
            TraceLog(LOG_ERROR, "Failed to load Goblin ready texture from: %s", readyPath);
        }
        
        if (idleTexture.id != 0 && walkTexture.id != 0) {
            texturesLoaded = true;
            TraceLog(LOG_INFO, "All Goblin textures loaded successfully!");
        } else {
            texturesLoaded = false;
            TraceLog(LOG_ERROR, "Failed to load essential Goblin textures!");
        }
    }
    
    void draw() override {
        if (!texturesLoaded) {
            DrawRectangleRec(rect, RED);
            DrawText("Texture Load Failed!", (int)rect.x, (int)rect.y - 20, 20, RED);
            return;
        }
        
        Texture2D* currentTexture = &idleTexture;
        int framesInTexture = 7; // Default for idle texture (7 frames)
        
        switch(state) {
            case WALK: 
                currentTexture = &walkTexture; 
                framesInTexture = 8; // Walk has 8 frames
                break;
            case ATTACK_CLUB: 
                currentTexture = &attackTextures[0]; 
                framesInTexture = 10; // Attack 1 has 10 frames
                break;
            case ATTACK_STOMP: 
                currentTexture = &attackTextures[2]; 
                framesInTexture = 24; // Attack 3 has 24 frames
                break;
            case ATTACK_AOE: 
                currentTexture = &attackTextures[3]; 
                framesInTexture = 9; // Attack 4 has 9 frames
                break;
            case DEAD: 
                currentTexture = &dieTexture; 
                framesInTexture = 9; // Die has 9 frames
                break;
            default: 
                currentTexture = &idleTexture;
                framesInTexture = 7; // Idle has 7 frames
        }
        
        if (currentTexture->id != 0) {
            // Calculate frame width based on number of frames
            float frameWidth = (float)currentTexture->width / framesInTexture;
            float frameHeight = (float)currentTexture->height;
            
            // Ensure currentFrame is within bounds
            if (currentFrame >= framesInTexture) {
                currentFrame = 0;
            }
            
            // Source rectangle for the current frame
            Rectangle frameRec = {
                currentFrame * frameWidth,
                0,
                frameWidth,
                frameHeight
            };
            
            // Calculate the destination rectangle - align bottom with character rect
            Rectangle destRec = {
                rect.x + (direction < 0 ? rect.width : 0), // Adjust X position when facing left
                rect.y - (frameHeight * scale - rect.height), // Align bottom with character rect
                frameWidth * scale * (direction < 0 ? -1.0f : 1.0f), // Flip width when facing left
                frameHeight * scale
            };
            
            Color tint = isInvulnerable ? ColorAlpha(WHITE, 0.5f) : WHITE;
            
            // Draw the sprite
            DrawTexturePro(*currentTexture, frameRec, destRec, Vector2{0, 0}, 0.0f, tint);
            
            // Debug: Draw collision box
            DrawRectangleLinesEx(rect, 1, GREEN);
        }
    }
    
    void move() override {
        if (state == DEAD) {
            velocity.x = 0;
            return;
        }
        
        attackTimer -= GetFrameTime();
        aiTimer += GetFrameTime();
        
        if (aiTimer >= aiDecisionTime) {
            aiTimer = 0;
            
            // Random movement decision
            if (GetRandomValue(0, 100) < 70) {  // 70% chance to change direction
                direction = GetRandomValue(0, 1) ? 1.0f : -1.0f;
            }
            
            // Random attack decision
            if (attackTimer <= 0 && GetRandomValue(0, 100) < 30) {  // 30% chance to attack
                state = (CharacterState)(ATTACK_CLUB + GetRandomValue(0, 2));  // Random attack type
                attackTimer = attackCooldown;
            }
        }
        
        if (attackTimer <= 0 && state != HURT) {
            velocity.x = direction * 150.0f;  // Slower than player
            state = WALK;
        } else {
            velocity.x = 0;
        }
        
        // Ensure character stays on the floor
        placeOnFloor();
    }
    
    void applyVelocity() override {
        if (state == DEAD) return;
        
        rect.x += velocity.x * GetFrameTime();
        
        if (rect.x < 0) {
            rect.x = 0;
            direction *= -1;
        }
        if (rect.x + rect.width > GetScreenWidth()) {
            rect.x = GetScreenWidth() - rect.width;
            direction *= -1;
        }
        
        // Ensure character stays on the floor
        placeOnFloor();
    }
    
    void updateAnimation() override {
        float deltaTime = GetFrameTime();
        animationTimer += deltaTime;
        
        // Different animation speeds based on state
        float animationSpeed = 0.1f;
        int maxFrames = 7; // Default for idle
        
        switch(state) {
            case WALK:
                animationSpeed = 0.1f;
                maxFrames = 8;
                break;
            case ATTACK_CLUB:
                animationSpeed = 0.08f;
                maxFrames = 10;
                break;
            case ATTACK_STOMP:
                animationSpeed = 0.08f;
                maxFrames = 24;
                break;
            case ATTACK_AOE:
                animationSpeed = 0.08f;
                maxFrames = 9;
                break;
            case DEAD:
                animationSpeed = 0.15f;
                maxFrames = 9;
                break;
            case HURT:
                animationSpeed = 0.1f;
                maxFrames = 7; // Use idle frames for hurt
                break;
            default: // IDLE
                animationSpeed = 0.15f;
                maxFrames = 7;
        }
        
        // Update frame when timer exceeds animation speed
        if (animationTimer >= animationSpeed) {
            animationTimer = 0;
            currentFrame++;
            
            // Reset animation or transition to idle after attack
            if (currentFrame >= maxFrames) {
                if (state == ATTACK_CLUB || state == ATTACK_STOMP || state == ATTACK_AOE) {
                    state = IDLE;
                }
                currentFrame = 0;
            }
        }
        
        // Update combat state
        updateCombatState(deltaTime);
    }
};

bool checkCharacterCollision(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

#endif // CHARACTER_H 
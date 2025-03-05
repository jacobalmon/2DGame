#ifndef WIZARD_H
#define WIZARD_H

#include "raylib.h"
#include "Character.h"
#include <vector>

// Reference to the global collision box toggle
extern bool showCollisionBoxes;

enum DirectionWizard {
    LEFT_WIZARD = -1,
    RIGHT_WIZARD = 1
};

enum CurrentStateWizard {
    DEAD_WIZARD = 0,
    ATTACK1_WIZARD = 1,
    ATTACK2_WIZARD = 2,
    HURT_WIZARD = 3,
    IDLE_WIZARD = 4,
    JUMP_WIZARD = 5,
    RUN_WIZARD = 6
};

enum AnimationTypeWizard {
    REPEATING_WIZARD,
    ONESHOT_WIZARD
};

struct AnimationWizard {
    int firstFrame, lastFrame, currentFrame, offset;
    float speed, timeLeft;
    AnimationTypeWizard type;
};

const float GRAVITY_WIZARD = 800.0f;
const float JUMP_FORCE_WIZARD = -500.0f;
const float GROUND_LEVEL_WIZARD = 480.0f - 128.0f;  // Adjusted to match floor level

class Wizard : public Character {
    private:
        std::vector<Texture2D> sprites;
        std::vector<AnimationWizard> animations;
        CurrentStateWizard state;
        DirectionWizard direction;
        bool isDead;
        bool isOnGround;
        bool hasFinishedAttack;

    public:
        Wizard(Vector2 position) : Character(position) {
            rect.width = 64;
            rect.height = 128;
            velocity = {0, 0};
            direction = RIGHT_WIZARD;
            state = IDLE_WIZARD;
            currentHealth = 80;
            maxHealth = 80;
            attackDamage = 30;
            isDead = false;
            isOnGround = true;
            hasFinishedAttack = true;
            
            // Initialize animations
            sprites.resize(7);
            animations.resize(7);
            
            // Place on floor
            placeOnFloor();
        }
        
        ~Wizard() {
            for (auto& sprite : sprites) {
                UnloadTexture(sprite);
            }
        }
        
        void loadTextures() override {
            sprites[DEAD_WIZARD] = LoadTexture("assets/Wizard/Sprites/Death.png");
            sprites[ATTACK1_WIZARD] = LoadTexture("assets/Wizard/Sprites/Attack1.png");
            sprites[ATTACK2_WIZARD] = LoadTexture("assets/Wizard/Sprites/Attack2.png");
            sprites[HURT_WIZARD] = LoadTexture("assets/Wizard/Sprites/Take hit.png");
            sprites[IDLE_WIZARD] = LoadTexture("assets/Wizard/Sprites/Idle.png");
            sprites[JUMP_WIZARD] = LoadTexture("assets/Wizard/Sprites/Jump.png");
            sprites[RUN_WIZARD] = LoadTexture("assets/Wizard/Sprites/Run.png");
            
            // Setup animations
            animations[IDLE_WIZARD] = {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_WIZARD};
            animations[RUN_WIZARD] = {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_WIZARD};
            animations[JUMP_WIZARD] = {0, 1, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD};
            animations[ATTACK1_WIZARD] = {0, 7, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD};
            animations[ATTACK2_WIZARD] = {0, 7, 0, 0, 0.1f, 0.1f, ONESHOT_WIZARD};
            animations[HURT_WIZARD] = {0, 2, 0, 0, 0.2f, 0.2f, ONESHOT_WIZARD};
            animations[DEAD_WIZARD] = {0, 6, 0, 0, 0.2f, 0.2f, ONESHOT_WIZARD};
        }
        
        void updateAnimation() override {
            AnimationWizard& anim = animations[state];
            anim.timeLeft -= GetFrameTime();
            
            if (anim.timeLeft <= 0) {
                anim.timeLeft = anim.speed;
                
                if (anim.type == REPEATING_WIZARD) {
                    anim.currentFrame = (anim.currentFrame + 1) % (anim.lastFrame + 1);
                } else {
                    if (anim.currentFrame < anim.lastFrame) {
                        anim.currentFrame++;
                    } else {
                        // Animation finished
                        if (state == ATTACK1_WIZARD || state == ATTACK2_WIZARD) {
                            hasFinishedAttack = true;
                            state = IDLE_WIZARD;
                        }
                    }
                }
            }
        }
        
        Rectangle getAnimationFrame() const {
            const AnimationWizard& anim = animations[state];
            int frameWidth = sprites[state].width / (anim.lastFrame + 1);
            int frameHeight = sprites[state].height;
    
            return { (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight };
        }
        
        void draw() override {
            if (state == DEAD_WIZARD && isDead) return;
            
            Rectangle frameRec = getAnimationFrame();
            
            // Calculate destination rectangle
            Rectangle destRec = {
                rect.x + (direction == LEFT_WIZARD ? rect.width : 0),
                rect.y,
                frameRec.width * (direction == LEFT_WIZARD ? -2.0f : 2.0f),
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
                state = DEAD_WIZARD;
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
        
            if (IsKeyDown(KEY_N)) {
                velocity.x = -moveSpeed;
                direction = LEFT_WIZARD;
                if (isOnGround) state = RUN_WIZARD;
            } 
            else if (IsKeyDown(KEY_M)) {
                velocity.x = moveSpeed;
                direction = RIGHT_WIZARD;
                if (isOnGround) state = RUN_WIZARD;
            } 
            else if (isOnGround) {
                state = IDLE_WIZARD;
            }
        
            if (IsKeyPressed(KEY_J) && isOnGround) {
                velocity.y = JUMP_FORCE_WIZARD;
                state = JUMP_WIZARD;
                isOnGround = false;
        
                animations[JUMP_WIZARD].currentFrame = animations[JUMP_WIZARD].firstFrame;
            }
        
            if (IsKeyPressed(KEY_KP_7) && hasFinishedAttack) {
                state = ATTACK1_WIZARD;
                hasFinishedAttack = false;
                animations[ATTACK1_WIZARD].currentFrame = animations[ATTACK1_WIZARD].firstFrame;
            }
        
            if (IsKeyPressed(KEY_KP_8) && hasFinishedAttack) {
                state = ATTACK2_WIZARD;
                hasFinishedAttack = false;
                animations[ATTACK2_WIZARD].currentFrame = animations[ATTACK2_WIZARD].firstFrame;
            }
        }

        void applyVelocity() override {
            if (isDead) return;
            
            rect.x += velocity.x * GetFrameTime();
            rect.y += velocity.y * GetFrameTime();
            
            // Apply gravity
            if (!isOnGround) {
                velocity.y += GRAVITY_WIZARD * GetFrameTime();
            }
            
            // Check floor collision
            if (rect.y + rect.height > GROUND_LEVEL_WIZARD + rect.height) {
                rect.y = GROUND_LEVEL_WIZARD;
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
            if (state < 0 || state >= sprites.size() || sprites[state].id == 0) {
                return; // Invalid state or texture not loaded
            }
            
            const AnimationWizard& anim = animations[state];
            int frameWidth = sprites[state].width / (anim.lastFrame + 1);
            int frameHeight = sprites[state].height;
            
            // Calculate the actual position of the sprite
            float spriteX = rect.x + (direction == LEFT_WIZARD ? rect.width : 0);
            float spriteY = rect.y;
            float spriteWidth = frameWidth * 2.0f; // Scale is 2.0
            float spriteHeight = frameHeight * 2.0f;
            
            // Update collision box to match the sprite's actual body
            // Make the collision box about 30% of the sprite width and 70% of the sprite height
            // Position it to be centered horizontally and aligned with the bottom of the sprite
            collisionBox.width = spriteWidth * 0.3f;
            collisionBox.height = spriteHeight * 0.7f;
            
            // Adjust position based on direction
            if (direction == RIGHT_WIZARD) {
                collisionBox.x = spriteX + (spriteWidth - collisionBox.width) / 2;
            } else {
                collisionBox.x = spriteX - spriteWidth + (spriteWidth - collisionBox.width) / 2;
            }
            
            collisionBox.y = spriteY + spriteHeight - collisionBox.height;
            
            // Update hit box based on state and direction
            if (state == ATTACK1_WIZARD || state == ATTACK2_WIZARD) {
                // Different hit boxes for different attack types
                if (state == ATTACK1_WIZARD) {
                    // Attack1 extends further in front
                    if (direction == RIGHT_WIZARD) {
                        hitBox.width = spriteWidth * 0.7f;
                        hitBox.height = spriteHeight * 0.4f;
                        hitBox.x = spriteX + spriteWidth * 0.3f;
                        hitBox.y = spriteY + spriteHeight * 0.3f;
                    } else {
                        hitBox.width = spriteWidth * 0.7f;
                        hitBox.height = spriteHeight * 0.4f;
                        hitBox.x = spriteX - spriteWidth - hitBox.width * 0.5f;
                        hitBox.y = spriteY + spriteHeight * 0.3f;
                    }
                } else if (state == ATTACK2_WIZARD) {
                    // Attack2 is a wider area effect
                    if (direction == RIGHT_WIZARD) {
                        hitBox.width = spriteWidth * 0.9f;
                        hitBox.height = spriteHeight * 0.5f;
                        hitBox.x = spriteX + spriteWidth * 0.2f;
                        hitBox.y = spriteY + spriteHeight * 0.25f;
                    } else {
                        hitBox.width = spriteWidth * 0.9f;
                        hitBox.height = spriteHeight * 0.5f;
                        hitBox.x = spriteX - spriteWidth - hitBox.width * 0.1f;
                        hitBox.y = spriteY + spriteHeight * 0.25f;
                    }
                }
            } else {
                // No hit box for other states
                hitBox = { 0, 0, 0, 0 };
            }
        }
};

#endif // WIZARD_H
#ifndef DEMON_H
#define DEMON_H

#include "raylib.h"
#include "Character.h"
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

// Reference to the global collision box toggle
extern bool showCollisionBoxes;

// Constants for Demon
const float GRAVITY_DEMON = 800.0f;
const float JUMP_FORCE_DEMON = -500.0f;
const float GROUND_LEVEL_DEMON = 480.0f - 128.0f;  // Adjusted to match floor level

enum DirectionDemon {
    LEFT_DEMON = -1,
    RIGHT_DEMON = 1
};

enum CurrentStateDemon {
    IDLE_DEMON = 0,
    WALK_DEMON = 1,
    ATTACK_DEMON = 2,
    HURT_DEMON = 3,
    DEAD_DEMON = 4
};

enum AnimationTypeDemon {
    REPEATING_DEMON,
    ONESHOT_DEMON
};

struct AnimationDemon {
    int firstFrame, lastFrame, currentFrame;
    float speed, timeLeft;
    AnimationTypeDemon type;
    std::vector<Texture2D> frames;  // Store the individual textures for frames
};

class Demon : public Character {
    private:
        std::vector<Texture2D> sprites;
        std::vector<AnimationDemon> animations;
        CurrentStateDemon state;
        DirectionDemon direction;
        bool isDead;
        bool isOnGround;
        bool hasFinishedAttack;

    public:
        Demon(Vector2 position) : Character(position) {
            rect.width = 64;
            rect.height = 128;
            velocity = {0, 0};
            direction = RIGHT_DEMON;
            state = IDLE_DEMON;
            currentHealth = 200;
            maxHealth = 200;
            attackDamage = 30;
            isDead = false;
            isOnGround = true;
            hasFinishedAttack = true;
            
            // Initialize animations
            sprites.resize(5);
            animations.resize(5);
            
            // Place on floor
            placeOnFloor();
        }
        
        ~Demon() {
            for (auto& anim : animations) {
                for (auto& frame : anim.frames) {
                    UnloadTexture(frame);  // Unload individual frames
                }
            }
        }

        void loadAnimations() {
            // Helper function to load all PNG files from a directory
            auto loadFramesFromDirectory = [](const std::string& dirPath) -> std::vector<Texture2D> {
                std::vector<Texture2D> frames;
                std::vector<std::string> filePaths;
                
                // Find all PNG files in the directory
                for (const auto& entry : fs::directory_iterator(dirPath)) {
                    if (entry.path().extension() == ".png") {
                        filePaths.push_back(entry.path().string());
                        std::cout << "Found PNG: " << entry.path().string() << std::endl;
                    }
                }
                
                // Sort filenames to ensure correct order
                std::sort(filePaths.begin(), filePaths.end());
                
                // Load textures
                std::cout << "Loading " << filePaths.size() << " frames." << std::endl;
                for (const auto& path : filePaths) {
                    Texture2D texture = LoadTexture(path.c_str());
                    frames.push_back(texture);
                    std::cout << "Loaded texture: " << path << std::endl;
                }
                
                return frames;
            };
            
            // Load frames for each animation state
            animations[IDLE_DEMON].frames = loadFramesFromDirectory("assets/Demon/individual sprites/01_demon_idle");
            animations[IDLE_DEMON].firstFrame = 0;
            animations[IDLE_DEMON].lastFrame = animations[IDLE_DEMON].frames.size() - 1;
            animations[IDLE_DEMON].currentFrame = 0;
            animations[IDLE_DEMON].speed = 0.1f;
            animations[IDLE_DEMON].timeLeft = 0.1f;
            animations[IDLE_DEMON].type = REPEATING_DEMON;
            
            animations[WALK_DEMON].frames = loadFramesFromDirectory("assets/Demon/individual sprites/02_demon_walk");
            animations[WALK_DEMON].firstFrame = 0;
            animations[WALK_DEMON].lastFrame = animations[WALK_DEMON].frames.size() - 1;
            animations[WALK_DEMON].currentFrame = 0;
            animations[WALK_DEMON].speed = 0.1f;
            animations[WALK_DEMON].timeLeft = 0.1f;
            animations[WALK_DEMON].type = REPEATING_DEMON;
            
            animations[ATTACK_DEMON].frames = loadFramesFromDirectory("assets/Demon/individual sprites/03_demon_cleave");
            animations[ATTACK_DEMON].firstFrame = 0;
            animations[ATTACK_DEMON].lastFrame = animations[ATTACK_DEMON].frames.size() - 1;
            animations[ATTACK_DEMON].currentFrame = 0;
            animations[ATTACK_DEMON].speed = 0.1f;
            animations[ATTACK_DEMON].timeLeft = 0.1f;
            animations[ATTACK_DEMON].type = ONESHOT_DEMON;
            
            animations[HURT_DEMON].frames = loadFramesFromDirectory("assets/Demon/individual sprites/04_demon_take_hit");
            animations[HURT_DEMON].firstFrame = 0;
            animations[HURT_DEMON].lastFrame = animations[HURT_DEMON].frames.size() - 1;
            animations[HURT_DEMON].currentFrame = 0;
            animations[HURT_DEMON].speed = 0.1f;
            animations[HURT_DEMON].timeLeft = 0.1f;
            animations[HURT_DEMON].type = ONESHOT_DEMON;
            
            animations[DEAD_DEMON].frames = loadFramesFromDirectory("assets/Demon/individual sprites/05_demon_death");
            animations[DEAD_DEMON].firstFrame = 0;
            animations[DEAD_DEMON].lastFrame = animations[DEAD_DEMON].frames.size() - 1;
            animations[DEAD_DEMON].currentFrame = 0;
            animations[DEAD_DEMON].speed = 0.1f;
            animations[DEAD_DEMON].timeLeft = 0.1f;
            animations[DEAD_DEMON].type = ONESHOT_DEMON;
            
            std::cout << "Loaded " << animations[IDLE_DEMON].frames.size() << " frames for idle animation." << std::endl;
        }

        void updateAnimation() override {
            if (state >= animations.size()) return;
            
            AnimationDemon& anim = animations[state];
            anim.timeLeft -= GetFrameTime();
            
            if (anim.timeLeft <= 0) {
                anim.timeLeft = anim.speed;
                
                if (anim.type == REPEATING_DEMON) {
                    anim.currentFrame = (anim.currentFrame + 1) % (anim.lastFrame + 1);
                } else {
                    if (anim.currentFrame < anim.lastFrame) {
                        anim.currentFrame++;
                    } else {
                        // Animation finished
                        if (state == ATTACK_DEMON) {
                            hasFinishedAttack = true;
                            state = IDLE_DEMON;
                        }
                    }
                }
            }
        }

        Rectangle getAnimationFrame() const {
            const AnimationDemon& anim = animations[state];

            return {
                0, 0,
                (float)anim.frames[anim.currentFrame].width,
                (float)anim.frames[anim.currentFrame].height
            };
        }

        void draw() override {
            if (state == DEAD_DEMON && isDead) return;
            
            if (state >= animations.size()) return;
            
            const AnimationDemon& anim = animations[state];
            if (anim.currentFrame >= anim.frames.size()) return;
            
            Rectangle frameRec = getAnimationFrame();
            
            // Calculate destination rectangle
            Rectangle destRec = {
                rect.x + (direction == LEFT_DEMON ? rect.width : 0),
                rect.y,
                frameRec.width * (direction == LEFT_DEMON ? -2.0f : 2.0f),
                frameRec.height * 2.0f
            };
            
            Color tint = isInvulnerable ? ColorAlpha(WHITE, 0.5f) : WHITE;
            
            // Draw the sprite
            DrawTexturePro(anim.frames[anim.currentFrame], frameRec, destRec, Vector2{0, 0}, 0.0f, tint);
            
            // Draw debug boxes
            drawDebugBoxes();
        }

        void takeDamage(int damage) override {
            if (isDead) return;
            currentHealth -= damage;
            if (currentHealth <= 0) {
                currentHealth = 0;
                isDead = true;
                state = DEAD_DEMON;
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

            if (IsKeyDown(KEY_H)) {
                velocity.x = -moveSpeed;
                direction = LEFT_DEMON;
                state = WALK_DEMON;
            }
            else if (IsKeyDown(KEY_K)) {
                velocity.x = moveSpeed;
                direction = RIGHT_DEMON;
                state = WALK_DEMON;
            }
            else {
                state = IDLE_DEMON;
            }

            if (IsKeyPressed(KEY_L) && hasFinishedAttack) {
                state = ATTACK_DEMON;
                hasFinishedAttack = false;
                animations[ATTACK_DEMON].currentFrame = animations[ATTACK_DEMON].firstFrame;
            }
        }

        void applyVelocity() override {
            if (isDead) return;
            
            rect.x += velocity.x * GetFrameTime();
            rect.y += velocity.y * GetFrameTime();
            
            // Apply gravity
            if (!isOnGround) {
                velocity.y += GRAVITY_DEMON * GetFrameTime();
            }
            
            // Check floor collision
            if (rect.y + rect.height > GROUND_LEVEL_DEMON + rect.height) {
                rect.y = GROUND_LEVEL_DEMON;
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
            // Check if state is valid and animations are loaded
            if (state >= animations.size() || animations[state].frames.empty()) {
                return;
            }
            
            const AnimationDemon& anim = animations[state];
            if (anim.currentFrame >= anim.frames.size()) {
                return;
            }
            
            // Get the current frame dimensions
            float frameWidth = (float)anim.frames[anim.currentFrame].width;
            float frameHeight = (float)anim.frames[anim.currentFrame].height;
            
            // Calculate the actual position of the sprite
            float spriteX = rect.x + (direction == LEFT_DEMON ? rect.width : 0);
            float spriteY = rect.y;
            float spriteWidth = frameWidth * 2.0f; // Scale is 2.0
            float spriteHeight = frameHeight * 2.0f;
            
            // Update collision box to match the sprite's actual body
            // Make the collision box about 35% of the sprite width and 70% of the sprite height
            // Position it to be centered horizontally and aligned with the bottom of the sprite
            collisionBox.width = spriteWidth * 0.35f;
            collisionBox.height = spriteHeight * 0.7f;
            
            // Adjust position based on direction
            if (direction == RIGHT_DEMON) {
                collisionBox.x = spriteX + (spriteWidth - collisionBox.width) / 2;
            } else {
                collisionBox.x = spriteX - spriteWidth + (spriteWidth - collisionBox.width) / 2;
            }
            
            collisionBox.y = spriteY + spriteHeight - collisionBox.height;
            
            // Update hit box based on state and direction
            if (state == ATTACK_DEMON) {
                // Attack hit box extends in front of the demon
                if (direction == RIGHT_DEMON) {
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
        
        void loadTextures() override {
            // This is handled by loadAnimations() for the Demon class
        }
};

#endif // DEMON_H
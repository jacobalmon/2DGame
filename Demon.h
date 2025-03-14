#include <raylib.h>
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>
#include <algorithm>  // For sorting files by name

namespace fs = std::filesystem;

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

class Demon {
    public:
        Rectangle rect;
        Vector2 velocity;
        DirectionDemon direction;
        CurrentStateDemon state;
        bool isAttacking = false;
        bool hasFinishedAttack = true;

        // Health related to damage and death
        int health;
        bool isDead = false;

        std::vector<AnimationDemon> animations;

        // Sound Variables.
        Sound deathSound;
        Sound explosionSound;
        Sound attackSound;
        Sound hurtSound;
        Sound dialogueSound;
        Sound walkSound;
        Sound laughSound;

        bool isWalkingSoundPlaying = false;

        Demon(Vector2 position) {
            rect = { position.x, position.y, 64.0f, 64.0f };
            velocity = { 0.0f, 0.0f };
            direction = RIGHT_DEMON;
            state = IDLE_DEMON;

            // Initialize health
            health = 100;

            // Load animations from directories
            loadAnimations();

            // Debug: Check if the textures load correctly
            std::cout << "Loaded " << animations[IDLE_DEMON].frames.size() << " frames for idle animation." << std::endl;
        }

        ~Demon() {
            // Unload all textures for each animation
            for (auto& anim : animations) {
                for (auto& frame : anim.frames) {
                    UnloadTexture(frame);  // Unload individual frames
                }
            }
            UnloadSound(attackSound);
            UnloadSound(laughSound);
            UnloadSound(deathSound);
            UnloadSound(dialogueSound);
            UnloadSound(hurtSound);
            UnloadSound(explosionSound);
            UnloadSound(walkSound);
        }

        void loadAnimations() {
            std::vector<std::string> animationFolders = { 
                "assets/Demon/individual sprites/01_demon_idle", 
                "assets/Demon/individual sprites/02_demon_walk", 
                "assets/Demon/individual sprites/03_demon_cleave", 
                "assets/Demon/individual sprites/04_demon_take_hit", 
                "assets/Demon/individual sprites/05_demon_death" 
            };

            for (int i = 0; i < animationFolders.size(); ++i) {
                AnimationDemon anim;
                anim.speed = 0.1f;
                anim.timeLeft = anim.speed;
                anim.type = (i == ATTACK_DEMON) ? ONESHOT_DEMON : REPEATING_DEMON;
                
                std::vector<std::string> framePaths;
                
                // Collect all PNG paths from the directory
                for (const auto& entry : fs::directory_iterator(animationFolders[i])) {
                    if (entry.path().extension() == ".png") {
                        framePaths.push_back(entry.path().string());
                        std::cout << "Found PNG: " << entry.path().string() << std::endl;
                    }
                }

                // Sort the paths to ensure frames are loaded in the correct order (by filename)
                std::sort(framePaths.begin(), framePaths.end(), [](const std::string& a, const std::string& b) {
                    // Custom sort by numeric value in the filename, assuming filenames have numbers like "frame1.png", "frame2.png", etc.
                    std::string baseA = a.substr(a.find_last_of("/\\") + 1); // Get the filename
                    std::string baseB = b.substr(b.find_last_of("/\\") + 1); // Get the filename
                    
                    // Sort numerically if filenames contain numbers
                    int numA = std::stoi(baseA.substr(baseA.find_first_of("0123456789")));
                    int numB = std::stoi(baseB.substr(baseB.find_first_of("0123456789")));
                    return numA < numB;
                });

                std::cout << "Loading " << framePaths.size() << " frames." << std::endl;

                // Load textures in sorted order
                for (const auto& path : framePaths) {
                    Texture2D texture = LoadTexture(path.c_str());
                    if (texture.id == 0) {
                        std::cerr << "Failed to load texture: " << path << std::endl;
                    } else {
                        anim.frames.push_back(texture);
                        std::cout << "Loaded texture: " << path << std::endl;
                    }
                }

                anim.firstFrame = 0;
                anim.lastFrame = anim.frames.size() - 1;
                animations.push_back(anim);
            }
        }

        void loadSounds() {
            laughSound = LoadSound("sounds/demon/demon-2-102993.wav");
            deathSound = LoadSound("sounds/demon/demonic-roar-40349.wav");
            dialogueSound = LoadSound("sounds/demon/devil-says2-73855.wav");
            explosionSound = LoadSound("sounds/demon/large-explosion-100420.wav");
            hurtSound = LoadSound("sounds/demon/mixkit-fantasy-monster-grunt-1977.wav");
            walkSound = LoadSound("sounds/demon/stompwav-14753.wav");
            attackSound = LoadSound("sounds/demon/sword-clash-1-6917.wav");
        }

        void updateAnimation() {
            AnimationDemon& anim = animations[state];
            float deltaTime = GetFrameTime();
            anim.timeLeft -= deltaTime;
        
            if (isDead) {
                state = DEAD_DEMON;
                if (anim.timeLeft <= 0) {
                    anim.timeLeft = anim.speed;
                    anim.currentFrame++;
                    if (anim.currentFrame > anim.lastFrame) {
                        anim.currentFrame = anim.lastFrame;
                        return;
                    }
                }
            }
            else if (state == HURT_DEMON) {
                if (anim.timeLeft <= 0) {
                    anim.timeLeft = anim.speed;
                    anim.currentFrame++;
                    if (anim.currentFrame > anim.lastFrame) {
                        anim.currentFrame = anim.firstFrame;
                        state = IDLE_DEMON;
                    }
                }
            }
            else if (state == ATTACK_DEMON) {
                if (anim.timeLeft <= 0) {
                    anim.timeLeft = anim.speed;
                    anim.currentFrame++;
                    if (anim.currentFrame > anim.lastFrame) {
                        anim.currentFrame = anim.lastFrame;  // Hold on the last frame
                        hasFinishedAttack = true;            // Mark attack as finished
                        state = IDLE_DEMON;                  // Transition back to idle
                        // Optionally reset the idle animation frame:
                        // animations[IDLE_DEMON].currentFrame = animations[IDLE_DEMON].firstFrame;
                        printf("Attack animation finished: now going to idle\n");
                    }
                }
            }
            else {
                if (anim.timeLeft <= 0) {
                    anim.timeLeft = anim.speed;
                    anim.currentFrame++;
                    if (anim.currentFrame > anim.lastFrame) {
                        if (anim.type == REPEATING_DEMON) {
                            anim.currentFrame = anim.firstFrame;
                        } else {
                            anim.currentFrame = anim.lastFrame;
                            if (state != ATTACK_DEMON) { // Only go back to idle if not attacking
                                state = IDLE_DEMON;
                            }
                        }
                    }
                }
            }
        
            // Debugging log
            printf("Current state: %d, currentFrame: %d\n", state, anim.currentFrame);
        }               

        Rectangle getAnimationFrame() const {
            const AnimationDemon& anim = animations[state];

            return {
                0.0f, 0.0f,                        // No X, Y offset
                (float)anim.frames[anim.currentFrame].width,  // Width of frame
                (float)anim.frames[anim.currentFrame].height  // Height of frame
            };
        }

        void draw() const {
            Rectangle source = getAnimationFrame();
            float scale = 5.0f;

            Rectangle dest = { rect.x, rect.y, rect.width * scale, rect.height * scale };

            if (direction == LEFT_DEMON) {
                source.width = -source.width;  // Flip frame for left movement
                source.x += source.width;  // Adjust the X offset after flipping
            }

            // Draw the frame from the texture corresponding to the current animation frame
            DrawTexturePro(animations[state].frames[animations[state].currentFrame], source, dest, { 0, 0 }, 0.0f, WHITE);
        }

        void move() {
            // Don't allow any movement if the demon is dead or hurt
            if (isDead || state == ATTACK_DEMON || state == HURT_DEMON) {
                velocity.x = 0.0f;  // Stop movement
                return;
            }
    
            float moveSpeed = 100.0f;
            velocity.x = 0.0f;
    
            // Movement controls (walking)
            if (IsKeyDown(KEY_H)) {
                velocity.x = -moveSpeed;
                if (state != HURT_DEMON && state != ATTACK_DEMON) {
                    if (state != WALK_DEMON) {  // Only change to WALK_DEMON if not already walking
                        state = WALK_DEMON;
                        animations[WALK_DEMON].currentFrame = animations[WALK_DEMON].firstFrame;
                    }
                }
                direction = RIGHT_DEMON;
    
                // Play walking sound if it's not already playing
                if (!isWalkingSoundPlaying) {
                    PlaySound(walkSound);
                     SetSoundPitch(walkSound, 1.4f);
                    isWalkingSoundPlaying = true;
                }
            }
            else if (IsKeyDown(KEY_K)) {
                velocity.x = moveSpeed;
                if (state != HURT_DEMON && state != ATTACK_DEMON) {
                    if (state != WALK_DEMON) {  // Only change to WALK_DEMON if not already walking
                        state = WALK_DEMON;
                        animations[WALK_DEMON].currentFrame = animations[WALK_DEMON].firstFrame;
                    }
                }
                direction = LEFT_DEMON;
    
                // Play walking sound if it's not already playing
                if (!isWalkingSoundPlaying) {
                    PlaySound(walkSound);
                    SetSoundPitch(walkSound, 1.4f);
                    isWalkingSoundPlaying = true;
                }
            }
            else {
                if (state != HURT_DEMON && state != ATTACK_DEMON) {
                    if (state != IDLE_DEMON) {  // Only change to IDLE_DEMON if not already idle
                        state = IDLE_DEMON;
                        animations[IDLE_DEMON].currentFrame = animations[IDLE_DEMON].firstFrame;
                    }
                }
    
                // Stop walking sound when key is released
                if (isWalkingSoundPlaying) {
                    StopSound(walkSound);
                    isWalkingSoundPlaying = false;
                }
            }
    
            // Attack control: allow attack only if the previous attack has finished
            if (IsKeyPressed(KEY_L) && hasFinishedAttack) {
                state = ATTACK_DEMON;
                hasFinishedAttack = false;
                animations[ATTACK_DEMON].currentFrame = animations[ATTACK_DEMON].firstFrame;
                velocity.x = 0.0f;  // Stop any movement during attack
                PlaySound(attackSound);
            }
    
            // Damage control (only if the demon is not dead)
            if (IsKeyPressed(KEY_T) && !isDead) {
                takeDamage(10);
            }
        }           

        void applyVelocity() {
            // If the demon is hurt or dead, don't apply velocity
            if (state == HURT_DEMON || isDead) {
                velocity.x = 0.0f;  // Stop movement
                velocity.y = 0.0f;  // Prevent vertical movement too
            }
        
            float deltaTime = GetFrameTime();
            rect.x += velocity.x * deltaTime;
            rect.y += velocity.y * deltaTime;
        }        

        void takeDamage(int damage) {
            if (isDead) return;  // Do nothing if the demon is already dead
        
            // Reduce health
            health -= damage;
            PlaySound(hurtSound);
            StopSound(walkSound);
            if (health <= 0) {
                health = 0;
                isDead = true;
                state = DEAD_DEMON;
                
                // Start the death animation from the first frame
                animations[DEAD_DEMON].currentFrame = animations[DEAD_DEMON].firstFrame;
                animations[DEAD_DEMON].timeLeft = animations[DEAD_DEMON].speed;
                PlaySound(deathSound);
                PlaySound(explosionSound);
            } else {
                state = HURT_DEMON;  // Trigger hurt animation
                animations[HURT_DEMON].currentFrame = animations[HURT_DEMON].firstFrame;
                animations[HURT_DEMON].timeLeft = animations[HURT_DEMON].speed;
            }
        }
        
};
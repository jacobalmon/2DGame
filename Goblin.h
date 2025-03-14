#include <raylib.h>
#include <vector>

// Directions for Goblin movement (LEFT_GOBLIN or RIGHT_GOBLIN)
enum DirectionGoblin {
    LEFT_GOBLIN = -1,
    RIGHT_GOBLIN = 1
};

// Possible states for the Goblin (Idle, Walking, Attacking, Dead)
enum CurrentStateGoblin {
    DEAD_GOBLIN = 0,
    ATTACK_CLUB = 1,
    ATTACK_STOMP = 2,
    ATTACK_AOE = 3,
    IDLE_GOBLIN = 4,
    WALK_GOBLIN = 5
};

// Animation types for Goblin (Repeating or One-shot)
enum AnimationTypeGoblin {
    REPEATING_GOBLIN,
    ONESHOT_GOBLIN
};

// Struct that holds animation data for each state
struct AnimationGoblin {
    int firstFrame;      // First frame of the animation
    int lastFrame;       // Last frame of the animation
    int currentFrame;    // Current frame being displayed
    int offset;          // Offset for animation positioning
    float speed;         // Speed of animation (time per frame)
    float timeLeft;      // Remaining time until frame change
    AnimationTypeGoblin type;  // Type of animation (repeating or one-shot)
};

class Goblin {
    public:
        Rectangle rect;          
        Vector2 velocity;        
        Texture2D spriteSheet;   
        DirectionGoblin direction;
        CurrentStateGoblin state;
        std::vector<AnimationGoblin> animations;
        std::vector<Texture2D> sprites;
    
        bool isAttacking = false;
        bool hasFinishedAttack = true;
        int health = 100; // Goblin's health
        bool isWalkingSoundPlaying = false;

        Sound aoeSound;
        Sound deadSound;
        Sound attackSound;
        Sound walkSound;
    
        Goblin(Vector2 position) {
            rect = (Rectangle) {position.x, position.y, 64.0f, 64.0f};
            velocity = (Vector2) {0.0f, 0.0f}; 
            direction = RIGHT_GOBLIN;
            state = IDLE_GOBLIN;
    
            animations = {
                {0, 8, 0, 0, 0.1f, 0.1f, ONESHOT_GOBLIN},  // Dead Goblin
                {0, 9, 0, 5, 0.1f, 0.1f, ONESHOT_GOBLIN},  // Attack Club
                {5, 9, 0, 0, 0.1f, 0.1f, ONESHOT_GOBLIN},  // Attack Stomp
                {0, 23, 0, 0, 0.1f, 0.1f, ONESHOT_GOBLIN}, // Attack AoE
                {0, 6, 0, 0, 0.1f, 0.1f, REPEATING_GOBLIN}, // Idle Goblin
                {0, 7, 0, 0, 0.1f, 0.1f, REPEATING_GOBLIN}  // Walking Goblin
            };
        }
    
        ~Goblin() {
            for (auto& sprite : sprites) {
                UnloadTexture(sprite);
            }

            UnloadSound(deadSound);
            UnloadSound(attackSound);
            UnloadSound(aoeSound);
            UnloadSound(walkSound);
        }
    
        void loadTextures() {
            sprites.resize(6);
            sprites[DEAD_GOBLIN] = LoadTexture("assets/Goblin/Hobgoblin Die/Hobgoblin Beheaded.png");
            sprites[ATTACK_CLUB] = LoadTexture("assets/Goblin/Hobgoblin Attack 1 & 2/Hobgoblin Attack 1 and 2.png");
            sprites[ATTACK_STOMP] = LoadTexture("assets/Goblin/Hobgoblin Attack 1 & 2/Hobgoblin Attack 1 and 2.png");
            sprites[ATTACK_AOE] = LoadTexture("assets/Goblin/Hobgoblin Attack 3/Hobgoblin Attack 3.png");
            sprites[IDLE_GOBLIN] = LoadTexture("assets/Goblin/Hobgoblin Idle/GoblinK Idle.png");
            sprites[WALK_GOBLIN] = LoadTexture("assets/Goblin/Hobgoblin Walk/Hobgoblin Walk.png");
        }

        void loadSounds() {
            attackSound = LoadSound("sounds/goblin/thud-82914.wav");
            deadSound = LoadSound("sounds/goblin/goblin-scream-87564.wav");
            walkSound = LoadSound("sounds/goblin/walking-sound-effect-272246.wav");
            aoeSound = LoadSound("sounds/goblin/elemental-magic-spell-cast-d-228349.wav");
        }
    
        void updateAnimation() {
            if (state == DEAD_GOBLIN) {
                AnimationGoblin& anim = animations[DEAD_GOBLIN];
                float deltaTime = GetFrameTime();
                anim.timeLeft -= deltaTime;
        
                if (anim.timeLeft <= 0) {
                    anim.timeLeft = anim.speed;
                    if (anim.currentFrame < anim.lastFrame) {
                        anim.currentFrame++; // Progress the death animation
                    }
                }
                return; // Prevent any other updates once dead
            }
        
            AnimationGoblin& anim = animations[state];
            float deltaTime = GetFrameTime();
            
            anim.timeLeft -= deltaTime;
            if (anim.timeLeft <= 0) {
                anim.timeLeft = anim.speed;
                anim.currentFrame++;
        
                if (state == ATTACK_CLUB && anim.currentFrame > 4) {
                    anim.currentFrame = 4;
                    hasFinishedAttack = true;
                    state = IDLE_GOBLIN;
                }
        
                if (anim.currentFrame > anim.lastFrame) {
                    if (anim.type == REPEATING_GOBLIN) {
                        anim.currentFrame = anim.firstFrame;
                    } else if (anim.type == ONESHOT_GOBLIN) {
                        anim.currentFrame = anim.lastFrame;
                        hasFinishedAttack = true;
                        state = IDLE_GOBLIN;
                    }
                }
            }
        }        
    
        Rectangle getAnimationFrame() const {
            const AnimationGoblin& anim = animations[state];
            int frameWidth = sprites[state].width / (anim.lastFrame + 1);
            int frameHeight = sprites[state].height;
    
            return (Rectangle) {
                (float)frameWidth * anim.currentFrame, 0, (float)frameWidth, (float)frameHeight
            };
        }
    
        void draw() const {
            Rectangle source = getAnimationFrame();
            float scale = 2.0f;
    
            Rectangle dest = { 
                rect.x, rect.y, 
                rect.width * scale,   
                rect.height * scale   
            };
    
            source.width *= direction;
            DrawTexturePro(sprites[state], source, dest, {0, 0}, 0.0f, WHITE);
        }
    
        void move() {
            if (!hasFinishedAttack || state == DEAD_GOBLIN) return;  
            
            float moveSpeed = 180.0f;
            velocity.x = 0.0f;  
            
            if (IsKeyDown(KEY_X)) {
                velocity.x = -moveSpeed;
                direction = LEFT_GOBLIN;
                state = WALK_GOBLIN;

                if (!isWalkingSoundPlaying) {
                    PlaySound(walkSound);
                    isWalkingSoundPlaying = true;
                }
            } 
            else if (IsKeyDown(KEY_C)) {
                velocity.x = moveSpeed;
                direction = RIGHT_GOBLIN;
                state = WALK_GOBLIN;

                if (!isWalkingSoundPlaying) {
                    PlaySound(walkSound);
                    isWalkingSoundPlaying = true;
                }
            } 
            else {
                state = IDLE_GOBLIN;

                if (isWalkingSoundPlaying) {
                    StopSound(walkSound);
                    isWalkingSoundPlaying = false;
                }
            }
        
            if (IsKeyPressed(KEY_KP_1) && hasFinishedAttack) {
                state = ATTACK_CLUB;
                hasFinishedAttack = false;
                velocity.x = 0;
                animations[ATTACK_CLUB].currentFrame = animations[ATTACK_CLUB].firstFrame;
                PlaySound(attackSound);
            }
            if (IsKeyPressed(KEY_KP_2) && hasFinishedAttack) {
                state = ATTACK_STOMP;
                hasFinishedAttack = false;
                velocity.x = 0;
                animations[ATTACK_STOMP].currentFrame = animations[ATTACK_STOMP].firstFrame;
                PlaySound(attackSound);
            }
            if (IsKeyPressed(KEY_KP_3) && hasFinishedAttack) {
                state = ATTACK_AOE;
                hasFinishedAttack = false;
                velocity.x = 0;
                animations[ATTACK_AOE].currentFrame = animations[ATTACK_AOE].firstFrame;
                PlaySound(aoeSound);
            }
        
            // **New: Press 'D' to make the Goblin die**
            if (IsKeyPressed(KEY_D)) {
                takeDamage(health); // Instantly set health to 0 and trigger death
            }
        }          
    
        void applyVelocity() {
            if (hasFinishedAttack && state != DEAD_GOBLIN) {
                rect.x += velocity.x * GetFrameTime();
            }
        }    
    
        void takeDamage(int damage) {
            if (state == DEAD_GOBLIN) return; // If already dead, ignore damage
    
            health -= damage;
            if (health <= 0) {
                health = 0;
                die();
            }
        }
    
        void die() {
            state = DEAD_GOBLIN;
            velocity.x = 0;
            animations[DEAD_GOBLIN].currentFrame = animations[DEAD_GOBLIN].firstFrame;
            PlaySound(deadSound);
        }
    };
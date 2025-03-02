#ifndef ASEPRITE_LOADER_H
#define ASEPRITE_LOADER_H

#include "raylib.h"
#include "parson.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

// Structure to hold frame data
struct AsepriteFrame {
    Rectangle sourceRect;  // Source rectangle in the sprite sheet
    float duration;        // Duration of the frame in seconds
};

// Structure to hold animation tag data
struct AsepriteTag {
    std::string name;      // Name of the animation tag
    int from;              // First frame index
    int to;                // Last frame index
    std::string direction; // Animation direction (forward, reverse, pingpong)
};

// Class to handle Aseprite animations
class AsepriteAnimation {
private:
    Texture2D spriteSheet;                  // The sprite sheet texture
    std::vector<AsepriteFrame> frames;      // All frames from the sprite sheet
    std::map<std::string, AsepriteTag> tags; // Animation tags
    
    int currentFrame;                       // Current frame index
    float frameTimer;                       // Timer for current frame
    std::string currentTag;                 // Current playing animation tag
    bool isPlaying;                         // Is animation playing
    bool isLooping;                         // Should animation loop
    int direction;                          // 1 for forward, -1 for reverse
    bool pingPongReverse;                   // For pingpong animations

    // Parse JSON data from Aseprite export
    bool ParseJSON(const char* jsonPath) {
        // Parse JSON file
        JSON_Value* root_value = json_parse_file(jsonPath);
        if (root_value == NULL) {
            std::cerr << "Failed to parse JSON file: " << jsonPath << std::endl;
            return false;
        }

        JSON_Object* root_object = json_value_get_object(root_value);
        JSON_Array* frames_array = json_object_get_array(root_object, "frames");
        
        if (frames_array == NULL) {
            std::cerr << "No frames found in JSON file" << std::endl;
            json_value_free(root_value);
            return false;
        }

        // Get frame data
        size_t frame_count = json_array_get_count(frames_array);
        frames.resize(frame_count);
        
        for (size_t i = 0; i < frame_count; i++) {
            JSON_Object* frame_object = json_array_get_object(frames_array, i);
            JSON_Object* frame_data = json_object_get_object(frame_object, "frame");
            
            if (frame_data) {
                frames[i].sourceRect.x = (float)json_object_get_number(frame_data, "x");
                frames[i].sourceRect.y = (float)json_object_get_number(frame_data, "y");
                frames[i].sourceRect.width = (float)json_object_get_number(frame_data, "w");
                frames[i].sourceRect.height = (float)json_object_get_number(frame_data, "h");
                
                // Duration is in milliseconds in Aseprite, convert to seconds
                frames[i].duration = (float)json_object_get_number(frame_object, "duration") / 1000.0f;
            }
        }

        // Get tag data if available
        JSON_Array* tags_array = json_object_get_array(root_object, "meta");
        if (tags_array != NULL) {
            JSON_Object* meta = json_value_get_object(json_array_get_value(tags_array, 0));
            JSON_Array* meta_tags = json_object_get_array(meta, "frameTags");
            
            if (meta_tags != NULL) {
                size_t tag_count = json_array_get_count(meta_tags);
                
                for (size_t i = 0; i < tag_count; i++) {
                    JSON_Object* tag_object = json_array_get_object(meta_tags, i);
                    
                    AsepriteTag tag;
                    tag.name = json_object_get_string(tag_object, "name");
                    tag.from = (int)json_object_get_number(tag_object, "from");
                    tag.to = (int)json_object_get_number(tag_object, "to");
                    tag.direction = json_object_get_string(tag_object, "direction");
                    
                    tags[tag.name] = tag;
                }
            }
        }

        json_value_free(root_value);
        return true;
    }

public:
    AsepriteAnimation() : 
        currentFrame(0), 
        frameTimer(0.0f), 
        isPlaying(false), 
        isLooping(true),
        direction(1),
        pingPongReverse(false) {
        spriteSheet = { 0 };
    }

    ~AsepriteAnimation() {
        Unload();
    }

    // Load animation from sprite sheet and JSON data
    bool LoadFromFiles(const char* pngPath, const char* jsonPath) {
        // Load sprite sheet texture
        spriteSheet = LoadTexture(pngPath);
        if (spriteSheet.id == 0) {
            std::cerr << "Failed to load sprite sheet: " << pngPath << std::endl;
            return false;
        }

        // Parse JSON data
        if (!ParseJSON(jsonPath)) {
            UnloadTexture(spriteSheet);
            spriteSheet = { 0 };
            return false;
        }

        return true;
    }

    // Unload resources
    void Unload() {
        if (spriteSheet.id != 0) {
            UnloadTexture(spriteSheet);
            spriteSheet = { 0 };
        }
        frames.clear();
        tags.clear();
    }

    // Play animation by tag name
    bool Play(const std::string& tagName, bool loop = true) {
        auto it = tags.find(tagName);
        if (it == tags.end()) {
            // If tag not found, check if it's a number (frame index)
            try {
                int frameIndex = std::stoi(tagName);
                if (frameIndex >= 0 && frameIndex < (int)frames.size()) {
                    currentFrame = frameIndex;
                    frameTimer = 0.0f;
                    isPlaying = true;
                    isLooping = false;
                    currentTag = "";
                    return true;
                }
            } catch (...) {
                // Not a number, so tag really doesn't exist
                std::cerr << "Animation tag not found: " << tagName << std::endl;
                return false;
            }
            
            std::cerr << "Animation tag not found: " << tagName << std::endl;
            return false;
        }

        const AsepriteTag& tag = it->second;
        currentFrame = tag.from;
        frameTimer = 0.0f;
        isPlaying = true;
        isLooping = loop;
        currentTag = tagName;
        
        // Set direction based on tag direction
        if (tag.direction == "forward") {
            direction = 1;
            pingPongReverse = false;
        } else if (tag.direction == "reverse") {
            direction = -1;
            pingPongReverse = false;
        } else if (tag.direction == "pingpong") {
            direction = 1;
            pingPongReverse = false;
        }
        
        return true;
    }

    // Stop animation
    void Stop() {
        isPlaying = false;
    }

    // Update animation
    void Update(float deltaTime) {
        if (!isPlaying) return;
        
        frameTimer += deltaTime;
        
        // Check if it's time to advance to the next frame
        if (frameTimer >= frames[currentFrame].duration) {
            frameTimer = 0.0f;
            
            // Handle different animation types
            if (currentTag.empty()) {
                // Single frame or no tag
                if (isLooping) {
                    currentFrame = (currentFrame + 1) % frames.size();
                } else {
                    currentFrame++;
                    if (currentFrame >= (int)frames.size()) {
                        currentFrame = (int)frames.size() - 1;
                        isPlaying = false;
                    }
                }
            } else {
                // Tagged animation
                const AsepriteTag& tag = tags[currentTag];
                
                if (tag.direction == "pingpong") {
                    // Ping-pong animation
                    if (!pingPongReverse) {
                        currentFrame += 1;
                        if (currentFrame > tag.to) {
                            currentFrame = tag.to - 1;
                            pingPongReverse = true;
                            if (currentFrame < tag.from) currentFrame = tag.from;
                        }
                    } else {
                        currentFrame -= 1;
                        if (currentFrame < tag.from) {
                            currentFrame = tag.from + 1;
                            pingPongReverse = false;
                            if (currentFrame > tag.to) currentFrame = tag.to;
                            
                            // Check if we should stop looping
                            if (!isLooping) {
                                isPlaying = false;
                                currentFrame = tag.from;
                            }
                        }
                    }
                } else {
                    // Forward or reverse animation
                    currentFrame += direction;
                    
                    if (direction > 0 && currentFrame > tag.to) {
                        if (isLooping) {
                            currentFrame = tag.from;
                        } else {
                            currentFrame = tag.to;
                            isPlaying = false;
                        }
                    } else if (direction < 0 && currentFrame < tag.from) {
                        if (isLooping) {
                            currentFrame = tag.to;
                        } else {
                            currentFrame = tag.from;
                            isPlaying = false;
                        }
                    }
                }
            }
        }
    }

    // Draw the current frame
    void Draw(float x, float y, float scale = 1.0f, Color tint = WHITE) {
        if (spriteSheet.id == 0 || frames.empty() || currentFrame < 0 || currentFrame >= (int)frames.size()) {
            return;
        }
        
        Rectangle sourceRect = frames[currentFrame].sourceRect;
        Rectangle destRect = { 
            x, 
            y, 
            sourceRect.width * scale, 
            sourceRect.height * scale 
        };
        
        DrawTexturePro(spriteSheet, sourceRect, destRect, Vector2{0, 0}, 0.0f, tint);
    }

    // Draw the current frame with flipping options
    void DrawEx(float x, float y, bool flipX = false, float scale = 1.0f, Color tint = WHITE) {
        if (spriteSheet.id == 0 || frames.empty() || currentFrame < 0 || currentFrame >= (int)frames.size()) {
            return;
        }
        
        Rectangle sourceRect = frames[currentFrame].sourceRect;
        Rectangle destRect = { 
            x, 
            y, 
            sourceRect.width * scale * (flipX ? -1.0f : 1.0f), 
            sourceRect.height * scale 
        };
        
        Vector2 origin = { 0, 0 };
        if (flipX) {
            origin.x = sourceRect.width * scale;
        }
        
        DrawTexturePro(spriteSheet, sourceRect, destRect, origin, 0.0f, tint);
    }

    // Check if animation is playing
    bool IsPlaying() const {
        return isPlaying;
    }

    // Get current frame width
    float GetWidth() const {
        if (currentFrame >= 0 && currentFrame < (int)frames.size()) {
            return frames[currentFrame].sourceRect.width;
        }
        return 0.0f;
    }

    // Get current frame height
    float GetHeight() const {
        if (currentFrame >= 0 && currentFrame < (int)frames.size()) {
            return frames[currentFrame].sourceRect.height;
        }
        return 0.0f;
    }

    // Get current animation tag
    std::string GetCurrentTag() const {
        return currentTag;
    }

    // Get all available animation tags
    std::vector<std::string> GetAllTags() const {
        std::vector<std::string> tagNames;
        for (const auto& pair : tags) {
            tagNames.push_back(pair.first);
        }
        return tagNames;
    }
};

#endif // ASEPRITE_LOADER_H 
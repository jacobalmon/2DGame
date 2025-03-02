#include "AsepriteLoader.h"
#include <filesystem>
#include <iostream>

// Utility function to check if a file exists
bool FileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

// Utility function to load an Aseprite animation from a file
AsepriteAnimation* LoadAsepriteAnimation(const std::string& pngPath, const std::string& jsonPath) {
    // Check if files exist
    if (!FileExists(pngPath)) {
        std::cerr << "Error: PNG file not found: " << pngPath << std::endl;
        return nullptr;
    }
    
    if (!FileExists(jsonPath)) {
        std::cerr << "Error: JSON file not found: " << jsonPath << std::endl;
        return nullptr;
    }
    
    // Create and load animation
    AsepriteAnimation* animation = new AsepriteAnimation();
    if (!animation->LoadFromFiles(pngPath.c_str(), jsonPath.c_str())) {
        std::cerr << "Error: Failed to load animation from files" << std::endl;
        delete animation;
        return nullptr;
    }
    
    return animation;
}

// Utility function to unload an Aseprite animation
void UnloadAsepriteAnimation(AsepriteAnimation* animation) {
    if (animation) {
        animation->Unload();
        delete animation;
    }
}

// Utility function to get the base name of a file without extension
std::string GetFileBaseName(const std::string& filePath) {
    std::filesystem::path path(filePath);
    return path.stem().string();
}

// Utility function to generate PNG and JSON paths from an Aseprite file path
bool GetAsepriteExportPaths(const std::string& asepritePath, std::string& outPngPath, std::string& outJsonPath) {
    if (!FileExists(asepritePath)) {
        std::cerr << "Error: Aseprite file not found: " << asepritePath << std::endl;
        return false;
    }
    
    std::filesystem::path path(asepritePath);
    std::string baseName = path.stem().string();
    std::string directory = path.parent_path().string();
    
    if (directory.empty()) {
        directory = ".";
    }
    
    outPngPath = directory + "/" + baseName + "_sheet.png";
    outJsonPath = directory + "/" + baseName + "_sheet.json";
    
    return true;
}

// Utility function to load all animations from a directory
std::map<std::string, AsepriteAnimation*> LoadAllAnimationsFromDirectory(const std::string& directory) {
    std::map<std::string, AsepriteAnimation*> animations;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == ".json") {
                std::string jsonPath = entry.path().string();
                std::string baseName = entry.path().stem().string();
                
                // Check if this is an Aseprite JSON file (ends with _sheet.json)
                if (baseName.length() > 6 && baseName.substr(baseName.length() - 6) == "_sheet") {
                    // Get the animation name (remove _sheet suffix)
                    std::string animName = baseName.substr(0, baseName.length() - 6);
                    
                    // Construct the PNG path
                    std::string pngPath = entry.path().parent_path().string() + "/" + baseName + ".png";
                    
                    // Try to load the animation
                    AsepriteAnimation* animation = LoadAsepriteAnimation(pngPath, jsonPath);
                    if (animation) {
                        animations[animName] = animation;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading animations from directory: " << e.what() << std::endl;
    }
    
    return animations;
}

// Utility function to unload all animations in a map
void UnloadAllAnimations(std::map<std::string, AsepriteAnimation*>& animations) {
    for (auto& pair : animations) {
        UnloadAsepriteAnimation(pair.second);
    }
    animations.clear();
} 
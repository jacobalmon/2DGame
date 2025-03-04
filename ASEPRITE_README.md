# Using Aseprite Files in Your Game

This guide explains how to use Aseprite animations in your Raylib game.

## Overview

The `AsepriteLoader` class provides a way to load and display animations created in Aseprite. It works with exported sprite sheets and JSON data files.

## Exporting from Aseprite

### Method 1: Using the Export Dialog

1. Open your animation in Aseprite
2. Go to `File > Export Sprite Sheet`
3. In the export dialog:
   - Set Layout to "Horizontal Strip"
   - Check "JSON Data" and set Format to "Array"
   - Set the output paths for both the PNG and JSON files
   - Click "Export"

### Method 2: Using the Export Script

We've included a script to help with exporting:

```bash
./export_aseprite.sh path/to/your/file.aseprite
```

This will create `filename_sheet.png` and `filename_sheet.json` in the same directory as your Aseprite file.

## Using in Your Game

### Basic Usage

```cpp
#include "AsepriteLoader.h"

// Create an animation object
AsepriteAnimation animation;

// Load the animation from exported files
if (animation.LoadFromFiles("character_sheet.png", "character_sheet.json")) {
    // Play a specific animation tag (defined in Aseprite)
    animation.Play("idle");
}

// In your game loop:
void Update() {
    // Update animation with delta time
    animation.Update(GetFrameTime());
}

void Draw() {
    // Draw the current frame
    animation.Draw(400, 300, 2.0f);  // x, y, scale
    
    // Or with flipping
    animation.DrawEx(400, 300, true, 2.0f);  // x, y, flipX, scale
}
```

### Using the AsepriteCharacter Class

We've included an example character class that shows how to integrate Aseprite animations:

```cpp
#include "AsepriteExample.h"

// Create a character
AsepriteCharacter character(Vector2{400, 300});

// Load animations
character.LoadAnimations("character_sheet.png", "character_sheet.json");

// In your game loop:
void Update() {
    character.Update(GetFrameTime());
}

void Draw() {
    character.Draw();
}
```

## Testing Your Animations

Run the test program to see if your animations work correctly:

```bash
make OBJS="AsepriteTest.cpp parson.c" && ./game
```

This will look for animation files in:
- `assets/character_sheet.png` and `assets/character_sheet.json`
- `assets/Goblin/goblin_sheet.png` and `assets/Goblin/goblin_sheet.json`

## Tips for Creating Animations in Aseprite

1. **Use Tags**: Create tags (via the Timeline panel) to define different animations like "idle", "walk", "attack", etc.

2. **Consistent Frame Size**: Keep a consistent frame size for all frames in your animation.

3. **Frame Duration**: Set appropriate frame durations in Aseprite. These will be preserved in the exported JSON.

4. **Origin Point**: Consider where the "origin" of your character is (usually the feet for platformers). The loader draws from the top-left corner.

## Troubleshooting

- If animations aren't showing, check that both the PNG and JSON files exist and are in the correct format.
- Make sure your Aseprite tags are correctly defined.
- Check the console for any error messages from the loader.

## Advanced Usage

The `AsepriteAnimation` class provides additional methods:

- `IsPlaying()`: Check if an animation is currently playing
- `GetWidth()` and `GetHeight()`: Get the dimensions of the current frame
- Additional customization options in the `AsepriteLoader.h` file 
# Which compiler to use
CC = g++

# === DIRECTORY LOCATIONS =======================

# FOR ENGINE

# Where to find the include folder
ENGINE_INCLUDE_DIRECTORY = .\include\engine

# Where to find the objects folder
ENGINE_OBJECT_DIRECTORY = .\src\engine\obj

# Where to find source code
ENGINE_SOURCE_DIRECTORY = .\src\engine

# FOR GAME

# Where to find the include folder
GAME_INCLUDE_DIRECTORY = .\include\game

# Where to find the objects folder
GAME_OBJECT_DIRECTORY = .\src\game\obj

# Where to find source code
GAME_SOURCE_DIRECTORY = .\src\game

# SDL Include Directory
SDL_INCLUDE = -IC:\TDM-GCC-32\sdl2\include\SDL2

# SDL Library Directory
SDL_LIBRARY = -LC:\TDM-GCC-32\sdl2\lib

# === ARGUMENTS ================================

# Which libraries to include
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

# Additional compilation arguments
COMPILER_FLAGS = -std=c++17 -Wall -Wextra -pedantic

# Compilation arguments
COMPILATION_ARGS = -I $(GAME_INCLUDE_DIRECTORY) -I $(ENGINE_INCLUDE_DIRECTORY) $(SDL_INCLUDE) $(COMPILER_FLAGS)

# === FILES ===================================

# FOR ENGINE

# Header files
_ENGINE_DEPS = Game.h GameState.h Sprite.h Helper.h Music.h Vector2.h Rectangle.h Component.h GameObject.h Sound.h TileSet.h TileMap.h Resources.h InputManager.h Camera.h CameraFollower.h Debug.h RenderLayer.h SpriteAnimator.h SatCollision.h Collider.h Recipes.h Text.h Color.h GameData.h Timer.h Tag.h

# Generate header filepaths
ENGINE_DEPS = $(patsubst %,$(ENGINE_INCLUDE_DIRECTORY)\\%,$(_ENGINE_DEPS))

# Object files
_ENGINE_OBJS = main.o Game.o GameState.o Sprite.o Music.o Component.o GameObject.o Sound.o TileSet.o TileMap.o Resources.o InputManager.o Camera.o Debug.o SpriteAnimator.o Collider.o Recipes.o Text.o

# Generate object filepaths
ENGINE_OBJS = $(patsubst %,$(ENGINE_OBJECT_DIRECTORY)\\%,$(_ENGINE_OBJS))

# FOR GAME

# Header files
_GAME_DEPS = Movement.h Health.h Alien.h Minion.h Projectile.h MainState.h PenguinBody.h PenguinCannon.h TitleState.h EndState.h Hazard.h

# Generate header filepaths
GAME_DEPS = $(patsubst %,$(GAME_INCLUDE_DIRECTORY)\\%,$(_GAME_DEPS))

# Object files
_GAME_OBJS = Movement.o Alien.o Health.o Minion.o Projectile.o MainState.o PenguinBody.o PenguinCannon.o EndState.o Hazard.o TitleState.o

# Generate object filepaths
GAME_OBJS = $(patsubst %,$(GAME_OBJECT_DIRECTORY)\\%,$(_GAME_OBJS))

# === RULES ===================================

# FOR ENGINE

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
# The void cast is simply there to hide output of mkdir
$(ENGINE_OBJECT_DIRECTORY)\\%.o: $(ENGINE_SOURCE_DIRECTORY)\%.cpp $(ENGINE_DEPS)
	$(CC) -c -o $@ $< $(COMPILATION_ARGS)

# FOR GAME

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
# The void cast is simply there to hide output of mkdir
$(GAME_OBJECT_DIRECTORY)\\%.o: $(GAME_SOURCE_DIRECTORY)\%.cpp $(GAME_DEPS) $(ENGINE_DEPS)
	$(CC) -c -o $@ $< $(COMPILATION_ARGS)
	
# Makes the game
game: $(GAME_OBJS) $(ENGINE_OBJS)
	$(CC) $^ $(COMPILATION_ARGS) $(LIBS) $(SDL_LIBRARY) -o $@
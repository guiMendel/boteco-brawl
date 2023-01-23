# Which compiler to use
CC = g++

# === DIRECTORY LOCATIONS =======================

# FOR EDITOR

# Where to find the include folder
EDITOR_INCLUDE_DIRECTORY = .\include\editor

# Where to find the objects folder
EDITOR_OBJECT_DIRECTORY = .\src\editor\obj

# Where to find source code
EDITOR_SOURCE_DIRECTORY = .\src\editor

# FOR ENGINE

# Where to find the include folder
ENGINE_INCLUDE_DIRECTORY = .\include\engine

# Where to find the objects folder
ENGINE_OBJECT_DIRECTORY = .\src\engine\obj

# Where to find source code
ENGINE_SOURCE_DIRECTORY = .\src\engine

# FOR INTEGRATION

# Where to find the include folder
INTEGRATION_INCLUDE_DIRECTORY = .\include\integration

# Where to find the objects folder
INTEGRATION_OBJECT_DIRECTORY = .\src\integration\obj

# Where to find source code
INTEGRATION_SOURCE_DIRECTORY = .\src\integration

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
COMPILATION_ARGS = -I $(GAME_INCLUDE_DIRECTORY) -I $(ENGINE_INCLUDE_DIRECTORY) -I $(EDITOR_INCLUDE_DIRECTORY) -I $(INTEGRATION_INCLUDE_DIRECTORY) $(SDL_INCLUDE) $(COMPILER_FLAGS)

# === FILES ===================================

# FOR ENGINE

# Header files
_ENGINE_DEPS = Game.h GameState.h SpriteRenderer.h Helper.h Music.h Vector2.h Rectangle.h Component.h GameObject.h Sound.h Resources.h InputManager.h Camera.h CameraFollower.h Debug.h Animator.h Collision.h Collider.h Text.h Color.h Timer.h Rigidbody.h PhysicsSystem.h Sprite.h Animation.h AnimationFrame.h ParticleEmitter.h Circle.h Particle.h ParticleSystem.h PhysicsLayerHandler.h PlatformEffector.h TriggerCollisionData.h BoxCollider.h CircleCollider.h Shape.h

# Generate header filepaths
ENGINE_DEPS = $(patsubst %,$(ENGINE_INCLUDE_DIRECTORY)\\%,$(_ENGINE_DEPS))

# Object files
_ENGINE_OBJS = main.o Game.o GameState.o Sprite.o Music.o Component.o GameObject.o Sound.o Resources.o InputManager.o Camera.o Debug.o Animator.o Collider.o Text.o Rigidbody.o PhysicsSystem.o Sprite.o Animation.o AnimationFrame.o SpriteRenderer.o ParticleEmitter.o Particle.o ParticleSystem.o Circle.o Vector2.o Rectangle.o Color.o PhysicsLayerHandler.o PlatformEffector.o TriggerCollisionData.o Collision.o Helper.o BoxCollider.o CircleCollider.o Shape.o

# Generate object filepaths
ENGINE_OBJS = $(patsubst %,$(ENGINE_OBJECT_DIRECTORY)\\%,$(_ENGINE_OBJS))

# FOR INTEGRATION

# Header files
_INTEGRATION_DEPS = ColliderDensity.h GameData.h ObjectRecipes.h RenderLayer.h Tag.h PhysicsLayer.h BuildConfigurations.h

# Generate header filepaths
INTEGRATION_DEPS = $(patsubst %,$(INTEGRATION_INCLUDE_DIRECTORY)\\%,$(_INTEGRATION_DEPS))

# Object files
_INTEGRATION_OBJS = ObjectRecipes.o InitialState.o GameConfiguration.o PhysicsConfiguration.o

# Generate object filepaths
INTEGRATION_OBJS = $(patsubst %,$(INTEGRATION_OBJECT_DIRECTORY)\\%,$(_INTEGRATION_OBJS))

# FOR GAME

# Header files
_GAME_DEPS = MainState.h Movement.h PlayerInput.h Action.h CharacterStateManager.h CharacterState.h CharacterStateRecipes.h CharacterController.h Actions.h KeyboardInput.h ControllerInput.h Player.h PlayerManager.h ControllerDevice.h ParticleFX.h CharacterRepelCollision.h PlatformDrop.h FallOffDeath.h Arena.h Heat.h Damage.h Attack.h Character.h TestCharacter.h GeneralAnimations.h NewAnimationTypes.h TimeScaleManager.h Parry.h GunParry.h LandingAttackEffector.h LandEffector.h ShakeEffectManager.h CharacterBadge.h CharacterVFX.h

# Generate header filepaths
GAME_DEPS = $(patsubst %,$(GAME_INCLUDE_DIRECTORY)\\%,$(_GAME_DEPS))

# Object files
_GAME_OBJS = MainState.o Movement.o PlayerInput.o CharacterStateManager.o CharacterStateRecipes.o CharacterController.o Action.o CharacterState.o Actions.o KeyboardInput.o ControllerInput.o Player.o PlayerManager.o ControllerDevice.o ParticleFX.o CharacterRepelCollision.o PlatformDrop.o FallOffDeath.o Arena.o Heat.o Attack.o Character.o TestCharacter.o GeneralAnimations.o NewAnimationTypes.o TimeScaleManager.o GunParry.o Damage.o LandingAttackEffector.o ShakeEffectManager.o CharacterBadge.o CharacterVFX.o

# Generate object filepaths
GAME_OBJS = $(patsubst %,$(GAME_OBJECT_DIRECTORY)\\%,$(_GAME_OBJS))

# === RULES ===================================

# FOR ENGINE

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
# The void cast is simply there to hide output of mkdir
$(ENGINE_OBJECT_DIRECTORY)\\%.o: $(ENGINE_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(ENGINE_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)

# FOR INTEGRATION

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
# The void cast is simply there to hide output of mkdir
$(INTEGRATION_OBJECT_DIRECTORY)\\%.o: $(INTEGRATION_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(ENGINE_DEPS) $(GAME_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)

# FOR GAME

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
# The void cast is simply there to hide output of mkdir
$(GAME_OBJECT_DIRECTORY)\\%.o: $(GAME_SOURCE_DIRECTORY)\%.cpp $(GAME_DEPS) $(ENGINE_DEPS) $(INTEGRATION_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)
	
# Makes the game
game: $(GAME_OBJS) $(ENGINE_OBJS) $(INTEGRATION_OBJS)
#	./src/editor/componentTable/tableScrapper.sh
	$(CC) $^ $(COMPILATION_ARGS) $(LIBS) $(SDL_LIBRARY) -g -o $@
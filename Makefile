# Which compiler to use
CC = g++


# ==========================================================================================
# DIRECTORY LOCATIONS
# ==========================================================================================



# === ENGINE ===============================================================================


# === EDITOR

# Where to find the include folder
EDITOR_INCLUDE_DIRECTORY = .\include\engine\editor

# Where to find source code
EDITOR_SOURCE_DIRECTORY = .\src\engine\editor

# Where to find the objects folder
EDITOR_OBJECT_DIRECTORY = $(EDITOR_SOURCE_DIRECTORY)\obj

# === WORLD

# Where to find the include folder
WORLD_INCLUDE_DIRECTORY = .\include\engine\world

# Where to find source code
WORLD_SOURCE_DIRECTORY = .\src\engine\world

# Where to find the objects folder
WORLD_OBJECT_DIRECTORY = $(WORLD_SOURCE_DIRECTORY)\obj

# === UI

# Where to find the include folder
UI_INCLUDE_DIRECTORY = .\include\engine\ui

# Where to find source code
UI_SOURCE_DIRECTORY = .\src\engine\ui

# Where to find the objects folder
UI_OBJECT_DIRECTORY = $(UI_SOURCE_DIRECTORY)\obj

# === WORLD-UI

# Where to find the include folder
WORLD_UI_INCLUDE_DIRECTORY = .\include\engine\world-ui

# Where to find source code
WORLD_UI_SOURCE_DIRECTORY = .\src\engine\world-ui

# Where to find the objects folder
WORLD_UI_OBJECT_DIRECTORY = $(WORLD_UI_SOURCE_DIRECTORY)\obj

# === GENERAL

# Where to find the include folder
GENERAL_INCLUDE_DIRECTORY = .\include\engine\general

# Where to find source code
GENERAL_SOURCE_DIRECTORY = .\src\engine\general

# Where to find the objects folder
GENERAL_OBJECT_DIRECTORY = $(GENERAL_SOURCE_DIRECTORY)\obj


# === INTEGRATION ===============================================================================


# Where to find the include folder
INTEGRATION_INCLUDE_DIRECTORY = .\include\integration

# Where to find source code
INTEGRATION_SOURCE_DIRECTORY = .\src\integration

# Where to find the objects folder
INTEGRATION_OBJECT_DIRECTORY = $(INTEGRATION_SOURCE_DIRECTORY)\obj


# === GAME ===============================================================================


# Where to find the include folder
GAME_INCLUDE_DIRECTORY = .\include\game

# Where to find source code
GAME_SOURCE_DIRECTORY = .\src\game

# Where to find the objects folder
GAME_OBJECT_DIRECTORY = $(GAME_SOURCE_DIRECTORY)\obj

# SDL Include Directory
SDL_INCLUDE = -IC:\TDM-GCC-32\sdl2\include\SDL2

# SDL Library Directory
SDL_LIBRARY = -LC:\TDM-GCC-32\sdl2\lib



# ==========================================================================================
# FILES
# ==========================================================================================



# === ENGINE ===============================================================================


# === EDITOR

# === WORLD

# Header files
_WORLD_DEPS = Animation.h AnimationFrame.h Animator.h BoxCollider.h CameraFollower.h CircleCollider.h Collider.h Collision.h Music.h Particle.h ParticleEmitter.h ParticleSystem.h PhysicsLayerHandler.h PhysicsSystem.h PlatformEffector.h Rigidbody.h Sound.h SpriteRenderer.h TriggerCollisionData.h WorldComponent.h WorldObject.h 

# Generate header filepaths
WORLD_DEPS = $(patsubst %,$(WORLD_INCLUDE_DIRECTORY)\\%,$(_WORLD_DEPS))

# Object files
_WORLD_OBJS = Animation.o AnimationFrame.o Animator.o BoxCollider.o CircleCollider.o Collider.o Collision.o Music.o Particle.o ParticleEmitter.o ParticleSystem.o PhysicsLayerHandler.o PhysicsSystem.o PlatformEffector.o Rigidbody.o Sound.o SpriteRenderer.o TriggerCollisionData.o WorldObject.o WorldComponent.o

# Generate object filepaths
WORLD_OBJS = $(patsubst %,$(WORLD_OBJECT_DIRECTORY)\\%,$(_WORLD_OBJS))

# === WORLD-UI

# Header files
_WORLD_UI_DEPS = Camera.h Component.h Debug.h Game.h GameScene.h InputManager.h Resources.h Sprite.h Timer.h GameObject.h Canvas.h Player.h PlayerManager.h ControllerDevice.h 

# Generate header filepaths
WORLD_UI_DEPS = $(patsubst %,$(WORLD_UI_INCLUDE_DIRECTORY)\\%,$(_WORLD_UI_DEPS))

# Object files
_WORLD_UI_OBJS = Camera.o Component.o Debug.o Game.o GameScene.o InputManager.o Resources.o Sprite.o GameObject.o Canvas.o Player.o PlayerManager.o ControllerDevice.o 

# Generate object filepaths
WORLD_UI_OBJS = $(patsubst %,$(WORLD_UI_OBJECT_DIRECTORY)\\%,$(_WORLD_UI_OBJS))

# === UI

# Header files
_UI_DEPS = UIText.h UIComponent.h UIContainer.h UIContent.h UIDimension.h UIObject.h UIImage.h UIInheritable.h UIChildrenBox.h UIBackground.h UIEvent.h UIControllerSelectable.h

# Generate header filepaths
UI_DEPS = $(patsubst %,$(UI_INCLUDE_DIRECTORY)\\%,$(_UI_DEPS))

# Object files
_UI_OBJS = UIText.o UIComponent.o UIContainer.o UIContent.o UIDimension.o UIObject.o UIImage.o UIChildrenBox.o UIBackground.o UIEvent.o UIControllerSelectable.o

# Generate object filepaths
UI_OBJS = $(patsubst %,$(UI_OBJECT_DIRECTORY)\\%,$(_UI_OBJS))

# === GENERAL

# Header files
_GENERAL_DEPS = Circle.h Color.h Event.h Helper.h Rectangle.h Shape.h Vector2.h ComponentOwner.h Parent.h MouseCursor.h

# Generate header filepaths
GENERAL_DEPS = $(patsubst %,$(GENERAL_INCLUDE_DIRECTORY)\\%,$(_GENERAL_DEPS))

# Object files
_GENERAL_OBJS = Circle.o Color.o Helper.o Rectangle.o Shape.o Vector2.o main.o ComponentOwner.o MouseCursor.o

# Generate object filepaths
GENERAL_OBJS = $(patsubst %,$(GENERAL_OBJECT_DIRECTORY)\\%,$(_GENERAL_OBJS))


# === INTEGRATION ===============================================================================


# Header files
_INTEGRATION_DEPS = ColliderDensity.h GameData.h RenderLayer.h Tag.h PhysicsLayer.h BuildConfigurations.h

# Generate header filepaths
INTEGRATION_DEPS = $(patsubst %,$(INTEGRATION_INCLUDE_DIRECTORY)\\%,$(_INTEGRATION_DEPS))

# Object files
_INTEGRATION_OBJS = InitialScene.o GameConfiguration.o PhysicsConfiguration.o

# Generate object filepaths
INTEGRATION_OBJS = $(patsubst %,$(INTEGRATION_OBJECT_DIRECTORY)\\%,$(_INTEGRATION_OBJS))


# === GAME ===============================================================================


# Header files
_GAME_DEPS = MainScene.h Movement.h PlayerInput.h Action.h CharacterStateManager.h CharacterState.h CharacterStateRecipes.h CharacterController.h Actions.h KeyboardInput.h ControllerInput.h ParticleFX.h CharacterRepelCollision.h PlatformDrop.h FallDeath.h Arena.h Heat.h Damage.h Attack.h Character.h CharacterTest.h CharacterTestAnimations.h NewAnimationTypes.h TimeScaleManager.h Parry.h GunParry.h LandingAttackEffector.h LandEffector.h ShakeEffectManager.h CharacterVFX.h CameraBehavior.h CharacterUIManager.h MenuScene.h SplashAnimation.h MainMenuInput.h CharacterUIOption.h BrawlPlayer.h ObjectRecipes.h CharacterKafta.h CharacterKiba.h CharacterAnimations.h CharacterKibaAnimations.h CharacterKaftaAnimations.h Invulnerability.h ArenaUIAnimation.h

# Generate header filepaths
GAME_DEPS = $(patsubst %,$(GAME_INCLUDE_DIRECTORY)\\%,$(_GAME_DEPS))

# Object files
_GAME_OBJS = MainScene.o Movement.o PlayerInput.o CharacterStateManager.o CharacterStateRecipes.o CharacterController.o Action.o CharacterState.o Actions.o KeyboardInput.o ControllerInput.o ParticleFX.o CharacterRepelCollision.o PlatformDrop.o FallDeath.o Arena.o Heat.o Attack.o Character.o CharacterTest.o CharacterTestAnimations.o NewAnimationTypes.o TimeScaleManager.o GunParry.o Damage.o LandingAttackEffector.o ShakeEffectManager.o CharacterVFX.o CameraBehavior.o CharacterUIManager.o SplashAnimation.o MainMenuInput.o MenuScene.o CharacterUIOption.o BrawlPlayer.o ObjectRecipes.o CharacterKafta.o CharacterKiba.o CharacterKibaAnimations.o CharacterKaftaAnimations.o CharacterAnimations.o Invulnerability.o ArenaUIAnimation.o

# Generate object filepaths
GAME_OBJS = $(patsubst %,$(GAME_OBJECT_DIRECTORY)\\%,$(_GAME_OBJS))



# ==========================================================================================
# ARGUMENTS
# ==========================================================================================



# Which libraries to include
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

# Additional compilation arguments
COMPILER_FLAGS = -std=c++17 -Wall -Wextra -pedantic

# Compilation arguments
COMPILATION_ARGS = -I $(GAME_INCLUDE_DIRECTORY) -I $(INTEGRATION_INCLUDE_DIRECTORY) -I $(GENERAL_INCLUDE_DIRECTORY) -I $(EDITOR_INCLUDE_DIRECTORY)  -I $(WORLD_INCLUDE_DIRECTORY) -I $(WORLD_UI_INCLUDE_DIRECTORY) -I $(UI_INCLUDE_DIRECTORY) $(SDL_INCLUDE) $(COMPILER_FLAGS)



# ==========================================================================================
# RULES
# ==========================================================================================



# === GAME ===============================================================================


# Define how to make .o files, and make them dependent on their .c counterparts and the h files
$(GAME_OBJECT_DIRECTORY)\\%.o: $(GAME_SOURCE_DIRECTORY)\%.cpp $(GAME_DEPS) $(INTEGRATION_DEPS) $(WORLD_DEPS) $(UI_DEPS) $(WORLD_UI_DEPS) $(GENERAL_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)


# === INTEGRATION ===============================================================================


# Define how to make .o files, and make them dependent on their .c counterparts and the h files
$(INTEGRATION_OBJECT_DIRECTORY)\\%.o: $(INTEGRATION_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(GAME_DEPS) $(WORLD_DEPS) $(UI_DEPS) $(WORLD_UI_DEPS) $(GENERAL_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)


# === ENGINE ===============================================================================


# === EDITOR

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
# $(EDITOR_OBJECT_DIRECTORY)\\%.o: $(EDITOR_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(EDITOR_DEPS)
#	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)

# === WORLD

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
$(WORLD_OBJECT_DIRECTORY)\\%.o: $(WORLD_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(WORLD_DEPS) $(WORLD_UI_DEPS) $(GENERAL_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)

# === UI

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
$(UI_OBJECT_DIRECTORY)\\%.o: $(UI_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(UI_DEPS) $(WORLD_UI_DEPS) $(GENERAL_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)
	
# === WORLD-UI

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
$(WORLD_UI_OBJECT_DIRECTORY)\\%.o: $(WORLD_UI_SOURCE_DIRECTORY)\%.cpp $(INTEGRATION_DEPS) $(WORLD_UI_DEPS) $(UI_DEPS) $(WORLD_DEPS) $(GENERAL_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)

# === GENERAL

# Define how to make .o files, and make them dependent on their .c counterparts and the h files
$(GENERAL_OBJECT_DIRECTORY)\\%.o: $(GENERAL_SOURCE_DIRECTORY)\%.cpp $(GENERAL_DEPS)
	$(CC) -g -c -o $@ $< $(COMPILATION_ARGS)

	
# Makes the game
game: $(GAME_OBJS) $(INTEGRATION_OBJS) $(WORLD_OBJS) $(UI_OBJS) $(WORLD_UI_OBJS) $(GENERAL_OBJS)
#	./src/editor/componentTable/tableScrapper.sh
	$(CC) $^ $(COMPILATION_ARGS) $(LIBS) $(SDL_LIBRARY) -g -o $@
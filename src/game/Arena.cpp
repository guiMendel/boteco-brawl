#include "Arena.h"
#include "ArenaUIAnimation.h"
#include "TimeScaleManager.h"
#include "ArenaScene.h"
#include "FallDeath.h"
#include "Debug.h"

using namespace std;

// New timescale of world when battle ends
static const float endTimeScale{0.2};

Arena::Arena(GameObject &associatedObject)
    : WorldComponent(associatedObject) {}

void Arena::Awake()
{
  InitializeArena();

  Assert(width > 0 && height > 0, "Failed to initialize dimensions of arena " + string(*this));
}

void Arena::Render()
{
  auto box = Rectangle({0, 0}, width, height);

  Debug::DrawBox(box, Color::Red());
  Debug::DrawBox(box * 0.95, Color::Red());
}

void Arena::CheckBattleOver()
{
  // Get all fall deaths
  auto characterLives = GetScene()->RequireFindComponents<FallDeath>();

  // If found one character still alive
  bool oneAlive{false};

  for (auto life : characterLives)
    if (life->GetLives() != 0)
    {
      // If already had another one alive, stop
      if (oneAlive)
        return;

      oneAlive = true;
    }

  // Slow everything down
  auto timeScaleManager = GetScene()->RequireFindComponent<TimeScaleManager>();
  auto mainParent = GetScene()->RequireWorldObject(MAIN_PARENT_OBJECT);

  timeScaleManager->AlterTimeScale(mainParent, endTimeScale, 9999);

  // Transition out
  GetScene()->RequireFindComponent<ArenaUIAnimation>()->EndGame("Fim de Jogo");
}

float Arena::GetWidth() const { return width; }
float Arena::GetHeight() const { return height; }

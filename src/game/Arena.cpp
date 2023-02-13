#include "Arena.h"
#include "ArenaUIAnimation.h"
#include "TimeScaleManager.h"
#include "MainScene.h"
#include "FallDeath.h"
#include "Debug.h"

using namespace std;

// New timescale of world when battle ends
static const float endTimeScale{0.2};

Arena::Arena(GameObject &associatedObject, float width, float height)
    : WorldComponent(associatedObject), width(width), height(height) {}

Arena::Arena(GameObject &associatedObject, std::shared_ptr<SpriteRenderer> backgroundRenderer)
    : Arena(associatedObject, backgroundRenderer->GetSprite()->GetWidth(), backgroundRenderer->GetSprite()->GetHeight())
{
  weakBackgroundRenderer = backgroundRenderer;
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

#ifndef __UI_INHERITABLE__
#define __UI_INHERITABLE__

#include <memory>
#include <functional>
#include "Color.h"
#include "Helper.h"
#include "RenderLayer.h"
#include "UIContainer.h"

// Defines an inheritable property field
#define PROPERTY(Type, name)       \
  UIInheritableProperty<Type> name \
  {                                \
    [this](UIObject *object) {     \
      return &object->style->name; \
    },                             \
        uiObject                   \
  }

// Defines a property that a UI Object can be inherit from it's parent
template <typename T>
struct UIInheritableProperty
{
  using AccessCallback = std::function<UIInheritableProperty<T> *(UIObject *object)>;

  // Raised when this property's value (or it's inherited value) changes
  Event OnChangeValue;

  UIInheritableProperty(AccessCallback accessFrom, UIObject *uiObject)
      : accessFrom(accessFrom), uiObject(uiObject) {}

  // Gets the value of the property for this UI Object
  // If it's not set locally, it will be inherited from the parent
  T Get() const { return property ? *property : Inherit(); }

  // Sets the value of this property for this UI Object
  // It will be available for inheritance by any children down the canvas object tree
  void Set(T newValue)
  {
    property = std::make_shared<T>(newValue);
    RaiseValueChanges();
  }

  // Erases a previously set value, allowing the property to be inherited
  void Unset() { property = nullptr; }

  // Whether this value will be inherited
  bool IsInheriting() const { return property == nullptr; }

private:
  // Returns the value for this property's inheritance
  T Inherit() const
  {
    Helper::Assert(uiObject->IsCanvasRoot() == false, "Canvas root has no value set for inheritable property");
    return accessFrom(uiObject->RequireParent().get())->Get();
  }

  // Raise OnChangeValue for this property and all properties inheriting from it
  void RaiseValueChanges()
  {
    OnChangeValue.Invoke();

    auto uiContainer = uiObject->GetScene()->GetUIObject<UIContainer>(uiObject->id);

    if (uiContainer == nullptr)
      return;

    for (std::shared_ptr<UIObject> child : uiContainer->GetChildren())
    {
      auto childProperty = accessFrom(child.get());

      if (childProperty->IsInheriting())
        childProperty->RaiseValueChanges();
    }
  }

  // The property itself
  // Can be null, in which case it should be inherited when read
  std::shared_ptr<T> property;

  // Given a UI Object, returns a reference to it's value for this property
  AccessCallback accessFrom;

  // The associated UI Object
  UIObject *uiObject;
};

// Defines the group of properties of a UI Object that can be inherited up the canvas object tree
struct UIInheritable
{
  UIInheritable(UIObject *uiObject) : uiObject(uiObject) {}

private:
  // The associated UI Object
  UIObject *uiObject;

public:
  // Which RenderLayer to render to
  PROPERTY(RenderLayer, renderLayer);

  // Render order
  PROPERTY(int, renderOrder);

  // The size of text
  PROPERTY(int, fontSize);

  // The font path of text
  PROPERTY(std::string, fontPath);

  // The color of text
  PROPERTY(Color, textColor);

  // The color of text's border
  PROPERTY(Color, textBorderColor);

  // The size of text's border, in real pixels
  PROPERTY(float, textBorderSize);

  // How much scaling should be applied by default to image's sizes when no specific size is set
  PROPERTY(float, imageScaling);

  // Color modulation applied to images
  PROPERTY(Color, imageColor);
};

#endif
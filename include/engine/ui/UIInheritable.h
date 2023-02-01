#ifndef __UI_INHERITABLE__
#define __UI_INHERITABLE__

#include <memory>
#include <functional>
#include "Color.h"
#include "Helper.h"
#include "RenderLayer.h"
#include "UIContainer.h"

// Defines an inheritable property field
#define PROPERTY(Type, name)                                                                                            \
  UIInheritableProperty<Type> name                                                                                      \
  {                                                                                                                     \
    [this]() {                                                                                                          \
      Helper::Assert(uiObject.IsCanvasRoot() == false, "Canvas root has no value set for inheritable property " #name); \
      return uiObject.GetParent()->style->name.Get();                                                                   \
    }                                                                                                                   \
  }

// Defines a property that a UI Object can be inherit from it's parent
template <typename T>
struct UIInheritableProperty
{
  UIInheritableProperty(std::function<T()> inherit)
      : inherit(inherit) {}

  // Gets the value of the property for this UI Object
  // If it's not set locally, it will be inherited from the parent
  T Get() const { return property ? *property : inherit(); }

  // Sets the value of this property for this UI Object
  // It will be available for inheritance by any children down the canvas object tree
  void Set(T newValue) { property = std::make_shared<T>(newValue); }

  // Erases a previously set value, allowing the property to be inherited
  void Unset() { property = nullptr; }

private:
  // The property itself
  // Can be null, in which case it should be inherited when read
  std::shared_ptr<T> property;

  // Function that returns the value for this property's inheritance
  std::function<T()> inherit;
};

// Defines the group of properties of a UI Object that can be inherited up the canvas object tree
struct UIInheritable
{
  UIInheritable(const UIObject &uiObject) : uiObject(uiObject) {}

private:
  // The associated UI Object
  const UIObject &uiObject;

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
#pragma once

#include <algorithm> // for max, min
#include <chrono>
#include <cstdio>
#include <fstream>
#include <ftxui/component/component_base.hpp> // for Component, ComponentBase
#include <ftxui/component/event.hpp> // for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include <ftxui/screen/color.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <random>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <utility> // for move

#include "ftxui/component/component.hpp" // for Make
#include "ftxui/component/mouse.hpp" // for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include "ftxui/dom/deprecated.hpp" // for text
#include "ftxui/dom/elements.hpp" // for operator|, Element, size, vbox, EQUAL, HEIGHT, dbox, reflect, focus, inverted, nothing, select, vscroll_indicator, yflex, yframe
#include "ftxui/dom/node.hpp"     // for Node
#include "ftxui/dom/requirement.hpp" // for Requirement
#include "ftxui/screen/box.hpp"      // for Box

namespace ftxui {
class ScrollerBase : public ComponentBase {
public:
  ScrollerBase(Component child) { Add(child); };
  bool Terminal = false;
  int Selected() { return (selected_); };

  int Get_Size() { return (size_); }

  void Set_Selected(int i) {
    if (i < size_) {
      selected_ = i;
    }
  }

private:
  int last_size_ = 0;
  int selected_ = 0;
  int size_ = 0;
  Box box_;
  Element OnRender() final;
  bool OnEvent(Event event);
};
} // namespace ftxui

using namespace ftxui;

Component Scroller(Component child);
Component Wrap(std::string name, Component component);
Color ColourSelector(std::string ColourSTR);
std::vector<std::string> GenerateTextList(const std::string Data);
Components ParseText(std::string Data);

Color DEFAULT_COLOUR = Color::Default;
Decorator DEFAULT_DECORATION = color(Color::Default);

// Forward declare ColorState
struct ColorState;

// TEXT DISPLAY IMPLEMENTATION

class TextDisplayBase : public ScrollerBase {
public:
  Component DisplayParent;

  using ScrollerBase::Get_Size;
  using ScrollerBase::Selected;
  using ScrollerBase::Set_Selected;
  using ScrollerBase::Terminal;

  static std::vector<TextDisplayBase *> Displays;

  std::string *Display = nullptr;
  std::vector<std::string> DisplayStrings;

  bool Focusable() const override { return true; }

  TextDisplayBase(std::string &display, bool IsTerminal)
      : ScrollerBase(Container::Vertical({})), Display(&display) {
    DisplayStrings = GenerateTextList(*Display);
    Terminal = IsTerminal;
    DisplayParent = ChildAt(0);
    Displays.push_back(this);
    Update();
  }

  static void UpdateAll() {
    auto Temp = Displays;
    for (auto *display : Temp) {
      display->Update();
    }
  }

  std::string SelectedEntry() {
    if (Selected() < DisplayStrings.size()) {
      return DisplayStrings[Selected()];
    }
    return "";
  }

  void Update() {
    DisplayParent->DetachAllChildren();
    auto NewEntries = ParseText(*Display);
    DisplayStrings = GenerateTextList(*Display);
    for (auto &entry : NewEntries) {
      DisplayParent->Add(entry);
    }
  }
};

Component TextDisplay(std::string &display, bool IsTerminal);

using namespace ftxui;

Element ScrollerBase::OnRender() {
  auto focused = Focused() ? focus : ftxui::select;
  auto style = Focused() ? inverted : nothing;

  Element background = ComponentBase::Render();
  background->ComputeRequirement();
  size_ = background->requirement().min_y;
  if (Terminal) {
    if (selected_ == last_size_ - 1) {
      selected_ = size_ - 1;
    }
    last_size_ = size_;
  }
  return dbox({
             std::move(background),
             vbox({
                 text(L"") | size(HEIGHT, EQUAL, selected_),
                 text(L"") | style | focused,
             }),
         }) |
         vscroll_indicator | yframe | reflect(box_);
}

bool ScrollerBase::OnEvent(Event event) {
  if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y))
    TakeFocus();

  int selected_old = selected_;
  if (event == Event::ArrowUp || event == Event::Character('k') ||
      (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
    selected_--;
  }
  if ((event == Event::ArrowDown || event == Event::Character('j') ||
       (event.is_mouse() && event.mouse().button == Mouse::WheelDown))) {
    selected_++;
  }
  if (event == Event::PageDown)
    selected_ += box_.y_max - box_.y_min;
  if (event == Event::PageUp)
    selected_ -= box_.y_max - box_.y_min;
  if (event == Event::Home)
    selected_ = 0;
  if (event == Event::End)
    selected_ = size_;

  selected_ = std::max(0, std::min(size_ - 1, selected_));
  return selected_old != selected_;
}

Component Scroller(Component child) {
  return Make<ScrollerBase>(std::move(child));
}

// Copyright 2021 Arthur Sonzogni. All rights reserved. (THIS FUNCTION ONLY)
Component Wrap(std::string name, Component component) {
  return Renderer(component, [name, component] {
    return window(text(name) | color(Color::Cyan) | bold,
                  {
                      component->Render() | xflex,
                  }) |
           xflex;
  });
}

Color ColourSelector(std::string ColourSTR) {
  if (ColourSTR == "YELLOW") {
    return (Color::Yellow);
  } else if (ColourSTR == "BLUE") {
    return (Color::Blue);
  } else if (ColourSTR == "RED") {
    return (Color::Red);
  } else if (ColourSTR == "CYAN") {
    return (Color::Cyan);
  }
  return (DEFAULT_COLOUR);
}

// ColorState struct definition
struct ColorState {
  Color fg = Color::Default;
  Color bg = Color::Default;

  Decorator toDecorator() const { return color(fg) | bgcolor(bg); }

  void reset() {
    fg = Color::Default;
    bg = Color::Default;
  }

  void applyANSI(const std::string &params) {
    if (params.empty()) {
      reset();
      return;
    }

    std::stringstream stream(params);
    std::string token;

    while (std::getline(stream, token, ';')) {
      int code = 0;
      try {
        code = std::stoi(token);
      } catch (...) {
        continue;
      }

      switch (code) {
      case 0:
        reset();
        break;
      case 39:
        fg = Color::Default;
        break;
      case 49:
        bg = Color::Default;
        break;

      // Foreground colors
      case 30:
        fg = Color::Black;
        break;
      case 31:
        fg = Color::Red;
        break;
      case 32:
        fg = Color::Green;
        break;
      case 33:
        fg = Color::Yellow;
        break;
      case 34:
        fg = Color::Blue;
        break;
      case 35:
        fg = Color::Magenta;
        break;
      case 36:
        fg = Color::Cyan;
        break;
      case 37:
        fg = Color::White;
        break;
      case 90:
        fg = Color::GrayLight;
        break;
      case 91:
        fg = Color::RedLight;
        break;
      case 92:
        fg = Color::GreenLight;
        break;
      case 93:
        fg = Color::YellowLight;
        break;
      case 94:
        fg = Color::BlueLight;
        break;
      case 95:
        fg = Color::MagentaLight;
        break;
      case 96:
        fg = Color::CyanLight;
        break;
      case 97:
        fg = Color::White;
        break;

      // Background colors
      case 40:
        bg = Color::Black;
        break;
      case 41:
        bg = Color::Red;
        break;
      case 42:
        bg = Color::Green;
        break;
      case 43:
        bg = Color::Yellow;
        break;
      case 44:
        bg = Color::Blue;
        break;
      case 45:
        bg = Color::Magenta;
        break;
      case 46:
        bg = Color::Cyan;
        break;
      case 47:
        bg = Color::White;
        break;
      case 100:
        bg = Color::GrayLight;
        break;
      case 101:
        bg = Color::RedLight;
        break;
      case 102:
        bg = Color::GreenLight;
        break;
      case 103:
        bg = Color::YellowLight;
        break;
      case 104:
        bg = Color::BlueLight;
        break;
      case 105:
        bg = Color::MagentaLight;
        break;
      case 106:
        bg = Color::CyanLight;
        break;
      case 107:
        bg = Color::White;
        break;

      default:
        break;
      }
    }
  }
};

std::vector<std::string> GenerateTextList(const std::string Data) {
  std::vector<std::string> ReturnValues;

  // Strip ANSI codes for the line list
  std::string stripped;
  size_t idx = 0;
  while (idx < Data.length()) {
    if (Data[idx] == '\033' && idx + 1 < Data.length() &&
        Data[idx + 1] == '[') {
      size_t k = idx + 2;
      while (k < Data.length() && !isalpha((unsigned char)Data[k])) {
        k++;
      }
      if (k < Data.length()) {
        idx = k + 1;
        continue;
      }
    }
    if (Data[idx] == '\r') {
      idx++;
      continue;
    }
    stripped += Data[idx];
    idx++;
  }

  int i = 0;
  int Prev = 0;
  while (i < (int)stripped.length()) {
    if (stripped[i] == '\n') {
      ReturnValues.push_back(stripped.substr(Prev, i - Prev));
      Prev = i + 1;
    }
    i++;
  }
  // Don't forget the last line if there's no trailing newline
  if (Prev < (int)stripped.length()) {
    ReturnValues.push_back(stripped.substr(Prev));
  }
  return ReturnValues;
}

Components ParseText(std::string Data) {
  Components ReturnValues;
  Elements CurrentLineElems;
  ColorState CurrentColor;

  size_t i = 0;
  size_t Prev = 0;
  std::string CurrentLineText = "";

  auto FlushCurrentText = [&]() {
    if (!CurrentLineText.empty()) {
      CurrentLineElems.push_back(text(CurrentLineText) |
                                 CurrentColor.toDecorator());
      CurrentLineText.clear();
    }
  };

  auto FlushCurrentLine = [&]() {
    FlushCurrentText();
    auto LineElems = CurrentLineElems;
    if (LineElems.empty()) {
      ReturnValues.push_back(Renderer([](bool) { return text(""); }));
    } else {
      ReturnValues.push_back(
          Renderer([LineElems](bool) { return hbox(LineElems); }));
    }
    CurrentLineElems.clear();
  };

  while (i < Data.length()) {
    // Handle carriage return - reset to beginning of line
    if (Data[i] == '\r') {
      FlushCurrentText();
      CurrentLineElems.clear();
      i++;
      Prev = i;
    }
    // Handle ANSI escape sequences
    else if (Data[i] == '\033' && i + 1 < Data.length() && Data[i + 1] == '[') {
      // Add text before escape sequence
      if (i > Prev) {
        CurrentLineText += Data.substr(Prev, i - Prev);
      }
      FlushCurrentText();

      // Find the end of the escape sequence
      size_t k = i + 2;
      while (k < Data.length() && !isalpha((unsigned char)Data[k])) {
        k++;
      }

      if (k >= Data.length()) {
        break; // Incomplete escape sequence
      }

      char finalChar = Data[k];
      std::string params = "";
      if (k > i + 2) {
        params = Data.substr(i + 2, k - (i + 2));
      }

      // Handle erase line (K)
      if (finalChar == 'K') {
        if (params.empty() || params == "0") {
          // Erase to end - nothing to do
        } else if (params == "1") {
          CurrentLineElems.clear();
        } else if (params == "2") {
          CurrentLineElems.clear();
        }
      }
      // Handle SGR color codes (m)
      else if (finalChar == 'm') {
        CurrentColor.applyANSI(params);
      }

      i = k + 1;
      Prev = i;
    }
    // Handle newline
    else if (Data[i] == '\n') {
      // Add remaining text on this line
      if (i > Prev) {
        CurrentLineText += Data.substr(Prev, i - Prev);
      }

      FlushCurrentLine();

      i++;
      Prev = i;
    } else {
      i++;
    }
  }

  // Handle remaining text
  if (i > Prev) {
    CurrentLineText += Data.substr(Prev, i - Prev);
  }

  FlushCurrentText();
  if (!CurrentLineElems.empty()) {
    auto LineElems = CurrentLineElems;
    ReturnValues.push_back(
        Renderer([LineElems](bool) { return hbox(LineElems); }));
  }

  return ReturnValues;
}

std::vector<TextDisplayBase *> TextDisplayBase::Displays{};

Component TextDisplay(std::string &display, bool IsTerminal) {
  return (Make<TextDisplayBase>(display, IsTerminal));
}

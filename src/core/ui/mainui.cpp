#include "mainui.hpp"

#include "core/globals.hpp"
#include "services/state.hpp"
#include "core/engines/engine_manager.hpp"

namespace otto::ui {
  static constexpr const char * initial_engine = "TapeDeck";

  // Local vars
  namespace {

    struct EmptyScreen : Screen {

      void draw(vg::Canvas& ctx)
      {}

    } empty_screen;

    std::string selected_engine_name = "";
    Screen* cur_screen = &empty_screen;

    PressedKeys keys;
  }

  bool is_pressed(Key k) noexcept
  {
    return keys[static_cast<unsigned>(k)];
  }

  void selectEngine(std::string engine_name) {
    selected_engine_name = engine_name;

    auto& engineManager = engines::EngineManager::get();
    auto engine = engineManager.getEngineByName(engine_name);
    if (!engine) {
      engine = engineManager.getEngineByName(initial_engine);
    }

    display(engine->screen());
  }

  void init() {
    auto load = [](const nlohmann::json &j) {
      if (j.is_object()) {
        selected_engine_name = j["SelectedEngine"];
      }

      selectEngine(selected_engine_name);
    };

    auto save = []() {
      return nlohmann::json({
        {"SelectedEngine", selected_engine_name}
      });
    };

    services::state::attach("UI", load, save);
  }

  void display(Screen& screen)
  {
    cur_screen->on_hide();
    cur_screen = &screen;
    cur_screen->on_show();
  }

  namespace impl {
    static void toggle_play_state() {
      auto& tapedeck = engines::EngineManager::get().tapedeck;
      if (tapedeck.state.playing()) {
        tapedeck.state.stop();
      } else {
        tapedeck.state.play();
      }
    }

    static bool global_keypress(Key key)
    {
      switch (key) {
      case Key::quit: global::exit(global::ErrorCode::user_exit); break;
      case Key::tape: selectEngine("TapeDeck"); break;
      case Key::mixer: selectEngine("Mixer"); break;
      case Key::synth: selectEngine("Synth"); break;
      case Key::drums: selectEngine("Drums"); break;
      case Key::metronome:
        selectEngine("Metronome");
        break;
      case ui::Key::play:
        toggle_play_state();
        break;
      default: return false;
      }
      return true;
    }

    void draw_frame(vg::Canvas& ctx)
    {
      ctx.lineWidth(2);
      ctx.lineCap(vg::Canvas::LineCap::ROUND);
      ctx.lineJoin(vg::Canvas::Canvas::LineJoin::ROUND);
      cur_screen->draw(ctx);
    }

    bool keypress(Key key)
    {
      switch (key) {
      case Key::red_up: cur_screen->rotary({Rotary::Red, 1}); break;
      case Key::red_down: cur_screen->rotary({Rotary::Red, -1}); break;
      case Key::blue_up: cur_screen->rotary({Rotary::Blue, 1}); break;
      case Key::blue_down: cur_screen->rotary({Rotary::Blue, -1}); break;
      case Key::white_up: cur_screen->rotary({Rotary::White, 1}); break;
      case Key::white_down: cur_screen->rotary({Rotary::White, -1}); break;
      case Key::green_up: cur_screen->rotary({Rotary::Green, 1}); break;
      case Key::green_down: cur_screen->rotary({Rotary::Green, -1}); break;
      default:
        keys[static_cast<unsigned>(key)] = true;
        if (global_keypress(key)) return true;
        return cur_screen->keypress(key);
      }
      return true;
    }

    bool keyrelease(Key key)
    {
      keys[static_cast<unsigned>(key)] = false;
      return cur_screen->keyrelease(key);
    }
  } // namespace impl
} // namespace otto::ui

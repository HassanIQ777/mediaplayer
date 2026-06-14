// declarations.hpp

#ifndef DECLARATIONS
#define DECLARATIONS

#include <atomic>
#include <csignal>
#include <fstream>
#include <map>
#include <numeric> // std::iota
#include <string>
#include <thread>

#include "json.hpp"
#include "libutils/src/CLIParser.hpp"
#include "libutils/src/File.hpp"
#include "libutils/src/Log.hpp"
#include "libutils/src/color.hpp"
#include "libutils/src/funcs.hpp"

const std::string BORDER_CHAR = "─";
using funcs::print;
using json = nlohmann::json;

struct Paths // files and dirs
{
  std::string home_dir, mediaplayer_dir, exception_list, history, latest_media,
      settings;
};

enum class UI_State { MAIN_MENU, SETTINGS, LATEST_MEDIA };

struct Settings {
  size_t columns = 15;

  // The canonical "what a fresh Settings looks like"
  static Settings defaults() { return Settings{}; } // just uses member defaults

  static Settings fromJson(const json &j) {
    Settings s = defaults(); // start from defaults, not garbage
    // .value() = "give me this key, or this fallback" — never throws on missing
    // keys
    s.columns = j.value("columns", s.columns);
    return s;
  }

  json toJson() const { return json{{"columns", columns}}; }

  void save(const std::string &filepath) {
    // Make sure parent dirs exist first (no directory = no file = sad ofstream)
    std::filesystem::create_directories(
        std::filesystem::path(filepath).parent_path());

    std::ofstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("save(): couldn't open file: " + filepath);

    file << toJson().dump(4); // 4-space indent, because we're not animals

    if (!file.good())
      throw std::runtime_error("save(): write failed for: " + filepath);
  }

  // Returns true if it had to create the file (useful for "first run" logic)
  bool loadOrCreate(const std::string &filepath) {
    namespace fs = std::filesystem;

    if (!fs::exists(filepath)) {
      *this = defaults();
      save(filepath); // write the defaults so the file exists next time
      return true;    // "hey, I was born just now"
    }

    std::ifstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("load(): couldn't open file: " + filepath);

    json data;
    try {
      file >> data;
    } catch (const json::parse_error &e) {
      // File exists but is cursed — fall back to defaults, overwrite the crime
      // scene
      *this = defaults();
      save(filepath);
      return true; // "I was reborn"
    }

    *this =
        fromJson(data); // fromJson already merges with defaults via .value()
    return false;       // "I remembered who I was"
  }
};

struct Globals {
  Paths paths;
  UI_State ui_state;
  std::string VERSION;
  std::string delimiter;
  Settings settings;
};

inline void printLogo() {
  std::string time_hours_mins = funcs::currentTime().substr(11, 5);

  print(color::A_BOLD);
  funcs::printLeftMiddleRight("", "Mediaplayer Settings", time_hours_mins);
  print(color::A_RESET);

  print(color::A_RESET, "\n");

  std::cout.flush();
}
void settingsMenu(Globals &globals);
void parseSettingsMenuOption(Globals &globals, const std::string &option_str);

void setColumns(Globals &globals);

#endif
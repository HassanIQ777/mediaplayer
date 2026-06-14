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
  size_t columns;

  static Settings fromJson(const json &j) { return Settings{j["columns"]}; }
  json toJson() const { return json{{"columns", columns}}; }

  void save(const std::string &filepath) {
    std::ofstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("save(): couldn't open file: " + filepath);

    file << toJson();

    if (!file.good())
      throw std::runtime_error("save(): write failed for: " + filepath);
  }

  void load(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("load(): couldn't open file: " + filepath);

    json data;
    try {
      file >> data;
    } catch (const json::parse_error &e) {
      throw std::runtime_error("load(): bad JSON in " + filepath + ": " +
                               e.what());
    }

    (*this) = fromJson(data);
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

  print(color::_BOLD);
  funcs::printLeftMiddleRight("", "Mediaplayer Settings", time_hours_mins);
  print(color::_RESET);

  print(color::_RESET, "\n");

  std::cout.flush();
}

void loadSettings(Globals &globals);
void saveSettings(Globals &globals);

void settingsMenu(Globals &globals);
void parseSettingsMenuOption(Globals &globals, const std::string &option_str);

void setColumns(Globals &globals);

#endif
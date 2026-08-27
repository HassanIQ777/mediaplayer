// declarations.hpp

#ifndef DECLARATIONS
#define DECLARATIONS

#include "json.hpp"
#include "libutils/src/color.hpp"
#include "libutils/src/funcs.hpp"
#include <fstream>
#include <string>

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
  bool is_audio_only = false;

  static Settings defaults() { return Settings{}; }

  static Settings fromJson(const json &j) {
    Settings s = defaults(); // start from defaults
    s.columns = j.value("columns", s.columns);
    s.is_audio_only = j.value("is_audio_only", s.is_audio_only);
    return s;
  }

  json toJson() const {
    return json{{"columns", columns}, 
                {"is_audio_only", is_audio_only}};
  }

  void save(const std::string &filepath) {
    // Make sure parent dirs exist first
    std::filesystem::create_directories(
        std::filesystem::path(filepath).parent_path());

    std::ofstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("save(): couldn't open file: " + filepath);

    file << toJson().dump(4);

    if (!file.good())
      throw std::runtime_error("save(): write failed for: " + filepath);
  }

  // Returns true if it had to create the file (useful for "first run" logic)
  bool loadOrCreate(const std::string &filepath) {
    namespace fs = std::filesystem;

    if (!fs::exists(filepath)) {
      *this = defaults();
      save(filepath);
      return true;
    }

    std::ifstream file(filepath);
    if (!file.is_open())
      throw std::runtime_error("load(): couldn't open file: " + filepath);

    json data;
    try {
      file >> data; // parsing happens here
      *this = fromJson(data);
    } catch (const json::exception &e) {

      *this = defaults();
      save(filepath);
      return true;
    }

    return false;
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
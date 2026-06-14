#include "declarations.hpp"

// Helper to keep values in bounds without underflow
template <typename T> T clamp_idx(T val, T min, T max) {
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

inline void settingsMenu(Globals &globals) {
  printLogo();

  print("\n", color::TXT_GREEN, color::A_BOLD, "Settings Menu", color::A_RESET,
        "\n\n");
  print(color::TXT_YELLOW, "1) ", color::TXT_CYAN,
        "Set columns = ", globals.settings.columns, color::A_RESET, '\n');

  print("\n");

  print(color::TXT_YELLOW, ",) ", color::TXT_CYAN, "Go Back", color::A_RESET,
        '\n');

  print(color::TXT_BLUE, "\nSelect option:", color::A_RESET, " _");

  std::string option = funcs::getKeyPress();
  parseSettingsMenuOption(globals, option);
}

inline void parseSettingsMenuOption(Globals &globals,
                                    const std::string &option_str) {

  if (option_str == "9")
    setColumns(globals);

  if (option_str == ",")
    globals.ui_state = UI_State::MAIN_MENU;
}

inline void setColumns(Globals &globals) {
  globals.settings.loadOrCreate(globals.paths.settings);

  print("\n");

  print("\nNew max column number:\n> ", color::A_ITALIC);
  std::string column;
  std::cin >> column;
  std::cin.ignore();
  print(color::A_RESET);

  size_t amount;
  try {
    amount = std::stoul(column);
  } catch (...) {
    Log::warn("Failed operation.");
    funcs::getKeyPress();
    return;
  }

  if (amount == 0) {
    Log::info("Canceled operation.");
    funcs::getKeyPress();
    return;
  }

  globals.settings.columns = amount;

  globals.settings.save(globals.paths.settings);
}

inline void addToLatestMedia(Globals &globals, const std::string &file) {
  std::map<std::string, size_t> media_plays; // media : times played
  std::vector<std::string> latest_media_content =
      File::readfile(globals.paths.latest_media);

  for (const std::string &line : latest_media_content) {
    std::vector<std::string> pair =
        funcs::split(line, 0x1F); // media : times its been played
    media_plays[pair[0]] = std::stoul(pair[1]);
  }
  media_plays[file]++;

  // copy the map :3
  std::vector<std::pair<std::string, size_t>> items(media_plays.begin(),
                                                    media_plays.end());

  std::sort(items.begin(), items.end(),
            [](auto &a, auto &b) { return a.second > b.second; });

  std::vector<std::string> result;
  result.reserve(media_plays.size()); // optional but nice

  for (const auto &pair : items) {
    const std::string &key = pair.first;
    size_t value = pair.second;
    if (File::isfile(key)) {
      result.push_back(key + globals.delimiter + std::to_string(value));
    }
  }

  File::writefile(globals.paths.latest_media, result);
}

inline void assignPaths(Globals &globals) {
  globals.paths.mediaplayer_dir =
      fs::path(globals.paths.home_dir) / "mediaplayer";

  globals.paths.exception_list =
      fs::path(globals.paths.mediaplayer_dir) / "exception-list";
  globals.paths.history = fs::path(globals.paths.mediaplayer_dir) / "history";
  globals.paths.latest_media =
      fs::path(globals.paths.mediaplayer_dir) / "latest-media";
  globals.paths.settings =
      fs::path(globals.paths.mediaplayer_dir) / "settings.json";
}

inline void createFiles(const Globals &globals) {
  if (!File::isdirectory(globals.paths.mediaplayer_dir)) {
    std::string path = globals.paths.mediaplayer_dir;
    File::createdir(path);

    Log::debug("Successfully created '" + path + "'");
  }

  if (!File::isfile(globals.paths.exception_list)) {
    std::string fp = globals.paths.exception_list;
    File::createfile(fp);
    Log::debug("Successfully created '" + fp + "'");
  }

  if (!File::isfile(globals.paths.history)) {
    std::string fp = globals.paths.history;
    File::createfile(fp);
    Log::debug("Successfully created '" + fp + "'");
  }

  if (!File::isfile(globals.paths.latest_media)) {
    std::string fp = globals.paths.latest_media;
    File::createfile(fp);
    Log::debug("Successfully created '" + fp + "'");
  }
}

inline void parseArgs(CLIParser &parser, Globals &globals) {
  if (parser.hasFlag("-H")) {
    if (parser.getValue("-H") == "") {
      Log::error("No home directory provided.", true);
    }
  } else if (parser.hasFlag("-v")) {
    std::cout << "MediaPlayer version " << globals.VERSION << std::endl;
    exit(EXIT_SUCCESS);
  } else {
    std::cerr << "Usage:" << std::endl
              << "  -H	[PATH TO HOME FOLDER]\n"
              << "  -v    to show version number" << std::endl
              << std::endl;
    exit(EXIT_SUCCESS);
  }

  globals.paths.home_dir = parser.getValue("-H");
}

inline bool isMediaFile(const std::string &filename) {
  static const std::vector<std::string> VIDEO_EXTENSIONS = {
      ".mp4", ".mkv", ".mov", ".avi", ".wmv", ".flv", ".webm", ".mpeg"};
  static const std::vector<std::string> AUDIO_EXTENSIONS = {
      ".mp3", ".wav", ".flac", ".aac", ".ogg", ".m4a", ".wma", ".aiff"};

  std::string ext = File::getExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 ::tolower); // Convert to lowercase
  return std::find(VIDEO_EXTENSIONS.begin(), VIDEO_EXTENSIONS.end(), ext) !=
             VIDEO_EXTENSIONS.end() ||
         std::find(AUDIO_EXTENSIONS.begin(), AUDIO_EXTENSIONS.end(), ext) !=
             AUDIO_EXTENSIONS.end();
}

inline bool isVideoFile(const std::string &filename) {
  static const std::vector<std::string> VIDEO_EXTENSIONS = {
      ".mp4", ".mkv", ".mov", ".avi", ".wmv", ".flv", ".webm", ".mpeg"};

  std::string ext = File::getExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 ::tolower); // Convert to lowercase
  return std::find(VIDEO_EXTENSIONS.begin(), VIDEO_EXTENSIONS.end(), ext) !=
         VIDEO_EXTENSIONS.end();
}

inline std::string trimStr(std::string text, size_t i_width) {
  size_t WIDTH = funcs::getTerminalWidth(); // removed -5
  if (text.length() + i_width < WIDTH)
    return text;
  // if (text.length() + i_width  + 1 == WIDTH) return text.substr(0, WIDTH - 4
  // - i_width) + "··";
  return text.substr(0, WIDTH - 3 - i_width) + "··";
}

inline void alternativeTerminal() {
  std::cout << "\033[?1049h"; // Switch to the alternate screen buffer
}

inline void restoreTerminal() {
  std::cout << "\033[?1049l"; // Switch back to the normal screen buffer
}

inline void SIGINT_handle(int) {
  funcs::printTimed("\nReceived interruption signal. ABORTING\n", 10, 500);
  restoreTerminal();
  exit(0);
}

inline void loadingBar(std::atomic<bool> &isLoading) {
  size_t i = 0;
  const std::vector<std::string> BAR = {"⠋", "⠙", "⠹", "⠼", "⠴",
                                        "⠦", "⠧", "⠇", "⠏"};
  while (isLoading.load(
      std::memory_order_acquire)) { // play animation while fetching files:
    std::cout << "\r" << color::TXT_GREEN << color::A_BOLD
              << BAR[i % BAR.size()] << color::A_RESET << " Fetching files ";
    std::cout.flush();
    funcs::msleep(100);
    i++;
  }
}

inline void termuxSendToast(std::string message) {
  std::string command =
      "termux-toast -s -g top -b \"#33FFFFFF\" -c yellow \"" + message + "\"";
  system(command.c_str());
}

inline void termuxShareFile(std::string filename) {
  std::string command = "termux-share -a send \"" + filename + "\"";
  system(command.c_str());
}
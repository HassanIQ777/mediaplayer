/* @MediaPlayer by HassanIQ777 - Fixed Version */

#include "helpers.hpp"
#include "libutils/src/funcs.hpp"
#include <algorithm>
#include <numeric>

using namespace color;

void printfiles(const std::vector<size_t> &indices,
                const std::vector<std::string> &files, size_t from, size_t to,
                size_t selected);

int main(int argc, char *argv[]) {
  Globals globals;
  globals.ui_state = UI_State::MAIN_MENU;
  globals.VERSION = "2.1-fixed";
  globals.delimiter = std::string(1, 0x1F);
  CLIParser parser(argc, argv);

  std::signal(SIGINT, SIGINT_handle);

  bool isMobileDevice = (getenv("ANDROID_DATA") != nullptr);
  bool launch_in_mpv = true;

  parseArgs(parser, globals);
  assignPaths(globals);
  createFiles(globals);
  globals.settings.loadOrCreate(globals.paths.settings);

  const std::vector<std::string> exception_list =
      File::readfile(globals.paths.exception_list);

  alternativeTerminal();

  std::atomic<bool> is_loading{true};
  std::thread loadingBarThread(loadingBar, std::ref(is_loading));

  const std::string home_folder = parser.getValue("-H");
  std::vector<std::string> full_paths;

  try {
    std::vector<std::string> contents =
        exception_list.empty()
            ? File::listfiles_recursive(home_folder)
            : File::listfiles_recursive(home_folder, exception_list);

    for (const auto &content : contents) {
      if (File::isfile(content) && isMediaFile(content))
        full_paths.push_back(content);
    }
  } catch (...) {
    is_loading = false;
    if (loadingBarThread.joinable())
      loadingBarThread.join();
    std::cerr << "\rError accessing: " << home_folder << std::endl;
    return -1;
  }

  is_loading = false;
  if (loadingBarThread.joinable())
    loadingBarThread.join();

  if (full_paths.empty()) {
    std::cerr << "No media found in '" << home_folder << "'" << std::endl;
    restoreTerminal();
    return -1;
  }

  full_paths = File::sortChronological(full_paths);

  // Helper lambda to refresh file names based on full_paths
  auto refreshFiles = [&](std::vector<std::string> &f_names,
                          const std::vector<std::string> &paths) {
    f_names.clear();
    for (const auto &p : paths)
      f_names.push_back(File::getFileName(p));
  };

  std::vector<std::string> files;
  refreshFiles(files, full_paths);

  std::vector<size_t> show_files_indices(files.size());
  std::iota(show_files_indices.begin(), show_files_indices.end(), 0);

  size_t from = 0;
  size_t to =
      std::min((size_t)globals.settings.columns, show_files_indices.size());
  size_t selected = 0;
  bool running = true;
  bool is_alpha_sort = false;

  while (running) {
    funcs::clearTerminal();

    if (globals.ui_state == UI_State::MAIN_MENU) {
      printfiles(show_files_indices, files, from, to, selected);
      std::cout << " [h] Help | [s] Search | [q] Quit";

      std::string key = funcs::getKeyPress();

      // NAVIGATION: Up
      if (key == "\033[A" || key == "k") {
        if (selected > 0) {
          selected--;
          if (selected < from) {
            from--;
            to--;
          }
        } else {
          selected = show_files_indices.size() - 1;
          to = show_files_indices.size();
          from = (to > (size_t)globals.settings.columns)
                     ? to - globals.settings.columns
                     : 0;
        }
      }
      // NAVIGATION: Down
      else if (key == "\033[B" || key == "m") {
        if (selected < show_files_indices.size() - 1) {
          selected++;
          if (selected >= to) {
            from++;
            to++;
          }
        } else {
          selected = 0;
          from = 0;
          to = std::min((size_t)globals.settings.columns,
                        show_files_indices.size());
        }
      }
      // NAVIGATION: Page Up (j)
      else if (key == "\033[D" || key == "j") {
        size_t step = (size_t)globals.settings.columns;
        selected = (selected > step) ? selected - step : 0;
        from = (from > step) ? from - step : 0;
        to = from + std::min(step, show_files_indices.size());
      }
      // NAVIGATION: Page Down (l)
      else if (key == "\033[C" || key == "l") {
        size_t step = (size_t)globals.settings.columns;
        selected = std::min(selected + step, show_files_indices.size() - 1);
        from = std::min(from + step, show_files_indices.size() - step);
        if (show_files_indices.size() < step)
          from = 0;
        to = from + step;
      } else if (key == "q" || key == "Q")
        running = false;

      // ACTION: Selection
      else if (key == "\n" || key == " " || key == "/") {
        if (show_files_indices.empty())
          continue;
        const std::string fp = full_paths[show_files_indices[selected]];

        if (File::isfile(fp)) {
          addToLatestMedia(globals, fp);
          File::insertline(globals.paths.history,
                           fp + globals.delimiter + funcs::currentTime(), 0);

          if (isMobileDevice && isVideoFile(fp) && launch_in_mpv) {
            const std::string command =
                "am start -a android.intent.action.VIEW -d \"file://" + fp +
                "\" -n is.xyz.mpv/.MPVActivity";
            system(command.c_str());
          } else {
            funcs::clearTerminal();
            print("Launching:\n");
            fs::path path = fp;
            print(path.remove_filename().string(), "\n");
            print("└── ", File::getFileName(fp), "\n\n");

            const std::string command = "mpv \"" + fp + "\"";
            system(command.c_str());

            // after finishing:
            const std::string msg = "Finished Playing " + File::getFileName(fp);
            termuxSendToast(msg);
          }
        }
      }

      else if (key == "h" || key == "H") {
        print("\n");
        print("Use arrow keys to move or skip pages\n");
        if (isMobileDevice) {
          print("[S] Share media\n");
        }
        print("[r] Reset\n");
        print("[R] Restart (refetches files)\n");
        print("[f] Change sorting mode. Current Mode: ");
        print((is_alpha_sort ? "Alphabetical" : "Chronological"), "\n");
        print("[,] Settings Menu\n");
        print("[,] Latest Media\n");
        funcs::getKeyPress();
      }

      // ACTION: Search
      else if (key == "s" || (key == "S" && !isMobileDevice)) {
        print("\nSearch: ", color::A_ITALIC, color::A_BOLD, color::TXT_YELLOW);
        std::string search_string;
        std::getline(std::cin, search_string);
        print(color::A_RESET);

        search_string = funcs::lowercase(search_string);

        show_files_indices.clear();
        for (size_t i = 0; i < files.size(); i++) {
          if (funcs::hasSequence(funcs::lowercase(full_paths[i]),
                                 search_string)) {
            show_files_indices.push_back(i);
          }
        }
        selected = 0;
        from = 0;
        to = std::min((size_t)globals.settings.columns,
                      show_files_indices.size());
      }

      else if (key == "S") {
        const std::string fp = full_paths[show_files_indices[selected]];
        funcs::printTimed("\nSharing " + fp + "\n", 5);
        termuxShareFile(fp);
      }

      // Reset
      else if (key == "r") {
        show_files_indices.resize(files.size());
        std::iota(show_files_indices.begin(), show_files_indices.end(), 0);
        selected = 0;
        from = 0;
        to = std::min((size_t)globals.settings.columns,
                      show_files_indices.size());
      }

      // Full Reset
      else if (key == "R") {
        full_paths.clear();
        const std::vector<std::string> contents =
            exception_list.empty()
                ? File::listfiles_recursive(home_folder)
                : File::listfiles_recursive(home_folder, exception_list);

        for (const auto &content : contents) {
          if (File::isfile(content) && isMediaFile(content))
            full_paths.push_back(content);
        }

        full_paths = File::sortChronological(full_paths);
        is_alpha_sort = false;
        refreshFiles(files, full_paths);

        show_files_indices.resize(files.size());
        std::iota(show_files_indices.begin(), show_files_indices.end(), 0);

        from = 0;
        to = std::min((size_t)globals.settings.columns,
                      show_files_indices.size());
        selected = 0;
      }

      else if (key == "f") { // Toggle Sort
        is_alpha_sort = !is_alpha_sort;
        if (is_alpha_sort)
          std::sort(full_paths.begin(), full_paths.end());
        else
          full_paths = File::sortChronological(full_paths);

        refreshFiles(files, full_paths);
        std::iota(show_files_indices.begin(), show_files_indices.end(), 0);
      }

      else if (key == ".") {
        globals.ui_state = UI_State::LATEST_MEDIA;
      }

      else if (key == ",") {
        globals.ui_state = UI_State::SETTINGS;
      }
    }

    // ... Other states (LATEST MEDIA) ...
    else if (globals.ui_state == UI_State::LATEST_MEDIA) {
      std::vector<std::string> latest =
          File::readfile(globals.paths.latest_media);
      if (!latest.empty()) {
        full_paths.clear();
        for (auto &line : latest) {
          auto parts = funcs::split(line, 0x1F);
          if (!parts.empty() && File::isfile(parts[0]))
            full_paths.push_back(parts[0]);
        }
        refreshFiles(files, full_paths);
        show_files_indices.assign(files.size(), 0);
        std::iota(show_files_indices.begin(), show_files_indices.end(), 0);
        selected = 0;
        from = 0;
        to = std::min((size_t)globals.settings.columns,
                      show_files_indices.size());
      }
      globals.ui_state = UI_State::MAIN_MENU;
    }

    // ... Other states (SETTINGS) ...
    else if (globals.ui_state == UI_State::SETTINGS) {
      settingsMenu(globals);
      // TODO: clamp to, from, and selected
    }
  }

  restoreTerminal();

  std::cout << std::endl;
  funcs::printCentered("Thanks for using MediaPlayer!\n");
  funcs::printCentered("By HassanIQ777\n");
}

void printfiles(const std::vector<size_t> &indices,
                const std::vector<std::string> &files, size_t from, size_t to,
                size_t selected) {
  from = std::max(0UL, std::min(from, indices.size() - 1));
  to = std::max(from, std::min(to, indices.size()));
  selected = std::max(from, std::min(selected, to - 1));

  if (indices.empty()) {
    std::cout << "No files available.\n";
    return;
  }

  std::string border;
  for (size_t i = 0; i < funcs::getTerminalWidth(); i++)
    border += BORDER_CHAR;

  std::cout << TXT_CYAN << "Select media " << A_BOLD << selected + 1 << " / "
            << indices.size() << ":" << A_RESET << std::endl;
  std::cout << TXT_BLUE << border << A_RESET << std::endl;

  for (size_t i = from; i < to; i++) {
    size_t file_index = indices[i];
    std::string display_name =
        trimStr(files[file_index], funcs::str(i + 1).length());
    if (i == selected) {
      std::cout << A_BOLD << TXT_YELLOW << i + 1 << A_RESET << " " << A_BOLD
                << color::bg_rgb(80, 80, 80) << color::fg_rgb(120, 255, 120)
                << display_name << A_RESET << std::endl;
    } else {
      std::cout << TXT_YELLOW << i + 1 << A_RESET << " " << display_name
                << std::endl;
    }
  }

  if (to - from <= 0) {
    std::cout << "No files to display." << std::endl;
    selected = 0;
  }

  std::cout << TXT_BLUE << border << A_RESET << std::endl;
}

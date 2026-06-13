// declarations.hpp

#ifndef DECLARATIONS
#define DECLARATIONS

#include <string>
#include <numeric> // std::iota
#include <atomic>
#include <csignal>
#include <thread>
#include <map>

#include "libutils/src/File.hpp"
#include "libutils/src/funcs.hpp"
#include "libutils/src/color.hpp"
#include "libutils/src/CLIParser.hpp"
#include "libutils/src/Log.hpp"

const std::string BORDER_CHAR = "─";
using funcs::print;

struct Paths //files and dirs
{
	std::string home_dir,
		mediaplayer_dir,
		exception_list,
		history,
		latest_media,
		settings;
};

enum class UI_State
{
	MAIN_MENU,
	SETTINGS,
	LATEST_MEDIA
};

struct Globals
{
	Paths paths;
	UI_State ui_state;
	std::string VERSION;
	size_t columns;
	std::string delimiter;
};

void printLogo()
{
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
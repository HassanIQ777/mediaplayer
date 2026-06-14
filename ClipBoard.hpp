#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Converts "/home/hassan/music/song.mp3" → "file:///home/hassan/music/song.mp3"
// The triple slash is: "file://" + "/home/..." — not a typo, not a cry for help
inline std::string toFileUri(const fs::path &path) {
  return "file://" + fs::absolute(path).string();
}

#include <cstdlib>
#include <stdexcept>

enum class ClipboardBackend { XCLIP, XSEL, WLCLIP, TERMUX, UNKNOWN };

inline ClipboardBackend detectBackend() {
  // The "which" trick — if it returns 0, the tool exists
  if (std::system("command -v termux-clipboard-set >/dev/null 2>&1") == 0)
    return ClipboardBackend::TERMUX;
  if (std::system("command -v wl-copy >/dev/null 2>&1") == 0)
    return ClipboardBackend::WLCLIP;
  if (std::system("command -v xclip >/dev/null 2>&1") == 0)
    return ClipboardBackend::XCLIP;
  if (std::system("command -v xsel >/dev/null 2>&1") == 0)
    return ClipboardBackend::XSEL;
  return ClipboardBackend::UNKNOWN;
}

inline void copyToClipboard(const std::string &text) {
  // Escape single quotes in the text so the shell doesn't have an aneurysm
  // "it's a test" → "it'\''s a test" (shell dark magic, don't question it)
  std::string escaped;
  for (char c : text)
    if (c == '\'')
      escaped += "'\\''";
    else
      escaped += c;

  std::string cmd;
  switch (detectBackend()) {
  case ClipboardBackend::TERMUX:
    cmd = "termux-clipboard-set '" + escaped + "'";
    break;
  case ClipboardBackend::WLCLIP:
    cmd = "wl-copy '" + escaped + "'";
    break;
  case ClipboardBackend::XCLIP:
    cmd = "echo '" + escaped + "' | xclip -selection clipboard";
    break;
  case ClipboardBackend::XSEL:
    cmd = "echo '" + escaped + "' | xsel --clipboard --input";
    break;
  case ClipboardBackend::UNKNOWN:
    throw std::runtime_error(
        "No clipboard backend found. Install xclip, xsel, or wl-clipboard.");
  }

  if (std::system(cmd.c_str()) != 0)
    throw std::runtime_error("Clipboard command failed: " + cmd);
}
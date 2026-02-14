[README.md]

\# WOFL IDE v.0.0.2


<p align="center">
  <a href="https://github.com/whisprer/wofl-ide/releases"> 
    <img src="https://img.shields.io/github/v/release/whisprer/wofl-ide?color=4CAF50&label=release" alt="Release Version"> 
  </a>
  <a href="https://github.com/whisprer/wofl-ide/actions"> 
    <img src="https://img.shields.io/github/actions/workflow/status/whisprer/wofl-ide/lint-and-plot.yml?label=build" alt="Build Status"> 
  </a>
</p>

![Commits](https://img.shields.io/github/commit-activity/m/whisprer/wofl-ide?label=commits) 
![Last Commit](https://img.shields.io/github/last-commit/whisprer/wofl-ide) 
![Issues](https://img.shields.io/github/issues/whisprer/wofl-ide) 
[![Version](https://img.shields.io/badge/version-3.1.1-blue.svg)](https://github.com/whisprer/wofl-ide) 
[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-lightgrey.svg)](https://www.microsoft.com/windows)
[![Python](https://img.shields.io/badge/python-3.8%2B-blue.svg)](https://www.python.org)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

<p align="center">
  <img src="wofl-ide-banner.png" width="850" alt="Wofl-Ide Banner">

# Wofl-Ide


[![wakatime](https://wakatime.com/badge/github/whisprer/wofl-IDE.svg)](https://wakatime.com/badge/github/whisprer/wofl-IDE)

A \*\*ruthlessly minimal\*\* yet fully functional IDE that prioritizes speed and a tiny footprint above all else. Perfect for systems with limited resources or developers who prefer a no-frills, distraction-free coding experience.

Available for both \*\*Windows\*\* (Win32) and \*\*Linux\*\* (SDL2) platforms.

!\[WOFL IDE Screenshot](screenshot.png)

\## Philosophy

I began this project after, (very shortly after i might add), listening to a conversation between Lex Fridman and DHH, in which it was discussed that the average professional coder will spend upto 40-60,000 hrs using their IDE and so it should be like an extension of your body - a thrid-arm. you should know it so inside-out that it requires no thought to use it.
ofc, right then i became quite intensely and uncomfortably aware that the vs code i so deeply find myself dug into, i know maybe... 5%? of its capabilities, at best, and even those i don't necessarily find readily come to mind without some looking to a help-file occasionally. so, typically, instead of deciding to sit down and really learn vs code well, instead i reasoned the best way to learn an IDE inside-out would be to... build one from scratch! ...and also conveniently at the same time therefore have it be absolutely perfectly designed to my own needs and wants.
this project is the result and although it does have a _very_ few minor quirks in the choices of built in pre-programmed modules as far as language choices or themes go, for the most part is is surprisingly lacking in color from my own personailty. fortunately it is ridiculously modular so if you wish to make your copy as tweaked to your own personality as possible or to add something particularly obsucre/unique - that is as easy as can be imagined. furthermore, the thing just lends  itself to a community-driven, module sharing type ethos, for those who wish to develop for others or swap code or w/e. so, without further ado, i introduce wofl-IDE - a very very minimal IDE, in all honesty designed to help peopl to learn their IDEs better.

WOFL IDE follows an ultra-minimalist approach:
\- \*\*Single executable\*\* - Minimal dependencies 
\- \*\*Blazing fast startup\*\* - Ready to code in milliseconds  
\- \*\*Tiny footprint\*\* - Minimal memory and disk usage
\- \*\*Keyboard-driven\*\* - Mouse optional, productivity maximized
\- \*\*Essential features only\*\* - No bloat, just what you need to code


\## Features

\### Core Functionality
\- \*\*Syntax highlighting\*\* for 12+ common languages (C, C++, Python, JavaScript, Go, Rust, HTML, CSS, JSON, Markdown, Shell, Assembly, CSV)
\- \*\*Fast text editing\*\* with gap buffer for efficient insertions/deletions
\- \*\*Basic file operations\*\* - Open, save, create new files
\- \*\*Integrated build system\*\* - Execute files or run build scripts (Windows)
\- \*\*Built-in console\*\* - View program output without leaving the editor (Windows)
\- \*\*Find functionality\*\* - Quick text search with F3/Shift+F3
\- \*\*Command palette\*\* - Ctrl+P for quick actions
\- \*\*Git integration\*\* - Basic git commands and configurable auto-backup (Windows)
\- \*\*Plugin system\*\* - Modular architecture for extending functionality (Windows)
\- \*\*Theme system\*\* - Customizable color schemes and UI themes (Windows)

\### User Interface
\- \*\*Single pane design\*\* - Clean canvas, no sidebars or complex menus
\- \*\*Minimal UI elements\*\* - Focus on your code, not the interface
\- \*\*Customizable themes\*\* - Dark and light color schemes (Windows)
\- \*\*Status bar\*\* - Essential info without clutter


\## Quick Start

\### Windows Version
\*\*Prerequisites:\*\*
\- Windows 10/11
\- Visual Studio Build Tools or Visual Studio Community
\- x64 Native Tools Command Prompt

\*\*Build Commands:\*\*

```batch
\# Debug build (with symbols)
build\_debug.bat
\# Release build (optimized)
build.bat
```

\*\*Project Structure (Windows):\*\*

```
wofl-ide/
├── src/                    # Source code
│   ├── main\_win32.c       # Main application (558 lines)
│   ├── editor\_buffer.c    # Text buffer management (148 lines)  
│   ├── editor\_render.c    # Rendering and UI (226 lines)
│   ├── cmd\_palette.c      # Command palette (70 lines)
│   ├── config.c           # Configuration system (70 lines)
│   ├── find.c             # Find functionality (35 lines)
│   ├── run\_exec\_win32.c   # Process execution (71 lines)
│   ├── syntax\_\*.c         # Language syntax highlighters
│   ├── editor.h           # Main header (171 lines)
│   └── syntax.h           # Syntax definitions (9 lines)
├── build\_debug.bat        # Debug build script
├── build.bat              # Release build script  
└── dist/                  # Output directory
```


\### Linux Version (SDL2)

\*\*Prerequisites:\*\*
\- GCC compiler
\- SDL2 development libraries
\- SDL2\_ttf development libraries

\*\*Ubuntu/Debian:\*\*

```bash
sudo apt update
sudo apt install build-essential libsdl2-dev libsdl2-ttf-dev
```

\*\*Fedora/CentOS:\*\*

```bash
sudo dnf install gcc SDL2-devel SDL2\_ttf-devel
```

\*\*Arch Linux:\*\*

```bash
sudo pacman -S gcc sdl2 sdl2\_ttf
```

\*\*Build Commands:\*\*

```bash
\# Build the editor
make
\# Clean build artifacts
make clean
```

\*\*Project Structure (Linux):\*\*

```
wofl-ide-linux/
├── src/                    # Source code
│   ├── main.c             # Main application entry point
│   ├── app.h              # Core application state and types
│   ├── gap\_buffer.c/h     # Efficient text buffer implementation
│   ├── cursor.c/h         # Cursor movement and positioning
│   ├── editing.c/h        # Text insertion and deletion
│   ├── find.c/h           # Search functionality
│   ├── file\_ops.c/h       # File I/O operations
│   ├── input.c/h          # Keyboard and mouse input handling
│   ├── language.c/h       # Language detection
│   ├── rendering.c/h      # SDL2 rendering engine
│   ├── sdl\_utils.c/h      # SDL2 initialization and cleanup
│   └── syntax.c/h         # Advanced syntax highlighting
└── Makefile               # Build configuration
```


\## Usage

\### Essential Shortcuts (Both Platforms)

\*\*File Operations:\*\*
\- `Ctrl+O` - Open file
\- `Ctrl+S` - Save file  
\- `Ctrl+Shift+S` - Save as (Windows) / Save as (Linux via dialog)
\- `Ctrl+Q` - Quit

\*\*Navigation:\*\*
\- `Arrow Keys` - Move cursor
\- `Ctrl+F` - Find text
\- `F3` / `Shift+F3` - Find next/previous
\- `Ctrl+P` - Command palette

\*\*Editing:\*\*
\- `Tab` - Insert 4 spaces
\- `Backspace` - Delete backward
\- `Delete` - Delete forward
\- `Enter` - New line

\*\*Execution (Windows only):\*\*
\- `F5` - Run/execute current file
\- `Ctrl+/` - Toggle line comment


\### Running Code

\*\*Windows:\*\*
1\. Open a source file
2\. Press `F5` to execute
3\. View output in the integrated console

\*\*Linux:\*\*
1\. Open files using `Ctrl+O` (uses system file dialog if available)
2\. Edit and save as needed
3\. Use external terminal for compilation/execution


\## Platform Differences

| Feature | Windows (Win32) | Linux (SDL2) |
|---------|----------------|--------------|
| \*\*Backend\*\* | Pure Win32 API | SDL2 + TTF |
| \*\*Dependencies\*\* | None | SDL2, SDL2\_ttf |
| \*\*File Dialogs\*\* | Native Windows | Zenity (fallback to console) |
| \*\*Execution\*\* | Integrated F5 runner | External terminal |
| \*\*Themes\*\* | Full theme system | Basic syntax colors |
| \*\*Plugins\*\* | Plugin architecture | Core features only |
| \*\*Build System\*\* | Integrated | External make/cmake |


\## Supported Languages

| Language | Extension | Features |
|----------|-----------|----------|
| C/C++ | `.c`, `.cpp`, `.h` | Keywords, strings, comments, preprocessor |
| Python | `.py` | Keywords, strings, triple quotes, f-strings |
| JavaScript | `.js` | ES6+ syntax, template literals, regex |
| Go | `.go` | Keywords, strings, comments |
| Rust | `.rs` | Keywords, strings, lifetimes |
| HTML | `.html` | Tags, attributes, entities |
| CSS | `.css` | Selectors, properties, values |
| JSON | `.json` | Structure validation, syntax |
| Markdown | `.md` | Headers, emphasis, code blocks |
| Shell | `.sh`, `.bash` | Variables, keywords, strings |
| Assembly | `.asm` | Instructions, registers, labels |
| Lua | `.lua` | Keywords, strings, comments |


\## Configuration

\### Windows

WOFL IDE uses a simple `.wofl` config file:

```ini
theme=dark
font\_size=12
tab\_width=4
show\_line\_numbers=true
word\_wrap=false
```

\### Linux

Configuration is currently hardcoded. Font detection automatically tries:
1\. `/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf`
2\. `/usr/share/fonts/TTF/DejaVuSansMono.ttf`  
3\. `/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf`


\## Design Principles
1\. \*\*Minimal Resource Usage\*\* - Optimized for speed and low memory consumption
2\. \*\*Platform Native\*\* - Win32 API on Windows, SDL2 on Linux
3\. \*\*Fast Compilation\*\* - Single-pass build in seconds
4\. \*\*Keyboard-First\*\* - Every feature accessible without mouse
5\. \*\*Extensible Core\*\* - Clean architecture for future enhancements

\## Performance
\*\*Windows Version:\*\*
\- \*\*Startup time:\*\* < 10ms
\- \*\*Memory usage:\*\* < 2MB typical
\- \*\*File size:\*\* ~150KB executable
\- \*\*Supports files:\*\* Up to 1M+ lines efficiently

\*\*Linux Version:\*\*
\- \*\*Startup time:\*\* < 50ms
\- \*\*Memory usage:\*\* < 5MB typical (including SDL2)
\- \*\*Dependencies:\*\* SDL2 (~1MB), SDL2\_ttf (~100KB)
\- \*\*Supports files:\*\* Up to 1M+ lines efficiently

\## Architecture Notes

Both versions share the same core text editing algorithms:
\- \*\*Gap buffer implementation\*\* for O(1) insertions at cursor
\- \*\*Efficient cursor management\*\* with line/column tracking
\- \*\*Modular syntax highlighting\*\* with extensible token system
\- \*\*Memory-safe string handling\*\* throughout

The Windows version prioritizes zero dependencies while the Linux version leverages SDL2 for cross-platform compatibility and robust font rendering.


\## Contributing

Please, Please, Please: don't just consider collaboration - damn well **take** to the point of absolute theivery and make this thing your own. that is exactly and completely what it was all about - something for people to customise to their needs and make specific to their wants. so take no shame in customizing or doing ridiculously useless or selfish things with it. have at it.

This project values simplicity and minimalism. When contributing:
1\. \*\*Keep it minimal\*\* - Only essential features
2\. \*\*Optimize for speed\*\* - Performance over convenience  
3\. \*\*Respect platform conventions\*\* - Win32 on Windows, SDL2 on Linux
4\. \*\*Test thoroughly\*\* - Ensure stability across platforms
5\. \*\*Document clearly\*\* - Code should be self-explanatory


\## Troubleshooting

\### Linux Build Issues
\*\*Missing SDL2 libraries:\*\*

\# Check if libraries are installed
`pkg-config --exists sdl2 sdl2\_ttf \&\& echo "SDL2 found" || echo "SDL2 missing"`
\# Install on Ubuntu/Debian
`sudo apt install libsdl2-dev libsdl2-ttf-dev`

\*\*Font loading failures:\*\*
\- Ensure DejaVu or Liberation fonts are installed
\- Check font paths in `/usr/share/fonts/`

\### Windows Build Issues

\*\*Missing Visual Studio tools:\*\*
\- Install Visual Studio Build Tools
\- Use x64 Native Tools Command Prompt
\- Ensure Windows SDK is available


\## License

Hybrid MIT \& CC0

#
\## Acknowledgments

Built for developers who believe that sometimes less is more. Inspired by the philosophy that a great tool should be invisible - letting you focus entirely on your code. Thnx CaudeOpus4.1, ChatGPT5, Grok4 and Gemini2.5Pro - RIP/Welcome Back frens. And finally RTC for providing the evidence it _is_possible to actually complete a project!

---

\*"Simplicity is the ultimate sophistication." - Leonardo da Vinci\*

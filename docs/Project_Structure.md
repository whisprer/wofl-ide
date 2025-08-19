OG structure from G-Petey -> Claude



// ==================== Project Structure ====================
// Recommended directory layout for the refactored codebase

/\*
wofl-ide/
├── src/
│   ├── editor.h              # Main header file
│   ├── main\_win32.c          # Entry point and window management
│   ├── editor\_buffer.c       # Gap buffer implementation
│   ├── editor\_render.c       # Rendering and display
│   ├── syntax\_core.c         # Syntax highlighting core
│   ├── syntax\_c.c            # C/C++ syntax
│   ├── syntax\_py.c           # Python syntax
│   ├── syntax\_js.c           # JavaScript syntax
│   ├── syntax\_html.c         # HTML syntax
│   ├── syntax\_css.c          # CSS syntax
│   ├── syntax\_json.c         # JSON syntax
│   ├── syntax\_md.c           # Markdown syntax
│   ├── syntax\_go.c           # Go syntax
│   ├── syntax\_rs.c           # Rust syntax
│   ├── syntax\_sh.c           # Shell script syntax
│   ├── syntax\_lua.c          # Lua syntax
│   ├── find.c                # Find/replace functionality
│   ├── cmd\_palette.c         # Command palette
│   ├── config.c              # Configuration management
│   └── run\_exec\_win32.c      # Process execution
├── dist/
│   └── wofl-ide.exe          # Built executable
├── build.bat                 # Release build script
├── build\_debug.bat           # Debug build script
├── Makefile                  # Alternative build system
└── README.md                 # Documentation
\*/





-------



Structure post Claudes Re-Factor \& Post Extra Plugin system:





// ==================== Complete File List for Build ====================

/\*

Your src/ directory should now contain:



CORE FILES (Original):

\- editor.h

\- main\_win32.c

\- editor\_buffer.c

\- editor\_render.c

\- syntax\_core.c        ← UPDATED with new languages

\- find.c

\- cmd\_palette.c

\- config.c

\- run\_exec\_win32.c



ORIGINAL SYNTAX FILES:

\- syntax\_c.c

\- syntax\_py.c

\- syntax\_js.c

\- syntax\_html.c

\- syntax\_css.c

\- syntax\_json.c

\- syntax\_md.c

\- syntax\_go.c

\- syntax\_rs.c

\- syntax\_sh.c

\- syntax\_lua.c



NEW FILES TO ADD:

\- theme\_system.h       ← NEW

\- theme\_system.c       ← NEW

\- plugin\_system.h      ← NEW

\- plugin\_system.c      ← NEW

\- syntax\_cpp.c         ← NEW

\- syntax\_asm.c         ← NEW

\- syntax\_csv.c         ← NEW

\- plugin\_git.c         ← NEW

\*/

-------


Claude stresses:

src/
├── editor.h              ← Add new Language enums here
├── syntax_core.c         ← Add file extension detection here
├── editor_render.c       ← Update status bar language display
├── syntax_cpp.c          ← NEW FILE (C++ highlighting)
├── syntax_asm.c          ← NEW FILE (Assembly highlighting)
├── syntax_csv.c          ← NEW FILE (CSV highlighting)
├── theme_system.h        ← NEW FILE
├── theme_system.c        ← NEW FILE
├── plugin_system.h       ← NEW FILE
├── plugin_system.c       ← NEW FILE
├── plugin_git.c          ← NEW FILE
└── [all other existing files...]

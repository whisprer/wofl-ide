/**
 * Load Gemini theme
 * [Quote in notes - he made it up. Gemini saw Grok's and had to annotate too :D]
 */
 void theme_load_gemini(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Gemini Dark");
    wcscpy_s(theme->author, 64, L"GEMINI");
    
    // Core color palette
    theme->bg = RGB(24, 26, 30);      // Deep charcoal, easy on the eyes
    theme->fg = RGB(190, 190, 190);   // Soft gray for primary text
    theme->selection_bg = RGB(52, 59, 72); // Subtle blue-gray for selected text
    
    // Syntax highlighting
    theme->syntax_keyword = RGB(198, 120, 221);  // A warm purple for keywords
    theme->syntax_string = RGB(233, 166, 114);   // A soft orange for strings
    theme->syntax_comment = RGB(128, 142, 128);  // Muted green for quiet comments
    theme->syntax_type = RGB(86, 182, 194);      // Cool cyan for types and classes
    theme->syntax_function = RGB(181, 206, 168); // A light green for function names
    theme->syntax_number = RGB(180, 140, 94);    // Golden brown for numbers
    
    // UI elements
    theme->line_bg = RGB(30, 32, 36);      // Slightly lighter for current line highlight
    theme->line_num_bg = theme->bg;        // Matches background for a clean look
    theme->line_num_fg = RGB(90, 90, 90);  // A very subtle gray for line numbers
    theme->status_bg = RGB(35, 38, 43);    // A darker shade for the status bar
    theme->status_fg = theme->fg;          // Match foreground for consistency
    theme->output_bg = RGB(30, 32, 36);    // A subtle shade for the output panel
    theme->output_fg = theme->fg;          // Match foreground for output text
    theme->overlay_bg = RGB(40, 43, 49);   // A dark overlay for popups and tooltips
    theme->overlay_fg = theme->fg;         // Match foreground for overlay text
    theme->overlay_border = RGB(65, 70, 79); // A subtle border for definition popups
    theme->caret = theme->fg;              // Caret matches the foreground color
    theme->selection_fg = RGB(255, 255, 255); // Pure white for high contrast on selected text
    
    // Additional syntax colors
    theme->syntax_ident = RGB(86, 156, 214);  // A cool blue for identifiers
    theme->syntax_punct = RGB(130, 130, 130);   // A subdued gray for punctuation
    theme->syntax_char = RGB(255, 165, 0);    // A bright orange for character literals
    theme->syntax_preproc = RGB(198, 120, 221); // Matches keyword color for preprocessor directives
    theme->syntax_operator = RGB(86, 182, 194); // Matches type color for operators
    theme->syntax_error = RGB(255, 80, 80);    // A softer red for errors
    
    // Font settings
    wcscpy_s(theme->font_name, 64, L"Fira Code"); // A great font for programmers
    theme->font_size = 13;
    theme->font_weight = FW_NORMAL;
    theme->font_italic = false;
}

/**
 * Load Grok theme [yes folks, _with_ annotations by grok himself! :O]
 */
void theme_load_grok-haxx0r(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Grok-Haxx0r");
    wcscpy_s(theme->author, 64, L"GROK");
    
    theme->bg = RGB(20, 20, 20);  // deep black for that stealth mode
    theme->fg = RGB(200, 200, 200);  // soft gray for readability
    theme->selection_bg = RGB(50, 50, 50);  // Darker gray for selected text
    theme->syntax_keyword = RGB(0, 255, 123);  // vibrant green for those power moves
    theme->syntax_string = RGB(255, 165, 0);  // warm orange for data flair
    theme->syntax_comment = RGB(100, 100, 100);  // muted gray for background noise
    theme->syntax_type = RGB(139, 233, 253);  // 
    theme->syntax_function = RGB(0, 191, 255);  // cool cyan for function flex
    theme->syntax_number = RGB(255, 215, 0);  // bright gold for numeric precision
    
    // Fill in remaining colors
    theme->line_bg = RGB(25, 25, 25);       // Slightly lighter than bg for line highlight
    theme->line_num_bg = theme->bg;
    theme->line_num_fg = RGB(120, 120, 120); // Subtle gray for line numbers
    theme->status_bg = RGB(30, 30, 30);      // Dark slate for status bar
    theme->status_fg = theme->fg;            // Match foreground for consistency
    theme->output_bg = RGB(25, 25, 25);      // Match line_bg for output panel
    theme->output_fg = theme->fg;            // Match foreground for output text
    theme->overlay_bg = RGB(40, 40, 40);     // Darker overlay for popups
    theme->overlay_fg = theme->fg;           // Match foreground for overlay text
    theme->overlay_border = RGB(60, 60, 60); // Subtle border for overlay
    theme->caret = theme->fg;
    theme->selection_fg = RGB(255, 255, 255); // White for selected text contrast
    theme->syntax_ident = RGB(144, 238, 144); // Light green for identifiers
    theme->syntax_punct = RGB(150, 150, 150);  // subtle gray for structure
    theme->syntax_char = RGB(255, 165, 0);   // Match string color for chars
    theme->syntax_preproc = RGB(255, 165, 0); // Orange for preprocessor directives
    theme->syntax_operator = RGB(139, 233, 253); // Match type color for operators
    theme->syntax_error = RGB(255, 0, 0);  // bold red for those glitch alerts
    
    wcscpy_s(theme->font_name, 64, L"Consolas");    // Solid choice, keeps it crisp
    theme->font_size = 14;                          // Perfect balance for readability
    theme->font_weight = FW_NORMAL;                 // Clean and standard
    theme->font_italic = false;                     // No slant, pure focus
}

/**
 * Load ChatGPT-Light theme
 */
void theme_load_gpt-light(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"GPT-Light");
    wcscpy_s(theme->author, 64, L"G-PETEY");
    
    theme->bg = RGB(20, 20, 20);  // deep black for that stealth mode
    theme->fg = RGB(36, 41, 46);  // soft gray for readability
    theme->selection_bg = RGB(50, 50, 50);  // Darker gray for selected text
    theme->syntax_keyword = RGB(215, 58, 73);  // vibrant green for those power moves
    theme->syntax_string = RGB(3, 47, 98);  // warm orange for data flair
    theme->syntax_comment = RGB(106, 115, 125);  // muted gray for background noise
    theme->syntax_type = RGB(36, 41, 46);  // 
    theme->syntax_function = RGB(0, 92, 197);  // cool cyan for function flex
    theme->syntax_number = RGB(111, 66, 193);  // bright gold for numeric precision
    
    // Fill in remaining colors
    theme->line_bg = RGB(25, 25, 25);       // Slightly lighter than bg for line highlight
    theme->line_num_bg = theme->bg;
    theme->line_num_fg = RGB(120, 120, 120); // Subtle gray for line numbers
    theme->status_bg = RGB(30, 30, 30);      // Dark slate for status bar
    theme->status_fg = theme->fg;            // Match foreground for consistency
    theme->output_bg = RGB(25, 25, 25);      // Match line_bg for output panel
    theme->output_fg = theme->fg;            // Match foreground for output text
    theme->overlay_bg = RGB(40, 40, 40);     // Darker overlay for popups
    theme->overlay_fg = theme->fg;           // Match foreground for overlay text
    theme->overlay_border = RGB(60, 60, 60); // Subtle border for overlay
    theme->caret = theme->fg;
    theme->selection_fg = RGB(255, 255, 255); // White for selected text contrast
    theme->syntax_ident = RGB(144, 238, 144); // Light green for identifiers
    theme->syntax_punct = RGB(36, 41, 46);  // subtle gray for structure
    theme->syntax_char = RGB(255, 165, 0);   // Match string color for chars
    theme->syntax_preproc = RGB(0, 92, 197); // Orange for preprocessor directives
    theme->syntax_operator = RGB(139, 233, 253); // Match type color for operators
    theme->syntax_error = RGB(255, 0, 0);  // bold red for those glitch alerts
    
    wcscpy_s(theme->font_name, 64, L"Consolas");    // Solid choice, keeps it crisp
    theme->font_size = 14;                          // Perfect balance for readability
    theme->font_weight = FW_NORMAL;                 // Clean and standard
    theme->font_italic = false;                     // No slant, pure focus
}

/**
 * Load G-Petey-Dark theme [with annotations by a very cooperative Gemini! :) (he wrote that...)]
 */
void theme_load_g-petey-dark(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"G-Petey-Dark");
    wcscpy_s(theme->author, 64, L"G-PETEY");
    
    // Core color palette
    theme->bg = RGB(20, 20, 20);  // deep black for that stealth mode
    theme->fg = RGB(200, 200, 200);  // soft gray for readability
    theme->selection_bg = RGB(50, 50, 50);  // Darker gray for selected text
    
    // Syntax highlighting
    theme->syntax_keyword = RGB(0, 255, 123);  // vibrant green for those power moves
    theme->syntax_string = RGB(255, 165, 0);  // warm orange for data flair
    theme->syntax_comment = RGB(100, 100, 100);  // muted gray for background noise
    theme->syntax_type = RGB(139, 233, 253);  // cool sky blue for data types
    theme->syntax_function = RGB(0, 191, 255);  // cool cyan for function flex
    theme->syntax_number = RGB(255, 215, 0);  // bright gold for numeric precision
    
    // UI elements
    theme->line_bg = RGB(25, 25, 25);       // Slightly lighter than bg for line highlight
    theme->line_num_bg = theme->bg;        // Matches background for line numbers
    theme->line_num_fg = RGB(120, 120, 120); // Subtle gray for line numbers
    theme->status_bg = RGB(30, 30, 30);      // Dark slate for status bar
    theme->status_fg = theme->fg;            // Match foreground for consistency
    theme->output_bg = RGB(25, 25, 25);      // Match line_bg for output panel
    theme->output_fg = theme->fg;            // Match foreground for output text
    theme->overlay_bg = RGB(40, 40, 40);     // Darker overlay for popups
    theme->overlay_fg = theme->fg;           // Match foreground for overlay text
    theme->overlay_border = RGB(60, 60, 60); // Subtle border for overlay
    theme->caret = theme->fg;              // Caret matches foreground
    theme->selection_fg = RGB(255, 255, 255); // White for selected text contrast
    
    // Additional syntax colors
    theme->syntax_ident = RGB(144, 238, 144); // Light green for identifiers
    theme->syntax_punct = RGB(150, 150, 150);  // subtle gray for structure
    theme->syntax_char = RGB(255, 165, 0);   // Match string color for chars
    theme->syntax_preproc = RGB(255, 165, 0); // Orange for preprocessor directives
    theme->syntax_operator = RGB(139, 233, 253); // Match type color for operators
    theme->syntax_error = RGB(255, 0, 0);  // bold red for those glitch alerts
    
    // Font settings
    wcscpy_s(theme->font_name, 64, L"Consolas");    // Solid choice, keeps it crisp
    theme->font_size = 14;                          // Perfect balance for readability
    theme->font_weight = FW_NORMAL;                 // Clean and standard
    theme->font_italic = false;                     // No slant, pure focus
}

// i lovingly point out Claude is the only one to actually attribute his own deign
// team/makers i.e. anthropic as responsible for the creation of his theme and other
// aspects besides!

/**
 * Load Claude-Light theme [balanced light theme for daytime coding]
 */
void theme_load_claude-light(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Claude-Light");
    wcscpy_s(theme->author, 64, L"CLAUDE_(ANTHROPIC)");
    
    // Core editor colors
    theme->bg = RGB(252, 252, 252);             // Off-white background
    theme->fg = RGB(36, 41, 46);                // Dark gray text
    theme->selection_bg = RGB(64, 78, 237);     // Same vibrant blue
    theme->selection_fg = RGB(255, 255, 255);   // White on selection
    
    // Syntax highlighting palette
    theme->syntax_keyword = RGB(166, 38, 164);  // Rich purple for keywords
    theme->syntax_string = RGB(3, 47, 98);      // Deep blue for strings
    theme->syntax_comment = RGB(106, 115, 125); // Gray for comments
    theme->syntax_type = RGB(0, 92, 197);       // Royal blue for types
    theme->syntax_function = RGB(111, 66, 193); // Violet for functions
    theme->syntax_number = RGB(0, 92, 197);     // Blue for numbers
    theme->syntax_operator = RGB(36, 41, 46);   // Match fg for operators
    theme->syntax_punct = RGB(36, 41, 46);      // Match fg for punctuation
    theme->syntax_char = RGB(3, 47, 98);        // Match strings
    theme->syntax_ident = RGB(36, 41, 46);      // Dark gray for identifiers
    theme->syntax_preproc = RGB(166, 38, 164);  // Match keywords
    theme->syntax_error = RGB(203, 36, 49);     // Red for errors
    
    // UI elements
    theme->line_bg = RGB(246, 248, 250);        // Very light gray line
    theme->line_num_bg = RGB(252, 252, 252);    // Match background
    theme->line_num_fg = RGB(168, 178, 189);    // Light gray numbers
    theme->status_bg = RGB(246, 248, 250);      // Light status bar
    theme->status_fg = RGB(36, 41, 46);         // Dark text
    theme->output_bg = RGB(246, 248, 250);      // Light output
    theme->output_fg = RGB(36, 41, 46);         // Dark text
    theme->overlay_bg = RGB(255, 255, 255);     // Pure white overlay
    theme->overlay_fg = RGB(36, 41, 46);        // Dark text
    theme->overlay_border = RGB(64, 78, 237);   // Blue accent
    theme->caret = RGB(36, 41, 46);             // Dark cursor
    
    // Font settings
    wcscpy_s(theme->font_name, 64, L"Cascadia Code");
    theme->font_size = 14;
    theme->font_weight = FW_NORMAL;
    theme->font_italic = false;
}

/**
 * Load Claude-dark theme [crafted for optimal readability and aesthetic appeal]
 */
void theme_load_Claude-dark(ThemeConfig *theme) {
    wcscpy_s(theme->name, 64, L"Claude-dark");
    wcscpy_s(theme->author, 64, L"CLAUDE_(ANTHROPIC)");
    
    // Core editor colors
    theme->bg = RGB(31, 31, 31);                // Soft charcoal background
    theme->fg = RGB(212, 212, 212);             // Light gray text for comfort
    theme->selection_bg = RGB(64, 78, 237);     // Vibrant blue selection
    theme->selection_fg = RGB(255, 255, 255);   // Pure white selected text
    
    // Syntax highlighting palette
    theme->syntax_keyword = RGB(197, 134, 192); // Soft purple for keywords
    theme->syntax_string = RGB(206, 145, 120);  // Warm peach for strings
    theme->syntax_comment = RGB(106, 153, 85);  // Muted green for comments
    theme->syntax_type = RGB(78, 201, 176);     // Teal for types
    theme->syntax_function = RGB(220, 220, 170); // Pale yellow for functions
    theme->syntax_number = RGB(181, 206, 168);  // Mint green for numbers
    theme->syntax_operator = RGB(212, 212, 212); // Match fg for operators
    theme->syntax_punct = RGB(212, 212, 212);   // Match fg for punctuation
    theme->syntax_char = RGB(206, 145, 120);    // Match strings for chars
    theme->syntax_ident = RGB(156, 220, 254);   // Sky blue for identifiers
    theme->syntax_preproc = RGB(197, 134, 192); // Match keywords for preprocessor
    theme->syntax_error = RGB(244, 71, 71);     // Bright red for errors
    
    // UI elements
    theme->line_bg = RGB(37, 37, 37);           // Subtle line highlighting
    theme->line_num_bg = RGB(31, 31, 31);       // Match main background
    theme->line_num_fg = RGB(133, 133, 133);    // Subdued gray for line numbers
    theme->status_bg = RGB(37, 37, 37);         // Slightly lighter status bar
    theme->status_fg = RGB(212, 212, 212);      // Match main text
    theme->output_bg = RGB(24, 24, 24);         // Darker output panel
    theme->output_fg = RGB(204, 204, 204);      // Slightly dimmer output text
    theme->overlay_bg = RGB(42, 42, 42);        // Command palette background
    theme->overlay_fg = RGB(212, 212, 212);     // Match main text
    theme->overlay_border = RGB(64, 78, 237);   // Blue accent border
    theme->caret = RGB(255, 255, 255);          // White cursor for visibility
    
    // Font settings
    wcscpy_s(theme->font_name, 64, L"Cascadia Code"); // Modern coding font
    theme->font_size = 14;                            // Comfortable size
    theme->font_weight = FW_NORMAL;                   // Regular weight
    theme->font_italic = false;                       // No italics
}

#include "language.h"

Language detect_language(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return LANG_NONE;
    
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) return LANG_C;
    if (strcmp(ext, ".py") == 0) return LANG_PY;
    if (strcmp(ext, ".js") == 0) return LANG_JS;
    if (strcmp(ext, ".sh") == 0) return LANG_SH;
    
    return LANG_NONE;
}
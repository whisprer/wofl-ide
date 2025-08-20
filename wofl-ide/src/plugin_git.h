#ifndef PLUGIN_GIT_H
#define PLUGIN_GIT_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

// Return 1 if the given directory is a Git repo, 0 otherwise.
// Safe, simple signature to match typical C impls.
int git_is_repo(const wchar_t *dir);

// Return 0 on success, nonzero on failure.
// 'message' is the commit message.
int git_commit(const wchar_t *dir, const wchar_t *message);

#ifdef __cplusplus
}
#endif

#endif // PLUGIN_GIT_H

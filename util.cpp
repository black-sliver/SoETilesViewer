#include "util.h"

#ifdef WIN32
#include <windows.h>
std::string tmpfn(const char* prefix) {
    char tempfile_path[MAX_PATH];
    char tempfile_name[MAX_PATH]; tempfile_name[0]=0;
    int res;

    res = GetTempPathA(MAX_PATH, tempfile_path);
    if (res > MAX_PATH || res == 0) return "";

    res = GetTempFileNameA(tempfile_path, prefix, 0, tempfile_name);
    if (res == 0) return "";

    return tempfile_name;
}
#endif

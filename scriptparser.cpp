#include "scriptparser.h"
#include <string.h>

// yes, this is ugly.
static FILE* pipe = nullptr;

#define main _scriptparser
#define printf(args...) fprintf(pipe, args)
#ifdef SCRIPT_PARSER_HTML5
#define HTML5
#else
#define HTML4
#endif
#define PRINT_HEX
#include "SoEScriptDumper/list-rooms.cpp"
#undef main
#undef printf

ScriptParser::ScriptParser(const char* rom)
{
    _romfile = rom;
}

std::string ScriptParser::parse() const
{
    assert(!pipe);

    // NOTE: css below seems to be the fastest
    std::string res = "<html>"
            "<head><style>"
            "body { white-space: pre; } "
        #ifndef HTML4 // we use <font color> for HTML4 instead
            ".r { color:#c22; } "
            ".g { color:#2c2; } "
            ".u { color:#c2c; } "
            ".h,.t { color:#cc2; } "
            ".t { font-weight:bold; } "
        #endif
            "</style></head><body>";

    char app[] = "";
    char* fn = strdup(_romfile.c_str());
    char* args[] = {app,fn};
    if (!fn) goto done;

    pipe = tmpfile();
    if (!pipe) goto done;
    _scriptparser(2, args);
    rewind(pipe);
    while (!feof(pipe)) {
        char buf[4096];
        size_t rd = fread(buf, 1, sizeof(buf), pipe);
        res += std::string(buf, rd);
    }

done:
    if (pipe) fclose(pipe);
    pipe = nullptr;
    free(fn);
    res += " </body></html>";
    return res;
}

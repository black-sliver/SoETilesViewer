#include "scriptparser.h"
#include "util.h"
#include <string.h>
#include <stdio.h> // include stdio before overriding stderr
#include <assert.h>
#include <errno.h>

// yes, this is ugly.
static FILE* outpipe = nullptr;
static FILE* errpipe = nullptr;

#ifdef stderr
#undef stderr
#endif
#define stderr errpipe

#define die(msg) do {\
    fprintf(stderr, "%s", msg);\
    free(buf);\
    return 1;\
} while (false);
#define main _scriptparser
#define printf(args...) fprintf(outpipe, args)
#ifdef SCRIPT_PARSER_HTML5
#define HTML5
#else
#define HTML4
#endif
#define NO_BOLD // does not play nice with synchronized textboxes
#define PRINT_HEX
#include "SoEScriptDumper/list-rooms.cpp"
#undef main
#undef printf
#undef stderr

ScriptParser::ScriptParser(const char* rom)
{
    _romfile = rom;
}

std::string ScriptParser::parse(std::string* err) const
{
    assert(!outpipe);
    assert(!errpipe);

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

#ifdef WIN32
    std::string outfn = tmpfn("out");
    std::string errfn = tmpfn("err");
#endif

    char app[] = "";
    char* fn = strdup(_romfile.c_str());
    char* args[] = {app,fn};
    assert(fn);

#ifdef WIN32
    outpipe = fopen(outfn.c_str(), "w+");
#else
    outpipe = tmpfile();
#endif
    if (!outpipe) {
        if (err) *err = std::string("tmpfile: ") + strerror(errno);
        goto done;
    }
#ifdef WIN32
    errpipe = fopen(errfn.c_str(), "w+");
#else
    errpipe = tmpfile();
#endif
    if (!errpipe) {
        if (err) *err = std::string("tmpfile: ") + strerror(errno);
        goto done;
    }

    _scriptparser(2, args);

    rewind(outpipe);
    while (!feof(outpipe)) {
        char buf[4096];
        size_t rd = fread(buf, 1, sizeof(buf), outpipe);
        res += std::string(buf, rd);
    }
    rewind(errpipe);
    while (err && !feof(errpipe)) {
        char buf[4096];
        size_t rd = fread(buf, 1, sizeof(buf), errpipe);
        *err += std::string(buf, rd);
    }

done:
    if (outpipe) fclose(outpipe);
    outpipe = nullptr;
    if (errpipe) fclose(errpipe);
    errpipe = nullptr;
    free(fn);
    res += " </body></html>";
#ifdef WIN32
    if (! outfn.empty()) unlink(outfn.c_str());
    if (! errfn.empty()) unlink(errfn.c_str());
#endif
    return res;
}

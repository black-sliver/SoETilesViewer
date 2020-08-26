#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

#include <string>

class ScriptParser
{
public:
    ScriptParser(const char* filename);

    std::string parse() const;
private:
    std::string _romfile;
};

#endif // SCRIPTPARSER_H

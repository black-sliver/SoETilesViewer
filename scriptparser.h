#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

#include <string>

class ScriptParser
{
public:
    ScriptParser(const char* filename);

    std::string parse(std::string* err=NULL, bool dark=false) const;
private:
    std::string _romfile;
};

#endif // SCRIPTPARSER_H

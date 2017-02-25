#include "File.h"
#include <algorithm>
#include <functional>
#include <sstream>

// Trim functions from http://stackoverflow.com/a/217605

static bool canSkip(char ch)
{
    return isspace(ch) || ch == '\r';
}

static const auto canSkipFunc = std::ptr_fun<char>(canSkip);
static const std::string includeDirective = "#include";

inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(canSkipFunc)));
}

inline void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(canSkipFunc)).base(), s.end());
}

inline void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

inline bool startsWith(const std::string& str, const std::string& prefix)
{
    size_t prefixLength = prefix.length();
    return strncmp(str.c_str(), prefix.c_str(), prefixLength) == 0;
}

inline std::string getDirectory(const char* path)
{
    std::string dir = path;
    size_t pathSepLoc = dir.find_last_of('/');
    if (pathSepLoc == std::string::npos)
    {
        return dir;
    }
    return dir.substr(0, pathSepLoc + 1);
}

void File::processShaderSource(const char* path, resource_t<Extension::TXT>& resource)
{
    std::istringstream scanner(resource);
    std::string line;
    std::string processedSource;
    std::string includeFilePath;
    resource_t<Extension::TXT> includeFileContent;
    const std::string directory = getDirectory(path);

    while (std::getline(scanner, line))
    {
        trim(line); // Needed because \r on Windows and to allow spaces/tabs before '#include'

        if (startsWith(line, includeDirective))
        {
            const char* includeFileName = &line[9];
            includeFilePath = directory + includeFileName;

            includeFileContent = File::load<Extension::TXT>(includeFilePath.c_str());
            if (File::isValid<Extension::TXT>(includeFileContent))
            {
                processShaderSource(includeFilePath.c_str(), includeFileContent);
                processedSource += includeFileContent + "\n";
            }
        }
        else
        {
            processedSource += line + "\n";
        }
    }

    resource = processedSource;
}
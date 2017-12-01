#include "File.h"
#include <algorithm>
#include <functional>
#include <sstream>

// Trim functions from http://stackoverflow.com/a/217605

static bool canSkip(char ch) { return isspace(ch) || ch == '\r'; }

static const std::string includeDirective = "#include";

inline void ltrim(std::string& s) {
    s.erase(begin(s), std::find_if_not(begin(s), end(s), canSkip));
}

inline void rtrim(std::string& s) {
    s.erase(std::find_if_not(rbegin(s), rend(s), canSkip).base(), end(s));
}

inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

inline bool startsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
}

inline std::string getDirectory(const char* path) {
    std::string dir = path;
    size_t pathSepLoc = dir.find_last_of('/');
    return (pathSepLoc == std::string::npos) ? dir : dir.substr(0, pathSepLoc + 1);
}

void File::processShaderSource(const char* path, resource_t<Extension::TXT>& resource) {
    std::istringstream scanner(resource);
    std::string line, processedSource, includeFilePath;
    resource_t<Extension::TXT> includeFileContent;
    const std::string directory = getDirectory(path);

    size_t lineCount = 0;
    while (std::getline(scanner, line)) {
        ++lineCount;
        trim(line); // Needed because \r on Windows and to allow spaces/tabs before '#include'

        if (startsWith(line, includeDirective)) {
            const char* includeFileName = &line[9]; // TODO switch to tokenize
            includeFilePath = directory + includeFileName;

            includeFileContent = File::load<Extension::TXT>(includeFilePath.c_str());
            if (File::isValid<Extension::TXT>(includeFileContent)) {
                processShaderSource(includeFilePath.c_str(), includeFileContent);
                processedSource += "#line 1 \"" + includeFilePath + "\"\n"; // allow preprocessor to display errors from included file
                processedSource += includeFileContent + "\n";
                processedSource += "#line " + to_string(++lineCount) + " \"" + path + "\"\n"; // switch preprocessor back to current file
            }
        }
        else
            processedSource += line + "\n";
    }

    resource = processedSource;
}

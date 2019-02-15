#ifndef CLML_UTILS_HEADER_
#define CLML_UTILS_HEADER_

#include <string>
#include <list>
#include <fstream>
#include <map>

class ClmtUtils{
public:
    static int toInteger(std::string s);
    static void makeNumberValid(int& number, int low, int high, bool inclusive);
    static std::list<int> retrieveNumbers(std::string inputString, int totalNumber);
    static int getNumLines(std::string fileName);
    static bool hasQuality(const unsigned int& quality, const unsigned int& qualityIdentifier);
    static void initialiseOperatorTypeMap(std::map<std::string, unsigned int>& operatorTypeMap);
    static void initialiseMutantOperatorMap(std::map<std::string, std::list<std::string>>& mutantOperatorMap);
    static std::string colorString(const std::string& str, const char* const color);
    static void resolveTemplate(const std::string& templateStr, int &id, std::string& operatorStr);
    static void replaceStringPattern(std::string& originalString, const std::string& pattern, const std::string& value);
    static std::list<std::string> generateMutant(const std::string& kernelFilename, std::map<int, std::string>& mutableOperatorTemplates, const std::string& filePath);
    static bool fileEquals(const std::string& file1, const std::string& file2, const std::list<int>& lines);
    static bool fileLastLineEquals(const std::string& file1, const std::string& file2);
    static void runCommand(const std::string& command, const std::string& error_msg);
};

#endif
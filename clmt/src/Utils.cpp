#include "Utils.h"
#include "Constants.h"

#include <iostream>
#include <list>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>

int ClmtUtils::toInteger(std::string s){
    int power = 1;
    int result = 0;
    char c;
    for (int i = s.length() - 1; i >= 0; --i){
        c = s.at(i);
        if (!isdigit(c)) return 0;
        result = result + power * (c - '0');
        power *= 10;
    }
    return result;
}

void ClmtUtils::makeNumberValid(int& number, int low, int high, bool inclusive = true){
    if (!inclusive){
        low = low + 1;
        high = high - 1;
    }
    if (number < low) number = low;
    if (number > high) number = high;
}

std::list<int> ClmtUtils::retrieveNumbers(std::string inputString, int totalNumber){
    inputString = inputString + ',';
    std::list<int> numberList;
    std::string part = "";
    size_t i = 0;
    bool expectingSecondNumber = false;
    bool isNegative = false;
    int first, second;
    char c;
    while(i < inputString.length()){
        c = inputString.at(i);

        if (c == '-'){
            isNegative = true;
            ++i;
            continue;
        } else if (c == ':'){
            if (part.empty()){
                ++i;
                continue;
            }
            first = toInteger(part);
            if (isNegative) {
                first = totalNumber - first + 1;
                isNegative = false;
            }
                makeNumberValid(first, 1, totalNumber);
                expectingSecondNumber = true;
                part = "";
        } else if (c == ','){
            if (part.empty()){
                ++i;
                continue;
            }
            if (expectingSecondNumber){
                expectingSecondNumber = false;
                second = toInteger(part);
                if (isNegative) {
                    second = totalNumber - second + 1;
                    isNegative = false;
                }
                makeNumberValid(first, 1, totalNumber);
                makeNumberValid(second, 1, totalNumber);
                part = "";
                if (first == second) {
                    numberList.push_back(first);
                } else {
                    if (first > second) { //swap without introducing a new variable;
                        first = first + second;
                        second = first - second;
                        first = first - second;
                    }
            
                    while (first <= second){
                        numberList.push_back(first);
                        ++first;
                    }
                }
            } else {
                first = toInteger(part);
                if (isNegative) {
                    first = totalNumber - first + 1;
                    isNegative = false;
                }
                makeNumberValid(first, 1, totalNumber);
                numberList.push_back(first);
                part = "";
            }
        } else if (isdigit(c)){
            part = part + c;
        }
        ++i;
    }
    numberList.sort();
    numberList.unique();
    return numberList;
}

int ClmtUtils::getNumLines(std::string fileName){
    std::ifstream inFile(fileName);
    int i = std::count(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>(), '\n');
    inFile.close();
    return i;
}

bool ClmtUtils::hasQuality(const unsigned int& quality, const unsigned int& qualityIdentifier){
    return ((quality & qualityIdentifier) == qualityIdentifier)? true: false;
}

void ClmtUtils::initialiseOperatorTypeMap(std::map<std::string, unsigned int>& operatorTypeMap){
    operatorTypeMap["+B"]  = operator_type::ARITHMETIC;
    operatorTypeMap["-B"]  = operator_type::ARITHMETIC;
    operatorTypeMap["-U"]  = operator_type::ARITHMETIC;
    operatorTypeMap["*B"]   = operator_type::ARITHMETIC;
    operatorTypeMap["/B"]   = operator_type::ARITHMETIC;
    operatorTypeMap["%B"]   = operator_type::ARITHMETIC;
    operatorTypeMap["++U"]  = operator_type::ARITHMETIC;
    operatorTypeMap["--U"]  = operator_type::ARITHMETIC;
    operatorTypeMap["==B"]  = operator_type::RELATIONAL;
    operatorTypeMap["!=B"]  = operator_type::RELATIONAL;
    operatorTypeMap["<B"]   = operator_type::RELATIONAL;
    operatorTypeMap["<=B"]  = operator_type::RELATIONAL;
    operatorTypeMap[">B"]   = operator_type::RELATIONAL;
    operatorTypeMap[">=B"]  = operator_type::RELATIONAL;
    operatorTypeMap["&&B"]  = operator_type::LOGICAL;
    operatorTypeMap["||B"]  = operator_type::LOGICAL;
    operatorTypeMap["!U"]   = operator_type::LOGICAL;
    operatorTypeMap["&B"]   = operator_type::BITWISE;
    operatorTypeMap["|B"]   = operator_type::BITWISE;
    operatorTypeMap["^B"]   = operator_type::BITWISE;
    operatorTypeMap["~U"]   = operator_type::BITWISE;
    operatorTypeMap["<<B"]  = operator_type::BITWISE;
    operatorTypeMap[">>B"]  = operator_type::BITWISE;
    operatorTypeMap["=B"]   = operator_type::ASSIGNMENT;
    operatorTypeMap["+=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["-=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["*=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["/=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["%=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["<<=B"] = operator_type::ASSIGNMENT;
    operatorTypeMap[">>=B"] = operator_type::ASSIGNMENT;
    operatorTypeMap["&=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["|=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["^=B"]  = operator_type::ASSIGNMENT;
    operatorTypeMap["&U"]   = operator_type::OTHER;
    operatorTypeMap["*U"]   = operator_type::OTHER;
}

void ClmtUtils::initialiseMutantOperatorMap(std::map<std::string, std::list<std::string>>& mutantOperatorMap){
    //Arithmetic
    mutantOperatorMap["+B"] = {"-", "*", "/"};
    mutantOperatorMap["-B"] = {"+", "*", "/"};
    mutantOperatorMap["-U"] = {""};
    mutantOperatorMap["*B"] = {"/", "+", "-"};
    mutantOperatorMap["/B"] = {"*", "+", "-"};
    mutantOperatorMap["%B"] = {"*", "+", "-", "/"};
    mutantOperatorMap["++U"] = {"--"};
    mutantOperatorMap["--U"] = {"++"};
    //Relational
    mutantOperatorMap["<B"] = {">", ">=", "<="};
    mutantOperatorMap[">B"] = {"<", "<=", ">="};
    mutantOperatorMap["==B"] = {"!="};
    mutantOperatorMap["<=B"] = {">=", ">", "<"};
    mutantOperatorMap[">=B"] = {"<=", "<", ">"};
    mutantOperatorMap["!=B"] = {"=="};
    //Logical
    mutantOperatorMap["&&B"] = {"||"};
    mutantOperatorMap["||B"] = {"&&"};
    mutantOperatorMap["!U"] = {""};
    //Bitwise
    mutantOperatorMap["&B"] = {"|"};
    mutantOperatorMap["|B"] = {"&"};
    mutantOperatorMap["^B"] = {"&"};
    mutantOperatorMap["~U"] = {""};
    mutantOperatorMap["<<B"] = {">>"};
    mutantOperatorMap[">>B"] = {"<<"};
    //Assignment
    mutantOperatorMap["+=B"] = {"-=" ,"*=", "/="};
    mutantOperatorMap["-=B"] = {"+=", "*=", "/="};
    mutantOperatorMap["*=B"] = {"/=", "+=", "-="};
    mutantOperatorMap["/=B"] = {"*=", "+=", "-="};
    mutantOperatorMap["%=B"] = {"*=", "+=", "-=", "/="};
    mutantOperatorMap["<<=B"] = {">>="};
    mutantOperatorMap[">>=B"] = {"<<="};
    mutantOperatorMap["&=B"] = {"|="};
    mutantOperatorMap["|=B"] = {"&="};
    mutantOperatorMap["^=B"] = {"&="};
}

std::string ClmtUtils::colorString(const std::string& str, const char* const color){
    return color + str + output_color::KNRM;
}

void ClmtUtils::resolveTemplate(const std::string& templateStr, int &id, std::string& operatorStr){
    int p1 = templateStr.find_first_of("_");
    int p2 = templateStr.find_last_of("_");
    int p3 = templateStr.find_last_of("}");
    std::string idStr = templateStr.substr(p1+1, p2-p1-1);
    operatorStr = templateStr.substr(p2+1, p3-p2-1);
    id = toInteger(idStr);
}

void ClmtUtils::replaceStringPattern(std::string& originalString, const std::string& pattern, const std::string& value){
    auto locBegin = originalString.find(pattern);
    if (locBegin > originalString.length()){
        std::cout << "[debug] template:" << pattern << "\n";
    }
    originalString.replace(locBegin, pattern.length(), value);
}

std::list<std::string> ClmtUtils::generateMutant(const std::string& kernelFilename, std::map<int, std::string>& mutableOperatorTemplates, const std::string& filePath){
    std::ifstream kernelFile(kernelFilename);
    std::stringstream codeStream;
    std::string code;
    codeStream << kernelFile.rdbuf();
    code = codeStream.str();
    std::map<std::string, std::list<std::string>> mutantOperatorMap;
    initialiseMutantOperatorMap(mutantOperatorMap);
    std::map<std::string, unsigned int> operatorType;
    initialiseOperatorTypeMap(operatorType);
    int mutantID = 1;
    std::list<std::string> generatedMutants;
    
    int arithemeticOperatorCount = 0;
    int relationalOperatorCount = 0;
    int logicalOperatorCount = 0;
    int bitwiseOperatorCount = 0;
    int assignmentOperatorCount = 0;
    int otherOperatorCount = 0;

    for(auto it = mutableOperatorTemplates.begin(); it!=mutableOperatorTemplates.end(); it++){
        std::string tmpCode(code);
        std::string currentTemplate = it->second;
        int currentID;
        std::string currentOperatorStr;
        resolveTemplate(currentTemplate, currentID, currentOperatorStr);
        for (auto it2 = mutableOperatorTemplates.begin(); it2!=mutableOperatorTemplates.end(); it2++){
            if (it2->first != currentID){
                std::string template2 = it2->second;
                std::string operator2Str;
                int operator2ID;
                resolveTemplate(template2, operator2ID, operator2Str);
                operator2Str = operator2Str.substr(0, operator2Str.length()-1);
                replaceStringPattern(tmpCode, template2, operator2Str);
            }
        }
        std::list<std::string> mutantOperators = mutantOperatorMap[currentOperatorStr];

        auto type = operatorType[currentOperatorStr];
        int lenMutantOperators = mutantOperators.size();
        switch (type) {
            case operator_type::ARITHMETIC:
                arithemeticOperatorCount += lenMutantOperators;
                break;
            case operator_type::RELATIONAL:
                relationalOperatorCount += lenMutantOperators;
                break;
            case operator_type::LOGICAL:
                logicalOperatorCount += lenMutantOperators;
                break;
            case operator_type::ASSIGNMENT:
                assignmentOperatorCount += lenMutantOperators;
                break;
            case operator_type::BITWISE:
                bitwiseOperatorCount += lenMutantOperators;
                break;
            case operator_type::OTHER:
                otherOperatorCount += lenMutantOperators;
                break;
        }
    

        for (auto it3 = mutantOperators.begin(); it3!=mutantOperators.end(); it3++){
            std::string codeToWrite(tmpCode);
            replaceStringPattern(codeToWrite, currentTemplate, *it3);
            std::stringstream fileNameBuilder;
            fileNameBuilder << filePath << "." << kernel_rewriter_constants::MUTANT_FILENAME_SUFFIX << mutantID;
            std::ofstream outputFile(fileNameBuilder.str());
            generatedMutants.push_back(fileNameBuilder.str());
            outputFile << codeToWrite;
            outputFile.close();
            mutantID++;
        }
    }

    mutantID--;
    std::stringstream notification;
    notification << mutantID << " mutants of this kernel has been created.";


    std::cout << "Operator Summary\n"
        << "Arithmetic: " << arithemeticOperatorCount << "\n"
        << "Relational:" << relationalOperatorCount << "\n"
        << "Logical: " << logicalOperatorCount << "\n"
        << "Bitwise: " << bitwiseOperatorCount << "\n"
        << "Assignment: " << assignmentOperatorCount << "\n"
        << "Other: " << otherOperatorCount << "\n"
        << "End of Operator Summary\n";

    std::cout << colorString(notification.str(), output_color::KBLU) << "\n";
    return generatedMutants;
}

bool ClmtUtils::fileEquals(const std::string& file1, const std::string& file2, const std::list<int>& lines){
    std::ifstream file1In(file1);
    std::ifstream file2In(file2);
    std::string line1;
    std::string line2;
    int currentLine = 1;
    auto iter = lines.begin();
    while(std::getline(file1In, line1) && std::getline(file2In, line2)){
        if (currentLine == *iter){
            if (line1.compare(line2) != 0) {
                return false;
            }
            iter++;
            if (iter == lines.end()) break;
        }
        currentLine++ ;
    }
    return true;
}

bool ClmtUtils::fileLastLineEquals(const std::string& file1, const std::string& file2){
    std::ifstream file1In(file1);
    std::ifstream file2In(file2);
    std::string line1;
    std::string line2;
    while(std::getline(file1In,line1));
    while(std::getline(file2In,line2));
    return (line1.compare(line2)==0)? true: false;
}

void ClmtUtils::runCommand(const std::string& command, const std::string& error_msg){
    if (system(command.c_str())){
        std::cout << colorString(error_msg, output_color::KRED) << std::endl;
        exit(error_code::COMMAND_EXEC_ERROR); 
    }
}

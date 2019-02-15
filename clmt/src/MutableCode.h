#ifndef CLMT_REWRITABLE_CODE_HEADER_
#define CLMT_REWRITABLE_CODE_HEADER_

#include <string>
#include <fstream>

#include "Constants.h"

class MutableCode{
public:
    MutableCode();
    MutableCode(const std::string& code);
    std::string code() const;
    void code(const std::string& code);
    
    unsigned getNumBarriers() const;
    unsigned getNumConditions() const;
    unsigned getNumExpressions() const;

    unsigned enableBarrier(const int& barrierID);
    unsigned disableBarrier(const int& barrierID);
    unsigned setBarrierArgument(const int& barrierID, const unsigned int& argumentType);
    unsigned gettBarrierArgument(const int& barrierID);

private:
    std::string sourceCode;
    int numBarriers;
    int numConditions;
    int numExpressions;
};

#endif

#ifndef CLMT_OPENCL_KERNEL_INVASTIGATOR_HEADER_
#define CLMT_OPENCL_KERNEL_INVASTIGATOR_HEADER_

#include "clang/Tooling/Tooling.h"
#include "UserConfig.h"

int parseCode(clang::tooling::ClangTool* tool, const int& numKernelsIn, std::map<std::string, std::list<std::string>>** mutantFileList);

#endif
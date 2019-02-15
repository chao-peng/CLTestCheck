#ifndef CLMT_CONSTANTS_HEADER_
#define CLMT_CONSTANTS_HEADER_

namespace kernel_rewriter_constants{
    const char* const FAKE_HEADER_MACRO = "OPENCLBC_FAKE_HEADER_FOR_LIBTOOLING_";
    const char* const CODE_TEMPLATE_FILENAME_SUFFIX = "clmt_template";
    const char* const MUTANT_FILENAME_SUFFIX = "clmt_mutant";
    const char* const CLCOV_GET_GENERAL_SIZE_FUNCTION_NAME = "ocl_get_general_size";
}

namespace error_code{
    const int STATUS_OK = 0;
    const int TWO_MANY_HOST_FILE_SUPPLIED = 1;
    const int NO_HOST_FILE_SUPPLIED = 2;
    const int REMOVE_KERNEL_FAKE_HEADER_FAILED_KERNEL_DOES_NOT_EXIST = 3;
    const int KERNEL_FILE_ALREADY_HAS_FAKE_HEADER = 4;
    const int NO_NEED_TO_TEST_COVERAGE = 5;
    const int KERNEL_FILE_NOT_PROVIDED = 6;
    const int LINES_TO_COMPARE_NOT_VALID = 7;
    const int COMMAND_EXEC_ERROR = 8;
    const int COMPARISON_FILE_NOT_PROVIDED = 9;
}

namespace barrier_type{
    const unsigned int LOCAL = 0x01;
    const unsigned int GLOBAL = 0x02;
    const unsigned int LOCAL_AND_GLOBAL = 0x03;
}

namespace operator_type{
    const unsigned int RESERVED_KEY0 = 0x0001;
    const unsigned int RESERVED_KEY1 = 0x0002;
    const unsigned int ARITHMETIC = 0x0004;
    const unsigned int RELATIONAL = 0x0008;
    const unsigned int LOGICAL = 0x0010;
    const unsigned int BITWISE = 0x0020;
    const unsigned int ASSIGNMENT = 0x0040;
    const unsigned int OTHER = 0x0080;
}

namespace output_color{
    const char* const KNRM = "\x1B[0m";
    const char* const KRED = "\x1B[31m";
    const char* const KGRN = "\x1B[32m";
    const char* const KYEL = "\x1B[33m";
    const char* const KBLU = "\x1B[34m";
    const char* const KMAG = "\x1B[35m";
    const char* const KCYN = "\x1B[36m";
    const char* const KWHT = "\x1B[37m";
}

namespace mutant_type{
    const unsigned int killed = 0x01;
    const unsigned int survived = 0x02;
}
#endif
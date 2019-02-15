set(LLVM_LINK_COMPONENTS
    Support
)
        
add_clang_executable(clmt
    src/Utils.cpp
    src/Utils.h
    src/Constants.h
    src/OpenCLKernelInvastigator.cpp
    src/OpenCLKernelInvastigator.h
    src/UserConfig.cpp
    src/UserConfig.h
    src/Main.cpp)
        
target_link_libraries(clmt PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangTooling)

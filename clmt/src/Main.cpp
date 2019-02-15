#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <map>

#include "Utils.h"
#include "Constants.h"
#include "OpenCLKernelInvastigator.h"

#include "llvm/Support/CommandLine.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

static llvm::cl::OptionCategory ToolCategory("OpenCL kernel branch coverage checker options");

static llvm::cl::opt<std::string> executableFile(
    "run",
    llvm::cl::desc("Specify the executable file"),
    llvm::cl::value_desc("executable"),
    llvm::cl::Optional
);

static llvm::cl::opt<std::string> expectedOutput(
    "compare",
    llvm::cl::desc("Specify the expected output file path"),
    llvm::cl::value_desc("filename"),
    llvm::cl::Optional
);

static llvm::cl::opt<std::string> keepMutants(
    "keep",
    llvm::cl::desc("Keep which kind (killed, survived, all, none) of generated mutants after execution."),
    llvm::cl::value_desc("type string"),
    llvm::cl::Optional
);

static llvm::cl::opt<std::string> matchLines(
    "lines",
    llvm::cl::desc("Compare certain lines of output with the expected output file"),
    llvm::cl::value_desc("number of lines"),
    llvm::cl::Optional
);

static llvm::cl::opt<std::string> userConfigFileName(
    "config",
    llvm::cl::desc("Specify the user config file name"),
    llvm::cl::value_desc("filename"),
    llvm::cl::Optional // Will be empty string if not specified
);

static llvm::cl::opt<std::string> userSpecifiedTimeout(
    "timeout",
    llvm::cl::desc("Specify the timeout of execution"),
    llvm::cl::value_desc("seconds"),
    llvm::cl::Optional // Will be empty string if not specified
);

int main(int argc, const char** argv){
    clang::tooling::CommonOptionsParser optionsParser(argc, argv, ToolCategory);

    // Set of kernel files
    std::list<std::string> kernels;
    // Expected output file
    std::string expectedOutputFilePath;
    // Executable file
    std::string executableFilePath;
    // Lines in the expected output file to match
    std::list<int> lines;

    std::list<std::string> survivedMutants;

    std::string kernelFileName;
    std::stringstream notification;
    for (auto it = optionsParser.getSourcePathList().begin(); it!=optionsParser.getSourcePathList().end();it++){
        kernelFileName = it->c_str();
        kernels.push_back(kernelFileName);
    }

    int numKernels = kernels.size();
    if (numKernels == 0){
        notification << "Please provide at least one GPU kernel source code to process.";
        std::cout << ClmtUtils::colorString(notification.str(), output_color::KRED) << "\n";
        exit(error_code::KERNEL_FILE_NOT_PROVIDED);
    }

    expectedOutputFilePath = expectedOutput.c_str();
    int totalLines = ClmtUtils::getNumLines(expectedOutputFilePath);

    executableFilePath = executableFile.c_str();
    bool autoExecute = true;
    if (executableFilePath.empty()){
        autoExecute = false;
    } else {
        if (expectedOutputFilePath.empty()){
            std::cout << ClmtUtils::colorString("Please provide the reference file to compare execution result", output_color::KRED) << "\n";
            exit(error_code::COMPARISON_FILE_NOT_PROVIDED);
        }
    }
/*
    if (completeMatch && !matchLines.empty()){
        notification << "The matching mode should either be all (-all) or part match (specify lines in the format of \"3:5,7,-1\")";
        std::cout << ClmtUtils::colorString(notification.str(), output_color::KRED) << "\n";
        exit(error_code::LINES_TO_COMPARE_NOT_VALID);
    } 
*/

    if (!matchLines.empty() && matchLines!="all"){
        lines = ClmtUtils::retrieveNumbers(matchLines.c_str(), totalLines);
    } else {
        for (int i = 1; i <= totalLines; ++i){
            lines.push_back(i);
        }
    }

    unsigned int keepMutantsType;
    if (keepMutants == "killed"){
        keepMutantsType = mutant_type::killed;
    } else if (keepMutants == "none"){
        keepMutantsType = 0;
    } else if (keepMutants == "all"){
        keepMutantsType = mutant_type::killed | mutant_type::survived;
    } else {
        keepMutantsType = mutant_type::survived;
    }

    std::string timeout = "60";
    if (!userSpecifiedTimeout.empty()){
        timeout = userSpecifiedTimeout;
    }

    UserConfig userConfig(userConfigFileName.c_str());
    for (auto itKernel = kernels.begin(); itKernel != kernels.end(); itKernel++){
        userConfig.generateFakeHeader(*itKernel);
    }
    clang::tooling::ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());
    std::map<std::string, std::list<std::string>>* mutantFileList;
    int numOperators = parseCode(&tool, numKernels, &mutantFileList);
    for (auto itKernel = kernels.begin(); itKernel != kernels.end(); itKernel++){
        UserConfig::removeFakeHeader(*itKernel);
    }
    notification << "Code invastigation summary: " << numOperators << " mutable operators in total have been found.";
    std::cout << ClmtUtils::colorString(notification.str(), output_color::KBLU) << "\n";
    notification.str("");
    
    if (autoExecute){
        notification << "Start executing with mutants";
        std::cout << ClmtUtils::colorString(notification.str(), output_color::KBLU) << "\n";

        std::string execCommand;
        int compareResult;
        unsigned int numMutants = 0;
        unsigned int numSurvivedMutants = 0;
        for (auto itKernel = mutantFileList->begin(); itKernel != mutantFileList->end(); itKernel++){
            std::string currentKernel = itKernel->first;
            std::cout << "Kernel: " << currentKernel << "\n";
            execCommand = "mv " + currentKernel + " " + currentKernel + ".clmt_backup";
            ClmtUtils::runCommand(execCommand, "Error when making back up of current kernel source code.");
            for (auto itMutant = itKernel->second.begin() ; itMutant != itKernel->second.end(); itMutant++){
                numMutants++;
                std::string currentMutant = *itMutant;
                std::cout << "  Running with mutant " << currentMutant << "\n";
                execCommand = "cp " + currentMutant + " " + currentKernel;
                ClmtUtils::runCommand(execCommand, "Error when copying current mutant to execute.");
                execCommand = "timeout " + timeout + " ";
                if (executableFilePath.substr(0,2).compare("./") == 0) execCommand = execCommand + executableFilePath + " >> clmt_mutant_result.txt";
                else execCommand = execCommand + "./" + executableFilePath + " >> clmt_mutant_result.txt";
                if (system(execCommand.c_str()) == 124){
                    std::cout << "  Timeout executing this mutant" << std::endl;
                }

                //Compare
                bool mutantIsSurvived;
                if (matchLines == "last") mutantIsSurvived = ClmtUtils::fileLastLineEquals("clmt_mutant_result.txt", expectedOutputFilePath);
                else mutantIsSurvived = ClmtUtils::fileEquals("clmt_mutant_result.txt", expectedOutputFilePath, lines);
                if (mutantIsSurvived){
                    numSurvivedMutants++;
                    survivedMutants.push_back(currentMutant);
                    if (!ClmtUtils::hasQuality(keepMutantsType, mutant_type::survived)){
                        execCommand = "rm " + currentMutant;
                        ClmtUtils::runCommand(execCommand, "Error when deleting current mutant file.");
                    }
                } else {
                    if (!ClmtUtils::hasQuality(keepMutantsType, mutant_type::killed)){
                        execCommand = "rm " + currentMutant;
                        ClmtUtils::runCommand(execCommand, "Error when deleting current mutant file.");
                    }
                }
                execCommand = "rm clmt_mutant_result.txt";
                ClmtUtils::runCommand(execCommand, "Error when deleting current execution output.");
            }
            execCommand = "mv " + currentKernel + ".clmt_backup " + currentKernel;
            ClmtUtils::runCommand(execCommand, "Error when recovering the kernel source code back up.");
        }
        
        if (survivedMutants.size()){
            std::cout << "[Survived mutants]\n";
            for (auto itSurvivedMutants = survivedMutants.begin(); itSurvivedMutants != survivedMutants.end(); itSurvivedMutants++){
                std::cout << *itSurvivedMutants << "\n";
            }
            double survivedRate = (double) numSurvivedMutants / (double) numMutants * 100.0;
            std::cout << "Survived rate: " << survivedRate << "%\n";
        } else {
            std::cout << ClmtUtils::colorString("No survived mutants",output_color::KBLU) << std::endl;
        }
    }
    

}
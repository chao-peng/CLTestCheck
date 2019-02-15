#include <string>
#include <map>
#include <list>
#include <iostream>
#include <sstream>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Core/Replacement.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/LLVM.h"

#include "OpenCLKernelInvastigator.h"
#include "Constants.h"
#include "Utils.h"

using namespace clang;
using namespace clang::tooling;

std::string codeTemplate;
std::map<std::string, std::string> templateMap;

std::map<std::string, unsigned int> operatorType;
std::map<std::string, std::list<std::string>> mutantOperator;

int currentOperator;
int numOperators;
int currentKernel;
int numKernels;
std::string currentFile;
std::map<int, std::string> mutableOperatorTemplates;
std::map<std::string, std::list<std::string>> generatedMutantList;

int counter;
ASTContext* context;

class RecursiveASTVisitorForKerlInvastigator : public RecursiveASTVisitor<RecursiveASTVisitorForKerlInvastigator>{
public:
    explicit RecursiveASTVisitorForKerlInvastigator(Rewriter &r) : myRewriter(r) {}
    
    bool VisitFunctionDecl(FunctionDecl *f) {
        std::string functionName = f->getQualifiedNameAsString();
        if (!functionName.empty() && functionName.compare(kernel_rewriter_constants::CLCOV_GET_GENERAL_SIZE_FUNCTION_NAME)==0){
            counter = 3;
        }
        return true;
    }

    bool VisitStmt(Stmt *s){
        if (isa<BinaryOperator>(s) || isa<UnaryOperator>(s)){
            SourceLocation rewrittenCodeStart= myRewriter.getSourceMgr().getFileLoc(s->getBeginLoc());
            SourceLocation rewrittenCodeEnd = myRewriter.getSourceMgr().getFileLoc(s->getEndLoc());
            SourceRange rewrittenCodeRange;
            rewrittenCodeRange.setBegin(rewrittenCodeStart);
            rewrittenCodeRange.setEnd(rewrittenCodeEnd);
                    std::cout << s->getBeginLoc().printToString(myRewriter.getSourceMgr()) << std::endl;
                                        std::cout << s->getEndLoc().printToString(myRewriter.getSourceMgr()) << std::endl;

            std::string rewrittenCode = myRewriter.getRewrittenText(rewrittenCodeRange);

            if (rewrittenCode.find("${operator")!=std::string::npos){
                return true;
            }

            const Stmt* currentStmt = s;
            auto parents = context->getParents(*currentStmt);
            while(!parents.empty()){
                if (!parents[0].get<Stmt>()) break;
                if (isa<CallExpr>(parents[0].get<Stmt>())){
                    const CallExpr* functionCall = cast<CallExpr>(parents[0].get<Stmt>());
                    std::string functionName = myRewriter.getRewrittenText(functionCall->getCallee()->getSourceRange());
                    if (functionName == "barrier"){
                        return true;
                    }
                }
                currentStmt = parents[0].get<Stmt>();
                parents = context->getParents(*currentStmt);
            }
        }
        if (isa<BinaryOperator>(s)){
            BinaryOperator* binaryOperator = cast<BinaryOperator>(s);
            std::string operatorStr = binaryOperator->getOpcodeStr().str() + "B";
            if (isMutable(operatorStr)){
                if (notRewritable(myRewriter.getSourceMgr(), binaryOperator->getOperatorLoc())) return true;
                /*
                SourceLocation startLoc = myRewriter.getSourceMgr().getFileLoc(
                    binaryOperator->getLocStart());
                SourceLocation endLoc = myRewriter.getSourceMgr().getFileLoc(
                    binaryOperator->getLocEnd());
                SourceRange newRange;
                newRange.setBegin(startLoc);
                newRange.setEnd(endLoc);

                std::cout << "[debug]" << myRewriter.getRewrittenText(newRange) << std::endl;
                */

                std::stringstream operatorTemplate;
                operatorTemplate << "${operator_" << currentOperator << "_" << operatorStr << "}";
                mutableOperatorTemplates[currentOperator] = operatorTemplate.str();
                myRewriter.ReplaceText(binaryOperator->getOperatorLoc(), operatorTemplate.str());
                currentOperator++;
                numOperators++;
            }
        } else if (isa<UnaryOperator>(s)){
            UnaryOperator* unaryOperator = cast<UnaryOperator>(s);
            std::string operatorStr = unaryOperator->getOpcodeStr(unaryOperator->getOpcode()).str() + "U";
            if (isMutable(operatorStr)){
                if (notRewritable(myRewriter.getSourceMgr(), unaryOperator->getOperatorLoc())) return true;
                /*
                SourceLocation startLoc = myRewriter.getSourceMgr().getFileLoc(
                    unaryOperator->getLocStart());
                SourceLocation endLoc = myRewriter.getSourceMgr().getFileLoc(
                    unaryOperator->getLocEnd());
                SourceRange newRange;
                newRange.setBegin(startLoc);
                newRange.setEnd(endLoc);

                std::cout << "[debug]" << myRewriter.getRewrittenText(newRange) << std::endl;
                */

                std::stringstream operatorTemplate;
                operatorTemplate << "${operator_" << currentOperator << "_" << operatorStr << "}";
                mutableOperatorTemplates[currentOperator] = operatorTemplate.str();
                myRewriter.ReplaceText(unaryOperator->getOperatorLoc(), operatorTemplate.str());
                currentOperator++;
                numOperators++;
            }
        }
        //TODO: instrument barriers 
        /*else if (isa<CallExpr>(s)){
            CallExpr *functionCall = cast<CallExpr>(s);
            SourceLocation startLoc = myRewriter.getSourceMgr().getFileLoc(
                    functionCall->getCallee()->getLocStart());
            SourceLocation endLoc = myRewriter.getSourceMgr().getFileLoc(
                    functionCall->getCallee()->getLocEnd());
            SourceRange newRange;
            newRange.setBegin(startLoc);
            newRange.setEnd(endLoc);
            std::string functionName = myRewriter.getRewrittenText(newRange);
            std::cout << "[debug]" << functionName << "\n";
        }*/
        return true;
    }
private:
    Rewriter &myRewriter;

    unsigned int getOperatorType(const std::string& operatorStr){
        if (operatorType.find(operatorStr) == operatorType.end()){
            return 0;
        } else {
            return operatorType[operatorStr];
        }
    }

    bool isMutable(const std::string& operatorStr){
        return (mutantOperator.find(operatorStr) == mutantOperator.end())? false: true;
    }

    bool notRewritable(const SourceManager& sm, const SourceLocation& sl){
        if (counter){
            counter--;
            return true;
        }

        if (sm.isInExternCSystemHeader(sl)) return true;
        if (sm.isInSystemHeader(sl)) return true;
        if (sm.isMacroBodyExpansion(sl)) return true;
        if (sm.isMacroArgExpansion(sl)) return true;
        return false;
    }
};

class ASTConsumerForKernelInvastigator : public ASTConsumer{
public:
    ASTConsumerForKernelInvastigator(Rewriter &r): visitor(r) {}

    bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
            // Traverse the declaration using our AST visitor.
            visitor.TraverseDecl(*b);
            //(*b)->dump();
        }
        return true;
    }

private:
    RecursiveASTVisitorForKerlInvastigator visitor;
};

class ASTFrontendActionForKernelInvastigator : public ASTFrontendAction {
public:
    ASTFrontendActionForKernelInvastigator() {}

    void EndSourceFileAction() override {
        const RewriteBuffer *buffer = myRewriter.getRewriteBufferFor(myRewriter.getSourceMgr().getMainFileID());
        if (buffer == NULL){
            llvm::outs() << "Rewriter buffer is null. Cannot write in file.\n";
            return;
        }
        std::string rewriteBuffer = std::string(buffer->begin(), buffer->end());
        std::string source;
        std::string line;
        std::istringstream bufferStream(rewriteBuffer);
        while(getline(bufferStream, line)){
            source.append(line);
            source.append("\n");
        }
        
        std::string filename = currentFile + "." + kernel_rewriter_constants::CODE_TEMPLATE_FILENAME_SUFFIX;
        std::ofstream outputFileStream(filename);
        outputFileStream << source;
        outputFileStream.close();
        UserConfig::removeFakeHeader(filename);
        std::list<std::string> mutants = ClmtUtils::generateMutant(filename, mutableOperatorTemplates, currentFile);
        generatedMutantList[currentFile] = mutants;
        currentOperator = 1;
        currentKernel++;
        mutableOperatorTemplates.clear();
    }

    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci, 
        StringRef file) override {
            std::stringstream notification;
            notification << "[" << currentKernel << "/" << numKernels << "] Instrumenting " << file.str();
            currentFile = file.str();
            std::cout << ClmtUtils::colorString(notification.str(), output_color::KBLU) << "\n";
            myRewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
            context = &ci.getASTContext();
            return llvm::make_unique<ASTConsumerForKernelInvastigator>(myRewriter);
        }

private:
    Rewriter myRewriter;
};

int parseCode(clang::tooling::ClangTool* tool, const int& numKernelsIn, std::map<std::string, std::list<std::string>>** mutantFileList){
    codeTemplate = "";
    templateMap.clear();
    ClmtUtils::initialiseOperatorTypeMap(operatorType);
    ClmtUtils::initialiseMutantOperatorMap(mutantOperator);
    currentOperator = 1;
    numKernels = numKernelsIn;
    currentKernel = 1;
    numOperators=0;
    counter = 0;

    tool->run(newFrontendActionFactory<ASTFrontendActionForKernelInvastigator>().get());
    *mutantFileList = &generatedMutantList;
    return numOperators;
}
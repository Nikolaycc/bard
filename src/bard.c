#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s [program file]\n", argv[0]);
        fprintf(stderr, "ERROR: no path to program file was provided\n");
        exit(1);
    }
    char* program_filename = argv[1];

    LLVMModuleRef module = LLVMModuleCreateWithName(program_filename);

    LLVMTypeRef printf_param_types[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMTypeRef printf_return_type = LLVMFunctionType(LLVMInt32Type(), printf_param_types, 1, 0);
    LLVMValueRef printf_function = LLVMAddFunction(module, "printf", printf_return_type);

    LLVMTypeRef param_types[] = { LLVMInt32Type(), LLVMInt32Type() };
    LLVMTypeRef return_type = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
    LLVMValueRef main_function = LLVMAddFunction(module, "main", return_type);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_function, "entry");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMValueRef str = LLVMBuildGlobalStringPtr(builder, "Hello, World!\n", "hello_str");

    LLVMValueRef printf_args[] = { str };
    LLVMBuildCall2(builder, printf_return_type, printf_function, printf_args, 1, "printf_call");

    LLVMValueRef lhs = LLVMGetParam(main_function, 0);
    LLVMValueRef rhs = LLVMGetParam(main_function, 1);
    LLVMValueRef sum = LLVMBuildAdd(builder, lhs, rhs, "sum");

    LLVMBuildRet(builder, sum);

    char *error = NULL;
    if (LLVMVerifyModule(module, LLVMAbortProcessAction, &error)) {
        fprintf(stderr, "Module verification failed: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    LLVMDumpModule(module);
    
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    LLVMTargetRef target;
    const char *triple = LLVMGetDefaultTargetTriple();
    if (LLVMGetTargetFromTriple(triple, &target, &error) != 0) {
        fprintf(stderr, "Error getting target from triple: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(
        target, 
        triple, 
        "", 
        "", 
        LLVMCodeGenLevelDefault, 
        LLVMRelocPIC,
        LLVMCodeModelDefault
    );

    if (!target_machine) {
        fprintf(stderr, "Error creating target machine\n");
        exit(1);
    }

    LLVMSetTarget(module, triple);
    
    if (LLVMTargetMachineEmitToFile(target_machine, module, "output.o", LLVMObjectFile, &error) != 0) {
        fprintf(stderr, "Error emitting file: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

    char *const argv_command[] = { "clang", "output.o", "-o", program_filename, "-lm", NULL };
    execvp("clang", argv_command);
    
    return 0;
}

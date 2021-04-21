#include <roblang/stack.h>
#include <roblang/reloc.h>
#include <roblang/codegen.h>
#include <roblang/program.h>
#include <roblang/symtable.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

Program program = { NULL, 0 };

void codegenBinop(Node *node) {
  BinopType type = node->content.binopNode->type;
  if (type == BINOP_ASGN) {
    // Codegen the right-hand side and put the value with the name in the scope.
    // Allocate on the stack
    if (node->content.binopNode->RHS->type == NODE_NUMBER) {
      uint8_t offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
      appendToProgram(program, (unsigned char)0xc7, 0);
      appendToProgram(program, (unsigned char)0x45, 0);
      appendToProgram(program, offset, 1);
      uint32_t RHS = node->content.binopNode->RHS->content.numberNode->value;
      appendToProgram(program, RHS, 0);
    } else if (node->content.binopNode->RHS->type == NODE_IDENTIFIER) {
      uint8_t offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
      codegenVariable(node->content.binopNode->RHS, REGISTER_EAX);
      appendToProgram(program, (unsigned char)0x89, 0);
      appendToProgram(program, (unsigned char)0x45, 0);
      appendToProgram(program, offset, 1);
    } else if (node->content.binopNode->RHS->type == NODE_FUNCTION) {
      codegenFunction(node->content.binopNode->RHS, node->content.binopNode->LHS->content.identifierNode->name);
    } else if (node->content.binopNode->RHS->type == NODE_FUNCTIONCALL) {
      Offset offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
      codegenFunctionCall(node->content.binopNode->RHS);
      appendToProgram(program, (unsigned char)0x89, 0);
      appendToProgram(program, (unsigned char)0x45, 0);
      appendToProgram(program, offset, 1);
    } else if (node->content.binopNode->RHS->type == NODE_BINOP) {
      // First codegen the rhs, because = has the lowest presedence
      codegenBinop(node->content.binopNode->RHS);
      // The result is expected in eax, so assign the value of eax to a new stack place
      uint8_t offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
      appendToProgram(program, (unsigned char)0x89, 0);
      appendToProgram(program, (unsigned char)0x45, 0);
      appendToProgram(program, offset, 1);
    }
  } else if (type == BINOP_BNOT) {
    // BNOT doesn't have a LHS
    if (node->content.binopNode->RHS->type == NODE_BINOP) {
      codegenBinop(node->content.binopNode->RHS);
    } else if (node->content.binopNode->RHS->type == NODE_IDENTIFIER) {
      codegenVariable(node->content.binopNode->RHS, REGISTER_EAX);
    } else {
      appendToProgram(program, (unsigned char)0xb8, 0);
      appendToProgram(program, node->content.binopNode->RHS->content.numberNode->value, 0);
    }
    appendToProgram(program, (uint16_t)0x83f0, 1);
    appendToProgram(program, (unsigned char)0xff, 0);         
  } else {
    
    // Algo design:
    // (a - b) - c()
    // mov  eax, a
    // sub  eax, b
    // ---------------------------
    // Function call encounter: 
    // put eax onto stack
    // call c
    // mov ecx, eax
    // mov eax, stack-pushed value
    // ---------------------------
    // sub eax, ecx

    // (a - b) - c
    // mov eax, a
    // sub eax, b
    // sub eax, c

    // a * b + c
    // mov eax, a
    // mul eax, b
    // add eax, c

    // a + b * c
    // mov eax, a
    // ---------------------------
    // Binop encounter:
    // put eax onto stack
    // mov eax, b
    // mul eax, c
    // mov ecx, eax
    // mov eax, stack-pushed value
    // ---------------------------
    // add eax, ecx

    // LHS (moved into EAX)
    switch (node->content.binopNode->LHS->type) {
      case NODE_BINOP: {
        // Result is put into EAX by the function                 
        codegenBinop(node->content.binopNode->LHS);
      } break;
      case NODE_NUMBER: {
        appendToProgram(program, (unsigned char)0xb8, 0);
        appendToProgram(program, node->content.binopNode->LHS->content.numberNode->value, 0);
      } break;
      case NODE_IDENTIFIER: {
        codegenVariable(node->content.binopNode->LHS, REGISTER_EAX);
      } break;
      case NODE_FUNCTIONCALL: {
        codegenFunctionCall(node->content.binopNode->LHS);
      } break; 
      default: // TODO: error
      break;                        
    }     



    switch (node->content.binopNode->type) {
      case BINOP_ADD: {
        switch (node->content.binopNode->RHS->type) {
          case NODE_BINOP: {
            // Move result of left side onto stack and codegen the rhs and add them up together, remove the value from the stack
            uint8_t offset = stack_allocate("tmp"); // TODO: check if this has not been used yet
            appendToProgram(program, (unsigned char)0x89, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            codegenBinop(node->content.binopNode->RHS);
            appendToProgram(program, (unsigned char)0x03, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            // stack_deallocate(); // FIXME: better stack allocation functions
          } break;
          case NODE_NUMBER: {
            appendToProgram(program, (unsigned char)0x05, 0);
            appendToProgram(program, node->content.binopNode->RHS->content.numberNode->value, 0);
          } break;
          case NODE_IDENTIFIER: {
            codegenVariable(node->content.binopNode->RHS, REGISTER_EBX);
            appendToProgram(program, (uint16_t)0x01d8, 1);
          } break;                  
          // TODO: add support for function call
          case NODE_FUNCTIONCALL: {
            Offset offset = stack_allocate("temp");
            appendToProgram(program, (uint16_t)0x8945, 1);
            appendToProgram(program, offset, 0);
            codegenFunctionCall(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            appendToProgram(program, (uint16_t)0x01c8, 1);
          } break;
          default: // TODO: error
          break;                        
        }
      } break;

      case BINOP_MUL: {
        switch (node->content.binopNode->RHS->type) {
          case NODE_BINOP: {
            // Move result of left side onto stack and codegen the rhs and add them up together, remove the value from the stack
            uint8_t offset = stack_allocate("tmp"); // TODO: check if this has not been used yet
            appendToProgram(program, (unsigned char)0x89, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            codegenBinop(node->content.binopNode->RHS);
            appendToProgram(program, (unsigned char)0x0f, 0);
            appendToProgram(program, (unsigned char)0xaf, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            // stack_deallocate();
          } break;
          case NODE_NUMBER: {
            appendToProgram(program, (unsigned char)0x69, 0);
            appendToProgram(program, (unsigned char)0xc0, 0);
            appendToProgram(program, node->content.binopNode->RHS->content.numberNode->value, 0);
          } break;                
          case NODE_IDENTIFIER: {
            Variable *var = findInStack(node->content.binopNode->RHS->content.identifierNode->name);                      
            appendToProgram(program, (unsigned char)0xf7, 0);
            appendToProgram(program, (unsigned char)0x65, 0);
            appendToProgram(program, var->offset, 0);
          } break;                  
          case NODE_FUNCTIONCALL: {
            Offset offset = stack_allocate("temp");
            appendToProgram(program, (uint16_t)0x8945, 1);
            appendToProgram(program, offset, 0);
            codegenFunctionCall(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            appendToProgram(program, (uint16_t)0x0faf, 1);
            appendToProgram(program, (unsigned char)0xc1, 0);
          } break;
          default: // TODO: error
          break;                        
        }
      } break;

      case BINOP_SUB: {
        switch (node->content.binopNode->RHS->type) {
          case NODE_BINOP: {
            // move lhs onto stack, codegen the rhs, move lhs from stack into ebx and subtract eax from that and move result into eax
            uint8_t offset = stack_allocate("tmp"); // TODO: check if this has not been used yet
            appendToProgram(program, (unsigned char)0x89, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            codegenBinop(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x8b5d, 1);
            appendToProgram(program, offset, 0);
            // stack_deallocate(); FIXME
            appendToProgram(program, (uint16_t)0x29c3, 1);
            appendToProgram(program, (uint16_t)0x89d8, 1);
          } break;
          case NODE_NUMBER: {
            appendToProgram(program, (unsigned char)0x2d, 0);
            appendToProgram(program, node->content.binopNode->RHS->content.numberNode->value, 0);
          } break;                
          case NODE_IDENTIFIER: {
            Variable *var = findInStack(node->content.binopNode->RHS->content.identifierNode->name);                      
            appendToProgram(program, (unsigned char)0x2b, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, var->offset, 0);
          } break;                  
          case NODE_FUNCTIONCALL: {
            Offset offset = stack_allocate("temp");
            appendToProgram(program, (uint16_t)0x8945, 1);
            appendToProgram(program, offset, 0);
            codegenFunctionCall(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            appendToProgram(program, (uint16_t)0x29c8, 1);
          } break;
          default: // TODO: error
          break;                        
        }
      } break;

      case BINOP_DIV: {
        switch (node->content.binopNode->RHS->type) {
          case NODE_BINOP: {
            // move lhs onto stack, codegen the rhs, move lhs from stack into ebx and subtract eax from that and move result into eax
            uint8_t offset = stack_allocate("tmp"); // TODO: check if this has not been used yet
            appendToProgram(program, (unsigned char)0x89, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            codegenBinop(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c3, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            stack_deallocate();
            appendToProgram(program, (uint16_t)0x31d2, 1);
            appendToProgram(program, (uint16_t)0xf7f3, 1);
          } break;
          case NODE_NUMBER: {
            appendToProgram(program, (unsigned char)0xbb, 0);
            appendToProgram(program, node->content.binopNode->RHS->content.numberNode->value, 0);
            appendToProgram(program, (uint16_t)0x31d2, 1);
            appendToProgram(program, (uint16_t)0xf7f3, 1);
          } break;                
          case NODE_IDENTIFIER: {
            Variable *var = findInStack(node->content.binopNode->RHS->content.identifierNode->name);                      
            appendToProgram(program, (uint16_t)0x31d2, 1);
            appendToProgram(program, (uint16_t)0xf775, 1);
            appendToProgram(program, var->offset, 0);
          } break;                  
          case NODE_FUNCTIONCALL: {
            Offset offset = stack_allocate("temp");
            appendToProgram(program, (uint16_t)0x8945, 1);
            appendToProgram(program, offset, 0);
            codegenFunctionCall(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            appendToProgram(program, (uint16_t)0x31d2, 1);
            appendToProgram(program, (uint16_t)0xf7f1, 1);
          } break;
          default: // TODO: error
          break;                        
        }
      } break;

      case BINOP_BSR:
      case BINOP_BSL: {
        switch (node->content.binopNode->RHS->type) {
          case NODE_BINOP: {
            uint8_t offset = stack_allocate("tmp"); // TODO: check if this has not been used yet
            appendToProgram(program, (unsigned char)0x89, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            codegenBinop(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            stack_deallocate();
            // SHR or SHL
            if (node->content.binopNode->type == BINOP_BSL) 
              appendToProgram(program, (uint16_t)0xd3e0, 1);
            else
              appendToProgram(program, (uint16_t)0xd3f8, 1);
          } break;

          case NODE_IDENTIFIER: {
            codegenVariable(node->content.binopNode->RHS, REGISTER_ECX);
            if (node->content.binopNode->type == BINOP_BSR)
              appendToProgram(program, (uint16_t)0xd3e0, 1);
            else
              appendToProgram(program, (uint16_t)0xd3f8, 1);
          } break;
          case NODE_NUMBER: {
            appendToProgram(program, (unsigned char)0xb9, 0);
            appendToProgram(program, (uint32_t)node->content.binopNode->RHS->content.numberNode->value, 0);
            if (node->content.binopNode->type == BINOP_BSL)
              appendToProgram(program, (uint16_t)0xd3e0, 1);
            else
              appendToProgram(program, (uint16_t)0xd3f8, 1);
          } break;
          case NODE_FUNCTIONCALL: {
            Offset offset = stack_allocate("temp");
            appendToProgram(program, (uint16_t)0x8945, 1);
            appendToProgram(program, offset, 0);
            codegenFunctionCall(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            if (node->content.binopNode->type == BINOP_BSL)
              appendToProgram(program, (uint16_t)0xd3e8, 1);
            else 
              appendToProgram(program, (uint16_t)0xd3e0, 1);
          } break;
          default: // TODO: error
          break;                        
        }
      } break;                

      case BINOP_BAND:
      case BINOP_BOR: {
        switch (node->content.binopNode->RHS->type) {
          case NODE_BINOP: {
            uint8_t offset = stack_allocate("tmp"); // TODO: check if this has not been used yet
            appendToProgram(program, (unsigned char)0x89, 0);
            appendToProgram(program, (unsigned char)0x45, 0);
            appendToProgram(program, offset, 1);
            codegenBinop(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            stack_deallocate();
            // and or or
            if (node->content.binopNode->type == BINOP_BAND)
              appendToProgram(program, (uint16_t)0x21c8, 1);
            else
              appendToProgram(program, (uint16_t)0x09c8, 1);
          } break;

          case NODE_IDENTIFIER: {
            Variable *var = findInStack(node->content.binopNode->RHS->content.identifierNode->name);                      
            if (node->content.binopNode->type == BINOP_BAND)
              appendToProgram(program, (uint16_t)0x2345, 1);
            else
              appendToProgram(program, (uint16_t)0x0b45, 1);
            appendToProgram(program, var->offset, 0);
          } break;
          case NODE_NUMBER: {
            if (node->content.binopNode->type == BINOP_BAND)
              appendToProgram(program, (unsigned char)0x25, 1);
            else
              appendToProgram(program, (unsigned char)0x0d, 1);

            appendToProgram(program, (uint32_t)node->content.binopNode->RHS->content.numberNode->value, 0);
          } break;
          case NODE_FUNCTIONCALL: {
            Offset offset = stack_allocate("temp");
            appendToProgram(program, (uint16_t)0x8945, 1);
            appendToProgram(program, offset, 0);
            codegenFunctionCall(node->content.binopNode->RHS);
            appendToProgram(program, (uint16_t)0x89c1, 1);
            appendToProgram(program, (uint16_t)0x8b45, 1);
            appendToProgram(program, offset, 0);
            if (node->content.binopNode->type == BINOP_BAND)
              appendToProgram(program, (uint16_t)0x21c8, 1);
            else 
              appendToProgram(program, (uint16_t)0x09c8, 1);
          } break;
          default: // TODO: error
          break;                        
        }
      } break;                
      default: // TODO: error
      break;                        
    }
  }
}

Variable *codegenVariable(Node *node, Register r) {
  Variable *var = findInStack(node->content.identifierNode->name);
  if (var == NULL) return NULL; // TODO: Give error
  if (r == REGISTER_EAX) {
    appendToProgram(program, (unsigned char)0x8b, 0);
    appendToProgram(program, (unsigned char)0x45, 0);
    appendToProgram(program, var->offset, 0);
    return NULL; 
  } else if (r == REGISTER_EBX) {
    appendToProgram(program, (unsigned char)0x8b, 0);
    appendToProgram(program, (unsigned char)0x5d, 0);
    appendToProgram(program, var->offset, 0);
    return NULL;
  } else if (r == REGISTER_ECX) {
    appendToProgram(program, (uint16_t)0x8b4d, 1);
    appendToProgram(program, var->offset, 0);
  } else if (r == REGISTER_NONE) {
    return var;
  }
  return NULL;
}

static const Register argRegisters[] = { REGISTER_EDI, REGISTER_ESI, REGISTER_EDX, REGISTER_ECX, REGISTER_R8D, REGISTER_R9D };

void codegenArguments(FunctionCallNode *node) {
  const int registers = sizeof(argRegisters) / sizeof(argRegisters[0]);

  // if there are functions in argument list, do these first (functions are annoying things)
  Offset functionCallReturns[10];
  unsigned char functionCallIndex = 0;
  for (int i = 0; i < node->argsCount; i++) {
    if (node->args[i]->type == NODE_FUNCTIONCALL || node->args[i]->type == NODE_BINOP) {
      codegenNextTop(node->args[i]);
      if (i < registers) {
        Offset offset = stack_allocate("tmp");
        functionCallReturns[functionCallIndex++] = offset;
        appendToProgram(program, (unsigned char)0x89, 0);
        appendToProgram(program, (unsigned char)0x45, 0);
        appendToProgram(program, offset, 1);
      } else {
        // Move onto stack
        if (i - registers == 0) {
          appendToProgram(program, (uint16_t)0x8904, 1);
          appendToProgram(program, (unsigned char)0x24, 0);
        } else {
          appendToProgram(program, (uint16_t)0x8944, 1);
          appendToProgram(program, (unsigned char)0x24, 0);
          const unsigned char offset = (i - registers) * 8;
          appendToProgram(program, offset, 0);
        }
      }
    }
  }

  functionCallIndex = 0;
  for (int i = 0; i < node->argsCount; i++) {
    if (node->args[i]->type != NODE_FUNCTIONCALL && node->args[i]->type != NODE_BINOP) {
      if (i < registers) {
        if (node->args[i]->type == NODE_NUMBER) {
          codegenNumber(node->args[i], argRegisters[i]);
        } else {
          codegenNextTop(node->args[i]);
          switch (argRegisters[i]) {
            case REGISTER_EDI: { appendToProgram(program, (uint16_t)0x89c7, 1); } break;
            case REGISTER_ESI: { appendToProgram(program, (uint16_t)0x89c6, 1); } break;
            case REGISTER_EDX: { appendToProgram(program, (uint16_t)0x89c2, 1); } break;
            case REGISTER_ECX: { appendToProgram(program, (uint16_t)0x89c1, 1); } break;
            case REGISTER_R8D: { appendToProgram(program, (uint16_t)0x4189, 1); appendToProgram(program, (unsigned char)0xc0, 0); } break;
            case REGISTER_R9D: { appendToProgram(program, (uint16_t)0x4189, 1); appendToProgram(program, (unsigned char)0xc1, 0); } break;
            default: break;
          }
        }
      } else {
        codegenNextTop(node->args[i]);
        // Move onto stack
        if (i - registers == 0) {
          appendToProgram(program, (uint16_t)0x8904, 1);
          appendToProgram(program, (unsigned char)0x24, 0);
        } else {
          appendToProgram(program, (uint16_t)0x8944, 1);
          appendToProgram(program, (unsigned char)0x24, 0);
          const unsigned char offset = (i - registers) * 8;
          appendToProgram(program, offset, 0);
        }
      }
    } else {
      if (i < registers) {
        switch (argRegisters[i]) {
          case REGISTER_EDI: { appendToProgram(program, (uint16_t)(0x8b7d), 1); appendToProgram(program, functionCallReturns[functionCallIndex++], 0); } break;
          case REGISTER_ESI: { appendToProgram(program, (uint16_t)(0x8b75), 1); appendToProgram(program, functionCallReturns[functionCallIndex++], 0); } break;
          case REGISTER_ECX: { appendToProgram(program, (uint16_t)(0x8b4d), 1); appendToProgram(program, functionCallReturns[functionCallIndex++], 0); } break;
          case REGISTER_EDX: { appendToProgram(program, (uint16_t)(0x8b55), 1); appendToProgram(program, functionCallReturns[functionCallIndex++], 0); } break;
          case REGISTER_R8D: { appendToProgram(program, (uint16_t)(0x448b), 1); appendToProgram(program, (unsigned char)(0x45), 0); appendToProgram(program, functionCallReturns[functionCallIndex++], 0); } break;
          case REGISTER_R9D: { appendToProgram(program, (uint16_t)(0x448b), 1); appendToProgram(program, (unsigned char)(0x4d), 0); appendToProgram(program, functionCallReturns[functionCallIndex++], 0); } break;
          default: break;
        }
      }
    }
  }
}

void codegenFunctionCall(Node *node) {
  int symbol = findInSymTable(node->content.functionCallNode->function->content.identifierNode->name);
  codegenArguments(node->content.functionCallNode);
  if (symbol != -1) { // Symbol in this file
    appendToProgram(program, (unsigned char)0xe8, 1); appendToProgram(program, (uint32_t)0x00000000, 0);
    insertIntoRelocTable(program.size - 4, symbol);
  } else { // Undefined symbol
    int index = addEntryToSymTable(node->content.functionCallNode->function->content.identifierNode->name, 0, SYMBOL_UNDEFINED);
    appendToProgram(program, (unsigned char)0xe8, 1); appendToProgram(program, (uint32_t)0x00000000, 0);
    insertIntoRelocTable(program.size - 4, index);
  }
}

void codegenNumber(Node *node, Register r) {
  switch (r) {
    case REGISTER_EAX: { appendToProgram(program, (unsigned char)0xb8, 0); } break;
    case REGISTER_ESI: { appendToProgram(program, (unsigned char)0xbe, 0); } break;
    case REGISTER_EDI: { appendToProgram(program, (unsigned char)0xbf, 0); } break;
    case REGISTER_ECX: { appendToProgram(program, (unsigned char)0xb9, 0); } break;
    case REGISTER_EDX: { appendToProgram(program, (unsigned char)0xba, 0); } break;
    case REGISTER_R8D: { appendToProgram(program, (uint16_t)0x41b8, 1); } break;
    case REGISTER_R9D: { appendToProgram(program, (uint16_t)0x41b9, 1); } break;
    default: { appendToProgram(program, (unsigned char)0xb8, 0); } break;
  }
  appendToProgram(program, node->content.numberNode->value, 0);
}

void codegenNextTop(Node *node) {
  if (node->type == NODE_BINOP) codegenBinop(node);
  else if (node->type == NODE_FUNCTIONCALL) codegenFunctionCall(node);
  else if (node->type == NODE_NUMBER) codegenNumber(node, REGISTER_EAX);
  else if (node->type == NODE_FUNCTION) codegenFunction(node, NULL);
  else if (node->type == NODE_IDENTIFIER) codegenVariable(node, REGISTER_EAX);
  else {
    printf("hmm unknown top node type");
    abort();
  }
}

static int lastAnonymousFunctionIndex = 0;

void codegenParameters(FunctionNode *node) {
  const unsigned char registers = sizeof(argRegisters) / sizeof(argRegisters[0]);
  for (int i = 0; i < node->paramsCount; i++) {
    if (i < registers) {
      Offset offset = stack_allocate(node->params[i]);
      switch (argRegisters[i]) {
        case REGISTER_EDI: { appendToProgram(program, (uint16_t)0x897d, 1); appendToProgram(program, offset, 0); } break;
        case REGISTER_ESI: { appendToProgram(program, (uint16_t)0x8975, 1); appendToProgram(program, offset, 0); } break;
        case REGISTER_EDX: { appendToProgram(program, (uint16_t)0x8955, 1); appendToProgram(program, offset, 0); } break;
        case REGISTER_ECX: { appendToProgram(program, (uint16_t)0x894d, 1); appendToProgram(program, offset, 0); } break;
        case REGISTER_R8D: { appendToProgram(program, (uint16_t)0x4489, 1); appendToProgram(program, (unsigned char)0x45, 0); appendToProgram(program, offset, 0); } break;
        case REGISTER_R9D: { appendToProgram(program, (uint16_t)0x4489, 1); appendToProgram(program, (unsigned char)0x4d, 0); appendToProgram(program, offset, 0); } break;
        default: break;
      }
    } else {
      // Move onto stack
      Offset offset = stack_allocate(node->params[i]);
      appendToProgram(program, (uint16_t)0x8b45, 1);
      appendToProgram(program, (unsigned char)((i - registers) * 8 + 16), 0);
    }
  }
}

void codegenFunction(Node *node, char *label) {
  if (label == NULL) {
    // generate function name
    addEntryToSymTable(label, program.size, SYMBOL_LOCAL);
  } else {
    addEntryToSymTable(label, program.size, SYMBOL_EXTERN);
  }
  appendToProgram(program, (uint32_t)0x554889e5, 1);
  appendToProgram(program, (uint32_t)0x4883ec20, 1);
  codegenParameters(node->content.functionNode);
  for (int i = 0; i < node->content.functionNode->bodyCount; i++) {
    Node *curNode = node->content.functionNode->body[i];
    codegenNextTop(curNode);
  }
  appendToProgram(program, (uint32_t)0x4883c420, 1);
  appendToProgram(program, (uint16_t)0x5dc3, 1);
}

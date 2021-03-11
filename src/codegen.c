#include <roblang/stack.h>
#include <roblang/codegen.h>
#include <roblang/program.h>
#include <roblang/symtable.h>

#include <stdlib.h>
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
        // to be done
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
            stack_deallocate();
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
            stack_deallocate();
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
          // TODO: add support for function call
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
            stack_deallocate();
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
          // TODO: add support for function call
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
          // TODO: add support for function call
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
            // SAR or SHL
            if (node->content.binopNode->type == BINOP_BSL)
              appendToProgram(program, (uint16_t)0xd3e0, 1);
            else
              appendToProgram(program, (uint16_t)0xd3f8, 1);
          } break;

          case NODE_IDENTIFIER: {
            codegenVariable(node->content.binopNode->RHS, REGISTER_ECX);
            if (node->content.binopNode->type == BINOP_BSL)
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
              appendToProgram(program, (uint16_t)0x21d8, 1);
            else
              appendToProgram(program, (uint16_t)0x09d8, 1);
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

void codegenFunction(Node *node, char *label) {
  if (label == NULL) {
    // generate function name
    addEntryToSymTable(label, program.size);
  } else {
    addEntryToSymTable(label, program.size);
  }
  appendToProgram(program, (uint32_t)0x554889e5, 1);
  for (int i = 0; i < node->content.functionNode->bodyCount; i++) {
    Node *curNode = node->content.functionNode->body[i];
    if (curNode->type == NODE_BINOP) codegenBinop(curNode);
    else if (curNode->type == NODE_FUNCTIONCALL) continue; //codegenFunctionCall(curNode);
    else continue;
  }
  appendToProgram(program, (uint16_t)0x5dc3, 1);
}

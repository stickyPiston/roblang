#include <roblang/stack.h>
#include <roblang/codegen.h>
#include <roblang/program.h>
#include <roblang/symtable.h>

#include <stdlib.h>
#include <inttypes.h>

Program program = { NULL, 0 };


void codegenBinop(Node *node) {
  switch (node->content.binopNode->type) {
    case BINOP_ASGN: {
      // Codegen the right-hand side and put the value with the name in the scope.
      // Allocate on the stack
      if (node->content.binopNode->RHS->type == NODE_NUMBER) {
        uint16_t offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
        appendToProgram(program, (unsigned char)0xc7, 0);
        appendToProgram(program, offset, 1);
        uint32_t RHS = node->content.binopNode->RHS->content.numberNode->value;
        appendToProgram(program, RHS, 0);
      } else if (node->content.binopNode->RHS->type == NODE_IDENTIFIER) {
        uint16_t offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
        codegenVariable(node->content.binopNode->RHS, REGISTER_EAX);
        appendToProgram(program, (unsigned char)0x89, 0);
        appendToProgram(program, offset, 1);
      } else if (node->content.binopNode->RHS->type == NODE_FUNCTION) {
        codegenFunction(node->content.binopNode->RHS, node->content.binopNode->LHS->content.identifierNode->name);
      } else if (node->content.binopNode->RHS->type == NODE_FUNCTIONCALL) {

      } else if (node->content.binopNode->RHS->type == NODE_BINOP) {
        // First codegen the rhs, because = has the lowest presedence
        codegenBinop(node->content.binopNode->RHS);
        // The result is expected in eax, so assign the value of eax to a new stack place
        uint16_t offset = stack_allocate(node->content.binopNode->LHS->content.identifierNode->name);
        appendToProgram(program, (unsigned char)0x89, 0);
        appendToProgram(program, offset, 1);
      }
    } break;

    case BINOP_ADD: {
      if (node->content.binopNode->LHS->type == NODE_IDENTIFIER) {
        codegenVariable(node->content.binopNode->LHS, REGISTER_EAX);
        if (node->content.binopNode->RHS->type == NODE_IDENTIFIER) {
          appendToProgram(program, (unsigned char)0x03, 0);
          Variable *var = findInStack(node->content.binopNode->RHS->content.identifierNode->name);
          appendToProgram(program, var->offset, 1);
        } else if (node->content.binopNode->RHS->type == NODE_NUMBER) {
          appendToProgram(program, (unsigned char)0x05, 0);
          appendToProgram(program, (uint32_t)node->content.binopNode->RHS->content.numberNode->value, 0);
        }
      } else if (node->content.binopNode->LHS->type == NODE_NUMBER) {
        if (node->content.binopNode->RHS->type == NODE_NUMBER) {
          // two numbers can be added up statically, and the result stored in eax immediately
          appendToProgram(program, (unsigned char)0xb8, 0);
          uint32_t result = node->content.binopNode->LHS->content.numberNode->value + node->content.binopNode->RHS->content.numberNode->value;
          appendToProgram(program, result, 0);
        }
      }
    } break;

    default:
      break;
  }
}

void codegenVariable(Node *node, Register r) {
  Variable *var = findInStack(node->content.identifierNode->name);
  if (var == NULL) return; // TODO: Give error
  if (r == REGISTER_EAX) {
    appendToProgram(program, (unsigned char)0x8b, 0);
    appendToProgram(program, var->offset, 1);
    return; 
  }
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <roblang/parser.h>

extern int lexIndex;
extern int row, col;
Chunk *prevChunk = NULL;
char lastDelimiter;

BinopType stringToBinopType(char *string) {
  switch (string[0]) {
    case '+':
      return BINOP_ADD;
    case '-':
      return BINOP_SUB;
    case '*': {
      if (prevChunk == NULL || prevChunk->type == CHUNK_BINOP)
        return BINOP_VAL;
      else
        return BINOP_MUL;
    }
      
    case '/':
      return BINOP_DIV;
    case '~':
      return BINOP_BNOT;
    case '>': {
      if (strlen(string) == 1)
        return BINOP_GT;
      else if (string[1] == '=')
        return BINOP_GTE;
      else
        return BINOP_BSR;
    }
    case '<': {
      if (strlen(string) == 1)
        return BINOP_LT;
      else if (string[1] == '=')
        return BINOP_LTE;
      else
        return BINOP_BSL;
    }
    case '=': {
      if (strlen(string) == 1)
        return BINOP_ASGN;
      else
        return BINOP_EQ;
    }
    case '!': {
      if (strlen(string) == 1)
        return BINOP_NOT;
      else
        return BINOP_NEQ;
    }
    case '&': {
      if (strlen(string) == 1)
        return BINOP_BAND;
      else
        return BINOP_AND;
    }
    case '|': {
      if (strlen(string) == 1)
        return BINOP_BOR;
      else
        return BINOP_OR;
    }
    default:
      break;
  }

  return BINOP_NONE;
}

NodeType convertChunkType(ChunkType type) {
  switch (type) {
    case CHUNK_FUNCTION: return NODE_FUNCTION;
    case CHUNK_FUNCTIONCALL: return NODE_FUNCTIONCALL;
    case CHUNK_IDENTIFIER: return NODE_IDENTIFIER;
    case CHUNK_NUMBER: return NODE_NUMBER;
    case CHUNK_STRINGLITERAL: return NODE_STRINGLITERAL;
    case CHUNK_BINOP: return NODE_BINOP;

    default: return NODE_NONE;
  }
}

Chunk *parseNextChunk() {
  Token *curTok = lexNextToken();
  switch (curTok->type) {
    case TOKEN_NUMBER: {
      Chunk *chunk = malloc(sizeof(Chunk));
      NumberChunk *numberChunk = malloc(sizeof(NumberChunk));
      *numberChunk = (NumberChunk){ .value = atoi(curTok->value) };
      *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_NUMBER, .content.numberChunk = numberChunk };
      return chunk;
    }

    case TOKEN_IDENTIFIER: {
      Chunk *chunk = malloc(sizeof(Chunk));
      IdentifierChunk *identifierChunk = malloc(sizeof(IdentifierChunk));
      *identifierChunk = (IdentifierChunk){ .name = curTok->value };
      *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_IDENTIFIER, .content.identifierChunk = identifierChunk };
      return chunk;
    }

    case TOKEN_OPERATOR: {
      Chunk *chunk = malloc(sizeof(Chunk));
      BinopChunk *binopChunk = malloc(sizeof(BinopChunk));
      *binopChunk = (BinopChunk){ .type = stringToBinopType(curTok->value) };
      *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_BINOP, .content.binopChunk = binopChunk };
      return chunk;
    }

    case TOKEN_STRINGLITERAL: {
      Chunk *chunk = malloc(sizeof(Chunk));
      StringLiteralChunk *stringLiteralChunk = malloc(sizeof(StringLiteralChunk));
      *stringLiteralChunk = (StringLiteralChunk){ .value = curTok->value };
      *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_STRINGLITERAL, .content.stringLiteralChunk = stringLiteralChunk };
      return chunk; 
    }

    case TOKEN_DELIMITER: {
      Chunk *chunk = malloc(sizeof(Chunk));
      DelimiterChunk *delimiterChunk = malloc(sizeof(DelimiterChunk));
      *delimiterChunk = (DelimiterChunk){ .delimiter = curTok->value[0] };
      *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_DELIMITER, .content.delimiterChunk = delimiterChunk };
      return chunk; 
    }

    case TOKEN_BRACKET: {
      if (prevChunk != NULL && (prevChunk->type == CHUNK_FUNCTION || prevChunk->type == CHUNK_IDENTIFIER || prevChunk->type == CHUNK_FUNCTIONCALL)) {
        // Function call
        Chunk *callee = prevChunk;
        Node **args = NULL;
        int argsSize = 0;
        if (strcmp(peekNextToken()->value, ")") == 0) {
          lexNextToken(); // eat )
        } else {
          while (lastDelimiter != ')') {
            if (argsSize == 0) {
              args = malloc(sizeof(*args));
              args[argsSize++] = parseNextExpression("),");
            } else {
              args = realloc(args, ++argsSize * sizeof(*args));
              args[argsSize - 1] = parseNextExpression("),");
            }
          }
        }
        Chunk *chunk = malloc(sizeof(*chunk));
        FunctionCallChunk *functionCallChunk = malloc(sizeof(*functionCallChunk));
        *functionCallChunk = (FunctionCallChunk){ .args = args, .argsCount = argsSize, .callee = callee };
        *chunk = (Chunk){ .col = callee->col, .row = callee->row, .type = CHUNK_FUNCTIONCALL, .content.functionCallChunk = functionCallChunk };
        return chunk;
      } else {
        // Function or expression in parens
        int savedIndex = lexIndex, savedCol = col, savedRow = row;
        while (strcmp(peekNextToken()->value, ")") != 0) {
          free(parseNextChunk());
        }
        lexNextToken(); // Eat )
        
        uint8_t isFunction = peekNextToken()->type == TOKEN_ARROW;
        lexIndex = savedIndex, col = savedCol, row = savedRow;
        if (isFunction) {
          // Function
          char **params = NULL;
          int paramsSize = 0;
          while (strcmp(peekNextToken()->value, ")") != 0) {
            if (paramsSize == 0) {
              Chunk *chunk = parseNextChunk();
              if (chunk->type != CHUNK_DELIMITER) {
                params = malloc(sizeof(*params));
                params[paramsSize++] = chunk->content.identifierChunk->name;
              }
            } else {
              Chunk *chunk = parseNextChunk();
              if (chunk->type != CHUNK_DELIMITER) {
                params = realloc(params, ++paramsSize * sizeof(*params));
                params[paramsSize - 1] = chunk->content.identifierChunk->name;
              }
            }
          }

          free(lexNextToken()); free(lexNextToken()); free(lexNextToken()); // eat ) -> {

          Node **body = NULL;
          int bodySize = 0;
          while (strcmp(peekNextToken()->value, "}") != 0) {
            if (bodySize == 0) {
              body = malloc(sizeof(*body));
              body[bodySize++] = parseNextExpression(";");
            } else {
              body = realloc(body, ++bodySize * sizeof(*body));
              body[bodySize - 1] = parseNextExpression(";");
            }
          }

          free(lexNextToken()); // eat }

          Chunk *chunk = malloc(sizeof(*chunk));
          FunctionChunk *functionChunk = malloc(sizeof(*functionChunk));
          *functionChunk = (FunctionChunk){ .params = params, .paramsCount = paramsSize, .body = body, .bodyLength = bodySize };
          *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_FUNCTION, .content.functionChunk = functionChunk };
          return chunk;
        } else {
          // Expression
          Node *expression = parseNextExpression(")");
          Chunk *chunk = malloc(sizeof(Chunk));
          ExpressionChunk *expressionChunk = malloc(sizeof(*expressionChunk));
          *expressionChunk = (ExpressionChunk){ .expression = expression };
          *chunk = (Chunk){ .col = curTok->col, .row = curTok->row, .type = CHUNK_EXPRESSION, .content.expressionChunk = expressionChunk };
          return chunk;
        }
      }
    }

    default:
      break;
  }

  return NULL;
}

// TODO: Add support for ^ (binary xor)
const BinopType presedenceTable[] = { BINOP_ASGN, BINOP_OR, BINOP_AND, BINOP_BOR, BINOP_BAND, BINOP_EQ, BINOP_NEQ, BINOP_GT, BINOP_GTE, BINOP_LT, BINOP_LTE, BINOP_BSL, BINOP_BSR, BINOP_ADD, BINOP_SUB, BINOP_MUL, BINOP_DIV, BINOP_NOT, BINOP_BNOT, BINOP_VAL };
const int presedenceTableSize = sizeof(presedenceTable) / sizeof(presedenceTable[0]);

Node *parseNextExpression(char *delimiter) {
  // TODO: Turn tokens into temporary nodes
  prevChunk = NULL;
  Chunk **chunks = NULL;
  int size = 0;
  // if ";" is entered, NULL is returned
  // if "hello;" is entered, hello is returned
  // if "hello() + 5;" is entered, hello(), +, 5 is returned
  int delimiterCount = strlen(delimiter);
  while (1) {
    int needToBreak = 0;
    for (int i = 0; i < delimiterCount; i++) {
      if (strcmp(peekNextToken()->value, (char[]){delimiter[i], '\0'}) == 0) {
        needToBreak = 1;
        lastDelimiter = delimiter[i];
        break;
      }
    }
    if (needToBreak) break;
    if (size == 0) {
      chunks = malloc(sizeof(Chunk *));
      Chunk *chunk = parseNextChunk();
      chunks[size++] = chunk;
      prevChunk = chunk;
    } else {
      Chunk *chunk = parseNextChunk();
      if (chunk->type != CHUNK_FUNCTIONCALL)
        chunks = realloc(chunks, ++size * sizeof(Chunk *));
      chunks[size - 1] = chunk;
      prevChunk = chunk;
    }
  }
  lexNextToken(); // Eat delimiter

  Node *node = arrangeChunksIntoNode(chunks, size);
  return node;
}

Node *arrangeChunksIntoNode(Chunk **chunks, int count) {
  Node *node = NULL;
  if (count == 0) return NULL;

  for (int i = 0; i < presedenceTableSize; i++) {
    for (int j = 0; j < count; j++) {
      if ((chunks[j]->type == CHUNK_BINOP && chunks[j]->content.binopChunk->type == presedenceTable[i]) || count == 1) {

        Chunk *chunk = chunks[j];
        
        node = malloc(sizeof(Node));
        *node = (Node){ .col = chunk->col, .row = chunk->row, .type = convertChunkType(chunk->type) };
        switch (chunk->type) {
          case CHUNK_BINOP: {
            BinopNode *binopNode = malloc(sizeof(*binopNode));
            Node *LHS = NULL; Node *RHS = NULL;
            if (chunk->content.binopChunk->type != BINOP_NOT && chunk->content.binopChunk->type != BINOP_BNOT && chunk->content.binopChunk->type != BINOP_VAL) {
              LHS = arrangeChunksIntoNode(chunks, j);
              RHS = arrangeChunksIntoNode(&chunks[j + 1], count - j - 1);
            } else {
              RHS = arrangeChunksIntoNode(&chunks[j + 1], 1);
            }
            *binopNode = (BinopNode){ .LHS = LHS, .RHS = RHS, .type = chunk->content.binopChunk->type };
            node->content.binopNode = binopNode;
          } break;

          case CHUNK_IDENTIFIER: {
            IdentifierNode *identifierNode = malloc(sizeof(*identifierNode));
            identifierNode->name = chunk->content.identifierChunk->name;
            node->content.identifierNode = identifierNode;
          } break;

          case CHUNK_NUMBER: {
            NumberNode *numberNode = malloc(sizeof(*numberNode));
            numberNode->value = chunk->content.numberChunk->value;
            node->content.numberNode = numberNode;
          } break;

          case CHUNK_STRINGLITERAL: {
            StringLiteralNode *stringLiteralNode = malloc(sizeof(*stringLiteralNode));
            stringLiteralNode->value = chunk->content.stringLiteralChunk->value;
            node->content.stringLiteralNode = stringLiteralNode;
          } break;

          case CHUNK_FUNCTION: {
            FunctionNode *functionNode = malloc(sizeof(*functionNode));
            *functionNode = (FunctionNode){
              .params = chunk->content.functionChunk->params,
              .paramsCount = chunk->content.functionChunk->paramsCount,
              .body = chunk->content.functionChunk->body,
              .bodyCount = chunk->content.functionChunk->bodyLength
            };
            node->content.functionNode = functionNode;
          } break;

          case CHUNK_FUNCTIONCALL: {
            FunctionCallNode *functionCallNode = malloc(sizeof(*functionCallNode));
            *functionCallNode = (FunctionCallNode){
              .args = chunk->content.functionCallChunk->args,
              .argsCount = chunk->content.functionCallChunk->argsCount,
              .function = arrangeChunksIntoNode(&chunk->content.functionCallChunk->callee, 1)
            };
            node->content.functionCallNode = functionCallNode;
          } break;

          case CHUNK_EXPRESSION: {
            free(node);
            return chunk->content.expressionChunk->expression;
          } break;
          
          default:
            return NULL;
        }

        return node;

      }
    }
  }

  return NULL;

}
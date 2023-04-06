#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "scanner.h"
#include "common/object/object.h"

#ifdef DEBUG_PRINT_CODE
#include "debug/debug.h"
#endif // DEBUG_PRINT_CODE

typedef struct {
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY
} Precedence;

typedef void(*ParseFn)(bool can_assign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static Parser parser;
static Chunk* compiling_chunk;

static Chunk* currentChunk() {
    return compiling_chunk;
}

static void errorAt(Token* token, const char* message) {
    if (parser.panic_mode) return;
    parser.panic_mode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR) {
        // nothing
    }
    else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}
static void error(const char* message) {
    errorAt(&parser.previous, message);
}
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    uint16_t constant_id = addConstant(currentChunk(), value);
    if (constant_id > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant_id;
}
static uint16_t makeConstantLong(Value value) {
    uint16_t constant_id = addConstant(currentChunk(), value);
    if (constant_id > UINT16_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return constant_id;
}

static void emitConstant(Value value) {
    uint16_t constant_id = makeConstantLong(value);
    if (constant_id <= UINT8_MAX) {
        emitByte(OP_CONSTANT);
        emitByte((uint8_t)constant_id);
    }
    else if (constant_id < UINT16_MAX) {
        emitByte(OP_CONSTANT_LONG);
        uint8_t* bytes = (uint8_t*)&constant_id;
        emitByte(bytes[0]);
        emitByte(bytes[1]);
    }
}

static void endCompiler() {
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassebleChunk(currentChunk(), "code");
    }
#endif // DEBUG_PRINT_CODE
}

static void parsePrecedence(Precedence precedence);
static void expression();
static uint8_t identifierConstant(Token* name);
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);

static void grouping(bool can_assign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool can_assign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void string(bool can_assign) {
    emitConstant(OBJ_VAL(constantString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool can_assign) {
    uint8_t arg = identifierConstant(&name);

    if (can_assign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_GLOBAL, arg);
    }
    else {
        emitBytes(OP_GET_GLOBAL, arg);
    }
}

static void variable(bool can_assign) {
    namedVariable(parser.previous, can_assign);
}

static void unary(bool can_assign) {
    TokenType operator_type = parser.previous.type;

    // compile the operand
    parsePrecedence(PREC_UNARY);

    // emit the operator instruction
    switch (operator_type) {
    case TOKEN_BANG: emitByte(OP_NOT); break;
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return; // unreachable
    }
}

static void binary(bool can_assign) {
    TokenType operator_type = parser.previous.type;
    ParseRule* rule = getRule(operator_type);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operator_type) {
    case TOKEN_BANG_EQUAL: emitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL_EQUAL: emitByte(OP_EQUAL); break;
    case TOKEN_GREATER: emitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
    case TOKEN_LESS: emitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL: emitBytes(OP_GREATER, OP_NOT); break;
    case TOKEN_PLUS: emitByte(OP_ADD); break;
    case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR: emitByte(OP_MULTILPY); break;
    case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
    default: return; // unreachable
    }
}

static void literal(bool can_assign) {
    switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    default: return; // unreachable
    }
}

static ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = { grouping, NULL, PREC_NONE },
    [TOKEN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
    [TOKEN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
    [TOKEN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
    [TOKEN_COMMA] = { NULL, NULL, PREC_NONE },
    [TOKEN_DOT] = { NULL, NULL, PREC_NONE },
    [TOKEN_MINUS] = { unary, binary, PREC_TERM },
    [TOKEN_PLUS] = { NULL, binary, PREC_TERM },
    [TOKEN_SEMICOLON] = { NULL, NULL, PREC_NONE },
    [TOKEN_SLASH] = { NULL, binary, PREC_FACTOR },
    [TOKEN_STAR] = { NULL, binary, PREC_FACTOR },
    [TOKEN_BANG] = { unary, NULL, PREC_NONE },
    [TOKEN_BANG_EQUAL] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_EQUAL] = { NULL, NULL, PREC_NONE },
    [TOKEN_EQUAL_EQUAL] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_GREATER] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_GREATER_EQUAL] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_LESS] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_LESS_EQUAL] = { NULL, binary, PREC_EQUALITY },
    [TOKEN_IDENTIFIER] = { variable, NULL, PREC_NONE },
    [TOKEN_STRING] = { string, NULL, PREC_NONE },
    [TOKEN_NUMBER] = { number, NULL, PREC_NONE },
    [TOKEN_AND] = { NULL, NULL, PREC_NONE },
    [TOKEN_CLASS] = { NULL, NULL, PREC_NONE },
    [TOKEN_ELSE] = { NULL, NULL, PREC_NONE },
    [TOKEN_FALSE] = { literal, NULL, PREC_NONE },
    [TOKEN_FOR] = { NULL, NULL, PREC_NONE },
    [TOKEN_FUN] = { NULL, NULL, PREC_NONE },
    [TOKEN_IF] = { NULL, NULL, PREC_NONE },
    [TOKEN_NIL] = { literal, NULL, PREC_NONE },
    [TOKEN_OR] = { NULL, NULL, PREC_NONE },
    [TOKEN_PRINT] = { NULL, NULL, PREC_NONE },
    [TOKEN_RETURN] = { NULL, NULL, PREC_NONE },
    [TOKEN_SUPER] = { NULL, NULL, PREC_NONE },
    [TOKEN_THIS] = { NULL, NULL, PREC_NONE },
    [TOKEN_TRUE] = { literal, NULL, PREC_NONE },
    [TOKEN_VAR] = { NULL, NULL, PREC_NONE },
    [TOKEN_WHILE] = { NULL, NULL, PREC_NONE },
    [TOKEN_ERROR] = { NULL, NULL, PREC_NONE },
    [TOKEN_EOF] = { NULL, NULL, PREC_NONE },
};

void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefix_rule = getRule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        error("Expect expression.");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule = getRule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(constantString(name->start, name->length)));
}

static uint8_t parseVariable(const char* error_message) {
    consume(TOKEN_IDENTIFIER, error_message);
    return identifierConstant(&parser.previous);
}

static void defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

ParseRule* getRule(TokenType type) {
    return &rules[type];
}

void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name");

    if (match(TOKEN_EQUAL)) {
        expression();
    }
    else {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");

    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emitByte(OP_POP);
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value");
    emitByte(OP_PRINT);
}

static void synchronize() {
    parser.panic_mode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;
        default:
            ;
        }

        advance();
    }
}

void declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    }
    else {
        statement();
    }

    if (parser.panic_mode) synchronize();
}

void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    }
    else {
        expressionStatement();
    }
}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compiling_chunk = chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    endCompiler();

    return !parser.had_error;
}
#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>

enum class TokenType {
    TYPE,
    STRUCT,
    IDENTIFIER,
    NUMBER,
    STRING,
    EQUAL,
    OPERATOR,
    COMPARISON,
    EXPRESSION,
    TERM,
    FACTOR,
    SEMICOLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    COMMA,
    DOT,
    LOOP,
    PRINT,
    IF,
    ELSE,
    RETURN,
    IMPORT,
    UNKNOWN,
    END_OF_FILE
};

enum class SymbolType {
    TOKEN,
    EXPRESSION
};

struct Token {
    TokenType type;
    std::string value;
    size_t line = 1;
    size_t column = 1;
};

struct GrammarSymbol {
    SymbolType type;
    TokenType token;
};

struct Operator {
    std::string symbol;
    int precedence;
};

struct Parameter {
    std::string type;
    std::string name;
};

struct FunctionDef {
    std::string returnType;
    std::string name;
    std::vector<Parameter> parameters;
    size_t bodyStart;
    size_t bodyEnd;
    std::vector<Token> bodyTokens;  // Store the token vector for this function
};

struct StructFieldDef {
    std::string type;
    std::string name;
    std::vector<Token> initializerTokens;
};

struct StructMethodDef {
    std::string returnType;
    std::string name;
    std::vector<Parameter> parameters;
    std::vector<Token> bodyTokens;
};

struct StructDef {
    std::string name;
    std::vector<StructFieldDef> fields;
    std::vector<StructMethodDef> methods;
};

inline Token CurrentToken;

struct Grammar {
    std::string name;
    std::vector<GrammarSymbol> order;
};

inline std::unordered_map<std::string, Grammar> GrammarRules;
inline std::unordered_map<std::string, TokenType> keywords;
inline std::vector<Operator> operators;
inline std::unordered_map<std::string, FunctionDef> functions;
inline std::unordered_map<std::string, StructDef> structDefs;

inline void registerRule(const Grammar& rule) {
    GrammarRules[rule.name] = rule;
}
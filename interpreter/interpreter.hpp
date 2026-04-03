#pragma once

#include "rules.hpp"
#include "init.hpp"
#include <cctype>
#include <iostream>
#include <string>

inline std::vector<Token> tokenize(const std::string& source) {
    std::vector<Token> tokens;
    size_t line = 1;
    size_t column = 1;

    auto pushToken = [&](TokenType type, const std::string& value, size_t tokenLine, size_t tokenColumn) {
        tokens.push_back({type, value, tokenLine, tokenColumn});
    };

    auto advance = [&](char c) {
        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    };

    auto isUnaryMinusContext = [&tokens]() {
        if (tokens.empty()) {
            return true;
        }

        const TokenType previous = tokens.back().type;
        return previous == TokenType::EQUAL ||
               previous == TokenType::OPERATOR ||
               previous == TokenType::COMPARISON ||
               previous == TokenType::LPAREN ||
               previous == TokenType::LBRACE ||
               previous == TokenType::LBRACKET ||
               previous == TokenType::COMMA ||
               previous == TokenType::SEMICOLON ||
               previous == TokenType::RETURN ||
               previous == TokenType::IMPORT;
    };
    
    size_t i = 0;
    while (i < source.length()) {
        char c = source[i];
        unsigned char uc = static_cast<unsigned char>(c);
        const size_t tokenLine = line;
        const size_t tokenColumn = column;
        
        // Skip whitespace
        if(std::isspace(uc)) {
            advance(c);
            i++;
            continue;
        }
        
        // Skip comments
        if(c == '/' && i + 1 < source.length() && source[i + 1] == '/') {
            while(i < source.length() && source[i] != '\n') {
                advance(source[i]);
                i++;
            }
            continue;
        }
        
        // String literals
        if(c == '"') {
            std::string str;
            i++;
            advance(c);
            while(i < source.length() && source[i] != '"') {
                if(source[i] == '\\' && i + 1 < source.length()) {
                    char escape = source[i + 1];
                    if(escape == 'n') str += '\n';
                    else if(escape == 't') str += '\t';
                    else if(escape == '"') str += '"';
                    else str += escape;
                    advance(source[i]);
                    i++;
                    advance(source[i]);
                    i++;
                } else {
                    str += source[i];
                    advance(source[i]);
                    i++;
                }
            }
            if(i < source.length()) {
                advance(source[i]);
                i++;
            }
            pushToken(TokenType::STRING, str, tokenLine, tokenColumn);
            continue;
        }
        
        // Identifiers and keywords
        if(std::isalpha(uc)) {
            std::string word;
            while(i < source.length() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) {
                word += source[i];
                advance(source[i]);
                i++;
            }
            
            if(keywords.find(word) != keywords.end()) {
                pushToken(keywords[word], word, tokenLine, tokenColumn);
            } else {
                pushToken(TokenType::IDENTIFIER, word, tokenLine, tokenColumn);
            }
            continue;
        }
        // Numbers
        else if(std::isdigit(uc) || (c == '-' && i + 1 < source.length() && std::isdigit(static_cast<unsigned char>(source[i + 1])) && isUnaryMinusContext())) {
            std::string num;
            if (c == '-') {
                num += c;
                advance(c);
                i++;
            }
            while(i < source.length() && (std::isdigit(static_cast<unsigned char>(source[i])) || source[i] == '.')) {
                num += source[i];
                advance(source[i]);
                i++;
            }
            pushToken(TokenType::NUMBER, num, tokenLine, tokenColumn);
            continue;
        }
        // Operators and punctuation
        else if(c == '=') {
            if(i + 1 < source.length() && source[i + 1] == '=') {
                pushToken(TokenType::COMPARISON, "==", tokenLine, tokenColumn);
                advance(c);
                i++;
                advance(source[i]);
                i++;
            } else {
                pushToken(TokenType::EQUAL, "=", tokenLine, tokenColumn);
                advance(c);
                i++;
            }
            continue;
        }
        else if(c == '!' && i + 1 < source.length() && source[i + 1] == '=') {
            pushToken(TokenType::COMPARISON, "!=", tokenLine, tokenColumn);
            advance(c);
            i++;
            advance(source[i]);
            i++;
            continue;
        }
        else if(c == '<') {
            if(i + 1 < source.length() && source[i + 1] == '=') {
                pushToken(TokenType::COMPARISON, "<=", tokenLine, tokenColumn);
                advance(c);
                i++;
                advance(source[i]);
                i++;
            } else {
                pushToken(TokenType::COMPARISON, "<", tokenLine, tokenColumn);
                advance(c);
                i++;
            }
            continue;
        }
        else if(c == '>') {
            if(i + 1 < source.length() && source[i + 1] == '=') {
                pushToken(TokenType::COMPARISON, ">=", tokenLine, tokenColumn);
                advance(c);
                i++;
                advance(source[i]);
                i++;
            } else {
                pushToken(TokenType::COMPARISON, ">", tokenLine, tokenColumn);
                advance(c);
                i++;
            }
            continue;
        }
        else if(c == ';') { pushToken(TokenType::SEMICOLON, ";", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == '(') { pushToken(TokenType::LPAREN, "(", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == ')') { pushToken(TokenType::RPAREN, ")", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == '{') { pushToken(TokenType::LBRACE, "{", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == '}') { pushToken(TokenType::RBRACE, "}", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == '[') { pushToken(TokenType::LBRACKET, "[", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == ']') { pushToken(TokenType::RBRACKET, "]", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == ',') { pushToken(TokenType::COMMA, ",", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if(c == '.') { pushToken(TokenType::DOT, ".", tokenLine, tokenColumn); advance(c); i++; continue; }
        else if((c == '+' || c == '-' || c == '*' || c == '/') && i + 1 < source.length() && source[i + 1] == '=') {
            pushToken(TokenType::EQUAL, std::string(1, c) + "=", tokenLine, tokenColumn);
            advance(c);
            i++;
            advance(source[i]);
            i++;
            continue;
        }
        else if(c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            pushToken(TokenType::OPERATOR, std::string(1, c), tokenLine, tokenColumn);
            advance(c);
            i++;
            continue;
        }
        else {
            pushToken(TokenType::UNKNOWN, std::string(1, c), tokenLine, tokenColumn);
            advance(c);
            i++;
            continue;
        }

        advance(c);
        i++;
    }
    
    tokens.push_back({TokenType::END_OF_FILE, "", line, column});
    return tokens;
}

#include "execution.hpp"

inline bool matchSymbol(const GrammarSymbol& symbol, const Token& token) {
    if(symbol.type == SymbolType::TOKEN)
        return symbol.token == token.type;

    if(symbol.type == SymbolType::EXPRESSION)
        return true;

    return false;
}

bool matchRule(const std::vector<Token>& tokens, int startIndex, const Grammar& rule) {
    // Check bounds
    if(startIndex + rule.order.size() > tokens.size()) {
        return false;
    }
    
    for(size_t i = 0; i < rule.order.size(); i++)
    {
        if(!matchSymbol(
            rule.order[i],
            tokens[startIndex + i]
        ))
            return false;
    }

    return true;
}

inline void parse(const std::vector<Token>& tokens) {
    for(size_t i = 0; i < tokens.size(); i++) {
        if(matchRule(tokens, i, GrammarRules["Declaration"])) {
            i += GrammarRules["Declaration"].order.size() - 1;
        }
    }
}

inline void runInterpreter(const std::string& source) {
    init();
    
    std::vector<Token> tokens = tokenize(source);
    parse(tokens);
    std::unordered_map<std::string, RuntimeValue> state = executeProgram(tokens);

    // for (const auto& entry : state) {
    //     std::cout << entry.first << " = " << entry.second.numberValue << "\n";
    // }
    
    // std::cout << "Interpreter completed successfully\n";
    //std::cout << "\n";
}
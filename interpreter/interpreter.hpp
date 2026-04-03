#pragma once

#include "rules.hpp"
#include "init.hpp"
#include <cctype>
#include <iostream>
#include <string>

inline std::vector<Token> tokenize(const std::string& source) {
    std::vector<Token> tokens;
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
    
    for(size_t i = 0; i < source.length(); i++) {
        char c = source[i];
        unsigned char uc = static_cast<unsigned char>(c);
        
        // Skip whitespace
        if(std::isspace(uc)) continue;
        
        // Skip comments
        if(c == '/' && i + 1 < source.length() && source[i + 1] == '/') {
            while(i < source.length() && source[i] != '\n') i++;
            continue;
        }
        
        // String literals
        if(c == '"') {
            std::string str;
            i++;
            while(i < source.length() && source[i] != '"') {
                if(source[i] == '\\' && i + 1 < source.length()) {
                    i++;
                    if(source[i] == 'n') str += '\n';
                    else if(source[i] == 't') str += '\t';
                    else if(source[i] == '"') str += '"';
                    else str += source[i];
                } else {
                    str += source[i];
                }
                i++;
            }
            if(i < source.length()) i++; // skip closing quote
            tokens.push_back({TokenType::STRING, str});
            i--;
            continue;
        }
        
        // Identifiers and keywords
        if(std::isalpha(uc)) {
            std::string word;
            while(i < source.length() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) {
                word += source[i];
                i++;
            }
            i--; // back up one since the loop will increment
            
            if(keywords.find(word) != keywords.end()) {
                tokens.push_back({keywords[word], word});
            } else {
                tokens.push_back({TokenType::IDENTIFIER, word});
            }
        }
        // Numbers
        else if(std::isdigit(uc) || (c == '-' && i + 1 < source.length() && std::isdigit(static_cast<unsigned char>(source[i + 1])) && isUnaryMinusContext())) {
            std::string num;
            if (c == '-') {
                num += c;
                i++;
            }
            while(i < source.length() && (std::isdigit(static_cast<unsigned char>(source[i])) || source[i] == '.')) {
                num += source[i];
                i++;
            }
            i--;
            tokens.push_back({TokenType::NUMBER, num});
        }
        // Operators and punctuation
        else if(c == '=') {
            if(i + 1 < source.length() && source[i + 1] == '=') {
                tokens.push_back({TokenType::COMPARISON, "=="});
                i++;
            } else {
                tokens.push_back({TokenType::EQUAL, "="});
            }
        }
        else if(c == '!' && i + 1 < source.length() && source[i + 1] == '=') {
            tokens.push_back({TokenType::COMPARISON, "!="});
            i++;
        }
        else if(c == '<') {
            if(i + 1 < source.length() && source[i + 1] == '=') {
                tokens.push_back({TokenType::COMPARISON, "<="});
                i++;
            } else {
                tokens.push_back({TokenType::COMPARISON, "<"});
            }
        }
        else if(c == '>') {
            if(i + 1 < source.length() && source[i + 1] == '=') {
                tokens.push_back({TokenType::COMPARISON, ">="});
                i++;
            } else {
                tokens.push_back({TokenType::COMPARISON, ">"});
            }
        }
        else if(c == ';') tokens.push_back({TokenType::SEMICOLON, ";"});
        else if(c == '(') tokens.push_back({TokenType::LPAREN, "("});
        else if(c == ')') tokens.push_back({TokenType::RPAREN, ")"});
        else if(c == '{') tokens.push_back({TokenType::LBRACE, "{"});
        else if(c == '}') tokens.push_back({TokenType::RBRACE, "}"});
        else if(c == '[') tokens.push_back({TokenType::LBRACKET, "["});
        else if(c == ']') tokens.push_back({TokenType::RBRACKET, "]"});
        else if(c == ',') tokens.push_back({TokenType::COMMA, ","});
        else if(c == '.') tokens.push_back({TokenType::DOT, "."});
        else if((c == '+' || c == '-' || c == '*' || c == '/') && i + 1 < source.length() && source[i + 1] == '=') {
            tokens.push_back({TokenType::EQUAL, std::string(1, c) + "="});
            i++;
        }
        else if(c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            tokens.push_back({TokenType::OPERATOR, std::string(1, c)});
        }
        else {
            tokens.push_back({TokenType::UNKNOWN, std::string(1, c)});
        }
    }
    
    tokens.push_back({TokenType::END_OF_FILE, ""});
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
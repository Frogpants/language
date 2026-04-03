#pragma once

#include "rules.hpp"

inline void SetKeywords() {
    keywords["void"] = TokenType::TYPE;
    keywords["int"] = TokenType::TYPE;
    keywords["float"] = TokenType::TYPE;
    keywords["string"] = TokenType::TYPE;
    keywords["list"] = TokenType::TYPE;
    keywords["bool"] = TokenType::TYPE;
    keywords["vec2"] = TokenType::TYPE;
    keywords["vec3"] = TokenType::TYPE;
    keywords["vec4"] = TokenType::TYPE;
    
    keywords["print"] = TokenType::PRINT;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["for"] = TokenType::LOOP;
    keywords["while"] = TokenType::LOOP;
    keywords["return"] = TokenType::RETURN;
    keywords["import"] = TokenType::IMPORT;
    keywords["true"] = TokenType::IDENTIFIER;
    keywords["false"] = TokenType::IDENTIFIER;
}

inline void SetOperators() {
    operators.push_back({"+", 10});
    operators.push_back({"-", 20});
    operators.push_back({"*", 30});
    operators.push_back({"/", 40});
    operators.push_back({"%", 50});
    operators.push_back({"^", 60});
}

inline void SetDeclarations() {
    Grammar declaration =
    {
        "Declaration",
        {
            {SymbolType::TOKEN, TokenType::TYPE},
            {SymbolType::TOKEN, TokenType::IDENTIFIER},
            {SymbolType::TOKEN, TokenType::EQUAL},
            {SymbolType::EXPRESSION, TokenType::UNKNOWN},
            {SymbolType::TOKEN, TokenType::SEMICOLON}
        }
    };

    registerRule(declaration);
}

inline void init() {
    SetKeywords();
    SetOperators();
    SetDeclarations();
}
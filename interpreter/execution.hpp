#pragma once

#include "rules.hpp"
#include "types.hpp"
#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <set>
#include <unordered_set>
#include <thread>
#include <chrono>
#include "window_backend.hpp"

struct RuntimeValue {
    GETypes::VariableType type;
    std::string typeName;
    
    // Type instances
    GETypes::IntType intVal;
    GETypes::FloatType floatVal;
    GETypes::StringType stringVal;
    GETypes::BoolType boolVal;
    GETypes::Vec2Type vec2Val;
    GETypes::Vec3Type vec3Val;
    GETypes::Vec4Type vec4Val;
    GETypes::ListType listVal;
    
    RuntimeValue() 
        : type(GETypes::VariableType::INT), typeName("int"), 
                      intVal(0), floatVal(0.0f), stringVal(""), boolVal(false),
                      vec2Val(0.0f, 0.0f), vec3Val(0.0f, 0.0f, 0.0f), 
                      vec4Val(0.0f, 0.0f, 0.0f, 0.0f), listVal() {}
    
    RuntimeValue(const std::string& typeStr, double num) {
        type = GETypes::stringToType(typeStr);
        typeName = typeStr;
        
        if (typeStr == "int") {
            intVal = GETypes::IntType(static_cast<int>(num));
        } else if (typeStr == "float") {
            floatVal = GETypes::FloatType(static_cast<float>(num));
        } else {
            intVal = GETypes::IntType(0);
        }
    }
    
    RuntimeValue(const std::string& typeStr, const std::string& str) {
        type = GETypes::stringToType(typeStr);
        typeName = typeStr;
        
        if (typeStr == "string") {
            stringVal = GETypes::StringType(str);
        }
    }
    
    // Getter for numeric value
    double getNumberValue() const {
        if (typeName == "int") return static_cast<double>(intVal.value);
        if (typeName == "float") return static_cast<double>(floatVal.value);
        if (typeName == "bool") return static_cast<double>(boolVal.value);
        return 0.0;
    }
    
    // Getter for string value
    std::string getStringValue() const {
        if (typeName == "string") return stringVal.value;
        return "";
    }
    
    // Getter for bool value
    bool getBoolValue() const {
        if (typeName == "bool") return boolVal.value;
        return false;
    }
    
    // Setter for int
    void setIntValue(int val) {
        if (typeName == "int") intVal.value = val;
    }
    
    // Setter for float
    void setFloatValue(double val) {
        if (typeName == "float") floatVal.value = static_cast<float>(val);
    }
    
    // Setter for string
    void setStringValue(const std::string& val) {
        if (typeName == "string") stringVal.value = val;
    }
    
    // Setter for bool
    void setBoolValue(bool val) {
        if (typeName == "bool") boolVal.value = val;
    }
};

inline int getOperatorPrecedence(const std::string& op) {
    for (const auto& registeredOp : operators) {
        if (registeredOp.symbol == op) {
            return registeredOp.precedence;
        }
    }
    return -1;
}

inline double applyOperator(const std::string& op, double lhs, double rhs) {
    if (op == "+") return lhs + rhs;
    if (op == "-") return lhs - rhs;
    if (op == "*") return lhs * rhs;
    if (op == "/") {
        if (rhs == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        return lhs / rhs;
    }
    if (op == "%") {
        if (rhs == 0.0) {
            throw std::runtime_error("Modulo by zero");
        }
        return std::fmod(lhs, rhs);
    }
    if (op == "^") return std::pow(lhs, rhs);

    throw std::runtime_error("Unknown operator: " + op);
}

inline bool compareValues(const std::string& op, double lhs, double rhs) {
    if (op == "==") return lhs == rhs;
    if (op == "!=") return lhs != rhs;
    if (op == "<") return lhs < rhs;
    if (op == "<=") return lhs <= rhs;
    if (op == ">") return lhs > rhs;
    if (op == ">=") return lhs >= rhs;
    throw std::runtime_error("Unknown comparison operator: " + op);
}

inline std::string formatNumber(double value) {
    std::ostringstream out;
    out << std::setprecision(15) << value;
    return out.str();
}

inline std::string formatRuntimeNumber(const RuntimeValue& val) {
    if (val.typeName == "int") {
        return std::to_string(val.intVal.value);
    }
    return formatNumber(val.getNumberValue());
}

inline bool isVectorTypeName(const std::string& typeName) {
    return typeName == "vec2" || typeName == "vec3" || typeName == "vec4";
}

inline RuntimeValue negateRuntimeValue(const RuntimeValue& value) {
    if (value.typeName == "int") {
        return RuntimeValue("int", -value.getNumberValue());
    }
    if (value.typeName == "float") {
        return RuntimeValue("float", -value.getNumberValue());
    }
    if (value.typeName == "vec2") {
        RuntimeValue out("vec2", 0.0);
        out.vec2Val = GETypes::Vec2Type(-value.vec2Val.x, -value.vec2Val.y);
        return out;
    }
    if (value.typeName == "vec3") {
        RuntimeValue out("vec3", 0.0);
        out.vec3Val = GETypes::Vec3Type(-value.vec3Val.x, -value.vec3Val.y, -value.vec3Val.z);
        return out;
    }
    if (value.typeName == "vec4") {
        RuntimeValue out("vec4", 0.0);
        out.vec4Val = GETypes::Vec4Type(-value.vec4Val.x, -value.vec4Val.y, -value.vec4Val.z, -value.vec4Val.w);
        return out;
    }

    throw std::runtime_error("Unary minus is not supported for type: " + value.typeName);
}

inline RuntimeValue makeDefaultValueForType(const std::string& typeName) {
    RuntimeValue v;
    v.typeName = typeName;
    v.type = GETypes::stringToType(typeName);
    if (typeName == "int") {
        v.intVal = GETypes::IntType(0);
    } else if (typeName == "float") {
        v.floatVal = GETypes::FloatType(0.0f);
    } else if (typeName == "string") {
        v.stringVal = GETypes::StringType("");
    } else if (typeName == "bool") {
        v.boolVal = GETypes::BoolType(false);
    } else if (typeName == "vec2") {
        v.vec2Val = GETypes::Vec2Type();
    } else if (typeName == "vec3") {
        v.vec3Val = GETypes::Vec3Type();
    } else if (typeName == "vec4") {
        v.vec4Val = GETypes::Vec4Type();
    } else if (typeName == "list") {
        v.listVal = GETypes::ListType();
    }
    return v;
}

inline std::string getRuntimeTypeToken(const RuntimeValue& value) {
    if (value.typeName == "int" || value.typeName == "float" || value.typeName == "string" ||
        value.typeName == "bool" || value.typeName == "vec2" || value.typeName == "vec3" ||
        value.typeName == "vec4" || value.typeName == "list") {
        return value.typeName;
    }
    return "unknown";
}

inline std::string buildFunctionKey(const std::string& name, const std::vector<std::string>& argTypes) {
    std::string key;
    key.reserve(name.size() + 2 + argTypes.size() * 8);
    key += name;
    key += "(";
    for (size_t i = 0; i < argTypes.size(); i++) {
        key += argTypes[i];
        if (i + 1 < argTypes.size()) {
            key += ",";
        }
    }
    key += ")";
    return key;
}

inline std::string buildFunctionKeyFromParams(const std::string& name, const std::vector<Parameter>& params) {
    std::vector<std::string> paramTypes;
    paramTypes.reserve(params.size());
    for (const auto& p : params) {
        paramTypes.push_back(p.type);
    }
    return buildFunctionKey(name, paramTypes);
}

inline bool isMemberAccessStart(const std::vector<Token>& tokens, size_t start) {
    return (
        start + 2 < tokens.size() &&
        tokens[start].type == TokenType::IDENTIFIER &&
        tokens[start + 1].type == TokenType::DOT &&
        tokens[start + 2].type == TokenType::IDENTIFIER
    );
}

inline double getMemberNumericValue(
    const std::unordered_map<std::string, RuntimeValue>& symbolTable,
    const std::string& objectName,
    const std::string& memberName
) {
    RuntimeValue memberValue;

    auto it = symbolTable.find(objectName);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Unknown identifier: " + objectName);
    }
    const RuntimeValue& obj = it->second;

    if (obj.typeName == "vec2") {
        if (memberName == "x") memberValue = RuntimeValue("float", obj.vec2Val.x);
        else if (memberName == "y") memberValue = RuntimeValue("float", obj.vec2Val.y);
        else {
            throw std::runtime_error("Unknown member access: " + objectName + "." + memberName);
        }
    } else if (obj.typeName == "vec3") {
        if (memberName == "x") memberValue = RuntimeValue("float", obj.vec3Val.x);
        else if (memberName == "y") memberValue = RuntimeValue("float", obj.vec3Val.y);
        else if (memberName == "z") memberValue = RuntimeValue("float", obj.vec3Val.z);
        else if (memberName == "xy") {
            memberValue = makeDefaultValueForType("vec2");
            memberValue.vec2Val = obj.vec3Val.xy;
        } else if (memberName == "yz") {
            memberValue = makeDefaultValueForType("vec2");
            memberValue.vec2Val = obj.vec3Val.yz;
        } else if (memberName == "xz") {
            memberValue = makeDefaultValueForType("vec2");
            memberValue.vec2Val = obj.vec3Val.xz;
        } else {
            throw std::runtime_error("Unknown member access: " + objectName + "." + memberName);
        }
    } else if (obj.typeName == "vec4") {
        if (memberName == "x") memberValue = RuntimeValue("float", obj.vec4Val.x);
        else if (memberName == "y") memberValue = RuntimeValue("float", obj.vec4Val.y);
        else if (memberName == "z") memberValue = RuntimeValue("float", obj.vec4Val.z);
        else if (memberName == "w") memberValue = RuntimeValue("float", obj.vec4Val.w);
        else if (memberName == "xyz") {
            memberValue = makeDefaultValueForType("vec3");
            memberValue.vec3Val = obj.vec4Val.xyz;
        } else if (memberName == "zyx") {
            memberValue = makeDefaultValueForType("vec3");
            memberValue.vec3Val = obj.vec4Val.zyx;
        } else if (memberName == "yzw") {
            memberValue = makeDefaultValueForType("vec3");
            memberValue.vec3Val = obj.vec4Val.yzw;
        } else if (memberName == "wzy") {
            memberValue = makeDefaultValueForType("vec3");
            memberValue.vec3Val = obj.vec4Val.wzy;
        } else {
            throw std::runtime_error("Unknown member access: " + objectName + "." + memberName);
        }
    } else {
        throw std::runtime_error("Unknown member access: " + objectName + "." + memberName);
    }

    if (memberValue.typeName == "float" || memberValue.typeName == "int" || memberValue.typeName == "bool") {
        return memberValue.getNumberValue();
    }
    throw std::runtime_error("Member is not numeric: " + objectName + "." + memberName);
}

inline RuntimeValue getMemberRuntimeValue(
    const std::unordered_map<std::string, RuntimeValue>& symbolTable,
    const std::string& objectName,
    const std::string& memberName
) {
    auto it = symbolTable.find(objectName);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Unknown identifier: " + objectName);
    }
    const RuntimeValue& obj = it->second;
    if (obj.typeName == "vec2") {
        if (memberName == "x") return RuntimeValue("float", obj.vec2Val.x);
        if (memberName == "y") return RuntimeValue("float", obj.vec2Val.y);
    } else if (obj.typeName == "vec3") {
        if (memberName == "x") return RuntimeValue("float", obj.vec3Val.x);
        if (memberName == "y") return RuntimeValue("float", obj.vec3Val.y);
        if (memberName == "z") return RuntimeValue("float", obj.vec3Val.z);
        if (memberName == "xy") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = obj.vec3Val.xy;
            return out;
        }
        if (memberName == "yz") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = obj.vec3Val.yz;
            return out;
        }
        if (memberName == "xz") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = obj.vec3Val.xz;
            return out;
        }
    } else if (obj.typeName == "vec4") {
        if (memberName == "x") return RuntimeValue("float", obj.vec4Val.x);
        if (memberName == "y") return RuntimeValue("float", obj.vec4Val.y);
        if (memberName == "z") return RuntimeValue("float", obj.vec4Val.z);
        if (memberName == "w") return RuntimeValue("float", obj.vec4Val.w);
        if (memberName == "xyz") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = obj.vec4Val.xyz;
            return out;
        }
        if (memberName == "zyx") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = obj.vec4Val.zyx;
            return out;
        }
        if (memberName == "yzw") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = obj.vec4Val.yzw;
            return out;
        }
        if (memberName == "wzy") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = obj.vec4Val.wzy;
            return out;
        }
    }
    throw std::runtime_error("Unknown member access: " + objectName + "." + memberName);
}

inline void printValue(const RuntimeValue& val) {
    std::string out = "";
    if (val.typeName == "string") {
        out = out + val.getStringValue();
    } else if (val.typeName == "bool") {
        out = out + (val.getBoolValue() ? "true" : "false");
    } else if (val.typeName == "float" || val.typeName == "int") {
        out = out + formatRuntimeNumber(val);
    } else if (val.typeName == "vec2") {
        out = out + "vec2(" + formatNumber(val.vec2Val.x) + ", " + formatNumber(val.vec2Val.y) + ")";
    } else if (val.typeName == "vec3") {
        out = out + "vec3(" + formatNumber(val.vec3Val.x) + ", " + formatNumber(val.vec3Val.y) + ", " + formatNumber(val.vec3Val.z) + ")";
    } else if (val.typeName == "vec4") {
        out = out + "vec4(" + formatNumber(val.vec4Val.x) + ", " + formatNumber(val.vec4Val.y) + ", " + formatNumber(val.vec4Val.z) + ", " + formatNumber(val.vec4Val.w) + ")";
    } else if (val.typeName == "list") {
        out = out + "[";
        for (int i = 0; i < val.listVal.size(); ++i) {
            if (i == 0) {
                out = out + val.listVal.getValue(i);
            } else {
                out = out + ", " + val.listVal.getValue(i);
            }
        }
        out = out + "]";
    }
    out = out + "\n";
    std::cout << out;
}

struct ReturnValue {
    bool hasValue;
    RuntimeValue value;
    ReturnValue() : hasValue(false), value() {}
    ReturnValue(const RuntimeValue& v) : hasValue(true), value(v) {}
};

inline void reduceTopOperator(std::vector<double>& values, std::vector<std::string>& ops) {
    if (values.size() < 2 || ops.empty()) {
        throw std::runtime_error("Invalid expression");
    }

    const std::string op = ops.back();
    ops.pop_back();

    const double rhs = values.back();
    values.pop_back();
    const double lhs = values.back();
    values.pop_back();

    values.push_back(applyOperator(op, lhs, rhs));
}

inline size_t findMatchingParen(const std::vector<Token>& tokens, size_t openParen) {
    if (openParen >= tokens.size() || tokens[openParen].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '('");
    }

    int depth = 1;
    size_t i = openParen + 1;
    while (i < tokens.size()) {
        if (tokens[i].type == TokenType::LPAREN) depth++;
        if (tokens[i].type == TokenType::RPAREN) depth--;
        if (depth == 0) return i;
        i++;
    }

    throw std::runtime_error("Mismatched parentheses");
}

inline RuntimeValue callFunction(
    const std::vector<Token>& tokens,
    size_t start,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
);

inline RuntimeValue callFunction(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
);

inline RuntimeValue evaluateRuntimeExpression(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
);

inline bool isMethodCallStart(const std::vector<Token>& tokens, size_t start) {
    return (
        start + 3 < tokens.size() &&
        tokens[start].type == TokenType::IDENTIFIER &&
        tokens[start + 1].type == TokenType::DOT &&
        tokens[start + 2].type == TokenType::IDENTIFIER &&
        tokens[start + 3].type == TokenType::LPAREN
    );
}

inline GETypes::ListType::Value runtimeToListValue(const RuntimeValue& value) {
    if (value.typeName == "int") {
        return GETypes::IntType(value.intVal.value);
    }
    if (value.typeName == "float") {
        return GETypes::FloatType(value.floatVal.value);
    }
    if (value.typeName == "string") {
        return GETypes::StringType(value.stringVal.value);
    }
    if (value.typeName == "bool") {
        return GETypes::BoolType(value.boolVal.value);
    }
    if (value.typeName == "vec2") {
        return value.vec2Val;
    }
    if (value.typeName == "vec3") {
        return value.vec3Val;
    }
    if (value.typeName == "vec4") {
        return value.vec4Val;
    }
    return GETypes::StringType("");
}

inline RuntimeValue listValueToRuntimeValue(const GETypes::ListType::Value& value) {
    return std::visit([](const auto& item) -> RuntimeValue {
        using T = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<T, GETypes::IntType>) {
            return RuntimeValue("int", static_cast<double>(item.value));
        } else if constexpr (std::is_same_v<T, GETypes::FloatType>) {
            return RuntimeValue("float", static_cast<double>(item.value));
        } else if constexpr (std::is_same_v<T, GETypes::StringType>) {
            return RuntimeValue("string", item.value);
        } else if constexpr (std::is_same_v<T, GETypes::BoolType>) {
            return RuntimeValue("bool", static_cast<double>(item.value ? 1 : 0));
        } else {
            return RuntimeValue("string", GETypes::ListType::valueToString(GETypes::ListType::Value(item)));
        }
    }, value);
}

struct WindowState {
    bool created = false;
    bool shouldClose = false;
    int width = 800;
    int height = 600;
    std::string title = "GE Window";

    float clearR = 0.1f;
    float clearG = 0.1f;
    float clearB = 0.1f;
    float clearA = 1.0f;

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    bool mouseVisible = true;
    bool mouseCaptured = false;

    std::set<int> keysDown;
    std::set<int> mouseButtonsDown;
    int queuedDrawCommands = 0;
    bool frameCleared = false;
    int targetFPS = 0;
    std::chrono::steady_clock::time_point frameStartTime;
    bool frameStartValid = false;
    NativeWindowBackend backend;
};

inline WindowState windowState;

inline float mouseToCenteredX(float rawX) {
    return rawX - static_cast<float>(windowState.width) * 0.5f;
}

inline float mouseToCenteredY(float rawY) {
    return static_cast<float>(windowState.height) * 0.5f - rawY;
}

inline float centeredToMouseX(float centeredX) {
    return centeredX + static_cast<float>(windowState.width) * 0.5f;
}

inline float centeredToMouseY(float centeredY) {
    return static_cast<float>(windowState.height) * 0.5f - centeredY;
}

inline int centeredToWindowPixelX(double centeredX) {
    return static_cast<int>(centeredX + static_cast<double>(windowState.width) * 0.5);
}

inline int centeredToWindowPixelY(double centeredY) {
    return static_cast<int>(static_cast<double>(windowState.height) * 0.5 - centeredY);
}

inline bool isAssignmentOperatorToken(const Token& token) {
    return token.type == TokenType::EQUAL &&
           (token.value == "=" || token.value == "+=" || token.value == "-=" ||
            token.value == "*=" || token.value == "/=");
}

inline std::string assignmentToBinaryOperator(const std::string& assignmentOp) {
    if (assignmentOp == "+=") return "+";
    if (assignmentOp == "-=") return "-";
    if (assignmentOp == "*=") return "*";
    if (assignmentOp == "/=") return "/";
    return "";
}

inline double applyScalarAssignmentOperator(double lhs, double rhs, const std::string& assignmentOp) {
    if (assignmentOp == "=") {
        return rhs;
    }
    const std::string binaryOp = assignmentToBinaryOperator(assignmentOp);
    if (binaryOp.empty()) {
        throw std::runtime_error("Unsupported assignment operator: " + assignmentOp);
    }
    return applyOperator(binaryOp, lhs, rhs);
}

inline RuntimeValue applyCompoundVectorAssignment(
    const RuntimeValue& lhs,
    const RuntimeValue& rhs,
    const std::string& assignmentOp
) {
    if (assignmentOp == "=") {
        RuntimeValue out = makeDefaultValueForType(lhs.typeName);
        if (lhs.typeName == "vec2") {
            if (rhs.typeName == "vec2") out.vec2Val = rhs.vec2Val;
            else out.vec2Val = GETypes::Vec2Type(rhs.getNumberValue());
        } else if (lhs.typeName == "vec3") {
            if (rhs.typeName == "vec3") out.vec3Val = rhs.vec3Val;
            else out.vec3Val = GETypes::Vec3Type(rhs.getNumberValue());
        } else if (lhs.typeName == "vec4") {
            if (rhs.typeName == "vec4") out.vec4Val = rhs.vec4Val;
            else out.vec4Val = GETypes::Vec4Type(rhs.getNumberValue());
        }
        return out;
    }

    const std::string op = assignmentToBinaryOperator(assignmentOp);
    if (op.empty()) {
        throw std::runtime_error("Unsupported assignment operator: " + assignmentOp);
    }

    RuntimeValue result = makeDefaultValueForType(lhs.typeName);

    if (lhs.typeName == "vec2") {
        if (rhs.typeName == "vec2") {
            if (op == "+") result.vec2Val = lhs.vec2Val + rhs.vec2Val;
            else if (op == "-") result.vec2Val = lhs.vec2Val - rhs.vec2Val;
            else if (op == "*") result.vec2Val = lhs.vec2Val * rhs.vec2Val;
            else if (op == "/") result.vec2Val = lhs.vec2Val / rhs.vec2Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
            return result;
        }
        if (rhs.typeName == "int" || rhs.typeName == "float" || rhs.typeName == "bool") {
            const double scalar = rhs.getNumberValue();
            if (op == "+") result.vec2Val = lhs.vec2Val + scalar;
            else if (op == "-") result.vec2Val = lhs.vec2Val - scalar;
            else if (op == "*") result.vec2Val = lhs.vec2Val * scalar;
            else if (op == "/") result.vec2Val = lhs.vec2Val / scalar;
            else throw std::runtime_error("Unsupported vector operator: " + op);
            return result;
        }
    }

    if (lhs.typeName == "vec3") {
        if (rhs.typeName == "vec3") {
            if (op == "+") result.vec3Val = lhs.vec3Val + rhs.vec3Val;
            else if (op == "-") result.vec3Val = lhs.vec3Val - rhs.vec3Val;
            else if (op == "*") result.vec3Val = lhs.vec3Val * rhs.vec3Val;
            else if (op == "/") result.vec3Val = lhs.vec3Val / rhs.vec3Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
            return result;
        }
        if (rhs.typeName == "int" || rhs.typeName == "float" || rhs.typeName == "bool") {
            const double scalar = rhs.getNumberValue();
            if (op == "+") result.vec3Val = lhs.vec3Val + scalar;
            else if (op == "-") result.vec3Val = lhs.vec3Val - scalar;
            else if (op == "*") result.vec3Val = lhs.vec3Val * scalar;
            else if (op == "/") result.vec3Val = lhs.vec3Val / scalar;
            else throw std::runtime_error("Unsupported vector operator: " + op);
            return result;
        }
    }

    if (lhs.typeName == "vec4") {
        if (rhs.typeName == "vec4") {
            if (op == "+") result.vec4Val = lhs.vec4Val + rhs.vec4Val;
            else if (op == "-") result.vec4Val = lhs.vec4Val - rhs.vec4Val;
            else if (op == "*") result.vec4Val = lhs.vec4Val * rhs.vec4Val;
            else if (op == "/") result.vec4Val = lhs.vec4Val / rhs.vec4Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
            return result;
        }
        if (rhs.typeName == "int" || rhs.typeName == "float" || rhs.typeName == "bool") {
            const double scalar = rhs.getNumberValue();
            if (op == "+") result.vec4Val = lhs.vec4Val + scalar;
            else if (op == "-") result.vec4Val = lhs.vec4Val - scalar;
            else if (op == "*") result.vec4Val = lhs.vec4Val * scalar;
            else if (op == "/") result.vec4Val = lhs.vec4Val / scalar;
            else throw std::runtime_error("Unsupported vector operator: " + op);
            return result;
        }
    }

    throw std::runtime_error("Invalid vector assignment operation");
}

inline std::string getWindowBackendName() {
#if defined(__linux__)
    return "linux-x11";
#elif defined(_WIN32)
    return "windows-win32";
#elif defined(__APPLE__)
    return "macos-cocoa";
#else
    return "unknown";
#endif
}

inline RuntimeValue executeWindowMethodReadOnly(const std::string& methodName, const std::vector<RuntimeValue>& args) {
    (void)args;
    windowState.backend.pumpEvents(
        windowState.shouldClose,
        windowState.width,
        windowState.height,
        windowState.mouseX,
        windowState.mouseY,
        windowState.keysDown,
        windowState.mouseButtonsDown
    );

    if (methodName == "platform") {
        return RuntimeValue("string", getWindowBackendName());
    }
    if (methodName == "isOpen") {
        return RuntimeValue("int", (windowState.created && !windowState.shouldClose) ? 1.0 : 0.0);
    }
    if (methodName == "shouldClose") {
        return RuntimeValue("int", windowState.shouldClose ? 1.0 : 0.0);
    }
    if (methodName == "width") {
        return RuntimeValue("int", static_cast<double>(windowState.width));
    }
    if (methodName == "height") {
        return RuntimeValue("int", static_cast<double>(windowState.height));
    }
    if (methodName == "mouseX") {
        return RuntimeValue("float", static_cast<double>(mouseToCenteredX(windowState.mouseX)));
    }
    if (methodName == "mouseY") {
        return RuntimeValue("float", static_cast<double>(mouseToCenteredY(windowState.mouseY)));
    }
    if (methodName == "keyCode") {
        if (args.size() != 1 || args[0].typeName != "string") {
            throw std::runtime_error("window.keyCode expects exactly 1 string argument");
        }
        const int code = windowState.backend.resolveKeyCode(args[0].getStringValue());
        return RuntimeValue("int", static_cast<double>(code));
    }
    if (methodName == "drawCount") {
        return RuntimeValue("int", static_cast<double>(windowState.queuedDrawCommands));
    }
    if (methodName == "fps") {
        return RuntimeValue("int", static_cast<double>(windowState.targetFPS));
    }
    if (methodName == "keyDown") {
        if (args.size() != 1) {
            throw std::runtime_error("window.keyDown expects exactly 1 argument");
        }
        int keyCode = 0;
        if (args[0].typeName == "string") {
            keyCode = windowState.backend.resolveKeyCode(args[0].getStringValue());
        } else {
            keyCode = static_cast<int>(args[0].getNumberValue());
        }
        return RuntimeValue("int", windowState.keysDown.count(keyCode) ? 1.0 : 0.0);
    }
    if (methodName == "mouseDown") {
        if (args.size() != 1) {
            throw std::runtime_error("window.mouseDown expects exactly 1 argument");
        }
        int button = static_cast<int>(args[0].getNumberValue());
        return RuntimeValue("int", windowState.mouseButtonsDown.count(button) ? 1.0 : 0.0);
    }

    throw std::runtime_error("Unsupported window method in expression: " + methodName);
}

inline void executeWindowMethodStatement(const std::string& methodName, const std::vector<RuntimeValue>& args) {
    if (methodName == "create") {
        if (args.size() < 2 || args.size() > 3) {
            throw std::runtime_error("window.create expects 2 or 3 arguments: width, height, [title]");
        }
        windowState.width = std::max(1, static_cast<int>(args[0].getNumberValue()));
        windowState.height = std::max(1, static_cast<int>(args[1].getNumberValue()));
        if (args.size() == 3) {
            if (args[2].typeName != "string") {
                throw std::runtime_error("window.create title must be a string");
            }
            windowState.title = args[2].getStringValue();
        }

        std::string createError;
        if (!windowState.backend.create(windowState.width, windowState.height, windowState.title, createError)) {
            throw std::runtime_error(createError);
        }

        windowState.created = true;
        windowState.shouldClose = false;
        windowState.keysDown.clear();
        windowState.mouseButtonsDown.clear();
        windowState.queuedDrawCommands = 0;
        windowState.frameCleared = false;
        windowState.backend.setMouseVisible(windowState.mouseVisible);
        windowState.backend.setMouseCaptured(windowState.mouseCaptured);
        return;
    }
    if (methodName == "close") {
        windowState.shouldClose = true;
        windowState.backend.destroy();
        windowState.created = false;
        return;
    }
    if (methodName == "setTitle") {
        if (args.size() != 1 || args[0].typeName != "string") {
            throw std::runtime_error("window.setTitle expects exactly 1 string argument");
        }
        windowState.title = args[0].getStringValue();
        if (windowState.created) {
            windowState.backend.setTitle(windowState.title);
        }
        return;
    }
    if (methodName == "setSize") {
        if (args.size() != 2) {
            throw std::runtime_error("window.setSize expects exactly 2 arguments");
        }
        windowState.width = std::max(1, static_cast<int>(args[0].getNumberValue()));
        windowState.height = std::max(1, static_cast<int>(args[1].getNumberValue()));
        if (windowState.created) {
            windowState.backend.setSize(windowState.width, windowState.height);
        }
        return;
    }
    if (methodName == "beginFrame") {
        windowState.queuedDrawCommands = 0;
        windowState.frameCleared = false;
        windowState.frameStartTime = std::chrono::steady_clock::now();
        windowState.frameStartValid = true;
        windowState.backend.pumpEvents(
            windowState.shouldClose,
            windowState.width,
            windowState.height,
            windowState.mouseX,
            windowState.mouseY,
            windowState.keysDown,
            windowState.mouseButtonsDown
        );
        return;
    }
    if (methodName == "endFrame") {
        if (windowState.created) {
            if (!windowState.frameCleared) {
                windowState.backend.beginFrame(windowState.clearR, windowState.clearG, windowState.clearB, windowState.width, windowState.height);
            }
            windowState.backend.endFrame(windowState.width, windowState.height);
        }

        if (windowState.targetFPS > 0 && windowState.frameStartValid) {
            const auto targetFrameDuration = std::chrono::milliseconds(1000 / windowState.targetFPS);
            const auto elapsed = std::chrono::steady_clock::now() - windowState.frameStartTime;
            if (elapsed < targetFrameDuration) {
                std::this_thread::sleep_for(targetFrameDuration - elapsed);
            }
            windowState.frameStartValid = false;
        }

        windowState.backend.pumpEvents(
            windowState.shouldClose,
            windowState.width,
            windowState.height,
            windowState.mouseX,
            windowState.mouseY,
            windowState.keysDown,
            windowState.mouseButtonsDown
        );
        return;
    }
    if (methodName == "clear") {
        if (args.size() != 4) {
            throw std::runtime_error("window.clear expects exactly 4 arguments (r, g, b, a)");
        }
        windowState.clearR = static_cast<float>(args[0].getNumberValue());
        windowState.clearG = static_cast<float>(args[1].getNumberValue());
        windowState.clearB = static_cast<float>(args[2].getNumberValue());
        windowState.clearA = static_cast<float>(args[3].getNumberValue());
        if (windowState.created) {
            windowState.backend.beginFrame(windowState.clearR, windowState.clearG, windowState.clearB, windowState.width, windowState.height);
            windowState.frameCleared = true;
        }
        return;
    }
    if (methodName == "drawRect") {
        if (args.size() != 8) {
            throw std::runtime_error("window.drawRect expects 8 arguments");
        }
        if (windowState.created) {
            const int x = centeredToWindowPixelX(args[0].getNumberValue());
            const int y = centeredToWindowPixelY(args[1].getNumberValue());
            const unsigned int w = static_cast<unsigned int>(std::max(0.0, args[2].getNumberValue()));
            const unsigned int h = static_cast<unsigned int>(std::max(0.0, args[3].getNumberValue()));
            windowState.backend.drawRect(
                x,
                y,
                w,
                h,
                static_cast<float>(args[4].getNumberValue()),
                static_cast<float>(args[5].getNumberValue()),
                static_cast<float>(args[6].getNumberValue())
            );
        }
        windowState.queuedDrawCommands++;
        return;
    }
    if (methodName == "drawCircle") {
        if (args.size() != 7) {
            throw std::runtime_error("window.drawCircle expects 7 arguments");
        }
        if (windowState.created) {
            const int cx = centeredToWindowPixelX(args[0].getNumberValue());
            const int cy = centeredToWindowPixelY(args[1].getNumberValue());
            const int radius = static_cast<int>(std::max(0.0, args[2].getNumberValue()));
            windowState.backend.drawCircle(
                cx,
                cy,
                radius,
                static_cast<float>(args[3].getNumberValue()),
                static_cast<float>(args[4].getNumberValue()),
                static_cast<float>(args[5].getNumberValue())
            );
        }
        windowState.queuedDrawCommands++;
        return;
    }
    if (methodName == "drawLine") {
        if (args.size() != 9) {
            throw std::runtime_error("window.drawLine expects 9 arguments");
        }
        if (windowState.created) {
            const int x1 = centeredToWindowPixelX(args[0].getNumberValue());
            const int y1 = centeredToWindowPixelY(args[1].getNumberValue());
            const int x2 = centeredToWindowPixelX(args[2].getNumberValue());
            const int y2 = centeredToWindowPixelY(args[3].getNumberValue());
            const int thickness = std::max(1, static_cast<int>(args[4].getNumberValue()));
            windowState.backend.drawLine(
                x1,
                y1,
                x2,
                y2,
                thickness,
                static_cast<float>(args[5].getNumberValue()),
                static_cast<float>(args[6].getNumberValue()),
                static_cast<float>(args[7].getNumberValue())
            );
        }
        windowState.queuedDrawCommands++;
        return;
    }
    if (methodName == "drawTri") {
        const bool numericSignature = (args.size() == 11);
        const bool vec2Signature = (
            args.size() == 8 &&
            args[0].typeName == "vec2" &&
            args[1].typeName == "vec2" &&
            args[2].typeName == "vec2"
        );

        if (!numericSignature && !vec2Signature) {
            throw std::runtime_error("window.drawTri expects either (x1, y1, x2, y2, x3, y3, thickness, r, g, b, a) or (vec2 a, vec2 b, vec2 c, thickness, r, g, b, a)");
        }

        if (windowState.created) {
            int x1 = 0;
            int y1 = 0;
            int x2 = 0;
            int y2 = 0;
            int x3 = 0;
            int y3 = 0;
            int thickness = 1;
            float r = 1.0f;
            float g = 1.0f;
            float b = 1.0f;

            if (numericSignature) {
                x1 = centeredToWindowPixelX(args[0].getNumberValue());
                y1 = centeredToWindowPixelY(args[1].getNumberValue());
                x2 = centeredToWindowPixelX(args[2].getNumberValue());
                y2 = centeredToWindowPixelY(args[3].getNumberValue());
                x3 = centeredToWindowPixelX(args[4].getNumberValue());
                y3 = centeredToWindowPixelY(args[5].getNumberValue());
                thickness = std::max(1, static_cast<int>(args[6].getNumberValue()));
                r = static_cast<float>(args[7].getNumberValue());
                g = static_cast<float>(args[8].getNumberValue());
                b = static_cast<float>(args[9].getNumberValue());
            } else {
                x1 = centeredToWindowPixelX(static_cast<double>(args[0].vec2Val.x));
                y1 = centeredToWindowPixelY(static_cast<double>(args[0].vec2Val.y));
                x2 = centeredToWindowPixelX(static_cast<double>(args[1].vec2Val.x));
                y2 = centeredToWindowPixelY(static_cast<double>(args[1].vec2Val.y));
                x3 = centeredToWindowPixelX(static_cast<double>(args[2].vec2Val.x));
                y3 = centeredToWindowPixelY(static_cast<double>(args[2].vec2Val.y));
                thickness = std::max(1, static_cast<int>(args[3].getNumberValue()));
                r = static_cast<float>(args[4].getNumberValue());
                g = static_cast<float>(args[5].getNumberValue());
                b = static_cast<float>(args[6].getNumberValue());
            }

            windowState.backend.drawTriangle(
                x1,
                y1,
                x2,
                y2,
                x3,
                y3,
                thickness,
                r,
                g,
                b
            );
        }
        windowState.queuedDrawCommands++;
        return;
    }
    if (methodName == "drawPoly") {
        if (args.size() != 6 || args[0].typeName != "list") {
            throw std::runtime_error("window.drawPoly expects (list points, thickness, r, g, b, a)");
        }

        const auto& listPoints = args[0].listVal.elements;
        if (listPoints.size() < 2) {
            throw std::runtime_error("window.drawPoly expects at least 2 points");
        }

        std::vector<std::pair<int, int>> points;
        points.reserve(listPoints.size());
        bool hasPendingX = false;
        double pendingX = 0.0;
        for (const auto& p : listPoints) {
            if (std::holds_alternative<GETypes::Vec2Type>(p)) {
                const auto& v = std::get<GETypes::Vec2Type>(p);
                points.push_back({
                    centeredToWindowPixelX(static_cast<double>(v.x)),
                    centeredToWindowPixelY(static_cast<double>(v.y))
                });
                continue;
            }
            if (std::holds_alternative<GETypes::Vec3Type>(p)) {
                const auto& v = std::get<GETypes::Vec3Type>(p);
                points.push_back({
                    centeredToWindowPixelX(static_cast<double>(v.x)),
                    centeredToWindowPixelY(static_cast<double>(v.y))
                });
                continue;
            }
            if (std::holds_alternative<GETypes::Vec4Type>(p)) {
                const auto& v = std::get<GETypes::Vec4Type>(p);
                points.push_back({
                    centeredToWindowPixelX(static_cast<double>(v.x)),
                    centeredToWindowPixelY(static_cast<double>(v.y))
                });
                continue;
            }

            double scalar = 0.0;
            if (std::holds_alternative<GETypes::IntType>(p)) {
                scalar = static_cast<double>(std::get<GETypes::IntType>(p).value);
            } else if (std::holds_alternative<GETypes::FloatType>(p)) {
                scalar = static_cast<double>(std::get<GETypes::FloatType>(p).value);
            } else {
                throw std::runtime_error("window.drawPoly points list must contain vec2/vec3/vec4 or numeric x,y pairs");
            }

            if (!hasPendingX) {
                pendingX = scalar;
                hasPendingX = true;
            } else {
                points.push_back({
                    centeredToWindowPixelX(pendingX),
                    centeredToWindowPixelY(scalar)
                });
                hasPendingX = false;
            }
        }

        if (hasPendingX) {
            throw std::runtime_error("window.drawPoly numeric point list requires an even number of values");
        }

        if (points.size() < 2) {
            throw std::runtime_error("window.drawPoly requires at least two resolved points");
        }

        if (windowState.created) {
            const int thickness = std::max(1, static_cast<int>(args[1].getNumberValue()));
            windowState.backend.drawPolygon(
                points,
                thickness,
                static_cast<float>(args[2].getNumberValue()),
                static_cast<float>(args[3].getNumberValue()),
                static_cast<float>(args[4].getNumberValue())
            );
        }

        windowState.queuedDrawCommands++;
        return;
    }
    if (methodName == "drawText") {
        if (args.size() != 4 || args[0].typeName != "string") {
            throw std::runtime_error("window.drawText expects (string text, x, y, size)");
        }
        if (windowState.created) {
            const std::string text = args[0].getStringValue();
            const int x = centeredToWindowPixelX(args[1].getNumberValue());
            const int y = centeredToWindowPixelY(args[2].getNumberValue());
            (void)args[3];
            windowState.backend.drawText(text, x, y);
        }
        windowState.queuedDrawCommands++;
        return;
    }
    if (methodName == "setMousePosition") {
        if (args.size() != 2) {
            throw std::runtime_error("window.setMousePosition expects exactly 2 arguments");
        }
        windowState.mouseX = centeredToMouseX(static_cast<float>(args[0].getNumberValue()));
        windowState.mouseY = centeredToMouseY(static_cast<float>(args[1].getNumberValue()));
        if (windowState.created) {
            windowState.backend.setMousePosition(static_cast<int>(windowState.mouseX), static_cast<int>(windowState.mouseY));
        }
        return;
    }
    if (methodName == "setMouseVisible") {
        if (args.size() != 1) {
            throw std::runtime_error("window.setMouseVisible expects exactly 1 argument");
        }
        windowState.mouseVisible = args[0].getNumberValue() != 0.0;
        if (windowState.created) {
            windowState.backend.setMouseVisible(windowState.mouseVisible);
        }
        return;
    }
    if (methodName == "captureMouse") {
        if (args.size() != 1) {
            throw std::runtime_error("window.captureMouse expects exactly 1 argument");
        }
        windowState.mouseCaptured = args[0].getNumberValue() != 0.0;
        if (windowState.created) {
            windowState.backend.setMouseCaptured(windowState.mouseCaptured);
        }
        return;
    }
    if (methodName == "setKeyState") {
        if (args.size() != 2) {
            throw std::runtime_error("window.setKeyState expects exactly 2 arguments (keyCode, down)");
        }
        int keyCode = static_cast<int>(args[0].getNumberValue());
        bool down = args[1].getNumberValue() != 0.0;
        if (down) {
            windowState.keysDown.insert(keyCode);
        } else {
            windowState.keysDown.erase(keyCode);
        }
        return;
    }
    if (methodName == "setMouseButton") {
        if (args.size() != 2) {
            throw std::runtime_error("window.setMouseButton expects exactly 2 arguments (button, down)");
        }
        int button = static_cast<int>(args[0].getNumberValue());
        bool down = args[1].getNumberValue() != 0.0;
        if (down) {
            windowState.mouseButtonsDown.insert(button);
        } else {
            windowState.mouseButtonsDown.erase(button);
        }
        return;
    }
    if (methodName == "setFPS" || methodName == "setFps") {
        if (args.size() != 1) {
            throw std::runtime_error("window.setFPS expects exactly 1 argument (fps)");
        }
        const int fps = static_cast<int>(args[0].getNumberValue());
        windowState.targetFPS = std::max(0, fps);
        return;
    }
    if (methodName == "sleep") {
        if (args.size() != 1) {
            throw std::runtime_error("window.sleep expects exactly 1 argument (milliseconds)");
        }
        int ms = std::max(0, static_cast<int>(args[0].getNumberValue()));
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return;
    }

    if (
        methodName == "platform" ||
        methodName == "isOpen" ||
        methodName == "shouldClose" ||
        methodName == "width" ||
        methodName == "height" ||
        methodName == "mouseX" ||
        methodName == "mouseY" ||
        methodName == "keyCode" ||
        methodName == "drawCount" ||
        methodName == "fps" ||
        methodName == "keyDown" ||
        methodName == "mouseDown"
    ) {
        (void)executeWindowMethodReadOnly(methodName, args);
        return;
    }

    throw std::runtime_error("Unknown window method: " + methodName);
}

inline std::vector<RuntimeValue> parseArgumentList(
    const std::vector<Token>& tokens,
    size_t argsStart,
    size_t argsEnd,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    std::vector<RuntimeValue> args;
    if (argsStart > argsEnd) {
        return args;
    }

    args.reserve(4);

    size_t segmentStart = argsStart;
    int depth = 0;
    for (size_t i = argsStart; i <= argsEnd; i++) {
        if (tokens[i].type == TokenType::LPAREN) depth++;
        if (tokens[i].type == TokenType::RPAREN) depth--;

        if (tokens[i].type == TokenType::COMMA && depth == 0) {
            if (i > segmentStart) {
                args.push_back(evaluateRuntimeExpression(tokens, segmentStart, i - 1, symbolTable));
            }
            segmentStart = i + 1;
        }
    }

    if (segmentStart <= argsEnd) {
        args.push_back(evaluateRuntimeExpression(tokens, segmentStart, argsEnd, symbolTable));
    }

    return args;
}

inline RuntimeValue executeMethodCallReadOnly(
    const std::vector<Token>& tokens,
    size_t start,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (!isMethodCallStart(tokens, start)) {
        throw std::runtime_error("Invalid method call syntax");
    }

    const std::string objectName = tokens[start].value;
    const std::string methodName = tokens[start + 2].value;
    const size_t closeParen = findMatchingParen(tokens, start + 3);

    const std::vector<RuntimeValue> args = parseArgumentList(tokens, start + 4, closeParen - 1, symbolTable);

    if (objectName == "window") {
        return executeWindowMethodReadOnly(methodName, args);
    }

    auto objIt = symbolTable.find(objectName);
    if (objIt == symbolTable.end()) {
        throw std::runtime_error("Unknown object: " + objectName);
    }
    if (objIt->second.typeName != "list") {
        throw std::runtime_error("Method calls currently supported only on list objects");
    }

    const RuntimeValue& obj = objIt->second;

    if (methodName == "size") {
        return RuntimeValue("int", static_cast<double>(obj.listVal.size()));
    }
    if (methodName == "get") {
        if (args.size() != 1) {
            throw std::runtime_error("list.get expects exactly 1 argument");
        }
        size_t index = static_cast<size_t>(args[0].getNumberValue());
        if (index >= obj.listVal.size()) {
            throw std::runtime_error("list.get index out of range");
        }
        return listValueToRuntimeValue(obj.listVal.elements[index]);
    }
    if (methodName == "indexOf") {
        if (args.size() != 1) {
            throw std::runtime_error("list.indexOf expects exactly 1 argument");
        }
        size_t index = obj.listVal.getIndex(runtimeToListValue(args[0]));
        if (index == static_cast<size_t>(-1)) {
            return RuntimeValue("int", -1.0);
        }
        return RuntimeValue("int", static_cast<double>(index));
    }

    throw std::runtime_error("Unsupported list method in expression: " + methodName);
}

inline double evaluateExpression(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start == end) {
        const Token& token = tokens[start];
        if (token.type == TokenType::NUMBER) {
            return std::stod(token.value);
        }
        if (token.type == TokenType::IDENTIFIER) {
            auto it = symbolTable.find(token.value);
            if (it == symbolTable.end()) {
                throw std::runtime_error("Unknown identifier: " + token.value);
            }
            return it->second.getNumberValue();
        }
    }

    if (isMemberAccessStart(tokens, start) && start + 2 == end) {
        return getMemberNumericValue(symbolTable, tokens[start].value, tokens[start + 2].value);
    }

    if (
        (tokens[start].type == TokenType::IDENTIFIER || tokens[start].type == TokenType::TYPE) &&
        start + 1 <= end &&
        tokens[start + 1].type == TokenType::LPAREN
    ) {
        const size_t callEnd = findMatchingParen(tokens, start + 1);
        if (callEnd == end) {
            RuntimeValue callResult = callFunction(tokens, start, symbolTable);
            return callResult.getNumberValue();
        }
    }

    if (
        start < end &&
        tokens[start].type == TokenType::OPERATOR &&
        (tokens[start].value == "-" || tokens[start].value == "+")
    ) {
        const double value = evaluateExpression(tokens, start + 1, end, symbolTable);
        return tokens[start].value == "-" ? -value : value;
    }

    std::vector<double> values;
    std::vector<std::string> ops;
    if (end >= start) {
        const size_t reserveCount = end - start + 1;
        values.reserve(reserveCount);
        ops.reserve(reserveCount);
    }

    for (size_t i = start; i <= end; i++) {
        const Token& token = tokens[i];

        if (token.type == TokenType::NUMBER) {
            values.push_back(std::stod(token.value));
            continue;
        }

        if (
            isMethodCallStart(tokens, i)
        ) {
            size_t callEnd = findMatchingParen(tokens, i + 3);
            if (callEnd > end) {
                throw std::runtime_error("Method call exceeds expression boundary");
            }
            RuntimeValue callResult = executeMethodCallReadOnly(tokens, i, symbolTable);
            values.push_back(callResult.getNumberValue());
            i = callEnd;
            continue;
        }

        if (isMemberAccessStart(tokens, i)) {
            const std::string objectName = tokens[i].value;
            const std::string memberName = tokens[i + 2].value;
            values.push_back(getMemberNumericValue(symbolTable, objectName, memberName));
            i += 2;
            continue;
        }

        if (
            (token.type == TokenType::IDENTIFIER || token.type == TokenType::TYPE) &&
            i + 1 <= end &&
            tokens[i + 1].type == TokenType::LPAREN
        ) {
            size_t callEnd = findMatchingParen(tokens, i + 1);
            if (callEnd > end) {
                throw std::runtime_error("Function call exceeds expression boundary");
            }

            RuntimeValue callResult = callFunction(tokens, i, symbolTable);
            values.push_back(callResult.getNumberValue());
            i = callEnd;
            continue;
        }

        if (token.type == TokenType::IDENTIFIER) {
            auto it = symbolTable.find(token.value);
            if (it == symbolTable.end()) {
                throw std::runtime_error("Unknown identifier: " + token.value);
            }
            values.push_back(it->second.getNumberValue());
            continue;
        }

        if (token.type == TokenType::LPAREN) {
            ops.push_back(token.value);
            continue;
        }

        if (token.type == TokenType::RPAREN) {
            while (!ops.empty() && ops.back() != "(") {
                reduceTopOperator(values, ops);
            }

            if (ops.empty() || ops.back() != "(") {
                throw std::runtime_error("Mismatched parentheses");
            }
            ops.pop_back();
            continue;
        }

        if (token.type == TokenType::OPERATOR) {
            while (
                !ops.empty() &&
                ops.back() != "(" &&
                getOperatorPrecedence(ops.back()) >= getOperatorPrecedence(token.value)
            ) {
                reduceTopOperator(values, ops);
            }
            ops.push_back(token.value);
            continue;
        }

        throw std::runtime_error("Invalid token in expression: " + token.value);
    }

    while (!ops.empty()) {
        if (ops.back() == "(") {
            throw std::runtime_error("Mismatched parentheses");
        }
        reduceTopOperator(values, ops);
    }

    if (values.size() != 1) {
        throw std::runtime_error("Invalid expression result");
    }

    return values.back();
}

inline size_t findComparisonIndexAtDepthZero(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end
) {
    int depth = 0;
    for (size_t i = start; i <= end && i < tokens.size(); i++) {
        if (tokens[i].type == TokenType::LPAREN) {
            depth++;
            continue;
        }
        if (tokens[i].type == TokenType::RPAREN) {
            depth--;
            continue;
        }
        if (depth == 0 && tokens[i].type == TokenType::COMPARISON) {
            return i;
        }
    }
    return tokens.size();
}

inline bool evaluateConditionWithComparisonIndex(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    size_t comparisonIndex,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    auto tryGetSimpleNumericValue = [&](size_t exprStart, size_t exprEnd, double& outValue) -> bool {
        if (exprStart > exprEnd || exprEnd >= tokens.size()) {
            return false;
        }

        if (exprStart == exprEnd) {
            const Token& token = tokens[exprStart];
            if (token.type == TokenType::NUMBER) {
                outValue = std::stod(token.value);
                return true;
            }
            if (token.type == TokenType::IDENTIFIER) {
                auto it = symbolTable.find(token.value);
                if (it != symbolTable.end()) {
                    outValue = it->second.getNumberValue();
                    return true;
                }
            }
            return false;
        }

        if (isMemberAccessStart(tokens, exprStart) && exprStart + 2 == exprEnd) {
            outValue = getMemberNumericValue(symbolTable, tokens[exprStart].value, tokens[exprStart + 2].value);
            return true;
        }

        return false;
    };

    if (comparisonIndex < tokens.size()) {
        double lhs = 0.0;
        if (!tryGetSimpleNumericValue(start, comparisonIndex - 1, lhs)) {
            lhs = evaluateExpression(tokens, start, comparisonIndex - 1, symbolTable);
        }

        double rhs = 0.0;
        if (!tryGetSimpleNumericValue(comparisonIndex + 1, end, rhs)) {
            rhs = evaluateExpression(tokens, comparisonIndex + 1, end, symbolTable);
        }

        return compareValues(tokens[comparisonIndex].value, lhs, rhs);
    }

    double value = 0.0;
    if (!tryGetSimpleNumericValue(start, end, value)) {
        value = evaluateExpression(tokens, start, end, symbolTable);
    }
    return value != 0.0;
}

inline bool tryEvaluateSimpleLoopIncrementRhs(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable,
    const std::string& targetVar,
    double& outValue
) {
    auto trySimpleValue = [&](size_t index, double& value) -> bool {
        if (index >= tokens.size()) {
            return false;
        }

        const Token& token = tokens[index];
        if (token.type == TokenType::NUMBER) {
            value = std::stod(token.value);
            return true;
        }

        if (token.type == TokenType::IDENTIFIER) {
            auto it = symbolTable.find(token.value);
            if (it != symbolTable.end()) {
                value = it->second.getNumberValue();
                return true;
            }
        }

        return false;
    };

    if (start > end || end >= tokens.size()) {
        return false;
    }

    if (start == end) {
        return trySimpleValue(start, outValue);
    }

    if (end == start + 2 &&
        tokens[start].type == TokenType::IDENTIFIER &&
        tokens[start].value == targetVar &&
        tokens[start + 1].type == TokenType::OPERATOR &&
        (tokens[start + 1].value == "+" || tokens[start + 1].value == "-" ||
         tokens[start + 1].value == "*" || tokens[start + 1].value == "/")) {
        auto lhsIt = symbolTable.find(targetVar);
        if (lhsIt == symbolTable.end()) {
            return false;
        }

        double rhs = 0.0;
        if (!trySimpleValue(start + 2, rhs)) {
            return false;
        }

        outValue = applyOperator(tokens[start + 1].value, lhsIt->second.getNumberValue(), rhs);
        return true;
    }

    return false;
}

inline RuntimeValue evaluateBinaryVectorOperation(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    // Find the operator at depth 0
    int depth = 0;
    size_t opIndex = end + 1;
    for (size_t i = start; i <= end; i++) {
        if (tokens[i].type == TokenType::LPAREN) depth++;
        if (tokens[i].type == TokenType::RPAREN) depth--;
        if (depth == 0 && tokens[i].type == TokenType::OPERATOR) {
            opIndex = i;
            // Take the last operator at depth 0 (lowest precedence)
        }
    }

    if (opIndex > end) {
        return RuntimeValue("float", 0.0); // No operator found
    }

    const std::string op = tokens[opIndex].value;
    const RuntimeValue lhs = evaluateRuntimeExpression(tokens, start, opIndex - 1, symbolTable);
    const RuntimeValue rhs = evaluateRuntimeExpression(tokens, opIndex + 1, end, symbolTable);

    // Check if both are same vector type
    if (lhs.typeName == rhs.typeName && isVectorTypeName(lhs.typeName)) {
        RuntimeValue result = makeDefaultValueForType(lhs.typeName);

        if (lhs.typeName == "vec2") {
            if (op == "+") result.vec2Val = lhs.vec2Val + rhs.vec2Val;
            else if (op == "-") result.vec2Val = lhs.vec2Val - rhs.vec2Val;
            else if (op == "*") result.vec2Val = lhs.vec2Val * rhs.vec2Val;
            else if (op == "/") result.vec2Val = lhs.vec2Val / rhs.vec2Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        } else if (lhs.typeName == "vec3") {
            if (op == "+") result.vec3Val = lhs.vec3Val + rhs.vec3Val;
            else if (op == "-") result.vec3Val = lhs.vec3Val - rhs.vec3Val;
            else if (op == "*") result.vec3Val = lhs.vec3Val * rhs.vec3Val;
            else if (op == "/") result.vec3Val = lhs.vec3Val / rhs.vec3Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        } else if (lhs.typeName == "vec4") {
            if (op == "+") result.vec4Val = lhs.vec4Val + rhs.vec4Val;
            else if (op == "-") result.vec4Val = lhs.vec4Val - rhs.vec4Val;
            else if (op == "*") result.vec4Val = lhs.vec4Val * rhs.vec4Val;
            else if (op == "/") result.vec4Val = lhs.vec4Val / rhs.vec4Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        }
        return result;
    }

    // Vector-scalar operations
    if (isVectorTypeName(lhs.typeName) && (rhs.typeName == "float" || rhs.typeName == "int")) {
        const double scalar = rhs.getNumberValue();
        RuntimeValue result = makeDefaultValueForType(lhs.typeName);

        if (lhs.typeName == "vec2") {
            if (op == "+") result.vec2Val = lhs.vec2Val + scalar;
            else if (op == "-") result.vec2Val = lhs.vec2Val - scalar;
            else if (op == "*") result.vec2Val = lhs.vec2Val * scalar;
            else if (op == "/") result.vec2Val = lhs.vec2Val / scalar;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        } else if (lhs.typeName == "vec3") {
            if (op == "+") result.vec3Val = lhs.vec3Val + scalar;
            else if (op == "-") result.vec3Val = lhs.vec3Val - scalar;
            else if (op == "*") result.vec3Val = lhs.vec3Val * scalar;
            else if (op == "/") result.vec3Val = lhs.vec3Val / scalar;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        } else if (lhs.typeName == "vec4") {
            if (op == "+") result.vec4Val = lhs.vec4Val + scalar;
            else if (op == "-") result.vec4Val = lhs.vec4Val - scalar;
            else if (op == "*") result.vec4Val = lhs.vec4Val * scalar;
            else if (op == "/") result.vec4Val = lhs.vec4Val / scalar;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        }
        return result;
    }

    // Scalar-vector operations
    if ((lhs.typeName == "float" || lhs.typeName == "int") && isVectorTypeName(rhs.typeName)) {
        const double scalar = lhs.getNumberValue();
        RuntimeValue result = makeDefaultValueForType(rhs.typeName);

        if (rhs.typeName == "vec2") {
            if (op == "+") result.vec2Val = GETypes::Vec2Type(scalar) + rhs.vec2Val;
            else if (op == "-") result.vec2Val = GETypes::Vec2Type(scalar) - rhs.vec2Val;
            else if (op == "*") result.vec2Val = GETypes::Vec2Type(scalar) * rhs.vec2Val;
            else if (op == "/") result.vec2Val = GETypes::Vec2Type(scalar) / rhs.vec2Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        } else if (rhs.typeName == "vec3") {
            if (op == "+") result.vec3Val = GETypes::Vec3Type(scalar) + rhs.vec3Val;
            else if (op == "-") result.vec3Val = GETypes::Vec3Type(scalar) - rhs.vec3Val;
            else if (op == "*") result.vec3Val = GETypes::Vec3Type(scalar) * rhs.vec3Val;
            else if (op == "/") result.vec3Val = GETypes::Vec3Type(scalar) / rhs.vec3Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        } else if (rhs.typeName == "vec4") {
            if (op == "+") result.vec4Val = GETypes::Vec4Type(scalar) + rhs.vec4Val;
            else if (op == "-") result.vec4Val = GETypes::Vec4Type(scalar) - rhs.vec4Val;
            else if (op == "*") result.vec4Val = GETypes::Vec4Type(scalar) * rhs.vec4Val;
            else if (op == "/") result.vec4Val = GETypes::Vec4Type(scalar) / rhs.vec4Val;
            else throw std::runtime_error("Unsupported vector operator: " + op);
        }
        return result;
    }

    throw std::runtime_error("Invalid vector operation");
}

inline RuntimeValue evaluateRuntimeExpression(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start > end) {
        throw std::runtime_error("Empty expression");
    }

    if (
        start < end &&
        tokens[start].type == TokenType::OPERATOR &&
        (tokens[start].value == "-" || tokens[start].value == "+")
    ) {
        RuntimeValue value = evaluateRuntimeExpression(tokens, start + 1, end, symbolTable);
        if (tokens[start].value == "-") {
            return negateRuntimeValue(value);
        }
        return value;
    }

    if (start == end) {
        if (tokens[start].type == TokenType::STRING) {
            return RuntimeValue("string", tokens[start].value);
        }
        if (tokens[start].type == TokenType::NUMBER) {
            const bool isFloat = tokens[start].value.find('.') != std::string::npos;
            return RuntimeValue(isFloat ? "float" : "int", std::stod(tokens[start].value));
        }
        if (tokens[start].type == TokenType::IDENTIFIER) {
            auto it = symbolTable.find(tokens[start].value);
            if (it == symbolTable.end()) {
                throw std::runtime_error("Unknown identifier: " + tokens[start].value);
            }
            return it->second;
        }
    }

    if (
        isMethodCallStart(tokens, start)
    ) {
        const size_t callEnd = findMatchingParen(tokens, start + 3);
        if (callEnd == end) {
            return executeMethodCallReadOnly(tokens, start, symbolTable);
        }
    }

    if (isMemberAccessStart(tokens, start) && start + 2 == end) {
        return getMemberRuntimeValue(symbolTable, tokens[start].value, tokens[start + 2].value);
    }

    if (
        (tokens[start].type == TokenType::IDENTIFIER || tokens[start].type == TokenType::TYPE) &&
        start + 1 <= end &&
        tokens[start + 1].type == TokenType::LPAREN
    ) {
        const size_t callEnd = findMatchingParen(tokens, start + 1);
        if (callEnd == end) {
            return callFunction(tokens, start, symbolTable);
        }
    }

    // Try vector binary operation
    try {
        RuntimeValue vecResult = evaluateBinaryVectorOperation(tokens, start, end, symbolTable);
        if (vecResult.typeName != "float" || start != end) {
            return vecResult;
        }
    } catch (...) {
        // Fall through to scalar evaluation
    }

    const double value = evaluateExpression(tokens, start, end, symbolTable);
    return RuntimeValue("float", value);
}

inline RuntimeValue castRuntimeValueToType(const RuntimeValue& value, const std::string& targetType) {
    if (targetType == value.typeName) {
        return value;
    }

    if (targetType == "int") {
        return RuntimeValue("int", static_cast<double>(static_cast<int>(value.getNumberValue())));
    }
    if (targetType == "float") {
        return RuntimeValue("float", value.getNumberValue());
    }
    if (targetType == "bool") {
        return RuntimeValue("bool", value.getNumberValue() != 0.0 ? 1.0 : 0.0);
    }
    if (targetType == "string") {
        return RuntimeValue("string", value.typeName == "string" ? value.getStringValue() : formatRuntimeNumber(value));
    }

    if (targetType == "vec2") {
        RuntimeValue out = makeDefaultValueForType("vec2");
        if (value.typeName == "vec2") {
            out.vec2Val = value.vec2Val;
        } else {
            const double n = value.getNumberValue();
            out.vec2Val = GETypes::Vec2Type(n);
        }
        return out;
    }
    if (targetType == "vec3") {
        RuntimeValue out = makeDefaultValueForType("vec3");
        if (value.typeName == "vec3") {
            out.vec3Val = value.vec3Val;
        } else {
            const double n = value.getNumberValue();
            out.vec3Val = GETypes::Vec3Type(n);
        }
        return out;
    }
    if (targetType == "vec4") {
        RuntimeValue out = makeDefaultValueForType("vec4");
        if (value.typeName == "vec4") {
            out.vec4Val = value.vec4Val;
        } else {
            const double n = value.getNumberValue();
            out.vec4Val = GETypes::Vec4Type(n);
        }
        return out;
    }

    return value;
}

inline size_t executeDeclaration(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start + 1 >= tokens.size()) {
        throw std::runtime_error("Incomplete declaration");
    }

    if (
        tokens[start].type != TokenType::TYPE ||
        tokens[start + 1].type != TokenType::IDENTIFIER ||
        (tokens[start + 2].type != TokenType::EQUAL && tokens[start + 2].type != TokenType::SEMICOLON)
    ) {
        throw std::runtime_error("Invalid declaration syntax");
    }

    std::string varName = tokens[start + 1].value;
    std::string varType = tokens[start].value;
    
    // Support GLSL-style default declarations: vec2 a;
    if (tokens[start + 2].type == TokenType::SEMICOLON) {
        symbolTable[varName] = makeDefaultValueForType(varType);
        return start + 3;
    }

    size_t semicolonIndex = start + 3;
    while (semicolonIndex < tokens.size() && tokens[semicolonIndex].type != TokenType::SEMICOLON) {
        semicolonIndex++;
    }

    if (semicolonIndex >= tokens.size()) {
        throw std::runtime_error("Missing ';' at end of declaration");
    }

    if (semicolonIndex == start + 3) {
        throw std::runtime_error("Missing expression in declaration");
    }

    // Handle list initialization
    if (varType == "list" && tokens[start + 3].type == TokenType::LBRACKET) {
        RuntimeValue listVal(varType, "");
        listVal.type = GETypes::VariableType::LIST;
        
        // Parse list elements
        size_t i = start + 4;
        while (i < semicolonIndex && tokens[i].type != TokenType::RBRACKET) {
            if (tokens[i].type == TokenType::NUMBER) {
                if (tokens[i].value.find('.') != std::string::npos) {
                    listVal.listVal.add(GETypes::FloatType(static_cast<float>(std::stod(tokens[i].value))));
                } else {
                    listVal.listVal.add(GETypes::IntType(std::stoi(tokens[i].value)));
                }
            } else if (tokens[i].type == TokenType::STRING) {
                listVal.listVal.add(GETypes::StringType(tokens[i].value));
            } else if (tokens[i].type == TokenType::IDENTIFIER) {
                auto it = symbolTable.find(tokens[i].value);
                if (it != symbolTable.end()) {
                    if (it->second.typeName == "string") {
                        listVal.listVal.add(GETypes::StringType(it->second.getStringValue()));
                    } else if (it->second.typeName == "bool") {
                        listVal.listVal.add(GETypes::BoolType(it->second.getBoolValue()));
                    } else if (it->second.typeName == "float") {
                        listVal.listVal.add(GETypes::FloatType(static_cast<float>(it->second.getNumberValue())));
                    } else if (it->second.typeName == "int") {
                        listVal.listVal.add(GETypes::IntType(static_cast<int>(it->second.getNumberValue())));
                    } else {
                        listVal.listVal.add(GETypes::StringType(formatRuntimeNumber(it->second)));
                    }
                }
            }
            i++;
        }
        
        symbolTable[varName] = listVal;
    }
    // Handle string assignments
    else if (varType == "string" && tokens[start + 3].type == TokenType::STRING) {
        symbolTable[varName] = RuntimeValue(varType, tokens[start + 3].value);
    }
    // Handle vector assignments and constructor calls
    else if (isVectorTypeName(varType)) {
        RuntimeValue value = evaluateRuntimeExpression(tokens, start + 3, semicolonIndex - 1, symbolTable);
        symbolTable[varName] = castRuntimeValueToType(value, varType);
    } else {
        const double value = evaluateExpression(tokens, start + 3, semicolonIndex - 1, symbolTable);
        symbolTable[varName] = RuntimeValue(varType, value);
    }

    return semicolonIndex + 1;
}

inline size_t executeMemberAssignment(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (
        start + 4 >= tokens.size() ||
        tokens[start].type != TokenType::IDENTIFIER ||
        tokens[start + 1].type != TokenType::DOT ||
        tokens[start + 2].type != TokenType::IDENTIFIER ||
        !isAssignmentOperatorToken(tokens[start + 3])
    ) {
        throw std::runtime_error("Invalid member assignment syntax");
    }

    const std::string objectName = tokens[start].value;
    const std::string memberName = tokens[start + 2].value;

    auto it = symbolTable.find(objectName);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Assignment to unknown identifier: " + objectName);
    }

    size_t semicolonIndex = start + 4;
    while (semicolonIndex < tokens.size() && tokens[semicolonIndex].type != TokenType::SEMICOLON) {
        semicolonIndex++;
    }
    if (semicolonIndex >= tokens.size()) {
        throw std::runtime_error("Missing ';' at end of member assignment");
    }

    const std::string assignmentOp = tokens[start + 3].value;
    const float rhsValue = static_cast<float>(evaluateExpression(tokens, start + 4, semicolonIndex - 1, symbolTable));
    RuntimeValue& obj = it->second;
    if (obj.typeName == "vec2") {
        if (memberName == "x") obj.vec2Val.x = static_cast<float>(applyScalarAssignmentOperator(obj.vec2Val.x, rhsValue, assignmentOp));
        else if (memberName == "y") obj.vec2Val.y = static_cast<float>(applyScalarAssignmentOperator(obj.vec2Val.y, rhsValue, assignmentOp));
        else throw std::runtime_error("Unknown vec2 member: " + memberName);
    } else if (obj.typeName == "vec3") {
        if (memberName == "x") obj.vec3Val.x = static_cast<float>(applyScalarAssignmentOperator(obj.vec3Val.x, rhsValue, assignmentOp));
        else if (memberName == "y") obj.vec3Val.y = static_cast<float>(applyScalarAssignmentOperator(obj.vec3Val.y, rhsValue, assignmentOp));
        else if (memberName == "z") obj.vec3Val.z = static_cast<float>(applyScalarAssignmentOperator(obj.vec3Val.z, rhsValue, assignmentOp));
        else throw std::runtime_error("Unknown vec3 member: " + memberName);
    } else if (obj.typeName == "vec4") {
        if (memberName == "x") obj.vec4Val.x = static_cast<float>(applyScalarAssignmentOperator(obj.vec4Val.x, rhsValue, assignmentOp));
        else if (memberName == "y") obj.vec4Val.y = static_cast<float>(applyScalarAssignmentOperator(obj.vec4Val.y, rhsValue, assignmentOp));
        else if (memberName == "z") obj.vec4Val.z = static_cast<float>(applyScalarAssignmentOperator(obj.vec4Val.z, rhsValue, assignmentOp));
        else if (memberName == "w") obj.vec4Val.w = static_cast<float>(applyScalarAssignmentOperator(obj.vec4Val.w, rhsValue, assignmentOp));
        else throw std::runtime_error("Unknown vec4 member: " + memberName);
    } else {
        throw std::runtime_error("Member assignment is only supported for vec types");
    }

    return semicolonIndex + 1;
}

inline size_t executeAssignment(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start + 2 >= tokens.size()) {
        throw std::runtime_error("Incomplete assignment");
    }

    if (
        tokens[start].type != TokenType::IDENTIFIER ||
        !isAssignmentOperatorToken(tokens[start + 1])
    ) {
        throw std::runtime_error("Invalid assignment syntax");
    }

    const std::string varName = tokens[start].value;
    auto it = symbolTable.find(varName);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Assignment to unknown variable: " + varName);
    }

    size_t semicolonIndex = start + 2;
    while (semicolonIndex < tokens.size() && tokens[semicolonIndex].type != TokenType::SEMICOLON) {
        semicolonIndex++;
    }

    if (semicolonIndex >= tokens.size()) {
        throw std::runtime_error("Missing ';' at end of assignment");
    }

    if (semicolonIndex == start + 2) {
        throw std::runtime_error("Missing expression in assignment");
    }

    const std::string assignmentOp = tokens[start + 1].value;
    const std::string varType = it->second.typeName;
    if (varType == "string" && tokens[start + 2].type == TokenType::STRING && assignmentOp == "=") {
        symbolTable[varName] = RuntimeValue(varType, tokens[start + 2].value);
    } else if (varType == "list" && tokens[start + 2].type == TokenType::LBRACKET) {
        if (assignmentOp != "=") {
            throw std::runtime_error("Compound assignment is not supported for list");
        }
        RuntimeValue listVal(varType, "");
        listVal.type = GETypes::VariableType::LIST;

        size_t i = start + 3;
        while (i < semicolonIndex && tokens[i].type != TokenType::RBRACKET) {
            if (tokens[i].type == TokenType::NUMBER) {
                if (tokens[i].value.find('.') != std::string::npos) {
                    listVal.listVal.add(GETypes::FloatType(static_cast<float>(std::stod(tokens[i].value))));
                } else {
                    listVal.listVal.add(GETypes::IntType(std::stoi(tokens[i].value)));
                }
            } else if (tokens[i].type == TokenType::STRING) {
                listVal.listVal.add(GETypes::StringType(tokens[i].value));
            } else if (tokens[i].type == TokenType::IDENTIFIER) {
                auto rit = symbolTable.find(tokens[i].value);
                if (rit != symbolTable.end()) {
                    listVal.listVal.add(runtimeToListValue(rit->second));
                }
            }
            i++;
        }

        symbolTable[varName] = listVal;
    } else if (isVectorTypeName(varType)) {
        RuntimeValue rhs = evaluateRuntimeExpression(tokens, start + 2, semicolonIndex - 1, symbolTable);
        symbolTable[varName] = applyCompoundVectorAssignment(it->second, rhs, assignmentOp);
    } else {
        const double rhs = evaluateExpression(tokens, start + 2, semicolonIndex - 1, symbolTable);
        const double lhs = it->second.getNumberValue();
        const double assigned = applyScalarAssignmentOperator(lhs, rhs, assignmentOp);
        if (varType == "int") {
            symbolTable[varName] = RuntimeValue(varType, static_cast<double>(static_cast<int>(assigned)));
        } else if (varType == "bool") {
            symbolTable[varName] = RuntimeValue(varType, assigned != 0.0 ? 1.0 : 0.0);
        } else {
            symbolTable[varName] = RuntimeValue(varType, assigned);
        }
    }

    return semicolonIndex + 1;
}

inline size_t executeMethodCallStatement(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (!isMethodCallStart(tokens, start)) {
        throw std::runtime_error("Invalid method call statement");
    }

    const std::string objectName = tokens[start].value;
    const std::string methodName = tokens[start + 2].value;
    const size_t closeParen = findMatchingParen(tokens, start + 3);

    size_t semicolonIndex = closeParen + 1;
    while (semicolonIndex < tokens.size() && tokens[semicolonIndex].type != TokenType::SEMICOLON) {
        semicolonIndex++;
    }
    if (semicolonIndex >= tokens.size()) {
        throw std::runtime_error("Missing ';' at end of method call");
    }

    std::vector<RuntimeValue> args = parseArgumentList(tokens, start + 4, closeParen - 1, symbolTable);

    if (objectName == "window") {
        executeWindowMethodStatement(methodName, args);
        return semicolonIndex + 1;
    }

    auto objIt = symbolTable.find(objectName);
    if (objIt == symbolTable.end()) {
        throw std::runtime_error("Unknown object: " + objectName);
    }
    if (objIt->second.typeName != "list") {
        throw std::runtime_error("Method calls currently supported only on list objects and window");
    }

    RuntimeValue& obj = objIt->second;

    if (methodName == "add") {
        if (args.size() != 1) {
            throw std::runtime_error("list.add expects exactly 1 argument");
        }
        obj.listVal.add(runtimeToListValue(args[0]));
        return semicolonIndex + 1;
    }

    if (methodName == "remove") {
        if (args.size() != 1) {
            throw std::runtime_error("list.remove expects exactly 1 argument");
        }
        size_t index = static_cast<size_t>(args[0].getNumberValue());
        obj.listVal.remove(index);
        return semicolonIndex + 1;
    }

    if (methodName == "clear") {
        if (!args.empty()) {
            throw std::runtime_error("list.clear expects no arguments");
        }
        obj.listVal.clear();
        return semicolonIndex + 1;
    }

    // Read-only methods are also valid as statements, return value is ignored.
    if (methodName == "size" || methodName == "get" || methodName == "indexOf") {
        (void)executeMethodCallReadOnly(tokens, start, symbolTable);
        return semicolonIndex + 1;
    }

    throw std::runtime_error("Unknown list method: " + methodName);
}

inline RuntimeValue executeBuiltinFunction(const std::string& funcName, const std::vector<RuntimeValue>& args) {
    constexpr double kPi = 3.141593652589;
    constexpr double kDegToRad = kPi / 180.0;

    if (funcName == "vec2") {
        if (args.size() == 1) {
            RuntimeValue v = makeDefaultValueForType("vec2");
            if (args[0].typeName == "vec2") {
                v.vec2Val = args[0].vec2Val;
            } else {
                double n = args[0].getNumberValue();
                v.vec2Val = GETypes::Vec2Type(n);
            }
            return v;
        }
        if (args.size() == 2) {
            RuntimeValue v = makeDefaultValueForType("vec2");
            double x_val = args[0].getNumberValue();
            double y_val = args[1].getNumberValue();
            v.vec2Val = GETypes::Vec2Type(static_cast<float>(x_val), static_cast<float>(y_val));
            return v;
        }
        throw std::runtime_error("vec2 expects 1 or 2 arguments");
    }

    if (funcName == "vec3") {
        RuntimeValue v = makeDefaultValueForType("vec3");
        if (args.size() == 1) {
            if (args[0].typeName == "vec3") {
                v.vec3Val = args[0].vec3Val;
            } else {
                double n = args[0].getNumberValue();
                v.vec3Val = GETypes::Vec3Type(n);
            }
            return v;
        }
        if (args.size() == 2 && args[0].typeName == "vec2") {
            v.vec3Val = GETypes::Vec3Type(args[0].vec2Val, args[1].getNumberValue());
            return v;
        }
        if (args.size() == 2 && args[1].typeName == "vec2") {
            v.vec3Val = GETypes::Vec3Type(args[0].getNumberValue(), args[1].vec2Val);
            return v;
        }
        if (args.size() == 3) {
            v.vec3Val = GETypes::Vec3Type(
                args[0].getNumberValue(),
                args[1].getNumberValue(),
                args[2].getNumberValue()
            );
            return v;
        }
        throw std::runtime_error("vec3 constructor mismatch");
    }

    if (funcName == "vec4") {
        RuntimeValue v = makeDefaultValueForType("vec4");
        if (args.size() == 1) {
            if (args[0].typeName == "vec4") {
                v.vec4Val = args[0].vec4Val;
            } else {
                double n = args[0].getNumberValue();
                v.vec4Val = GETypes::Vec4Type(n);
            }
            return v;
        }
        if (args.size() == 2 && args[0].typeName == "vec3") {
            v.vec4Val = GETypes::Vec4Type(args[0].vec3Val, args[1].getNumberValue());
            return v;
        }
        if (args.size() == 2 && args[1].typeName == "vec3") {
            v.vec4Val = GETypes::Vec4Type(args[0].getNumberValue(), args[1].vec3Val);
            return v;
        }
        if (args.size() == 2 && args[0].typeName == "vec2" && args[1].typeName == "vec2") {
            v.vec4Val = GETypes::Vec4Type(args[0].vec2Val, args[1].vec2Val);
            return v;
        }
        if (args.size() == 4) {
            v.vec4Val = GETypes::Vec4Type(
                args[0].getNumberValue(),
                args[1].getNumberValue(),
                args[2].getNumberValue(),
                args[3].getNumberValue()
            );
            return v;
        }
        throw std::runtime_error("vec4 constructor mismatch");
    }

    if (funcName == "sin") {
        if (args.size() != 1) {
            throw std::runtime_error("sin expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(
                std::sin(args[0].vec2Val.x * kDegToRad),
                std::sin(args[0].vec2Val.y * kDegToRad)
            );
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                std::sin(args[0].vec3Val.x * kDegToRad),
                std::sin(args[0].vec3Val.y * kDegToRad),
                std::sin(args[0].vec3Val.z * kDegToRad)
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::sin(args[0].vec4Val.x * kDegToRad),
                std::sin(args[0].vec4Val.y * kDegToRad),
                std::sin(args[0].vec4Val.z * kDegToRad),
                std::sin(args[0].vec4Val.w * kDegToRad)
            );
            return out;
        }
        return RuntimeValue("float", std::sin(args[0].getNumberValue() * kDegToRad));
    }

    if (funcName == "cos") {
        if (args.size() != 1) {
            throw std::runtime_error("cos expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(
                std::cos(args[0].vec2Val.x * kDegToRad),
                std::cos(args[0].vec2Val.y * kDegToRad)
            );
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                std::cos(args[0].vec3Val.x * kDegToRad),
                std::cos(args[0].vec3Val.y * kDegToRad),
                std::cos(args[0].vec3Val.z * kDegToRad)
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::cos(args[0].vec4Val.x * kDegToRad),
                std::cos(args[0].vec4Val.y * kDegToRad),
                std::cos(args[0].vec4Val.z * kDegToRad),
                std::cos(args[0].vec4Val.w * kDegToRad)
            );
            return out;
        }
        return RuntimeValue("float", std::cos(args[0].getNumberValue() * kDegToRad));
    }

    if (funcName == "tan") {
        if (args.size() != 1) {
            throw std::runtime_error("tan expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(
                std::tan(args[0].vec2Val.x * kDegToRad),
                std::tan(args[0].vec2Val.y * kDegToRad)
            );
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                std::tan(args[0].vec3Val.x * kDegToRad),
                std::tan(args[0].vec3Val.y * kDegToRad),
                std::tan(args[0].vec3Val.z * kDegToRad)
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::tan(args[0].vec4Val.x * kDegToRad),
                std::tan(args[0].vec4Val.y * kDegToRad),
                std::tan(args[0].vec4Val.z * kDegToRad),
                std::tan(args[0].vec4Val.w * kDegToRad)
            );
            return out;
        }
        return RuntimeValue("float", std::tan(args[0].getNumberValue() * kDegToRad));
    }

    if (funcName == "length") {
        if (args.size() != 1) {
            throw std::runtime_error("length expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") return RuntimeValue("float", GETypes::length(args[0].vec2Val));
        if (args[0].typeName == "vec3") return RuntimeValue("float", GETypes::length(args[0].vec3Val));
        if (args[0].typeName == "vec4") return RuntimeValue("float", GETypes::length(args[0].vec4Val));
        return RuntimeValue("float", std::abs(args[0].getNumberValue()));
    }

    if (funcName == "dot") {
        if (args.size() != 2) {
            throw std::runtime_error("dot expects exactly 2 arguments");
        }
        if (args[0].typeName == "vec2" && args[1].typeName == "vec2") return RuntimeValue("float", GETypes::dot(args[0].vec2Val, args[1].vec2Val));
        if (args[0].typeName == "vec3" && args[1].typeName == "vec3") return RuntimeValue("float", GETypes::dot(args[0].vec3Val, args[1].vec3Val));
        if (args[0].typeName == "vec4" && args[1].typeName == "vec4") return RuntimeValue("float", GETypes::dot(args[0].vec4Val, args[1].vec4Val));
        throw std::runtime_error("dot overload mismatch");
    }

    if (funcName == "normalize") {
        if (args.size() != 1) {
            throw std::runtime_error("normalize expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::normalize(args[0].vec2Val);
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::normalize(args[0].vec3Val);
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::normalize(args[0].vec4Val);
            return out;
        }
        throw std::runtime_error("normalize overload mismatch");
    }

    if (funcName == "cross") {
        if (args.size() == 2 && args[0].typeName == "vec3" && args[1].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::cross(args[0].vec3Val, args[1].vec3Val);
            return out;
        }
        throw std::runtime_error("cross expects 2 vec3 arguments");
    }

    if (funcName == "min") {
        if (args.size() != 2) {
            throw std::runtime_error("min expects exactly 2 arguments");
        }
        if (args[0].typeName == "vec2" && args[1].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::min(args[0].vec2Val.x, args[1].vec2Val.x), std::min(args[0].vec2Val.y, args[1].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec2") {
            float b = static_cast<float>(args[1].getNumberValue());
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::min(args[0].vec2Val.x, b), std::min(args[0].vec2Val.y, b));
            return out;
        }
        if (args[0].typeName == "vec3" && args[1].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                std::min(args[0].vec3Val.x, args[1].vec3Val.x),
                std::min(args[0].vec3Val.y, args[1].vec3Val.y),
                std::min(args[0].vec3Val.z, args[1].vec3Val.z)
            );
            return out;
        }
        if (args[0].typeName == "vec3") {
            float b = static_cast<float>(args[1].getNumberValue());
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(std::min(args[0].vec3Val.x, b), std::min(args[0].vec3Val.y, b), std::min(args[0].vec3Val.z, b));
            return out;
        }
        if (args[0].typeName == "vec4" && args[1].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::min(args[0].vec4Val.x, args[1].vec4Val.x),
                std::min(args[0].vec4Val.y, args[1].vec4Val.y),
                std::min(args[0].vec4Val.z, args[1].vec4Val.z),
                std::min(args[0].vec4Val.w, args[1].vec4Val.w)
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            float b = static_cast<float>(args[1].getNumberValue());
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::min(args[0].vec4Val.x, b),
                std::min(args[0].vec4Val.y, b),
                std::min(args[0].vec4Val.z, b),
                std::min(args[0].vec4Val.w, b)
            );
            return out;
        }
        return RuntimeValue("float", std::min(args[0].getNumberValue(), args[1].getNumberValue()));
    }

    if (funcName == "max") {
        if (args.size() != 2) {
            throw std::runtime_error("max expects exactly 2 arguments");
        }
        if (args[0].typeName == "vec2" && args[1].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::max(args[0].vec2Val.x, args[1].vec2Val.x), std::max(args[0].vec2Val.y, args[1].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec2") {
            float b = static_cast<float>(args[1].getNumberValue());
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::max(args[0].vec2Val.x, b), std::max(args[0].vec2Val.y, b));
            return out;
        }
        if (args[0].typeName == "vec3" && args[1].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                std::max(args[0].vec3Val.x, args[1].vec3Val.x),
                std::max(args[0].vec3Val.y, args[1].vec3Val.y),
                std::max(args[0].vec3Val.z, args[1].vec3Val.z)
            );
            return out;
        }
        if (args[0].typeName == "vec3") {
            float b = static_cast<float>(args[1].getNumberValue());
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(std::max(args[0].vec3Val.x, b), std::max(args[0].vec3Val.y, b), std::max(args[0].vec3Val.z, b));
            return out;
        }
        if (args[0].typeName == "vec4" && args[1].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::max(args[0].vec4Val.x, args[1].vec4Val.x),
                std::max(args[0].vec4Val.y, args[1].vec4Val.y),
                std::max(args[0].vec4Val.z, args[1].vec4Val.z),
                std::max(args[0].vec4Val.w, args[1].vec4Val.w)
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            float b = static_cast<float>(args[1].getNumberValue());
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::max(args[0].vec4Val.x, b),
                std::max(args[0].vec4Val.y, b),
                std::max(args[0].vec4Val.z, b),
                std::max(args[0].vec4Val.w, b)
            );
            return out;
        }
        return RuntimeValue("float", std::max(args[0].getNumberValue(), args[1].getNumberValue()));
    }

    if (funcName == "clamp") {
        if (args.size() != 3) {
            throw std::runtime_error("clamp expects exactly 3 arguments");
        }
        const float minv = static_cast<float>(args[1].getNumberValue());
        const float maxv = static_cast<float>(args[2].getNumberValue());
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(
                std::max(minv, std::min(args[0].vec2Val.x, maxv)),
                std::max(minv, std::min(args[0].vec2Val.y, maxv))
            );
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                std::max(minv, std::min(args[0].vec3Val.x, maxv)),
                std::max(minv, std::min(args[0].vec3Val.y, maxv)),
                std::max(minv, std::min(args[0].vec3Val.z, maxv))
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                std::max(minv, std::min(args[0].vec4Val.x, maxv)),
                std::max(minv, std::min(args[0].vec4Val.y, maxv)),
                std::max(minv, std::min(args[0].vec4Val.z, maxv)),
                std::max(minv, std::min(args[0].vec4Val.w, maxv))
            );
            return out;
        }
        const float x = static_cast<float>(args[0].getNumberValue());
        return RuntimeValue("float", std::max(minv, std::min(x, maxv)));
    }

    if (funcName == "abs") {
        if (args.size() != 1) {
            throw std::runtime_error("abs expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::abs(args[0].vec2Val.x), std::abs(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(std::abs(args[0].vec3Val.x), std::abs(args[0].vec3Val.y), std::abs(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(std::abs(args[0].vec4Val.x), std::abs(args[0].vec4Val.y), std::abs(args[0].vec4Val.z), std::abs(args[0].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", std::abs(args[0].getNumberValue()));
    }

    if (funcName == "floor") {
        if (args.size() != 1) {
            throw std::runtime_error("floor expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::floor(args[0].vec2Val.x), std::floor(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(std::floor(args[0].vec3Val.x), std::floor(args[0].vec3Val.y), std::floor(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(std::floor(args[0].vec4Val.x), std::floor(args[0].vec4Val.y), std::floor(args[0].vec4Val.z), std::floor(args[0].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", std::floor(args[0].getNumberValue()));
    }

    if (funcName == "ceil") {
        if (args.size() != 1) {
            throw std::runtime_error("ceil expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::ceil(args[0].vec2Val.x), std::ceil(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(std::ceil(args[0].vec3Val.x), std::ceil(args[0].vec3Val.y), std::ceil(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(std::ceil(args[0].vec4Val.x), std::ceil(args[0].vec4Val.y), std::ceil(args[0].vec4Val.z), std::ceil(args[0].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", std::ceil(args[0].getNumberValue()));
    }

    if (funcName == "fract") {
        if (args.size() != 1) {
            throw std::runtime_error("fract expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(args[0].vec2Val.x - std::floor(args[0].vec2Val.x), args[0].vec2Val.y - std::floor(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(
                args[0].vec3Val.x - std::floor(args[0].vec3Val.x),
                args[0].vec3Val.y - std::floor(args[0].vec3Val.y),
                args[0].vec3Val.z - std::floor(args[0].vec3Val.z)
            );
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                args[0].vec4Val.x - std::floor(args[0].vec4Val.x),
                args[0].vec4Val.y - std::floor(args[0].vec4Val.y),
                args[0].vec4Val.z - std::floor(args[0].vec4Val.z),
                args[0].vec4Val.w - std::floor(args[0].vec4Val.w)
            );
            return out;
        }
        const double x = args[0].getNumberValue();
        return RuntimeValue("float", x - std::floor(x));
    }

    if (funcName == "sign") {
        auto sign1 = [](double x) -> double { return x > 0.0 ? 1.0 : (x < 0.0 ? -1.0 : 0.0); };
        if (args.size() != 1) {
            throw std::runtime_error("sign expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(sign1(args[0].vec2Val.x), sign1(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(sign1(args[0].vec3Val.x), sign1(args[0].vec3Val.y), sign1(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(
                sign1(args[0].vec4Val.x),
                sign1(args[0].vec4Val.y),
                sign1(args[0].vec4Val.z),
                sign1(args[0].vec4Val.w)
            );
            return out;
        }
        return RuntimeValue("float", sign1(args[0].getNumberValue()));
    }

    if (funcName == "smoothstep") {
        if (args.size() != 3) {
            throw std::runtime_error("smoothstep expects exactly 3 arguments");
        }
        const double e0 = args[0].getNumberValue();
        const double e1 = args[1].getNumberValue();
        auto sstep = [e0, e1](double x) -> double {
            double t = (x - e0) / (e1 - e0);
            t = std::max(0.0, std::min(t, 1.0));
            return t * t * (3.0 - 2.0 * t);
        };
        if (args[2].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(sstep(args[2].vec2Val.x), sstep(args[2].vec2Val.y));
            return out;
        }
        if (args[2].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(sstep(args[2].vec3Val.x), sstep(args[2].vec3Val.y), sstep(args[2].vec3Val.z));
            return out;
        }
        if (args[2].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(sstep(args[2].vec4Val.x), sstep(args[2].vec4Val.y), sstep(args[2].vec4Val.z), sstep(args[2].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", sstep(args[2].getNumberValue()));
    }

    if (funcName == "radians") {
        if (args.size() != 1) {
            throw std::runtime_error("radians expects exactly 1 argument");
        }
        auto rad = [](double x) -> double { return x * kPi / 180.0; };
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(rad(args[0].vec2Val.x), rad(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(rad(args[0].vec3Val.x), rad(args[0].vec3Val.y), rad(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(rad(args[0].vec4Val.x), rad(args[0].vec4Val.y), rad(args[0].vec4Val.z), rad(args[0].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", rad(args[0].getNumberValue()));
    }

    if (funcName == "degrees") {
        if (args.size() != 1) {
            throw std::runtime_error("degrees expects exactly 1 argument");
        }
        auto deg = [](double x) -> double { return (x * 180.0) / kPi; };
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(deg(args[0].vec2Val.x), deg(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(deg(args[0].vec3Val.x), deg(args[0].vec3Val.y), deg(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(deg(args[0].vec4Val.x), deg(args[0].vec4Val.y), deg(args[0].vec4Val.z), deg(args[0].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", deg(args[0].getNumberValue()));
    }

    if (funcName == "mix") {
        if (args.size() != 3) {
            throw std::runtime_error("mix expects exactly 3 arguments");
        }
        const double x = args[0].getNumberValue();
        const double y = args[1].getNumberValue();
        const double a = args[2].getNumberValue();
        return RuntimeValue("float", x * (1.0 - a) + y * a);
    }

    if (funcName == "sqrt") {
        if (args.size() != 1) {
            throw std::runtime_error("sqrt expects exactly 1 argument");
        }
        if (args[0].typeName == "vec2") {
            RuntimeValue out = makeDefaultValueForType("vec2");
            out.vec2Val = GETypes::Vec2Type(std::sqrt(args[0].vec2Val.x), std::sqrt(args[0].vec2Val.y));
            return out;
        }
        if (args[0].typeName == "vec3") {
            RuntimeValue out = makeDefaultValueForType("vec3");
            out.vec3Val = GETypes::Vec3Type(std::sqrt(args[0].vec3Val.x), std::sqrt(args[0].vec3Val.y), std::sqrt(args[0].vec3Val.z));
            return out;
        }
        if (args[0].typeName == "vec4") {
            RuntimeValue out = makeDefaultValueForType("vec4");
            out.vec4Val = GETypes::Vec4Type(std::sqrt(args[0].vec4Val.x), std::sqrt(args[0].vec4Val.y), std::sqrt(args[0].vec4Val.z), std::sqrt(args[0].vec4Val.w));
            return out;
        }
        return RuntimeValue("float", std::sqrt(args[0].getNumberValue()));
    }

    throw std::runtime_error("Unknown builtin function: " + funcName);
}

inline bool isBuiltinFunctionName(const std::string& name) {
    return name == "vec2" || name == "vec3" || name == "vec4" || name == "sin" ||
           name == "cos" || name == "tan" || name == "length" || name == "dot" ||
           name == "normalize" || name == "cross" || name == "min" || name == "max" ||
           name == "clamp" || name == "abs" || name == "floor" || name == "ceil" ||
           name == "fract" || name == "sign" || name == "smoothstep" || name == "radians" ||
           name == "degrees" || name == "mix" || name == "sqrt";
}

inline size_t executeFunctionCallStatement(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start + 1 >= tokens.size() || tokens[start].type != TokenType::IDENTIFIER || tokens[start + 1].type != TokenType::LPAREN) {
        throw std::runtime_error("Invalid function call statement");
    }
    const size_t closeParen = findMatchingParen(tokens, start + 1);
    size_t semicolonIndex = closeParen + 1;
    while (semicolonIndex < tokens.size() && tokens[semicolonIndex].type != TokenType::SEMICOLON) {
        semicolonIndex++;
    }
    if (semicolonIndex >= tokens.size()) {
        throw std::runtime_error("Missing ';' at end of function call");
    }
    (void)callFunction(tokens, start, symbolTable);
    return semicolonIndex + 1;
}

inline size_t executePrint(
    const std::vector<Token>& tokens,
    size_t start,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start + 1 >= tokens.size() || tokens[start].type != TokenType::PRINT) {
        throw std::runtime_error("Invalid print statement");
    }
    
    if (tokens[start + 1].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after print");
    }
    
    size_t rparen = findMatchingParen(tokens, start + 1);
    
    if (rparen >= tokens.size()) {
        throw std::runtime_error("Expected ')' in print statement");
    }
    
    // Handle print with single argument
    if (rparen == start + 2) {
        throw std::runtime_error("Print requires an argument");
    }
    
    RuntimeValue printArg = evaluateRuntimeExpression(tokens, start + 2, rparen - 1, symbolTable);
    printValue(printArg);
    
    // Find the semicolon
    size_t semi = rparen + 1;
    while (semi < tokens.size() && tokens[semi].type != TokenType::SEMICOLON) {
        semi++;
    }
    
    return semi + 1;
}

inline size_t findMatchingBrace(const std::vector<Token>& tokens, size_t openBrace) {
    if (tokens[openBrace].type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{'");
    }
    
    int braceCount = 1;
    size_t i = openBrace + 1;
    while (i < tokens.size() && braceCount > 0) {
        if (tokens[i].type == TokenType::LBRACE) braceCount++;
        if (tokens[i].type == TokenType::RBRACE) braceCount--;
        if (braceCount == 0) return i;
        i++;
    }
    throw std::runtime_error("Mismatched braces");
}

// Forward declarations for control flow executors used by executeFunction.
size_t executeIf(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable);
size_t executeFor(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable);
size_t executeWhile(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable);

enum class FunctionPlannedStatementKind {
    RETURN_STMT,
    DECLARATION,
    ASSIGNMENT,
    MEMBER_ASSIGNMENT,
    METHOD_CALL,
    PRINT,
    IF_STMT,
    FOR_STMT,
    WHILE_STMT,
    FUNCTION_CALL,
    UNKNOWN
};

struct FunctionPlannedStatement {
    size_t start;
    FunctionPlannedStatementKind kind;
};

inline FunctionPlannedStatementKind classifyFunctionBodyStatementAt(const std::vector<Token>& tokens, size_t i) {
    if (i >= tokens.size()) return FunctionPlannedStatementKind::UNKNOWN;

    if (tokens[i].type == TokenType::RETURN) return FunctionPlannedStatementKind::RETURN_STMT;
    if (tokens[i].type == TokenType::TYPE) return FunctionPlannedStatementKind::DECLARATION;
    if (tokens[i].type == TokenType::PRINT) return FunctionPlannedStatementKind::PRINT;
    if (tokens[i].type == TokenType::IF) return FunctionPlannedStatementKind::IF_STMT;
    if (tokens[i].type == TokenType::LOOP && tokens[i].value == "for") return FunctionPlannedStatementKind::FOR_STMT;
    if (tokens[i].type == TokenType::LOOP && tokens[i].value == "while") return FunctionPlannedStatementKind::WHILE_STMT;

    if (tokens[i].type == TokenType::IDENTIFIER) {
        if (i + 3 < tokens.size() && tokens[i + 1].type == TokenType::DOT && tokens[i + 3].type == TokenType::EQUAL) {
            return FunctionPlannedStatementKind::MEMBER_ASSIGNMENT;
        }
        if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::EQUAL) {
            return FunctionPlannedStatementKind::ASSIGNMENT;
        }
        if (isMethodCallStart(tokens, i)) {
            return FunctionPlannedStatementKind::METHOD_CALL;
        }
        if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::LPAREN) {
            return FunctionPlannedStatementKind::FUNCTION_CALL;
        }
    }

    return FunctionPlannedStatementKind::UNKNOWN;
}

inline size_t findNextFunctionBodyStatement(const std::vector<Token>& tokens, size_t start, FunctionPlannedStatementKind kind) {
    if (start >= tokens.size()) {
        return start;
    }

    if (kind == FunctionPlannedStatementKind::DECLARATION ||
        kind == FunctionPlannedStatementKind::ASSIGNMENT ||
        kind == FunctionPlannedStatementKind::MEMBER_ASSIGNMENT ||
        kind == FunctionPlannedStatementKind::METHOD_CALL ||
        kind == FunctionPlannedStatementKind::FUNCTION_CALL ||
        kind == FunctionPlannedStatementKind::PRINT ||
        kind == FunctionPlannedStatementKind::RETURN_STMT) {
        size_t i = start;
        while (i < tokens.size() && tokens[i].type != TokenType::SEMICOLON) {
            i++;
        }
        return (i < tokens.size()) ? (i + 1) : tokens.size();
    }

    if (kind == FunctionPlannedStatementKind::IF_STMT ||
        kind == FunctionPlannedStatementKind::FOR_STMT ||
        kind == FunctionPlannedStatementKind::WHILE_STMT) {
        if (start + 1 >= tokens.size() || tokens[start + 1].type != TokenType::LPAREN) {
            return std::min(tokens.size(), start + 1);
        }

        size_t condEnd = start + 2;
        int parenDepth = 1;
        while (condEnd < tokens.size() && parenDepth > 0) {
            if (tokens[condEnd].type == TokenType::LPAREN) parenDepth++;
            if (tokens[condEnd].type == TokenType::RPAREN) parenDepth--;
            if (parenDepth > 0) condEnd++;
        }

        if (parenDepth != 0 || condEnd + 1 >= tokens.size() || tokens[condEnd + 1].type != TokenType::LBRACE) {
            return std::min(tokens.size(), start + 1);
        }

        size_t next = findMatchingBrace(tokens, condEnd + 1) + 1;
        if (kind == FunctionPlannedStatementKind::IF_STMT &&
            next < tokens.size() &&
            tokens[next].type == TokenType::ELSE &&
            next + 1 < tokens.size() &&
            tokens[next + 1].type == TokenType::LBRACE) {
            next = findMatchingBrace(tokens, next + 1) + 1;
        }
        return next;
    }

    return std::min(tokens.size(), start + 1);
}

inline std::vector<FunctionPlannedStatement> buildFunctionBodyPlan(const std::vector<Token>& tokens) {
    std::vector<FunctionPlannedStatement> plan;
    size_t i = 0;
    while (i < tokens.size() && tokens[i].type != TokenType::END_OF_FILE) {
        FunctionPlannedStatementKind kind = classifyFunctionBodyStatementAt(tokens, i);
        if (kind != FunctionPlannedStatementKind::UNKNOWN) {
            plan.push_back({i, kind});
        }

        size_t next = findNextFunctionBodyStatement(tokens, i, kind);
        i = (next > i) ? next : (i + 1);
    }
    return plan;
}

inline size_t parseFunctionDef(const std::vector<Token>& tokens, size_t start) {
    if (start < tokens.size()) {
        CurrentToken = tokens[start];
    }
    // Format: returnType funcName ( param1, param2 ) { body }
    if (start + 2 >= tokens.size() || tokens[start].type != TokenType::TYPE) {
        throw std::runtime_error("Invalid function definition");
    }
    
    std::string returnType = tokens[start].value;
    std::string funcName = tokens[start + 1].value;
    
    if (tokens[start + 1].type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected function name");
    }
    
    if (tokens[start + 2].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after function name");
    }
    
    // Parse parameters
    std::vector<Parameter> params;
    size_t i = start + 3;
    while (i < tokens.size() && tokens[i].type != TokenType::RPAREN) {
        if (tokens[i].type == TokenType::TYPE) {
            std::string paramType = tokens[i].value;
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::IDENTIFIER) {
                params.push_back({paramType, tokens[i + 1].value});
                i += 2;
                if (i < tokens.size() && tokens[i].type == TokenType::COMMA) {
                    i++;
                }
            } else {
                throw std::runtime_error("Expected parameter name");
            }
        } else {
            i++;
        }
    }
    
    if (i >= tokens.size() || tokens[i].type != TokenType::RPAREN) {
        throw std::runtime_error("Expected ')' after parameters");
    }
    
    i++; // move past RPAREN
    if (i >= tokens.size() || tokens[i].type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{' for function body");
    }
    
    size_t bodyStart = i + 1;
    size_t bodyEnd = findMatchingBrace(tokens, i);
    
    FunctionDef func;
    func.returnType = returnType;
    func.name = funcName;
    func.parameters = params;
    func.bodyStart = bodyStart;
    func.bodyEnd = bodyEnd;
    func.bodyTokens.assign(tokens.begin() + bodyStart, tokens.begin() + bodyEnd);
    
    functions[buildFunctionKeyFromParams(funcName, params)] = func;
    
    return bodyEnd + 1;
}

inline ReturnValue executeFunction(const std::string& funcName, const std::vector<RuntimeValue>& args, const std::vector<Token>& tokens, std::unordered_map<std::string, RuntimeValue>& globalSymbolTable) {
    auto funcIt = functions.find(funcName);
    if (funcIt == functions.end()) {
        throw std::runtime_error("Unknown function: " + funcName);
    }
    
    const FunctionDef& func = funcIt->second;
    if (args.size() != func.parameters.size()) {
        throw std::runtime_error("Function " + funcName + " expects " + std::to_string(func.parameters.size()) + " arguments");
    }
    
    // Execute against the parent symbol table and only shadow parameter/local names.
    std::unordered_map<std::string, RuntimeValue>& localSymbolTable = globalSymbolTable;

    struct ScopedBindings {
        std::unordered_map<std::string, RuntimeValue>& symbols;
        std::unordered_map<std::string, RuntimeValue> shadowedValues;
        std::unordered_set<std::string> transientNames;

        explicit ScopedBindings(std::unordered_map<std::string, RuntimeValue>& table)
            : symbols(table) {}

        void capture(const std::string& name) {
            if (shadowedValues.find(name) != shadowedValues.end() || transientNames.count(name) != 0) {
                return;
            }

            auto it = symbols.find(name);
            if (it != symbols.end()) {
                shadowedValues.emplace(name, it->second);
            } else {
                transientNames.insert(name);
            }
        }

        ~ScopedBindings() {
            for (const auto& name : transientNames) {
                symbols.erase(name);
            }
            for (const auto& entry : shadowedValues) {
                symbols[entry.first] = entry.second;
            }
        }
    } scopedBindings(localSymbolTable);

    std::unordered_set<std::string> parameterNames;
    parameterNames.reserve(func.parameters.size());
    for (size_t i = 0; i < args.size(); i++) {
        scopedBindings.capture(func.parameters[i].name);
        localSymbolTable[func.parameters[i].name] = args[i];
        parameterNames.insert(func.parameters[i].name);
    }

    std::unordered_set<std::string> locallyDeclared;
    
    // Use the function's stored body tokens instead of the global tokens
    const std::vector<Token>& bodyTokens = func.bodyTokens;
    static std::unordered_map<std::string, std::vector<FunctionPlannedStatement>> functionBodyPlanCache;
    auto planIt = functionBodyPlanCache.find(funcName);
    if (planIt == functionBodyPlanCache.end()) {
        planIt = functionBodyPlanCache.emplace(funcName, buildFunctionBodyPlan(bodyTokens)).first;
    }
    const std::vector<FunctionPlannedStatement>& bodyPlan = planIt->second;
    
    for (const auto& stmt : bodyPlan) {
        const size_t i = stmt.start;
        if (i >= bodyTokens.size()) {
            continue;
        }

        CurrentToken = bodyTokens[i];
        if (stmt.kind == FunctionPlannedStatementKind::RETURN_STMT) {
            size_t exprStart = i + 1;
            // Find the semicolon
            size_t semi = exprStart;
            while (semi < bodyTokens.size() && bodyTokens[semi].type != TokenType::SEMICOLON) {
                semi++;
            }
            
            // Evaluate return value
            if (semi == exprStart) {
                return ReturnValue();
            }
            
            // Check if it's a simple number
            if (bodyTokens[exprStart].type == TokenType::NUMBER && semi == exprStart + 1) {
                return ReturnValue(RuntimeValue(func.returnType, std::stod(bodyTokens[exprStart].value)));
            } 
            // Check if it's a simple identifier (not followed by operators)
            else if (bodyTokens[exprStart].type == TokenType::IDENTIFIER && semi == exprStart + 1) {
                auto it = localSymbolTable.find(bodyTokens[exprStart].value);
                if (it != localSymbolTable.end()) {
                    return ReturnValue(it->second);
                }
            }
            
            // Try to evaluate as expression
            if (semi > exprStart) {
                double result = evaluateExpression(bodyTokens, exprStart, semi - 1, localSymbolTable);
                return ReturnValue(RuntimeValue(func.returnType, result));
            }
            
            return ReturnValue();
        }
        
        if (stmt.kind == FunctionPlannedStatementKind::DECLARATION) {
            if (i + 1 < bodyTokens.size() && bodyTokens[i + 1].type == TokenType::IDENTIFIER) {
                const std::string& declaredName = bodyTokens[i + 1].value;
                if (locallyDeclared.insert(declaredName).second) {
                    scopedBindings.capture(declaredName);
                }
            }
            executeDeclaration(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::ASSIGNMENT) {
            executeAssignment(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::MEMBER_ASSIGNMENT) {
            executeMemberAssignment(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::METHOD_CALL) {
            executeMethodCallStatement(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::PRINT) {
            executePrint(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::IF_STMT) {
            executeIf(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::FOR_STMT) {
            executeFor(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::WHILE_STMT) {
            executeWhile(bodyTokens, i, localSymbolTable);
            continue;
        }

        if (stmt.kind == FunctionPlannedStatementKind::FUNCTION_CALL) {
            executeFunctionCallStatement(bodyTokens, i, localSymbolTable);
            continue;
        }
    }

    // If no explicit return, return default value
    if (func.returnType == "int" || func.returnType == "float") {
        return ReturnValue(RuntimeValue(func.returnType, 0.0));
    } else if (func.returnType == "string") {
        return ReturnValue(RuntimeValue(func.returnType, std::string()));
    }
    
    return ReturnValue();
}

inline RuntimeValue callFunction(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable) {
    // Format: funcName(arg1, arg2, ...)
    if (start < tokens.size()) {
        CurrentToken = tokens[start];
    }
    std::string funcName = tokens[start].value;
    
    std::vector<RuntimeValue> args;
    size_t i = start + 1;
    
    if (i >= tokens.size() || tokens[i].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after function name");
    }
    
    i++; // skip LPAREN
    size_t argStart = i;
    
    // Parse arguments
    int parenDepth = 1;
    while (i < tokens.size() && parenDepth > 0) {
        if (tokens[i].type == TokenType::LPAREN) parenDepth++;
        if (tokens[i].type == TokenType::RPAREN) parenDepth--;
        
        if (parenDepth == 0) {
            if (i > argStart) {
                args.push_back(evaluateRuntimeExpression(tokens, argStart, i - 1, symbolTable));
            }
        } else if (tokens[i].type == TokenType::COMMA && parenDepth == 1) {
            if (i > argStart) {
                args.push_back(evaluateRuntimeExpression(tokens, argStart, i - 1, symbolTable));
            }
            argStart = i + 1;
        }
        
        i++;
    }
    
    if (isBuiltinFunctionName(funcName)) {
        return executeBuiltinFunction(funcName, args);
    }

    std::vector<std::string> argTypes;
    argTypes.reserve(args.size());
    for (const auto& arg : args) {
        argTypes.push_back(getRuntimeTypeToken(arg));
    }

    const std::string typedKey = buildFunctionKey(funcName, argTypes);
    if (functions.find(typedKey) != functions.end()) {
        funcName = typedKey;
    } else {
        // Backward compatibility: allow old untyped entry if present.
        if (functions.find(funcName) == functions.end()) {
            throw std::runtime_error("Unknown function overload: " + typedKey);
        }
    }

    // Execute the function
    ReturnValue ret = executeFunction(funcName, args, tokens, symbolTable);
    
    if (ret.hasValue) {
        return ret.value;
    }
    
    return RuntimeValue();
}

inline RuntimeValue callFunction(const std::vector<Token>& tokens, size_t start, const std::unordered_map<std::string, RuntimeValue>& symbolTable) {
    // Most expression evaluators pass a const reference for convenience, but function
    // calls may need to mutate the active runtime scope. Forward to the mutable
    // overload to avoid copying the entire symbol table on every call.
    return callFunction(tokens, start, const_cast<std::unordered_map<std::string, RuntimeValue>&>(symbolTable));
}

// Forward declarations
size_t executeIf(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable);
size_t executeFor(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable);
size_t executeWhile(const std::vector<Token>& tokens, size_t start, std::unordered_map<std::string, RuntimeValue>& symbolTable);

inline bool evaluateCondition(
    const std::vector<Token>& tokens,
    size_t start,
    size_t end,
    const std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    const size_t comparisonIndex = findComparisonIndexAtDepthZero(tokens, start, end);
    return evaluateConditionWithComparisonIndex(tokens, start, end, comparisonIndex, symbolTable);
}

enum class PlannedStatementKind {
    DECLARATION,
    ASSIGNMENT,
    MEMBER_ASSIGNMENT,
    METHOD_CALL,
    FUNCTION_CALL,
    PRINT,
    IF_STMT,
    FOR_STMT,
    WHILE_STMT,
    UNKNOWN
};

struct PlannedStatement {
    size_t start;
    PlannedStatementKind kind;
};

inline PlannedStatementKind classifyStatementAt(const std::vector<Token>& tokens, size_t i) {
    if (i >= tokens.size()) return PlannedStatementKind::UNKNOWN;

    if (tokens[i].type == TokenType::TYPE) return PlannedStatementKind::DECLARATION;
    if (tokens[i].type == TokenType::PRINT) return PlannedStatementKind::PRINT;
    if (tokens[i].type == TokenType::IF) return PlannedStatementKind::IF_STMT;
    if (tokens[i].type == TokenType::LOOP && tokens[i].value == "for") return PlannedStatementKind::FOR_STMT;
    if (tokens[i].type == TokenType::LOOP && tokens[i].value == "while") return PlannedStatementKind::WHILE_STMT;

    if (tokens[i].type == TokenType::IDENTIFIER) {
        if (i + 3 < tokens.size() && tokens[i + 1].type == TokenType::DOT && isAssignmentOperatorToken(tokens[i + 3])) {
            return PlannedStatementKind::MEMBER_ASSIGNMENT;
        }
        if (i + 1 < tokens.size() && isAssignmentOperatorToken(tokens[i + 1])) {
            return PlannedStatementKind::ASSIGNMENT;
        }
        if (isMethodCallStart(tokens, i)) {
            return PlannedStatementKind::METHOD_CALL;
        }
        if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::LPAREN) {
            return PlannedStatementKind::FUNCTION_CALL;
        }
    }

    return PlannedStatementKind::UNKNOWN;
}

inline size_t findNextSemicolon(const std::vector<Token>& tokens, size_t start, size_t endExclusive) {
    size_t i = start;
    while (i < endExclusive && i < tokens.size() && tokens[i].type != TokenType::SEMICOLON) {
        i++;
    }
    if (i < tokens.size() && tokens[i].type == TokenType::SEMICOLON) {
        return i + 1;
    }
    return std::min(endExclusive, start + 1);
}

inline size_t nextStatementStart(
    const std::vector<Token>& tokens,
    size_t start,
    size_t endExclusive,
    PlannedStatementKind kind
) {
    if (kind == PlannedStatementKind::DECLARATION ||
        kind == PlannedStatementKind::ASSIGNMENT ||
        kind == PlannedStatementKind::MEMBER_ASSIGNMENT ||
        kind == PlannedStatementKind::METHOD_CALL ||
        kind == PlannedStatementKind::FUNCTION_CALL ||
        kind == PlannedStatementKind::PRINT) {
        return findNextSemicolon(tokens, start, endExclusive);
    }

    if (kind == PlannedStatementKind::IF_STMT || kind == PlannedStatementKind::FOR_STMT || kind == PlannedStatementKind::WHILE_STMT) {
        if (start + 1 >= tokens.size() || tokens[start + 1].type != TokenType::LPAREN) {
            return std::min(endExclusive, start + 1);
        }

        size_t condEnd = start + 2;
        int parenDepth = 1;
        while (condEnd < tokens.size() && parenDepth > 0) {
            if (tokens[condEnd].type == TokenType::LPAREN) parenDepth++;
            if (tokens[condEnd].type == TokenType::RPAREN) parenDepth--;
            if (parenDepth > 0) condEnd++;
        }

        if (parenDepth != 0 || condEnd + 1 >= tokens.size() || tokens[condEnd + 1].type != TokenType::LBRACE) {
            return std::min(endExclusive, start + 1);
        }

        size_t next = findMatchingBrace(tokens, condEnd + 1) + 1;

        if (kind == PlannedStatementKind::IF_STMT &&
            next < endExclusive &&
            next < tokens.size() &&
            tokens[next].type == TokenType::ELSE &&
            next + 1 < tokens.size() &&
            tokens[next + 1].type == TokenType::LBRACE) {
            next = findMatchingBrace(tokens, next + 1) + 1;
        }

        return std::min(next, endExclusive);
    }

    return std::min(endExclusive, start + 1);
}

inline std::vector<PlannedStatement> buildPlannedStatements(
    const std::vector<Token>& tokens,
    size_t start,
    size_t endExclusive
) {
    std::vector<PlannedStatement> plan;
    size_t i = start;
    while (i < endExclusive && i < tokens.size() && tokens[i].type != TokenType::END_OF_FILE) {
        PlannedStatementKind kind = classifyStatementAt(tokens, i);
        if (kind != PlannedStatementKind::UNKNOWN) {
            plan.push_back({i, kind});
        }
        size_t next = nextStatementStart(tokens, i, endExclusive, kind);
        i = (next > i) ? next : (i + 1);
    }
    return plan;
}

inline void executePlannedStatements(
    const std::vector<Token>& tokens,
    const std::vector<PlannedStatement>& plan,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    for (const auto& stmt : plan) {
        const size_t i = stmt.start;
        if (i >= tokens.size()) {
            continue;
        }
        CurrentToken = tokens[i];

        if (stmt.kind == PlannedStatementKind::DECLARATION) {
            executeDeclaration(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::ASSIGNMENT) {
            executeAssignment(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::MEMBER_ASSIGNMENT) {
            executeMemberAssignment(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::METHOD_CALL) {
            executeMethodCallStatement(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::FUNCTION_CALL) {
            executeFunctionCallStatement(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::PRINT) {
            executePrint(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::IF_STMT) {
            executeIf(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::FOR_STMT) {
            executeFor(tokens, i, symbolTable);
            continue;
        }
        if (stmt.kind == PlannedStatementKind::WHILE_STMT) {
            executeWhile(tokens, i, symbolTable);
            continue;
        }
    }
}

inline size_t executeIf(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start < tokens.size()) {
        CurrentToken = tokens[start];
    }
    if (start >= tokens.size() || tokens[start].type != TokenType::IF) {
        throw std::runtime_error("Expected 'if'");
    }
    
    if (start + 1 >= tokens.size() || tokens[start + 1].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after 'if'");
    }
    
    // Find matching closing paren
    size_t condEnd = start + 2;
    int parenDepth = 1;
    while (condEnd < tokens.size() && parenDepth > 0) {
        if (tokens[condEnd].type == TokenType::LPAREN) parenDepth++;
        if (tokens[condEnd].type == TokenType::RPAREN) parenDepth--;
        if (parenDepth > 0) condEnd++;
    }
    
    if (parenDepth != 0) {
        throw std::runtime_error("Mismatched parentheses in if condition");
    }
    
    bool condition = evaluateCondition(tokens, start + 2, condEnd - 1, symbolTable);
    
    if (condEnd + 1 >= tokens.size() || tokens[condEnd + 1].type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{' after if condition");
    }
    
    size_t bodyStart = condEnd + 2;
    size_t bodyEnd = findMatchingBrace(tokens, condEnd + 1);
    
    if (condition) {
        // Execute the if block
        size_t i = bodyStart;
        while (i < bodyEnd && tokens[i].type != TokenType::END_OF_FILE) {
            CurrentToken = tokens[i];
            if (tokens[i].type == TokenType::TYPE) {
                i = executeDeclaration(tokens, i, symbolTable);
            } else if (
                tokens[i].type == TokenType::IDENTIFIER &&
                i + 1 < tokens.size() &&
                isAssignmentOperatorToken(tokens[i + 1])
            ) {
                i = executeAssignment(tokens, i, symbolTable);
            } else if (
                tokens[i].type == TokenType::IDENTIFIER &&
                i + 3 < tokens.size() &&
                tokens[i + 1].type == TokenType::DOT &&
                isAssignmentOperatorToken(tokens[i + 3])
            ) {
                i = executeMemberAssignment(tokens, i, symbolTable);
            } else if (isMethodCallStart(tokens, i)) {
                i = executeMethodCallStatement(tokens, i, symbolTable);
            } else if (
                tokens[i].type == TokenType::IDENTIFIER &&
                i + 1 < tokens.size() &&
                tokens[i + 1].type == TokenType::LPAREN
            ) {
                i = executeFunctionCallStatement(tokens, i, symbolTable);
            } else if (tokens[i].type == TokenType::PRINT) {
                i = executePrint(tokens, i, symbolTable);
            } else if (tokens[i].type == TokenType::IF) {
                i = executeIf(tokens, i, symbolTable);
            } else if (tokens[i].type == TokenType::LOOP && tokens[i].value == "for") {
                i = executeFor(tokens, i, symbolTable);
            } else if (tokens[i].type == TokenType::LOOP && tokens[i].value == "while") {
                i = executeWhile(tokens, i, symbolTable);
            } else {
                i++;
            }
        }
    } else {
        // Look for else block
        size_t nextIdx = bodyEnd + 1;
        if (nextIdx < tokens.size() && tokens[nextIdx].type == TokenType::ELSE) {
            if (nextIdx + 1 >= tokens.size() || tokens[nextIdx + 1].type != TokenType::LBRACE) {
                throw std::runtime_error("Expected '{' after 'else'");
            }
            
            size_t elseStart = nextIdx + 2;
            size_t elseEnd = findMatchingBrace(tokens, nextIdx + 1);
            
            // Execute the else block
            size_t i = elseStart;
            while (i < elseEnd && tokens[i].type != TokenType::END_OF_FILE) {
                CurrentToken = tokens[i];
                if (tokens[i].type == TokenType::TYPE) {
                    i = executeDeclaration(tokens, i, symbolTable);
                } else if (
                    tokens[i].type == TokenType::IDENTIFIER &&
                    i + 1 < tokens.size() &&
                    isAssignmentOperatorToken(tokens[i + 1])
                ) {
                    i = executeAssignment(tokens, i, symbolTable);
                } else if (
                    tokens[i].type == TokenType::IDENTIFIER &&
                    i + 3 < tokens.size() &&
                    tokens[i + 1].type == TokenType::DOT &&
                    isAssignmentOperatorToken(tokens[i + 3])
                ) {
                    i = executeMemberAssignment(tokens, i, symbolTable);
                } else if (isMethodCallStart(tokens, i)) {
                    i = executeMethodCallStatement(tokens, i, symbolTable);
                } else if (
                    tokens[i].type == TokenType::IDENTIFIER &&
                    i + 1 < tokens.size() &&
                    tokens[i + 1].type == TokenType::LPAREN
                ) {
                    i = executeFunctionCallStatement(tokens, i, symbolTable);
                } else if (tokens[i].type == TokenType::PRINT) {
                    i = executePrint(tokens, i, symbolTable);
                } else if (tokens[i].type == TokenType::IF) {
                    i = executeIf(tokens, i, symbolTable);
                } else if (tokens[i].type == TokenType::LOOP && tokens[i].value == "for") {
                    i = executeFor(tokens, i, symbolTable);
                } else if (tokens[i].type == TokenType::LOOP && tokens[i].value == "while") {
                    i = executeWhile(tokens, i, symbolTable);
                } else {
                    i++;
                }
            }
            
            return elseEnd + 1;
        }
    }
    
    return bodyEnd + 1;
}

inline size_t executeWhile(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start < tokens.size()) {
        CurrentToken = tokens[start];
    }
    if (start >= tokens.size() || tokens[start].type != TokenType::LOOP || tokens[start].value != "while") {
        throw std::runtime_error("Expected 'while'");
    }
    
    if (start + 1 >= tokens.size() || tokens[start + 1].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after 'while'");
    }
    
    // Find matching closing paren
    size_t condEnd = start + 2;
    int parenDepth = 1;
    while (condEnd < tokens.size() && parenDepth > 0) {
        if (tokens[condEnd].type == TokenType::LPAREN) parenDepth++;
        if (tokens[condEnd].type == TokenType::RPAREN) parenDepth--;
        if (parenDepth > 0) condEnd++;
    }
    
    if (parenDepth != 0) {
        throw std::runtime_error("Mismatched parentheses in while condition");
    }
    
    if (condEnd + 1 >= tokens.size() || tokens[condEnd + 1].type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{' after while condition");
    }
    
    size_t bodyStart = condEnd + 2;
    size_t bodyEnd = findMatchingBrace(tokens, condEnd + 1);
    const size_t condComparisonIndex = findComparisonIndexAtDepthZero(tokens, start + 2, condEnd - 1);
    const std::vector<PlannedStatement> bodyPlan = buildPlannedStatements(tokens, bodyStart, bodyEnd);
    
    // Execute while loop
    while (evaluateConditionWithComparisonIndex(tokens, start + 2, condEnd - 1, condComparisonIndex, symbolTable)) {
        executePlannedStatements(tokens, bodyPlan, symbolTable);
    }
    
    return bodyEnd + 1;
}

inline size_t executeFor(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable
) {
    if (start < tokens.size()) {
        CurrentToken = tokens[start];
    }
    if (start >= tokens.size() || tokens[start].type != TokenType::LOOP || tokens[start].value != "for") {
        throw std::runtime_error("Expected 'for'");
    }
    
    if (start + 1 >= tokens.size() || tokens[start + 1].type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after 'for'");
    }
    
    // Find the three parts: init; condition; increment
    size_t firstSemi = start + 2;
    while (firstSemi < tokens.size() && tokens[firstSemi].type != TokenType::SEMICOLON) {
        firstSemi++;
    }
    
    size_t secondSemi = firstSemi + 1;
    while (secondSemi < tokens.size() && tokens[secondSemi].type != TokenType::SEMICOLON) {
        secondSemi++;
    }
    
    size_t closeParen = secondSemi + 1;
    while (closeParen < tokens.size() && tokens[closeParen].type != TokenType::RPAREN) {
        closeParen++;
    }
    
    // Execute init
    if (firstSemi > start + 2) {
        if (tokens[start + 2].type == TokenType::TYPE) {
            executeDeclaration(tokens, start + 2, symbolTable);
        }
    }
    
    // Get the loop body
    if (closeParen + 1 >= tokens.size() || tokens[closeParen + 1].type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{' after for statement");
    }
    
    size_t bodyStart = closeParen + 2;
    size_t bodyEnd = findMatchingBrace(tokens, closeParen + 1);
    const size_t condComparisonIndex = findComparisonIndexAtDepthZero(tokens, firstSemi + 1, secondSemi - 1);
    const std::vector<PlannedStatement> bodyPlan = buildPlannedStatements(tokens, bodyStart, bodyEnd);
    
    // Execute for loop
    while (evaluateConditionWithComparisonIndex(tokens, firstSemi + 1, secondSemi - 1, condComparisonIndex, symbolTable)) {
        executePlannedStatements(tokens, bodyPlan, symbolTable);
        
        // Execute increment - handle assignment and ++/-- forms
        if (secondSemi + 1 < closeParen) {
            if (
                secondSemi + 3 < closeParen &&
                tokens[secondSemi + 1].type == TokenType::OPERATOR &&
                tokens[secondSemi + 1].value == "+" &&
                tokens[secondSemi + 2].type == TokenType::OPERATOR &&
                tokens[secondSemi + 2].value == "+" &&
                tokens[secondSemi + 3].type == TokenType::IDENTIFIER
            ) {
                // Prefix increment: ++i
                std::string varName = tokens[secondSemi + 3].value;
                auto it = symbolTable.find(varName);
                if (it != symbolTable.end()) {
                    std::string varType = it->second.typeName;
                    double newValue = it->second.getNumberValue() + 1.0;
                    symbolTable[varName] = RuntimeValue(varType, newValue);
                }
            } else if (
                secondSemi + 3 < closeParen &&
                tokens[secondSemi + 1].type == TokenType::IDENTIFIER &&
                tokens[secondSemi + 2].type == TokenType::OPERATOR &&
                tokens[secondSemi + 2].value == "+" &&
                tokens[secondSemi + 3].type == TokenType::OPERATOR &&
                tokens[secondSemi + 3].value == "+"
            ) {
                // Postfix increment: i++
                std::string varName = tokens[secondSemi + 1].value;
                auto it = symbolTable.find(varName);
                if (it != symbolTable.end()) {
                    std::string varType = it->second.typeName;
                    double newValue = it->second.getNumberValue() + 1.0;
                    symbolTable[varName] = RuntimeValue(varType, newValue);
                }
            } else if (
                secondSemi + 3 < closeParen &&
                tokens[secondSemi + 1].type == TokenType::OPERATOR &&
                tokens[secondSemi + 1].value == "-" &&
                tokens[secondSemi + 2].type == TokenType::OPERATOR &&
                tokens[secondSemi + 2].value == "-" &&
                tokens[secondSemi + 3].type == TokenType::IDENTIFIER
            ) {
                // Prefix decrement: --i
                std::string varName = tokens[secondSemi + 3].value;
                auto it = symbolTable.find(varName);
                if (it != symbolTable.end()) {
                    std::string varType = it->second.typeName;
                    double newValue = it->second.getNumberValue() - 1.0;
                    symbolTable[varName] = RuntimeValue(varType, newValue);
                }
            } else if (
                secondSemi + 3 < closeParen &&
                tokens[secondSemi + 1].type == TokenType::IDENTIFIER &&
                tokens[secondSemi + 2].type == TokenType::OPERATOR &&
                tokens[secondSemi + 2].value == "-" &&
                tokens[secondSemi + 3].type == TokenType::OPERATOR &&
                tokens[secondSemi + 3].value == "-"
            ) {
                // Postfix decrement: i--
                std::string varName = tokens[secondSemi + 1].value;
                auto it = symbolTable.find(varName);
                if (it != symbolTable.end()) {
                    std::string varType = it->second.typeName;
                    double newValue = it->second.getNumberValue() - 1.0;
                    symbolTable[varName] = RuntimeValue(varType, newValue);
                }
            } else if (tokens[secondSemi + 1].type == TokenType::IDENTIFIER && 
                secondSemi + 2 < tokens.size() && 
                isAssignmentOperatorToken(tokens[secondSemi + 2])) {
                // This is an assignment: var = expr
                std::string varName = tokens[secondSemi + 1].value;
                double rhsValue = 0.0;
                if (!tryEvaluateSimpleLoopIncrementRhs(tokens, secondSemi + 3, closeParen - 1, symbolTable, varName, rhsValue)) {
                    rhsValue = evaluateExpression(tokens, secondSemi + 3, closeParen - 1, symbolTable);
                }
                const std::string op = tokens[secondSemi + 2].value;
                
                // Get the variable type
                auto it = symbolTable.find(varName);
                if (it != symbolTable.end()) {
                    std::string varType = it->second.typeName;
                    double lhsValue = it->second.getNumberValue();
                    double newValue = applyScalarAssignmentOperator(lhsValue, rhsValue, op);
                    if (varType == "int") {
                        symbolTable[varName] = RuntimeValue(varType, static_cast<double>(static_cast<int>(newValue)));
                    } else if (varType == "bool") {
                        symbolTable[varName] = RuntimeValue(varType, newValue != 0.0 ? 1.0 : 0.0);
                    } else {
                        symbolTable[varName] = RuntimeValue(varType, newValue);
                    }
                }
            } else {
                // Just evaluate as expression
                try {
                    evaluateExpression(tokens, secondSemi + 1, closeParen - 1, symbolTable);
                } catch (...) {
                    // Ignore errors
                }
            }
        }
    }
    
    return bodyEnd + 1;
}

inline std::set<std::string> importedFiles;

// Forward declaration for mutual recursion
inline std::unordered_map<std::string, RuntimeValue> executeProgram(const std::vector<Token>& tokens);

inline std::string loadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline size_t executeImport(
    const std::vector<Token>& tokens,
    size_t start,
    std::unordered_map<std::string, RuntimeValue>& symbolTable,
    std::vector<Token>& allTokens
) {
    if (start < tokens.size()) {
        CurrentToken = tokens[start];
    }
    if (start >= tokens.size() || tokens[start].type != TokenType::IMPORT) {
        throw std::runtime_error("Expected 'import'");
    }

    if (start + 1 >= tokens.size() ||
        tokens[start + 1].type != TokenType::COMPARISON ||
        tokens[start + 1].value != "<") {
        throw std::runtime_error("Expected '<' after import");
    }

    size_t filenameStart = start + 2;
    size_t filenameEnd = filenameStart;

    while (filenameEnd < tokens.size()) {
        if (tokens[filenameEnd].type == TokenType::COMPARISON && tokens[filenameEnd].value == ">") {
            break;
        }
        filenameEnd++;
    }

    if (filenameStart >= filenameEnd) {
        throw std::runtime_error("Expected file name inside import brackets");
    }

    if (filenameEnd >= tokens.size() ||
        tokens[filenameEnd].type != TokenType::COMPARISON ||
        tokens[filenameEnd].value != ">") {
        throw std::runtime_error("Expected '>' after import file name");
    }

    std::string filename;
    for (size_t i = filenameStart; i < filenameEnd; i++) {
        filename += tokens[i].value;
    }

    if (filename == "window") {
        importedFiles.insert(filename);
        return filenameEnd + 1;
    }
    
    // Check if already imported
    if (importedFiles.find(filename) != importedFiles.end()) {
        // Skip already imported file
        return filenameEnd + 1;
    }
    
    // Mark as imported
    importedFiles.insert(filename);
    
    // Load and tokenize the file
    std::string source = loadFile(filename);
    std::vector<Token> importedTokens = tokenize(source);
    
    // Execute the imported file's tokens to populate symbol table and functions
    std::unordered_map<std::string, RuntimeValue> importedSymbolTable = executeProgram(importedTokens);
    
    // Merge imported variables and functions into the caller's symbol table
    for (const auto& pair : importedSymbolTable) {
        if (symbolTable.find(pair.first) == symbolTable.end()) {
            symbolTable[pair.first] = pair.second;
        }
    }
    
    return filenameEnd + 1;
}

inline std::unordered_map<std::string, RuntimeValue> executeProgram(const std::vector<Token>& tokens) {
    std::unordered_map<std::string, RuntimeValue> symbolTable;

    // First pass: collect function definitions
    size_t i = 0;
    while (i < tokens.size() && tokens[i].type != TokenType::END_OF_FILE) {
        CurrentToken = tokens[i];
        // Look for TYPE followed by IDENTIFIER followed by LPAREN (function definition)
        if (tokens[i].type == TokenType::TYPE && 
            i + 2 < tokens.size() && 
            tokens[i + 1].type == TokenType::IDENTIFIER &&
            tokens[i + 2].type == TokenType::LPAREN) {
            i = parseFunctionDef(tokens, i);
        } else {
            i++;
        }
    }

    // Second pass: execute statements
    i = 0;
    while (i < tokens.size() && tokens[i].type != TokenType::END_OF_FILE) {
        CurrentToken = tokens[i];
        // Skip function definitions
        if (tokens[i].type == TokenType::TYPE && 
            i + 2 < tokens.size() && 
            tokens[i + 1].type == TokenType::IDENTIFIER &&
            tokens[i + 2].type == TokenType::LPAREN) {
            // Find the closing brace of the function
            size_t braceIdx = i + 3;
            while (braceIdx < tokens.size() && tokens[braceIdx].type != TokenType::LBRACE) {
                braceIdx++;
            }
            i = findMatchingBrace(tokens, braceIdx) + 1;
            continue;
        }
        
        if (tokens[i].type == TokenType::TYPE) {
            // Check if this is a function call in a declaration
            // e.g., float z = rand();
            size_t j = i + 2;
            while (j < tokens.size() && tokens[j].type != TokenType::EQUAL && tokens[j].type != TokenType::SEMICOLON) {
                j++;
            }
            
            if (j < tokens.size() && tokens[j].type == TokenType::EQUAL) {
                j++;
                // Check if the right side is a function call
                if (
                    j < tokens.size() &&
                    (tokens[j].type == TokenType::IDENTIFIER || tokens[j].type == TokenType::TYPE)
                ) {
                    size_t k = j + 1;
                    if (k < tokens.size() && tokens[k].type == TokenType::LPAREN) {
                        // This is a function call in a declaration
                        std::string varName = tokens[i + 1].value;
                        std::string varType = tokens[i].value;
                        RuntimeValue result = callFunction(tokens, j, symbolTable);
                        symbolTable[varName] = result;
                        
                        // Skip to semicolon
                        while (i < tokens.size() && tokens[i].type != TokenType::SEMICOLON) {
                            i++;
                        }
                        i++;
                        continue;
                    }
                }
            }
            
            i = executeDeclaration(tokens, i, symbolTable);
            continue;
        }
        
        if (tokens[i].type == TokenType::PRINT) {
            i = executePrint(tokens, i, symbolTable);
            continue;
        }

        if (
            tokens[i].type == TokenType::IDENTIFIER &&
            i + 1 < tokens.size() &&
            isAssignmentOperatorToken(tokens[i + 1])
        ) {
            i = executeAssignment(tokens, i, symbolTable);
            continue;
        }

        if (
            tokens[i].type == TokenType::IDENTIFIER &&
            i + 3 < tokens.size() &&
            tokens[i + 1].type == TokenType::DOT &&
            isAssignmentOperatorToken(tokens[i + 3])
        ) {
            i = executeMemberAssignment(tokens, i, symbolTable);
            continue;
        }

        if (isMethodCallStart(tokens, i)) {
            i = executeMethodCallStatement(tokens, i, symbolTable);
            continue;
        }

        if (
            tokens[i].type == TokenType::IDENTIFIER &&
            i + 1 < tokens.size() &&
            tokens[i + 1].type == TokenType::LPAREN
        ) {
            i = executeFunctionCallStatement(tokens, i, symbolTable);
            continue;
        }
        
        if (tokens[i].type == TokenType::IF) {
            i = executeIf(tokens, i, symbolTable);
            continue;
        }
        
        if (tokens[i].type == TokenType::LOOP) {
            if (tokens[i].value == "while") {
                i = executeWhile(tokens, i, symbolTable);
            } else if (tokens[i].value == "for") {
                i = executeFor(tokens, i, symbolTable);
            } else {
                i++;
            }
            continue;
        }
        
        if (tokens[i].type == TokenType::IMPORT) {
            i = executeImport(tokens, i, symbolTable, const_cast<std::vector<Token>&>(tokens));
            continue;
        }

        i++;
    }

    return symbolTable;
}
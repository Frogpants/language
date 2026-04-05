#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <cmath>
#include <memory>

namespace GETypes {

// Float type
struct FloatType {
    float value = 0.0f;
    
    FloatType() = default;
    FloatType(float v) : value(v) {}
};

// Int type
struct IntType {
    int value = 0;
    
    IntType() = default;
    IntType(int v) : value(v) {}
};

// String type
struct StringType {
    std::string value = "";
    
    StringType() = default;
    StringType(const std::string& v) : value(v) {}
};

// Bool type
struct BoolType {
    bool value = false;
    
    BoolType() = default;
    BoolType(bool v) : value(v) {}
};

// Vector2 type
struct Vec2Type {
    float x = 0.0f;
    float y = 0.0f;
    
    Vec2Type() = default;
    explicit Vec2Type(float n) : x(n), y(n) {}
    Vec2Type(float x_val, float y_val) : x(x_val), y(y_val) {}

    Vec2Type operator+(const Vec2Type& other) const { return Vec2Type(x + other.x, y + other.y); }
    Vec2Type operator-(const Vec2Type& other) const { return Vec2Type(x - other.x, y - other.y); }
    Vec2Type operator*(const Vec2Type& other) const { return Vec2Type(x * other.x, y * other.y); }
    Vec2Type operator/(const Vec2Type& other) const { return Vec2Type(x / other.x, y / other.y); }

    Vec2Type operator+(float other) const { return Vec2Type(x + other, y + other); }
    Vec2Type operator-(float other) const { return Vec2Type(x - other, y - other); }
    Vec2Type operator*(float other) const { return Vec2Type(x * other, y * other); }
    Vec2Type operator/(float other) const { return Vec2Type(x / other, y / other); }

    Vec2Type operator+=(const Vec2Type& other) const { return Vec2Type(x + other.x, y + other.y); }
    Vec2Type operator-=(const Vec2Type& other) const { return Vec2Type(x - other.x, y - other.y); }
    Vec2Type operator*=(const Vec2Type& other) const { return Vec2Type(x * other.x, y * other.y); }
    Vec2Type operator/=(const Vec2Type& other) const { return Vec2Type(x / other.x, y / other.y); }

    Vec2Type operator+=(float other) { return Vec2Type(x + other, y + other); }
    Vec2Type operator-=(float other) { return Vec2Type(x - other, y - other); }
    Vec2Type operator*=(float other) { return Vec2Type(x * other, y * other); }
    Vec2Type operator/=(float other) { return Vec2Type(x / other, y / other); }
};

// Vector3 type
struct Vec3Type {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    Vec2Type xy;
    Vec2Type yz;
    Vec2Type xz;
    
    Vec3Type() : x(0.0f), y(0.0f), z(0.0f), xy(x, y), yz(y, z), xz(x, z) {}
    explicit Vec3Type(float n) : x(n), y(n), z(n), xy(x, y), yz(y, z), xz(x, z) {}
    Vec3Type(const Vec2Type& v, float n) : x(v.x), y(v.y), z(n), xy(x, y), yz(y, z), xz(x, z) {}
    Vec3Type(float n, const Vec2Type& v) : x(n), y(v.x), z(v.y), xy(x, y), yz(y, z), xz(x, z) {}
    Vec3Type(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val), xy(x, y), yz(y, z), xz(x, z) {}

    Vec3Type operator+(const Vec3Type& other) const { return Vec3Type(x + other.x, y + other.y, z + other.z); }
    Vec3Type operator-(const Vec3Type& other) const { return Vec3Type(x - other.x, y - other.y, z - other.z); }
    Vec3Type operator*(const Vec3Type& other) const { return Vec3Type(x * other.x, y * other.y, z * other.z); }
    Vec3Type operator/(const Vec3Type& other) const { return Vec3Type(x / other.x, y / other.y, z / other.z); }

    Vec3Type operator+(float other) const { return Vec3Type(x + other, y + other, z + other); }
    Vec3Type operator-(float other) const { return Vec3Type(x - other, y - other, z - other); }
    Vec3Type operator*(float other) const { return Vec3Type(x * other, y * other, z * other); }
    Vec3Type operator/(float other) const { return Vec3Type(x / other, y / other, z / other); }

    Vec3Type operator+=(const Vec3Type& other) const { return Vec3Type(x + other.x, y + other.y, z + other.z); }
    Vec3Type operator-=(const Vec3Type& other) const { return Vec3Type(x - other.x, y - other.y, z - other.z); }
    Vec3Type operator*=(const Vec3Type& other) const { return Vec3Type(x * other.x, y * other.y, z * other.z); }
    Vec3Type operator/=(const Vec3Type& other) const { return Vec3Type(x / other.x, y / other.y, z / other.z); }

    Vec3Type operator+=(float other) { return Vec3Type(x + other, y + other, z + other); }
    Vec3Type operator-=(float other) { return Vec3Type(x - other, y - other, z - other); }
    Vec3Type operator*=(float other) { return Vec3Type(x * other, y * other, z * other); }
    Vec3Type operator/=(float other) { return Vec3Type(x / other, y / other, z / other); }
};

// Vector4 type
struct Vec4Type {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
    Vec3Type xyz;
    Vec3Type zyx;
    Vec3Type yzw;
    Vec3Type wzy;
    
    Vec4Type() : x(0.0f), y(0.0f), z(0.0f), w(0.0f), xyz(x, y, z), zyx(z, y, x), yzw(y, z, w), wzy(w, z, y) {}
    explicit Vec4Type(float n) : x(n), y(n), z(n), w(n), xyz(x, y, z), zyx(z, y, x), yzw(y, z, w), wzy(w, z, y) {}
    Vec4Type(const Vec3Type& v, float n) : x(v.x), y(v.y), z(v.z), w(n), xyz(x, y, z), zyx(z, y, x), yzw(y, z, w), wzy(w, z, y) {}
    Vec4Type(float n, const Vec3Type& v) : x(n), y(v.x), z(v.y), w(v.z), xyz(x, y, z), zyx(z, y, x), yzw(y, z, w), wzy(w, z, y) {}
    Vec4Type(const Vec2Type& v1, const Vec2Type& v2) : x(v1.x), y(v1.y), z(v2.x), w(v2.y), xyz(x, y, z), zyx(z, y, x), yzw(y, z, w), wzy(w, z, y) {}
    Vec4Type(float x_val, float y_val, float z_val, float w_val) 
        : x(x_val), y(y_val), z(z_val), w(w_val), xyz(x, y, z), zyx(z, y, x), yzw(y, z, w), wzy(w, z, y) {}

    Vec4Type operator+(const Vec4Type& other) const { return Vec4Type(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vec4Type operator-(const Vec4Type& other) const { return Vec4Type(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vec4Type operator*(const Vec4Type& other) const { return Vec4Type(x * other.x, y * other.y, z * other.z, w * other.w); }
    Vec4Type operator/(const Vec4Type& other) const { return Vec4Type(x / other.x, y / other.y, z / other.z, w / other.w); }

    Vec4Type operator+(float other) const { return Vec4Type(x + other, y + other, z + other, w + other); }
    Vec4Type operator-(float other) const { return Vec4Type(x - other, y - other, z - other, w - other); }
    Vec4Type operator*(float other) const { return Vec4Type(x * other, y * other, z * other, w * other); }
    Vec4Type operator/(float other) const { return Vec4Type(x / other, y / other, z / other, w / other); }

    Vec4Type operator+=(const Vec4Type& other) const { return Vec4Type(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vec4Type operator-=(const Vec4Type& other) const { return Vec4Type(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vec4Type operator*=(const Vec4Type& other) const { return Vec4Type(x * other.x, y * other.y, z * other.z, w * other.w); }
    Vec4Type operator/=(const Vec4Type& other) const { return Vec4Type(x / other.x, y / other.y, z / other.z, w / other.w); }

    Vec4Type operator+=(float other) { return Vec4Type(x + other, y + other, z + other, w + other); }
    Vec4Type operator-=(float other) { return Vec4Type(x - other, y - other, z - other, w - other); }
    Vec4Type operator*=(float other) { return Vec4Type(x * other, y * other, z * other, w * other); }
    Vec4Type operator/=(float other) { return Vec4Type(x / other, y / other, z / other, w / other); }
};

inline float length(const Vec2Type& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline float length(const Vec3Type& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float length(const Vec4Type& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

inline float dot(const Vec2Type& a, const Vec2Type& b) {
    return a.x * b.x + a.y * b.y;
}

inline float dot(const Vec3Type& a, const Vec3Type& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float dot(const Vec4Type& a, const Vec4Type& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline Vec3Type cross(const Vec3Type& a, const Vec3Type& b) {
    return Vec3Type(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline Vec2Type normalize(const Vec2Type& v) {
    const float l = length(v);
    return v / l;
}

inline Vec3Type normalize(const Vec3Type& v) {
    const float l = length(v);
    return v / l;
}

inline Vec4Type normalize(const Vec4Type& v) {
    const float l = length(v);
    return v / l;
}

struct ListType;
using ListPtr = std::shared_ptr<ListType>;

// List type - can store any type of values, including nested lists
struct ListType {
    using Value = std::variant<IntType, FloatType, StringType, BoolType, Vec2Type, Vec3Type, Vec4Type, ListPtr>;
    std::vector<Value> elements;
    
    ListType() = default;
    ListType(const std::vector<Value>& els) : elements(els) {}
    
    template <typename T>
    void add(const T& value) {
        elements.push_back(value);
    }

    void remove(const size_t index) {
        if (index < elements.size()) {
            elements.erase(elements.begin() + index);
        }
    }

    void clear() {
        elements.clear();
    }

    static std::string numberToString(double value) {
        std::ostringstream out;
        out << std::setprecision(15) << value;
        return out.str();
    }

    static std::string valueToString(const Value& value) {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, IntType>) {
                return std::to_string(v.value);
            } else if constexpr (std::is_same_v<T, FloatType>) {
                return numberToString(v.value);
            } else if constexpr (std::is_same_v<T, StringType>) {
                return v.value;
            } else if constexpr (std::is_same_v<T, BoolType>) {
                return v.value ? "true" : "false";
            } else if constexpr (std::is_same_v<T, Vec2Type>) {
                return "vec2(" + numberToString(v.x) + ", " + numberToString(v.y) + ")";
            } else if constexpr (std::is_same_v<T, Vec3Type>) {
                return "vec3(" + numberToString(v.x) + ", " + numberToString(v.y) + ", " + numberToString(v.z) + ")";
            } else if constexpr (std::is_same_v<T, Vec4Type>) {
                return "vec4(" + numberToString(v.x) + ", " + numberToString(v.y) + ", " + numberToString(v.z) + ", " + numberToString(v.w) + ")";
            } else if constexpr (std::is_same_v<T, ListPtr>) {
                if (!v) {
                    return "[]";
                }
                std::string out = "[";
                for (size_t i = 0; i < v->elements.size(); ++i) {
                    if (i > 0) {
                        out += ", ";
                    }
                    out += valueToString(v->elements[i]);
                }
                out += "]";
                return out;
            }
            return std::string();
        }, value);
    }

    std::string getValue(size_t index) const {
        if (index < elements.size()) {
            return valueToString(elements[index]);
        }
        return "";
    }

    size_t getIndex(const Value& value) const {
        const std::string needle = valueToString(value);
        for (size_t i = 0; i < elements.size(); ++i) {
            if (valueToString(elements[i]) == needle) {
                return i;
            }
        }
        return static_cast<size_t>(-1);
    }
    
    size_t size() const {
        return elements.size();
    }
};

// Type enum for type identification
enum class VariableType {
    INT,
    FLOAT,
    STRING,
    BOOL,
    VEC2,
    VEC3,
    VEC4,
    LIST,
    UNKNOWN
};

// Utility function to get type name as string
inline std::string getTypeName(VariableType type) {
    switch (type) {
        case VariableType::INT:
            return "int";
        case VariableType::FLOAT:
            return "float";
        case VariableType::STRING:
            return "string";
        case VariableType::BOOL:
            return "bool";
        case VariableType::VEC2:
            return "vec2";
        case VariableType::VEC3:
            return "vec3";
        case VariableType::VEC4:
            return "vec4";
        case VariableType::LIST:
            return "list";
        default:
            return "unknown";
    }
}

// Utility function to convert type name string to VariableType
inline VariableType stringToType(const std::string& typeName) {
    if (typeName == "int") return VariableType::INT;
    if (typeName == "float") return VariableType::FLOAT;
    if (typeName == "string") return VariableType::STRING;
    if (typeName == "bool") return VariableType::BOOL;
    if (typeName == "vec2") return VariableType::VEC2;
    if (typeName == "vec3") return VariableType::VEC3;
    if (typeName == "vec4") return VariableType::VEC4;
    if (typeName == "list") return VariableType::LIST;
    return VariableType::UNKNOWN;
}

}  // namespace GETypes

#endif  // TYPES_HPP

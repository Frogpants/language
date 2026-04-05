#pragma once

#include <algorithm>
#include <cmath>
#include <cctype>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(__linux__)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#endif

struct NativeWindowBackend {
#if defined(__linux__)
    struct TriangleCommand {
        float x1 = 0.0f;
        float y1 = 0.0f;
        float z1 = 0.0f;
        float x2 = 0.0f;
        float y2 = 0.0f;
        float z2 = 0.0f;
        float x3 = 0.0f;
        float y3 = 0.0f;
        float z3 = 0.0f;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
    };

    Display* display = nullptr;
    int screen = 0;
    Window window = 0;
    GC gc = 0;
    Colormap colormap = 0;
    GLXContext glContext = nullptr;
    Atom wmDeleteMessage = 0;
    Cursor invisibleCursor = 0;
    Visual* visual = nullptr;
    XVisualInfo* visualInfo = nullptr;
    unsigned long redMask = 0;
    unsigned long greenMask = 0;
    unsigned long blueMask = 0;
    int redShift = 0;
    int greenShift = 0;
    int blueShift = 0;
    std::vector<TriangleCommand> pendingTriangles;
    bool shaderApiReady = false;
    GLuint activeProgram = 0;
    std::unordered_map<std::string, GLenum> uniformTypes;
    std::unordered_map<std::string, GLint> uniformLocations;

    PFNGLCREATESHADERPROC glCreateShaderPtr = nullptr;
    PFNGLSHADERSOURCEPROC glShaderSourcePtr = nullptr;
    PFNGLCOMPILESHADERPROC glCompileShaderPtr = nullptr;
    PFNGLGETSHADERIVPROC glGetShaderivPtr = nullptr;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLogPtr = nullptr;
    PFNGLDELETESHADERPROC glDeleteShaderPtr = nullptr;
    PFNGLCREATEPROGRAMPROC glCreateProgramPtr = nullptr;
    PFNGLATTACHSHADERPROC glAttachShaderPtr = nullptr;
    PFNGLLINKPROGRAMPROC glLinkProgramPtr = nullptr;
    PFNGLGETPROGRAMIVPROC glGetProgramivPtr = nullptr;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLogPtr = nullptr;
    PFNGLDELETEPROGRAMPROC glDeleteProgramPtr = nullptr;
    PFNGLUSEPROGRAMPROC glUseProgramPtr = nullptr;
    PFNGLDETACHSHADERPROC glDetachShaderPtr = nullptr;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocationPtr = nullptr;
    PFNGLUNIFORM1FPROC glUniform1fPtr = nullptr;
    PFNGLUNIFORM1IPROC glUniform1iPtr = nullptr;
    PFNGLUNIFORM2FPROC glUniform2fPtr = nullptr;
    PFNGLUNIFORM3FPROC glUniform3fPtr = nullptr;
    PFNGLUNIFORM4FPROC glUniform4fPtr = nullptr;
#endif

    bool created = false;

    static std::string readTextFile(const std::string& path, std::string& error) {
        std::ifstream in(path);
        if (!in.is_open()) {
            error = "Failed to open file: " + path;
            return std::string();
        }
        std::ostringstream contents;
        contents << in.rdbuf();
        return contents.str();
    }

    static int computeMaskShift(unsigned long mask) {
        int shift = 0;
        while (((mask >> shift) & 1UL) == 0UL && shift < 63) {
            shift++;
        }
        return shift;
    }

    static unsigned long toMaskedChannel(float channel, unsigned long mask, int shift) {
        if (mask == 0) {
            return 0;
        }
        const unsigned long maxChannel = mask >> shift;
        const float clamped = std::clamp(channel, 0.0f, 1.0f);
        const unsigned long value = static_cast<unsigned long>(std::lround(clamped * static_cast<float>(maxChannel)));
        return (value << shift) & mask;
    }

    unsigned long colorToPixel(float r, float g, float b) const {
#if defined(__linux__)
        return toMaskedChannel(r, redMask, redShift) |
               toMaskedChannel(g, greenMask, greenShift) |
               toMaskedChannel(b, blueMask, blueShift);
#else
        (void)r;
        (void)g;
        (void)b;
        return 0;
#endif
    }

    bool create(int width, int height, const std::string& title, std::string& error) {
        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);

#if defined(__linux__)
        destroy();

        display = XOpenDisplay(nullptr);
        if (!display) {
            error = "Failed to open X11 display. Ensure a graphical session is available.";
            return false;
        }

        screen = DefaultScreen(display);
        int glxAttrs[] = {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_DEPTH_SIZE, 24,
            None
        };
        visualInfo = glXChooseVisual(display, screen, glxAttrs);
        if (!visualInfo) {
            error = "Failed to choose an OpenGL visual (GLX).";
            destroy();
            return false;
        }

        visual = visualInfo->visual;
        redMask = visual ? visual->red_mask : 0;
        greenMask = visual ? visual->green_mask : 0;
        blueMask = visual ? visual->blue_mask : 0;
        redShift = computeMaskShift(redMask);
        greenShift = computeMaskShift(greenMask);
        blueShift = computeMaskShift(blueMask);

        colormap = XCreateColormap(display, RootWindow(display, screen), visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap = colormap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                         ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;

        window = XCreateWindow(
            display,
            RootWindow(display, screen),
            100,
            100,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight),
            0,
            visualInfo->depth,
            InputOutput,
            visual,
            CWColormap | CWEventMask,
            &swa
        );

        if (!window) {
            error = "Failed to create X11 window for OpenGL rendering.";
            destroy();
            return false;
        }

        XStoreName(display, window, title.c_str());
        wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, window, &wmDeleteMessage, 1);

        gc = XCreateGC(display, window, 0, nullptr);

        glContext = glXCreateContext(display, visualInfo, nullptr, GL_TRUE);
        if (!glContext) {
            error = "Failed to create GLX OpenGL context.";
            destroy();
            return false;
        }

        if (!glXMakeCurrent(display, window, glContext)) {
            error = "Failed to bind GLX context to window.";
            destroy();
            return false;
        }

        initShaderApi();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glViewport(0, 0, safeWidth, safeHeight);

        XMapRaised(display, window);
        XFlush(display);
        created = true;
        return true;
#else
        (void)safeWidth;
        (void)safeHeight;
        (void)title;
        error = "Native window backend is not implemented for this platform yet.";
        created = false;
        return false;
#endif
    }

    void destroy() {
#if defined(__linux__)
        if (display) {
            if (glContext) {
                if (window) {
                    glXMakeCurrent(display, window, glContext);
                }
                clearShader();
                glXMakeCurrent(display, None, nullptr);
                glXDestroyContext(display, glContext);
                glContext = nullptr;
            }
            if (gc) {
                XFreeGC(display, gc);
                gc = 0;
            }
            if (colormap) {
                XFreeColormap(display, colormap);
                colormap = 0;
            }
            if (invisibleCursor) {
                XFreeCursor(display, invisibleCursor);
                invisibleCursor = 0;
            }
            if (window) {
                XDestroyWindow(display, window);
                window = 0;
            }
            XCloseDisplay(display);
            display = nullptr;
        }
        wmDeleteMessage = 0;
        visual = nullptr;
        if (visualInfo) {
            XFree(visualInfo);
            visualInfo = nullptr;
        }
        redMask = greenMask = blueMask = 0;
        shaderApiReady = false;
        activeProgram = 0;
#endif
        created = false;
    }

    void initShaderApi() {
#if defined(__linux__)
        auto load = [](const char* name) -> void* {
            return reinterpret_cast<void*>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(name)));
        };

        glCreateShaderPtr = reinterpret_cast<PFNGLCREATESHADERPROC>(load("glCreateShader"));
        glShaderSourcePtr = reinterpret_cast<PFNGLSHADERSOURCEPROC>(load("glShaderSource"));
        glCompileShaderPtr = reinterpret_cast<PFNGLCOMPILESHADERPROC>(load("glCompileShader"));
        glGetShaderivPtr = reinterpret_cast<PFNGLGETSHADERIVPROC>(load("glGetShaderiv"));
        glGetShaderInfoLogPtr = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(load("glGetShaderInfoLog"));
        glDeleteShaderPtr = reinterpret_cast<PFNGLDELETESHADERPROC>(load("glDeleteShader"));
        glCreateProgramPtr = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(load("glCreateProgram"));
        glAttachShaderPtr = reinterpret_cast<PFNGLATTACHSHADERPROC>(load("glAttachShader"));
        glLinkProgramPtr = reinterpret_cast<PFNGLLINKPROGRAMPROC>(load("glLinkProgram"));
        glGetProgramivPtr = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(load("glGetProgramiv"));
        glGetProgramInfoLogPtr = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(load("glGetProgramInfoLog"));
        glDeleteProgramPtr = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(load("glDeleteProgram"));
        glUseProgramPtr = reinterpret_cast<PFNGLUSEPROGRAMPROC>(load("glUseProgram"));
        glDetachShaderPtr = reinterpret_cast<PFNGLDETACHSHADERPROC>(load("glDetachShader"));
        glGetUniformLocationPtr = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(load("glGetUniformLocation"));
        glUniform1fPtr = reinterpret_cast<PFNGLUNIFORM1FPROC>(load("glUniform1f"));
        glUniform1iPtr = reinterpret_cast<PFNGLUNIFORM1IPROC>(load("glUniform1i"));
        glUniform2fPtr = reinterpret_cast<PFNGLUNIFORM2FPROC>(load("glUniform2f"));
        glUniform3fPtr = reinterpret_cast<PFNGLUNIFORM3FPROC>(load("glUniform3f"));
        glUniform4fPtr = reinterpret_cast<PFNGLUNIFORM4FPROC>(load("glUniform4f"));

        if (!glCreateShaderPtr) glCreateShaderPtr = &glCreateShader;
        if (!glShaderSourcePtr) glShaderSourcePtr = &glShaderSource;
        if (!glCompileShaderPtr) glCompileShaderPtr = &glCompileShader;
        if (!glGetShaderivPtr) glGetShaderivPtr = &glGetShaderiv;
        if (!glGetShaderInfoLogPtr) glGetShaderInfoLogPtr = &glGetShaderInfoLog;
        if (!glDeleteShaderPtr) glDeleteShaderPtr = &glDeleteShader;
        if (!glCreateProgramPtr) glCreateProgramPtr = &glCreateProgram;
        if (!glAttachShaderPtr) glAttachShaderPtr = &glAttachShader;
        if (!glLinkProgramPtr) glLinkProgramPtr = &glLinkProgram;
        if (!glGetProgramivPtr) glGetProgramivPtr = &glGetProgramiv;
        if (!glGetProgramInfoLogPtr) glGetProgramInfoLogPtr = &glGetProgramInfoLog;
        if (!glDeleteProgramPtr) glDeleteProgramPtr = &glDeleteProgram;
        if (!glUseProgramPtr) glUseProgramPtr = &glUseProgram;
        if (!glDetachShaderPtr) glDetachShaderPtr = &glDetachShader;
        if (!glGetUniformLocationPtr) glGetUniformLocationPtr = &glGetUniformLocation;
        if (!glUniform1fPtr) glUniform1fPtr = &glUniform1f;
        if (!glUniform1iPtr) glUniform1iPtr = &glUniform1i;
        if (!glUniform2fPtr) glUniform2fPtr = &glUniform2f;
        if (!glUniform3fPtr) glUniform3fPtr = &glUniform3f;
        if (!glUniform4fPtr) glUniform4fPtr = &glUniform4f;

        shaderApiReady =
            glCreateShaderPtr &&
            glShaderSourcePtr &&
            glCompileShaderPtr &&
            glGetShaderivPtr &&
            glGetShaderInfoLogPtr &&
            glDeleteShaderPtr &&
            glCreateProgramPtr &&
            glAttachShaderPtr &&
            glLinkProgramPtr &&
            glGetProgramivPtr &&
            glGetProgramInfoLogPtr &&
            glDeleteProgramPtr &&
            glUseProgramPtr &&
            glGetUniformLocationPtr &&
            glUniform1fPtr &&
            glUniform1iPtr &&
            glUniform2fPtr &&
            glUniform3fPtr &&
            glUniform4fPtr;
#endif
    }

    static std::string trimCopy(const std::string& input) {
        size_t start = 0;
        while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
            start++;
        }
        size_t end = input.size();
        while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
            end--;
        }
        return input.substr(start, end - start);
    }

    static GLenum uniformTypeFromKeyword(const std::string& keyword) {
        if (keyword == "float") return GL_FLOAT;
        if (keyword == "int") return GL_INT;
        if (keyword == "bool") return GL_BOOL;
        if (keyword == "vec2") return GL_FLOAT_VEC2;
        if (keyword == "vec3") return GL_FLOAT_VEC3;
        if (keyword == "vec4") return GL_FLOAT_VEC4;
        return 0;
    }

    void registerUniformTypesFromSource(const std::string& source) {
        std::istringstream in(source);
        std::string line;
        while (std::getline(in, line)) {
            const size_t commentPos = line.find("//");
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }
            line = trimCopy(line);
            if (line.rfind("uniform", 0) != 0) {
                continue;
            }

            std::istringstream ls(line);
            std::string uniformWord;
            std::string typeWord;
            std::string nameWord;
            ls >> uniformWord >> typeWord >> nameWord;
            if (uniformWord != "uniform" || typeWord.empty() || nameWord.empty()) {
                continue;
            }

            size_t semicolon = nameWord.find(';');
            if (semicolon != std::string::npos) {
                nameWord = nameWord.substr(0, semicolon);
            }
            size_t bracket = nameWord.find('[');
            if (bracket != std::string::npos) {
                nameWord = nameWord.substr(0, bracket);
            }

            const GLenum type = uniformTypeFromKeyword(typeWord);
            if (type != 0 && !nameWord.empty()) {
                uniformTypes[nameWord] = type;
            }
        }
    }

    bool resolveUniform(const std::string& name, GLint& location, GLenum& type, std::string& error) {
#if defined(__linux__)
        if (activeProgram == 0) {
            error = "No active shader program. Call window.setShader(...) first.";
            return false;
        }
        auto typeIt = uniformTypes.find(name);
        if (typeIt == uniformTypes.end()) {
            error = "Uniform not found in active shader: " + name;
            return false;
        }

        auto locIt = uniformLocations.find(name);
        if (locIt != uniformLocations.end()) {
            location = locIt->second;
        } else {
            location = glGetUniformLocationPtr(activeProgram, name.c_str());
            uniformLocations[name] = location;
        }
        if (location < 0) {
            error = "Uniform '" + name + "' is not active in current shader (optimized out or missing).";
            return false;
        }
        type = typeIt->second;
        return true;
#else
        (void)name;
        (void)location;
        (void)type;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    static bool uniformTypeEquals(GLenum type, GLenum expected) {
        return type == expected;
    }

    bool compileShader(GLenum shaderType, const std::string& source, GLuint& shaderOut, std::string& error) {
#if defined(__linux__)
        shaderOut = 0;
        if (!shaderApiReady) {
            error = "GLSL shader API is not available on this OpenGL context.";
            return false;
        }
        shaderOut = glCreateShaderPtr(shaderType);
        if (!shaderOut) {
            error = "Failed to create shader object.";
            return false;
        }

        const char* src = source.c_str();
        glShaderSourcePtr(shaderOut, 1, &src, nullptr);
        glCompileShaderPtr(shaderOut);

        GLint compileOk = 0;
        glGetShaderivPtr(shaderOut, GL_COMPILE_STATUS, &compileOk);
        if (!compileOk) {
            GLint logLen = 0;
            glGetShaderivPtr(shaderOut, GL_INFO_LOG_LENGTH, &logLen);
            std::string info;
            if (logLen > 0) {
                info.resize(static_cast<size_t>(logLen));
                GLsizei written = 0;
                glGetShaderInfoLogPtr(shaderOut, logLen, &written, info.data());
                info.resize(static_cast<size_t>(std::max(0, static_cast<int>(written))));
            }
            glDeleteShaderPtr(shaderOut);
            shaderOut = 0;
            error = "Shader compile error: " + info;
            return false;
        }
        return true;
#else
        (void)shaderType;
        (void)source;
        (void)shaderOut;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setShader(const std::string& vertexSource, const std::string& fragmentSource, std::string& error) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            error = "Window must be created before loading a shader.";
            return false;
        }
        if (!glXMakeCurrent(display, window, glContext)) {
            error = "Failed to bind GL context before shader setup.";
            return false;
        }
        if (!shaderApiReady) {
            error = "GLSL shader API is not available on this OpenGL context.";
            return false;
        }

        flushTriangles();

        GLuint vert = 0;
        GLuint frag = 0;
        if (!compileShader(GL_VERTEX_SHADER, vertexSource, vert, error)) {
            return false;
        }
        if (!compileShader(GL_FRAGMENT_SHADER, fragmentSource, frag, error)) {
            glDeleteShaderPtr(vert);
            return false;
        }

        GLuint program = glCreateProgramPtr();
        if (!program) {
            glDeleteShaderPtr(vert);
            glDeleteShaderPtr(frag);
            error = "Failed to create shader program.";
            return false;
        }

        glAttachShaderPtr(program, vert);
        glAttachShaderPtr(program, frag);
        glLinkProgramPtr(program);

        GLint linkOk = 0;
        glGetProgramivPtr(program, GL_LINK_STATUS, &linkOk);
        if (!linkOk) {
            GLint logLen = 0;
            glGetProgramivPtr(program, GL_INFO_LOG_LENGTH, &logLen);
            std::string info;
            if (logLen > 0) {
                info.resize(static_cast<size_t>(logLen));
                GLsizei written = 0;
                glGetProgramInfoLogPtr(program, logLen, &written, info.data());
                info.resize(static_cast<size_t>(std::max(0, static_cast<int>(written))));
            }
            glDeleteShaderPtr(vert);
            glDeleteShaderPtr(frag);
            glDeleteProgramPtr(program);
            error = "Shader link error: " + info;
            return false;
        }

        if (glDetachShaderPtr) {
            glDetachShaderPtr(program, vert);
            glDetachShaderPtr(program, frag);
        }
        glDeleteShaderPtr(vert);
        glDeleteShaderPtr(frag);

        if (activeProgram != 0) {
            glDeleteProgramPtr(activeProgram);
            activeProgram = 0;
        }

        activeProgram = program;
        glUseProgramPtr(activeProgram);
        uniformTypes.clear();
        uniformLocations.clear();
        registerUniformTypesFromSource(vertexSource);
        registerUniformTypesFromSource(fragmentSource);
        return true;
#else
        (void)vertexSource;
        (void)fragmentSource;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setFragmentShader(const std::string& fragmentSource, std::string& error) {
        static const std::string kDefaultVertex =
            "#version 120\n"
            "void main() {\n"
            "    gl_Position = ftransform();\n"
            "    gl_FrontColor = gl_Color;\n"
            "}\n";
        return setShader(kDefaultVertex, fragmentSource, error);
    }

    bool setShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath, std::string& error) {
        std::string vs = readTextFile(vertexPath, error);
        if (!error.empty()) {
            return false;
        }
        std::string fs = readTextFile(fragmentPath, error);
        if (!error.empty()) {
            return false;
        }
        return setShader(vs, fs, error);
    }

    bool setFragmentShaderFromFile(const std::string& fragmentPath, std::string& error) {
        std::string fs = readTextFile(fragmentPath, error);
        if (!error.empty()) {
            return false;
        }
        return setFragmentShader(fs, error);
    }

    void clearShader() {
#if defined(__linux__)
        if (!shaderApiReady) {
            return;
        }
        flushTriangles();
        glUseProgramPtr(0);
        if (activeProgram != 0 && glDeleteProgramPtr) {
            glDeleteProgramPtr(activeProgram);
            activeProgram = 0;
        }
        uniformTypes.clear();
        uniformLocations.clear();
#endif
    }

    bool setUniformFloat(const std::string& name, float value, std::string& error) {
#if defined(__linux__)
        GLint location = -1;
        GLenum type = 0;
        if (!resolveUniform(name, location, type, error)) {
            return false;
        }
        if (!uniformTypeEquals(type, GL_FLOAT)) {
            error = "Uniform type mismatch for '" + name + "': expected float";
            return false;
        }
        flushTriangles();
        glUniform1fPtr(location, value);
        return true;
#else
        (void)name;
        (void)value;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setUniformInt(const std::string& name, int value, std::string& error) {
#if defined(__linux__)
        GLint location = -1;
        GLenum type = 0;
        if (!resolveUniform(name, location, type, error)) {
            return false;
        }
        if (!uniformTypeEquals(type, GL_INT)) {
            error = "Uniform type mismatch for '" + name + "': expected int";
            return false;
        }
        flushTriangles();
        glUniform1iPtr(location, value);
        return true;
#else
        (void)name;
        (void)value;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setUniformBool(const std::string& name, bool value, std::string& error) {
#if defined(__linux__)
        GLint location = -1;
        GLenum type = 0;
        if (!resolveUniform(name, location, type, error)) {
            return false;
        }
        if (!uniformTypeEquals(type, GL_BOOL)) {
            error = "Uniform type mismatch for '" + name + "': expected bool";
            return false;
        }
        flushTriangles();
        glUniform1iPtr(location, value ? 1 : 0);
        return true;
#else
        (void)name;
        (void)value;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setUniformVec2(const std::string& name, float x, float y, std::string& error) {
#if defined(__linux__)
        GLint location = -1;
        GLenum type = 0;
        if (!resolveUniform(name, location, type, error)) {
            return false;
        }
        if (!uniformTypeEquals(type, GL_FLOAT_VEC2)) {
            error = "Uniform type mismatch for '" + name + "': expected vec2";
            return false;
        }
        flushTriangles();
        glUniform2fPtr(location, x, y);
        return true;
#else
        (void)name;
        (void)x;
        (void)y;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setUniformVec3(const std::string& name, float x, float y, float z, std::string& error) {
#if defined(__linux__)
        GLint location = -1;
        GLenum type = 0;
        if (!resolveUniform(name, location, type, error)) {
            return false;
        }
        if (!uniformTypeEquals(type, GL_FLOAT_VEC3)) {
            error = "Uniform type mismatch for '" + name + "': expected vec3";
            return false;
        }
        flushTriangles();
        glUniform3fPtr(location, x, y, z);
        return true;
#else
        (void)name;
        (void)x;
        (void)y;
        (void)z;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool setUniformVec4(const std::string& name, float x, float y, float z, float w, std::string& error) {
#if defined(__linux__)
        GLint location = -1;
        GLenum type = 0;
        if (!resolveUniform(name, location, type, error)) {
            return false;
        }
        if (!uniformTypeEquals(type, GL_FLOAT_VEC4)) {
            error = "Uniform type mismatch for '" + name + "': expected vec4";
            return false;
        }
        flushTriangles();
        glUniform4fPtr(location, x, y, z, w);
        return true;
#else
        (void)name;
        (void)x;
        (void)y;
        (void)z;
        (void)w;
        error = "Shader support is not implemented for this platform.";
        return false;
#endif
    }

    bool shadersAvailable() const {
#if defined(__linux__)
        return shaderApiReady;
#else
        return false;
#endif
    }

    void ensureBackBufferSize(int width, int height) {
#if defined(__linux__)
        if (!display || !window || !glContext) {
            return;
        }
        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        glXMakeCurrent(display, window, glContext);
        glViewport(0, 0, safeWidth, safeHeight);
#else
        (void)width;
        (void)height;
#endif
    }

    void pumpEvents(
        bool& shouldClose,
        int& width,
        int& height,
        float& mouseX,
        float& mouseY,
        std::set<int>& keysDown,
        std::set<int>& keysPressed,
        std::set<int>& mouseButtonsDown,
        std::set<int>& mouseButtonsPressed
    ) {
#if defined(__linux__)
        if (!created || !display) {
            return;
        }

        while (XPending(display) > 0) {
            XEvent event;
            XNextEvent(display, &event);

            switch (event.type) {
                case ClientMessage:
                    if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteMessage) {
                        shouldClose = true;
                    }
                    break;
                case DestroyNotify:
                    shouldClose = true;
                    break;
                case ConfigureNotify:
                    if (width != event.xconfigure.width || height != event.xconfigure.height) {
                        width = event.xconfigure.width;
                        height = event.xconfigure.height;
                        ensureBackBufferSize(width, height);
                    }
                    break;
                case MotionNotify:
                    mouseX = static_cast<float>(event.xmotion.x);
                    mouseY = static_cast<float>(event.xmotion.y);
                    break;
                case KeyPress: {
                    const KeySym keySym = XLookupKeysym(&event.xkey, 0);
                    const int keyCode = static_cast<int>(keySym);
                    if (!keysDown.count(keyCode)) {
                        keysPressed.insert(keyCode);
                    }
                    keysDown.insert(keyCode);
                    break;
                }
                case KeyRelease: {
                    const KeySym keySym = XLookupKeysym(&event.xkey, 0);
                    keysDown.erase(static_cast<int>(keySym));
                    break;
                }
                case ButtonPress:
                    if (!mouseButtonsDown.count(static_cast<int>(event.xbutton.button))) {
                        mouseButtonsPressed.insert(static_cast<int>(event.xbutton.button));
                    }
                    mouseButtonsDown.insert(static_cast<int>(event.xbutton.button));
                    break;
                case ButtonRelease:
                    mouseButtonsDown.erase(static_cast<int>(event.xbutton.button));
                    break;
                default:
                    break;
            }
        }
#else
        (void)shouldClose;
        (void)width;
        (void)height;
        (void)mouseX;
        (void)mouseY;
        (void)keysDown;
        (void)mouseButtonsDown;
#endif
    }

    void setTitle(const std::string& title) {
#if defined(__linux__)
        if (created && display && window) {
            XStoreName(display, window, title.c_str());
            XFlush(display);
        }
#else
        (void)title;
#endif
    }

    void setSize(int width, int height) {
#if defined(__linux__)
        if (!created || !display || !window) {
            return;
        }

        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        XResizeWindow(
            display,
            window,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight)
        );
        ensureBackBufferSize(safeWidth, safeHeight);
        XFlush(display);
#else
        (void)width;
        (void)height;
#endif
    }

    void beginFrame(float clearR, float clearG, float clearB, int width, int height) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }

        pendingTriangles.clear();

        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        glXMakeCurrent(display, window, glContext);
        glViewport(0, 0, safeWidth, safeHeight);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, static_cast<double>(safeWidth), static_cast<double>(safeHeight), 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(std::clamp(clearR, 0.0f, 1.0f), std::clamp(clearG, 0.0f, 1.0f), std::clamp(clearB, 0.0f, 1.0f), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
#else
        (void)clearR;
        (void)clearG;
        (void)clearB;
        (void)width;
        (void)height;
#endif
    }

    void flushTriangles() {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        if (pendingTriangles.empty()) {
            return;
        }

        glBegin(GL_TRIANGLES);
        for (const auto& tri : pendingTriangles) {
            glColor3f(std::clamp(tri.r, 0.0f, 1.0f), std::clamp(tri.g, 0.0f, 1.0f), std::clamp(tri.b, 0.0f, 1.0f));
            glVertex3f(tri.x1, tri.y1, tri.z1);
            glVertex3f(tri.x2, tri.y2, tri.z2);
            glVertex3f(tri.x3, tri.y3, tri.z3);
        }
        glEnd();
        pendingTriangles.clear();
#endif
    }

    void endFrame(int width, int height) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }

        (void)width;
        (void)height;
        flushTriangles();
        glFlush();
        glXSwapBuffers(display, window);
        XFlush(display);
#else
        (void)width;
        (void)height;
#endif
    }

    void drawRect(int x, int y, unsigned int w, unsigned int h, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        flushTriangles();
        glColor3f(std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f));
        glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x + static_cast<int>(w), y);
        glVertex2i(x + static_cast<int>(w), y + static_cast<int>(h));
        glVertex2i(x, y + static_cast<int>(h));
        glEnd();
#else
        (void)x;
        (void)y;
        (void)w;
        (void)h;
        (void)r;
        (void)g;
        (void)b;
#endif
    }

    void drawCircle(int cx, int cy, int radius, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        flushTriangles();
        glColor3f(std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f));
        const int segments = std::max(12, radius * 2);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2i(cx, cy);
        for (int i = 0; i <= segments; i++) {
            const float t = static_cast<float>(i) / static_cast<float>(segments);
            const float ang = t * 2.0f * static_cast<float>(M_PI);
            const int px = cx + static_cast<int>(std::cos(ang) * static_cast<float>(radius));
            const int py = cy + static_cast<int>(std::sin(ang) * static_cast<float>(radius));
            glVertex2i(px, py);
        }
        glEnd();
#else
        (void)cx;
        (void)cy;
        (void)radius;
        (void)r;
        (void)g;
        (void)b;
#endif
    }

    void drawLine(int x1, int y1, int x2, int y2, int thickness, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        flushTriangles();
        glColor3f(std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f));
        glLineWidth(static_cast<float>(std::max(1, thickness)));
        glBegin(GL_LINES);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glEnd();
        glLineWidth(1.0f);
#else
        (void)x1;
        (void)y1;
        (void)x2;
        (void)y2;
        (void)thickness;
        (void)r;
        (void)g;
        (void)b;
#endif
    }

    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int thickness, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        (void)thickness;
        pendingTriangles.push_back(TriangleCommand{
            static_cast<float>(x1), static_cast<float>(y1), 0.0f,
            static_cast<float>(x2), static_cast<float>(y2), 0.0f,
            static_cast<float>(x3), static_cast<float>(y3), 0.0f,
            r, g, b
        });
#else
        (void)x1;
        (void)y1;
        (void)x2;
        (void)y2;
        (void)x3;
        (void)y3;
        (void)thickness;
        (void)r;
        (void)g;
        (void)b;
#endif
    }

    void drawTriangle3D(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, int thickness, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        (void)thickness;
        pendingTriangles.push_back(TriangleCommand{
            x1, y1, z1,
            x2, y2, z2,
            x3, y3, z3,
            r, g, b
        });
#else
        (void)x1;
        (void)y1;
        (void)z1;
        (void)x2;
        (void)y2;
        (void)z2;
        (void)x3;
        (void)y3;
        (void)z3;
        (void)thickness;
        (void)r;
        (void)g;
        (void)b;
#endif
    }

    void drawPolygon(const std::vector<std::pair<int, int>>& points, int thickness, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !window || !glContext) {
            return;
        }
        if (points.size() < 2) {
            return;
        }
        flushTriangles();
        glColor3f(std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f));
        glLineWidth(static_cast<float>(std::max(1, thickness)));
        glBegin(GL_LINE_LOOP);
        for (const auto& p : points) {
            glVertex2i(p.first, p.second);
        }
        glEnd();
        glLineWidth(1.0f);
#else
        (void)points;
        (void)thickness;
        (void)r;
        (void)g;
        (void)b;
#endif
    }

    void drawText(const std::string& text, int x, int y) {
#if defined(__linux__)
        if (!created || !display || !window || !gc) {
            return;
        }
        flushTriangles();
        // Text fallback remains X11-based; geometric primitives are GPU-rendered.
        XSetForeground(display, gc, colorToPixel(1.0f, 1.0f, 1.0f));
        XDrawString(display, window, gc, x, y, text.c_str(), static_cast<int>(text.size()));
#else
        (void)text;
        (void)x;
        (void)y;
#endif
    }

    void setMousePosition(int x, int y) {
#if defined(__linux__)
        if (created && display && window) {
            XWarpPointer(display, None, window, 0, 0, 0, 0, x, y);
            XFlush(display);
        }
#else
        (void)x;
        (void)y;
#endif
    }

    Cursor createInvisibleCursor() {
#if defined(__linux__)
        if (!display || !window) {
            return 0;
        }
        char bitmapData[1] = {0};
        Pixmap bitmap = XCreateBitmapFromData(display, window, bitmapData, 1, 1);
        XColor black;
        black.red = black.green = black.blue = 0;
        Cursor cursor = XCreatePixmapCursor(display, bitmap, bitmap, &black, &black, 0, 0);
        XFreePixmap(display, bitmap);
        return cursor;
#else
        return 0;
#endif
    }

    void setMouseVisible(bool visible) {
#if defined(__linux__)
        if (!created || !display || !window) {
            return;
        }

        if (visible) {
            XUndefineCursor(display, window);
        } else {
            if (!invisibleCursor) {
                invisibleCursor = createInvisibleCursor();
            }
            XDefineCursor(display, window, invisibleCursor);
        }
        XFlush(display);
#else
        (void)visible;
#endif
    }

    void setMouseCaptured(bool capture) {
#if defined(__linux__)
        if (!created || !display || !window) {
            return;
        }

        if (capture) {
            XGrabPointer(
                display,
                window,
                True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync,
                GrabModeAsync,
                window,
                None,
                CurrentTime
            );
        } else {
            XUngrabPointer(display, CurrentTime);
        }
        XFlush(display);
#else
        (void)capture;
#endif
    }

    int resolveKeyCode(const std::string& keyName) const {
        if (keyName.empty()) {
            return 0;
        }

#if defined(__linux__)
        if (keyName.size() == 1) {
            const unsigned char ch = static_cast<unsigned char>(keyName[0]);
            const char normalized = static_cast<char>(std::tolower(ch));
            KeySym sym = XStringToKeysym(std::string(1, normalized).c_str());
            if (sym != NoSymbol) {
                return static_cast<int>(sym);
            }
        }

        KeySym sym = XStringToKeysym(keyName.c_str());
        if (sym != NoSymbol) {
            return static_cast<int>(sym);
        }

        std::string lower = keyName;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        sym = XStringToKeysym(lower.c_str());
        if (sym != NoSymbol) {
            return static_cast<int>(sym);
        }

        std::string upper = keyName;
        std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        sym = XStringToKeysym(upper.c_str());
        if (sym != NoSymbol) {
            return static_cast<int>(sym);
        }
#endif

        if (keyName.size() == 1) {
            return static_cast<int>(keyName[0]);
        }

        return 0;
    }
};

#pragma once

#include <algorithm>
#include <cmath>
#include <cctype>
#include <set>
#include <string>

#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

struct NativeWindowBackend {
#if defined(__linux__)
    Display* display = nullptr;
    int screen = 0;
    Window window = 0;
    GC gc = 0;
    Pixmap backBuffer = 0;
    Atom wmDeleteMessage = 0;
    Cursor invisibleCursor = 0;
    Visual* visual = nullptr;
    unsigned long redMask = 0;
    unsigned long greenMask = 0;
    unsigned long blueMask = 0;
    int redShift = 0;
    int greenShift = 0;
    int blueShift = 0;
#endif

    bool created = false;

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
        visual = DefaultVisual(display, screen);
        redMask = visual ? visual->red_mask : 0;
        greenMask = visual ? visual->green_mask : 0;
        blueMask = visual ? visual->blue_mask : 0;
        redShift = computeMaskShift(redMask);
        greenShift = computeMaskShift(greenMask);
        blueShift = computeMaskShift(blueMask);

        window = XCreateSimpleWindow(
            display,
            RootWindow(display, screen),
            100,
            100,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight),
            0,
            BlackPixel(display, screen),
            BlackPixel(display, screen)
        );

        if (!window) {
            error = "Failed to create X11 window.";
            destroy();
            return false;
        }

        XStoreName(display, window, title.c_str());
        XSelectInput(
            display,
            window,
            ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
            ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask
        );

        wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, window, &wmDeleteMessage, 1);

        gc = XCreateGC(display, window, 0, nullptr);
        if (!gc) {
            error = "Failed to create X11 graphics context.";
            destroy();
            return false;
        }

        backBuffer = XCreatePixmap(
            display,
            window,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight),
            static_cast<unsigned int>(DefaultDepth(display, screen))
        );
        if (!backBuffer) {
            error = "Failed to create X11 backbuffer pixmap.";
            destroy();
            return false;
        }

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
            if (backBuffer) {
                XFreePixmap(display, backBuffer);
                backBuffer = 0;
            }
            if (gc) {
                XFreeGC(display, gc);
                gc = 0;
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
        redMask = greenMask = blueMask = 0;
#endif
        created = false;
    }

    void ensureBackBufferSize(int width, int height) {
#if defined(__linux__)
        if (!display || !window) {
            return;
        }
        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        if (backBuffer) {
            XFreePixmap(display, backBuffer);
            backBuffer = 0;
        }
        backBuffer = XCreatePixmap(
            display,
            window,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight),
            static_cast<unsigned int>(DefaultDepth(display, screen))
        );
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
        std::set<int>& mouseButtonsDown
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
                    keysDown.insert(static_cast<int>(keySym));
                    break;
                }
                case KeyRelease: {
                    const KeySym keySym = XLookupKeysym(&event.xkey, 0);
                    keysDown.erase(static_cast<int>(keySym));
                    break;
                }
                case ButtonPress:
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
        if (!created || !display || !backBuffer || !gc) {
            return;
        }

        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        const unsigned long clearColor = colorToPixel(clearR, clearG, clearB);
        XSetForeground(display, gc, clearColor);
        XFillRectangle(
            display,
            backBuffer,
            gc,
            0,
            0,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight)
        );
#else
        (void)clearR;
        (void)clearG;
        (void)clearB;
        (void)width;
        (void)height;
#endif
    }

    void endFrame(int width, int height) {
#if defined(__linux__)
        if (!created || !display || !window || !backBuffer || !gc) {
            return;
        }

        const int safeWidth = std::max(1, width);
        const int safeHeight = std::max(1, height);
        XCopyArea(
            display,
            backBuffer,
            window,
            gc,
            0,
            0,
            static_cast<unsigned int>(safeWidth),
            static_cast<unsigned int>(safeHeight),
            0,
            0
        );
        XFlush(display);
#else
        (void)width;
        (void)height;
#endif
    }

    void drawRect(int x, int y, unsigned int w, unsigned int h, float r, float g, float b) {
#if defined(__linux__)
        if (!created || !display || !backBuffer || !gc) {
            return;
        }
        XSetForeground(display, gc, colorToPixel(r, g, b));
        XFillRectangle(display, backBuffer, gc, x, y, w, h);
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
        if (!created || !display || !backBuffer || !gc) {
            return;
        }
        XSetForeground(display, gc, colorToPixel(r, g, b));
        XFillArc(
            display,
            backBuffer,
            gc,
            cx - radius,
            cy - radius,
            static_cast<unsigned int>(std::max(0, radius * 2)),
            static_cast<unsigned int>(std::max(0, radius * 2)),
            0,
            360 * 64
        );
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
        if (!created || !display || !backBuffer || !gc) {
            return;
        }
        XSetForeground(display, gc, colorToPixel(r, g, b));
        XSetLineAttributes(display, gc, std::max(1, thickness), LineSolid, CapRound, JoinRound);
        XDrawLine(display, backBuffer, gc, x1, y1, x2, y2);
        XSetLineAttributes(display, gc, 1, LineSolid, CapButt, JoinMiter);
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

    void drawText(const std::string& text, int x, int y) {
#if defined(__linux__)
        if (!created || !display || !backBuffer || !gc) {
            return;
        }
        XSetForeground(display, gc, colorToPixel(1.0f, 1.0f, 1.0f));
        XDrawString(display, backBuffer, gc, x, y, text.c_str(), static_cast<int>(text.size()));
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

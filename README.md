# GE Language (Current Implemented Behavior)

GE is an interpreted scripting language aimed at game-style math and logic.

This document describes what the interpreter supports right now.

## Quick Start

Build and run a script:

~~~bash
bash run.sh main.ge
~~~

Or compile and run directly:

~~~bash
g++ -std=c++17 interpreter/run.cpp -o run
./run test.ge
~~~

On Linux, when using the built-in window library, compile with X11:

~~~bash
g++ -std=c++17 interpreter/run.cpp -o run -lX11
~~~

## File Imports

Imports use angle brackets and do not require a semicolon.

~~~cpp
import <perlin.ge>
import <math/helpers.ge>
~~~

Import behavior:
- Imported files are executed once.
- Re-importing the same path is ignored.
- Imported functions become available to the importing file.

## Comments

Line comments are supported:

~~~cpp
// this is a comment
~~~

## Core Types

Built-in variable types:
- void
- int
- float
- string
- bool
- list
- vec2
- vec3
- vec4

Examples:

~~~cpp
int i = 10;
float f = 3.14;
float n = -2.5;
string s = "hello";
bool flag = true;
list items = [1, 2, 3, "ok", false];
vec2 a = vec2(1.0, 2.0);
vec3 b = vec3(1.0, 2.0, 3.0);
vec4 c = vec4(1.0, 2.0, 3.0, 4.0);
~~~

## Numbers and Precision

- Negative numeric literals are supported (for example -1, -3.5).
- Runtime numeric computation uses expression evaluation with high precision intermediates.
- Stored float/vector fields are compact float-backed values for memory efficiency.

## Operators

Arithmetic:
- `+`
- `-`
- `*`
- `/`
- `%`
- `^`

Comparison:
- `==`
- `!=`
- `<`
- `<=`
- `>`
- `>=`

## Statements

### Variable Declaration

Semicolon required.

~~~cpp
float x = 1.0;
vec3 p = vec3(0.0, 1.0, 2.0);
list vals = [1, 2, 3];
~~~

### Assignment

Semicolon required.

~~~cpp
x = x + 1.0;
p = p + vec3(1.0, 0.0, 0.0);
~~~

### Member Assignment

Semicolon required.

~~~cpp
p.x = 5.0;
p.y = p.y + 1.0;
~~~

### Print

Parentheses required. Semicolon required.

~~~cpp
print("hello");
print(x);
print(p);
~~~

### Function Call Statement

Semicolon required.

~~~cpp
updatePlayer(pos);
~~~

## Control Flow

### If / Else

~~~cpp
if (x > 0.0) {
	print("positive");
} else {
	print("non-positive");
}
~~~

### While

~~~cpp
while (x < 10.0) {
	x = x + 1.0;
}
~~~

### For

Current for-loop format:
- C-style header with semicolons inside parentheses.
- Initialization is currently expected as a typed declaration.
- Increment supports ++i, i++, --i, i--, and assignment-style forms.

~~~cpp
for (int i = 0; i < 10; i++) {
	print(i);
}
~~~

## Functions

Function definitions are typed and do not use a function keyword.

~~~cpp
float add(float a, float b) {
	return a + b;
}

void logValue(float x) {
	print(x);
}
~~~

Rules:
- Return type must be declared.
- Parameters are typed.
- Function overloading is type-based.
- Calls must match argument types expected by available overloads.

## Vectors

Supported constructors and operations:
- vec2(...), vec3(...), vec4(...)
- Scalar splat constructors are supported:
	vec2(n) -> vec2(n, n)
	vec3(n) -> vec3(n, n, n)
	vec4(n) -> vec4(n, n, n, n)
- Vector-vector math (+, -, *, /)
- Vector-scalar math (+, -, *, /)
- Member access (x, y, z, w)

Example:

~~~cpp
vec3 cam = vec3(0.0, 0.0, 0.0);
vec3 origin = vec3(0.0);
vec3 pos = vec3(5.0, 2.0, 8.0);
vec3 p = pos - cam;
print(p);
~~~

## Lists

List literals:

~~~cpp
list nums = [1, 2, 3];
~~~

List methods currently supported:
- Mutating statement methods:
  - add(value)
  - remove(index)
  - clear()
- Read methods (usable in expressions and also as statements):
  - size()
  - get(index)
  - indexOf(value)

Examples:

~~~cpp
list nums = [1, 2, 3];
nums.add(4);
nums.remove(0);
print(nums.size());
print(nums.get(1));
print(nums.indexOf(4));
~~~

## Built-in Functions

Constructors and math helpers currently available:
- vec2
- vec3
- vec4
- sin
- cos
- tan
- length
- dot
- normalize
- cross
- min
- max
- clamp
- abs
- floor
- ceil
- fract
- sign
- smoothstep
- radians
- degrees
- mix
- sqrt

## Built-in Library: window

The `window` library is built into the interpreter and can be loaded with:

~~~cpp
import <window>
~~~

It is exposed through `window.*` methods.

### Setup and Window State
- `window.create(width, height, title)`
- `window.close()`
- `window.setTitle(title)`
- `window.setSize(width, height)`
- `window.platform()`
- `window.isOpen()`
- `window.shouldClose()`
- `window.width()`
- `window.height()`

### Frame and Drawing API
- `window.beginFrame()`
- `window.endFrame()`
- `window.clear(r, g, b, a)`
- `window.drawRect(x, y, w, h, r, g, b, a)`
- `window.drawCircle(x, y, radius, r, g, b, a)`
- `window.drawLine(x1, y1, x2, y2, thickness, r, g, b, a)`
- `window.drawText(text, x, y, size)`
- `window.drawCount()`

### Input and Mouse API
- `window.mouseX()`
- `window.mouseY()`
- `window.setMousePosition(x, y)`
- `window.setMouseVisible(visible)`
- `window.captureMouse(enabled)`
- `window.keyDown(keyCode)`
- `window.keyPressed(keyCode)`
- `window.mouseDown(button)`
- `window.mousePressed(button)`

### Shader API (GLSL)
- `window.shaderSupported()`
- `window.setShader(vertexSource, fragmentSource)`
- `window.setFragmentShader(fragmentSource)`
- `window.setShaderFile(vertexPath, fragmentPath)`
- `window.setFragmentShaderFile(fragmentPath)`
- `window.setUniform(name, value)`
- `window.clearShader()`

`window.setUniform` accepts GE value types: `int`, `float`, `bool`, `vec2`, `vec3`, `vec4`.
The shader uniform type must match exactly (for example, `vec3` value must target a GLSL `vec3` uniform).

### Input State Helpers (for scripting/testing)
- `window.setKeyState(keyCode, down)`
- `window.setMouseButton(button, down)`

### Example

~~~cpp
import <window>

window.create(1280, 720, "GE Window Demo");
window.beginFrame();
window.clear(0.1, 0.1, 0.12, 1.0);
window.drawRect(50, 60, 240, 120, 0.3, 0.7, 1.0, 1.0);
window.drawText("Hello", 72, 90, 24);
window.endFrame();

print(window.width());
print(window.height());
print(window.drawCount());
~~~

The window backend is native to the operating system. On Linux, this currently uses X11, so created windows match OS-native window behavior.

## Current Syntax Notes

- Most statements require semicolons.
- Import intentionally does not require a semicolon.
- print requires parentheses.
- Method calls are currently supported on list objects and the built-in window library.
- Braces are required for if/else/for/while blocks.

## Example Program

~~~cpp
import <perlin.ge>

float add(float a, float b) {
	return a + b;
}

void runDemo() {
	list vals = [1, 2, 3];
	vals.add(4);

	vec3 cam = vec3(0.0, 0.0, 0.0);
	vec3 pos = vec3(1.0, 2.0, 3.0);
	vec3 p = pos - cam;

	print("vals size:");
	print(vals.size());
	print("p:");
	print(p);
	print("sum:");
	print(add(-3.5, 1.25));
}

runDemo();
~~~

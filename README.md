# GE Language (Current Implemented Behavior)

GE is an interpreted scripting language aimed at game-style math and logic.

This document describes what the interpreter supports right now.

## Quick Start

Build and run a script:

~~~bash
bash run.sh test.ge
~~~

Or compile and run directly:

~~~bash
g++ -std=c++17 interpreter/run.cpp -o run
./run test.ge
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
- Vector-vector math (+, -, *, /)
- Vector-scalar math (+, -, *, /)
- Member access (x, y, z, w)

Example:

~~~cpp
vec3 cam = vec3(0.0, 0.0, 0.0);
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

## Current Syntax Notes

- Most statements require semicolons.
- Import intentionally does not require a semicolon.
- print requires parentheses.
- Method calls are currently supported on list objects.
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

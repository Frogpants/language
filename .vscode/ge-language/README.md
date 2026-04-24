# GE Language Support for VS Code

Full IDE support for the GE programming language with syntax highlighting, code execution, and compilation features.

## Features

- **Syntax Highlighting**: Full syntax highlighting for `.ge` files
- **Run Directly**: Execute `.ge` files from within VS Code
- **Export to Windows**: Generate a runnable `.cmd` launcher for `.ge` files
- **Export to Linux**: Generate a runnable executable launcher for `.ge` files
- **Built-in Terminal**: View execution output directly in VS Code

## Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| `GE: Run GE` | Ctrl+Shift+R (Cmd+Shift+R on Mac) | Execute the current `.ge` file |
| `GE: Export to Windows` | — | Generate a Windows launcher (`.cmd`) |
| `GE: Export to Linux` | — | Generate a Linux executable launcher |

## Usage

### Running a GE Script

1. Open a `.ge` file in VS Code
2. Press `Ctrl+Shift+R` (or `Cmd+Shift+R` on Mac)
3. View the output in the GE output channel

### Exporting a GE Script

1. Open a `.ge` file in VS Code
2. Run command `GE: Export to Windows` or `GE: Export to Linux`
3. The launcher will be created in your project root

## Requirements

- The GE interpreter (`ge` executable) must be in your project root
- On Linux: X11 and OpenGL libraries (usually pre-installed)
- On Windows: No additional requirements (launcher uses the local `ge` executable)

## Project Structure

Your GE project should have the following structure:

```
project/
├── ge                          (GE interpreter executable)
├── interpreter/
│   └── (core interpreter files)
├── main.ge                     (your main script)
└── (other .ge files)
```

## Installation

1. Copy this extension to your VS Code extensions directory
2. Reload VS Code
3. The extension will activate when you open a `.ge` file

## License

This extension is part of the GE language project.

import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import * as os from 'os';
import { spawn, spawnSync } from 'child_process';

export function activate(context: vscode.ExtensionContext) {
    // Register "Run GE" command
    const runGeCommand = vscode.commands.registerCommand('ge.run', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active editor');
            return;
        }

        const filePath = editor.document.fileName;
        if (!filePath.endsWith('.ge')) {
            vscode.window.showErrorMessage('Current file is not a .ge file');
            return;
        }

        const geExe = resolveGeExecutable(filePath);
        if (!geExe) {
            vscode.window.showErrorMessage('Could not find GE executable. Set ge.executablePath or add ge to PATH.');
            return;
        }

        const projectRoot = getWorkspaceRoot(filePath);

        // Create output channel
        const outputChannel = vscode.window.createOutputChannel('GE');
        outputChannel.show();
        outputChannel.appendLine(`Running: ${filePath}`);

        try {
            const process = spawn(geExe, [filePath], {
                cwd: projectRoot,
                stdio: ['pipe', 'pipe', 'pipe']
            });

            process.stdout.on('data', (data) => {
                outputChannel.append(data.toString());
            });

            process.stderr.on('data', (data) => {
                outputChannel.append(data.toString());
            });

            process.on('close', (code) => {
                outputChannel.appendLine(`\nProcess exited with code ${code}`);
            });
        } catch (error) {
            vscode.window.showErrorMessage(`Error running GE: ${error}`);
        }
    });

    // Register "Export to Windows" command
    const exportWindowsCommand = vscode.commands.registerCommand('ge.exportWindows', async () => {
        await exportGe('windows');
    });

    // Register "Export to Linux" command
    const exportLinuxCommand = vscode.commands.registerCommand('ge.exportLinux', async () => {
        await exportGe('linux');
    });

    context.subscriptions.push(runGeCommand, exportWindowsCommand, exportLinuxCommand);
}

function getWorkspaceRoot(filePath: string): string {
    const folder = vscode.workspace.getWorkspaceFolder(vscode.Uri.file(filePath));
    return folder ? folder.uri.fsPath : path.dirname(filePath);
}

function resolveGeExecutable(filePath: string): string | null {
    const config = vscode.workspace.getConfiguration('ge', vscode.Uri.file(filePath));
    const configuredPath = (config.get<string>('executablePath') || '').trim();
    const workspaceRoot = getWorkspaceRoot(filePath);

    const candidates: string[] = [];

    if (configuredPath) {
        if (path.isAbsolute(configuredPath)) {
            candidates.push(configuredPath);
        } else {
            candidates.push(path.join(workspaceRoot, configuredPath));
        }
    }

    if (process.platform === 'win32') {
        candidates.push(path.join(workspaceRoot, 'ge.exe'));
    } else {
        candidates.push(path.join(workspaceRoot, 'ge'));
    }

    for (const candidate of candidates) {
        if (fs.existsSync(candidate)) {
            return candidate;
        }
    }

    const lookup = process.platform === 'win32'
        ? spawnSync('where', ['ge'], { encoding: 'utf8' })
        : spawnSync('which', ['ge'], { encoding: 'utf8' });

    if (lookup.status === 0 && lookup.stdout) {
        const firstLine = lookup.stdout.split(/\r?\n/).map(s => s.trim()).find(Boolean);
        if (firstLine && fs.existsSync(firstLine)) {
            return firstLine;
        }
    }

    return null;
}

async function exportGe(platform: 'windows' | 'linux') {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showErrorMessage('No active editor');
        return;
    }

    const filePath = editor.document.fileName;
    if (!filePath.endsWith('.ge')) {
        vscode.window.showErrorMessage('Current file is not a .ge file');
        return;
    }

    const geExe = resolveGeExecutable(filePath);
    if (!geExe) {
        vscode.window.showErrorMessage('Could not find GE executable. Set ge.executablePath or add ge to PATH.');
        return;
    }

    const projectRoot = getWorkspaceRoot(filePath);
    const baseName = path.basename(filePath, '.ge');
    const importPath = path.relative(projectRoot, filePath).split(path.sep).join('/');
    const outputFile = platform === 'windows'
        ? path.join(projectRoot, `${baseName}.cmd`)
        : path.join(projectRoot, baseName);

    const outputChannel = vscode.window.createOutputChannel('GE Export');
    outputChannel.show();
    outputChannel.appendLine(`Exporting launcher for ${platform}: ${baseName}`);

    try {
        const sourcePath = platform === 'windows'
            ? importPath.split('/').join('\\')
            : importPath;

        if (platform === 'linux') {
            const launcher = `#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
GE_BIN="${geExe}"
SOURCE_FILE="\${SCRIPT_DIR}/${sourcePath}"

if [[ ! -x "$GE_BIN" ]]; then
    if command -v ge >/dev/null 2>&1; then
        GE_BIN="$(command -v ge)"
    else
        echo "Could not find GE executable." >&2
        exit 1
    fi
fi

exec "$GE_BIN" "$SOURCE_FILE"
`;
            fs.writeFileSync(outputFile, launcher, { mode: 0o755 });
        } else {
            const windowsGeExe = geExe.split('/').join('\\');
            const windowsSource = sourcePath;
            const launcher = `@echo off
setlocal
set "GE_BIN=${windowsGeExe}"
set "SOURCE_FILE=%~dp0${windowsSource}"
if exist "%GE_BIN%" (
    "%GE_BIN%" "%SOURCE_FILE%"
) else (
    ge "%SOURCE_FILE%"
)
exit /b %errorlevel%
`;
            fs.writeFileSync(outputFile, launcher, { encoding: 'utf8' });
        }

        fs.chmodSync(outputFile, 0o755);
        outputChannel.appendLine(`\n✓ Export successful: ${outputFile}`);
        vscode.window.showInformationMessage(`Exported launcher: ${path.basename(outputFile)}`);
    } catch (error) {
        vscode.window.showErrorMessage(`Error exporting: ${error}`);
    }
}

export function deactivate() {}

# ZotBins Core

## Prerequisites
- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO](https://platformio.org/), a cross-platform VSCode-based IDE for embedded system development

## How to Build & Upload
1. First, clone the repository using the command
```bash
git clone git@github.com:zotbins/ZotbinsCore.git
```

2. Create an `.env` file in the project directory following `.env.txt` as a template.

3. Fill in the values in `.env` with the appropriate values.

4. Build the project by clicking on the ```Build Button``` (the checkmark) in the PlatformIO Toolbar, located in the left corner.

![PlatformIO VSCode Toolbar](docs/platformio-ide-vscode-toolbar.png)

Image from [PlatformIO Quick Start](https://docs.platformio.org/en/latest/ide/vscode.html#id2)

5. Upload the application by clicking on the ```Upload Button``` (the arrow pointing right) in the PlatformIO Toolbar.

6. Verify that the application was successfully uploaded by clicking on the ```Serial Monitor Button``` (the electical plug) for logs.

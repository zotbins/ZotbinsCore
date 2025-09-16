# code formatting guide

this project uses **clang-format** to enforce consistent style for all C/C++ source files.

formatting is automatically checked in GitHub Actions when you open or update a pull request. if your code doesn’t match the expected style (defined in `.clang-format` at the root of the repo), the check will fail.

## when do i need to fix formatting?

you’ll see a PR failure like:

```
Run DoozyX/clang-format-lint-action@v0.18.1
format-check failed
```

that means some of your files don't match the required style.

## how to fix formatting

you can apply the correct formatting locally using the `clang-format` tool.

### 1. install clang-format

- **macOS**  
  ```bash
  brew install clang-format
  ```

- **ubuntu/debian**  
  ```bash
  sudo apt install clang-format
  ```

- **windows**  
  use the [LLVM installer](https://releases.llvm.org/download.html) or install the `Clang-Format` VSCode extension

### 2. run clang-format across the repo

from the project root, run:

```bash
find components main -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
  -exec clang-format -i --style=file {} +
```

this will format all relevant source files in-place, based on the rules in `.clang-format`.

then just commit and push the changes:

```bash
git add .
git commit -m "apply clang-format"
git push
```

your PR should now pass the formatting check.

## optional: automate it before every commit

to avoid manual formatting, you can set up a `pre-commit` git hook to auto-format your code before each commit:

create a file at `.git/hooks/pre-commit` with this content:

```bash
#!/bin/sh
echo "running clang-format..."
find components main -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
  -exec clang-format -i --style=file {} +
```

then make it executable:

```bash
chmod +x .git/hooks/pre-commit
```

## customizing the style

the formatting rules are defined in `.clang-format` (already in the root of this repo). if you want to update the style guide, you can regenerate a config with:

```bash
clang-format -style=llvm -dump-config > .clang-format
```

then tweak it as needed (e.g., changing indentation width, brace style, etc.).

## tl;dr

- run the `find ... | clang-format` command  
- commit and push your changes  
- your PR formatting check will pass

### Applying clang-format to Your Project

You can apply consistent formatting locally using the `clang-format` tool.

### 1. Install clang-format

**macOS**
brew install clang-format

**Ubuntu/Debian**
sudo apt install clang-format

**Windows**
- Use the LLVM installer, or  
- Install something like MSYS and use pacman, or
- Install the **Clang-Format** VSCode extension

### 2. Run clang-format Across the Repo

**Please create a commit before you run clang-format in case any of the changes are breaking.** From the project root, run:

```
find components main -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
  -exec clang-format -i --style=file {} +
```

This will format all relevant source files in-place, based on the rules in `.clang-format`.

Right now `clang-format` is set up to not touch the headers beceause the RTOS headers' orders are very sensitive.

### 3. Commit and Push the Changes

Before you commit the changes from `clang-format`, attempt to build the project to see if any changes were breaking. If the build is unaffected, then run:

```
git add .
git commit -m "apply clang-format"
git push
```

### 4. Undo Formatting (if needed)

If formatting breaks the build please revert the changes and document it:

- Restore everything in the repo:
`git restore .`
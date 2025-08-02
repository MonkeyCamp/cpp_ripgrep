# GitHub Workflows

This directory contains GitHub Actions workflows for the cpp_ripgrep project.

## Workflow Files

### 1. `ci.yml` - Comprehensive CI
**Triggers:** Push to main/develop, Pull requests
**Purpose:** Full continuous integration with multiple compiler versions and platforms

**Features:**
- Linux builds with GCC 9-12 and Clang 10-15
- macOS builds with Clang and GCC
- Windows builds with MSVC and MinGW
- Cross-compilation for Windows from Linux
- Code quality checks (clang-tidy, cppcheck)
- Performance benchmarks
- Package creation for releases

### 2. `pr.yml` - Pull Request Workflow
**Triggers:** Pull requests to main/develop
**Purpose:** Fast feedback for pull requests

**Features:**
- Essential Linux build with GCC
- Essential macOS build
- Code quality checks
- Basic functionality tests

### 3. `release.yml` - Release Workflow
**Triggers:** Push of version tags (v*)
**Purpose:** Create GitHub releases with packaged binaries

**Features:**
- Builds for Linux x64, Windows x64, Windows x86
- Creates compressed packages (.tar.gz for Linux, .zip for Windows)
- Automatically creates GitHub release with release notes
- Uploads binaries as release assets

### 4. `daily.yml` - Daily Build
**Triggers:** Daily at 2 AM UTC, manual dispatch
**Purpose:** Comprehensive daily testing and monitoring

**Features:**
- Comprehensive Linux builds with multiple compilers
- Performance benchmarking
- Cross-compilation testing
- Memory leak detection with Valgrind
- Extended test suite

## Usage

### For Contributors

1. **Pull Requests:** The `pr.yml` workflow will automatically run on your PR
2. **Code Quality:** Ensure your code passes clang-tidy and cppcheck checks
3. **Testing:** Add tests for new features in the test sections

### For Maintainers

1. **Releases:** Create a new tag to trigger automatic release creation:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. **Daily Monitoring:** Check the daily workflow results for performance regressions

3. **Manual Testing:** Use the "workflow_dispatch" trigger in daily.yml for manual runs

## Workflow Dependencies

### Linux Dependencies
- `build-essential` - Basic build tools
- `cmake` - Build system
- `libpcre2-dev` - PCRE2 regex library
- `libre2-dev` - RE2 regex library
- `pkg-config` - Package configuration
- `clang-tidy` - Code analysis
- `cppcheck` - Static analysis
- `valgrind` - Memory checking

### Windows Cross-Compilation
- `gcc-mingw-w64-x86-64` - 64-bit Windows cross-compiler
- `gcc-mingw-w64-i686` - 32-bit Windows cross-compiler
- `wine64` and `wine32` - Windows emulation for testing

### macOS Dependencies
- `cmake` - Build system
- `pcre2` - PCRE2 regex library
- `re2` - RE2 regex library

## Customization

### Adding New Compilers
To add support for new compiler versions, modify the `matrix.compiler` arrays in the workflows.

### Adding New Platforms
To add support for new platforms (e.g., ARM), add new jobs with appropriate runners and toolchains.

### Modifying Tests
Update the test sections in each workflow to include new functionality tests.

### Performance Benchmarks
The benchmark jobs create test data and measure performance. Modify the benchmark data generation and test patterns as needed.

## Troubleshooting

### Common Issues

1. **Build Failures:**
   - Check if all dependencies are installed
   - Verify CMake configuration
   - Check compiler compatibility

2. **Test Failures:**
   - Ensure test files are created correctly
   - Check expected output matches actual output
   - Verify command-line arguments

3. **Cross-Compilation Issues:**
   - Verify MinGW toolchain installation
   - Check toolchain file paths
   - Ensure Wine is available for testing

### Debugging

1. **Enable Debug Output:** Add `-DCMAKE_BUILD_TYPE=Debug` to CMake configuration
2. **Verbose Build:** Add `VERBOSE=1` to make commands
3. **Check Logs:** Review workflow logs for detailed error messages

## Performance Considerations

- The comprehensive CI workflow can take 30-60 minutes to complete
- Pull request workflow is optimized for speed (5-15 minutes)
- Daily workflow includes performance benchmarks to detect regressions
- Release workflow creates optimized builds for distribution

## Security

- All workflows use official GitHub Actions
- Dependencies are installed from official package repositories
- No sensitive data is exposed in workflow logs
- Release workflow requires appropriate permissions for creating releases 
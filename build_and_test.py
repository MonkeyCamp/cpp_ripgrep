import os
import subprocess
import sys

def run_command(args, cwd=None):
    print(f"Running command: {' '.join(args)}")
    result = subprocess.run(args, cwd=cwd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running command: {' '.join(args)}")
        print(f"Stdout: {result.stdout}")
        print(f"Stderr: {result.stderr}")
        sys.exit(1)
    print(result.stdout)
    return result

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(script_dir, "build")

    # Clean build directory
    if os.path.exists(build_dir):
        import shutil
        shutil.rmtree(build_dir)

    os.makedirs(build_dir)

    # Configure
    cmake_args = ["cmake", "-S", script_dir, "-B", build_dir]
    run_command(cmake_args)

    # Build
    build_args = ["cmake", "--build", build_dir]
    run_command(build_args)

    # Test
    test_args = ["ctest", "--output-on-failure"]
    run_command(test_args, cwd=build_dir)

if __name__ == "__main__":
    main()

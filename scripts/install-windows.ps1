# PowerShell script for Windows users to install cpp_ripgrep
# Run this script as Administrator

param(
    [string]$InstallPath = "C:\Program Files\cpp_ripgrep",
    [switch]$AddToPath
)

Write-Host "Installing cpp_ripgrep for Windows..." -ForegroundColor Green

# Check if running as Administrator
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "This script requires Administrator privileges. Please run as Administrator." -ForegroundColor Red
    exit 1
}

# Create installation directory
if (!(Test-Path $InstallPath)) {
    New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
    Write-Host "Created installation directory: $InstallPath" -ForegroundColor Yellow
}

# Determine architecture
$Architecture = $env:PROCESSOR_ARCHITECTURE
if ($Architecture -eq "AMD64") {
    $ExeName = "cpp_ripgrep-x64.exe"
    Write-Host "Detected 64-bit Windows" -ForegroundColor Yellow
} else {
    $ExeName = "cpp_ripgrep-x86.exe"
    Write-Host "Detected 32-bit Windows" -ForegroundColor Yellow
}

# Check if executable exists in current directory
$ExePath = Join-Path $PSScriptRoot "..\dist\$ExeName"
if (!(Test-Path $ExePath)) {
    Write-Host "Error: $ExeName not found in dist directory" -ForegroundColor Red
    Write-Host "Please run the build script first: scripts/build-windows.sh" -ForegroundColor Yellow
    exit 1
}

# Copy executable to installation directory
$TargetPath = Join-Path $InstallPath "cpp_ripgrep.exe"
Copy-Item $ExePath $TargetPath -Force
Write-Host "Installed cpp_ripgrep to: $TargetPath" -ForegroundColor Green

# Add to PATH if requested
if ($AddToPath) {
    $CurrentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
    if ($CurrentPath -notlike "*$InstallPath*") {
        $NewPath = "$CurrentPath;$InstallPath"
        [Environment]::SetEnvironmentVariable("PATH", $NewPath, "Machine")
        Write-Host "Added $InstallPath to system PATH" -ForegroundColor Green
        Write-Host "Note: You may need to restart your terminal for PATH changes to take effect" -ForegroundColor Yellow
    } else {
        Write-Host "Installation directory already in PATH" -ForegroundColor Yellow
    }
}

# Test installation
Write-Host "Testing installation..." -ForegroundColor Yellow
try {
    & $TargetPath --version
    Write-Host "Installation successful!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage examples:" -ForegroundColor Cyan
    Write-Host "  cpp_ripgrep.exe 'pattern' file.txt" -ForegroundColor White
    Write-Host "  cpp_ripgrep.exe -i 'hello' *.txt" -ForegroundColor White
    Write-Host "  cpp_ripgrep.exe --regex-engine re2 '\\w+' document.txt" -ForegroundColor White
} catch {
    Write-Host "Error testing installation: $_" -ForegroundColor Red
    exit 1
} 
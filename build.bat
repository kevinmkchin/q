@echo off
if not exist build mkdir build
echo Build started: %time%
if "%1"=="release" (
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --config Release
) else (
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --config Debug
)
echo Build finished: %time%
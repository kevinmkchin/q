# Untitled game


## Build on Windows
```
cmake -S <cmakelists.txt source directory> -B <output directory>
```
Then open VS project and build or
```
cmake --build <output directory> --config <Debug or Release>
```

## Build on Mac
```
cmake -S <cmakelists.txt source directory> -B <output directory> -G Xcode
sudo chown -R <macos username> <output directory>
sudo chmod -R 774 <output directory>
```
Then open Xcode project and run Game project.

### Troubleshooting
If `-- The CXX compiler identification is unknown` when trying to generate project:
```
sudo xcode-select --reset
```

# PathViz

Projeto base com:
- `pathcore`: biblioteca C++ (sem Qt)
- `pathviz`: app Qt6 Widgets

## Requisitos
- CMake >= 3.20
- Compilador com suporte a C++20 (ou C++17 se necessario)
- Qt6 Widgets (desenvolvimento instalado)

## Build (Linux/macOS)
```bash
cmake -S . -B build
cmake --build build
```

## Run
```bash
./build/pathviz
```

## Build (Windows - PowerShell)
```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run (Windows - PowerShell)
```powershell
.\build\Release\pathviz.exe
```

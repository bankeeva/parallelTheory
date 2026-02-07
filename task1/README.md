### Сборка с `double`

```bash
cmake -S . -B build -DUSE_DOUBLE=ON
cmake --build build
```

### Сборка с `float` (по умолчанию)

```bash
cmake -S . -B build -DUSE_DOUBLE=OFF
cmake --build build
```

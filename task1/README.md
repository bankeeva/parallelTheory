### Сборка с `double`

```bash
cmake -S . -B build -DUSE_DOUBLE=ON
cmake --build build
```
Результат: -5.25862e-11

### Сборка с `float` (по умолчанию)

```bash
cmake -S . -B build -DUSE_DOUBLE=OFF
cmake --build build
```
Результат: 0.039351

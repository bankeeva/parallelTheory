## Выбор типа `float` / `double` при сборке

- `USE_DOUBLE=1` → используется `double`
- `USE_DOUBLE=0` → используется `float` (по умолчанию)

### Сборка с `double`

```bash
cmake -S . -B build -DUSE_DOUBLE=ON
cmake --build build

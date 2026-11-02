# Неделя 9 — камера OpenGL

Источник: [задания и примеры 9-week](https://github.com/whvtnk/cg/tree/main/9-week),
включая `main1.cpp`, `main2.cpp`, `main3.cpp` и `results.md`.
Сцена соответствует примеру: пять вращающихся кубов, цветные грани,
перспектива 45°, depth test и отсечение задних граней.
Камера начинает в `(0, 0, 3)`, смотрит вдоль `(0, 0, -1)`,
`camUp = (0, 1, 0)`, обычная скорость — 2.5 единицы/с.

## Сборка

Из корня `cg-course`, где находятся общий `CMakeLists.txt` и `external/glad`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --target week09_task1 week09_task2 week09_task3 week09_camera_tests
```

Windows / Visual Studio:

```powershell
.\build\bin\Debug\week09_task1.exe
.\build\bin\Debug\week09_task2.exe
.\build\bin\Debug\week09_task3.exe
```

Linux / WSL (нужны CMake, C++17 и зависимости GLFW для X11):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DGLFW_BUILD_WAYLAND=OFF
cmake --build build --target week09_task1 week09_task2 week09_task3 week09_camera_tests -j4
./build/bin/week09_task1
./build/bin/week09_task2
./build/bin/week09_task3
```

При первой конфигурации общий проект скачивает GLFW 3.4 и GLM 1.0.1.
Используй отдельный каталог сборки для Windows и WSL.

## Управление

Во всех заданиях: `W/S` — вперёд/назад, `A/D` — влево/вправо,
`R` — сброс позиции, `V` — VSync, `Esc` — выход.
Заголовок показывает координаты, текущий режим и FPS.

| Программа | Дополнительные клавиши | Что проверяется |
|---|---|---|
| `week09_task1` | `O` — обычный взгляд / взгляд на начало координат | `lookAt(camPos, camPos + camFront, camUp)` и `lookAt(camPos, vec3(0), camUp)` |
| `week09_task2` | `Space` — вверх, `Left Shift` — вниз; `U` — мировая Y / camUp | Вертикальное движение; изначально выбрана мировая Y |
| `week09_task3` | Удерживать `Left Ctrl` — бег; `B` — порядок умножения | Скорость 5 единиц/с при беге; множитель 2 до или после dt |

Переключатели `O/U/B` заменяют необходимость менять `ORBIT_TEST`, `UP_MODE`
и `SPEED_ORDER` в исходниках и пересобирать программу.
В задании 3 вертикальное движение отсутствует, как и в `main3.cpp` источника.

## Как устроена работа

- `main.cpp` — общая сцена, шейдеры, ввод, цикл отрисовки и освобождение ресурсов.
- `camera.hpp` — движение с `speed * dt` и построение матрицы вида.
- `camera_tests.cpp` — проверки направлений, ускорения, разных FPS и вырожденного lookAt.
- `results.md` — результаты и объяснение экспериментов.

`uView` и `uProjection` вычисляются один раз за кадр, `uModel` — для каждого
куба. При изменении размера обновляются viewport и aspect ratio. При нулевом
размере фреймбуфера отрисовка приостанавливается. Отсчёт `dt` начинается после
инициализации, чтобы первый кадр не включал время создания окна и шейдеров.

## Проверки и воспроизводимые снимки

```sh
ctest --test-dir build -C Debug --output-on-failure
```

Для проверки отрисовки без ручного ввода есть `--capture`:
программа создаёт скрытое окно OpenGL, выполняет 60 кадров с `dt = 1/60`,
сохраняет настоящий фреймбуфер в PPM и завершает работу. Нужен графический
сеанс (например, WSLg); это не программная подмена изображения.

```sh
./build/bin/week09_task1 --capture initial.ppm
./build/bin/week09_task1 --capture orbit.ppm --demo left --orbit
./build/bin/week09_task2 --capture up.ppm --demo up
./build/bin/week09_task3 --capture sprint.ppm --demo sprint
```

`--demo forward|left|up|sprint` подаёт движение в тот же метод камеры,
который используется клавиатурой; `up` доступен только в задании 2,
`sprint` — в задании 3. При обычном запуске движение использует реальный `dt`.

# cg-course — лабораторные по компьютерной графике

C++17 · OpenGL 3.3 Core · GLFW 3.4 · GLAD · GLM 1.0.1 · CMake

## Структура

```
cg-course/
├── CMakeLists.txt        общий CMake: зависимости + подключение недель
├── external/glad/        загрузчик функций OpenGL 3.3 Core (сгенерирован glad2)
├── week01/               окно, цикл отрисовки, ввод, FPS
└── README.md
```

GLFW и GLM скачиваются автоматически при первой конфигурации CMake
(`FetchContent`), поэтому первый запуск `cmake` требует интернет.

## Сборка и запуск

### Windows — Visual Studio 2022

Нужен Visual Studio 2022 с компонентом **Desktop development with C++**
(в него входит **C++ CMake tools for Windows**).

Вариант 1 — из IDE: **File → Open → Folder** → папка `cg-course`,
вверху выбрать цель (`week01.exe` и т.д.) и нажать **F5**.

Вариант 2 — из терминала **x64 Native Tools Command Prompt for VS 2022**:

```
cmake -B build
cmake --build build --config Debug
build\bin\Debug\week01.exe
```

### Linux / macOS

```bash
# Linux: sudo apt install build-essential cmake libx11-dev libxrandr-dev \
#        libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/bin/week01
```

Собрать только одну неделю: `cmake --build build --target week01`.

Если после правки `CMakeLists.txt` что-то странно ломается — удалите папку
`build` и сконфигурируйте заново.

---

## Неделя 1 — окно и главный цикл

**Задача.** Настроить окружение (CMake + GLFW + GLAD + GLM), открыть окно
1280×720 с контекстом OpenGL 3.3 Core, вывести в консоль версию OpenGL и
название видеокарты, сделать анимированный цвет фона, переключение фона
пробелом, выход по Esc и счётчик FPS.

**Что сделано** (`week01/main.cpp`):

- окно создаётся с запросом OpenGL 3.3 Core, функции OpenGL загружаются через GLAD;
- в консоль выводятся vendor, renderer, версия OpenGL и GLSL;
- фон плавно переливается: три синусоиды со сдвигом фазы 120° для R, G, B;
- клавиши обрабатываются через `glfwSetKeyCallback` — колбэк срабатывает один
  раз на нажатие, поэтому переключатели не «дребезжат»;
- FPS считается раз в секунду и выводится в консоль и в заголовок окна;
- при изменении размера окна обновляется `glViewport`.

| Клавиша | Действие |
|---|---|
| Space | анимированный фон ⇄ белый фон |
| V | включить/выключить VSync (видно, как меняется FPS) |
| Esc | выход |

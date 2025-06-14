# SalixGameStudio

*An experimental C++17 game engine featuring AI-assisted development tooling and Python scripting.*

---

## About The Project

SalixGameStudio is an ambitious project that integrates several prototypes into a cohesive game engine. The core is written in modern C++ for high performance, while the architecture is designed to support and be enhanced by AI agents, streamlining the creative process. Game logic can be scripted using Python, providing a flexible and powerful workflow.

This repository tracks the development from its initial commit on **June 14, 2025**.

### Key Features

* **AI-Assisted Development:** Core tooling designed to integrate with AI agents to aid in content creation and logic.
* **Modern C++ Core:** Built with C++17 for performance, safety, and access to modern language features.
* **Python Scripting:** A flexible and powerful game logic scripting layer supporting Python 3.13 and higher.
* **Cross-Platform Media Layer:** Uses the robust and battle-tested SDL2 library as its default windowing and input backend.

### Tech Stack

* **Language:** C++17
* **Media Layer:** SDL2 (v2.28.5)
* **Scripting:** Python 3.13+
* **Build System (Windows):** MSVC (x64) via `build.bat` script

---

## Getting Started

Follow these instructions to get a local copy up and running.

### Prerequisites

You must have **Visual Studio 2022** installed with the **"Desktop development with C++"** workload. This provides the required 64-bit MSVC compiler (`cl.exe`) and toolchain.

### Building the Project

1.  **Clone the repository:**
    ```sh
    git clone [https://github.com/ebdsaleh/SalixGameStudio.git](https://github.com/ebdsaleh/SalixGameStudio.git)
    ```
2.  **Open the Correct Terminal:**
    From the Windows Start Menu, launch the **"x64 Native Tools Command Prompt for VS 2022"**. This is crucial for setting up the 64-bit build environment.

3.  **Navigate to the Project Directory:**
    ```
    cd SalixGameStudio
    ```
4.  **Run the Build Script:**
    ```
    build.bat
    ```
5.  **Run the Executable:**
    If the build is successful, the executable will be located in the `build` folder.
    ```
    .\build\salix.exe
    ```

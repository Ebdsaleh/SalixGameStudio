# SalixGameStudio

*An experimental C++17 game engine featuring a custom reflection system, AI-assisted development tooling, and Python scripting.*

---

## About The Project

SalixGameStudio is an ambitious project that integrates several prototypes into a cohesive game engine. The core is written in modern C++ for high performance, while the architecture is designed to support and be enhanced by AI agents, streamlining the creative process. Game logic can be scripted using Python, providing a flexible and powerful workflow.

This repository tracks the development from its initial commit on **June 14, 2025**.

### Key Features

* **Custom Reflection System:** A custom-built, compile-time reflection system (`ByteMirror`) that automatically generates editor UI for C++ components, drives scene serialization, and enables deep editor integration.
* **Robust Editor Framework:** Built on a hardened implementation of Dear ImGui and ImGuizmo, featuring a custom panel architecture (`IPanel`, `LockablePanel`) that supports complex, multi-purpose editor windows.
* **AI-Assisted Development:** Core tooling designed to integrate with AI agents to aid in content creation and logic.
* **Modern C++ Core:** Built with C++17 for performance, safety, and access to modern language features.
* **Python Scripting:** A flexible and powerful game logic scripting layer supporting Python 3.13 and higher.
* **Cross-Platform Media Layer:** Uses the robust and battle-tested SDL2 library as its default windowing and input backend.

### Tech Stack

* **Language:** C++17
* **Media Layer:** SDL2 (v2.28.5)
* **GUI:** Dear ImGui (+ ImGuizmo)
* **Scripting:** Python 3.13+
* **Build System (Windows):** MSVC (x64) via batch scripts

---

## Getting Started

Follow these instructions to get a local copy up and running on Windows.

### Prerequisites

You must have **Visual Studio 2022** installed with the **"Desktop development with C++"** workload. This provides the required 64-bit MSVC compiler (`cl.exe`) and toolchain.

### Building and Running the Project

1.  **Clone the repository:**
    ```sh
    git clone [https://github.com/Ebdsaleh/SalixGameStudio.git](https://github.com/Ebdsaleh/SalixGameStudio.git)
    ```
2.  **Open the Correct Terminal:**
    From the Windows Start Menu, launch the **"x64 Native Tools Command Prompt for VS 2022"**. This is crucial for setting up the 64-bit build environment.

3.  **Navigate to the Project Directory:**
    ```batch
    cd SalixGameStudio
    ```
4.  **Build the Engine:**
    You can build in either Debug or Release mode. The scripts will automatically clean the build directory.

    *To build for **Debug**:*
    ```batch
    clean_build_debug.bat
    ```
    *To build for **Release**:*
    ```batch
    clean_build_release.bat
    ```

5.  **Run the Engine:**
    Convenience scripts are provided to run the correct executable.

    *To run the **Debug** build:*
    ```batch
    run_debug.bat
    ```
    *To run the **Release** build:*
    ```batch
    run_release.bat
    ```
    Alternatively, you can run the executable directly from the `build` folder:
    * `.\build\Debug\SalixGameStudio.exe`
    * `.\build\Release\SalixGameStudio.exe`

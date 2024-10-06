<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

This project is related to the teaching unit **Modélisation Géométrique** in second year of computer science master degree ID3D at the University of Claude Bernard Lyon 1 - Villeurbanne.    

### Mesh surfaces generation
The first goal was to generate mesh surfaces from a set of point or curve control. 

### Surfaces of revolution and extrusion

The second goal was to generate extrusion surfaces from a generating curve.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

The project is in C++. The libraries used for project are listed below : 

* [GKit](https://perso.univ-lyon1.fr/jean-claude.iehl/Public/educ/M1IMAGE/html/index.html) : Used for visualisation. 
* [ImGUI](https://github.com/ocornut/imgui) : Used to managed the UI of the application.
* [exprtk - C++ Mathematical Expression Toolkit Library](https://github.com/ArashPartow/exprtk) : Used to parse string that represent mathematical expression.

They are located in the *vendor* directory. 


  - Project Structure 
    ```
    ├── data                  
    |   └── shaders             # Shaders utilisés pour le rendu avec GKit.
    ├── src                   # Code 
    |   ├── Include             # Fichiers .h.  
    |   |   └── utils.h           # Utilitaires
    |   ├── Source              # Fichiers .cpp.  
    |   └── main.cpp              
    ├── vendor 
    |   ├── exprtk # Exprtk lib
    |   ├── gkit   # Gkit lib
    |   └── imgui  # ImGUI lib
    ├── .gitignore            # Fichier .gitignore.
    ├── CMakeLists.txt        # Configuration Cmake.
    ├── imgui.ini             # Configuration ImGui.
    └── README.md             # Fichier README du projet.
    ```
    
<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

GKit is using SDL2 and Glew. 
* sdl & glew 
  ```sh
  sudo apt install libsdl2-dev libsdl2-image-dev libglew-dev
  ```

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/tgrillon/modgeo.git
   ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Compilation 

1. Building the project using cmake : 
    ```sh
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build/ -t modgeo -j 12
    ```
2. Launch the application :
    ```sh
    ./build/modgeo 
    ```

<!-- USAGE EXAMPLES -->
## Usage

Use this space to show useful examples of how a project can be used. Additional screenshots, code examples and demos work well in this space. You may also link to more resources.

_For more examples, please refer to the [Documentation](https://example.com)_

<p align="right">(<a href="#readme-top">back to top</a>)</p>

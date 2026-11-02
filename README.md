# Dijkstra's Algorithm Visualizer (C++ & SFML)

An interactive, graphical visualization of Dijkstra's Shortest Path algorithm built with C++ and the SFML (Simple and Fast Multimedia Library). 

This tool allows users to dynamically create a weighted graph on the screen by placing nodes and connecting them with edges. The algorithm then visually processes the graph in real-time, showing the step-by-step relaxation of edges and finding the shortest path from the starting node to the target.

## ✨ Features
* **Interactive Graph Creation:** Add nodes freely on the canvas using mouse clicks.
* **Dynamic Edge Binding:** Connect any two nodes to create undirected, weighted edges. The weight is automatically calculated based on the Euclidean distance (pixels) between the nodes.
* **Real-time Visualization:** Watch the algorithm work step-by-step with color-coded states:
  * ⚪ **White:** Unvisited node
  * 🟡 **Yellow:** Currently processing node
  * 🟢 **Green:** Fully processed node (shortest distance found)
  * 🔴 **Red:** The final shortest path backtracking
* **Live Distance Tracking:** Displays the current minimum distance above each node as the algorithm explores the graph.

## 🛠️ Technologies Used
* **Language:** C++17
* **Graphics Library:** [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library)
* **Data Structures:** Adjacency list representation (`std::vector`), Priority Queue (`std::priority_queue`) for greedy node selection.

## 🎮 Controls
* **Left Mouse Click:** Place a new node on the canvas.
* **Right Mouse Click (on Node A, then Node B):** Create a weighted edge between two existing nodes.
* **ENTER:** Start the Dijkstra visualization (calculates the path from Node `0` to the last created node).
* **R:** Reset and clear the entire canvas.

## 🚀 How to Run
1. Make sure you have the SFML library configured in your C++ IDE (e.g., Code::Blocks, Visual Studio, or via CMake).
2. Clone this repository.
3. Ensure the `assets/arial.ttf` font file is present in the specified directory.
4. Compile and run `main.cpp`.
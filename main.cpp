/* 
A graph is an abstract mathematical structure that models relationships between objects. 
It consists of two main elements:
- Vertices (also called nodes or points).
- Edges (lines) that connect pairs of these vertices.
*/

/* 
This is an algorithm for finding the shortest path from a starting node to all others 
(or to a specific target) in a weighted graph.
Principle: Uses a "greedy approach" — at each step, it selects the closest unvisited 
node and updates the distances to its neighbors.
Condition: Works correctly only if the edge weights are non-negative (the numbers 
on the connections must be >= 0).
*/

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <limits>
#include <queue>
#include <thread> // For pauses (animation)
#include <string>
#include <iostream>

// --- CONFIGURATION ---
const float NODE_RADIUS = 20.0f; // Radius of the nodes in pixels
const int ANIMATION_DELAY_MS = 2000; // Animation speed
const float INF = std::numeric_limits<float>::max(); // Infinity - initially we only know the distance to the start node, all others are considered unreachable (infinity)

// --- COLORS ---
const sf::Color COLOR_DEFAULT = sf::Color::White;       // Unvisited node
const sf::Color COLOR_PROCESSING = sf::Color::Yellow;   // The node Dijkstra is currently processing
const sf::Color COLOR_VISITED = sf::Color(50, 200, 50); // Processed node (Green) - the shortest path to it has been definitively found
const sf::Color COLOR_PATH = sf::Color::Red;            // Final path - the nodes traversed to form the shortest path

// --- STRUCTURES ---

// Edges
struct Edge {
    int targetNodeIndex; // The index of the neighboring node this edge leads to
    float weight; // The length of the edge (distance in pixels)
};

struct Node {
    sf::CircleShape shape; // The circle representing the node graphically
    std::vector<Edge> neighbors; // List of all edges starting from this node
    
    // Dijkstra specific data
    float minDistance; // The shortest distance from the start to this node found so far
    int previousNodeIndex; // For path reconstruction - the neighbor we came from to get this minimum distance
    bool isVisited; // Whether the node has been fully processed

    sf::Text textInfo; // Drawable text object for the node ID

    // Prepares the node for a fresh algorithm execution
    void resetDijkstra() {
        minDistance = INF;
        previousNodeIndex = -1;
        isVisited = false;
        shape.setFillColor(COLOR_DEFAULT);
    }

    // Constructor for visualizing nodes on the screen
    Node(float x, float y, int id, sf::Font& font) {
        shape.setRadius(NODE_RADIUS);
        shape.setOrigin(NODE_RADIUS, NODE_RADIUS); // Center the origin point
        shape.setPosition(x, y);
        shape.setFillColor(COLOR_DEFAULT);
        shape.setOutlineThickness(3);
        shape.setOutlineColor(sf::Color::Black);

        // Text setup
        textInfo.setFont(font);
        textInfo.setString(std::to_string(id)); // Convert the ID to text for rendering
        textInfo.setCharacterSize(18);
        textInfo.setFillColor(sf::Color::Black);
        
        // Center the text inside the node
        sf::FloatRect textRect = textInfo.getLocalBounds();
        textInfo.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        textInfo.setPosition(x, y);

        resetDijkstra();
    }
};

// Global variables
std::vector<Node> graph; // Dynamic array holding all created nodes
sf::Font globalFont; // Font used for node numbers

// --- HELPER FUNCTIONS ---

// Calculates the distance between two nodes (Pythagorean theorem)
float getDistance(const Node& a, const Node& b) {
    float dx = a.shape.getPosition().x - b.shape.getPosition().x;
    float dy = a.shape.getPosition().y - b.shape.getPosition().y;
    return std::sqrt(dx * dx + dy * dy); // The distance is the hypotenuse
}

// Checks if we clicked inside an existing node
// x, y are the mouse coordinates
int getClickedNodeIndex(float x, float y) {
    for (size_t i = 0; i < graph.size(); i++) {
        float dx = graph[i].shape.getPosition().x - x;
        float dy = graph[i].shape.getPosition().y - y;
        
        // If the distance from the mouse cursor to the node center is <= its radius
        if (std::sqrt(dx * dx + dy * dy) <= NODE_RADIUS) { 
            return i; // Return the index of the clicked node
        }
    }
    return -1;
}

// RENDERING
void renderGraph(sf::RenderWindow& window) {
    window.clear(sf::Color(50, 50, 50)); // Dark gray background

    // 1. Draw the edges first (so they appear behind the nodes in the Z-buffer)
    for (size_t i = 0; i < graph.size(); i++) {
        for (const auto& edge : graph[i].neighbors) {
            // Draw a line from the current node to its neighbor
            sf::Vertex line[] = {
                sf::Vertex(graph[i].shape.getPosition(), sf::Color::White),
                sf::Vertex(graph[edge.targetNodeIndex].shape.getPosition(), sf::Color::White)
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    // 2. Draw the nodes and their text
    for (const auto& node : graph) {
        window.draw(node.shape);
        window.draw(node.textInfo);

        // Display the current shortest distance above the node (if it's not infinity)
        // This shows Dijkstra's relaxation process in real-time
        if (node.minDistance != INF) {
            sf::Text distText;
            distText.setFont(globalFont);
            distText.setString(std::to_string((int)node.minDistance));
            distText.setCharacterSize(14);
            distText.setFillColor(sf::Color::Cyan);
            distText.setPosition(node.shape.getPosition().x - 10, node.shape.getPosition().y - 40);
            window.draw(distText);
        }
    }
    
    // Push the buffer to the screen
    window.display();
}

// --- DIJKSTRA ALGORITHM WITH VISUALIZATION ---
void runDijkstraVisual(sf::RenderWindow& window, int startIndex, int endIndex) {
    // Reset state for all nodes
    for (auto& node : graph) node.resetDijkstra();

    // The start node has a distance of 0, all others are infinite
    graph[startIndex].minDistance = 0;

    // Priority Queue: always keeps the smallest distance at the top
    // 'greater' setup ensures it acts as a Min-Heap based on distance
    // pair <distance, node index>
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> pq;
    
    // Push the start node into the queue
    pq.push({0, startIndex});

    while (!pq.empty()) {
        // Get the node with the smallest distance currently in the queue
        int u = pq.top().second;
        pq.pop(); // Remove it from the queue

        // If we have already fully processed this node, skip it
        if (graph[u].isVisited) continue;

        // Mark the current node as "processing"
        graph[u].isVisited = true;
        graph[u].shape.setFillColor(COLOR_PROCESSING);

        // Update the screen and wait to animate the step
        renderGraph(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(ANIMATION_DELAY_MS));

        // If we reached the target node, we can stop early
        if (u == endIndex) break;

        // Iterate through all neighbors of the current node
        for (const auto& edge : graph[u].neighbors) {
            int v = edge.targetNodeIndex; // Neighbor index
            float weight = edge.weight; // Edge length

            // Relaxation - can we reach neighbor 'v' faster by going through 'u'?
            // Check if current shortest distance + edge weight is less than the known path to the neighbor
            if (graph[u].minDistance + weight < graph[v].minDistance) { 
                graph[v].minDistance = graph[u].minDistance + weight; // Update new shortest distance
                graph[v].previousNodeIndex = u; // Record the path taken
                pq.push({graph[v].minDistance, v}); // Push the updated neighbor into the queue
            }
        }

        // The node is fully processed
        graph[u].shape.setFillColor(COLOR_VISITED);
        renderGraph(window);
    }

    // --- PATH RECONSTRUCTION ---
    if (graph[endIndex].minDistance != INF) { // Check if the target was actually reachable
        int curr = endIndex; // Backtrack from finish to start
        while (curr != -1) { 
            graph[curr].shape.setFillColor(COLOR_PATH); // Highlight the final path
            renderGraph(window);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            curr = graph[curr].previousNodeIndex; // Move to the predecessor node
        }
    }
}

int main() {
    // Font setup
    if (!globalFont.loadFromFile("assets/arial.ttf")) {
        std::cerr << "ERROR: Font not found! Please ensure arial.ttf is in the 'assets' directory." << std::endl;
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode(1000, 800), "Dijkstra Visualization");

    int selectedNodeIndex = -1; // Tracks the first selected node for creating edges

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // MOUSE CLICKS
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                if (event.mouseButton.button == sf::Mouse::Left) {
                    // Left click: Add a new node
                    int clickedIdx = getClickedNodeIndex(mousePos.x, mousePos.y);
                    if (clickedIdx == -1) { // Only add if not clicking on an existing node
                        graph.emplace_back(mousePos.x, mousePos.y, graph.size(), globalFont);
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    // Right click: Connect two nodes
                    int clickedIdx = getClickedNodeIndex(mousePos.x, mousePos.y);

                    if (clickedIdx != -1) {
                        if (selectedNodeIndex == -1) {
                            // First node selected (highlight it slightly to indicate selection)
                            selectedNodeIndex = clickedIdx;
                            graph[selectedNodeIndex].shape.setOutlineColor(sf::Color::Magenta);
                        } else {
                            // Second node selected, create connection
                            if (selectedNodeIndex != clickedIdx) {
                                float weight = getDistance(graph[selectedNodeIndex], graph[clickedIdx]);

                                // Bidirectional connection (undirected graph)
                                graph[selectedNodeIndex].neighbors.push_back({clickedIdx, weight});
                                graph[clickedIdx].neighbors.push_back({selectedNodeIndex, weight});

                                // Restore the outline color of the first node
                                graph[selectedNodeIndex].shape.setOutlineColor(sf::Color::Black);
                                selectedNodeIndex = -1;
                            }
                        }
                    } else {
                        // Clicked on empty space -> cancel edge creation
                        if(selectedNodeIndex != -1) {
                            graph[selectedNodeIndex].shape.setOutlineColor(sf::Color::Black);
                            selectedNodeIndex = -1;
                        }
                    }
                }
            }

            // KEYBOARD INPUT
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    // Start the algorithm (from node 0 to the last created node)
                    if (graph.size() >= 2) {
                        runDijkstraVisual(window, 0, graph.size() - 1);
                    }
                }
                if (event.key.code == sf::Keyboard::R) {
                    // Reset the graph entirely
                    graph.clear();
                    selectedNodeIndex = -1;
                }
            }
        }

        renderGraph(window);
    }

    return 0;
}
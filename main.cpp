/*Графът е абстрактна математическа структура, която моделира връзки между обекти. Той се състои от два основни елемента:
Върхове (наричани още възли или точки).
Ребра (линии), които свързват двойки от тези върхове.*/
/*Това е алгоритъм за намиране на най-краткия път от един начален връх до всички останали (или до конкретна цел) в претеглен граф.
Принцип: Използва „алчен подход“ — на всяка стъпка избира най-близкия непосетен връх и обновява разстоянията до съседите му.
Условие: Работи коректно само ако теглата на ребрата са неотрицателни (числата по връзките трябва да са ≥0).
*/
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <limits>
#include <queue>
#include <thread> // За паузите (анимацията)
#include <string>
#include <iostream>

// --- КОНФИГУРАЦИЯ ---
const float NODE_RADIUS = 20.0f;// радиуса на възлите в пиксели
const int ANIMATION_DELAY_MS = 2000; // Скорост на анимацията
const float INF = std::numeric_limits<float>::max();// безкрайност -  в началото на алгоритъма знаем само дистанцията до началния възел - докато не намерим път до останалите приемаме че е безкрайност

// --- ЦВЕТОВЕ ---
const sf::Color COLOR_DEFAULT = sf::Color::White;       // Непосетен възел
const sf::Color COLOR_PROCESSING = sf::Color::Yellow;   // Възелът, който Дейкстра обработва в момента
const sf::Color COLOR_VISITED = sf::Color(50, 200, 50); // Обработен възел(Зелен) - алгоритъмът е приключил с този възел - намерил е най краткият възможен път до него
const sf::Color COLOR_PATH = sf::Color::Red;            // Финален път - всички възли, през които преминаваме, за да стигнем до най-краткия възможен път

// --- СТРУКТУРИ ---
//ребрата
struct Edge {
    int targetNodeIndex; // номерът на съседния възел, до който води връзката
    float weight; // колко е дълго реброто(разстояние в пиксели)
};

struct Node {
    sf::CircleShape shape; // кръгчетата, с които инициализираме възлите
    std::vector<Edge> neighbors; // списък с всички пътища, които тръгват от този възел;
    // Данни за Дейкстра
    float minDistance;//най-късото разстояние от старта до този възел, което сме открили до момента
    int previousNodeIndex; // За възстановяване на пътя - от кой съсед сме дошли, за да намерим най краткото разстояние
    bool isVisited;// дали възелът е обработен

    sf::Text textInfo;// картинка на текст,рисува номера на възела
    //функция, която подготвя възела за изпълнение на алгоритъма(бял,непосетен)
    void resetDijkstra() {
        minDistance = INF;
        previousNodeIndex = -1;
        isVisited = false;
        shape.setFillColor(COLOR_DEFAULT);
    }
//конструктор за визуализация на възлите на екрана
    Node(float x, float y, int id, sf::Font& font) {
        shape.setRadius(NODE_RADIUS);
        shape.setOrigin(NODE_RADIUS, NODE_RADIUS); // Центърът да е в средата
        shape.setPosition(x, y);
        shape.setFillColor(COLOR_DEFAULT);
        shape.setOutlineThickness(3);
        shape.setOutlineColor(sf::Color::Black);

        // Настройка на текста
        textInfo.setFont(font);
        textInfo.setString(std::to_string(id));// превръща числото в текст, за да може да се нарисува на екрана
        textInfo.setCharacterSize(18);
        textInfo.setFillColor(sf::Color::Black);
        // Центриране на текста във възела
        sf::FloatRect textRect = textInfo.getLocalBounds();
        textInfo.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        textInfo.setPosition(x, y);

        resetDijkstra();
    }

};

// Глобални променливи
std::vector<Node> graph; // динамичен масив, в който пазим всички създадени възли
sf::Font globalFont; // шрифт - за писане на номерата

// --- ПОМОЩНИ ФУНКЦИИ ---

// Изчислява разстояние между два възела (Питагорова теорема)
float getDistance(const Node& a, const Node& b) {
    float dx = a.shape.getPosition().x - b.shape.getPosition().x; // единият катет
    float dy = a.shape.getPosition().y - b.shape.getPosition().y; // другият катед
    return std::sqrt(dx * dx + dy * dy); // разстоянието е хипотенузата
}

// Проверява дали сме кликнали върху съществуващ възел
// х, у координатите на върха на мишката
int getClickedNodeIndex(float x, float y) {
    for (size_t i = 0; i < graph.size(); i++) {
        float dx = graph[i].shape.getPosition().x - x;
        float dy = graph[i].shape.getPosition().y - y;
        if (std::sqrt(dx * dx + dy * dy) <= NODE_RADIUS) { // ако дистанцията от върха на мишката до центъра на възела е по - малка или равна на радиуса му
            return i; // връщаме номера на уцеления възел
        }
    }
    return -1;
}

// РИСУВАНЕ НА ЕКРАНА
void renderGraph(sf::RenderWindow& window) {
    window.clear(sf::Color(50, 50, 50)); // Тъмно сив фон

    // 1. Рисуване на ребрата - преди възлите, защото в компютърната графика това, което нарисуваш последно, застава най-отгоре
    for (size_t i = 0; i < graph.size(); i++) {
        for (const auto& edge : graph[i].neighbors) {
            // Рисуваме линия от текущия възел до съседа
            sf::Vertex line[] = {
                sf::Vertex(graph[i].shape.getPosition(), sf::Color::White),
                sf::Vertex(graph[edge.targetNodeIndex].shape.getPosition(), sf::Color::White)
            };
            window.draw(line, 2, sf::Lines);
        }
    }

    // 2. Рисуване на възлите и текста
    for (const auto& node : graph) {
        window.draw(node.shape);
        window.draw(node.textInfo);

        // Показване на текущото най-късо разстояние над възела (ако не е безкрайност)
        // Показва как работи Дейкстра в реално време
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
// window.draw работи в буфера, това го визуализира на екрана
    window.display();
}

// --- АЛГОРИТЪМ НА ДЕЙКСТРА С ВИЗУАЛИЗАЦИЯ ---
void runDijkstraVisual(sf::RenderWindow& window, int startIndex/*0*/, int endIndex/*graph.size() - 1 */) {
    //Нулиране на състоянието
    for (auto& node : graph) node.resetDijkstra();

    // началния възел ни е с разстояние 0, а всички останали са с безкрайност
    graph[startIndex].minDistance = 0;

    // Priority Queue: винаги държи най- малкото число най-отгоре в този специалне списък
    // настройката'greater' - дава ни възела с най- късо разстояние до момента
    //pair < разтояние, индекс на възела>, в опашката първо се подреждат по дължина, после по индекс
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> pq;
// добавяме началния възел най отпред в опашката
    pq.push({0, startIndex});

    while (!pq.empty()) {
        // Показва ни възела най-отгоре(greater ги е подредило), взимаме индекса му
        int u = pq.top().second;
        pq.pop(); // премахваме най - близкия възел, вече сме взели индекса му

        // Ако вече сме го посетили, пропускаме
        if (graph[u].isVisited) continue;

        // Маркираме текущия възел като "обработващ се"
        graph[u].isVisited = true;
        graph[u].shape.setFillColor(COLOR_PROCESSING);

        // Обновяваме екрана и чакаме
        renderGraph(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(ANIMATION_DELAY_MS)); // време за стигане до следваща стъпка

        // Ако сме стигнали финала, спираме търсенето
        if (u == endIndex) break;

        // Обхождаме съседните възли на текущия
        for (const auto& edge : graph[u].neighbors) {
            int v = edge.targetNodeIndex; // индексът на съсед
            float weight = edge.weight; // дължината на реброто

            // Релаксация - мога ли да стигна до съседната точка по бързо ако мина през текущата
            if (graph[u].minDistance + weight < graph[v].minDistance) { //! проверка дали сегашното най- късо разстояние + дължината на реброто до съседа е по - малко от сегашния път до съседа
                graph[v].minDistance = graph[u].minDistance + weight; // новото най-късо разстояние
                graph[v].previousNodeIndex = u; // пътят, по който ще стигнем най-бързо
                pq.push({graph[v].minDistance, v});// добавяме съседа в опашката
            }
        }

        //Възелът е напълно обработен
        graph[u].shape.setFillColor(COLOR_VISITED);
        renderGraph(window);
    }

    // --- ВЪЗСТАНОВЯВАНЕ НА ПЪТЯ ---
    if (graph[endIndex].minDistance != INF) { // проверяваме дали сме стигнали до финала
        int curr = endIndex; // вървим от финала към старта
        while (curr != -1) { // докато не стигнем старта вървим назад
            graph[curr].shape.setFillColor(COLOR_PATH); // Оцветяваме пътя
            renderGraph(window);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            curr = graph[curr].previousNodeIndex; // предишният възел, от където сме дошли, за да стигнем по най бързия начин тук
        }
    }
}

int main() {
    // Проверка за шрифта
    if (!globalFont.loadFromFile("assets/arial.ttf")) {
        std::cerr << "ERROR: Font not found! Please ensure arial.ttf is in the project directory." << std::endl;
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode(1000, 800), "Dijkstra Visualization");

    int selectedNodeIndex = -1; // За създаване на връзки

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // КЛИК С МИШКА
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                if (event.mouseButton.button == sf::Mouse::Left) {
                    // Ляв бутон: Добавяне на нов възел
                    int clickedIdx = getClickedNodeIndex(mousePos.x, mousePos.y);
                    if (clickedIdx == -1) { // Ако не кликаме върху съществуващ
                        graph.emplace_back(mousePos.x, mousePos.y, graph.size(), globalFont);
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    // Десен бутон: Свързване на два възела
                    int clickedIdx = getClickedNodeIndex(mousePos.x, mousePos.y);

                    if (clickedIdx != -1) {
                        if (selectedNodeIndex == -1) {
                            // Избираме първи възел (оцветяваме го леко за индикация)
                            selectedNodeIndex = clickedIdx;
                            graph[selectedNodeIndex].shape.setOutlineColor(sf::Color::Magenta);
                        } else {
                            // Избираме втори възел и правим връзка
                            if (selectedNodeIndex != clickedIdx) {
                                float weight = getDistance(graph[selectedNodeIndex], graph[clickedIdx]);

                                // Двупосочна връзка (неориентиран граф)
                                graph[selectedNodeIndex].neighbors.push_back({clickedIdx, weight});
                                graph[clickedIdx].neighbors.push_back({selectedNodeIndex, weight});

                                // Връщаме цвета на първия
                                graph[selectedNodeIndex].shape.setOutlineColor(sf::Color::Black);
                                selectedNodeIndex = -1;
                            }
                        }
                    } else {
                        // Клик на празно -> отказ
                        if(selectedNodeIndex != -1) {
                            graph[selectedNodeIndex].shape.setOutlineColor(sf::Color::Black);
                            selectedNodeIndex = -1;
                        }
                    }
                }
            }

            // БУТОН ОТ КЛАВИАТУРАТА
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    // Стартиране на алгоритъма (от възел 0 до последния добавен)
                    if (graph.size() >= 2) {
                        runDijkstraVisual(window, 0, graph.size() - 1);
                    }
                }
                if (event.key.code == sf::Keyboard::R) {
                    // Рестарт на графа
                    graph.clear();
                    selectedNodeIndex = -1;
                }
            }
        }

        renderGraph(window);
    }

    return 0;
}

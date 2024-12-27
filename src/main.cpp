#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

// dimensions
const int mapWidth = 8;
const int mapHeight = 8;

// data
int worldMap[mapWidth][mapHeight] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}};

// player properties
float playerX = 2.0f, playerY = 2.0f; 
float playerAngle = 0.0f;             
const float fov = 3.14159f / 4.0f;  
const float moveSpeed = 0.05f;
const float rotationSpeed = 0.007f;
const float maxDepth = 16.0f;

// screen dimensions
const int screenWidth = 800;
const int screenHeight = 400;

void renderBirdsEyeView(sf::RenderWindow &window, float playerX, float playerY, float playerAngle, const std::vector<float> &distances)
{
    int cellSize = 50;
    window.clear(sf::Color::White);

    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
            cell.setPosition(x * cellSize, y * cellSize);
            cell.setFillColor(worldMap[x][y] == 1 ? sf::Color::Black : sf::Color::White);
            cell.setOutlineColor(sf::Color::Blue);
            cell.setOutlineThickness(1);
            window.draw(cell);
        }
    }

    // draw player
    sf::CircleShape player(5);
    player.setFillColor(sf::Color::Red);
    player.setPosition(playerX * cellSize, playerY * cellSize);
    window.draw(player);

    // draw rays
    for (size_t i = 0; i < distances.size(); ++i)
    {
        float rayAngle = (playerAngle - fov / 2.0f) + (i / (float)distances.size()) * fov;
        float rayEndX = playerX + cos(rayAngle) * distances[i];
        float rayEndY = playerY + sin(rayAngle) * distances[i];

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(playerX * cellSize + 5, playerY * cellSize + 5), sf::Color::Yellow),
            sf::Vertex(sf::Vector2f(rayEndX * cellSize, rayEndY * cellSize), sf::Color::Yellow)};
        window.draw(line, 2, sf::Lines);
    }

    window.display();
}

void renderPlayerView(sf::RenderWindow &window, const std::vector<float> &distances, const sf::Texture &floorTexture)
{
    window.clear(sf::Color::Black);

    sf::Image floorImage = floorTexture.copyToImage();
    const unsigned int texWidth = floorTexture.getSize().x;
    const unsigned int texHeight = floorTexture.getSize().y;

    for (int x = 0; x < screenWidth; x++)
    {
        float distance = distances[x];
        int wallHeight = (int)(screenHeight / distance);


        sf::RectangleShape wallSlice(sf::Vector2f(1, wallHeight));
        wallSlice.setPosition(x, screenHeight / 2 - wallHeight / 2);
        int shade = 255 - std::min(255, (int)(distance * 20));
        wallSlice.setFillColor(sf::Color(shade, shade, shade));

        window.draw(wallSlice);

        // draw floor
        for (int y = screenHeight / 2 + wallHeight / 2; y < screenHeight; y++)
        {
            float floorDistance = screenHeight / (2.0f * y - screenHeight);
            float floorX = playerX + cos(playerAngle + (x / (float)screenWidth - 0.5f) * fov) * floorDistance;
            float floorY = playerY + sin(playerAngle + (x / (float)screenWidth - 0.5f) * fov) * floorDistance;

            int texX = (int)(floorX * texWidth) % texWidth;
            int texY = (int)(floorY * texHeight) % texHeight;

            sf::Color floorColor = floorImage.getPixel(texX, texY);
            sf::Vertex pixel(sf::Vector2f(x, y), floorColor);
            window.draw(&pixel, 1, sf::Points);
        }
    }

    window.display();
}

std::vector<float> castRays(float playerX, float playerY, float playerAngle)
{
    std::vector<float> distances(screenWidth);

    for (int x = 0; x < screenWidth; x++)
    {
        float rayAngle = (playerAngle - fov / 2.0f) + (x / (float)screenWidth) * fov;

        float rayX = cos(rayAngle);
        float rayY = sin(rayAngle);

        float distanceToWall = 0.0f;
        bool hitWall = false;

        while (!hitWall && distanceToWall < maxDepth)
        {
            distanceToWall += 0.1f;
            int testX = (int)(playerX + rayX * distanceToWall);
            int testY = (int)(playerY + rayY * distanceToWall);

            if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight)
            {
                hitWall = true;
                distanceToWall = maxDepth;
            }
            else if (worldMap[testX][testY] == 1)
            {
                hitWall = true;
            }
        }

        distances[x] = distanceToWall;
    }

    return distances;
}

void handleInput(sf::RenderWindow &window)
{
    float newPlayerX = playerX;
    float newPlayerY = playerY;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        newPlayerX += cos(playerAngle) * moveSpeed;
        newPlayerY += sin(playerAngle) * moveSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        newPlayerX -= cos(playerAngle) * moveSpeed;
        newPlayerY -= sin(playerAngle) * moveSpeed;
    }

    //  strafing (A and D)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        newPlayerX -= sin(playerAngle) * moveSpeed;
        newPlayerY += cos(playerAngle) * moveSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        newPlayerX += sin(playerAngle) * moveSpeed;
        newPlayerY -= cos(playerAngle) * moveSpeed;
    }

    // Check for collisions before updating position
    if (worldMap[(int)newPlayerX][(int)playerY] == 0)
    {
        playerX = newPlayerX;
    }
    if (worldMap[(int)playerX][(int)newPlayerY] == 0)
    {
        playerY = newPlayerY;
    }


    static bool firstMouseMovement = true;
    static sf::Vector2i previousMousePosition;


    sf::Vector2i currentMousePosition = sf::Mouse::getPosition(window);


    if (firstMouseMovement)
    {
        previousMousePosition = currentMousePosition;
        firstMouseMovement = false;
    }


    int mouseDeltaX = currentMousePosition.x - previousMousePosition.x;


    playerAngle += mouseDeltaX * 0.002f; // Adjust sensitivity 

    previousMousePosition = currentMousePosition;

    window.setMouseCursorVisible(false);
}

int main()
{
    sf::RenderWindow birdEyeView(sf::VideoMode(400, 400), "Bird's Eye View");
    sf::RenderWindow playerView(sf::VideoMode(screenWidth, screenHeight), "Player's View");

    sf::Texture floorTexture;
    if (!floorTexture.loadFromFile("floor.jpg"))
    {

        return -1;
    }

    while (birdEyeView.isOpen() && playerView.isOpen())
    {
        sf::Event event;
        while (birdEyeView.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                birdEyeView.close();
        }
        while (playerView.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                playerView.close();
        }

        handleInput(playerView);
        auto distances = castRays(playerX, playerY, playerAngle);

        renderBirdsEyeView(birdEyeView, playerX, playerY, playerAngle, distances);
        renderPlayerView(playerView, distances, floorTexture);
    }

    return 0;
}

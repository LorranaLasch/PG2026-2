#include "Game.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

static Game* g_gameInstance = nullptr;

static void keyCallbackDispatcher(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (g_gameInstance) {
        g_gameInstance->handleKey(key, action);
    }
}

Game::Game()
    : window(nullptr),
      screenWidth(1200),
      screenHeight(600),
      shaderProgram(0),
      state(STATE_START),
      texKatnissRun(0),
      texBgDistant(0),
      texBgTrees(0),
      texBgGround(0),
      texStump(0),
      texWasp(0),
      texPin(0),
      texArrow(0),
      baseGameSpeed(360.0f),
      spawnTimer(0.0f),
      spawnInterval(2.2f),
      score(0.0f),
      pinsCollected(0),
      arrowsFired(0),
      shootCooldown(0.0f)
{
    g_gameInstance = this;
    std::srand((unsigned int)std::time(nullptr));
}

Game::~Game() {
    Sprite::cleanupQuadGeometry();

    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
    }

    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

bool Game::init(int width, int height) {
    this->screenWidth = width;
    this->screenHeight = height;

    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(screenWidth, screenHeight, "Jogos Vorazes: A Fuga de Katniss - Lorrana Lasch", nullptr, nullptr);
    if (!window) {
        std::cerr << "Falha ao criar janela GLFW!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallbackDispatcher);
    glfwSwapInterval(1); // VSync ativo

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar GLAD!" << std::endl;
        return false;
    }

    // Inicializa a malha quad (VAO/VBO/EBO) após o GLAD estar pronto
    Sprite::initQuadGeometry();

    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initShaders();
    loadAssets();

    // Configuração de Câmera 2D Ortográfica (0..1200 em X e 0..600 em Y)
    projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);

    glUseProgram(shaderProgram);
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Inicializa a Katniss no nível do chão
    float groundY = 125.0f;
    player.init(texKatnissRun, groundY);

    // Inicializa as camadas de Parallax da Arena
    background.addLayer(texBgDistant, glm::vec2(screenWidth * 0.5f, screenHeight * 0.5f), glm::vec2(screenWidth, screenHeight), 0.15f); // Montanhas/Céu
    background.addLayer(texBgTrees,   glm::vec2(screenWidth * 0.5f, screenHeight * 0.5f), glm::vec2(screenWidth, screenHeight), 0.50f); // Pinheiros do meio
    background.addLayer(texBgGround,  glm::vec2(screenWidth * 0.5f, 50.0f),              glm::vec2(screenWidth, 100.0f),       1.00f); // Chão (terra/grama)

    std::cout << "=================================================================" << std::endl;
    std::cout << " JOGOS VORAZES: A FUGA DE KATNISS NA ARENA" << std::endl;
    std::cout << " Aluna: Lorrana Lasch | Disciplina: Processamento Grafico Unisinos" << std::endl;
    std::cout << "=================================================================" << std::endl;
    std::cout << " Comandos:" << std::endl;
    std::cout << "  [ESPACO] ou [W] ou [SETA CIMA] : Pular obstaculos" << std::endl;
    std::cout << "  [F] ou [ENTER]                 : Disparar Flecha com o arco" << std::endl;
    std::cout << "  [R]                            : Reiniciar partida" << std::endl;
    std::cout << "  [ESC]                          : Sair" << std::endl;
    std::cout << "-----------------------------------------------------------------" << std::endl;

    return true;
}

void Game::initShaders() {
    auto readShaderSource = [](const std::string& filepath) -> std::string {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::ifstream fileAlt("src/TrabalhoPraticoGA/" + filepath);
            if (fileAlt.is_open()) {
                std::stringstream ss;
                ss << fileAlt.rdbuf();
                return ss.str();
            }
            return "";
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    };

    std::string vSourceStr = readShaderSource("shaders/sprite.vs");
    if (vSourceStr.empty()) {
        vSourceStr = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec2 tex_coord;
            out vec2 TexCoord;
            uniform mat4 model;
            uniform mat4 projection;
            uniform vec2 uvOffset;
            uniform vec2 uvScale;
            void main() {
                gl_Position = projection * model * vec4(position, 1.0f);
                TexCoord = uvOffset + tex_coord * uvScale;
            }
        )glsl";
    }

    std::string fSourceStr = readShaderSource("shaders/sprite.fs");
    if (fSourceStr.empty()) {
        fSourceStr = R"glsl(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D spriteTexture;
            uniform vec4 spriteColor;
            void main() {
                vec4 texColor = texture(spriteTexture, TexCoord);
                if (texColor.a < 0.05) discard;
                FragColor = texColor * spriteColor;
            }
        )glsl";
    }

    const char* vSource = vSourceStr.c_str();
    const char* fSource = fSourceStr.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERRO: Vertex Shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERRO: Fragment Shader compilation failed:\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERRO: Shader Program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Game::loadAssets() {
    texKatnissRun = Sprite::loadTexture("textures/katniss_run.png");
    texBgDistant  = Sprite::loadTexture("textures/bg_distant.png");
    texBgTrees    = Sprite::loadTexture("textures/bg_trees.png");
    texBgGround   = Sprite::loadTexture("textures/bg_ground.png");
    texStump      = Sprite::loadTexture("textures/obstacle_stump.png");
    texWasp       = Sprite::loadTexture("textures/obstacle_wasp.png");
    texPin        = Sprite::loadTexture("textures/item_pin.png");
    texArrow      = Sprite::loadTexture("textures/arrow.png");

    // Texturas de interface (HUD e Avisos de Tela)
    texDigits     = Sprite::loadTexture("textures/digits.png");
    texUIGameOver = Sprite::loadTexture("textures/ui_gameover.png");
    texUIStart    = Sprite::loadTexture("textures/ui_start.png");
    texUIPinIcon  = Sprite::loadTexture("textures/ui_pin_icon.png");

    spriteDigit.setTexture(texDigits);
    spriteDigit.setSpritesheet(10, 1);

    spriteUIPin.setTexture(texUIPinIcon);
    spriteUIPin.setSize(glm::vec2(32.0f, 32.0f));
}

void Game::handleKey(int key, int action) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (action == GLFW_PRESS) {
        if (state == STATE_START) {
            if (key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_W || key == GLFW_KEY_UP) {
                state = STATE_PLAYING;
                player.jump();
            }
        }
        else if (state == STATE_PLAYING) {
            if (key == GLFW_KEY_SPACE || key == GLFW_KEY_W || key == GLFW_KEY_UP) {
                player.jump();
            }
            if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER) {
                shootArrow();
            }
        }
        else if (state == STATE_GAMEOVER) {
            if (key == GLFW_KEY_R || key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER) {
                reset();
                state = STATE_PLAYING;
            }
        }
    }
}

void Game::shootArrow() {
    if (shootCooldown <= 0.0f) {
        glm::vec2 pPos = player.getPosition();
        projectiles.emplace_back(texArrow, glm::vec2(pPos.x + 40.0f, pPos.y + 10.0f));
        shootCooldown = 0.35f;
        arrowsFired++;
        std::cout << ">> Flecha disparada por Katniss!" << std::endl;
    }
}

void Game::spawnObstacle() {
    // Escolhe aleatoriamente entre toco no chão, vespa voadora (Tracker Jacker) e broche do Tordo
    int choice = std::rand() % 10;

    if (choice < 5) {
        // Toco de árvore no chão
        float stumpY = 110.0f;
        obstacles.emplace_back(texStump, OBSTACLE_GROUND_STUMP, glm::vec2(screenWidth + 50.0f, stumpY), baseGameSpeed, glm::vec2(65.0f, 65.0f));
    }
    else if (choice < 8) {
        // Vespa voadora da Capital (no ar)
        float waspyY = 150.0f + (std::rand() % 120);
        obstacles.emplace_back(texWasp, OBSTACLE_FLYING_WASP, glm::vec2(screenWidth + 50.0f, waspyY), baseGameSpeed * 1.15f, glm::vec2(55.0f, 55.0f));
    }
    else {
        // Item colecionável: Broche do Tordo (Mockingjay Pin)
        float pinY = 140.0f + (std::rand() % 140);
        obstacles.emplace_back(texPin, ITEM_PIN, glm::vec2(screenWidth + 50.0f, pinY), baseGameSpeed, glm::vec2(50.0f, 50.0f));
    }
}

void Game::update(float deltaTime) {
    if (shootCooldown > 0.0f) {
        shootCooldown -= deltaTime;
    }

    if (state == STATE_PLAYING) {
        score += deltaTime * 10.0f;
        baseGameSpeed += deltaTime * 3.5f; // Dificuldade aumenta suavemente com o tempo

        player.update(deltaTime);
        background.update(baseGameSpeed, deltaTime);

        // Spawn de obstáculos
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.0f;
            spawnInterval = 1.4f + ((float)(std::rand() % 12) / 10.0f); // Intervalo variado entre 1.4s e 2.6s
            spawnObstacle();
        }

        // Atualização dos obstáculos
        for (auto& obs : obstacles) {
            obs.update(deltaTime);

            if (obs.isActive()) {
                // Checagem de colisão da Katniss com obstáculos
                if (obs.getType() == ITEM_PIN) {
                    if (player.getSprite().checkCollision(obs.getSprite(), 10.0f)) {
                        obs.deactivate();
                        pinsCollected++;
                        score += 50.0f;
                        std::cout << "+50 Pontos! Broche do Tordo resgatado! Total: " << pinsCollected << std::endl;
                    }
                }
                else {
                    // Obstáculo perigoso: toco ou vespa
                    if (player.getSprite().checkCollision(obs.getSprite(), 18.0f)) {
                        state = STATE_GAMEOVER;
                        std::cout << "\n========================================================" << std::endl;
                        std::cout << " O CANHAO DISPARA: Katniss foi atingida na Arena!" << std::endl;
                        std::cout << " Pontuacao Final: " << (int)score << " metros" << std::endl;
                        std::cout << " Broches do Tordo: " << pinsCollected << std::endl;
                        std::cout << " Pressione [R] ou [ESPACO] para tentar novamente!" << std::endl;
                        std::cout << "========================================================\n" << std::endl;
                        break;
                    }
                }
            }
        }

        // Atualização dos projéteis (flechas)
        for (auto& proj : projectiles) {
            proj.update(deltaTime);

            if (proj.isActive()) {
                // Checa colisão da flecha com vespas voadoras da Capital
                for (auto& obs : obstacles) {
                    if (obs.isActive() && obs.getType() == OBSTACLE_FLYING_WASP) {
                        if (proj.getSprite().checkCollision(obs.getSprite(), 10.0f)) {
                            proj.deactivate();
                            obs.deactivate();
                            score += 100.0f;
                            std::cout << "+100 Pontos! Vespa Rastreadora abatida pela flecha!" << std::endl;
                            break;
                        }
                    }
                }
            }
        }

        // Limpeza de entidades fora da tela
        for (size_t i = 0; i < obstacles.size(); ) {
            if (obstacles[i].isOffScreen() || !obstacles[i].isActive()) {
                obstacles.erase(obstacles.begin() + i);
            } else {
                ++i;
            }
        }

        for (size_t i = 0; i < projectiles.size(); ) {
            if (projectiles[i].isOffScreen() || !projectiles[i].isActive()) {
                projectiles.erase(projectiles.begin() + i);
            } else {
                ++i;
            }
        }
    }
}

void Game::drawNumber(int number, float startX, float y, float digitWidth, float digitHeight) {
    if (number < 0) number = 0;
    std::string str = std::to_string(number);

    spriteDigit.setSize(glm::vec2(digitWidth, digitHeight));
    spriteDigit.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    for (size_t i = 0; i < str.length(); ++i) {
        int d = str[i] - '0';
        spriteDigit.setFrame(d);
        spriteDigit.setPosition(glm::vec2(startX + i * (digitWidth * 0.82f), y));
        spriteDigit.draw(shaderProgram);
    }
}

void Game::drawHUD() {
    // 1. Placar de Pontos no Canto Superior Esquerdo: Distância
    drawNumber((int)score, 45.0f, screenHeight - 45.0f, 26.0f, 38.0f);

    // 2. Contador de Broches do Tordo (Pins) Coletados
    spriteUIPin.setPosition(glm::vec2(220.0f, screenHeight - 45.0f));
    spriteUIPin.draw(shaderProgram);
    drawNumber(pinsCollected, 250.0f, screenHeight - 45.0f, 22.0f, 32.0f);

    // 3. Banner de Início
    if (state == STATE_START) {
        spriteBanner.setTexture(texUIStart);
        spriteBanner.setSize(glm::vec2(560.0f, 180.0f));
        spriteBanner.setPosition(glm::vec2(screenWidth * 0.5f, screenHeight * 0.55f));
        spriteBanner.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        spriteBanner.draw(shaderProgram);
    }
    // 4. Banner de Game Over na Tela
    else if (state == STATE_GAMEOVER) {
        spriteBanner.setTexture(texUIGameOver);
        spriteBanner.setSize(glm::vec2(600.0f, 220.0f));
        spriteBanner.setPosition(glm::vec2(screenWidth * 0.5f, screenHeight * 0.55f));
        spriteBanner.setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        spriteBanner.draw(shaderProgram);

        // Mostra a pontuação final centralizada logo abaixo do aviso
        drawNumber((int)score, screenWidth * 0.5f - 30.0f, screenHeight * 0.55f - 75.0f, 32.0f, 46.0f);
    }
}

void Game::render() {
    glClearColor(0.08f, 0.12f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // 1. Desenha as camadas de Parallax da Arena
    background.draw(shaderProgram);

    // 2. Desenha a Katniss Everdeen com o Spritesheet
    if (state == STATE_GAMEOVER) {
        // Tinge de vermelho suave no Game Over
        player.getSprite().setColor(glm::vec4(1.0f, 0.4f, 0.4f, 1.0f));
    } else {
        player.getSprite().setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    player.getSprite().draw(shaderProgram);

    // 3. Desenha obstáculos e itens
    for (auto& obs : obstacles) {
        obs.draw(shaderProgram);
    }

    // 4. Desenha as flechas disparadas
    for (auto& proj : projectiles) {
        proj.draw(shaderProgram);
    }

    // 5. Desenha a Interface do Usuário (HUD na tela)
    drawHUD();
}

void Game::reset() {
    state = STATE_START;
    player.reset();
    obstacles.clear();
    projectiles.clear();
    score = 0.0f;
    pinsCollected = 0;
    arrowsFired = 0;
    baseGameSpeed = 360.0f;
    spawnTimer = 0.0f;
    spawnInterval = 2.0f;
    shootCooldown = 0.0f;
    std::cout << "\n>> Arena reiniciada! Pressione [ESPACO] para iniciar a corrida da Katniss.\n" << std::endl;
}

void Game::run() {
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // Limita o deltaTime para evitar saltos bruscos em quedas de quadros
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        update(deltaTime);
        render();

        // Atualização periódica do título da janela com estatísticas
        static double titleTimer = 0.0;
        titleTimer += deltaTime;
        if (titleTimer >= 0.1) {
            titleTimer = 0.0;
            std::string title;
            if (state == STATE_START) {
                title = "Jogos Vorazes: Katniss | Pressione [ESPACO] para Iniciar!";
            } else if (state == STATE_PLAYING) {
                title = "Katniss | Distancia: " + std::to_string((int)score) + "m | Tordos: " + std::to_string(pinsCollected) + " | FPS: " + std::to_string((int)(1.0f / deltaTime));
            } else {
                title = "Katniss | GAME OVER! Placar: " + std::to_string((int)score) + "m | [R] Reiniciar";
            }
            glfwSetWindowTitle(window, title.c_str());
        }

        glfwSwapBuffers(window);
    }
}

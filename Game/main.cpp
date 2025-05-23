#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm> 
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "character.hpp"
#include "texture_manager.hpp"

const int WIDTH = 1500;
const int HEIGHT = 900;

const float CHARACTER_SCALE = 1.0f;

const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord) * vec4(1.2, 1.2, 1.2, 1.0);
    }
)";

GLuint CreateShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << "\n";
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << "\n";
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

int main() {
    std::cout << "Starting main\n";

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LQHN - Game Doi Khang 1v1", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

    srand(static_cast<unsigned int>(time(nullptr)));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint shaderProgram = CreateShaderProgram();

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    TextureManager textureManager;
    textureManager.loadTexture("menu_background", "../x64/Debug/bg.jpg");
    textureManager.loadTexture("game_background", "../x64/Debug/fbg.jpg");
    textureManager.loadTexture("arrow", "../x64/Debug/arrow.PNG");

    if (textureManager.getTexture("menu_background") == 0) {
        std::cerr << "Warning: Could not load menu background texture. Falling back to default background.\n";
    }
    if (textureManager.getTexture("game_background") == 0) {
        std::cerr << "Warning: Could not load game background texture. Falling back to default background.\n";
    }
    if (textureManager.getTexture("arrow") == 0) {
        std::cerr << "Warning: Could not load arrow texture. Falling back to default rectangle.\n";
    }

    bool battleStarted = false;
    int gameMode = 0;
    bool selectingCharacter = false;
    int p1Character = -1, p2Character = -1;
    bool p1Chosen = false, p2Chosen = false;

    Character* p1 = nullptr;
    Character* p2 = nullptr;
    std::vector<BuffItem*> buffs;
    float lastSpawnTime = 0.0f;
    const float SPAWN_INTERVAL = 15.0f;

    float gameEndTime = 0.0f;
    bool gameEnded = false;
    const float GAME_END_DELAY = 1.0f;
    std::string p1BuffMessage = "";
    std::string p2BuffMessage = "";
    float p1BuffMessageTime = 0.0f;
    float p2BuffMessageTime = 0.0f;
    const float BUFF_MESSAGE_DURATION = 3.0f;
    float lastCollisionDamageTime = 0.0f;
    const float COLLISION_DAMAGE_INTERVAL = 1.0f;
    const float COLLISION_DAMAGE = 10.0f;

    bool showGuide = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentTime = static_cast<float>(glfwGetTime());
        if (!battleStarted) {
            GLuint menuTex = textureManager.getTexture("menu_background");
            if (menuTex != 0) {
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, menuTex);
                glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glUseProgram(0);
            }
            else {
                std::cerr << "Warning: Could not load menu background texture during rendering.\n";
            }
        }

        if (battleStarted && !gameEnded) {
            GLuint gameTex = textureManager.getTexture("game_background");
            if (gameTex != 0) {
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gameTex);
                glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glUseProgram(0);
            }
            else {
                std::cerr << "Warning: Could not load game background texture during rendering.\n";
            }

            if (p1 && !p1->isDead) {
                float maxHealth = (p1Character == 1) ? 150.0f : 120.0f;
                float healthPercent = p1->health / maxHealth;
                ImGui::GetForegroundDrawList()->AddRectFilled(
                    ImVec2(10, 10), ImVec2(10 + 200 * healthPercent, 30),
                    ImColor(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::GetForegroundDrawList()->AddRect(
                    ImVec2(10, 10), ImVec2(210, 30), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                char healthText[32];
                snprintf(healthText, sizeof(healthText), "P1: %.1f/%.1f", p1->health, maxHealth);
                ImGui::GetForegroundDrawList()->AddText(ImVec2(10, 40), ImColor(1.0f, 1.0f, 1.0f, 1.0f), healthText);

                // Thanh tụ lực cho Player 1
                XaThu* p1XaThu = dynamic_cast<XaThu*>(p1);
                if (p1XaThu) {
                    float chargePercent = p1XaThu->chargeTime / 5.0f * 100.0f;
                    ImGui::GetForegroundDrawList()->AddRectFilled(
                        ImVec2(10, 50), ImVec2(10 + 200 * (chargePercent / 100.0f), 70),
                        ImColor(0.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::GetForegroundDrawList()->AddRect(
                        ImVec2(10, 50), ImVec2(210, 70), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                    char chargeText[32];
                    snprintf(chargeText, sizeof(chargeText), "Charge: %.0f%%", chargePercent);
                    ImGui::GetForegroundDrawList()->AddText(ImVec2(10, 80), ImColor(1.0f, 1.0f, 1.0f, 1.0f), chargeText);
                }
            }
            if (p2 && !p2->isDead) {
                float maxHealth = (p2Character == 1) ? 150.0f : 120.0f;
                float healthPercent = p2->health / maxHealth;
                ImGui::GetForegroundDrawList()->AddRectFilled(
                    ImVec2(WIDTH - 210, 10), ImVec2(WIDTH - 210 + 200 * healthPercent, 30),
                    ImColor(0.0f, 1.0f, 1.0f, 1.0f));
                ImGui::GetForegroundDrawList()->AddRect(
                    ImVec2(WIDTH - 210, 10), ImVec2(WIDTH - 10, 30), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                char healthText[32];
                snprintf(healthText, sizeof(healthText), "P2: %.1f/%.1f", p2->health, maxHealth);
                ImGui::GetForegroundDrawList()->AddText(ImVec2(WIDTH - 210, 40), ImColor(1.0f, 1.0f, 1.0f, 1.0f), healthText);

                // Thanh tụ lực cho Player 2
                XaThu* p2XaThu = dynamic_cast<XaThu*>(p2);
                if (p2XaThu) {
                    float chargePercent = p2XaThu->chargeTime / 5.0f * 100.0f;
                    ImGui::GetForegroundDrawList()->AddRectFilled(
                        ImVec2(WIDTH - 210, 50), ImVec2(WIDTH - 210 + 200 * (chargePercent / 100.0f), 70),
                        ImColor(0.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::GetForegroundDrawList()->AddRect(
                        ImVec2(WIDTH - 210, 50), ImVec2(WIDTH - 10, 70), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                    char chargeText[32];
                    snprintf(chargeText, sizeof(chargeText), "Charge: %.0f%%", chargePercent);
                    ImGui::GetForegroundDrawList()->AddText(ImVec2(WIDTH - 210, 80), ImColor(1.0f, 1.0f, 1.0f, 1.0f), chargeText);
                }
            }

            if (p1 && !p1->isDead && !p1->damageNumbers.empty()) {
                p1->updateDamageNumbers(currentTime);
                p1->drawDamageNumbers();
            }
            if (p2 && !p2->isDead && !p2->damageNumbers.empty()) {
                p2->updateDamageNumbers(currentTime);
                p2->drawDamageNumbers();
            }
        }

        if (!selectingCharacter && !battleStarted && !showGuide) {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_Always);
            ImGui::Begin("Main Menu", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground);

            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Chon che do choi:").x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.3f);
            ImGui::Text("Chon che do choi:");

            ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.5f);
            if (ImGui::Button("PvP", ImVec2(200, 50))) {
                gameMode = 1;
                selectingCharacter = true;
                p1Character = p2Character = -1;
                p1Chosen = p2Chosen = false;
                p1 = nullptr;
                p2 = nullptr;
                for (BuffItem* b : buffs) delete b;
                buffs.clear();
                gameEnded = false;
                p1BuffMessage = "";
                p2BuffMessage = "";
            }

            ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.6f);
            if (ImGui::Button("Huong dan choi", ImVec2(200, 50))) {
                showGuide = true;
            }

            ImGui::End();
        }

        if (showGuide) {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_Always);
            ImGui::Begin("Huong dan choi", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Huong dan choi").x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.2f);
            ImGui::Text("Huong dan choi");

            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Player 1 (Do):").x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.3f);
            ImGui::TextWrapped("Player 1 (Do):\n- Di chuyen: W A S D\n- Tan cong: E\n- Ne don: Q\n- Skill dac biet: Q (voi XaThu la mui ten dac biet)\n");
            ImGui::Spacing();
            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Player 2 (Xanh):").x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.4f);
            ImGui::TextWrapped("Player 2 (Xanh):\n- Di chuyen: Mui ten\n- Tan cong: Space\n- Ne don: Enter\n- Skill dac biet: Q (voi XaThu la mui ten dac biet)\n");
            ImGui::Spacing();
            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Buffs:").x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.5f);
            ImGui::TextWrapped("- Buffs xuat hien ngau nhien sau moi 15s.\n- Buffs gom: HOI MAU, GIAP, TANG SAT THUONG, TANG TOC.\n- Co the tan cong ke dich de gay sat thuong va thang tran khi ke dich het mau.");

            ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.7f);
            if (ImGui::Button("Quay lai menu", ImVec2(200, 50))) {
                showGuide = false;
            }
            ImGui::End();
        }

        if (selectingCharacter) {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_Always);
            ImGui::Begin("Character Selection", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoBackground);

            if (gameMode == 1) {
                if (!p1Chosen) {
                    ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Nguoi choi 1: Chon tuong").x) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.2f);
                    ImGui::Text("Nguoi choi 1: Chon tuong");

                    ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.4f);
                    if (ImGui::Button("Xa Thu##P1", ImVec2(200, 50))) {
                        p1Character = 0;
                        p1Chosen = true;
                        p1 = new XaThu(150.0f, HEIGHT - 50.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), textureManager);
                    }
                    ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.5f);
                    if (ImGui::Button("Dau Si##P1", ImVec2(200, 50))) {
                        p1Character = 1;
                        p1Chosen = true;
                        p1 = new DauSi(150.0f, HEIGHT - 50.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), textureManager);
                    }
                }
                else if (!p2Chosen) {
                    ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Nguoi choi 2: Chon tuong").x) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.2f);
                    ImGui::Text("Nguoi choi 2: Chon tuong");

                    ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.4f);
                    if (ImGui::Button("Xa Thu##P2", ImVec2(200, 50))) {
                        p2Character = 0;
                        p2Chosen = true;
                        p2 = new XaThu(1350.0f, HEIGHT - 50.0f, ImVec4(0.0f, 1.0f, 1.0f, 1.0f), textureManager);
                    }
                    ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.5f);
                    if (ImGui::Button("Dau Si##P2", ImVec2(200, 50))) {
                        p2Character = 1;
                        p2Chosen = true;
                        p2 = new DauSi(1350.0f, HEIGHT - 50.0f, ImVec4(0.0f, 1.0f, 1.0f, 1.0f), textureManager);
                    }
                }
                else {
                    ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("San sang bat dau!").x) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.2f);
                    ImGui::Text("San sang bat dau!");

                    ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
                    ImGui::SetCursorPosY(HEIGHT * 0.5f);
                    if (ImGui::Button("Bat dau tran dau", ImVec2(200, 50))) {
                        selectingCharacter = false;
                        battleStarted = true;
                        lastSpawnTime = static_cast<float>(glfwGetTime());
                        lastCollisionDamageTime = lastSpawnTime;
                        gameEnded = false;
                    }
                }
            }

            ImGui::End();
        }

        if (battleStarted && p1 && p2 && !gameEnded) {
            if (!p1->isDead) {
                p1->move(ImGuiKey_A, ImGuiKey_D, ImGuiKey_W, ImGuiKey_S);
                p1->dodge(ImGuiKey_Q);
                if (p1Character == 0) { // XaThu
                    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
                        std::cerr << "P1 (XaThu) pressed attack key E\n";
                    }
                    p1->attack(p2, ImGuiKey_E);
                }
                else { // DauSi
                    if (ImGui::IsKeyPressed(ImGuiKey_E)) {
                        std::cerr << "P1 (DauSi) pressed attack key E\n";
                    }
                    p1->attack(p2, ImGuiKey_E);
                }
                p1->useSkill(p2);
            }
            if (!p2->isDead) {
                p2->move(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
                p2->dodge(ImGuiKey_Enter);
                if (p2Character == 0) { // XaThu
                    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                        std::cerr << "P2 (XaThu) pressed attack key Space\n";
                    }
                    p2->attack(p1, ImGuiKey_Space); // Player 2 bắn mũi tên với phím Space
                }
                else { // DauSi
                    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
                        std::cerr << "P2 (DauSi) pressed attack key Space\n";
                    }
                    p2->attack(p1, ImGuiKey_Space);
                }
                p2->useSkill(p1);
            }

            if (!p1->isDead) p1->draw();
            if (!p2->isDead) p2->draw();

            if (currentTime - lastSpawnTime > SPAWN_INTERVAL) {
                lastSpawnTime = currentTime;

                float randomX = (WIDTH / 3.0f) + (rand() % static_cast<int>(WIDTH / 3.0f));
                float randomY = (HEIGHT / 3.0f) + (rand() % static_cast<int>(HEIGHT / 3.0f));

                float scaledSize = 50.0f * CHARACTER_SCALE;
                float LEFT_LIMIT = -100.0f;
                float RIGHT_LIMIT = 1500.0f - scaledSize + 100.0f;
                float GRASS_TOP = 300.0f;
                float GRASS_BOTTOM = 900.0f - scaledSize + 89.0f;

                float spawnX = std::clamp(randomX, LEFT_LIMIT, RIGHT_LIMIT);
                float spawnY = std::clamp(randomY, GRASS_TOP, GRASS_BOTTOM);
                BuffItem::BuffType buff = static_cast<BuffItem::BuffType>(rand() % 4);
                ImVec4 buffColor = (buff == BuffItem::DAMAGE_BOOST) ? ImVec4(1.0f, 0.5f, 0.5f, 1.0f) :
                    (buff == BuffItem::HEAL) ? ImVec4(0.5f, 1.0f, 0.5f, 1.0f) :
                    (buff == BuffItem::SHIELD) ? ImVec4(0.5f, 0.5f, 1.0f, 1.0f) :
                    ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                buffs.push_back(new BuffItem(spawnX, spawnY, buffColor, buff));
            }

            for (auto it = buffs.begin(); it != buffs.end(); ) {
                BuffItem* b = *it;
                bool p1Hit = p1 && !p1->isDead && p1->isCollidingWith(b);
                bool p2Hit = p2 && !p2->isDead && p2->isCollidingWith(b);

                if (p1Hit) {
                    if (!p1->isDodging) {
                        b->applyBuff(p1);
                        std::string buffType = (b->type == BuffItem::HEAL) ? "HEAL" :
                            (b->type == BuffItem::DAMAGE_BOOST) ? "DAMAGE_BOOST" :
                            (b->type == BuffItem::SHIELD) ? "SHIELD" : "SPEED";
                        p1BuffMessage = "Player 1 received " + buffType + " buff!";
                        p1BuffMessageTime = currentTime;
                        delete b;
                        it = buffs.erase(it);
                        continue;
                    }
                }
                else if (p2Hit) {
                    if (!p2->isDodging) {
                        b->applyBuff(p2);
                        std::string buffType = (b->type == BuffItem::HEAL) ? "HEAL" :
                            (b->type == BuffItem::DAMAGE_BOOST) ? "DAMAGE_BOOST" :
                            (b->type == BuffItem::SHIELD) ? "SHIELD" : "SPEED";
                        p2BuffMessage = "Player 2 received " + buffType + " buff!";
                        p2BuffMessageTime = currentTime;
                        delete b;
                        it = buffs.erase(it);
                        continue;
                    }
                }
                else {
                    b->draw();
                    ++it;
                }
            }

            if (!p1BuffMessage.empty() && (currentTime - p1BuffMessageTime < BUFF_MESSAGE_DURATION)) {
                ImGui::SetNextWindowPos(ImVec2(10, 60), ImGuiCond_Always);
                ImGui::Begin("P1 Buff", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::Text("%s", p1BuffMessage.c_str());
                ImGui::End();
            }
            if (!p2BuffMessage.empty() && (currentTime - p2BuffMessageTime < BUFF_MESSAGE_DURATION)) {
                ImGui::SetNextWindowPos(ImVec2(WIDTH - 10, 60), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
                ImGui::Begin("P2 Buff", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::Text("%s", p2BuffMessage.c_str());
                ImGui::End();
            }

            if ((p1->health <= 0 || p2->health <= 0) && battleStarted && !gameEnded) {
                gameEndTime = currentTime;
                gameEnded = true;
                if (p1->health <= 0) p1->isDead = true;
                if (p2->health <= 0) p2->isDead = true;
            }
        }

        if (gameEnded && p1 && p2 && currentTime - gameEndTime > GAME_END_DELAY) {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT), ImGuiCond_Always);
            ImGui::Begin("Game Over", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Ket Qua").x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.3f);
            ImGui::Text("Ket Qua");

            if (p1->isDead && !p2->isDead) {
                ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Player 2 wins!").x) * 0.5f);
                ImGui::SetCursorPosY(HEIGHT * 0.4f);
                ImGui::Text("Player 2 wins!");
            }
            else if (p2->isDead && !p1->isDead) {
                ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Player 1 wins!").x) * 0.5f);
                ImGui::SetCursorPosY(HEIGHT * 0.4f);
                ImGui::Text("Player 1 wins!");
            }
            else {
                ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize("Draw!").x) * 0.5f);
                ImGui::SetCursorPosY(HEIGHT * 0.4f);
                ImGui::Text("Draw!");
            }

            char scoreText[100];
            snprintf(scoreText, sizeof(scoreText), "Final Score - P1 Health: %.1f, P2 Health: %.1f",
                p1->health, p2->health);
            ImGui::SetCursorPosX((WIDTH - ImGui::CalcTextSize(scoreText).x) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.5f);
            ImGui::Text("%s", scoreText);

            ImGui::SetCursorPosX((WIDTH - 200) * 0.5f);
            ImGui::SetCursorPosY(HEIGHT * 0.6f);
            if (ImGui::Button("Quay lai menu", ImVec2(200, 50))) {
                battleStarted = false;
                selectingCharacter = false;
                delete p1; p1 = nullptr;
                delete p2; p2 = nullptr;
                for (BuffItem* b : buffs) delete b;
                buffs.clear();
                p1BuffMessage = "";
                p2BuffMessage = "";
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    textureManager.clear();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    delete p1;
    delete p2;
    for (BuffItem* b : buffs) delete b;

    return 0;
}
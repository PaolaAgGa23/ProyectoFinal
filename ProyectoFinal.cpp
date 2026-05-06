#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include "stb_image.h"

// ============================================================
//  CONSTANTES
// ============================================================
const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ============================================================
//  PROTOTIPOS — entrada
// ============================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void DoMovement();

// ============================================================
//  PROTOTIPOS — objetos y animaciones
// ============================================================
GLuint LoadTexture(const char* path);

static void _SendAndDraw(GLuint VAO, GLint modelLoc, GLint colorLoc,
    GLint useTexLoc, GLuint texID,
    const glm::mat4& model, glm::vec3 color);

void DrawBox(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color);
void DrawBoxRotatedX(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle);
void DrawBoxRotatedY(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle);

void DrawStand(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tPVC, GLuint tMetal, glm::vec3 pos, float rotY);
void DrawChair(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tWood, glm::vec3 pos, float rotY);
void DrawBrochure(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos);
void DrawFlag(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos, float time);
void DrawLattice(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tWood, glm::vec3 basePos);
void DrawDino(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos, float time);
void DrawDoor(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos, float openAngle);
void DrawConfetti(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, float timer);
void DrawSignage(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos, float time);
void DrawMural(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 basePos);
void DrawAgents(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, float time);

// ============================================================
//  CAMARA Y ESTADO
// ============================================================
Camera  camera(glm::vec3(0.0f, 3.0f, 12.0f));
GLfloat lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;
bool    keys[1024] = {};
bool    firstMouse = true;
GLfloat deltaTime = 0.0f, lastFrame = 0.0f;

bool  confettiActive = false;
float confettiTimer = 0.0f;
float doorAngle = 0.0f;
bool  doorOpen = false;

// ============================================================
//  CONFETI — pool preinicializado
// ============================================================
struct ConfettiParticle { float x, z, speed, rotSpeed, colorR, colorG, colorB, phase; };
const int NUM_CONFETTI = 200;
ConfettiParticle confParticles[NUM_CONFETTI];

void InitConfetti()
{
    glm::vec3 pal[3] = { {0.0f,0.27f,0.55f},{0.85f,0.65f,0.13f},{1.0f,1.0f,1.0f} };
    for (int i = 0; i < NUM_CONFETTI; i++) {
        confParticles[i].x = ((rand() % 2800) / 100.0f) - 14.0f;
        confParticles[i].z = ((rand() % 1900) / 100.0f) - 9.5f;
        confParticles[i].speed = 0.8f + (rand() % 50) / 100.0f;
        confParticles[i].rotSpeed = 30.0f + (rand() % 120);
        confParticles[i].phase = (rand() % 300) / 100.0f;
        int c = rand() % 3;
        confParticles[i].colorR = pal[c].r;
        confParticles[i].colorG = pal[c].g;
        confParticles[i].colorB = pal[c].b;
    }
}

// ============================================================
//  BEZIER — Animacion compleja 2
// ============================================================
struct BezierPath { glm::vec3 p0, p1, p2, p3; float speed, phase; };

glm::vec3 BezierEval(const BezierPath& b, float t)
{
    float u = 1.0f - t;
    return u * u * u * b.p0 + 3 * u * u * t * b.p1 + 3 * u * t * t * b.p2 + t * t * t * b.p3;
}

const int NUM_AGENTS = 5;
BezierPath agentPaths[NUM_AGENTS] = {
    // Ruta 1: entrada norte → stands oeste (se queda en pasillo central)
    {{ -8.0f,0, 7.0f},{ -5.0f,0, 4.0f},{ -3.0f,0, 0.0f},{ -7.0f,0,-4.0f}, 0.18f, 0.00f},
    // Ruta 2: cruce diagonal por el pasillo central
    {{  7.0f,0, 6.0f},{  2.0f,0, 4.0f},{ -2.0f,0,-1.0f},{  6.0f,0,-5.0f}, 0.15f, 0.20f},
    // Ruta 3: ruta cerca del busto
    {{  2.0f,0, 7.0f},{  1.0f,0, 3.5f},{  0.0f,0, 0.5f},{ -2.0f,0,-2.5f}, 0.20f, 0.40f},
    // Ruta 4: paseo largo de lado a lado (dentro de límites)
    {{  8.0f,0, 0.5f},{  3.0f,0, 3.0f},{ -3.0f,0, 3.0f},{ -8.0f,0, 0.5f}, 0.12f, 0.60f},
    // Ruta 5: corta junto al portafolletos
    {{ -6.0f,0, 6.5f},{ -7.0f,0, 2.5f},{ -7.5f,0,-1.5f},{ -7.0f,0,-6.0f}, 0.22f, 0.80f},
};

// ============================================================
//  CUBO CON NORMALES Y UVS (8 floats por vertice)
// ============================================================
float vertices[] = {
    // pos              normal      uv
    -0.5f,-0.5f, 0.5f, 0,0,1,  0.0f,0.0f,
     0.5f,-0.5f, 0.5f, 0,0,1,  1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 0,0,1,  1.0f,1.0f,
     0.5f, 0.5f, 0.5f, 0,0,1,  1.0f,1.0f,
    -0.5f, 0.5f, 0.5f, 0,0,1,  0.0f,1.0f,
    -0.5f,-0.5f, 0.5f, 0,0,1,  0.0f,0.0f,

    -0.5f,-0.5f,-0.5f, 0,0,-1, 1.0f,0.0f,
     0.5f,-0.5f,-0.5f, 0,0,-1, 0.0f,0.0f,
     0.5f, 0.5f,-0.5f, 0,0,-1, 0.0f,1.0f,
     0.5f, 0.5f,-0.5f, 0,0,-1, 0.0f,1.0f,
    -0.5f, 0.5f,-0.5f, 0,0,-1, 1.0f,1.0f,
    -0.5f,-0.5f,-0.5f, 0,0,-1, 1.0f,0.0f,

    -0.5f, 0.5f, 0.5f,-1,0,0,  1.0f,0.0f,
    -0.5f, 0.5f,-0.5f,-1,0,0,  1.0f,1.0f,
    -0.5f,-0.5f,-0.5f,-1,0,0,  0.0f,1.0f,
    -0.5f,-0.5f,-0.5f,-1,0,0,  0.0f,1.0f,
    -0.5f,-0.5f, 0.5f,-1,0,0,  0.0f,0.0f,
    -0.5f, 0.5f, 0.5f,-1,0,0,  1.0f,0.0f,

     0.5f, 0.5f, 0.5f, 1,0,0,  1.0f,0.0f,
     0.5f, 0.5f,-0.5f, 1,0,0,  1.0f,1.0f,
     0.5f,-0.5f,-0.5f, 1,0,0,  0.0f,1.0f,
     0.5f,-0.5f,-0.5f, 1,0,0,  0.0f,1.0f,
     0.5f,-0.5f, 0.5f, 1,0,0,  0.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1,0,0,  1.0f,0.0f,

    -0.5f,-0.5f,-0.5f, 0,-1,0, 0.0f,1.0f,
     0.5f,-0.5f,-0.5f, 0,-1,0, 1.0f,1.0f,
     0.5f,-0.5f, 0.5f, 0,-1,0, 1.0f,0.0f,
     0.5f,-0.5f, 0.5f, 0,-1,0, 1.0f,0.0f,
    -0.5f,-0.5f, 0.5f, 0,-1,0, 0.0f,0.0f,
    -0.5f,-0.5f,-0.5f, 0,-1,0, 0.0f,1.0f,

    -0.5f, 0.5f,-0.5f, 0,1,0,  0.0f,1.0f,
     0.5f, 0.5f,-0.5f, 0,1,0,  1.0f,1.0f,
     0.5f, 0.5f, 0.5f, 0,1,0,  1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 0,1,0,  1.0f,0.0f,
    -0.5f, 0.5f, 0.5f, 0,1,0,  0.0f,0.0f,
    -0.5f, 0.5f,-0.5f, 0,1,0,  0.0f,1.0f,
};

// ============================================================
//  MAIN
// ============================================================
int main()
{
    srand(42);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
        "Lobby JBS — Auditorio Javier Barros Sierra", nullptr, nullptr);
    if (!window) { glfwTerminate(); return EXIT_FAILURE; }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return EXIT_FAILURE;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // --------------------------------------------------------
    //  Shaders
    // --------------------------------------------------------
    Shader shaderColor("Shader/core.vs", "Shader/core.frag");
    Shader shaderModel("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // --------------------------------------------------------
    //  Modelo externo
    // --------------------------------------------------------
    Model person((char*)"Models/person.obj");

    // --------------------------------------------------------
    //  VAO / VBO con posicion + normal + UV
    // --------------------------------------------------------
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // attrib 0: posicion
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // attrib 1: normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // attrib 2: UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // --------------------------------------------------------
    //  Texturas
    //  Pon los archivos en una carpeta Textures/ junto al .exe
    //  Si falta un archivo, carga una textura blanca (no crashea)
    // --------------------------------------------------------
    GLuint tFloor = LoadTexture("images/floor_tile.jpg");
    GLuint tWall = LoadTexture("images/wall_concrete.jpg");
    GLuint tStone = LoadTexture("images/stone_volcanic.jpg");
    GLuint tWood = LoadTexture("images/wood_oak.jpg");
    GLuint tMetal = LoadTexture("images/metal_brushed.jpg");
    GLuint tPVC = LoadTexture("images/pvc_white.jpg");
    GLuint tConcr = LoadTexture("images/concrete_grey.jpg");

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 200.0f);

    InitConfetti();

    // ============================================================
    //  LOOP PRINCIPAL
    // ============================================================
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        if (confettiActive) {
            confettiTimer += deltaTime;
            if (confettiTimer > 5.0f) { confettiActive = false; confettiTimer = 0.0f; }
        }
        float targetAngle = doorOpen ? 90.0f : 0.0f;
        doorAngle += (targetAngle - doorAngle) * (1.0f - expf(-deltaTime / 0.12f));

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();

        // ==============  SHADER COLOR + TEXTURA  ==============
        shaderColor.Use();

        GLint modelLoc = glGetUniformLocation(shaderColor.Program, "model");
        GLint viewLoc = glGetUniformLocation(shaderColor.Program, "view");
        GLint projLoc = glGetUniformLocation(shaderColor.Program, "projection");
        GLint colorLoc = glGetUniformLocation(shaderColor.Program, "color");
        GLint useTexLoc = glGetUniformLocation(shaderColor.Program, "useTexture");
        GLint lightPLoc = glGetUniformLocation(shaderColor.Program, "lightPos");
        GLint lightCLoc = glGetUniformLocation(shaderColor.Program, "lightColor");
        GLint viewPLoc = glGetUniformLocation(shaderColor.Program, "viewPos");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glm::vec3 lightPos(0.0f, 15.0f, 5.0f);
        glm::vec3 lightColor(1.0f, 0.98f, 0.92f);
        glUniform3fv(lightPLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(lightCLoc, 1, glm::value_ptr(lightColor));
        glUniform3fv(viewPLoc, 1, glm::value_ptr(camera.GetPosition()));

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderColor.Program, "texture1"), 0);

        // ====================================================
        //  PISO — cuadros oscuros grandes con retícula beige
        //  Fiel a fotos: baldosas ~2.5x2.5 m, líneas beige gruesas
        // ====================================================
        // Base negra/oscura
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tFloor,
            glm::vec3(0, -0.15f, 0), glm::vec3(30.0f, 0.3f, 22.0f),
            glm::vec3(0.15f, 0.15f, 0.16f));
        // Líneas beige horizontales (separadas 2.5 m = tamaño de baldosa real)
        for (int li = -4; li <= 4; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(0, 0.01f, li * 2.5f), glm::vec3(28.0f, 0.025f, 0.18f),
                glm::vec3(0.70f, 0.65f, 0.48f));
        // Líneas beige verticales
        for (int li = -5; li <= 5; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(li * 2.5f, 0.01f, 0), glm::vec3(0.18f, 0.025f, 22.0f),
                glm::vec3(0.70f, 0.65f, 0.48f));

        // ====================================================
        //  COLUMNAS CILINDRICAS — 8 caras (octágono)
        //  Cantera gris granulada, radio ~0.45 m, altura 8 m
        // ====================================================
        {
            float colH = 8.0f, r = 0.45f, fw = 0.34f;
            glm::vec3 cCol(0.72f, 0.70f, 0.66f);
            auto DrawColumn = [&](float cx, float cz) {
                float ang[8] = { 0,45,90,135,180,225,270,315 };
                for (int fi = 0; fi < 8; fi++) {
                    float a = glm::radians(ang[fi]);
                    float fx = cx + r * cosf(a), fz_ = cz + r * sinf(a);
                    glm::mat4 m(1);
                    m = glm::translate(m, { fx,colH / 2.0f,fz_ });
                    m = glm::rotate(m, a, { 0,1,0 });
                    m = glm::scale(m, { fw,colH,fw });
                    _SendAndDraw(VAO, modelLoc, colorLoc, useTexLoc, tStone, m, cCol);
                }
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
                    { cx,colH / 2.0f,cz }, { r * 1.35f,colH,r * 1.35f }, cCol);
                };
            // Fila izquierda (lado abierto explanada) — 6 columnas
            for (int ci = 0; ci < 6; ci++) DrawColumn(-10.0f, -8.0f + ci * 3.5f);
            // Fila derecha (lado muro/celosía) — 6 columnas
            for (int ci = 0; ci < 6; ci++) DrawColumn(10.0f, -8.0f + ci * 3.5f);
            // Columnas interiores del lobby — alejadas de la escalera
            DrawColumn(-6.5f, 0.0f);
            DrawColumn(6.5f, 0.0f);
        }

        // ====================================================
        //  TECHO — concreto blanco con ductos (fiel a fotos)
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tConcr,
            glm::vec3(0, 8.15f, 0), glm::vec3(28.0f, 0.3f, 22.0f),
            glm::vec3(0.88f, 0.88f, 0.86f));
        // Ductos en techo
        for (int di = 0; di < 3; di++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(-5.0f + di * 4.0f, 7.85f, 0), { 0.18f,0.18f,20.0f },
                glm::vec3(0.50f, 0.50f, 0.52f));
        // Lámparas fluorescentes
        for (int li = 0; li < 4; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(-6.0f + li * 4.0f, 7.95f, -2.0f), { 1.5f,0.08f,0.5f },
                glm::vec3(0.98f, 0.98f, 0.92f));

        // ====================================================
        //  ESCALERA — oscura con barandal AMARILLO (fotos reales)
        // ====================================================
        {
            int nSteps = 14; float stepH = 0.28f, stepD = 0.48f, stairW = 5.20f;
            float stairX = -2.20f, startZ = 3.20f, startY = 0.0f;
            glm::vec3 sTC(0.20f, 0.20f, 0.22f), sFC(0.15f, 0.15f, 0.17f);
            glm::vec3 sSide(0.88f, 0.88f, 0.86f), cAmar(0.88f, 0.72f, 0.04f);

            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { stairX - stairW / 2 - 0.25f,1.05f,0.05f }, { 0.35f,2.10f,7.40f }, sSide);
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { stairX + stairW / 2 + 0.25f,1.05f,0.05f }, { 0.35f,2.10f,7.40f }, sSide);

            for (int i = 0; i < nSteps; i++) {
                float y = startY + (i * stepH), z = startZ - (i * stepD);
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    { stairX,y + 0.04f,z }, { stairW,0.08f,stepD }, sTC);
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    { stairX,y - stepH / 2,z + stepD / 2 }, { stairW,stepH,0.08f }, sFC);
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    { stairX,y / 2 - 0.06f,z }, { stairW,y + 0.12f,stepD }, { 0.18f,0.18f,0.20f });
                // Barandales AMARILLOS — postes verticales + tubo inclinado
                // Los postes van a los lados de la escalera, un poste cada 2 escalones
                for (int i = 0; i < nSteps; i += 2) {
                    float y = startY + (i * stepH) + 0.55f;
                    float z = startZ - (i * stepD);
                    // Poste izquierdo
                    DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                        glm::vec3(stairX - stairW / 2 + 0.15f, startY + (i * stepH) + 0.30f, z),
                        { 0.06f,0.60f,0.06f }, cAmar);
                    // Poste derecho
                    DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                        glm::vec3(stairX + stairW / 2 - 0.15f, startY + (i * stepH) + 0.30f, z),
                        { 0.06f,0.60f,0.06f }, cAmar);
                }
                // Tubo horizontal del barandal (inclinado siguiendo la escalera)
                // Se dibuja segmento a segmento para seguir exactamente la pendiente
                for (int i = 0; i < nSteps - 1; i++) {
                    float y0 = startY + (i * stepH) + 0.58f, z0 = startZ - (i * stepD);
                    float y1 = startY + ((i + 1) * stepH) + 0.58f, z1 = startZ - ((i + 1) * stepD);
                    float cy = (y0 + y1) / 2.0f, cz_ = (z0 + z1) / 2.0f;
                    float len = sqrtf((y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0));
                    float ang = glm::degrees(atan2f(y1 - y0, z0 - z1));
                    // Tubo izquierdo
                    DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                        glm::vec3(stairX - stairW / 2 + 0.15f, cy, cz_),
                        { 0.06f,0.06f,len }, cAmar, ang);
                    // Tubo derecho
                    DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                        glm::vec3(stairX + stairW / 2 - 0.15f, cy, cz_),
                        { 0.06f,0.06f,len }, cAmar, ang);
                }
            }
            float topY = startY + (nSteps * stepH), topZ = startZ - (nSteps * stepD) - 0.20f;
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { stairX,topY - 0.08f,topZ }, { stairW + 1.20f,0.22f,2.00f }, { 0.72f,0.72f,0.70f });
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { stairX,topY - 0.10f,topZ - 1.55f }, { stairW + 3.00f,0.25f,2.20f }, { 0.68f,0.68f,0.66f });
            DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tConcr,
                { 4.20f,topY + 2.45f,topZ - 2.90f }, { 5.60f,0.28f,6.80f }, { 0.85f,0.85f,0.83f }, -42.0f);
            // (tubos ya dibujados segmento a segmento arriba)
        }

        // ====================================================
        //  MOSTRADOR ROJO SEMICIRCULAR (foto 1 — forma curva)
        //  6 segmentos en arco de 180°
        // ====================================================
        {
            glm::vec3 cRed(0.82f, 0.08f, 0.06f), cTop(0.96f, 0.96f, 0.96f);
            glm::vec3 ctr(6.5f, 0.0f, 2.2f); float R = 1.5f, h = 1.2f;
            for (int si = 0; si < 6; si++) {
                float a0 = glm::radians(180.0f + si * 30.0f);
                float a1 = glm::radians(180.0f + (si + 1) * 30.0f);
                float amid = (a0 + a1) * 0.5f;
                float cx_ = ctr.x + R * cosf(amid), cz_ = ctr.z + R * sinf(amid);
                glm::mat4 m(1);
                m = glm::translate(m, { cx_,h / 2.0f,cz_ });
                m = glm::rotate(m, amid + glm::radians(90.0f), { 0,1,0 });
                m = glm::scale(m, { R * (a1 - a0) * 1.1f,h,0.12f });
                _SendAndDraw(VAO, modelLoc, colorLoc, useTexLoc, 0, m, cRed);
            }
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { ctr.x,h + 0.06f,ctr.z }, { R * 2.1f,0.12f,R * 1.1f }, cTop);
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { ctr.x,h / 2.0f,ctr.z + R * 0.4f }, { R * 1.8f,h,0.10f }, cRed);
        }

        // ====================================================
        //  BUSTO JAVIER BARROS SIERRA (foto 1)
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { 1.8f,0.8f,2.0f }, { 0.9f,1.6f,0.9f }, { 0.55f,0.55f,0.52f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { 1.8f,1.62f,2.0f }, { 1.2f,0.12f,1.2f }, { 0.50f,0.50f,0.48f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMetal,
            { 1.8f,2.15f,2.0f }, { 0.7f,0.55f,0.5f }, { 0.52f,0.50f,0.48f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMetal,
            { 1.8f,2.68f,2.0f }, { 0.48f,0.52f,0.48f }, { 0.54f,0.52f,0.50f });
        // Placa negra
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { -0.5f,2.2f,-0.5f }, { 1.8f,0.45f,0.05f }, { 0.08f,0.08f,0.08f });

        // ====================================================
        //  PAREDES DEL LOBBY
        // ====================================================
        // Muro fondo blanco
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            { 0,4.0f,-9.8f }, { 30.0f,8.5f,0.3f }, { 0.90f,0.90f,0.90f });
        // Muro lateral derecho (con ventana de vidrio — foto 5)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            { 14.8f,4.0f,0 }, { 0.3f,8.5f,22.0f }, { 0.90f,0.90f,0.90f });
        // Muro de piedra volcánica negra (foto 2/6, junto a celosía)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { 13.5f,3.5f,3.0f }, { 0.4f,7.0f,6.0f }, { 0.20f,0.18f,0.16f });
        // Muro ladrillo rojo (foto 5)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 11.5f,3.5f,-2.0f }, { 0.3f,7.0f,4.0f }, { 0.55f,0.22f,0.15f });
        // Ventana de vidrio tipo cuadricula (foto 2 y 7)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 13.0f,2.0f,-2.0f }, { 0.15f,4.0f,3.5f }, { 0.62f,0.78f,0.85f });

        // Extintor rojo (foto 7)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.5f,1.8f,-7.5f }, { 0.22f,0.55f,0.22f }, { 0.80f,0.08f,0.06f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.5f,2.42f,-7.5f }, { 0.10f,0.15f,0.10f }, { 0.50f,0.50f,0.52f });

        // ====================================================
        //  MURAL — en pared superior inclinada del lobby (foto 7)
        //  Posicion alta, sobre la puerta de acceso al auditorio
        // ====================================================
        DrawMural(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(-5.5f, 6.2f, -9.4f));

        // ====================================================
        //  OBJETOS DE EVENTO (stands, sillas, portafolletos, etc.)
        // ====================================================
        // Stands + sillas
        for (int i = 0; i < 4; i++) {
            float zPos = -6.0f + i * 4.0f;
            DrawStand(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tPVC, tMetal,
                { -11,0,zPos }, 0.0f);
            DrawChair(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
                { -9,0,zPos }, 180.0f);
        }
        DrawBrochure(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { -9.5f,0,-7.5f });
        DrawBrochure(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { -9.5f,0, 0.5f });
        // Banderas
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { -5,0,-8.5f }, currentFrame);
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { 0,0,-8.5f }, currentFrame);
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { 5,0,-8.5f }, currentFrame);
        // Celosía naranja/dorada — lado izquierdo frontal (foto 6)
        // La reja está entre las columnas izquierdas, frente a la entrada
        DrawLattice(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            glm::vec3(-14.0f, 0.0f, -1.5f));

        // Dinosaurio — animacion compleja 1
        DrawDino(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(0, 0, 5.5f), currentFrame);

        // Puerta — animacion simple 1 (tecla F)
        DrawDoor(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(0, 0, -8.0f), doorAngle);

        // Señaletica pulsante — animacion simple 2
        DrawSignage(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(-8, 3.5f, -4), currentFrame);
        DrawSignage(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(8, 3.5f, -4), currentFrame);

        // Confeti — animacion simple 3 (tecla E)
        if (confettiActive)
            DrawConfetti(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, confettiTimer);

        // Agentes Bezier — animacion compleja 2
        DrawAgents(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, currentFrame);

        // ==============  SHADER MODELO (original)  ==============
        shaderModel.Use();
        GLint mLocM = glGetUniformLocation(shaderModel.Program, "model");
        GLint vLocM = glGetUniformLocation(shaderModel.Program, "view");
        GLint pLocM = glGetUniformLocation(shaderModel.Program, "projection");
        glUniformMatrix4fv(vLocM, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(pLocM, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 modelM(1.0f);
        modelM = glm::translate(modelM, glm::vec3(4.5f, 0, 4.5f));
        modelM = glm::rotate(modelM, glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelM = glm::scale(modelM, glm::vec3(0.42f));
        glUniformMatrix4fv(mLocM, 1, GL_FALSE, glm::value_ptr(modelM));
        person.Draw(shaderModel);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// ============================================================
//  CARGA DE TEXTURA — fallback a blanco si no encuentra el archivo
// ============================================================
GLuint LoadTexture(const char* path)
{
    GLuint id;
    glGenTextures(1, &id);

    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);

    if (data) {
        GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "Textura cargada: " << path << std::endl;
    }
    else {
        unsigned char white[3] = { 255,255,255 };
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
        std::cout << "ADVERTENCIA: No se encontro " << path << " — usando blanco\n";
    }
    stbi_image_free(data);
    return id;
}

// ============================================================
//  HELPER INTERNO — envia matriz + color/textura y dibuja
// ============================================================
static void _SendAndDraw(GLuint VAO, GLint modelLoc, GLint colorLoc,
    GLint useTexLoc, GLuint texID,
    const glm::mat4& model, glm::vec3 color)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    if (texID != 0) {
        glUniform1i(useTexLoc, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    else {
        glUniform1i(useTexLoc, GL_FALSE);
    }
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// ============================================================
//  PRIMITIVAS
// ============================================================
void DrawBox(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color)
{
    glm::mat4 m(1); m = glm::translate(m, pos); m = glm::scale(m, scale);
    _SendAndDraw(VAO, mL, cL, uTL, tex, m, color);
}

void DrawBoxRotatedX(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle)
{
    glm::mat4 m(1); m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(angle), { 1,0,0 }); m = glm::scale(m, scale);
    _SendAndDraw(VAO, mL, cL, uTL, tex, m, color);
}

void DrawBoxRotatedY(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle)
{
    glm::mat4 m(1); m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(angle), { 0,1,0 }); m = glm::scale(m, scale);
    _SendAndDraw(VAO, mL, cL, uTL, tex, m, color);
}

// ============================================================
//  OBJETO 1 — STAND MODULAR
// ============================================================
void DrawStand(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    GLuint tPVC, GLuint tMetal, glm::vec3 pos, float rotY)
{
    glm::vec3 cAlum(0.75f, 0.75f, 0.78f), cPVC_(0.96f, 0.96f, 0.96f);
    glm::vec3 cAzul(0.05f, 0.28f, 0.60f), cMostr(0.88f, 0.88f, 0.88f);

    auto DB = [&](GLuint tex, glm::vec3 lp, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 par(1); par = glm::translate(par, pos);
        par = glm::rotate(par, glm::radians(rotY), { 0,1,0 });
        glm::mat4 ch(1); ch = glm::translate(ch, lp); ch = glm::scale(ch, sc);
        _SendAndDraw(VAO, mL, cL, uTL, tex, par * ch, col);
        };

    float pw = 0.08f, ph = 2.6f;
    DB(tMetal, { -1.46f,ph / 2,-1.46f }, { pw,ph,pw }, cAlum);
    DB(tMetal, { 1.46f,ph / 2,-1.46f }, { pw,ph,pw }, cAlum);
    DB(tMetal, { -1.46f,ph / 2, 1.46f }, { pw,ph,pw }, cAlum);
    DB(tMetal, { 1.46f,ph / 2, 1.46f }, { pw,ph,pw }, cAlum);
    DB(tMetal, { 0,ph - 0.04f,-1.46f }, { 3,pw,pw }, cAlum);
    DB(tMetal, { 0,ph - 0.04f, 1.46f }, { 3,pw,pw }, cAlum);
    DB(tMetal, { -1.46f,ph - 0.04f,0 }, { pw,pw,3 }, cAlum);
    DB(tMetal, { 1.46f,ph - 0.04f,0 }, { pw,pw,3 }, cAlum);
    DB(tPVC, { 0,1.3f,1.44f }, { 2.92f,2.56f,0.04f }, cPVC_);
    DB(tPVC, { -1.44f,1.3f,0 }, { 0.04f,2.56f,2.92f }, cPVC_);
    DB(tPVC, { 1.44f,1.3f,.5f }, { 0.04f,2.56f,1.9f }, cPVC_);
    DB(0, { 0,2.2f,1.43f }, { 2.90f,0.45f,0.06f }, cAzul);
    DB(tPVC, { 0,1.05f,-1.10f }, { 2.10f,0.10f,0.60f }, cMostr);
    DB(tPVC, { 0,0.55f,-0.82f }, { 2.10f,1.00f,0.06f }, cPVC_);
    DB(tPVC, { 0,0.30f,0.80f }, { 1.50f,0.60f,0.80f }, { 0.85f,0.85f,0.85f });
}

// ============================================================
//  OBJETO 2 — SILLA
// ============================================================
void DrawChair(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    GLuint tWood, glm::vec3 pos, float rotY)
{
    glm::vec3 cLeg(0.30f, 0.20f, 0.10f), cSeat(0.20f, 0.45f, 0.75f), cBack(0.18f, 0.40f, 0.70f);

    auto DB = [&](GLuint tex, glm::vec3 lp, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 par(1); par = glm::translate(par, pos);
        par = glm::rotate(par, glm::radians(rotY), { 0,1,0 });
        glm::mat4 ch(1); ch = glm::translate(ch, lp); ch = glm::scale(ch, sc);
        _SendAndDraw(VAO, mL, cL, uTL, tex, par * ch, col);
        };

    DB(tWood, { -0.22f,0.23f,-0.22f }, { 0.06f,0.46f,0.06f }, cLeg);
    DB(tWood, { 0.22f,0.23f,-0.22f }, { 0.06f,0.46f,0.06f }, cLeg);
    DB(tWood, { -0.22f,0.23f, 0.22f }, { 0.06f,0.46f,0.06f }, cLeg);
    DB(tWood, { 0.22f,0.23f, 0.22f }, { 0.06f,0.46f,0.06f }, cLeg);
    DB(0, { 0,0.49f,0 }, { 0.52f,0.08f,0.52f }, cSeat);

    glm::mat4 par(1); par = glm::translate(par, pos);
    par = glm::rotate(par, glm::radians(rotY), { 0,1,0 });
    glm::mat4 ch(1); ch = glm::translate(ch, { 0,0.90f,0.26f });
    ch = glm::rotate(ch, glm::radians(-8.0f), { 1,0,0 });
    ch = glm::scale(ch, { 0.50f,0.70f,0.07f });
    _SendAndDraw(VAO, mL, cL, uTL, 0, par * ch, cBack);

    DB(tWood, { 0,1.26f,0.27f }, { 0.50f,0.06f,0.06f }, cLeg);
}

// ============================================================
//  OBJETO 3 — PORTAFOLLETOS
// ============================================================
void DrawBrochure(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos)
{
    glm::vec3 cPost(0.80f, 0.80f, 0.82f), cTray(0.92f, 0.92f, 0.92f), cPaper(0.98f, 0.95f, 0.88f);
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 0.85f, 0), { 0.06f,1.70f,0.06f }, cPost);
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 0.05f, 0), { 0.60f,0.06f,0.10f }, cPost);
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 0.05f, 0), { 0.10f,0.06f,0.60f }, cPost);
    float niv[4] = { 0.45f,0.80f,1.15f,1.50f };
    for (int n = 0; n < 4; n++) {
        float ny = pos.y + niv[n];
        DrawBox(VAO, s, mL, cL, uTL, 0, { pos.x + 0.22f,ny,pos.z }, { 0.38f,0.04f,0.12f }, cTray);
        DrawBox(VAO, s, mL, cL, uTL, 0, { pos.x - 0.22f,ny,pos.z }, { 0.38f,0.04f,0.12f }, cTray);
        DrawBox(VAO, s, mL, cL, uTL, 0, { pos.x,ny,pos.z + 0.22f }, { 0.12f,0.04f,0.38f }, cTray);
        DrawBox(VAO, s, mL, cL, uTL, 0, { pos.x,ny,pos.z - 0.22f }, { 0.12f,0.04f,0.38f }, cTray);
        DrawBox(VAO, s, mL, cL, uTL, 0, { pos.x + 0.22f,ny + 0.05f,pos.z }, { 0.32f,0.06f,0.10f }, cPaper);
        DrawBox(VAO, s, mL, cL, uTL, 0, { pos.x - 0.22f,ny + 0.05f,pos.z }, { 0.32f,0.06f,0.10f }, cPaper);
    }
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 1.73f, 0), { 0.12f,0.06f,0.12f }, cPost);
}

// ============================================================
//  OBJETO 4 — BANDERA CON ONDEO (animacion simple basada en sin)
// ============================================================
void DrawFlag(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float time)
{
    glm::vec3 cPole(0.55f, 0.55f, 0.57f), cBlue(0.05f, 0.28f, 0.60f), cGold(0.85f, 0.65f, 0.13f);
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 1.5f, 0), { 0.07f,3,0.07f }, cPole);
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 0.1f, 0), { 0.30f,0.20f,0.30f }, cPole);

    float baseY = pos.y + 2.40f, baseX = pos.x + 0.035f;
    for (int i = 0; i < 6; i++) {
        float ph = i * 0.55f, amp = 0.022f * (i + 1);
        float wZ = amp * sinf(time * 2.8f + ph);
        float cx = baseX + 0.20f * (i + 0.5f);
        DrawBox(VAO, s, mL, cL, uTL, 0, { cx,baseY + 0.80f * 0.30f,pos.z + wZ }, { 0.20f,0.80f * 0.60f,0.025f }, cBlue);
        DrawBox(VAO, s, mL, cL, uTL, 0, { cx,baseY - 0.80f * 0.20f,pos.z + wZ * 0.85f }, { 0.20f,0.80f * 0.40f,0.025f }, cGold);
    }
}

// ============================================================
//  OBJETO 5 — CELOSIA NARANJA/DORADA
//  Reja vertical de láminas, fiel a foto 6
//  Marco de madera oscura + 12 láminas naranjas verticales
// ============================================================
void DrawLattice(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    GLuint tWood, glm::vec3 bp)
{
    glm::vec3 cOr(0.88f, 0.52f, 0.06f);  // naranja dorado intenso (foto 6)
    glm::vec3 cFr(0.45f, 0.25f, 0.05f);  // marco madera oscura
    int   n = 12; float sw = 0.10f, sh = 7.20f, sd = 0.10f, gap = 0.38f;
    float totalW = n * gap + sw;

    // Marco horizontal superior
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(totalW / 2.0f - gap / 2.0f, sh + 0.12f, 0),
        glm::vec3(totalW + sw + 0.2f, 0.18f, sd + 0.06f), cFr);
    // Marco horizontal inferior
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(totalW / 2.0f - gap / 2.0f, 0.09f, 0),
        glm::vec3(totalW + sw + 0.2f, 0.18f, sd + 0.06f), cFr);
    // Marco vertical izquierdo
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(-0.12f, sh / 2.0f, 0),
        glm::vec3(0.16f, sh + 0.3f, sd + 0.06f), cFr);
    // Marco vertical derecho
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(totalW + 0.10f, sh / 2.0f, 0),
        glm::vec3(0.16f, sh + 0.3f, sd + 0.06f), cFr);

    // Láminas naranjas verticales
    for (int i = 0; i < n; i++) {
        float xOff = i * gap;
        float zOff = (i % 2 == 0) ? 0.0f : 0.04f; // leve zigzag
        DrawBox(VAO, s, mL, cL, uTL, 0,
            bp + glm::vec3(xOff, sh / 2.0f, zOff),
            glm::vec3(sw, sh, sd), cOr);
    }
}

// ============================================================
//  MURAL GEOMETRICO DEL LOBBY
// ============================================================
// ============================================================
//  MURAL GEOMETRICO DEL LOBBY — fiel a la imagen de referencia
//
//  El mural real es abstraccion geometrica con triangulos,
//  rombos y trapezoides en perspectiva. Paleta dominante:
//  azul marino, verde oscuro, gris, beige/arena, naranja
//  dorado, verde lima, morado y rojo ladrillo.
//
//  Tecnica: cajas delgadas (profundidad 0.18) con rotacion
//  en Z para simular las diagonales del mural. Capas de atras
//  hacia adelante para lograr el efecto de solapamiento.
// ============================================================
void DrawMural(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 bp)
{
    // Colores extraidos de la fotografia del mural real
    glm::vec3 cFondo = { 0.12f, 0.14f, 0.18f }; // fondo casi negro
    glm::vec3 cAzulM = { 0.08f, 0.14f, 0.38f }; // azul marino (dominante)
    glm::vec3 cAzulO = { 0.06f, 0.10f, 0.28f }; // azul marino oscuro
    glm::vec3 cVerdeO = { 0.10f, 0.25f, 0.20f }; // verde oscuro
    glm::vec3 cVerdeA = { 0.15f, 0.35f, 0.30f }; // verde azulado
    glm::vec3 cGrisM = { 0.32f, 0.35f, 0.38f }; // gris medio
    glm::vec3 cGrisC = { 0.55f, 0.56f, 0.52f }; // gris claro
    glm::vec3 cBeige = { 0.70f, 0.65f, 0.52f }; // beige/arena calido
    glm::vec3 cOro = { 0.80f, 0.55f, 0.08f }; // naranja dorado
    glm::vec3 cLima = { 0.18f, 0.58f, 0.15f }; // verde lima brillante
    glm::vec3 cMorado = { 0.35f, 0.18f, 0.48f }; // morado
    glm::vec3 cRojo = { 0.60f, 0.12f, 0.10f }; // rojo ladrillo
    glm::vec3 cAzulC = { 0.20f, 0.42f, 0.68f }; // azul claro medio
    glm::vec3 cTeal = { 0.08f, 0.35f, 0.40f }; // verde azulado teal

    // Helper: dibuja una caja delgada rotada en Z centrada en (bp + offset)
    auto T = [&](float ox, float oy, float w, float h, float rotZ, glm::vec3 col) {
        glm::mat4 m(1);
        m = glm::translate(m, bp + glm::vec3(ox, oy, 0.0f));
        m = glm::rotate(m, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(w, h, 0.18f));
        _SendAndDraw(VAO, mL, cL, uTL, 0, m, col);
        };

    // === CAPA 1: Fondo base que cubre toda la pared ===
    T(5.5f, 2.6f, 12.5f, 5.5f, 0.0f, cFondo);

    // === CAPA 2: Grandes bloques de color (estructura principal) ===

    // Bloque azul marino grande — domina el centro y lado izquierdo
    T(2.5f, 2.5f, 5.0f, 5.0f, 0.0f, cAzulO);
    T(6.5f, 2.5f, 4.5f, 5.0f, 0.0f, cAzulM);

    // Verde oscuro lado izquierdo y esquina superior
    T(0.8f, 3.0f, 2.2f, 4.0f, 0.0f, cVerdeO);
    T(10.5f, 3.5f, 2.5f, 3.5f, 0.0f, cVerdeO);

    // Gris medio — bloques intermedios
    T(4.5f, 1.5f, 3.5f, 2.2f, 0.0f, cGrisM);
    T(9.2f, 2.2f, 2.2f, 3.0f, 0.0f, cGrisM);

    // === CAPA 3: Formas diagonales grandes (triangulos simulados) ===

    // Gran triangulo beige diagonal hacia la derecha (caracteristico del mural)
    T(5.2f, 2.2f, 4.0f, 3.5f, -22.0f, cBeige);

    // Triangulo azul marino inclinado (estrella central del mural)
    T(3.8f, 3.0f, 3.2f, 4.5f, 18.0f, cAzulM);
    T(7.0f, 2.8f, 2.8f, 4.0f, -15.0f, cAzulO);

    // Cuña verde oscuro diagonal izquierda
    T(1.5f, 2.0f, 2.8f, 3.8f, 20.0f, cVerdeO);

    // Rombo gris claro (forma central con vertice hacia arriba)
    T(5.8f, 3.5f, 2.5f, 2.5f, 35.0f, cGrisC);

    // Bloque teal (verde azulado) — aparece en el sector central inferior
    T(4.2f, 1.2f, 3.0f, 2.0f, -10.0f, cTeal);

    // === CAPA 4: Formas medianas que dan profundidad ===

    // Beige/arena — trapezoides inclinados (muy caracteristicos del mural)
    T(3.0f, 3.8f, 2.2f, 1.8f, 28.0f, cBeige);
    T(8.0f, 2.0f, 2.0f, 2.5f, -30.0f, cBeige);

    // Verde azulado — formas en perspectiva
    T(1.2f, 1.8f, 1.8f, 3.0f, 25.0f, cVerdeA);
    T(9.5f, 3.0f, 1.5f, 2.8f, -18.0f, cVerdeA);

    // Azul claro — acentos medios
    T(2.2f, 1.5f, 1.8f, 2.8f, 15.0f, cAzulC);
    T(7.8f, 3.8f, 1.5f, 2.0f, -22.0f, cAzulC);

    // Gris medio — cuñas secundarias
    T(6.5f, 1.8f, 2.0f, 1.8f, 40.0f, cGrisM);
    T(10.0f, 1.5f, 1.8f, 2.5f, 12.0f, cGrisM);

    // === CAPA 5: Morado y rojo (esquina derecha del mural real) ===
    T(10.8f, 2.5f, 2.0f, 3.5f, -10.0f, cMorado);
    T(11.2f, 1.5f, 1.5f, 2.8f, 8.0f, cRojo);
    T(10.5f, 4.5f, 1.2f, 1.5f, 20.0f, cMorado);

    // === CAPA 6: Destellos de naranja dorado (lineas horizontales) ===
    // El mural tiene destellos de luz horizontales muy caracteristicos
    T(2.8f, 2.8f, 3.5f, 0.22f, -6.0f, cOro);
    T(6.5f, 3.5f, 2.5f, 0.18f, 8.0f, cOro);
    T(9.0f, 2.6f, 2.0f, 0.16f, -4.0f, cOro);
    T(4.5f, 4.8f, 1.8f, 0.15f, 5.0f, cOro);

    // === CAPA 7: Verde lima (acentos brillantes puntuales) ===
    T(0.5f, 2.2f, 1.0f, 2.0f, -18.0f, cLima);
    T(8.8f, 4.2f, 0.8f, 1.5f, 25.0f, cLima);
    T(5.5f, 0.8f, 0.6f, 1.0f, 42.0f, cLima);

    // === CAPA 8: Micro-detalles (rombos y puntos de color) ===
    T(4.0f, 1.0f, 0.9f, 1.4f, 48.0f, cBeige);  // rombo beige
    T(7.2f, 1.0f, 0.7f, 1.2f, -45.0f, cAzulM);  // cuña azul pequeña
    T(2.0f, 4.8f, 1.0f, 0.8f, 30.0f, cGrisC);  // detalle gris claro
    T(6.0f, 5.0f, 0.8f, 0.9f, 55.0f, cAzulC);  // rombo azul claro
    T(3.5f, 0.8f, 0.9f, 0.7f, 20.0f, cTeal);   // punto teal
    T(9.5f, 5.0f, 0.7f, 1.0f, -30.0f, cRojo);   // acento rojo pequeno
}

// ============================================================
//  OBJETO 6 — DINOSAURIO ANIMADO (jerarquico)
// ============================================================
void DrawDino(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float time)
{
    glm::vec3 cBody(0.30f, 0.72f, 0.15f), cDark(0.20f, 0.52f, 0.08f);
    glm::vec3 cBelly(0.85f, 0.95f, 0.70f), cEye(0.05f, 0.05f, 0.05f);
    float cy = time * (2.0f * 3.14159f / 2.0f);
    float bobY = sinf(cy) * 0.10f, tZ = sinf(cy) * 15.0f;
    float aL = sinf(cy) * 30.0f, aR = sinf(cy + 3.14159f) * 30.0f;
    float hB = sinf(cy * 2) * 5.0f, lL = sinf(cy) * 20.0f, lR = sinf(cy + 3.14159f) * 20.0f;
    glm::vec3 tp = pos + glm::vec3(0, 1.20f + bobY, 0);

    auto P = [&](glm::mat4 m, glm::vec3 col) { _SendAndDraw(VAO, mL, cL, uTL, 0, m, col); };

    {
        glm::mat4 m(1); m = glm::translate(m, tp);
        m = glm::rotate(m, glm::radians(tZ), { 0,0,1 }); m = glm::scale(m, { .80f,.90f,.70f }); P(m, cBody);
    }
    {
        glm::mat4 m(1); m = glm::translate(m, tp + glm::vec3(0, 0, .34f));
        m = glm::rotate(m, glm::radians(tZ), { 0,0,1 }); m = glm::scale(m, { .55f,.70f,.06f }); P(m, cBelly);
    }
    glm::vec3 np = tp + glm::vec3(0, .52f, .08f);
    {
        glm::mat4 m(1); m = glm::translate(m, np); m = glm::rotate(m, glm::radians(tZ * .5f), { 0,0,1 });
        m = glm::scale(m, { .35f,.45f,.35f }); P(m, cBody);
    }
    glm::vec3 hp = np + glm::vec3(0, .45f, 0);
    {
        glm::mat4 m(1); m = glm::translate(m, hp); m = glm::rotate(m, glm::radians(hB), { 1,0,0 });
        m = glm::scale(m, { .52f,.42f,.48f }); P(m, cBody);
    }
    {
        glm::mat4 m(1); m = glm::translate(m, hp + glm::vec3(0, -.06f, .26f));
        m = glm::rotate(m, glm::radians(hB), { 1,0,0 }); m = glm::scale(m, { .38f,.20f,.22f }); P(m, cDark);
    }
    for (int sd = -1; sd <= 1; sd += 2) {
        glm::mat4 m(1);
        m = glm::translate(m, hp + glm::vec3(sd * .18f, .10f, .22f));
        m = glm::rotate(m, glm::radians(hB), { 1,0,0 }); m = glm::scale(m, { .09f,.09f,.06f }); P(m, cEye);
    }
    {
        glm::vec3 sh = tp + glm::vec3(-.50f, .20f, 0); glm::mat4 m(1); m = glm::translate(m, sh);
        m = glm::rotate(m, glm::radians(tZ), { 0,0,1 }); m = glm::rotate(m, glm::radians(aL), { 0,0,1 });
        m = glm::translate(m, { -.18f,-.18f,0 }); m = glm::scale(m, { .18f,.42f,.18f }); P(m, cBody);
    }
    {
        glm::vec3 sh = tp + glm::vec3(.50f, .20f, 0); glm::mat4 m(1); m = glm::translate(m, sh);
        m = glm::rotate(m, glm::radians(tZ), { 0,0,1 }); m = glm::rotate(m, glm::radians(aR), { 0,0,1 });
        m = glm::translate(m, { .18f,-.18f,0 }); m = glm::scale(m, { .18f,.42f,.18f }); P(m, cBody);
    }
    {
        glm::vec3 hip = tp + glm::vec3(-.28f, -.50f, 0); glm::mat4 m(1); m = glm::translate(m, hip);
        m = glm::rotate(m, glm::radians(lL), { 1,0,0 }); m = glm::translate(m, { 0,-.30f,0 });
        m = glm::scale(m, { .22f,.60f,.22f }); P(m, cDark);
    }
    {
        glm::vec3 hip = tp + glm::vec3(.28f, -.50f, 0); glm::mat4 m(1); m = glm::translate(m, hip);
        m = glm::rotate(m, glm::radians(lR), { 1,0,0 }); m = glm::translate(m, { 0,-.30f,0 });
        m = glm::scale(m, { .22f,.60f,.22f }); P(m, cDark);
    }
    {
        glm::mat4 m(1); m = glm::translate(m, tp + glm::vec3(0, -.10f, -.45f));
        m = glm::rotate(m, glm::radians(tZ * .8f), { 0,0,1 }); m = glm::rotate(m, glm::radians(-30.0f), { 1,0,0 });
        m = glm::scale(m, { .22f,.65f,.22f }); P(m, cDark);
    }
}

// ============================================================
//  ANIMACION SIMPLE 1 — PUERTAS (pivote en borde interior)
// ============================================================
void DrawDoor(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float openAngle)
{
    glm::vec3 cFr(0.60f, 0.60f, 0.62f), cGl(0.72f, 0.88f, 0.95f), cHa(0.85f, 0.75f, 0.30f);
    DrawBox(VAO, s, mL, cL, uTL, 0, pos + glm::vec3(0, 1.30f, 0), { 3,2.60f,.10f }, cFr);

    auto Leaf = [&](float sign) {
        glm::vec3 piv = pos + glm::vec3(sign * 0.70f, 1.30f, 0);
        {
            glm::mat4 m(1); m = glm::translate(m, piv);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * 0.70f,0,0 }); m = glm::scale(m, { 1.38f,2.48f,.06f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, cGl);
        }
        {
            glm::mat4 m(1); m = glm::translate(m, piv);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * 0.28f,0,.06f }); m = glm::scale(m, { .06f,.22f,.06f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, cHa);
        }
        };
    Leaf(-1); Leaf(1);
}

// ============================================================
//  ANIMACION SIMPLE 2 — SEÑALETICA PULSANTE (BILLBOARD)
// ============================================================
void DrawSignage(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float time)
{
    float pulse = 1.0f + 0.15f * sinf(time * 1.5f);
    float dx = camera.GetPosition().x - pos.x, dz = camera.GetPosition().z - pos.z;
    float ang = glm::degrees(atan2f(dx, dz));

    auto Sg = [&](glm::vec3 off, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 m(1); m = glm::translate(m, pos + off);
        m = glm::rotate(m, glm::radians(ang), { 0,1,0 });
        m = glm::scale(m, sc * pulse); _SendAndDraw(VAO, mL, cL, uTL, 0, m, col);
        };
    Sg({ 0,0,0 }, { .38f,.38f,.04f }, { 0.06f,0.45f,0.15f });
    Sg({ 0,0,0 }, { .32f,.32f,.06f }, { 0.10f,0.72f,0.25f });
    Sg({ 0,-.04f,0 }, { .08f,.14f,.08f }, { 0.95f,0.95f,0.95f });
    Sg({ 0, .10f,0 }, { .07f,.07f,.07f }, { 0.95f,0.95f,0.95f });
}

// ============================================================
//  ANIMACION SIMPLE 3 — CONFETI (particulas)
// ============================================================
void DrawConfetti(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, float timer)
{
    float ceil = 8.0f, life = 4.0f;
    for (int i = 0; i < NUM_CONFETTI; i++) {
        const ConfettiParticle& p = confParticles[i];
        float rel = fmodf(timer + p.phase, life);
        float y = ceil - p.speed * rel * 1.8f; if (y < 0)continue;
        float sc = 0.12f, lr = rel / life;
        if (lr > 0.80f) sc *= (1 - (lr - 0.80f) / 0.20f);
        if (sc < 0.005f)continue;
        glm::mat4 m(1); m = glm::translate(m, { p.x,y,p.z });
        m = glm::rotate(m, glm::radians(p.rotSpeed * rel), { 0,0,1 });
        m = glm::scale(m, { sc,sc * .5f,sc * .1f });
        _SendAndDraw(VAO, mL, cL, uTL, 0, m, { p.colorR,p.colorG,p.colorB });
    }
}

// ============================================================
//  ANIMACION COMPLEJA 2 — AGENTES CAMINANTES EN BEZIER
//  5 agentes con velocidad variable (aceleran/frenan con sin).
//  Orientacion calculada desde la tangente de la curva.
//  Piernas oscilan con el paso (animacion de caminar).
// ============================================================
void DrawAgents(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, float time)
{
    glm::vec3 cols[NUM_AGENTS] = {
        {.20f,.40f,.80f},{.80f,.30f,.20f},{.20f,.65f,.25f},
        {.75f,.60f,.15f},{.50f,.20f,.65f}
    };
    for (int i = 0; i < NUM_AGENTS; i++) {
        const BezierPath& bp = agentPaths[i];
        // Velocidad variable: el agente acelera en rectas, frena en curvas
        float raw = fmodf(time * bp.speed + bp.phase, 1.0f);
        float t = fmodf(raw + 0.3f * sinf(raw * 6.28f) * 0.1f, 1.0f);
        if (t < 0)t += 1.0f;

        glm::vec3 pos = BezierEval(bp, t); pos.y = 0;
        // Limites estrictos del pasillo — nunca salen del area visible
        // X: entre las dos filas de columnas (-9 a +9)
        // Z: dentro del largo del pasillo (-8 a +8)
        pos.x = glm::clamp(pos.x, -8.5f, 8.5f);
        pos.z = glm::clamp(pos.z, -7.5f, 8.0f);
        glm::vec3 next = BezierEval(bp, fminf(t + 0.01f, 1.0f));
        glm::vec3 dir = next - pos;
        float yaw = 0; if (glm::length(dir) > 0.001f) yaw = glm::degrees(atan2f(dir.x, dir.z));

        glm::vec3 col = cols[i];
        float leg = sinf(time * 6 + bp.phase * 6.28f) * 15.0f;

        // Cuerpo
        {
            glm::mat4 m(1); m = glm::translate(m, pos + glm::vec3(0, .55f, 0));
            m = glm::rotate(m, glm::radians(yaw), { 0,1,0 }); m = glm::scale(m, { .35f,.70f,.30f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, col);
        }
        // Cabeza
        {
            glm::mat4 m(1); m = glm::translate(m, pos + glm::vec3(0, 1.05f, 0));
            m = glm::rotate(m, glm::radians(yaw), { 0,1,0 }); m = glm::scale(m, { .28f,.28f,.28f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, { .88f,.72f,.58f });
        }
        // Piernas (oscilacion alternada)
        for (int sd = -1; sd <= 1; sd += 2) {
            glm::vec3 hip = pos + glm::vec3(sd * .10f, .28f, 0);
            glm::mat4 m(1); m = glm::translate(m, hip);
            m = glm::rotate(m, glm::radians(yaw), { 0,1,0 });
            m = glm::rotate(m, glm::radians((float)sd * leg), { 1,0,0 });
            m = glm::translate(m, { 0,-.20f,0 }); m = glm::scale(m, { .12f,.38f,.12f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, { col.r * .6f,col.g * .6f,col.b * .6f });
        }
    }
}

// ============================================================
//  CALLBACKS DE ENTRADA
// ============================================================
void DoMovement()
{
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_E && action == GLFW_PRESS) { confettiActive = true; confettiTimer = 0; }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) doorOpen = !doorOpen;
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) { lastX = (float)xPos; lastY = (float)yPos; firstMouse = false; }
    float xOff = (float)xPos - lastX, yOff = lastY - (float)yPos;
    lastX = (float)xPos; lastY = (float)yPos;
    camera.ProcessMouseMovement(xOff, yOff);
}

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll((float)yOffset);
}
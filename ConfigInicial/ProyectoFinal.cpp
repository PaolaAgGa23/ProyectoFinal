// ================================================================
//  ProyectoFinal.cpp
//  Equipo 01 — PYI Technologies
//  Lobby Auditorio Javier Barros Sierra — Facultad de Ingenieria UNAM
//  Laboratorio de Computacion Grafica e Interaccion Humano-Computadora
//  Mayo 2026
// ================================================================

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
void DrawStandPlacing(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tPVC, GLuint tMetal, glm::vec3 pos, float rotY, float timer, float duration);
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
void DrawTotem(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, float time, glm::vec3 pos);
void DrawWaterStation(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, glm::vec3 pos);

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

// --- Animacion de colocacion de stand ---
bool  standPlacing = false;
float standTimer = 0.0f;
const float STAND_ANIM_DUR = 2.4f;

// ============================================================
//  CONTROL DE ILUMINACION
// ============================================================
float lightAmbient = 0.75f;
float lightDiffuse = 0.35f;
glm::vec3 lightPos = glm::vec3(0.0f, 15.0f, 5.0f);
int   lightMode = 1;

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
// Cada agente tiene su propio carril en X Y su propio phase.
// El phase escalonado (0.0, 0.2, 0.4, 0.6, 0.8) garantiza que
// cuando un agente esta en t=0 (inicio de ruta), el siguiente
// ya lleva 20% del recorrido — nunca coinciden en el mismo punto.
// Las rutas van en carriles paralelos separados para evitar cruces.
BezierPath agentPaths[NUM_AGENTS] = {
    // Carril -6.5 (izquierdo), norte→sur
    {{ -6.5f,0, 7.0f},{ -6.2f,0, 2.5f},{ -6.8f,0,-2.0f},{ -6.5f,0,-6.5f}, 0.13f, 0.00f},
    // Carril -3.0 (centro-izq), sur→norte
    {{ -3.0f,0,-6.5f},{ -3.3f,0,-1.5f},{ -2.7f,0, 3.0f},{ -3.0f,0, 7.0f}, 0.15f, 0.20f},
    // Carril  0.5 (central), norte→sur
    {{  0.5f,0, 7.0f},{  0.8f,0, 2.0f},{  0.2f,0,-2.5f},{  0.5f,0,-6.5f}, 0.12f, 0.40f},
    // Carril  3.5 (centro-der), sur→norte
    {{  3.5f,0,-6.5f},{  3.2f,0,-1.0f},{  3.8f,0, 3.5f},{  3.5f,0, 7.0f}, 0.16f, 0.60f},
    // Carril  6.5 (derecho), norte→sur
    {{  6.5f,0, 7.0f},{  6.2f,0, 2.0f},{  6.8f,0,-2.5f},{  6.5f,0,-6.5f}, 0.14f, 0.80f},
};

// ============================================================
//  EASING — para animacion de colocacion de stand
// ============================================================
static float SmoothStep(float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static float ElasticOut(float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    if (t >= 1.0f) return 1.0f;
    float base = SmoothStep(t);
    float bounce = sinf(t * 3.14159f * 3.5f) * (1.0f - t) * 0.18f;
    return base + bounce;
}

// ============================================================
//  CUBO CON NORMALES Y UVS
// ============================================================
float vertices[] = {
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

    Shader shaderColor("Shader/core.vs", "Shader/core.frag");
    Shader shaderModel("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader shaderUnlit("Shader/unlit.vs", "Shader/unlit.frag");

    Model person((char*)"Models/person.obj");

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    GLuint tFloor = LoadTexture("images/floor_tile.jpg");
    GLuint tWall = LoadTexture("images/wall_concrete.jpg");
    GLuint tStone = LoadTexture("images/stone_volcanic.jpg");
    GLuint tWood = LoadTexture("images/wood_oak.jpg");
    GLuint tMetal = LoadTexture("images/metal_brushed.jpg");
    GLuint tPVC = LoadTexture("images/pvc_white.jpg");
    GLuint tConcr = LoadTexture("images/concrete_grey.jpg");
    GLuint tMural = LoadTexture("images/mural.jpg");
    GLuint tCactus = LoadTexture("images/cactus.jpg");
    GLuint tColumna = LoadTexture("images/columna.jpg");

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

        // --- Confeti ---
        if (confettiActive) {
            confettiTimer += deltaTime;
            if (confettiTimer > 5.0f) { confettiActive = false; confettiTimer = 0.0f; }
        }

        // --- Puertas ---
        float targetAngle = doorOpen ? 90.0f : 0.0f;
        doorAngle += (targetAngle - doorAngle) * (1.0f - expf(-deltaTime / 0.12f));

        // --- Stand placing ---
        if (standPlacing) {
            standTimer += deltaTime;
            if (standTimer > STAND_ANIM_DUR + 0.8f) {
                standPlacing = false;
                standTimer = 0.0f;
            }
        }

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();

        shaderColor.Use();

        GLint modelLoc = glGetUniformLocation(shaderColor.Program, "model");
        GLint viewLoc = glGetUniformLocation(shaderColor.Program, "view");
        GLint projLoc = glGetUniformLocation(shaderColor.Program, "projection");
        GLint colorLoc = glGetUniformLocation(shaderColor.Program, "color");
        GLint useTexLoc = glGetUniformLocation(shaderColor.Program, "useTexture");
        GLint unlitLoc = glGetUniformLocation(shaderColor.Program, "unlit");
        GLint lightPLoc = glGetUniformLocation(shaderColor.Program, "lightPos");
        GLint lightCLoc = glGetUniformLocation(shaderColor.Program, "lightColor");
        GLint viewPLoc = glGetUniformLocation(shaderColor.Program, "viewPos");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(unlitLoc, 0);
        glUniform1i(useTexLoc, 0);

        glm::vec3 lightColor;
        switch (lightMode) {
        case 1: lightColor = glm::vec3(1.00f, 0.98f, 0.92f); break;
        case 2: lightColor = glm::vec3(1.00f, 0.75f, 0.40f); break;
        case 3: lightColor = glm::vec3(0.55f, 0.70f, 1.00f); break;
        case 4: lightColor = glm::vec3(1.00f, 0.92f, 0.70f); break;
        default: lightColor = glm::vec3(1.0f); break;
        }

        glUniform3fv(lightPLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(lightCLoc, 1, glm::value_ptr(lightColor));
        glUniform3fv(viewPLoc, 1, glm::value_ptr(camera.GetPosition()));
        glUniform1f(glGetUniformLocation(shaderColor.Program, "ambientStr"), lightAmbient);
        glUniform1f(glGetUniformLocation(shaderColor.Program, "diffuseStr"), lightDiffuse);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderColor.Program, "texture1"), 0);

        // ====================================================
        //  PISO
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tFloor,
            glm::vec3(0, -0.15f, 0), glm::vec3(30.0f, 0.3f, 22.0f),
            glm::vec3(0.15f, 0.15f, 0.16f));
        for (int li = -4; li <= 4; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(0, 0.01f, li * 2.5f), glm::vec3(28.0f, 0.025f, 0.18f),
                glm::vec3(0.70f, 0.65f, 0.48f));
        for (int li = -5; li <= 5; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(li * 2.5f, 0.01f, 0), glm::vec3(0.18f, 0.025f, 22.0f),
                glm::vec3(0.70f, 0.65f, 0.48f));

        // ====================================================
        //  COLUMNAS
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
                    _SendAndDraw(VAO, modelLoc, colorLoc, useTexLoc, tColumna, m, cCol);
                }
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tColumna,
                    { cx,colH / 2.0f,cz }, { r * 1.35f,colH,r * 1.35f }, cCol);
                };
            for (int ci = 0; ci < 6; ci++) DrawColumn(-10.0f, -8.0f + ci * 3.5f);
            for (int ci = 0; ci < 6; ci++) DrawColumn(10.0f, -8.0f + ci * 3.5f);
            DrawColumn(-6.5f, 0.0f);
            DrawColumn(6.5f, 0.0f);
        }

        // ====================================================
        //  TECHO Y DUCTOS
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tConcr,
            glm::vec3(0, 8.15f, 0), glm::vec3(28.0f, 0.3f, 22.0f),
            glm::vec3(0.88f, 0.88f, 0.86f));
        for (int di = 0; di < 3; di++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(-5.0f + di * 4.0f, 7.85f, 0), { 0.18f,0.18f,20.0f },
                glm::vec3(0.50f, 0.50f, 0.52f));
        for (int li = 0; li < 4; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(-6.0f + li * 4.0f, 7.95f, -2.0f), { 1.5f,0.08f,0.5f },
                glm::vec3(0.98f, 0.98f, 0.92f));

        // ====================================================
        //  ESCALERA CON BARANDAL AMARILLO
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
            }
            for (int i = 0; i < nSteps; i += 2) {
                float y = startY + (i * stepH) + 0.55f;
                float z = startZ - (i * stepD);
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    glm::vec3(stairX - stairW / 2 + 0.15f, startY + (i * stepH) + 0.30f, z),
                    { 0.06f,0.60f,0.06f }, cAmar);
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    glm::vec3(stairX + stairW / 2 - 0.15f, startY + (i * stepH) + 0.30f, z),
                    { 0.06f,0.60f,0.06f }, cAmar);
            }
            for (int i = 0; i < nSteps - 1; i++) {
                float y0 = startY + (i * stepH) + 0.58f, z0 = startZ - (i * stepD);
                float y1 = startY + ((i + 1) * stepH) + 0.58f, z1 = startZ - ((i + 1) * stepD);
                float cy = (y0 + y1) / 2.0f, cz_ = (z0 + z1) / 2.0f;
                float len = sqrtf((y1 - y0) * (y1 - y0) + (z1 - z0) * (z1 - z0));
                float ang = glm::degrees(atan2f(y1 - y0, z0 - z1));
                DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    glm::vec3(stairX - stairW / 2 + 0.15f, cy, cz_), { 0.06f,0.06f,len }, cAmar, ang);
                DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                    glm::vec3(stairX + stairW / 2 - 0.15f, cy, cz_), { 0.06f,0.06f,len }, cAmar, ang);
            }
            float topY = startY + (nSteps * stepH), topZ = startZ - (nSteps * stepD) - 0.20f;
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { stairX,topY - 0.08f,topZ }, { stairW + 1.20f,0.22f,2.00f }, { 0.72f,0.72f,0.70f });
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                { stairX,topY - 0.10f,topZ - 1.55f }, { stairW + 3.00f,0.25f,2.20f }, { 0.68f,0.68f,0.66f });
            DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tConcr,
                { 4.20f,topY + 2.45f,topZ - 2.90f }, { 5.60f,0.28f,6.80f }, { 0.85f,0.85f,0.83f }, -42.0f);
        }

        // ====================================================
        //  MOSTRADOR ROJO CON CACTUS
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.5f, 0.6f, 2.2f), { 3.0f,1.2f,1.5f },
            glm::vec3(0.82f, 0.08f, 0.06f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.5f, 1.26f, 2.2f), { 3.1f,0.10f,1.6f },
            glm::vec3(0.96f, 0.96f, 0.96f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.5f, 1.32f, 2.2f), { 0.35f,0.22f,0.35f },
            glm::vec3(0.42f, 0.22f, 0.10f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.5f, 1.46f, 2.2f), { 0.38f,0.06f,0.38f },
            glm::vec3(0.85f, 0.55f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tCactus,
            glm::vec3(6.5f, 1.95f, 2.2f), { 0.18f,0.85f,0.18f },
            glm::vec3(0.15f, 0.55f, 0.10f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tCactus,
            glm::vec3(6.18f, 1.75f, 2.2f), { 0.30f,0.14f,0.14f },
            glm::vec3(0.15f, 0.52f, 0.10f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tCactus,
            glm::vec3(6.03f, 1.98f, 2.2f), { 0.14f,0.35f,0.14f },
            glm::vec3(0.15f, 0.52f, 0.10f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tCactus,
            glm::vec3(6.82f, 1.75f, 2.2f), { 0.30f,0.14f,0.14f },
            glm::vec3(0.15f, 0.52f, 0.10f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tCactus,
            glm::vec3(6.97f, 1.98f, 2.2f), { 0.14f,0.35f,0.14f },
            glm::vec3(0.15f, 0.52f, 0.10f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.5f, 2.42f, 2.2f), { 0.12f,0.08f,0.12f },
            glm::vec3(0.90f, 0.45f, 0.55f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.03f, 2.20f, 2.2f), { 0.10f,0.07f,0.10f },
            glm::vec3(0.90f, 0.45f, 0.55f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(6.97f, 2.20f, 2.2f), { 0.10f,0.07f,0.10f },
            glm::vec3(0.90f, 0.45f, 0.55f));

        // ====================================================
        //  BUSTO JAVIER BARROS SIERRA
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { 1.8f,0.8f,2.0f }, { 0.9f,1.6f,0.9f }, { 0.55f,0.55f,0.52f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { 1.8f,1.62f,2.0f }, { 1.2f,0.12f,1.2f }, { 0.50f,0.50f,0.48f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMetal,
            { 1.8f,2.15f,2.0f }, { 0.7f,0.55f,0.5f }, { 0.52f,0.50f,0.48f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMetal,
            { 1.8f,2.68f,2.0f }, { 0.48f,0.52f,0.48f }, { 0.54f,0.52f,0.50f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { -0.5f,2.2f,-0.5f }, { 1.8f,0.45f,0.05f }, { 0.08f,0.08f,0.08f });

        // ====================================================
        //  PAREDES DEL LOBBY
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            glm::vec3(-14.8f, 4.0f, 0.0f), { 0.30f,8.5f,22.0f },
            glm::vec3(0.28f, 0.25f, 0.22f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            glm::vec3(14.8f, 4.0f, -4.0f), { 0.30f,8.5f,12.0f },
            glm::vec3(0.88f, 0.88f, 0.88f));

        // ====================================================
        //  TREN UNIVERSITARIO
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 1.2f, 4.0f), { 1.8f,2.0f,8.0f },
            glm::vec3(0.85f, 0.08f, 0.06f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 2.18f, 4.0f), { 1.82f,0.15f,8.02f },
            glm::vec3(0.90f, 0.72f, 0.04f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 2.18f, 4.0f), { 1.6f,0.9f,2.5f },
            glm::vec3(0.85f, 0.08f, 0.06f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 3.06f, 4.0f), { 1.62f,0.10f,2.52f },
            glm::vec3(0.90f, 0.72f, 0.04f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 3.13f, 4.0f), { 1.7f,0.12f,2.6f },
            glm::vec3(0.25f, 0.25f, 0.25f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.08f, 1.4f, 1.8f), { 0.06f,0.55f,1.0f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.08f, 1.4f, 6.2f), { 0.06f,0.55f,1.0f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.08f, 1.4f, 4.0f), { 0.06f,0.55f,0.8f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.08f, 3.0f, 3.2f), { 0.06f,0.35f,0.55f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.08f, 3.0f, 4.8f), { 0.06f,0.35f,0.55f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.07f, 1.4f, 1.8f), { 0.04f,0.65f,1.1f },
            glm::vec3(0.70f, 0.06f, 0.05f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.07f, 1.4f, 6.2f), { 0.04f,0.65f,1.1f },
            glm::vec3(0.70f, 0.06f, 0.05f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 0.38f, 0.3f), { 1.8f,0.15f,1.0f },
            glm::vec3(0.15f, 0.15f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 0.38f, 7.7f), { 1.8f,0.15f,1.0f },
            glm::vec3(0.15f, 0.15f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 0.15f, 0.1f), { 1.4f,0.08f,0.3f },
            glm::vec3(0.15f, 0.15f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, -0.05f, 0.0f), { 1.4f,0.08f,0.3f },
            glm::vec3(0.15f, 0.15f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 0.15f, 7.9f), { 1.4f,0.08f,0.3f },
            glm::vec3(0.15f, 0.15f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, -0.05f, 8.0f), { 1.4f,0.08f,0.3f },
            glm::vec3(0.15f, 0.15f, 0.15f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.2f, 0.5f, 0.2f), { 0.06f,0.8f,0.06f },
            glm::vec3(0.25f, 0.25f, 0.25f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(22.2f, 0.5f, 7.8f), { 0.06f,0.8f,0.06f },
            glm::vec3(0.25f, 0.25f, 0.25f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, 0.18f, 4.0f), { 1.85f,0.22f,8.2f },
            glm::vec3(0.12f, 0.12f, 0.12f));
        for (int wi = 0; wi < 4; wi++) {
            float wz[4] = { 1.2f,2.2f,5.8f,6.8f };
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(23.0f, 0.05f, wz[wi]), { 0.3f,0.45f,0.45f },
                glm::vec3(0.10f, 0.10f, 0.10f));
        }
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, -0.10f, 1.5f), { 0.10f,0.06f,10.0f },
            glm::vec3(0.35f, 0.35f, 0.35f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(23.0f, -0.10f, 6.5f), { 0.10f,0.06f,10.0f },
            glm::vec3(0.35f, 0.35f, 0.35f));
        for (int ti = 0; ti < 6; ti++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(23.0f, -0.12f, 0.5f + ti * 1.5f), { 0.25f,0.06f,6.5f },
                glm::vec3(0.30f, 0.20f, 0.10f));

        // ====================================================
        //  MURO FONDO Y PAREDES NORTE
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            glm::vec3(0.0f, 4.0f, -10.0f), { 30.0f,8.5f,0.30f },
            glm::vec3(0.90f, 0.90f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            glm::vec3(-8.0f, 4.0f, 15.8f), { 12.0f,8.5f,0.35f },
            glm::vec3(0.28f, 0.25f, 0.22f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            glm::vec3(8.0f, 4.0f, 15.8f), { 12.0f,8.5f,0.35f },
            glm::vec3(0.28f, 0.25f, 0.22f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tConcr,
            glm::vec3(0.0f, 6.8f, 15.8f), { 8.0f,2.8f,0.35f },
            glm::vec3(0.72f, 0.72f, 0.70f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            glm::vec3(0.0f, -0.12f, 18.5f), { 30.0f,0.25f,6.0f },
            glm::vec3(0.82f, 0.82f, 0.80f));

        // ====================================================
        //  MARCO DE PUERTAS (madera)
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            glm::vec3(-3.8f, 3.5f, 15.7f), { 0.25f,7.0f,0.25f },
            glm::vec3(0.45f, 0.32f, 0.18f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            glm::vec3(3.8f, 3.5f, 15.7f), { 0.25f,7.0f,0.25f },
            glm::vec3(0.45f, 0.32f, 0.18f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            glm::vec3(0.0f, 3.5f, 15.7f), { 0.20f,7.0f,0.20f },
            glm::vec3(0.45f, 0.32f, 0.18f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            glm::vec3(0.0f, 5.85f, 15.7f), { 7.8f,0.20f,0.20f },
            glm::vec3(0.45f, 0.32f, 0.18f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(-1.9f, 2.9f, 15.75f), { 3.6f,5.6f,0.08f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(1.9f, 2.9f, 15.75f), { 3.6f,5.6f,0.08f },
            glm::vec3(0.75f, 0.85f, 0.90f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(-2.8f, 2.9f, 15.78f), { 0.06f,5.6f,0.06f },
            glm::vec3(0.70f, 0.70f, 0.72f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(-1.0f, 2.9f, 15.78f), { 0.06f,5.6f,0.06f },
            glm::vec3(0.70f, 0.70f, 0.72f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(1.0f, 2.9f, 15.78f), { 0.06f,5.6f,0.06f },
            glm::vec3(0.70f, 0.70f, 0.72f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(2.8f, 2.9f, 15.78f), { 0.06f,5.6f,0.06f },
            glm::vec3(0.70f, 0.70f, 0.72f));
        for (int fi = 0; fi < 3; fi++) {
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(-1.9f, 1.5f + fi * 1.8f, 15.78f), { 3.6f,0.06f,0.06f },
                glm::vec3(0.70f, 0.70f, 0.72f));
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(1.9f, 1.5f + fi * 1.8f, 15.78f), { 3.6f,0.06f,0.06f },
                glm::vec3(0.70f, 0.70f, 0.72f));
        }

        // ====================================================
        //  BUSTO EXTERIOR
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { -5.5f,0.8f,15.0f }, { 0.9f,1.6f,0.9f }, { 0.40f,0.38f,0.35f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            { -5.5f,1.62f,15.0f }, { 1.2f,0.12f,1.2f }, { 0.35f,0.33f,0.30f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMetal,
            { -5.5f,2.15f,15.0f }, { 0.7f,0.55f,0.5f }, { 0.42f,0.40f,0.38f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMetal,
            { -5.5f,2.68f,15.0f }, { 0.48f,0.52f,0.48f }, { 0.44f,0.42f,0.40f });

        DrawSignage(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(4.5f, 2.2f, 15.5f), currentFrame);

        // ====================================================
        //  BOTE DE BASURA
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(13.5f, 0.7f, 2.5f), { 0.9f,1.4f,0.9f },
            glm::vec3(0.85f, 0.08f, 0.06f));
        for (int ri = 0; ri < 8; ri++) {
            float angR = glm::radians(ri * 45.0f);
            float rx = 13.5f + 0.46f * cosf(angR), rz = 2.5f + 0.46f * sinf(angR);
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(rx, 0.7f, rz), { 0.04f,1.4f,0.04f },
                glm::vec3(0.60f, 0.05f, 0.04f));
        }
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(13.5f, 1.35f, 2.5f), { 0.92f,0.08f,0.92f },
            glm::vec3(0.70f, 0.06f, 0.05f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(13.5f, 1.45f, 2.5f), { 0.95f,0.07f,0.95f },
            glm::vec3(0.12f, 0.12f, 0.12f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(13.5f, 1.58f, 2.5f), { 1.0f,0.15f,1.0f },
            glm::vec3(0.65f, 0.65f, 0.67f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(13.5f, 1.68f, 2.5f), { 0.70f,0.08f,0.70f },
            glm::vec3(0.60f, 0.60f, 0.62f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            glm::vec3(13.5f, -0.02f, 2.5f), { 0.85f,0.05f,0.85f },
            glm::vec3(0.12f, 0.12f, 0.12f));

        // ====================================================
        //  ZONA EXTERIOR (techo, piso, columna)
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tConcr,
            glm::vec3(0.0f, 8.15f, 13.0f), { 30.0f,0.3f,8.0f },
            glm::vec3(0.82f, 0.82f, 0.80f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            glm::vec3(0.0f, 0.00f, 13.0f), { 28.0f,0.30f,8.0f },
            glm::vec3(0.78f, 0.78f, 0.76f));
        for (int li = 0; li < 3; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(0.0f, 0.16f, 12.5f + li * 1.5f), { 28.0f,0.02f,0.12f },
                glm::vec3(0.88f, 0.86f, 0.80f));
        for (int li = -4; li <= 4; li++)
            DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
                glm::vec3(li * 3.0f, 0.16f, 14.0f), { 0.12f,0.02f,5.0f },
                glm::vec3(0.88f, 0.86f, 0.80f));
        {
            float colH = 8.0f, r = 0.45f, fw = 0.34f;
            glm::vec3 cCol(0.72f, 0.70f, 0.66f);
            float ang[8] = { 0,45,90,135,180,225,270,315 };
            for (int side = -1; side <= 1; side += 2) {
                for (int fi = 0; fi < 8; fi++) {
                    float a = glm::radians(ang[fi]);
                    float fx = side * 10.0f + r * cosf(a);
                    float fz_ = 13.0f + r * sinf(a);
                    glm::mat4 m(1);
                    m = glm::translate(m, { fx,colH / 2.0f + 0.15f,fz_ });
                    m = glm::rotate(m, a, { 0,1,0 });
                    m = glm::scale(m, { fw,colH,fw });
                    _SendAndDraw(VAO, modelLoc, colorLoc, useTexLoc, tColumna, m, cCol);
                }
                DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tColumna,
                    { side * 10.0f,colH / 2.0f + 0.15f,13.0f }, { r * 1.35f,colH,r * 1.35f }, cCol);
            }
        }

        // ====================================================
        //  CELOSIA NARANJA
        // ====================================================
        DrawLattice(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            glm::vec3(-14.0f, 0.0f, 4.5f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tStone,
            glm::vec3(-13.8f, 3.5f, 1.5f), { 0.5f,7.0f,3.5f },
            glm::vec3(0.22f, 0.20f, 0.18f));

        // ====================================================
        //  TABLERO DE ANUNCIOS
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWood,
            { 14.45f,2.8f,-7.5f }, { 0.08f,2.2f,3.8f }, { 0.40f,0.25f,0.10f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.48f,2.8f,-7.5f }, { 0.05f,2.0f,3.5f }, { 0.72f,0.52f,0.28f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,3.2f,-8.5f }, { 0.04f,0.7f,1.0f }, { 0.95f,0.95f,0.92f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,3.2f,-7.5f }, { 0.04f,0.8f,0.9f }, { 0.20f,0.35f,0.72f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,3.2f,-6.6f }, { 0.04f,0.6f,0.8f }, { 0.85f,0.82f,0.55f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,2.4f,-8.2f }, { 0.04f,0.5f,0.7f }, { 0.92f,0.30f,0.25f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,2.4f,-7.2f }, { 0.04f,0.6f,0.6f }, { 0.88f,0.88f,0.88f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,2.4f,-6.4f }, { 0.04f,0.4f,0.55f }, { 0.55f,0.80f,0.45f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,3.55f,-8.0f }, { 0.04f,0.35f,0.45f }, { 0.95f,0.85f,0.30f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.50f,3.55f,-7.1f }, { 0.04f,0.30f,0.55f }, { 0.75f,0.88f,0.95f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.3f,0.9f,-7.3f }, { 0.04f,1.8f,0.04f }, { 0.78f,0.78f,0.80f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.3f,0.9f,-7.7f }, { 0.04f,1.8f,0.04f }, { 0.78f,0.78f,0.80f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.3f,0.25f,-7.5f }, { 0.04f,0.04f,0.55f }, { 0.78f,0.78f,0.80f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.32f,1.5f,-7.5f }, { 0.04f,1.0f,0.75f }, { 0.95f,0.95f,0.92f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.33f,1.35f,-7.5f }, { 0.04f,0.55f,0.65f }, { 0.15f,0.30f,0.68f });
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, 0,
            { 14.33f,1.85f,-7.5f }, { 0.04f,0.28f,0.65f }, { 0.92f,0.92f,0.92f });

        // ====================================================
        //  MURAL
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tMural,
            glm::vec3(0.0f, 5.8f, -9.50f), glm::vec3(28.0f, 6.4f, 0.05f),
            glm::vec3(1.0f, 1.0f, 1.0f));

        // ====================================================
        //  OBJETOS DE EVENTO
        // ====================================================

        // --- Stand S-01: animacion de colocacion (tecla G) ---
        if (standPlacing || standTimer > 0.0f)
            DrawStandPlacing(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
                tPVC, tMetal,
                glm::vec3(-11.0f, 0.0f, -6.0f), 0.0f,
                standTimer, STAND_ANIM_DUR);
        else
            DrawStand(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
                tPVC, tMetal, glm::vec3(-11.0f, 0.0f, -6.0f), 0.0f);
        DrawChair(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            tWood, glm::vec3(-9.0f, 0.0f, -6.0f), 180.0f);

        // --- Stands S-02 a S-04 ---
        for (int i = 1; i < 4; i++) {
            float zPos = -6.0f + i * 4.0f;
            DrawStand(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
                tPVC, tMetal, { -11.0f,0.0f,zPos }, 0.0f);
            DrawChair(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
                tWood, { -9.0f,0.0f,zPos }, 180.0f);
        }

        DrawBrochure(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { -9.5f,0,-7.5f });
        DrawBrochure(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { -9.5f,0, 0.5f });

        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { -6.0f,0,-5.0f }, currentFrame);
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { 0.0f,0,-5.0f }, currentFrame);
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, { 6.0f,0,-5.0f }, currentFrame);

        // --- Totem informativo digital (zona derecha, libre de escalera) ---
        DrawTotem(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            currentFrame, glm::vec3(3.5f, 0.0f, -2.0f));

        // --- Estacion de hidratacion Puma Agua ---
        DrawWaterStation(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(8.5f, 0.0f, -3.0f));

        // --- Dinosaurio (animacion compleja 1) ---
        DrawDino(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(0, 0, 5.5f), currentFrame);

        // ====================================================
        //  PUERTAS DEL AUDITORIO
        // ====================================================
        DrawBox(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, tWall,
            glm::vec3(0, 3.5f, -9.75f), { 6.0f,7.0f,0.28f },
            glm::vec3(0.90f, 0.90f, 0.90f));
        DrawDoor(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(-5.5f, 0.0f, -9.72f), doorAngle);
        DrawDoor(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(5.5f, 0.0f, -9.72f), doorAngle);

        // ====================================================
        //  SENALETICA
        // ====================================================
        DrawSignage(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(-10.0f, 2.2f, -1.5f), currentFrame);
        DrawSignage(VAO, shaderColor, modelLoc, colorLoc, useTexLoc,
            glm::vec3(10.0f, 2.2f, -1.5f), currentFrame);

        // ====================================================
        //  CONFETI (animacion simple 3 — tecla E)
        // ====================================================
        if (confettiActive)
            DrawConfetti(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, confettiTimer);

        // ====================================================
        //  AGENTES BEZIER (animacion compleja 2)
        // ====================================================
        DrawAgents(VAO, shaderColor, modelLoc, colorLoc, useTexLoc, currentFrame);

        // ====================================================
        //  MODELO EXTERNO (persona)
        // ====================================================
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
//  CARGA DE TEXTURA
// ============================================================
GLuint LoadTexture(const char* path)
{
    GLuint id;
    glGenTextures(1, &id);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 3);
    if (data) {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "[TEX OK] " << path << " " << w << "x" << h << "\n";
    }
    else {
        unsigned char pink[3] = { 255,0,128 };
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pink);
        std::cout << "[TEX FALTA] " << path << "\n";
    }
    stbi_image_free(data);
    return id;
}

// ============================================================
//  HELPER INTERNO
// ============================================================
static void _SendAndDraw(GLuint VAO, GLint modelLoc, GLint colorLoc,
    GLint useTexLoc, GLuint texID,
    const glm::mat4& model, glm::vec3 color)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    if (texID != 0) {
        glUniform1i(useTexLoc, 1);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
    else {
        glUniform1i(useTexLoc, 0);
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
    glm::mat4 m(1);
    m = glm::translate(m, pos);
    m = glm::scale(m, scale);
    _SendAndDraw(VAO, mL, cL, uTL, tex, m, color);
}

void DrawBoxRotatedX(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle)
{
    glm::mat4 m(1);
    m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(angle), { 1,0,0 });
    m = glm::scale(m, scale);
    _SendAndDraw(VAO, mL, cL, uTL, tex, m, color);
}

void DrawBoxRotatedY(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, GLuint tex,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle)
{
    glm::mat4 m(1);
    m = glm::translate(m, pos);
    m = glm::rotate(m, glm::radians(angle), { 0,1,0 });
    m = glm::scale(m, scale);
    _SendAndDraw(VAO, mL, cL, uTL, tex, m, color);
}

// ============================================================
//  OBJETO 1 — STAND MODULAR (estatico)
// ============================================================
void DrawStand(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    GLuint tPVC, GLuint tMetal, glm::vec3 pos, float rotY)
{
    glm::vec3 cAlum(0.75f, 0.75f, 0.78f), cPVC_(0.96f, 0.96f, 0.96f);
    glm::vec3 cAzul(0.05f, 0.28f, 0.60f), cMostr(0.88f, 0.88f, 0.88f);

    auto DB = [&](GLuint tex, glm::vec3 lp, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 par(1);
        par = glm::translate(par, pos);
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
//  ANIMACION OBLIGATORIA #1 — COLOCACION DE STAND CON KEYFRAMES
//
//  Las piezas caen desde el techo (DROP_HEIGHT) y se ensamblan
//  en orden, cada una con un desfasamiento temporal diferente.
//  Easing: SmoothStep + rebote ElasticOut al encajar.
//
//  Keyframes:
//   KF0 (offset 0.00) — Postes delanteros
//   KF1 (offset 0.12) — Postes traseros
//   KF2 (offset 0.25) — Travesanos superiores
//   KF3 (offset 0.38) — Paneles PVC
//   KF4 (offset 0.52) — Mostrador frontal
// ============================================================
void DrawStandPlacing(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    GLuint tPVC, GLuint tMetal,
    glm::vec3 pos, float rotY,
    float timer, float duration)
{
    glm::vec3 cAlum(0.75f, 0.75f, 0.78f);
    glm::vec3 cPVC_(0.96f, 0.96f, 0.96f);
    glm::vec3 cAzul(0.05f, 0.28f, 0.60f);
    glm::vec3 cMostr(0.88f, 0.88f, 0.88f);

    const float DROP_HEIGHT = 8.5f;

    // Helper con offset Y de animacion
    auto DB = [&](GLuint tex, glm::vec3 lp, glm::vec3 sc, glm::vec3 col, float yExtra)
        {
            glm::mat4 par(1);
            par = glm::translate(par, pos);
            par = glm::rotate(par, glm::radians(rotY), { 0.0f,1.0f,0.0f });
            glm::mat4 ch(1);
            ch = glm::translate(ch, lp + glm::vec3(0.0f, yExtra, 0.0f));
            ch = glm::scale(ch, sc);
            _SendAndDraw(VAO, mL, cL, uTL, tex, par * ch, col);
        };

    // Calcula desplazamiento Y segun desfasamiento de la pieza
    auto PieceY = [&](float offset) -> float
        {
            float start = offset * duration * 0.40f;
            float tRaw = (timer - start) / (duration * 0.60f);
            float t = glm::clamp(tRaw, 0.0f, 1.0f);
            float ease = ElasticOut(t);
            return DROP_HEIGHT * (1.0f - ease);
        };

    float pw = 0.08f, ph = 2.6f;

    // KF0: Postes delanteros
    float y0 = PieceY(0.00f);
    DB(tMetal, { -1.46f,ph / 2.0f,-1.46f }, { pw,ph,pw }, cAlum, y0);
    DB(tMetal, { 1.46f,ph / 2.0f,-1.46f }, { pw,ph,pw }, cAlum, y0);

    // KF1: Postes traseros
    float y1 = PieceY(0.12f);
    DB(tMetal, { -1.46f,ph / 2.0f, 1.46f }, { pw,ph,pw }, cAlum, y1);
    DB(tMetal, { 1.46f,ph / 2.0f, 1.46f }, { pw,ph,pw }, cAlum, y1);

    // KF2: Travesanos superiores
    float y2 = PieceY(0.25f);
    DB(tMetal, { 0.0f,ph - 0.04f,-1.46f }, { 3.0f,pw,pw }, cAlum, y2);
    DB(tMetal, { 0.0f,ph - 0.04f, 1.46f }, { 3.0f,pw,pw }, cAlum, y2);
    DB(tMetal, { -1.46f,ph - 0.04f, 0.0f }, { pw,pw,3.0f }, cAlum, y2);
    DB(tMetal, { 1.46f,ph - 0.04f, 0.0f }, { pw,pw,3.0f }, cAlum, y2);

    // KF3: Paneles de PVC
    float y3 = PieceY(0.38f);
    DB(tPVC, { 0.0f, 1.3f, 1.44f }, { 2.92f,2.56f,0.04f }, cPVC_, y3);
    DB(tPVC, { -1.44f,1.3f, 0.0f }, { 0.04f,2.56f,2.92f }, cPVC_, y3);
    DB(tPVC, { 1.44f,1.3f, 0.5f }, { 0.04f,2.56f,1.90f }, cPVC_, y3);
    DB(0, { 0.0f, 2.2f, 1.43f }, { 2.90f,0.45f,0.06f }, cAzul, y3);

    // KF4: Mostrador frontal
    float y4 = PieceY(0.52f);
    DB(tPVC, { 0.0f,1.05f,-1.10f }, { 2.10f,0.10f,0.60f }, cMostr, y4);
    DB(tPVC, { 0.0f,0.55f,-0.82f }, { 2.10f,1.00f,0.06f }, cPVC_, y4);
    DB(tPVC, { 0.0f,0.30f, 0.80f }, { 1.50f,0.60f,0.80f },
        glm::vec3(0.85f, 0.85f, 0.85f), y4);
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
//  OBJETO 4 — BANDERA CON ONDEO (animacion simple)
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
//  OBJETO 5 — CELOSIA NARANJA
// ============================================================
void DrawLattice(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    GLuint tWood, glm::vec3 bp)
{
    glm::vec3 cOr(0.88f, 0.52f, 0.06f), cFr(0.45f, 0.25f, 0.05f);
    int   n = 14;
    float sh = 7.20f, gap = 0.28f, lw = 0.06f, ld = 0.12f;
    float totalZ = n * gap;

    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(0, sh + 0.10f, totalZ / 2.0f), { 0.15f,0.16f,totalZ + 0.20f }, cFr);
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(0, 0.08f, totalZ / 2.0f), { 0.15f,0.16f,totalZ + 0.20f }, cFr);
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(0, sh / 2.0f, -0.10f), { 0.15f,sh + 0.30f,0.15f }, cFr);
    DrawBox(VAO, s, mL, cL, uTL, tWood,
        bp + glm::vec3(0, sh / 2.0f, totalZ + 0.10f), { 0.15f,sh + 0.30f,0.15f }, cFr);
    for (int i = 0; i < n; i++) {
        float zPos = i * gap + gap * 0.5f;
        DrawBox(VAO, s, mL, cL, uTL, 0,
            bp + glm::vec3(0, sh / 2.0f, zPos), { lw,sh,ld }, cOr);
    }
}

// ============================================================
//  OBJETO 6 — DINOSAURIO ANIMADO (animacion compleja 1)
//
//  Jerarquia real: todas las partes derivan de mTorso,
//  por lo que heredan su traslacion Y rotacion.
//  Arbol:
//    mTorso → barriga
//           → mCuello → mCabeza → hocico, ojos
//           → mHombroIzq → brazo izq
//           → mHombroDer → brazo der
//           → mCaderaIzq → pierna izq
//           → mCaderaDer → pierna der
//           → cola
// ============================================================
void DrawDino(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float time)
{
    glm::vec3 cBody(0.30f, 0.72f, 0.15f);
    glm::vec3 cDark(0.20f, 0.52f, 0.08f);
    glm::vec3 cBelly(0.85f, 0.95f, 0.70f);
    glm::vec3 cEye(0.05f, 0.05f, 0.05f);

    float cy = time * (2.0f * 3.14159f / 2.0f);
    float bobY = sinf(cy) * 0.08f;
    float tZ = sinf(cy) * 12.0f;
    float aL = sinf(cy) * 28.0f;
    float aR = sinf(cy + 3.14159f) * 28.0f;
    float hB = sinf(cy * 2.0f) * 4.0f;
    float lL = sinf(cy) * 22.0f;
    float lR = sinf(cy + 3.14159f) * 22.0f;

    auto P = [&](glm::mat4 m, glm::vec3 col) { _SendAndDraw(VAO, mL, cL, uTL, 0, m, col); };

    // ---- TORSO (nodo raiz) ----
    // mTorso: solo traslacion + rotacion (NO escala)
    // Los hijos heredan esta matriz y le agregan su propio offset local
    // El scale del torso se aplica SOLO al dibujo, no se propaga
    glm::mat4 mTorso(1);
    mTorso = glm::translate(mTorso, pos + glm::vec3(0.0f, 1.10f + bobY, 0.0f));
    mTorso = glm::rotate(mTorso, glm::radians(tZ), glm::vec3(0, 0, 1));

    // Dibuja torso (escala local, no afecta hijos)
    { glm::mat4 m = mTorso; m = glm::scale(m, { 0.75f,0.85f,0.65f }); P(m, cBody); }
    // Barriga: offset local 0.33 en Z desde el centro del torso
    {
        glm::mat4 m = mTorso;
        m = glm::translate(m, { 0.0f,0.0f,0.33f });
        m = glm::scale(m, { 0.52f,0.65f,0.05f }); P(m, cBelly);
    }

    // ---- CUELLO — hijo del torso ----
    // Offset: 0.43 arriba del centro del torso (mitad torso = 0.425)
    glm::mat4 mCuello = mTorso;
    mCuello = glm::translate(mCuello, { 0.0f, 0.43f, 0.05f });
    mCuello = glm::rotate(mCuello, glm::radians(tZ * 0.4f), glm::vec3(0, 0, 1));
    { glm::mat4 m = mCuello; m = glm::scale(m, { 0.30f,0.38f,0.28f }); P(m, cBody); }

    // ---- CABEZA — hija del cuello ----
    // Offset: 0.19 arriba del centro del cuello (mitad cuello = 0.19)
    glm::mat4 mCabeza = mCuello;
    mCabeza = glm::translate(mCabeza, { 0.0f, 0.34f, 0.0f });
    mCabeza = glm::rotate(mCabeza, glm::radians(hB), glm::vec3(1, 0, 0));
    { glm::mat4 m = mCabeza; m = glm::scale(m, { 0.50f,0.40f,0.46f }); P(m, cBody); }
    // Hocico: 0.23 al frente de la cabeza
    {
        glm::mat4 m = mCabeza;
        m = glm::translate(m, { 0.0f,-0.05f,0.23f });
        m = glm::scale(m, { 0.36f,0.18f,0.20f }); P(m, cDark);
    }
    // Ojos
    for (int sd = -1; sd <= 1; sd += 2) {
        glm::mat4 m = mCabeza;
        m = glm::translate(m, { sd * 0.17f, 0.09f, 0.21f });
        m = glm::scale(m, { 0.08f,0.08f,0.05f }); P(m, cEye);
    }

    // ---- BRAZO IZQUIERDO — hijo del torso ----
    // Pivote en hombro: borde izq del torso (0.375) + altura hombro (0.25)
    {
        glm::mat4 mH = mTorso;
        mH = glm::translate(mH, { -0.375f, 0.25f, 0.0f });
        mH = glm::rotate(mH, glm::radians(aL), glm::vec3(0, 0, 1));
        // El brazo cuelga: su centro esta 0.20 debajo del pivote
        glm::mat4 m = mH;
        m = glm::translate(m, { -0.08f,-0.20f,0.0f });
        m = glm::scale(m, { 0.16f,0.40f,0.16f }); P(m, cBody);
    }
    // ---- BRAZO DERECHO — hijo del torso ----
    {
        glm::mat4 mH = mTorso;
        mH = glm::translate(mH, { 0.375f, 0.25f, 0.0f });
        mH = glm::rotate(mH, glm::radians(aR), glm::vec3(0, 0, 1));
        glm::mat4 m = mH;
        m = glm::translate(m, { 0.08f,-0.20f,0.0f });
        m = glm::scale(m, { 0.16f,0.40f,0.16f }); P(m, cBody);
    }

    // ---- PIERNA IZQUIERDA — hija del torso ----
    // Pivote en cadera: borde inferior torso (-0.425) + separacion lateral
    {
        glm::mat4 mC = mTorso;
        mC = glm::translate(mC, { -0.20f,-0.425f,0.0f });
        mC = glm::rotate(mC, glm::radians(lL), glm::vec3(1, 0, 0));
        // Pierna cuelga: centro 0.27 debajo del pivote
        glm::mat4 m = mC;
        m = glm::translate(m, { 0.0f,-0.27f,0.0f });
        m = glm::scale(m, { 0.20f,0.54f,0.20f }); P(m, cDark);
    }
    // ---- PIERNA DERECHA — hija del torso ----
    {
        glm::mat4 mC = mTorso;
        mC = glm::translate(mC, { 0.20f,-0.425f,0.0f });
        mC = glm::rotate(mC, glm::radians(lR), glm::vec3(1, 0, 0));
        glm::mat4 m = mC;
        m = glm::translate(m, { 0.0f,-0.27f,0.0f });
        m = glm::scale(m, { 0.20f,0.54f,0.20f }); P(m, cDark);
    }

    // ---- COLA — hija del torso ----
    {
        glm::mat4 mCola = mTorso;
        mCola = glm::translate(mCola, { 0.0f,-0.10f,-0.33f });
        mCola = glm::rotate(mCola, glm::radians(-28.0f + tZ * 0.6f), glm::vec3(1, 0, 0));
        glm::mat4 m = mCola;
        m = glm::translate(m, { 0.0f,0.0f,-0.26f });
        m = glm::scale(m, { 0.20f,0.20f,0.55f }); P(m, cDark);
    }
}

// ============================================================
//  OBJETO 7 — TOTEM INFORMATIVO DIGITAL
//
//  Kiosko digital de feria universitaria. La pantalla tiene un
//  pulso de brillo automatico (animacion simple integrada) que
//  simula contenido en reproduccion. LED indicador parpadea.
// ============================================================
void DrawTotem(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    float time, glm::vec3 pos)
{
    glm::vec3 cBase(0.12f, 0.12f, 0.13f);
    glm::vec3 cMarco(0.18f, 0.18f, 0.20f);
    glm::vec3 cScreen(0.04f, 0.10f, 0.32f);
    glm::vec3 cUI(0.88f, 0.92f, 0.98f);
    glm::vec3 cGold(0.85f, 0.65f, 0.13f);
    glm::vec3 cGreen(0.15f, 0.65f, 0.30f);

    // Pulso de brillo — simula contenido en loop
    float br = 0.85f + 0.15f * sinf(time * 0.8f);
    glm::vec3 cScreenLive(cScreen.r * br, cScreen.g * br, cScreen.b * br + 0.08f * br);

    auto DB = [&](glm::vec3 lp, glm::vec3 sc, glm::vec3 col) {
        DrawBox(VAO, s, mL, cL, uTL, 0, pos + lp, sc, col);
        };

    // Base escalonada
    DB({ 0.0f,0.05f,0.0f }, { 0.80f,0.10f,0.80f }, cBase);
    DB({ 0.0f,0.13f,0.0f }, { 0.55f,0.07f,0.55f }, cBase);
    DB({ 0.0f,0.19f,0.0f }, { 0.35f,0.06f,0.35f }, cBase);

    // Poste central
    DB({ 0.0f,1.20f,0.000f }, { 0.12f,2.02f,0.12f }, cBase);
    DB({ 0.0f,1.20f,0.055f }, { 0.06f,2.00f,0.06f }, cMarco);

    // Marco pantalla
    DB({ 0.0f,2.55f,0.00f }, { 0.68f,1.85f,0.10f }, cMarco);

    // Pantalla activa con pulso
    DB({ 0.0f,2.58f,0.04f }, { 0.58f,1.60f,0.06f }, cScreenLive);

    // UI — barra de titulo dorada (identidad UNAM)
    DB({ 0.0f,3.28f,0.08f }, { 0.54f,0.14f,0.04f }, cGold);

    // UI — subtitulo
    DB({ 0.0f,3.10f,0.08f }, { 0.42f,0.05f,0.04f }, cUI);

    // UI — 5 filas de cronograma
    for (int li = 0; li < 5; li++) {
        float ly = 2.90f - li * 0.22f;
        glm::vec3 rowCol = (li % 2 == 0)
            ? glm::vec3(0.20f, 0.26f, 0.48f)
            : glm::vec3(0.14f, 0.18f, 0.38f);
        DB({ 0.0f,  ly,0.08f }, { 0.52f,0.09f,0.04f }, rowCol);
        DB({ -0.22f,ly,0.10f }, { 0.07f,0.07f,0.04f }, cGreen);
        DB({ 0.04f, ly,0.10f }, { 0.30f,0.04f,0.04f }, cUI * 0.7f);
    }

    // UI — pie de pantalla / mapa
    DB({ 0.0f,  1.88f,0.08f }, { 0.54f,0.16f,0.04f }, glm::vec3(0.10f, 0.14f, 0.28f));
    DB({ -0.05f,1.88f,0.10f }, { 0.22f,0.10f,0.04f }, cGold * 0.7f);

    // Cornisa superior
    DB({ 0.0f,3.48f,0.01f }, { 0.72f,0.06f,0.12f }, cMarco);

    // LED indicador (parpadeo)
    float ledPulse = (sinf(time * 2.5f) > 0.3f) ? 1.0f : 0.2f;
    DB({ 0.28f,3.48f,0.07f }, { 0.04f,0.04f,0.04f }, cGreen * ledPulse);
}

// ============================================================
//  OBJETO 8 — ESTACION DE HIDRATACION PUMA AGUA
//
//  Dispensador institucional UNAM: cuerpo inox, logo Puma Agua,
//  grifo con boton azul, visor de nivel y bandeja con rejilla.
// ============================================================
void DrawWaterStation(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos)
{
    glm::vec3 cMetal(0.78f, 0.80f, 0.84f);
    glm::vec3 cDark(0.35f, 0.36f, 0.38f);
    glm::vec3 cAzulU(0.05f, 0.28f, 0.60f);
    glm::vec3 cGold(0.85f, 0.65f, 0.13f);
    glm::vec3 cBlue(0.55f, 0.80f, 0.95f);
    glm::vec3 cRubber(0.15f, 0.15f, 0.16f);

    auto DB = [&](glm::vec3 lp, glm::vec3 sc, glm::vec3 col) {
        DrawBox(VAO, s, mL, cL, uTL, 0, pos + lp, sc, col);
        };

    // Patas
    float lx = 0.18f, lz = 0.18f;
    for (int xi = -1; xi <= 1; xi += 2)
        for (int zi = -1; zi <= 1; zi += 2)
            DB({ xi * lx,0.12f,zi * lz }, { 0.04f,0.24f,0.04f }, cDark);

    // Bandeja recolectora
    DB({ 0.0f,0.27f,0.0f }, { 0.52f,0.06f,0.52f }, cMetal);
    DB({ 0.0f,0.28f,0.0f }, { 0.44f,0.02f,0.44f }, glm::vec3(0.40f, 0.65f, 0.75f));
    for (int ri = -2; ri <= 2; ri++) {
        DB({ ri * 0.08f,0.31f,0.0f }, { 0.015f,0.015f,0.44f }, cDark);
        DB({ 0.0f,0.31f,ri * 0.08f }, { 0.44f,0.015f,0.015f }, cDark);
    }

    // Cuerpo principal
    DB({ 0.0f,1.00f,0.000f }, { 0.48f,1.40f,0.48f }, cMetal);
    DB({ 0.0f,1.00f,0.235f }, { 0.46f,1.38f,0.02f }, glm::vec3(0.84f, 0.86f, 0.90f));

    // Logo UNAM / Puma Agua
    DB({ 0.0f,  1.25f,0.250f }, { 0.28f,0.22f,0.02f }, cAzulU);
    DB({ 0.0f,  1.25f,0.260f }, { 0.20f,0.14f,0.02f }, cGold);
    DB({ -0.06f,1.25f,0.265f }, { 0.025f,0.12f,0.02f }, cRubber);
    DB({ -0.06f,1.31f,0.265f }, { 0.080f,0.025f,0.02f }, cRubber);
    DB({ 0.02f,1.28f,0.265f }, { 0.025f,0.065f,0.02f }, cRubber);
    DB({ 0.0f,1.10f,0.255f }, { 0.26f,0.018f,0.02f }, cRubber);
    DB({ 0.0f,1.06f,0.255f }, { 0.20f,0.014f,0.02f }, cRubber);
    DB({ 0.0f,1.02f,0.255f }, { 0.16f,0.012f,0.02f }, cRubber);

    // Visor de nivel de agua (lateral)
    DB({ 0.24f,1.40f,0.0f }, { 0.02f,0.50f,0.12f }, glm::vec3(0.72f, 0.88f, 0.95f));
    DB({ 0.24f,1.22f,0.0f }, { 0.02f,0.14f,0.10f }, cBlue * 0.5f);

    // Zona de dispensado y grifo
    DB({ 0.0f, 0.60f,0.25f }, { 0.22f,0.08f,0.12f }, cDark);
    DB({ 0.0f, 0.55f,0.32f }, { 0.06f,0.12f,0.06f }, cDark);
    DB({ 0.0f, 0.50f,0.35f }, { 0.04f,0.04f,0.08f }, cRubber);
    DB({ 0.06f,0.58f,0.32f }, { 0.08f,0.03f,0.03f }, cAzulU);

    // Tapa superior y deposito de agua
    DB({ 0.0f,1.72f,0.0f }, { 0.52f,0.08f,0.52f }, cDark);
    DB({ 0.0f,1.78f,0.0f }, { 0.36f,0.12f,0.36f }, glm::vec3(0.20f, 0.45f, 0.75f));
    DB({ 0.0f,1.82f,0.0f }, { 0.32f,0.06f,0.32f }, cMetal);

    // Vaso biodegradable en la bandeja
    DB({ 0.12f,0.34f,0.12f }, { 0.06f,0.10f,0.06f }, glm::vec3(0.95f, 0.92f, 0.85f));
}

// ============================================================
//  ANIMACION SIMPLE 1 — PUERTAS DEL AUDITORIO
// ============================================================
void DrawDoor(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float openAngle)
{
    glm::vec3 cMarco(0.80f, 0.80f, 0.82f), cReja(0.91f, 0.91f, 0.91f), cVidrio(0.78f, 0.84f, 0.88f);
    float dw = 1.55f, dh = 2.40f;

    DrawBox(VAO, s, mL, cL, uTL, 0,
        pos + glm::vec3(0, dh + 0.08f, 0), { dw * 2 + 0.20f,0.14f,0.14f }, cMarco);
    DrawBox(VAO, s, mL, cL, uTL, 0,
        pos + glm::vec3(-dw - 0.10f, dh / 2.0f, 0), { 0.12f,dh + 0.18f,0.12f }, cMarco);
    DrawBox(VAO, s, mL, cL, uTL, 0,
        pos + glm::vec3(dw + 0.10f, dh / 2.0f, 0), { 0.12f,dh + 0.18f,0.12f }, cMarco);

    auto Leaf = [&](float sign) {
        glm::vec3 pivot = pos + glm::vec3(sign * (dw * 0.5f + 0.05f), dh / 2.0f, 0.0f);
        {
            glm::mat4 m(1); m = glm::translate(m, pivot);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * dw * 0.5f,0,0 }); m = glm::scale(m, { dw,dh,0.06f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, cVidrio);
        }
        for (float fy : {dh * 0.5f - 0.04f, -dh * 0.5f + 0.04f}) {
            glm::mat4 m(1); m = glm::translate(m, pivot);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * dw * 0.5f,fy,0.04f }); m = glm::scale(m, { dw,0.07f,0.07f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, cMarco);
        }
        for (float fx : {-dw * 0.5f + 0.04f, dw * 0.5f - 0.04f}) {
            glm::mat4 mp(1); mp = glm::translate(mp, pivot);
            mp = glm::rotate(mp, glm::radians(sign * openAngle), { 0,1,0 });
            mp = glm::translate(mp, { sign * (dw * 0.5f) + fx,0,0.04f }); mp = glm::scale(mp, { 0.07f,dh,0.07f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, mp, cMarco);
        }
        for (int col = 1; col <= 3; col++) {
            float lx_ = sign * (-dw * 0.5f + col * (dw / 4.0f));
            glm::mat4 m(1); m = glm::translate(m, pivot);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * dw * 0.5f + lx_,0,0.05f }); m = glm::scale(m, { 0.055f,dh - 0.10f,0.055f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, cReja);
        }
        for (int row = 0; row < 4; row++) {
            float ly = -dh * 0.5f + 0.08f + row * (dh - 0.16f) / 3.0f;
            glm::mat4 m(1); m = glm::translate(m, pivot);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * dw * 0.5f,ly,0.05f }); m = glm::scale(m, { dw - 0.10f,0.055f,0.055f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, cReja);
        }
        {
            glm::mat4 m(1); m = glm::translate(m, pivot);
            m = glm::rotate(m, glm::radians(sign * openAngle), { 0,1,0 });
            m = glm::translate(m, { sign * dw * 0.5f - sign * 0.35f,0,0.09f });
            m = glm::scale(m, { 0.06f,0.30f,0.06f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, glm::vec3(0.70f, 0.70f, 0.72f));
        }
        };
    Leaf(-1.0f);
    Leaf(1.0f);
}

// ============================================================
//  ANIMACION SIMPLE 2 — SENALETICA PULSANTE
// ============================================================
void DrawSignage(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL,
    glm::vec3 pos, float time)
{
    float pulse = 1.0f + 0.06f * sinf(time * 1.8f);
    glm::vec3 cFondo(0.08f, 0.45f, 0.18f), cFlecha(0.95f, 0.95f, 0.95f), cBorde(0.05f, 0.30f, 0.12f);

    auto Sg = [&](glm::vec3 off, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 m(1); m = glm::translate(m, pos + off); m = glm::scale(m, sc * pulse);
        _SendAndDraw(VAO, mL, cL, uTL, 0, m, col);
        };

    Sg({ 0,0,0 }, { 0.55f,0.30f,0.04f }, cBorde);
    Sg({ 0,0,0.03f }, { 0.50f,0.26f,0.04f }, cFondo);
    Sg({ 0.05f,0,0.05f }, { 0.22f,0.10f,0.04f }, cFlecha);
    Sg({ 0.18f,0,0.05f }, { 0.10f,0.18f,0.04f }, cFlecha);
    Sg({ -0.10f, 0.04f,0.05f }, { 0.18f,0.04f,0.04f }, cFlecha);
    Sg({ -0.10f,-0.04f,0.05f }, { 0.18f,0.04f,0.04f }, cFlecha);
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
        float y = ceil - p.speed * rel * 1.8f; if (y < 0) continue;
        float sc = 0.12f, lr = rel / life;
        if (lr > 0.80f) sc *= (1 - (lr - 0.80f) / 0.20f);
        if (sc < 0.005f) continue;
        glm::mat4 m(1); m = glm::translate(m, { p.x,y,p.z });
        m = glm::rotate(m, glm::radians(p.rotSpeed * rel), { 0,0,1 });
        m = glm::scale(m, { sc,sc * .5f,sc * .1f });
        _SendAndDraw(VAO, mL, cL, uTL, 0, m, { p.colorR,p.colorG,p.colorB });
    }
}

// ============================================================
//  ANIMACION COMPLEJA 2 — AGENTES EN BEZIER
// ============================================================
void DrawAgents(GLuint VAO, Shader& s, GLint mL, GLint cL, GLint uTL, float time)
{
    glm::vec3 cols[NUM_AGENTS] = {
        {.20f,.40f,.80f},{.80f,.30f,.20f},{.20f,.65f,.25f},
        {.75f,.60f,.15f},{.50f,.20f,.65f}
    };
    for (int i = 0; i < NUM_AGENTS; i++) {
        const BezierPath& bp = agentPaths[i];
        float raw = fmodf(time * bp.speed + bp.phase, 1.0f);
        float t = fmodf(raw + 0.3f * sinf(raw * 6.28f) * 0.1f, 1.0f);
        if (t < 0) t += 1.0f;

        glm::vec3 pos = BezierEval(bp, t); pos.y = 0;

        if (pos.x > -5.0f && pos.x<0.5f && pos.z>-4.0f && pos.z < 3.5f)
            pos.x = (pos.x < -2.25f) ? -5.2f : 0.7f;
        if (pos.x > 5.0f && pos.x < 8.5f && pos.z>0.5f && pos.z < 4.0f)
            pos.x = 4.8f;
        if (pos.x > 0.8f && pos.x < 2.8f && pos.z>0.8f && pos.z < 3.2f)
            pos.x = 3.0f;

        pos.x = glm::clamp(pos.x, -8.5f, 8.5f);
        pos.z = glm::clamp(pos.z, -7.5f, 8.0f);

        glm::vec3 next = BezierEval(bp, fminf(t + 0.01f, 1.0f));
        glm::vec3 dir = next - pos;
        float yaw = 0;
        if (glm::length(dir) > 0.001f) yaw = glm::degrees(atan2f(dir.x, dir.z));

        glm::vec3 col = cols[i];
        float leg = sinf(time * 6 + bp.phase * 6.28f) * 15.0f;

        {
            glm::mat4 m(1); m = glm::translate(m, pos + glm::vec3(0, .55f, 0));
            m = glm::rotate(m, glm::radians(yaw), { 0,1,0 }); m = glm::scale(m, { .35f,.70f,.30f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, col);
        }
        {
            glm::mat4 m(1); m = glm::translate(m, pos + glm::vec3(0, 1.05f, 0));
            m = glm::rotate(m, glm::radians(yaw), { 0,1,0 }); m = glm::scale(m, { .28f,.28f,.28f });
            _SendAndDraw(VAO, mL, cL, uTL, 0, m, { .88f,.72f,.58f });
        }

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Animaciones
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        confettiActive = true; confettiTimer = 0;
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        doorOpen = !doorOpen;
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        standPlacing = true; standTimer = 0.0f;
    }

    // Modos de iluminacion
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) lightMode = 1;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) lightMode = 2;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) lightMode = 3;
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) lightMode = 4;

    // Intensidad ambiente
    if ((key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) && action == GLFW_PRESS)
        lightAmbient = glm::min(lightAmbient + 0.10f, 1.0f);
    if ((key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) && action == GLFW_PRESS)
        lightAmbient = glm::max(lightAmbient - 0.10f, 0.0f);

    // Intensidad difusa
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        lightDiffuse = glm::min(lightDiffuse + 0.10f, 1.0f);
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
        lightDiffuse = glm::max(lightDiffuse - 0.10f, 0.0f);

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
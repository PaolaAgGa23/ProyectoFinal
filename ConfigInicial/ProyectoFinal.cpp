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
//  CONSTANTES DE VENTANA
// ============================================================
const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ============================================================
//  PROTOTIPOS
// ============================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void DoMovement();

// Primitivas de dibujo
void DrawBox(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color);
void DrawBoxRotatedX(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle);
void DrawBoxRotatedY(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle);

// Objetos de escena
void DrawStand(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float rotY);
void DrawChair(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float rotY);
void DrawBrochure(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos);
void DrawFlag(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float time);
void DrawLattice(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 basePos);
void DrawDino(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float time);
void DrawDoor(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float openAngle);
void DrawConfetti(GLuint VAO, Shader& s, GLint mL, GLint cL, float time, bool active);
void DrawSignage(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float time);

// ============================================================
//  CAMARA Y ENTRADA
// ============================================================
Camera  camera(glm::vec3(0.0f, 3.0f, 12.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool    keys[1024] = {};
bool    firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// ============================================================
//  ESTADO DE ANIMACIONES
// ============================================================
bool  confettiActive = false;   // Tecla E activa el confeti
float confettiTimer = 0.0f;   // Cuanto dura el confeti
float doorAngle = 0.0f;   // Angulo actual de las puertas
bool  doorOpen = false;   // Tecla F abre/cierra puerta

// Semilla para posiciones aleatorias del confeti (se calculan una sola vez)
struct ConfettiParticle {
    float x, z;        // Posicion XZ inicial (aleatoria en explanada)
    float speed;       // Velocidad de caida
    float rotSpeed;    // Velocidad de rotacion
    float colorR, colorG, colorB; // Color (azul UNAM, dorado, blanco)
    float phase;       // Desfase para variacion
};

const int NUM_CONFETTI = 200;
ConfettiParticle confParticles[NUM_CONFETTI];

// ============================================================
//  INICIALIZACION DE PARTICULAS (pool preinicializado)
// ============================================================
void InitConfetti()
{
    // Paleta: azul UNAM (0.0,0.27,0.55), dorado (0.85,0.65,0.13), blanco (1,1,1)
    glm::vec3 palette[3] = {
        {0.0f, 0.27f, 0.55f},
        {0.85f, 0.65f, 0.13f},
        {1.0f, 1.0f, 1.0f}
    };
    for (int i = 0; i < NUM_CONFETTI; i++) {
        confParticles[i].x = ((rand() % 2800) / 100.0f) - 14.0f; // -14 a 14
        confParticles[i].z = ((rand() % 1900) / 100.0f) - 9.5f;  // -9.5 a 9.5
        confParticles[i].speed = 0.8f + (rand() % 50) / 100.0f;      // 0.8 - 1.3
        confParticles[i].rotSpeed = 30.0f + (rand() % 120);             // 30 - 150 grados/s
        confParticles[i].phase = (rand() % 300) / 100.0f;            // 0 - 3
        int c = rand() % 3;
        confParticles[i].colorR = palette[c].r;
        confParticles[i].colorG = palette[c].g;
        confParticles[i].colorB = palette[c].b;
    }
}

// ============================================================
//  MAIN
// ============================================================
int main()
{
    srand(42); // Semilla fija para reproducibilidad

    // --- Inicializacion de GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT,
        "Lobby Universitario - Auditorio JBS", nullptr, nullptr);
    if (!window) {
        std::cout << "Error: no se pudo crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error: no se pudo inicializar GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Habilitar blending para alpha del confeti y señaletica
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --------------------------------------------------------
    //  Shaders
    // --------------------------------------------------------
    Shader shaderColor("Shader/core.vs", "Shader/core.frag");
    Shader shaderModel("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // --------------------------------------------------------
    //  Modelos externos
    // --------------------------------------------------------
    Model person((char*)"Models/person.obj");

    // --------------------------------------------------------
    //  Geometria del cubo (VAO/VBO igual que el original)
    // --------------------------------------------------------
    float vertices[] = {
        // Cara frontal (Z+)
        -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,
         // Cara trasera (Z-)
         -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,
          0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,
          // Cara derecha (X+)
           0.5f,-0.5f, 0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,
           0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,   0.5f,-0.5f, 0.5f,
           // Cara izquierda (X-)
           -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,
           -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
           // Cara inferior (Y-)
           -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,
            0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f,-0.5f,-0.5f,
            // Cara superior (Y+)
            -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,
             0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --------------------------------------------------------
    //  Proyeccion (igual que el original)
    // --------------------------------------------------------
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 200.0f);

    // Preinicializar pool de confeti
    InitConfetti();

    // ============================================================
    //  LOOP PRINCIPAL
    // ============================================================
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        // --- Actualizar estado de animaciones ---

        // Confeti: dura 5 segundos tras presionar E
        if (confettiActive) {
            confettiTimer += deltaTime;
            if (confettiTimer > 5.0f) {
                confettiActive = false;
                confettiTimer = 0.0f;
            }
        }

        // Puerta: interpolacion suave 0->90 grados (ease in-out) en 0.8s
        float targetAngle = doorOpen ? 90.0f : 0.0f;
        doorAngle += (targetAngle - doorAngle) * (1.0f - exp(-deltaTime / 0.12f));

        // --- Render ---
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();

        // ==============  SHADER COLOR  ==============
        shaderColor.Use();

        GLint modelLoc = glGetUniformLocation(shaderColor.Program, "model");
        GLint viewLoc = glGetUniformLocation(shaderColor.Program, "view");
        GLint projLoc = glGetUniformLocation(shaderColor.Program, "projection");
        GLint colorLoc = glGetUniformLocation(shaderColor.Program, "color");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // ====================================================
        //  ESCENA ORIGINAL (sin modificar)
        // ====================================================

        // Piso principal
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(0.0f, -0.15f, 0.0f),
            glm::vec3(30.0f, 0.3f, 20.0f),
            glm::vec3(0.78f, 0.72f, 0.60f));

        // Columnas y techo
        float colH = 8.0f;
        std::vector<glm::vec2> colPos = { {-8,-4},{8,-4},{-8,4},{8,4} };
        for (auto& cp : colPos) {
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(cp.x, colH / 2.0f, cp.y),
                glm::vec3(0.9f, colH, 0.9f),
                glm::vec3(0.75f, 0.75f, 0.75f));
        }
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(0.0f, 8.15f, 0.0f),
            glm::vec3(18.0f, 0.3f, 10.0f),
            glm::vec3(0.65f, 0.65f, 0.65f));

        // Escalera
        int   nSteps = 14;
        float stepH = 0.28f, stepD = 0.48f, stairW = 5.20f;
        float stairX = -2.20f, startZ = 3.20f, startY = 0.00f;

        glm::vec3 stairTopColor(0.74f, 0.74f, 0.72f);
        glm::vec3 stairFaceColor(0.58f, 0.58f, 0.56f);
        glm::vec3 sideColor(0.86f, 0.86f, 0.84f);

        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX - stairW / 2.0f - 0.25f, 1.05f, 0.05f),
            glm::vec3(0.35f, 2.10f, 7.40f), sideColor);
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX + stairW / 2.0f + 0.25f, 1.05f, 0.05f),
            glm::vec3(0.35f, 2.10f, 7.40f), sideColor);

        for (int i = 0; i < nSteps; i++) {
            float y = startY + (i * stepH);
            float z = startZ - (i * stepD);
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(stairX, y + 0.04f, z),
                glm::vec3(stairW, 0.08f, stepD), stairTopColor);
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(stairX, y - stepH / 2.0f, z + stepD / 2.0f),
                glm::vec3(stairW, stepH, 0.08f), stairFaceColor);
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(stairX, y / 2.0f - 0.06f, z),
                glm::vec3(stairW, y + 0.12f, stepD),
                glm::vec3(0.68f, 0.68f, 0.66f));
        }

        float topY = startY + (nSteps * stepH);
        float topZ = startZ - (nSteps * stepD) - 0.20f;

        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX, topY - 0.08f, topZ),
            glm::vec3(stairW + 1.20f, 0.22f, 2.00f),
            glm::vec3(0.72f, 0.72f, 0.70f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX, topY - 0.10f, topZ - 1.55f),
            glm::vec3(stairW + 3.00f, 0.25f, 2.20f),
            glm::vec3(0.68f, 0.68f, 0.66f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(0.0f, topY + 0.02f, topZ - 0.60f),
            glm::vec3(10.50f, 0.16f, 0.50f),
            glm::vec3(0.70f, 0.70f, 0.68f));
        DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(4.20f, topY + 2.45f, topZ - 2.90f),
            glm::vec3(5.60f, 0.28f, 6.80f),
            glm::vec3(0.70f, 0.70f, 0.68f), -42.0f);

        // Mostrador rojo (recepcion)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(5.75f, 0.65f, 2.70f),
            glm::vec3(3.10f, 1.30f, 1.25f),
            glm::vec3(0.80f, 0.10f, 0.05f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(7.15f, 0.65f, 1.65f),
            glm::vec3(1.25f, 1.30f, 1.20f),
            glm::vec3(0.80f, 0.10f, 0.05f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(5.95f, 1.34f, 2.55f),
            glm::vec3(3.65f, 0.12f, 1.75f),
            glm::vec3(0.95f, 0.95f, 0.95f));

        // Busto
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 0.8f, 2.0f),
            glm::vec3(0.9f, 1.6f, 0.9f),
            glm::vec3(0.60f, 0.60f, 0.60f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 1.62f, 2.0f),
            glm::vec3(1.2f, 0.12f, 1.2f),
            glm::vec3(0.55f, 0.55f, 0.55f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 2.15f, 2.0f),
            glm::vec3(0.7f, 0.55f, 0.5f),
            glm::vec3(0.55f, 0.55f, 0.55f));
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 2.68f, 2.0f),
            glm::vec3(0.48f, 0.52f, 0.48f),
            glm::vec3(0.58f, 0.58f, 0.58f));

        // ====================================================
        //  NUEVOS OBJETOS
        // ====================================================

        // --- OBJETO 1: Stands modulares (S-01 a S-04 contra muro oeste) ---
        // Cada stand es 3x3 m, los colocamos en X negativo (muro oeste)
        DrawStand(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-11.0f, 0.0f, -6.0f), 0.0f);
        DrawStand(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-11.0f, 0.0f, -2.0f), 0.0f);
        DrawStand(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-11.0f, 0.0f, 2.0f), 0.0f);
        DrawStand(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-11.0f, 0.0f, 6.0f), 0.0f);

        // --- OBJETO 2: Sillas para expositores (una por stand) ---
        DrawChair(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-9.0f, 0.0f, -6.0f), 180.0f);
        DrawChair(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-9.0f, 0.0f, -2.0f), 180.0f);
        DrawChair(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-9.0f, 0.0f, 2.0f), 180.0f);
        DrawChair(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-9.0f, 0.0f, 6.0f), 180.0f);

        // --- OBJETO 3: Portafolletos giratorios (junto a los stands) ---
        DrawBrochure(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-9.5f, 0.0f, -7.5f));
        DrawBrochure(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-9.5f, 0.0f, 0.5f));

        // --- OBJETO 4: Banderas/tótems con animacion de ondeo ---
        // Tecnica: ondas sinusoidales superpuestas sobre segmentos del mastil
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-5.0f, 0.0f, -8.5f), currentFrame);
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(0.0f, 0.0f, -8.5f), currentFrame);
        DrawFlag(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(5.0f, 0.0f, -8.5f), currentFrame);

        // --- OBJETO 5: Celosia naranja del lobby (elemento jerarquico repetitivo) ---
        // Una fila de 10 laminas verticales naranja-dorado, cada una separada 0.4m
        DrawLattice(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(-13.5f, 0.0f, 0.0f));

        // --- OBJETO 6: Dinosaurio animado (ANIMACION COMPLEJA) ---
        // Modelado jerarquico: torso -> cuello -> cabeza, torso -> brazos, torso -> patas
        // Animacion ciclica de 2s con interpolacion sinusoidal
        DrawDino(VAO, shaderColor, modelLoc, colorLoc, glm::vec3(0.0f, 0.0f, 5.5f), currentFrame);

        // --- ANIMACION SIMPLE 1: Puerta con transformacion jerarquica ---
        // Tecla F: abre/cierra las dos hojas con pivote en borde interior
        // Calcula distancia camara-puerta para apertura automatica opcional
        glm::vec3 doorPos(0.0f, 0.0f, -8.0f);
        DrawDoor(VAO, shaderColor, modelLoc, colorLoc, doorPos, doorAngle);

        // --- ANIMACION SIMPLE 2: Señaletica pulsante (billboarding simulado) ---
        // Iconos de seguridad que escalan con sin(time) y siempre apuntan a la camara
        DrawSignage(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(-8.0f, 3.5f, -4.0f), currentFrame);
        DrawSignage(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(8.0f, 3.5f, -4.0f), currentFrame);

        // --- ANIMACION SIMPLE 3: Lluvia de confeti (sistema de particulas) ---
        // Tecla E activa ~200 quads coloreados que caen desde el techo
        if (confettiActive) {
            DrawConfetti(VAO, shaderColor, modelLoc, colorLoc,
                confettiTimer, confettiActive);
        }

        // ==============  SHADER MODELO (original)  ==============
        shaderModel.Use();

        GLint modelLocM = glGetUniformLocation(shaderModel.Program, "model");
        GLint viewLocM = glGetUniformLocation(shaderModel.Program, "view");
        GLint projLocM = glGetUniformLocation(shaderModel.Program, "projection");

        glUniformMatrix4fv(viewLocM, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLocM, 1, GL_FALSE, glm::value_ptr(projection));

        // Persona (original)
        glm::mat4 modelM(1.0f);
        modelM = glm::translate(modelM, glm::vec3(4.5f, 0.0f, 4.5f));
        modelM = glm::rotate(modelM, glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelM = glm::scale(modelM, glm::vec3(0.42f));
        glUniformMatrix4fv(modelLocM, 1, GL_FALSE, glm::value_ptr(modelM));
        person.Draw(shaderModel);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// ============================================================
//  PRIMITIVAS DE DIBUJO (originales + nueva con rotacion Y)
// ============================================================

void DrawBox(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, scale);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void DrawBoxRotatedX(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1, 0, 0));
    model = glm::scale(model, scale);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// Nueva: rotacion en Y (util para stands, sillas y señaletica orientada a la camara)
void DrawBoxRotatedY(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
    model = glm::scale(model, scale);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// ============================================================
//  OBJETO 1 — STAND MODULAR 3x3 m
//  Estructura de aluminio + paneles PVC blancos + mostrador
// ============================================================
void DrawStand(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float rotY)
{
    // Colores del stand
    glm::vec3 cAlum(0.75f, 0.75f, 0.78f); // Aluminio
    glm::vec3 cPVC(0.96f, 0.96f, 0.96f); // Paneles blancos PVC
    glm::vec3 cMostr(0.88f, 0.88f, 0.88f); // Superficie del mostrador
    glm::vec3 cAzul(0.05f, 0.28f, 0.60f); // Franja de color empresarial

    // Helper lambda para dibujar con rotacion Y del stand completo
    // Usamos una matriz padre y multiplicamos manualmente
    auto DB = [&](glm::vec3 lPos, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 parent(1.0f);
        parent = glm::translate(parent, pos);
        parent = glm::rotate(parent, glm::radians(rotY), glm::vec3(0, 1, 0));

        glm::mat4 child(1.0f);
        child = glm::translate(child, lPos);
        child = glm::scale(child, sc);

        glm::mat4 model = parent * child;
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(col));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        };

    // 4 postes verticales de aluminio (esquinas del 3x3)
    float pw = 0.08f, ph = 2.6f;
    DB(glm::vec3(-1.46f, ph / 2.0f, -1.46f), glm::vec3(pw, ph, pw), cAlum);
    DB(glm::vec3(1.46f, ph / 2.0f, -1.46f), glm::vec3(pw, ph, pw), cAlum);
    DB(glm::vec3(-1.46f, ph / 2.0f, 1.46f), glm::vec3(pw, ph, pw), cAlum);
    DB(glm::vec3(1.46f, ph / 2.0f, 1.46f), glm::vec3(pw, ph, pw), cAlum);

    // Travesanos horizontales superiores (4 lados)
    DB(glm::vec3(0.0f, ph - 0.04f, -1.46f), glm::vec3(3.0f, pw, pw), cAlum); // frente
    DB(glm::vec3(0.0f, ph - 0.04f, 1.46f), glm::vec3(3.0f, pw, pw), cAlum); // fondo
    DB(glm::vec3(-1.46f, ph - 0.04f, 0.0f), glm::vec3(pw, pw, 3.0f), cAlum); // izq
    DB(glm::vec3(1.46f, ph - 0.04f, 0.0f), glm::vec3(pw, pw, 3.0f), cAlum); // der

    // Panel trasero PVC (pared de fondo del stand)
    DB(glm::vec3(0.0f, 1.3f, 1.44f), glm::vec3(2.92f, 2.56f, 0.04f), cPVC);

    // Panel lateral izquierdo PVC
    DB(glm::vec3(-1.44f, 1.3f, 0.0f), glm::vec3(0.04f, 2.56f, 2.92f), cPVC);

    // Panel lateral derecho PVC (solo medio panel, lado abierto)
    DB(glm::vec3(1.44f, 1.3f, 0.5f), glm::vec3(0.04f, 2.56f, 1.9f), cPVC);

    // Franja azul corporativa en panel trasero
    DB(glm::vec3(0.0f, 2.2f, 1.43f), glm::vec3(2.90f, 0.45f, 0.06f), cAzul);

    // Mostrador frontal (L invertida: tablero + frente)
    DB(glm::vec3(0.0f, 1.05f, -1.10f), glm::vec3(2.10f, 0.10f, 0.60f), cMostr); // tablero
    DB(glm::vec3(0.0f, 0.55f, -0.82f), glm::vec3(2.10f, 1.00f, 0.06f), cPVC);    // frente

    // Area de guardado trasera al mostrador (caja baja)
    DB(glm::vec3(0.0f, 0.30f, 0.80f), glm::vec3(1.50f, 0.60f, 0.80f),
        glm::vec3(0.85f, 0.85f, 0.85f));
}

// ============================================================
//  OBJETO 2 — SILLA PARA EXPOSITORES
//  Patas, asiento y respaldo
// ============================================================
void DrawChair(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float rotY)
{
    glm::vec3 cLeg(0.30f, 0.20f, 0.10f); // Madera oscura
    glm::vec3 cSeat(0.20f, 0.45f, 0.75f); // Azul UNAM tapizado
    glm::vec3 cBack(0.18f, 0.40f, 0.70f); // Azul respaldo (un poco mas oscuro)

    auto DB = [&](glm::vec3 lPos, glm::vec3 sc, glm::vec3 col) {
        glm::mat4 parent(1.0f);
        parent = glm::translate(parent, pos);
        parent = glm::rotate(parent, glm::radians(rotY), glm::vec3(0, 1, 0));
        glm::mat4 child(1.0f);
        child = glm::translate(child, lPos);
        child = glm::scale(child, sc);
        glm::mat4 model = parent * child;
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(col));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        };

    // 4 patas
    DB(glm::vec3(-0.22f, 0.23f, -0.22f), glm::vec3(0.06f, 0.46f, 0.06f), cLeg);
    DB(glm::vec3(0.22f, 0.23f, -0.22f), glm::vec3(0.06f, 0.46f, 0.06f), cLeg);
    DB(glm::vec3(-0.22f, 0.23f, 0.22f), glm::vec3(0.06f, 0.46f, 0.06f), cLeg);
    DB(glm::vec3(0.22f, 0.23f, 0.22f), glm::vec3(0.06f, 0.46f, 0.06f), cLeg);

    // Asiento tapizado
    DB(glm::vec3(0.0f, 0.49f, 0.0f), glm::vec3(0.52f, 0.08f, 0.52f), cSeat);

    // Respaldo (inclinado ligeramente con DrawBoxRotatedX)
    // Como usamos DB local, hacemos la inclinacion manual
    {
        glm::mat4 parent(1.0f);
        parent = glm::translate(parent, pos);
        parent = glm::rotate(parent, glm::radians(rotY), glm::vec3(0, 1, 0));

        glm::mat4 child(1.0f);
        child = glm::translate(child, glm::vec3(0.0f, 0.90f, 0.26f));
        child = glm::rotate(child, glm::radians(-8.0f), glm::vec3(1, 0, 0)); // leve inclinacion
        child = glm::scale(child, glm::vec3(0.50f, 0.70f, 0.07f));
        glm::mat4 model = parent * child;
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBack));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // Travesano superior del respaldo
    DB(glm::vec3(0.0f, 1.26f, 0.27f), glm::vec3(0.50f, 0.06f, 0.06f), cLeg);
}

// ============================================================
//  OBJETO 3 — PORTAFOLLETOS GIRATORIO (exhibidor de piso)
//  Poste central + 4 niveles de bandejas
// ============================================================
void DrawBrochure(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos)
{
    glm::vec3 cPost(0.80f, 0.80f, 0.82f); // Cromo/metal
    glm::vec3 cTray(0.92f, 0.92f, 0.92f); // Bandeja gris claro
    glm::vec3 cPaper(0.98f, 0.95f, 0.88f); // Trípticos amarillo claro

    // Poste central
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0, 0.85f, 0),
        glm::vec3(0.06f, 1.70f, 0.06f), cPost);

    // Base (estrella de 2 barras cruzadas + platillo circular aproximado)
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0, 0.05f, 0),
        glm::vec3(0.60f, 0.06f, 0.10f), cPost);
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0, 0.05f, 0),
        glm::vec3(0.10f, 0.06f, 0.60f), cPost);

    // 4 niveles de bandejas (cada una con 4 compartimentos)
    float niveles[4] = { 0.45f, 0.80f, 1.15f, 1.50f };
    for (int n = 0; n < 4; n++) {
        float ny = pos.y + niveles[n];
        // Cruz de bandejas orientadas +X, -X, +Z, -Z
        DrawBox(VAO, s, mL, cL, glm::vec3(pos.x + 0.22f, ny, pos.z),
            glm::vec3(0.38f, 0.04f, 0.12f), cTray);
        DrawBox(VAO, s, mL, cL, glm::vec3(pos.x - 0.22f, ny, pos.z),
            glm::vec3(0.38f, 0.04f, 0.12f), cTray);
        DrawBox(VAO, s, mL, cL, glm::vec3(pos.x, ny, pos.z + 0.22f),
            glm::vec3(0.12f, 0.04f, 0.38f), cTray);
        DrawBox(VAO, s, mL, cL, glm::vec3(pos.x, ny, pos.z - 0.22f),
            glm::vec3(0.12f, 0.04f, 0.38f), cTray);

        // Folletos en cada bandeja (quad delgado encima)
        DrawBox(VAO, s, mL, cL, glm::vec3(pos.x + 0.22f, ny + 0.05f, pos.z),
            glm::vec3(0.32f, 0.06f, 0.10f), cPaper);
        DrawBox(VAO, s, mL, cL, glm::vec3(pos.x - 0.22f, ny + 0.05f, pos.z),
            glm::vec3(0.32f, 0.06f, 0.10f), cPaper);
    }

    // Tope superior cromado
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0, 1.73f, 0),
        glm::vec3(0.12f, 0.06f, 0.12f), cPost);
}

// ============================================================
//  OBJETO 4 — BANDERA/TOTEM CON ANIMACION DE ONDEO
//  Tecnica: la tela se divide en 6 segmentos horizontales.
//  Cada segmento se desplaza en X con sin(time + fase),
//  simulando una onda que viaja desde el mastil hacia el borde.
//  Esta es una animacion SIMPLE (transformaciones con sin/cos).
// ============================================================
void DrawFlag(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float time)
{
    glm::vec3 cPole(0.55f, 0.55f, 0.57f); // Aluminio gris
    glm::vec3 cBlue(0.05f, 0.28f, 0.60f); // Azul UNAM
    glm::vec3 cGold(0.85f, 0.65f, 0.13f); // Dorado UNAM
    glm::vec3 cWhite(0.97f, 0.97f, 0.97f); // Blanco

    // Mastil vertical
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0, 1.5f, 0),
        glm::vec3(0.07f, 3.0f, 0.07f), cPole);

    // Base del mastil (plinto)
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0, 0.1f, 0),
        glm::vec3(0.30f, 0.20f, 0.30f), cPole);

    // Tela dividida en 6 franjas horizontales (cada 0.20 m de ancho)
    // La onda avanza de izquierda (cerca mastil) a derecha (borde libre)
    // Amplitud crece con la distancia al mastil (realismo fisico)
    int   nSegs = 6;
    float segW = 0.20f;       // Ancho de cada segmento
    float flagH = 0.80f;       // Alto de la bandera
    float baseY = pos.y + 2.40f; // Altura donde empieza la bandera
    float baseX = pos.x + 0.035f; // Pega al mastil

    for (int i = 0; i < nSegs; i++) {
        // La fase aumenta hacia el borde libre (onda viaja de mastil a borde)
        float phase = (float)i * 0.55f;
        // Amplitud crece con la distancia al mastil
        float amplitude = 0.022f * (float)(i + 1);
        // Desplazamiento en Z (el viento sopla en Z)
        float waveZ = amplitude * sinf(time * 2.8f + phase);

        // Centro del segmento en X
        float cx = baseX + segW * (i + 0.5f);

        // Franja azul (parte superior 60%)
        DrawBox(VAO, s, mL, cL,
            glm::vec3(cx, baseY + flagH * 0.30f, pos.z + waveZ),
            glm::vec3(segW, flagH * 0.60f, 0.025f),
            cBlue);

        // Franja dorada (parte inferior 40%)
        DrawBox(VAO, s, mL, cL,
            glm::vec3(cx, baseY - flagH * 0.20f, pos.z + waveZ * 0.85f),
            glm::vec3(segW, flagH * 0.40f, 0.025f),
            cGold);
    }
}

// ============================================================
//  OBJETO 5 — CELOSIA NARANJA DEL LOBBY
//  10 laminas verticales naranjas repetidas cada 0.4 m,
//  con profundidad variable (zigzag) para dar perspectiva.
//  Color naranja-dorado caracteristico del lobby del JBS.
// ============================================================
void DrawLattice(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 basePos)
{
    glm::vec3 cOrange(0.85f, 0.45f, 0.08f); // Naranja-dorado
    glm::vec3 cFrame(0.60f, 0.30f, 0.05f); // Marco madera oscura

    int   nSlats = 10;
    float slat_w = 0.12f;
    float slat_h = 6.80f;
    float slat_d = 0.08f;
    float gap = 0.40f;

    // Marco superior e inferior
    DrawBox(VAO, s, mL, cL,
        basePos + glm::vec3((nSlats * gap) / 2.0f - gap / 2.0f, slat_h + 0.08f, 0),
        glm::vec3(nSlats * gap + slat_w, 0.14f, slat_d + 0.04f), cFrame);
    DrawBox(VAO, s, mL, cL,
        basePos + glm::vec3((nSlats * gap) / 2.0f - gap / 2.0f, 0.08f, 0),
        glm::vec3(nSlats * gap + slat_w, 0.14f, slat_d + 0.04f), cFrame);

    // 10 laminas naranjas (zigzag sutil en Z para realismo)
    for (int i = 0; i < nSlats; i++) {
        float xOffset = i * gap;
        // Zigzag: laminas alternas ligeramente adelantadas/atrasadas
        float zOffset = (i % 2 == 0) ? 0.0f : 0.06f;

        DrawBox(VAO, s, mL, cL,
            basePos + glm::vec3(xOffset, slat_h / 2.0f, zOffset),
            glm::vec3(slat_w, slat_h, slat_d),
            cOrange);
    }
}

// ============================================================
//  OBJETO 6 — DINOSAURIO CON ANIMACION COMPLEJA (JERARQUICO)
//  Modelado por partes: torso -> cuello -> cabeza
//                       torso -> brazo izq / brazo der
//                       torso -> pata izq / pata der
//  Animacion ciclica de ~2 s:
//    - Torso: traslacion vertical ±0.10 m + rotacion en Z ±15°
//    - Brazos: oscilacion en Z ±30°
//    - Cabeza: rebote en Y ±5° respecto al cuello
//    - Patas: oscilacion en X ±20° alternada
//  Toda interpolacion con sinf() para suavidad
// ============================================================
void DrawDino(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float time)
{
    // Paleta del dinosaurio inflable (verde limon brillante)
    glm::vec3 cBody(0.30f, 0.72f, 0.15f); // Verde cuerpo
    glm::vec3 cDark(0.20f, 0.52f, 0.08f); // Verde oscuro acentos
    glm::vec3 cBelly(0.85f, 0.95f, 0.70f); // Panza beige claro
    glm::vec3 cEye(0.05f, 0.05f, 0.05f); // Ojos negros
    glm::vec3 cTooth(0.98f, 0.98f, 0.98f); // Dientes blancos

    // === Parametros de animacion (ciclo de 2 s) ===
    float cycle = time * (2.0f * 3.14159f / 2.0f); // 1 ciclo / 2 s
    float bobY = sinf(cycle) * 0.10f;       // Rebote vertical torso
    float tiltZ = sinf(cycle) * 15.0f;       // Inclinacion lateral torso
    float armSwingL = sinf(cycle) * 30.0f;       // Brazo izq
    float armSwingR = sinf(cycle + 3.14159f) * 30.0f;  // Brazo der (opuesto)
    float headBob = sinf(cycle * 2.0f) * 5.0f;       // Cabeza rapida
    float legSwingL = sinf(cycle) * 20.0f;       // Pata izq
    float legSwingR = sinf(cycle + 3.14159f) * 20.0f;  // Pata der (opuesto)

    // === Posicion del torso (con bobbing) ===
    glm::vec3 torsoPos = pos + glm::vec3(0.0f, 1.20f + bobY, 0.0f);

    // --- TORSO ---
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, torsoPos);
        model = glm::rotate(model, glm::radians(tiltZ), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.80f, 0.90f, 0.70f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBody));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // Panza del torso (quad frontal mas claro)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, torsoPos + glm::vec3(0, 0, 0.34f));
        model = glm::rotate(model, glm::radians(tiltZ), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.55f, 0.70f, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBelly));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- CUELLO (hijo del torso) ---
    glm::vec3 neckOffset(0.0f, 0.52f, 0.08f);
    glm::vec3 neckPos = torsoPos + neckOffset;
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, neckPos);
        model = glm::rotate(model, glm::radians(tiltZ * 0.5f), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.35f, 0.45f, 0.35f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBody));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- CABEZA (hijo del cuello) ---
    glm::vec3 headPos = neckPos + glm::vec3(0.0f, 0.45f, 0.0f);
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, headPos);
        model = glm::rotate(model, glm::radians(headBob), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(0.52f, 0.42f, 0.48f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBody));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // Hocico (saliente frontal de la cabeza)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, headPos + glm::vec3(0, -0.06f, 0.26f));
        model = glm::rotate(model, glm::radians(headBob), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(0.38f, 0.20f, 0.22f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cDark));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // Ojos (2 cubitos negros)
    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 model(1.0f);
        model = glm::translate(model,
            headPos + glm::vec3(side * 0.18f, 0.10f, 0.22f));
        model = glm::rotate(model, glm::radians(headBob), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(0.09f, 0.09f, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cEye));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- BRAZO IZQUIERDO (hijo del torso, pivote en hombro) ---
    {
        glm::vec3 shoulderL = torsoPos + glm::vec3(-0.50f, 0.20f, 0.0f);
        glm::mat4 model(1.0f);
        model = glm::translate(model, shoulderL);
        model = glm::rotate(model, glm::radians(tiltZ), glm::vec3(0, 0, 1));
        model = glm::rotate(model, glm::radians(armSwingL), glm::vec3(0, 0, 1));
        // El brazo cuelga hacia abajo desde el pivote
        model = glm::translate(model, glm::vec3(-0.18f, -0.18f, 0.0f));
        model = glm::scale(model, glm::vec3(0.18f, 0.42f, 0.18f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBody));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- BRAZO DERECHO ---
    {
        glm::vec3 shoulderR = torsoPos + glm::vec3(0.50f, 0.20f, 0.0f);
        glm::mat4 model(1.0f);
        model = glm::translate(model, shoulderR);
        model = glm::rotate(model, glm::radians(tiltZ), glm::vec3(0, 0, 1));
        model = glm::rotate(model, glm::radians(armSwingR), glm::vec3(0, 0, 1));
        model = glm::translate(model, glm::vec3(0.18f, -0.18f, 0.0f));
        model = glm::scale(model, glm::vec3(0.18f, 0.42f, 0.18f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBody));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- PATA IZQUIERDA (pivote en cadera) ---
    {
        glm::vec3 hipL = torsoPos + glm::vec3(-0.28f, -0.50f, 0.0f);
        glm::mat4 model(1.0f);
        model = glm::translate(model, hipL);
        model = glm::rotate(model, glm::radians(legSwingL), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0.0f, -0.30f, 0.0f));
        model = glm::scale(model, glm::vec3(0.22f, 0.60f, 0.22f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cDark));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- PATA DERECHA ---
    {
        glm::vec3 hipR = torsoPos + glm::vec3(0.28f, -0.50f, 0.0f);
        glm::mat4 model(1.0f);
        model = glm::translate(model, hipR);
        model = glm::rotate(model, glm::radians(legSwingR), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0.0f, -0.30f, 0.0f));
        model = glm::scale(model, glm::vec3(0.22f, 0.60f, 0.22f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cDark));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // --- COLA (apendice trasero, oscila con el torso) ---
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, torsoPos + glm::vec3(0.0f, -0.10f, -0.45f));
        model = glm::rotate(model, glm::radians(tiltZ * 0.8f), glm::vec3(0, 0, 1));
        model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(0.22f, 0.65f, 0.22f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cDark));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
}

// ============================================================
//  ANIMACION SIMPLE 1 — PUERTAS CON TRANSFORMACION JERARQUICA
//  Tecla F: alterna apertura/cierre.
//  Cada hoja tiene pivote en su borde interior (jamba).
//  Interpolacion ease in-out se calcula en el loop principal.
// ============================================================
void DrawDoor(GLuint VAO, Shader& s, GLint mL, GLint cL, glm::vec3 pos, float openAngle)
{
    glm::vec3 cFrame(0.60f, 0.60f, 0.62f); // Marco aluminio
    glm::vec3 cGlass(0.72f, 0.88f, 0.95f); // Panel vidrio azulado
    glm::vec3 cHandle(0.85f, 0.75f, 0.30f); // Manija dorada

    // Marco fijo (jamba)
    DrawBox(VAO, s, mL, cL, pos + glm::vec3(0.0f, 1.30f, 0.0f),
        glm::vec3(3.0f, 2.60f, 0.10f), cFrame);

    // ---- Hoja IZQUIERDA ----
    // Pivote en X = -0.70 (borde interior izquierdo)
    {
        glm::vec3 pivot = pos + glm::vec3(-0.70f, 1.30f, 0.0f);
        glm::mat4 model(1.0f);
        model = glm::translate(model, pivot);
        // La hoja abre hacia adentro (angulo positivo en Y para la izq)
        model = glm::rotate(model, glm::radians(openAngle), glm::vec3(0, 1, 0));
        // Desplaza la hoja hacia su centro local (0.70 m desde el pivote)
        model = glm::translate(model, glm::vec3(-0.70f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.38f, 2.48f, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cGlass));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Manija hoja izquierda
        glm::mat4 mH(1.0f);
        mH = glm::translate(model, glm::vec3(0.42f, 0.0f, 0.60f)); // local
        // Reusamos la misma jerarquia: copiamos pivot y rotacion
        glm::mat4 pivotM(1.0f);
        pivotM = glm::translate(pivotM, pivot);
        pivotM = glm::rotate(pivotM, glm::radians(openAngle), glm::vec3(0, 1, 0));
        pivotM = glm::translate(pivotM, glm::vec3(-0.28f, 0.0f, 0.06f));
        pivotM = glm::scale(pivotM, glm::vec3(0.06f, 0.22f, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(pivotM));
        glUniform3fv(cL, 1, glm::value_ptr(cHandle));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // ---- Hoja DERECHA ----
    // Pivote en X = +0.70, abre en sentido contrario
    {
        glm::vec3 pivot = pos + glm::vec3(0.70f, 1.30f, 0.0f);
        glm::mat4 model(1.0f);
        model = glm::translate(model, pivot);
        model = glm::rotate(model, glm::radians(-openAngle), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(0.70f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.38f, 2.48f, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cGlass));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Manija hoja derecha
        glm::mat4 pivotM(1.0f);
        pivotM = glm::translate(pivotM, pivot);
        pivotM = glm::rotate(pivotM, glm::radians(-openAngle), glm::vec3(0, 1, 0));
        pivotM = glm::translate(pivotM, glm::vec3(0.28f, 0.0f, 0.06f));
        pivotM = glm::scale(pivotM, glm::vec3(0.06f, 0.22f, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(pivotM));
        glUniform3fv(cL, 1, glm::value_ptr(cHandle));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
}

// ============================================================
//  ANIMACION SIMPLE 2 — SEÑALETICA PULSANTE (BILLBOARDING)
//  Siempre mira a la camara (orientacion en Y calculada).
//  Escala con sin(time) para efecto de "latido".
// ============================================================
void DrawSignage(GLuint VAO, Shader& s, GLint mL, GLint cL,
    glm::vec3 pos, float time)
{
    glm::vec3 cSign(0.10f, 0.72f, 0.25f); // Verde evacuacion
    glm::vec3 cBorder(0.06f, 0.45f, 0.15f); // Borde mas oscuro
    glm::vec3 cIcon(0.95f, 0.95f, 0.95f); // Icono blanco

    // Pulso: escala entre 0.85 y 1.15 con sin(time * 1.5)
    float pulse = 1.0f + 0.15f * sinf(time * 1.5f);

    // Billboarding: calcular angulo para que el quad mire a la camara en Y
    // Usamos GetPosition() que es el getter publico de Camera.h
    float dx = camera.GetPosition().x - pos.x;
    float dz = camera.GetPosition().z - pos.z;
    float angle = atan2f(dx, dz);
    float angleDeg = glm::degrees(angle);

    // Borde del cartel
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(angleDeg), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.38f * pulse, 0.38f * pulse, 0.04f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cBorder));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // Fondo verde del icono
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(angleDeg), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.32f * pulse, 0.32f * pulse, 0.06f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cSign));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    // Figura de persona (silueta simplificada: cuerpo + cabeza)
    {
        // Cuerpo
        glm::mat4 model(1.0f);
        model = glm::translate(model, pos + glm::vec3(0.0f, -0.04f, 0.0f));
        model = glm::rotate(model, glm::radians(angleDeg), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.08f * pulse, 0.14f * pulse, 0.08f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(cIcon));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        // Cabeza
        glm::mat4 modelH(1.0f);
        modelH = glm::translate(modelH, pos + glm::vec3(0.0f, 0.10f, 0.0f));
        modelH = glm::rotate(modelH, glm::radians(angleDeg), glm::vec3(0, 1, 0));
        modelH = glm::scale(modelH, glm::vec3(0.07f * pulse, 0.07f * pulse, 0.07f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(modelH));
        glUniform3fv(cL, 1, glm::value_ptr(cIcon));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
}

// ============================================================
//  ANIMACION SIMPLE 3 — LLUVIA DE CONFETI (SISTEMA DE PARTICULAS)
//  Pool de 200 particulas preinicializado en InitConfetti().
//  Cada quad cae desde Y = techo (8.0), rota en Z y hace fade-out
//  en el ultimo 20% de su vida (3-5 s).
//  Se usa el timer global confettiTimer para calcular posicion.
// ============================================================
void DrawConfetti(GLuint VAO, Shader& s, GLint mL, GLint cL,
    float timer, bool active)
{
    float ceilY = 8.0f;   // Altura del techo (punto de emision)
    float lifetime = 4.0f;  // Vida maxima de cada particula

    for (int i = 0; i < NUM_CONFETTI; i++) {
        const ConfettiParticle& p = confParticles[i];

        // Posicion Y: cae desde el techo a 0
        float t = timer + p.phase;           // tiempo desplazado por fase
        float relTime = fmodf(t, lifetime);         // tiempo ciclico
        float yPos = ceilY - p.speed * relTime * 1.8f;

        // Si ya paso del piso, no dibujar
        if (yPos < 0.0f) continue;

        // Rotacion acumulada en Z
        float rotAngle = p.rotSpeed * relTime;

        // Alpha (fade-out en el ultimo 20% de la vida)
        // Como no tenemos alpha en shaderColor simple, simulamos
        // con una escala pequeña al desaparecer
        float lifeRatio = relTime / lifetime;
        float scale = 0.12f;
        if (lifeRatio > 0.80f) {
            // Reducir escala en el ultimo 20% de vida (simula fade)
            scale *= (1.0f - (lifeRatio - 0.80f) / 0.20f);
        }
        if (scale < 0.005f) continue;

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(p.x, yPos, p.z));
        model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(scale, scale * 0.5f, scale * 0.1f));
        glUniformMatrix4fv(mL, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(cL, 1, glm::value_ptr(
            glm::vec3(p.colorR, p.colorG, p.colorB)));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
}

// ============================================================
//  CALLBACKS (identicos al original)
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

    // Tecla E: activar lluvia de confeti
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        confettiActive = true;
        confettiTimer = 0.0f;
    }

    // Tecla F: alternar apertura/cierre de puertas
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        doorOpen = !doorOpen;
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;
        if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) {
        lastX = (GLfloat)xPos;
        lastY = (GLfloat)yPos;
        firstMouse = false;
    }
    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;
    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll((GLfloat)yOffset);
}
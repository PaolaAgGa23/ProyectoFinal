#include <string>
#include <iostream>
#include <vector>


#include <GL/glew.h>


#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>          


#include "Shader.h"   
#include "Camera.h"   
#include "Model.h"    
#include "stb_image.h" 


const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void DoMovement();


void DrawBox(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color);

void DrawBoxRotatedX(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color, float angle);


Camera camera(glm::vec3(0.0f, 3.0f, 12.0f));

GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];
bool firstMouse = true;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


int main()
{
    // --- Inicializacion de GLFW ---
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Ventana de tamano fijo

    // Crear la ventana de OpenGL
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lobby Universitario - OpenGL", nullptr, nullptr);
    if (!window) {
        std::cout << "Error: no se pudo crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Registrar callbacks 
    glfwSetKeyCallback(window, KeyCallback);       // Teclado
    glfwSetCursorPosCallback(window, MouseCallback); // Movimiento del mouse
    glfwSetScrollCallback(window, ScrollCallback);   // Rueda del mouse 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Inicializacion de GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error: no se pudo inicializar GLEW" << std::endl;
        return EXIT_FAILURE;
    }


    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


    glEnable(GL_DEPTH_TEST);


    Shader shaderColor("Shader/core.vs", "Shader/core.frag");


    Shader shaderModel("Shader/modelLoading.vs", "Shader/modelLoading.frag");


    Model person((char*)"Models/person.obj");


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

    // Crear y configurar el VAO y VBO del cubo
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO); // Genera 1 VAO 
    glGenBuffers(1, &VBO);      // Genera 1 VBO 

    glBindVertexArray(VAO);     // Activar el VAO para configurarlo

    glBindBuffer(GL_ARRAY_BUFFER, VBO);                                          // Activar VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // Subir datos a GPU


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Desactivar VAO


    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f, 200.0f);


    while (!glfwWindowShouldClose(window))
    {

        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();


        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Obtener matriz de vista desde la camara (donde esta mirando)
        glm::mat4 view = camera.GetViewMatrix();


        shaderColor.Use();

        // Obtener las ubicaciones de los uniforms en el shader
        GLint modelLoc = glGetUniformLocation(shaderColor.Program, "model");
        GLint viewLoc = glGetUniformLocation(shaderColor.Program, "view");
        GLint projLoc = glGetUniformLocation(shaderColor.Program, "projection");
        GLint colorLoc = glGetUniformLocation(shaderColor.Program, "color");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(0.0f, -0.15f, 0.0f),
            glm::vec3(30.0f, 0.3f, 20.0f),
            glm::vec3(0.78f, 0.72f, 0.60f));


        float colH = 8.0f;
        std::vector<glm::vec2> colPos = { {-8,-4},{8,-4},{-8,4},{8,4} };
        for (auto& cp : colPos) {
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(cp.x, colH / 2.0f, cp.y),
                glm::vec3(0.9f, colH, 0.9f),
                glm::vec3(0.75f, 0.75f, 0.75f));
        }
        // Techo que conecta las columnas
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(0.0f, 8.15f, 0.0f),
            glm::vec3(18.0f, 0.3f, 10.0f),
            glm::vec3(0.65f, 0.65f, 0.65f));



        int nSteps = 14;
        float stepH = 0.28f;
        float stepD = 0.48f;
        float stairW = 5.20f;
        float stairX = -2.20f;   // mover escalera hacia la izquierda
        float startZ = 3.20f;    // empieza al frente
        float startY = 0.00f;

        glm::vec3 stairTopColor(0.74f, 0.74f, 0.72f);
        glm::vec3 stairFaceColor(0.58f, 0.58f, 0.56f);
        glm::vec3 sideColor(0.86f, 0.86f, 0.84f);


        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX - stairW / 2.0f - 0.25f, 1.05f, 0.05f),
            glm::vec3(0.35f, 2.10f, 7.40f),
            sideColor);

        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX + stairW / 2.0f + 0.25f, 1.05f, 0.05f),
            glm::vec3(0.35f, 2.10f, 7.40f),
            sideColor);


        for (int i = 0; i < nSteps; i++) {
            float y = startY + (i * stepH);
            float z = startZ - (i * stepD);

            // Huella del escalon
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(stairX, y + 0.04f, z),
                glm::vec3(stairW, 0.08f, stepD),
                stairTopColor);


            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(stairX, y - stepH / 2.0f, z + stepD / 2.0f),
                glm::vec3(stairW, stepH, 0.08f),
                stairFaceColor);

            // Relleno bajo el escalon para que la escalera sea solida
            DrawBox(VAO, shaderColor, modelLoc, colorLoc,
                glm::vec3(stairX, y / 2.0f - 0.06f, z),
                glm::vec3(stairW, y + 0.12f, stepD),
                glm::vec3(0.68f, 0.68f, 0.66f));
        }

        // Descanso / segundo piso donde termina la escalera
        float topY = startY + (nSteps * stepH);
        float topZ = startZ - (nSteps * stepD) - 0.20f;

        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX, topY - 0.08f, topZ),
            glm::vec3(stairW + 1.20f, 0.22f, 2.00f),
            glm::vec3(0.72f, 0.72f, 0.70f));

        // Plataforma del segundo piso hacia el fondo
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(stairX, topY - 0.10f, topZ - 1.55f),
            glm::vec3(stairW + 3.00f, 0.25f, 2.20f),
            glm::vec3(0.68f, 0.68f, 0.66f));

        // Losa/puente al nivel del segundo piso
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(0.0f, topY + 0.02f, topZ - 0.60f),
            glm::vec3(10.50f, 0.16f, 0.50f),
            glm::vec3(0.70f, 0.70f, 0.68f));

        // Losa inclinada hacia el segundo nivel.
        // Esta pieza reemplaza el bloque verde: simula la base/piso que continúa hacia arriba.
        DrawBoxRotatedX(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(4.20f, topY + 2.45f, topZ - 2.90f),
            glm::vec3(5.60f, 0.28f, 6.80f),
            glm::vec3(0.70f, 0.70f, 0.68f),
            -42.0f);



        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(5.75f, 0.65f, 2.70f),
            glm::vec3(3.10f, 1.30f, 1.25f),
            glm::vec3(0.80f, 0.10f, 0.05f)); // Rojo

        // Cuerpo lateral (forma la L)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(7.15f, 0.65f, 1.65f),
            glm::vec3(1.25f, 1.30f, 1.20f),
            glm::vec3(0.80f, 0.10f, 0.05f)); // Rojo

        // Tablero blanco encima del mostrador
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(5.95f, 1.34f, 2.55f),
            glm::vec3(3.65f, 0.12f, 1.75f),
            glm::vec3(0.95f, 0.95f, 0.95f)); // Blanco


        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 0.8f, 2.0f),
            glm::vec3(0.9f, 1.6f, 0.9f),
            glm::vec3(0.60f, 0.60f, 0.60f));

        // Plataforma sobre el pedestal (mas ancha que el pedestal)
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 1.62f, 2.0f),
            glm::vec3(1.2f, 0.12f, 1.2f),
            glm::vec3(0.55f, 0.55f, 0.55f));

        // Torso del busto
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 2.15f, 2.0f),
            glm::vec3(0.7f, 0.55f, 0.5f),
            glm::vec3(0.55f, 0.55f, 0.55f));

        // Cabeza del busto
        DrawBox(VAO, shaderColor, modelLoc, colorLoc,
            glm::vec3(1.8f, 2.68f, 2.0f),
            glm::vec3(0.48f, 0.52f, 0.48f),
            glm::vec3(0.58f, 0.58f, 0.58f));


        shaderModel.Use();

        // Obtener uniforms del shader de modelo
        GLint modelLocM = glGetUniformLocation(shaderModel.Program, "model");
        GLint viewLocM = glGetUniformLocation(shaderModel.Program, "view");
        GLint projLocM = glGetUniformLocation(shaderModel.Program, "projection");

        // Enviar matrices de camara y proyeccion (iguales que antes)
        glUniformMatrix4fv(viewLocM, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLocM, 1, GL_FALSE, glm::value_ptr(projection));


        glm::mat4 modelM(1.0f);
        modelM = glm::translate(modelM, glm::vec3(4.5f, 0.0f, 4.5f)); // MÁS AL FRENTE
        modelM = glm::rotate(modelM, glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelM = glm::scale(modelM, glm::vec3(0.42f)); // MÁS GRANDE
        glUniformMatrix4fv(modelLocM, 1, GL_FALSE, glm::value_ptr(modelM));
        person.Draw(shaderModel); // Dibujar el modelo con todas sus mallas



        glfwSwapBuffers(window);
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}


void DrawBox(GLuint VAO, Shader& shader, GLint modelLoc, GLint colorLoc,
    glm::vec3 pos, glm::vec3 scale, glm::vec3 color)
{
    glm::mat4 model(1.0f);                          // Iniciar con identidad
    model = glm::translate(model, pos);             // Mover al lugar indicado
    model = glm::scale(model, scale);               // Aplicar tamano

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Enviar matriz al shader
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));                 // Enviar color al shader

    glBindVertexArray(VAO);           // Activar el VAO del cubo
    glDrawArrays(GL_TRIANGLES, 0, 36); // Dibujar 36 vertices = 12 triangulos = 6 caras
    glBindVertexArray(0);             // Desactivar VAO
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


void DoMovement()
{
    if (keys[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime); // Adelante
    if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime); // Atras
    if (keys[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime); // Izquierda
    if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime); // Derecha
}


void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE); // Cerrar ventana al presionar Escape

    // Actualizar el arreglo de teclas
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   keys[key] = true;  // Tecla presionada
        if (action == GLFW_RELEASE) keys[key] = false; // Tecla soltada
    }
}


void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false; // Solo ignorar el primer evento
    }

    GLfloat xOffset = xPos - lastX;        // Desplazamiento horizontal
    GLfloat yOffset = lastY - yPos;        // Invertido: Y crece hacia abajo en pantalla
    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset); // Rotar camara
}


void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll(yOffset); // Zoom de la camara
}

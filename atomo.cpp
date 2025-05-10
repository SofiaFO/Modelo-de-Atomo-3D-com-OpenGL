#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float cameraDistance = 20.0f; // distância da câmera ao centro
float yaw = -90.0f;   // Ângulo de rotação horizontal (em graus)
float pitch = 0.0f;   // Ângulo de rotação vertical
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mousePressed = false;


// Posição da luz
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    
    out vec3 FragPos;
    out vec3 Normal;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
    )glsl";
    
    const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    
    in vec3 FragPos;
    in vec3 Normal;
    
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 viewPos;
    
    void main() {
        // Ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;
        
        // Diffuse 
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Specular
        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;
            
        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
    )glsl";

    void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<float>& normals, unsigned int sectorCount, unsigned int stackCount) {
    float x, y, z, xy;
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = cos(stackAngle);
        z = sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            x = xy * cos(sectorAngle);
            y = xy * sin(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normals (same as vertex positions for a unit sphere)
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

unsigned int createShaderProgram() {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void generateOrbitXZ(std::vector<float>& orbitVertices, float radius, int segments = 100) {
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        orbitVertices.push_back(radius * cos(angle));  // X
        orbitVertices.push_back(0.0f);                 // Y
        orbitVertices.push_back(radius * sin(angle));  // Z
    }
}

void generateOrbitYZ(std::vector<float>& orbitVertices, float radius, int segments = 100) {
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        orbitVertices.push_back(0.0f);                 // X
        orbitVertices.push_back(radius * cos(angle));  // Y
        orbitVertices.push_back(radius * sin(angle));  // Z
    }
}

void generateOrbitDiagonal(std::vector<float>& orbitVertices, float radius, int segments = 100) {
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        orbitVertices.push_back(x);       // X
        orbitVertices.push_back(0.0f);    // Y
        orbitVertices.push_back(z);       // Z
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraDistance -= (float)yoffset * 0.5f;
    if (cameraDistance < 1.0f) cameraDistance = 1.0f;
    if (cameraDistance > 20.0f) cameraDistance = 20.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            mousePressed = true;
        else if (action == GLFW_RELEASE)
            mousePressed = false;
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mousePressed) {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // invertido: y para cima é positivo
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Limitar pitch para evitar "flipping"
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Átomo", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);


    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST);

    std::vector<float> sphereVertices;
    std::vector<float> sphereNormals;
    std::vector<unsigned int> sphereIndices;
    generateSphere(sphereVertices, sphereIndices, sphereNormals, 40, 40);

    unsigned int sphereVAO, sphereVBO, sphereEBO, sphereNormalsVBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glGenBuffers(1, &sphereNormalsVBO);

    glBindVertexArray(sphereVAO);

    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normais
    glBindBuffer(GL_ARRAY_BUFFER, sphereNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereNormals.size() * sizeof(float), sphereNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    // Índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);


    // Orbitas XZ, YZ e Diagonal (usa XZ também)
    std::vector<float> orbitVerticesXZ, orbitVerticesYZ;
    generateOrbitXZ(orbitVerticesXZ, 2.0f);
    generateOrbitYZ(orbitVerticesYZ, 2.0f);

    unsigned int orbitVAOXZ, orbitVBOXZ;
    glGenVertexArrays(1, &orbitVAOXZ);
    glGenBuffers(1, &orbitVBOXZ);
    glBindVertexArray(orbitVAOXZ);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBOXZ);
    glBufferData(GL_ARRAY_BUFFER, orbitVerticesXZ.size() * sizeof(float), orbitVerticesXZ.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int orbitVAOYZ, orbitVBOYZ;
    glGenVertexArrays(1, &orbitVAOYZ);
    glGenBuffers(1, &orbitVBOYZ);
    glBindVertexArray(orbitVAOYZ);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBOYZ);
    glBufferData(GL_ARRAY_BUFFER, orbitVerticesYZ.size() * sizeof(float), orbitVerticesYZ.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<float> orbitVerticesDiag;
    generateOrbitDiagonal(orbitVerticesDiag, 2.0f);

    unsigned int orbitVAODiag, orbitVBODiag;
    glGenVertexArrays(1, &orbitVAODiag);
    glGenBuffers(1, &orbitVBODiag);
    glBindVertexArray(orbitVAODiag);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBODiag);
    glBufferData(GL_ARRAY_BUFFER, orbitVerticesDiag.size() * sizeof(float), orbitVerticesDiag.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    std::vector<float> orbitVerticesDiagMirror;
    generateOrbitDiagonal(orbitVerticesDiagMirror, 2.0f);

    unsigned int orbitVAODiagMirror, orbitVBODiagMirror;
    glGenVertexArrays(1, &orbitVAODiagMirror);
    glGenBuffers(1, &orbitVBODiagMirror);
    glBindVertexArray(orbitVAODiagMirror);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBODiagMirror);
    glBufferData(GL_ARRAY_BUFFER, orbitVerticesDiagMirror.size() * sizeof(float), orbitVerticesDiagMirror.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int shaderProgram = createShaderProgram();
    unsigned int lightShaderProgram = shaderProgram; // Usamos o mesmo shader para simplificar

    // Posição da câmera (view position)
    glm::vec3 cameraPos(0.0f, 0.0f, 6.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();
        
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraPos;
        cameraPos.x = cameraTarget.x + cameraDistance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        cameraPos.y = cameraTarget.y + cameraDistance * sin(glm::radians(pitch));
        cameraPos.z = cameraTarget.z + cameraDistance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(SCR_WIDTH) / SCR_HEIGHT, 0.1f, 100.0f);
        

        int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3fv(viewPosLoc, 1, &cameraPos[0]);

        glUseProgram(shaderProgram);
        // Configurar propriedades de luz
        glm::vec3 lightPos = cameraPos;
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);

        // Matrizes de visualização e projeção
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

        // Núcleo
        glBindVertexArray(sphereVAO);
        glm::mat4 modelNucleus = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelNucleus[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.0f, 0.0f, 1.0f); // vermelho
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Elétron 1 (horizontal)
        glm::mat4 modelElectron1 = glm::rotate(glm::mat4(1.0f), time, glm::vec3(0, 1, 0));
        modelElectron1 = glm::translate(modelElectron1, glm::vec3(2.0f, 0.0f, 0.0f));
        modelElectron1 = glm::scale(modelElectron1, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelElectron1[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Elétron 2 (vertical)
        glm::mat4 modelElectron2 = glm::rotate(glm::mat4(1.0f), time, glm::vec3(1, 0, 0));
        modelElectron2 = glm::translate(modelElectron2, glm::vec3(0.0f, 0.0f, 2.0f));
        modelElectron2 = glm::scale(modelElectron2, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelElectron2[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glm::mat4 modelElectron3 = glm::mat4(1.0f);
        // Inclinação do plano (primeiro)
        modelElectron3 = glm::rotate(modelElectron3, glm::radians(45.0f), glm::vec3(1, 0, 0));
        modelElectron3 = glm::rotate(modelElectron3, glm::radians(45.0f), glm::vec3(0, 1, 0));

        // Rotação em torno do núcleo
        modelElectron3 = glm::rotate(modelElectron3, time, glm::vec3(0, 1, 0));

        // Translação ao longo da órbita
        modelElectron3 = glm::translate(modelElectron3, glm::vec3(2.0f, 0.0f, 0.0f));

        // Escala
        modelElectron3 = glm::scale(modelElectron3, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelElectron3[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Elétron 4 (diagonal)
        glm::mat4 modelElectronDiag = glm::mat4(1.0f);
        modelElectronDiag = glm::rotate(modelElectronDiag, glm::radians(45.0f), glm::vec3(1, 0, 0));
        modelElectronDiag = glm::rotate(modelElectronDiag, glm::radians(45.0f), glm::vec3(0, 0, 1));
        modelElectronDiag = glm::rotate(modelElectronDiag, time, glm::vec3(0, 1, 0));  // rotação no plano já inclinado
        modelElectronDiag = glm::translate(modelElectronDiag, glm::vec3(2.0f, 0.0f, 0.0f));
        modelElectronDiag = glm::scale(modelElectronDiag, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelElectronDiag[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        glm::mat4 modelElectronDiagMirror = glm::mat4(1.0f);
        modelElectronDiagMirror = glm::rotate(modelElectronDiagMirror, glm::radians(-45.0f), glm::vec3(1, 0, 0));
        modelElectronDiagMirror = glm::rotate(modelElectronDiagMirror, glm::radians(-45.0f), glm::vec3(0, 0, 1));
        modelElectronDiagMirror = glm::rotate(modelElectronDiagMirror, time, glm::vec3(0, 1, 0));
        modelElectronDiagMirror = glm::translate(modelElectronDiagMirror, glm::vec3(2.0f, 0.0f, 0.0f));
        modelElectronDiagMirror = glm::scale(modelElectronDiagMirror, glm::vec3(0.2f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelElectronDiagMirror[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);


        // Órbita XZ
        glBindVertexArray(orbitVAOXZ);
        glm::mat4 orbitModel1 = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &orbitModel1[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawArrays(GL_LINE_LOOP, 0, orbitVerticesXZ.size() / 3);

        // Órbita YZ
        glBindVertexArray(orbitVAOYZ);
        glm::mat4 orbitModel2 = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &orbitModel2[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawArrays(GL_LINE_LOOP, 0, orbitVerticesYZ.size() / 3);

        // Órbita Diagonal (reutilizando XZ com rotação)
        glBindVertexArray(orbitVAOXZ);
        glm::mat4 orbitModelDiagonal = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1, 0, 0));
        orbitModelDiagonal = glm::rotate(orbitModelDiagonal, glm::radians(45.0f), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &orbitModelDiagonal[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawArrays(GL_LINE_LOOP, 0, orbitVerticesXZ.size() / 3);

        // Órbita Diagonal (reutilizando YZ com rotação)
        glBindVertexArray(orbitVAODiag);
        glm::mat4 orbitModelDiag = glm::mat4(1.0f);
        orbitModelDiag = glm::rotate(orbitModelDiag, glm::radians(45.0f), glm::vec3(1, 0, 0)); // inclinação no X
        orbitModelDiag = glm::rotate(orbitModelDiag, glm::radians(45.0f), glm::vec3(0, 0, 1)); // inclinação no Z
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &orbitModelDiag[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.6f, 0.0f); // cor laranja
        glDrawArrays(GL_LINE_LOOP, 0, orbitVerticesDiag.size() / 3);

        // Órbita Diagonal (espelho)
        glBindVertexArray(orbitVAODiagMirror);
        glm::mat4 orbitModelDiagMirror = glm::mat4(1.0f);
        orbitModelDiagMirror = glm::rotate(orbitModelDiagMirror, glm::radians(-45.0f), glm::vec3(1, 0, 0)); // inverso do X
        orbitModelDiagMirror = glm::rotate(orbitModelDiagMirror, glm::radians(-45.0f), glm::vec3(0, 0, 1)); // inverso do Z
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &orbitModelDiagMirror[0][0]);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.6f, 0.0f); // cor laranja igual à anterior
        glDrawArrays(GL_LINE_LOOP, 0, orbitVerticesDiagMirror.size() / 3);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}


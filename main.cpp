#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "std_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "Camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);
glm::mat4 calculateLightSpaceMatrix(glm::vec3 lightPosition, glm::vec3 lightDirection);

// settings
const unsigned int SCR_WIDTH = 1800;
const unsigned int SCR_HEIGHT = 1600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//stylized parameter
glm::vec3 lightPos(-7.0f, -20.0f, 10.0f);
glm::vec4 rim = glm::vec4(0.6f,0.6f,0.6f,0.75f);
float scale_x = 0, scale_y = 0;
float split_x = 0, split_y = 0;
float rotation_x = 0, rotation_y = 0, rotation_z = 0;
float translation_x = 0, translation_y = 0;
float square_num = 0, square_scale = 0;
float specular_scale = 0;

bool normalShading = false;
bool toonShading = true;
bool isCharacter = true;
float model_x = 0, model_y = 0, model_z = 0;
float anis = 0.4, sharp = 0.4;
float wx = 0, wy = 0;
float theta_r = 0;
float r = 0;
float G = 0;
float d = 0;
float bloomexp = 1.0f;
float bloommuti = 1.0f;
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //SET IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    bool show_demo_window = true;
    bool show_another_window = false;
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // build and compile shaders
    Shader shader("./Shader/testcube.glsl", "./Shader/testfragment.glsl");
    Shader roomShader("./Shader/testcube.glsl", "./Shader/testfragment.glsl");
    Shader sceneShader("./Shader/edgeVert.glsl", "./Shader/edgeFrag.glsl");
    Shader shadowShader("./Shader/shadow_vert.glsl", "./Shader/shadow_frag.glsl");
    Shader characterShader("./Shader/shadowRig_vert.glsl", "./Shader/shadowRig_frag.glsl");

    Model myModel("./resources/torso.obj");

    //Model myRoom("./resources/rooms/32.fbx");

    float quadVertices[] = { 
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    sceneShader.use();
    sceneShader.setInt("screenTexture", 0);

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

   /* unsigned int textureColorbuffer[2];
    glGenTextures(2, textureColorbuffer);
    for (unsigned int i = 0; i < 2; i++) 
    {
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureColorbuffer[i], 0);
    }
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);*/

    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const unsigned int SHADOW_WIDTH = 1800, SHADOW_HEIGHT = 1600;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader.use();
    shader.setInt("depthMap", 2);

    roomShader.use();
    shader.setInt("depthMap", 2);

    glm::vec3  centroid;
    for (int i = 0; i < myModel.position.size(); i++)
    {
        centroid.x += myModel.position[i].x;
        centroid.y += myModel.position[i].y;
        centroid.z += myModel.position[i].z;
    }

    centroid /= myModel.position.size();

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------

        glClearColor(0.1, 0.1, 0.1, 0.1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 0.1f, far_plane = 100.5f;
        //lightProjection = glm::ortho(-16.0f, 16.0f, -16.0f, 16.0f, near_plane, far_plane);
        lightProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.1f, 150.0f);
        lightView = glm::lookAt(lightPos,lightPos +  glm::vec3(0.0f,0.0f,-1.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        glm::mat4 characterModel = glm::mat4(1.0f);
       
        characterModel = glm::translate(characterModel, glm::vec3(1.0f, -15.0f, -40.0f));
        //characterModel = glm::rotate(characterModel, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        //characterModel = glm::rotate(characterModel, glm::radians(-90.0f), glm::vec3(0, 0, 1));
        characterModel = glm::scale(characterModel, glm::vec3(0.1f));

        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shadowShader.setMat4("model", characterModel);
        // render scene from light's point of view
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
 
        myModel.Draw(shadowShader);

        glm::mat4 sceneModel = glm::mat4(1.0f);
        sceneModel = glm::translate(sceneModel, glm::vec3(1.0f, 0.0f, -67.0f));
        sceneModel = glm::rotate(sceneModel, glm::radians(280.0f), glm::vec3(1, 0, 0));
        sceneModel = glm::rotate(sceneModel, glm::radians(180.0f), glm::vec3(0, 0, 1));

        shadowShader.setMat4("model", sceneModel);
        //myRoom.Draw(shadowShader);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        glDisable(GL_CULL_FACE);

        shader.use();
        shader.setVec3("myLight.ambient",0.25f,0.25f,0.2f);
        shader.setVec3("myLight.diffuse", 0.5f, 0.4f, 0.3f);
        shader.setVec3("myLight.specular", 1.0f, 0.8f, 0.6f);

        shader.setVec3("myLight.lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setFloat("shiness", 64.0f);
        shader.setVec4("rim", rim);
        shader.setFloat("scale_x", scale_x);
        shader.setFloat("scale_y", scale_y);

        shader.setFloat("rotation_x",rotation_x);
        shader.setFloat("rotation_y", rotation_y);
        shader.setFloat("rotation_z", rotation_z);

        shader.setFloat("translate_x", translation_x);
        shader.setFloat("translate_y", translation_y);

        shader.setFloat("split_x", split_x);
        shader.setFloat("split_y", split_y);

        shader.setFloat("square_num", square_num);
        shader.setFloat("square_scale", square_scale);

        shader.setFloat("specular_scale", specular_scale);

        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setBool("normalShading", normalShading);
        shader.setBool("toonShading", toonShading);

        shader.setFloat("near_plane", near_plane);
        shader.setFloat("far_plane", far_plane);
        shader.setBool("isCharacter", isCharacter);

        shader.setFloat("anis", anis);
        shader.setFloat("sharp", sharp);

        shader.setFloat("wx", wx);
        shader.setFloat("wy", wy);
        shader.setFloat("theta_r", theta_r);
        shader.setFloat("r", r);
        shader.setVec3("centroid", centroid);
        shader.setFloat("Gi", G);
        shader.setFloat("di", d);
        shader.setFloat("bloomexp", bloomexp);
        shader.setFloat("bloommuti", bloommuti);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("model", characterModel);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        //GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1 };
        //glDrawBuffers(1, drawBuffers);
        myModel.Draw(shader);

        roomShader.use();
        roomShader.setVec3("myLight.ambient", 0.2f, 0.2f, 0.2f);
        roomShader.setVec3("myLight.diffuse", 0.5f, 0.5f, 0.5f);
        roomShader.setVec3("myLight.specular", 1.0f, 1.0f, 1.0f);

        roomShader.setVec3("myLight.lightPos", lightPos);
        roomShader.setVec3("viewPos", camera.Position);
        roomShader.setVec4("rim", rim);
        roomShader.setFloat("shiness", 64.0f);
        roomShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        roomShader.setBool("normalShading", normalShading);
        roomShader.setBool("toonShading", toonShading);
        roomShader.setFloat("near_plane", near_plane);
        roomShader.setFloat("far_plane", far_plane);

        roomShader.setMat4("model", sceneModel);
        roomShader.setMat4("view", view);
        roomShader.setMat4("projection", projection);
        shader.setFloat("bloomexp", bloomexp);
        shader.setFloat("bloommuti", bloommuti);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        //drawBuffers[0] = GL_COLOR_ATTACHMENT1;
        //glDrawBuffers(1, drawBuffers);
        //myRoom.Draw(roomShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        sceneShader.use();
       

        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer); 
        sceneShader.setInt("characterTexture", 0); 


        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;

            ImGui::Begin("Parameter Controller"); 

            ImGui::Text("Shading Style");
            ImGui::Checkbox("Blinn-Phong", &normalShading);
            if (normalShading) toonShading = false;
            ImGui::Checkbox("Toon Shading", &toonShading);
            if (toonShading) normalShading = false;
            ImGui::Text("Light Position");
            ImGui::SliderFloat("Light Position X", &lightPos.x,-100.0f,100.0f);
            ImGui::SliderFloat("Light Position Y", &lightPos.y, -100.0f, 100.0f);
            ImGui::SliderFloat("Light Position Z", &lightPos.z, -100.0f, 100.0f);

            ImGui::Text("RimColor");
            ImGui::SliderFloat("RimColor X", &rim.x, 0.0f, 1.0f);
            ImGui::SliderFloat("RimColor Y", &rim.y, 0.0f, 1.0f);
            ImGui::SliderFloat("RimColor Z", &rim.z, 0.0f, 1.0f);
            ImGui::SliderFloat("RimColor A", &rim.a, 0.0f, 1.0f);

            ImGui::Text("Bloom");
            ImGui::SliderFloat("Bloom Exp", &bloomexp, 0.0f, 10.0f);
            ImGui::SliderFloat("Bloom Muti", &bloommuti, 0.0f, 10.0f);

            ImGui::Text("Scale");
            ImGui::SliderFloat("Scale X", &scale_x, -1.0f, 1.0f);
            ImGui::SliderFloat("Scale Y", &scale_y, -1.0f, 1.0f);

            ImGui::Text("Rotation");
            ImGui::SliderFloat("Rotation X", &rotation_x, -180.0f, 180.0f);
            ImGui::SliderFloat("Rotation Y", &rotation_y, -180.0f, 180.0f);
            ImGui::SliderFloat("Rotation Z", &rotation_z, -180.0f, 180.0f);

            ImGui::Text("Translation");
            ImGui::SliderFloat("Translation X", &translation_x, -1.0f, 1.0f);
            ImGui::SliderFloat("Translation Y", &translation_y, -1.0f, 1.0f);

            ImGui::Text("Split");
            ImGui::SliderFloat("Split X", &split_x, 0.0f, 1.0f);
            ImGui::SliderFloat("Split Y", &split_y, 0.0f, 1.0f);

            ImGui::Text("Square");
            ImGui::SliderFloat("Square Num", &square_num, 0.0f, 100.0f);
            ImGui::SliderFloat("Square Scale", &square_scale, 0.0f, 1.0f);

            ImGui::Text("Specular");
            ImGui::SliderFloat("Specular Scale", &specular_scale, 0.0f, 1.0f);

            ImGui::Text("Rig");
            ImGui::SliderFloat("Anisotropy", &anis, 0.0f, 0.99f);
            ImGui::SliderFloat("Sharpness", &sharp, 0.0f, 1.0f);

            ImGui::SliderFloat("wx", &wx, -0.5f, 0.5f);
            ImGui::SliderFloat("wy", &wy, -0.1f, 0.1f);
            ImGui::SliderFloat("theta_r", &theta_r, -180.0f, 180.0f);
            ImGui::SliderFloat("r", &r, -0.0f, 1.0f);
            ImGui::SliderFloat("G", &G, -10.0f, 10.0f);
            ImGui::SliderFloat("d", &d, -10.0f, 10.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
      
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteRenderbuffers(1, &rbo);

    glDeleteFramebuffers(1, &framebuffer);
    glDeleteFramebuffers(1, &depthMap);
    glfwTerminate();
    return 0;
 }


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (glfwGetMouseButton(window, 1) == GLFW_PRESS) 
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }
    
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


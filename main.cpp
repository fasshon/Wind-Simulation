#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <GLFW/glfw3.h>
#include <cmath>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <vector>
#include <cstdlib>
#include <ctime> // added
#include <random>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <thread>
#define M_PI 3.141

void RenderIMGUI();




// -----------------OBJECT TYPE ENUM---------------------
enum ObjectTypes
{
    Circle,
    Square,
    Triangle
};

// ------------------All wind speed shit--------------
static float dt = 0.1f;
static float f = 0.0f;
static float k = 0.1f;
static float dPdx = 0.0f;
static float dPdy = 0.0f;
static float u = 0.0f;
static float v = 0.0f;



static float R;
static float G;
static float B;
static float Size;
// -------------------- Structs --------------------
struct Vector2 { float x, y; };
struct Vector2D { double x, y; };
struct RGB { float R, G, B; };
struct Particle { float X, Y; Vector2 Velocity = { 0, 0 }; RGB color; };
struct Object { float X, Y, Size; RGB color; ObjectTypes ObjectType; };

// -------------------- Globals --------------------
Vector2 ScreenSize = { 1400, 1000 };
GLFWwindow* window = nullptr;
std::vector<Particle> ParticleList;
std::vector<Object> ObjectList;
std::vector<ObjectTypes> ObjectType = {Circle,Square,Triangle };
std::vector<std::string> ObjectTypeString = { "Cricle", "Square", "Triangle" };
int CurrentObjectType = 0;

int ParticleAmount = 20;
int ParticleDistanceX = 25;



//Functions
float WindSpeedEquation(float u, float v, float rho, float dPdx, float dPdy, float f, float k, float dt);
void UpdateWindParticles();
void DrawWindParticles();
Vector2D CheckCursorInWindow();
void DrawObjects();
void AddObjectWINDOWS();
void CheckCollision();
// -------------------- Functions --------------------
GLFWwindow* StartGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    // OpenGL 2.1 (for immediate mode) instead of 4.1 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // Removed Core profile hint

    GLFWwindow* w = glfwCreateWindow((int)ScreenSize.x, (int)ScreenSize.y, "Wind Particle Demo", NULL, NULL);
    if (!w) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(w);
    glfwSwapInterval(1); 
    return w;
}

void DrawCircle(float x, float y, float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= segments; i++) {
        float angle = i * 2.0f * M_PI / segments;
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void DrawWindParticles() {
    for (Particle& p : ParticleList) {
        glColor3f(p.color.R, p.color.G, p.color.B);
        DrawCircle(p.X, p.Y, 5, 100);
        if (p.X > ScreenSize.x)
        {
            p.X = 0;
        }
    }
}

void Render() {
    DrawWindParticles(); DrawObjects(); UpdateWindParticles(); CheckCollision();
}

bool PopulateParticleList() {
    float padding = ScreenSize.y / static_cast<float>(ParticleAmount);
    for (int j = 0; j < ScreenSize.x / ParticleDistanceX; j++)
    {
        for (int i = 0; i < ParticleAmount; i++) {
            Particle p;
            p.Y = padding * i + 50;
            p.X = 0 -(j * ParticleDistanceX); // start on the left
            p.color = { 0.0745f, 0.2745f, 0.0667f };
            ParticleList.push_back(p);
        }
    }

    return true;
}

void UpdateWindParticles()
{
    for (int i = 0; i < ParticleList.size(); i++)
    {
        Particle& CurrentParticle = ParticleList[i];
        CurrentParticle.Velocity.x = WindSpeedEquation(u, v, 1.225f, dPdx, dPdy, f, k, dt);
        CurrentParticle.X += CurrentParticle.Velocity.x;
    }
}
// Wind speed equation
float WindSpeedEquation(float u, float v, float rho, float dPdx, float dPdy, float f, float k, float dt) {
    float du_dt = f * v - (1.0f / rho) * dPdx - k * u;
    float dv_dt = -f * u - (1.0f / rho) * dPdy - k * v;

    float u_new = u + du_dt * dt;
    float v_new = v + dv_dt * dt;

    return std::sqrt(u_new * u_new + v_new * v_new);
}

// -------------------- Main --------------------
int main() {
    srand((unsigned)time(0));

    // Initialize GLFW and window
    window = StartGLFW();
    std::thread Contols(AddObjectWINDOWS);
    Contols.detach();
    if (!window) return -1;

    // Setup OpenGL 2D projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, ScreenSize.x, 0, ScreenSize.y, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    PopulateParticleList();

    // -------------------- ImGui Setup --------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120"); // OpenGL 2.1 uses GLSL 1.20

    // -------------------- Main Loop --------------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Render particles
        Render();

        // Render ImGui
        RenderIMGUI();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // -------------------- Cleanup --------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


void RenderIMGUI() {


    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui window
    ImGui::Begin("Wind Controls");
    ImGui::SliderFloat("dt", &dt, 0.01f, 1.0f);
    ImGui::SliderFloat("Coriolis f", &f, -1.0f, 1.0f);
    ImGui::SliderFloat("Friction k", &k, 0.0f, 1.0f);
    ImGui::SliderFloat("Pressure dPdx", &dPdx, -10.0f, 10.0f);
    ImGui::SliderFloat("Pressure dPdy", &dPdy, -10.0f, 10.0f);


    ImGui::SliderFloat("R", &R, 0.0f, 1.0f);
    ImGui::SliderFloat("G", &G, 0.0f, 1.0f);
    ImGui::SliderFloat("B", &B, 0.0f, 1.0f);
    ImGui::SliderFloat("Size", &Size, 0.0f, 100.0f);
    ImGui::Text("Wind Speed: %.3f m/s", WindSpeedEquation(u, v, 1.225f, dPdx, dPdy, f, k, dt));



    if (ImGui::BeginCombo("ObjectType", ObjectTypeString[CurrentObjectType].c_str())) // label + preview
    {
        for (int n = 0; n < ObjectType.size(); n++)
        {
            bool isSelected = (CurrentObjectType == n);
            if (ImGui::Selectable(ObjectTypeString[n].c_str(), isSelected))
                CurrentObjectType = n;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }


    if (ImGui::Button("Clear Objects"))
    {
        ObjectList.clear();
    }


    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();

}


void AddObjectWINDOWS()
{
    //Windows Verison of Adding Objects
    //Windows Verison of Adding Objects
    //Windows Verison of Adding Objects
    while (true)
    {
        if (GetAsyncKeyState('G') & 0x8000)
        {
            Object Item;
            Vector2D Pos = CheckCursorInWindow();
            Item.X = Pos.x;
            Item.Y = Pos.y;
            Item.color = { R, G, B };
            Item.ObjectType = ObjectType[CurrentObjectType];
            Item.Size = Size;
            ObjectList.push_back(Item);
            std::cout << "added color " << R << ":" << G << ":" << B << std::endl;
            std::cout << "Shouldve added the object hah" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(250)); // ~60 FPS check        }
        }
    }
}




void DrawObjects()
{
    for (int i = 0; i < ObjectList.size(); i++)
    {
        Object& CurrentObject = ObjectList[i];
        switch (CurrentObject.ObjectType)
        {
        case ObjectTypes::Circle:
            glColor3f(CurrentObject.color.R, CurrentObject.color.G, CurrentObject.color.B);
            DrawCircle(CurrentObject.X, CurrentObject.Y, CurrentObject.Size, 100);
            break;
        }
    }
}


Vector2D CheckCursorInWindow() {
    if (!window) return { -1, -1 }; // invalid cursor/window

    // Get window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Get cursor position relative to window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Flip Y coordinate
    ypos = height - ypos;

    // Check if inside bounds
    if (xpos < 0 || xpos > width || ypos < 0 || ypos > height)
        return { -1, -1 }; // cursor is outside

    return { xpos, ypos };
}

void CheckCollision()
{
    for (int i = 0; i < ParticleList.size(); i++)
    {
        Particle& p = ParticleList[i];

        for (int j = 0; j < ObjectList.size(); j++)
        {
            Object& obj = ObjectList[j];

            if (obj.ObjectType != Circle)
                continue; // only check circles

            float dx = p.X - obj.X;
            float dy = p.Y - obj.Y;
            float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared <= obj.Size * obj.Size)
            {
                // Collision detected
                std::cout << "Particle " << i << " collided with Object " << j << std::endl;

                // Example collision response: stop particle at object boundary
                // Compute normalized vector from object to particle
                float dist = std::sqrt(distanceSquared);
                if (dist != 0) {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    p.X = obj.X + nx * obj.Size;
                    p.Y = obj.Y + ny * obj.Size;

                    // Optionally, zero out velocity
                    p.Velocity.x = 0;
                    p.Velocity.y = 0;
                }
            }
        }
    }
}

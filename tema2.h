#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/camera.h"
#include "objects.h"

#define SKY_COLOR NormalizedRGB(135, 206, 235)
#define GRASS_COLOR NormalizedRGB(86, 125, 70)
#define CAMERA_HEIGHT 2.5
#define CAMERA_DISTANCE 4
#define PLAYER_COLOR // leave none for random


namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::mat4 proj, glm::mat4 view);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

     protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;

        Track* track;
        Player* player;
        glm::mat4 player_model_matrix;
        std::vector<Vehicle*> bots;
        std::vector<Tree*> trees;

    };
}   // namespace m1

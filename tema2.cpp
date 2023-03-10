#include "lab_m1/Tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{

    camera = new implemented::Camera();
    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 50.0f);

    // set number of ai vehicles to 0
    Vehicle::reset_nr();

    // initialize track in scene
    {
        track = new Track();
        AddMeshToList(track->create());
    }

    // initialize player in scene
    {
        player = new Player();
        AddMeshToList(player->create(PLAYER_COLOR));
    }

    // initialize grass in scene
    {
        AddMeshToList(tema2::create_plane(GRASS_COLOR));
    }

    // create and initialize the right number of trees in scene
    int tree_id = 0;
    for (glm::vec3 tree_pos : track->get_tree_positions())
    {
        bool too_close = false; // prevent trees being too close
        for (Tree* tree : trees)
        {
            if (tema2::XZ_distance(tree_pos, tree->get_position()) <= 5)
            {
                too_close = true;
                break;
            }
        }

        if (too_close)
            continue;

        // initialize tree
        Tree* tree = new Tree("tree" + to_string(tree_id), tree_pos);
        AddMeshToList(tree->get_crown());
        AddMeshToList(tree->get_trunk());
        trees.push_back(tree);
        tree_id++;
    }

    // initialize shader for horizon effect
    {
        Shader* shader = new Shader("TemaShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void Tema2::FrameStart()
{
    // main viewport
    glClearColor(SKY_COLOR[0], SKY_COLOR[1], SKY_COLOR[2], 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
    player->move(track, bots, deltaTimeSeconds); // compute new player position
    // update camera postion
    glm::vec3 camera_position = player->get_position();
    camera_position[0] -= CAMERA_DISTANCE * cos(player->get_rotation() + M_PI_2);
    camera_position[1] += CAMERA_HEIGHT;
    camera_position[2] -= CAMERA_DISTANCE * sin(player->get_rotation() + M_PI_2);
    camera->Set(camera_position, player->get_position(), glm::vec3(0, 1, 0));

    // render track
    RenderSimpleMesh(meshes["track"], shaders["TemaShader"], glm::mat4(1), projectionMatrix, camera->GetViewMatrix());

    // render terrain (grass)
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.05, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(50, 0, 50));
    RenderSimpleMesh(meshes["grass"], shaders["TemaShader"], modelMatrix, projectionMatrix, camera->GetViewMatrix());
    
    // render player vehicle
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, player->get_position() + glm::vec3(0, 0.15, 0));
    modelMatrix = glm::rotate(modelMatrix, -player->get_rotation(), glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, VEHICLE_SCALE);
    player_model_matrix = modelMatrix;
    RenderSimpleMesh(meshes[player->get_id()], shaders["TemaShader"], modelMatrix, projectionMatrix, camera->GetViewMatrix());

    // add new bot
    Vehicle* bot = tema2::spawn_bot(track, bots, player);
    if (bot)
    {
        AddMeshToList(bot->create());
        bots.push_back(bot);
    }

    // render bot vehicles
    for (Vehicle* bot : bots)
    {
        bot->move(track, deltaTimeSeconds); // update bot position
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, bot->get_position() + glm::vec3(0, 0.15, 0));
        modelMatrix = glm::rotate(modelMatrix, -bot->get_rotation(), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, VEHICLE_SCALE);
        RenderSimpleMesh(meshes[bot->get_id()], shaders["TemaShader"], modelMatrix, projectionMatrix, camera->GetViewMatrix());
    }

    // render trees
    for (Tree* tree : trees)
    {
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, tree->get_position() + glm::vec3(0, 2, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75, 0.75, 0.75));
        RenderSimpleMesh(meshes[tree->get_id() + "crown"], shaders["TemaShader"], modelMatrix, projectionMatrix, camera->GetViewMatrix());

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, tree->get_position() + glm::vec3(0, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25, 1.25, 0.25));
        RenderSimpleMesh(meshes[tree->get_id() + "trunk"], shaders["TemaShader"], modelMatrix, projectionMatrix, camera->GetViewMatrix());
    }
    
    // minimap
    glm::ivec2 resolution = window->GetResolution();
    glViewport(resolution.x - 330, resolution.y - 190, 320, 180);
    {
        // set "new camera" for minimap display
        float ortho_x = 20.0f;
        float ortho_y = 20.0f;

        glm::mat4 projectionMatrix = glm::ortho(-ortho_x, ortho_x, -ortho_y, ortho_y, 0.01f, 200.0f);
        glm::mat4 viewMatrix = glm::lookAt(player->get_position() + glm::vec3(0, 10, 0), player->get_position(), glm::vec3(1, 0, 0));

        // START RENDER STUFF
        // player and bot positions are not updated from here
        RenderSimpleMesh(meshes["track"], shaders["TemaShader"], glm::mat4(1), projectionMatrix, viewMatrix);

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.05, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(50, 0, 50));
        RenderSimpleMesh(meshes["grass"], shaders["TemaShader"], modelMatrix, projectionMatrix, viewMatrix);

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, player->get_position() + glm::vec3(0, 0.15, 0));
        modelMatrix = glm::rotate(modelMatrix, -player->get_rotation(), glm::vec3(0, 0, 0));
        modelMatrix = glm::scale(modelMatrix, VEHICLE_SCALE);
        RenderSimpleMesh(meshes[player->get_id()], shaders["TemaShader"], player_model_matrix, projectionMatrix, viewMatrix);

        for (Vehicle* bot : bots)
        {
            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, bot->get_position() + glm::vec3(0, 0.15, 0));
            modelMatrix = glm::rotate(modelMatrix, -bot->get_rotation(), glm::vec3(0, 1, 0));
            modelMatrix = glm::scale(modelMatrix, VEHICLE_SCALE);
            RenderSimpleMesh(meshes[bot->get_id()], shaders["TemaShader"], modelMatrix, projectionMatrix, viewMatrix);
        }

        for (Tree* tree : trees)
        {
            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tree->get_position() + glm::vec3(0, 2, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75, 0.75, 0.75));
            RenderSimpleMesh(meshes[tree->get_id() + "crown"], shaders["TemaShader"], modelMatrix, projectionMatrix, viewMatrix);

            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tree->get_position() + glm::vec3(0, -2, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25, 1.25, 0.25));
            RenderSimpleMesh(meshes[tree->get_id() + "trunk"], shaders["TemaShader"], modelMatrix, projectionMatrix, viewMatrix);
        }
    }
}


void Tema2::FrameEnd()
{
}



void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::mat4 proj, glm::mat4 view)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    int location = glGetUniformLocation(shader->GetProgramID(), "Model");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    int viewLocation = glGetUniformLocation(shader->GetProgramID(), "View");
    glm::mat4 viewMatrix = view;
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    int projectionLocation = glGetUniformLocation(shader->GetProgramID(), "Projection");
    glm::mat4 projectionMatrix = proj;
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int player_location = glGetUniformLocation(shader->GetProgramID(), "player_screen_pos");
    // get normalized "screen" position of player vehicle and send it to shader
    glm::vec4 player_screen_pos = projectionMatrix * viewMatrix * player_model_matrix * glm::vec4(0, 0, 0, 1);
    glUniform4fv(player_location, 1, glm::value_ptr(player_screen_pos));


    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // player controls
    if (window->KeyHold(GLFW_KEY_W))
        player->set_direction(DIRECTION_FORWARD);
    if (window->KeyHold(GLFW_KEY_S))
        player->set_direction(DIRECTION_REVERSE);
    if (window->KeyHold(GLFW_KEY_A))
        player->steer(STEER_LEFT, deltaTime);
    if (window->KeyHold(GLFW_KEY_D))
        player->steer(STEER_RIGHT, deltaTime);
}


void Tema2::OnKeyPress(int key, int mods)
{
        
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // let inertion do its job
    if (key == GLFW_KEY_W)
    {
        player->set_direction(DIRECTION_IDLE);
        player->reset_accel_timestamp();
    }
    if (key == GLFW_KEY_S)
    {
        player->set_direction(DIRECTION_IDLE);
        player->reset_accel_timestamp();
    }
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}

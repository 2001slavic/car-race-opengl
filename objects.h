#pragma once

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

#include <iostream>
#include <chrono>

#define COLOR_TRACK NormalizedRGB(42, 41, 34)

#define DIRECTION_FORWARD 1
#define DIRECTION_IDLE 0
#define DIRECTION_REVERSE -1

#define STEER_LEFT -1
#define STEER_RIGHT 1

#define VEHICLE_SCALE glm::vec3(0.3, 0.15, 0.5)

#define MAX_BOTS 10
#define BOT_SPAWN_INTERVAL 10
#define BOT_SPEED 2

#define DISCRETIZATION_DENSITY 100

class Track
{
private:
	std::vector<glm::vec3> center_waypoints;
	std::vector<std::vector<glm::vec3>> bot_lanes_waypoints;
	std::vector<std::vector<glm::vec3>> borders;
	std::vector<glm::vec3> tree_positions;
    float width;
	float scale;
    glm::vec3 color;

public:
    Track(float width = 3, float scale = 5, glm::vec3 color = COLOR_TRACK);
	std::vector<glm::vec3> get_center_waypoints();
	Mesh* create();

	std::vector<std::vector<glm::vec3>> get_borders();
	std::vector<std::vector<glm::vec3>> get_bot_waypoints();
	std::vector<glm::vec3> get_tree_positions();
};

class Tree
{
private:
	std::string id;
	glm::vec3 position;
	Mesh* trunk;
	Mesh* crown;
public:
	Tree(std::string id, glm::vec3 position, glm::vec3 crown_color = NormalizedRGB(0, 128, 0), glm::vec3 trunk_color = NormalizedRGB(90, 75, 45));
	std::string get_id();
	glm::vec3 get_position();
	Mesh* get_trunk();
	Mesh* get_crown();

};

// class mainly used for bot vehicles
class Vehicle
{
protected:
	static int nr;
	static std::chrono::system_clock::time_point last_spawned;
	std::string id;
	float top_speed;
	glm::vec3 position;
	float rotation;
	int waypoint;
	int lane;
	std::vector<glm::vec3> hitbox;
	
public:
	Vehicle() = default;
	Vehicle(std::string id, int waypoint, int lane, glm::vec3 position = glm::vec3(0), float top_speed = 1);

	Mesh* create(glm::vec3 color = glm::vec3(NormalizedRGB(rand() % 256, rand() % 256, rand() % 256)));

	std::string get_id();
	glm::vec3 get_position();
	float get_top_speed();
	float get_rotation();
	static int get_nr();
	static std::chrono::system_clock::time_point get_last_spawned();
	std::vector<glm::vec3> get_hitbox();

	static void reset_nr();
	static void reset_last_spawned();
	void hitbox_update();

	void move(Track* track, float deltaTime);
};

class Player : public Vehicle
{
private:
	float speed;
	float acceleration;
	float handling;
	int direction;
	int last_direction;
	float min_speed;
	float mass;
	glm::vec3 target_position;
	std::chrono::system_clock::time_point accel_timestamp;
public:
	Player(float top_speed = 5, float acceleration = 0.1, float handling = 3, float mass = 100);
	void move(Track* track, std::vector<Vehicle*> bots, float deltaTime);
	bool is_on_track(Track* track);
	void steer(int direction, float deltaTime);
	bool collides(std::vector<Vehicle*> bots);

	float get_speed();
	float get_accel();
	std::chrono::system_clock::time_point get_accel_timestamp();
	glm::vec3 get_target_position();

	void set_speed(float speed);
	void reset_accel_timestamp();
	void set_target_position(glm::vec3 position);
	void update_position();
	void set_direction(int direction);
};

namespace tema2
{
	bool cmpf(float A, float B, float epsilon = 0.005f);
	bool cmpv(glm::vec3 A, glm::vec3 B, float epsilon = 0.005f);
	std::vector<glm::vec3> discretize(glm::vec3 p1, glm::vec3 p2, int density = DISCRETIZATION_DENSITY);
	float XZ_distance(glm::vec3 a, glm::vec3 b);
	float XZ_triangle_area(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	Mesh* create_cube(std::string id, glm::vec3 color);
	Vehicle* spawn_bot(Track* track, std::vector<Vehicle*> bots, Player* player);
	bool point_in_quadrilateral(glm::vec3 p, std::vector<glm::vec3> quad);
	glm::vec3 rotate_point_XZ(glm::vec3 pivot, float rotation, glm::vec3 target);
	Mesh* create_plane(glm::vec3 color);
}

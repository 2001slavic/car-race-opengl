#pragma once

#include "objects.h"

Track::Track(float width, float scale, glm::vec3 color)
{
    // define track base points (center)
    this->center_waypoints = {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 0.56),
        glm::vec3(-0.1, 0, 1.4),
        glm::vec3(-0.56, 0, 1.76),
        glm::vec3(-1.2, 0, 1.9),
        glm::vec3(-2.12, 0, 1.86),
        glm::vec3(-2.84, 0, 2.2),
        glm::vec3(-3.1, 0, 2.71),
        glm::vec3(-3.17, 0, 3.39),
        glm::vec3(-2.88, 0, 3.78),
        glm::vec3(-2, 0, 4),
        glm::vec3(0, 0, 4),
        glm::vec3(1.36, 0, 3.88),
        glm::vec3(2.38, 0, 3.78),
        glm::vec3(3.5, 0, 3.54),
        glm::vec3(4.54, 0, 3.06),
        glm::vec3(4.78, 0, 2.54),
        glm::vec3(4.68, 0, 2.08),
        glm::vec3(4.36, 0, 1.32),
        glm::vec3(4.24, 0, 0.52),
        glm::vec3(4.28, 0, 0),
        glm::vec3(4.28, 0, -0.48),
        glm::vec3(4.26, 0, -1.04),
        glm::vec3(3.86, 0, -1.38),
        glm::vec3(3.08, 0, -1.56),
        glm::vec3(2.06, 0, -1.82),
        glm::vec3(1.28, 0, -1.94),
        glm::vec3(0.52, 0, -1.98),
        glm::vec3(0.1, 0, -1.79),
        glm::vec3(0, 0, -1.44),
        glm::vec3(0, 0, -0.56),
    };

    // scale coordinates used for logic
    for (int i = 0; i < this->center_waypoints.size(); i++)
        this->center_waypoints[i] *= scale;
    
    this->bot_lanes_waypoints = std::vector<std::vector<glm::vec3>>(2);
    this->borders = std::vector<std::vector<glm::vec3>>(this->center_waypoints.size() - 1);

    glm::vec3 UP = glm::vec3(0, 1, 0);
    for (int i = 1; i < this->center_waypoints.size(); i++)
    {
        glm::vec3 P1 = this->center_waypoints[i - 1];
        glm::vec3 P2 = this->center_waypoints[i];
        glm::vec3 D = P2 - P1;
        glm::vec3 P = glm::normalize(glm::cross(D, UP));

        // set bot waypoints
        this->bot_lanes_waypoints[0].push_back(P1 - (width / 4) * P);
        this->bot_lanes_waypoints[1].push_back(P1 + (width / 4) * P);

        // set track borders
        this->borders[i - 1].push_back(P1 - (width / 2) * P);
        this->borders[i - 1].push_back(P1 + (width / 2) * P);

        // set possible tree positions
        this->tree_positions.push_back(P1 - width * P);
        this->tree_positions.push_back(P1 + width * P);
        if (i >= 2)
        {
            // set common border points (for more comfortable "player on track" check)
            this->borders[i - 2].push_back(P1 - (width / 2) * P);
            this->borders[i - 2].push_back(P1 + (width / 2) * P);
        }
    }
    // reverse left lane
    std::reverse(this->bot_lanes_waypoints[0].begin(), this->bot_lanes_waypoints[0].end());

    // join last border quadrilateral with first one
    this->borders[this->borders.size() - 1].push_back(this->borders[0][0]);
    this->borders[this->borders.size() - 1].push_back(this->borders[0][1]);

    // set up other trac parameters
    this->width = width;
    this->color = color;
}

// TRACK GETTERS

std::vector<glm::vec3> Track::get_center_waypoints() { return this->center_waypoints; }
std::vector<std::vector<glm::vec3>> Track::get_bot_waypoints() { return this->bot_lanes_waypoints; }
std::vector<glm::vec3> Track::get_tree_positions() { return this->tree_positions; }
std::vector<std::vector<glm::vec3>> Track::get_borders() { return this->borders; }

// create track mesh
Mesh* Track::create()
{
    std::vector<glm::vec3> track_points = center_waypoints;

    std::vector<glm::vec3> base_track;

    // discretize base waypoints
    for (int i = 1; i < track_points.size(); i++)
    {
        glm::vec3 prev = track_points[i - 1];
        glm::vec3 current = track_points[i];

        auto discrete = tema2::discretize(prev, current);

        base_track.insert(base_track.end(), discrete.begin(), discrete.end());
    }

    glm::vec3 last = track_points[track_points.size() - 1];
    glm::vec3 first = track_points[0];

    // also discretize segment from last to first track point, and add to unified vector
    auto discrete = tema2::discretize(last, first);
    base_track.insert(base_track.end(), discrete.begin(), discrete.end());

    std::vector<VertexFormat> track_borders;
    glm::vec3 UP = glm::vec3(0, 1, 0);

    std::vector<unsigned int> indices;

    unsigned int index = 0;

    // compute coordinates for "to draw" borders from discretized base points
    for (int i = 1; i < base_track.size(); i++)
    {
        glm::vec3 P1 = base_track[i - 1];
        glm::vec3 P2 = base_track[i];
        glm::vec3 D = P2 - P1;
        glm::vec3 P = glm::normalize(glm::cross(D, UP));
        glm::vec3 inner = P1 - (width / 2) * P;
        glm::vec3 outer = P1 + (width / 2) * P;

        track_borders.push_back(VertexFormat(inner, color));
        indices.push_back(index);
        index++;
        track_borders.push_back(VertexFormat(outer, color));
        indices.push_back(index);
        index++;

    }

    // add first indices to back for proper track draw
    indices.push_back(0);
    indices.push_back(1);
    

    Mesh* track = new Mesh("track");
    track->SetDrawMode(GL_TRIANGLE_STRIP);

    track->InitFromData(track_borders, indices);

    return track;
}

// tree constructor
Tree::Tree(std::string id, glm::vec3 position, glm::vec3 crown_color, glm::vec3 trunk_color)
{
    this->id = id.c_str();
    this->position = position;

    this->crown = tema2::create_cube(this->id + "crown", crown_color);
    this->trunk = tema2::create_cube(this->id + "trunk", trunk_color);
}

// TREE GETTERS

std::string Tree::get_id() { return this->id; }
glm::vec3 Tree::get_position() { return this->position; }
Mesh* Tree::get_trunk() { return this->trunk; }
Mesh* Tree::get_crown() { return this->crown; }

// I love C++
int Vehicle::nr;
std::chrono::system_clock::time_point Vehicle::last_spawned;

// bot vehicle constructor
Vehicle::Vehicle(std::string id, int waypoint, int lane, glm::vec3 position, float top_speed)
{
    this->id = id.c_str();
    this->waypoint = waypoint;
    this->lane = lane;
    this->top_speed = top_speed;
    this->position = position;
    this->rotation = 0;
    Vehicle::last_spawned = std::chrono::system_clock::now();
    Vehicle::nr++;
    this->hitbox_update();
}

// BOT GETTERS

std::string Vehicle::get_id() { return this->id; }
float Vehicle::get_top_speed() { return this->top_speed; }
float Vehicle::get_rotation() { return this->rotation; }
int Vehicle::get_nr() { return Vehicle::nr; }
std::chrono::system_clock::time_point Vehicle::get_last_spawned() { return Vehicle::last_spawned; }
std::vector<glm::vec3> Vehicle::get_hitbox() { return this->hitbox; }
glm::vec3 Vehicle::get_position() { return this->position; }

// BOT (RE)SETTERS

void Vehicle::reset_nr() { Vehicle::nr = 0; }
void Vehicle::reset_last_spawned() { Vehicle::last_spawned = std::chrono::system_clock::now(); }

// update vehicle hitbox (player or bot)
void Vehicle::hitbox_update()
{
    float x1 = this->position[0] - VEHICLE_SCALE[0];
    float x2 = this->position[0] + VEHICLE_SCALE[0];
    float z1 = this->position[2] - VEHICLE_SCALE[2];
    float z2 = this->position[2] + VEHICLE_SCALE[2];

    std::vector<glm::vec3> hitbox = { glm::vec3(x1, 0, z2),
        glm::vec3(x2, 0, z2),
        glm::vec3(x1, 0 ,z1),
        glm::vec3(x2, 0, z1) };

    // compute the new world position for each hitbox corner based on vehicle rotation
    for (int i = 0; i < hitbox.size(); i++)
        hitbox[i] = tema2::rotate_point_XZ(this->position, this->rotation, hitbox[i]);

    this->hitbox = hitbox; // actually updating hitbox
}

// move bots in world
void Vehicle::move(Track* track, float deltaTime)
{
    glm::vec3 current_waypoint = track->get_bot_waypoints()[this->lane][this->waypoint];
    glm::vec3 next_waypoint;
    int next_waypoint_index;

    // if current waypoint is last
    if (this->waypoint == track->get_bot_waypoints()[this->lane].size() - 1)
    {
        // make a "loop"
        next_waypoint = track->get_bot_waypoints()[this->lane][0];
        next_waypoint_index = 0;
    }
    else
    {
        next_waypoint = track->get_bot_waypoints()[this->lane][this->waypoint + 1];
        next_waypoint_index = this->waypoint + 1;
    }

    // set up proper rotation angle
    glm::vec3 d = next_waypoint - current_waypoint;
    this->rotation = atan2(d[2], d[0]) + M_PI_2;

    glm::vec3 step = this->top_speed * (next_waypoint - current_waypoint) / 100.0f;

    // check if transition to next waypoint is needed
    if (tema2::cmpv(this->position + step, next_waypoint, 0.01))
    {
        this->waypoint = next_waypoint_index;
        this->position = track->get_bot_waypoints()[this->lane][this->waypoint];
        return;
    }

    // update position
    this->position += step;
    this->hitbox_update();
}

// create mesh for vehicle
Mesh* Vehicle::create(glm::vec3 color)
{
    return tema2::create_cube(this->id, color);
}

Player::Player(float top_speed, float acceleration, float handling, float mass)
{
    this->id = "player";
    this->top_speed = top_speed;
    this->speed = 0;
    this->acceleration = acceleration;
    this->direction = DIRECTION_IDLE;
    this->last_direction = DIRECTION_IDLE;
    this->mass = mass;
    this->min_speed = top_speed * 0.3;
    this->handling = handling;
    this->position = glm::vec3(0);
    this->rotation = 0;
    this->target_position = position;
    this->accel_timestamp = std::chrono::system_clock::now();
    this->hitbox_update();
}

// PLAYER GETTERS

float Player::get_accel() { return this->acceleration; }
float Player::get_speed() { return this->speed; }
std::chrono::system_clock::time_point Player::get_accel_timestamp() { return this->accel_timestamp; }
glm::vec3 Player::get_target_position() { return this->target_position; }

// PLAYER SETTERS

void Player::set_speed(float speed) { this->speed = speed; }
void Player::reset_accel_timestamp() { this->accel_timestamp = std::chrono::system_clock::now(); }
void Player::set_target_position(glm::vec3 position) { this->target_position = position; }
void Player::update_position() { this->position = this->target_position; }
void Player::set_direction(int direction) { this->direction = direction; }

// move player vehicle
void Player::move(Track* track, std::vector<Vehicle*> bots, float deltaTime)
{
    
    this->speed = abs(this->speed); // ensure speed is always >= 0

    if (!tema2::cmpf(this->speed, 0))
        this->speed -= 1 / this->mass; // apply friction
    else
    {
        if (this->direction != this->last_direction)
        {
            this->last_direction = this->direction;
            this->reset_accel_timestamp();
        }
    }
    // compute new speed based on acceleration
    float t = std::chrono::duration_cast<std::chrono::milliseconds>(this->accel_timestamp - std::chrono::system_clock::now()).count() / 1000.0f;
    float res_speed = abs((this->acceleration * t) - this->speed);
    
    // apply braking (transitioning from forward direction to reverse, and inverse)
    if (this->direction + this->last_direction == 0 && this->direction && this->last_direction)
    {
        float brake_res_speed = this->speed - this->handling / this->mass;
        if (brake_res_speed <= 0)
            this->speed = 0;
        else
            this->speed = brake_res_speed;
    } // if not brake, accelerate in whatever direction (forward or reverse)
    else if (this->direction == DIRECTION_REVERSE)
        if (res_speed >= this->min_speed)
            this->speed = this->min_speed;
        else
            this->speed = res_speed;
    else if (this->direction == DIRECTION_FORWARD)
        if (res_speed >= this->top_speed)
            this->speed = this->top_speed;
        else
            this->speed = res_speed;

    // update candidate position
    this->target_position[0] = this->position[0] + this->last_direction * cos(this->rotation + M_PI_2) * this->speed * deltaTime;
    this->target_position[2] = this->position[2] + this->last_direction * sin(this->rotation + M_PI_2) * this->speed * deltaTime;
    
    // if candidate position is good, make it the actual position
    if (this->is_on_track(track) && !this->collides(bots))
    {
        this->update_position();
        this->hitbox_update();
    }
    else // else, stop
        this->speed = 0;
}

// check if candidate position is on track
bool Player::is_on_track(Track* track)
{
    for (auto border : track->get_borders())
        if (tema2::point_in_quadrilateral(this->target_position, border))
            return true;
    return false;
}

// apply rotation for steering
void Player::steer(int direction, float deltaTime)
{
    if (this->speed <= 0.5) // do not rotate if player doesn't move
        return;
    float rotation = direction * this->last_direction * ((this->handling * 0.75) / (this->speed * 0.5)) * deltaTime;
    if (this->speed <= 1)
        rotation = direction * this->last_direction * ((this->handling * 0.75)) * deltaTime;
    this->rotation += rotation;
}

// check player collision with all bots
bool Player::collides(std::vector<Vehicle*> bots)
{
    // compute player hitbox based on candidate position
    float x1 = this->target_position[0] - VEHICLE_SCALE[0];
    float x2 = this->target_position[0] + VEHICLE_SCALE[0];
    float z1 = this->target_position[2] - VEHICLE_SCALE[2];
    float z2 = this->target_position[2] + VEHICLE_SCALE[2];

    // pack player hitbox to neat form
    std::vector<glm::vec3> player_hitbox = { glm::vec3(x1, 0, z2),
        glm::vec3(x2, 0, z2),
        glm::vec3(x1, 0 ,z1),
        glm::vec3(x2, 0, z1) };

    // apply rotation to hitbox
    for (int i = 0; i < player_hitbox.size(); i++)
        player_hitbox[i] = tema2::rotate_point_XZ(this->target_position, this->rotation, player_hitbox[i]);

    // check for collisions with bots
    for (Vehicle* bot : bots)
    {
        // check if at least one of player's hitbox corners is inside a bot hitbox
        for (glm::vec3 player_hitbox_point : player_hitbox)
            if (tema2::point_in_quadrilateral(player_hitbox_point, bot->get_hitbox()))
                return true;

        // check if at least one of bot's hitbox corners is inside a player hitbox
        for (glm::vec3 bot_hitbox_point : bot->get_hitbox())
            if (tema2::point_in_quadrilateral(bot_hitbox_point, player_hitbox))
                return true;
    }

    // return false if no collisions detected
    return false;
}
// namespace tema2 START

// compare two floats with required precision
bool tema2::cmpf(float A, float B, float epsilon) {
    return (fabs(A - B) < epsilon);
};

// split a segment to more points
std::vector<glm::vec3> tema2::discretize(glm::vec3 p1, glm::vec3 p2, int density)
{
    std::vector<glm::vec3> res;
    glm::vec3 step = (p2 - p1) / (float)density;

    glm::vec3 current = p1;
    do
    {
        res.push_back(current);
        current += step;
    } while (!cmpv(current, p2));
        

    return res;
}

// compare two glm::vec3's
bool tema2::cmpv(glm::vec3 A, glm::vec3 B, float epsilon)
{
    return tema2::cmpf(A[0], B[0], epsilon) && tema2::cmpf(A[1], B[1], epsilon) && tema2::cmpf(A[2], B[2], epsilon);
}

// compute distance between two vec3's on XZ plane, as Y is almost always 0
float tema2::XZ_distance(glm::vec3 a, glm::vec3 b)
{
    return sqrt(pow(a[0] - b[0], 2) + pow(a[2] - b[2], 2));
}

// area of triangle
float tema2::XZ_triangle_area(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    return abs(a[0] * (b[2] - c[2]) + b[0] * (c[2] - a[2]) + c[0] * (a[2] - b[2])) / 2;
}

// create cube mesh
Mesh* tema2::create_cube(std::string id, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
        // bottom
    {VertexFormat(glm::vec3(-1, -1, -1), color),
    VertexFormat(glm::vec3(-1, -1, 1), color),
    VertexFormat(glm::vec3(1, -1, 1), color),
    VertexFormat(glm::vec3(1, -1, -1), color),
        // up
    VertexFormat(glm::vec3(-1, 1, -1), color),
    VertexFormat(glm::vec3(-1, 1, 1), color),
    VertexFormat(glm::vec3(1, 1, 1), color),
    VertexFormat(glm::vec3(1, 1, -1), color)};

    // bottom
    std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0,
        // up
    4, 5, 6, 6, 7, 4,
        // face
    0, 4, 7, 7, 3, 0,
        // back
    1, 5, 6, 6, 2, 1,
        // left
    1, 5, 4, 4, 0, 1,
        // right
    3, 7, 6, 6, 2, 3 };

    Mesh* cube = new Mesh(id);

    cube->InitFromData(vertices, indices);

    return cube;
}

// create a new bot object if certain conditions are met
Vehicle* tema2::spawn_bot(Track* track, std::vector<Vehicle*> bots, Player* player)
{
    // deny if max bots is reached
    if (Vehicle::get_nr() >= MAX_BOTS)
        return NULL;

    float spawn_dist = 20;
    int lane = rand() % 2;
    // deny if player is too close to bot spawnpoint
    if (tema2::XZ_distance(player->get_position(), track->get_bot_waypoints()[lane][0]) < spawn_dist)
        return NULL;

    // deny if a bot is too close to spawnpoint
    for (Vehicle* bot : bots)
    {
        if (tema2::XZ_distance(bot->get_position(), track->get_bot_waypoints()[lane][0]) < 5)
            return NULL;
    }

    int spawn_interval = BOT_SPAWN_INTERVAL;
    // deny if too little time elapsed since last bot spawn
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - Vehicle::get_last_spawned()).count() < spawn_interval)
        return NULL;

    // if everything ok, create and return new bot
    return new Vehicle("bot" + std::to_string(Vehicle::get_nr()), 0, lane, track->get_bot_waypoints()[lane][0], BOT_SPEED);

}

// checks if a point is inside a quadliteral, used to detect collisions (road, bots)
bool tema2::point_in_quadrilateral(glm::vec3 p, std::vector<glm::vec3> quad)
{
    glm::vec3 A = quad[0];
    glm::vec3 B = quad[1];
    glm::vec3 C = quad[3];
    glm::vec3 D = quad[2];

    float quad_area = tema2::XZ_triangle_area(A, B, D) + tema2::XZ_triangle_area(B, C, D);
    float triangles_area = tema2::XZ_triangle_area(A, p, D) +
        tema2::XZ_triangle_area(D, p, C) +
        tema2::XZ_triangle_area(C, p, B) +
        tema2::XZ_triangle_area(p, B, A);

    return tema2::cmpf(quad_area, triangles_area);
}

// rotate a point around another point on XZ plane
glm::vec3 tema2::rotate_point_XZ(glm::vec3 pivot, float rotation, glm::vec3 target) {
    // translate point back to origin
    target[0] -= pivot[0];
    target[2] -= pivot[2];

    // rotate point
    float newX = target[0] * cos(rotation) - target[2] * sin(rotation);
    float newY = target[0] * sin(rotation) + target[2] * cos(rotation);

    // translate point back
    target[0] = newX + pivot[0];
    target[2] = newY + pivot[2];

    return target;

};

// create a plane mesh (grass)
Mesh* tema2::create_plane(glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // discretize (-1, 0, 1); (-1, 0, -1) segment
    std::vector<glm::vec3> discretized_vertical = tema2::discretize(glm::vec3(-1, 0, 1), glm::vec3(-1, 0, -1));

    unsigned int index = 0; // for indices vector

    // for each point from discretized segment
    for (int i = 1; i < discretized_vertical.size(); i++)
    {
        glm::vec3 v_prev = discretized_vertical[i - 1];
        glm::vec3 v = discretized_vertical[i];

        // discretize "lines" to form a discretized square
        std::vector<glm::vec3> discretized_horizontal_prev = tema2::discretize(v_prev, glm::vec3(1, 0, v_prev[2]));
        std::vector<glm::vec3> discretized_horizontal = tema2::discretize(v, glm::vec3(1, 0, v[2]));

        // form vertices and indices vectors
        for (int j = 0; j < discretized_horizontal.size(); j++)
        {
            vertices.push_back(VertexFormat(discretized_horizontal_prev[j], color));
            vertices.push_back(VertexFormat(discretized_horizontal[j], color));

            indices.push_back(index);
            index++;
            indices.push_back(index);
            index++;
        }
    }


    Mesh* plane = new Mesh("grass");

    plane->SetDrawMode(GL_TRIANGLE_STRIP);

    plane->InitFromData(vertices, indices);

    return plane;
}
#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <list>
#include <vector>
#include <deque>

const float PI = 3.1415926535897932384626f;
const float TIMESTEP = 1.0f / 60.0f;
const float DEACCEL = 0.5f;
const float SHOOT_MOMENTUM = 25.0f;
const float COLL_ROT_ACCEL = 80.0f;

// SharedTeture!
// Sharing reused textures
struct SharedTexture {
	GLuint tex = 0;
	glm::uvec2 image_size;

	SharedTexture(std::string file_path);
	~SharedTexture();
};

// Sprite2D!
// or, informally implementing Sprite class in a way I am familiar with
// so life is easier when drawing a bunch of sprites that has rotation also
// and another thing is to handle drawable life cycle
struct Sprite2D {
	Scene::Drawable *drawable = nullptr;
	std::string name;
	glm::vec4 tint;

	bool is_valid = false;
	bool is_texture_shared = false;

	Sprite2D(Scene &scene);
	Sprite2D(Scene &scene, std::string file_path);
	Sprite2D(Scene &scene, SharedTexture const &shared_texture);
	virtual ~Sprite2D();

	glm::vec2 get_position() const { return position; };
	glm::vec2 get_scale() const { return scale; };
	glm::f32 get_rotation() const { return rotation; };
	glm::f32 get_z() const { return z; };
	virtual void set_position(glm::vec2 new_position);
	virtual void set_scale(glm::vec2 new_scale);
	virtual void set_rotation(glm::f32 new_rotation);
	virtual void set_z(glm::f32 new_z);

	// No copy! no move!
	Sprite2D(Sprite2D const &) = delete;
	Sprite2D &operator=(Sprite2D const &) = delete;
	Sprite2D(Sprite2D &) = delete;
	Sprite2D &operator=(Sprite2D &) = delete;

private:
	GLuint tex = 0;
	// order of applying: scale --> rotation --> position
	glm::vec2 position {0.0f, 0.0f};  // position offset
	glm::vec2 scale {1.0f, 1.0f};     // axis-aligned scale
	glm::f32 rotation {0.0f};   // euler angle
	glm::f32 z {0.0f}; // z index

	// helping with access
	Scene &scene;
	std::list<Scene::Drawable>::iterator drawable_iter;
	std::list<Scene::Transform>::iterator transform_iter;

	// constructor shared
	void add_drawable_and_transform();
};
static int sprite_count = 0;


// RigidBody!
// It will always use a *Circle* collision shape, with configurable radius.
// I have been borrowing Godot's terminology; Though implementation-wise it's quite different.
struct RigidBody {

	glm::vec2 position;     // position of the rigid body
	glm::vec2 velocity;     // velocity!
	float mass = 1.0f;             // mass of object!
	float radius = 0.0f;           // radius of the collision shape
	float rotation = 0.0f;         // radian rotation
	float rotation_speed = 0.0f;   // rotation_speed in radian

	std::string name;       // Name of this thingy.
	Sprite2D *sprite;    	// sprite!

	RigidBody();
	RigidBody(glm::vec2 position, float mass, float radius, Sprite2D *sprite);
	~RigidBody();

	// // _physics_process(), a.k.a. a physics frame
	// void physics_process(float delta);
	// // queue_free, a.k.a. queuing to free this body. I think it would not be implemented at all.
	// void queue_free();
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//helper functions to add stuff
	virtual Sprite2D* add_sprite(std::string file_path, glm::vec2 position, glm::vec2 scale);
	virtual Sprite2D* add_sprite(SharedTexture const &shared_texture, glm::vec2 position, glm::vec2 scale);
	virtual RigidBody* add_body(std::string name, glm::vec2 position, float mass, float radius, Sprite2D *sprite) ;

	virtual void step_physics_frame();
	virtual void generate_bullet();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, shoot;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	
	//camera:
	Scene::Camera *camera = nullptr;

	//sprites!
	std::list<Sprite2D> sprites;
	//bodies!
	std::list<RigidBody> bodies;

	//ROCKET
	RigidBody *rocket = nullptr;

	//action
	size_t shoot_held_frame = 0;
	float physics_frame_delta = 0.0f;
	float shoot_cooldown = 0.0f;
	float shoot_pressed = 0.0f;
	bool shot = false;

	//statistics
	int bullets_shot = 0;
	int rocket_collision_count = 0;

};
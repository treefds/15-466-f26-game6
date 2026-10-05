#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

const float PI = 3.1415926535897932384626f;


// Sprite2D!
// or, informally implementing Sprite class in a way I am familiar with
// so life is easier when drawing a bunch of sprites that has rotation also
// and another thing is to handle drawable life cycle
struct Sprite2D {
	Scene::Drawable *drawable = nullptr;
	std::string name;
	glm::vec4 tint;

	bool is_valid = false;

	Sprite2D(Scene &scene);
	Sprite2D(Scene &scene, std::string file_path);
	virtual ~Sprite2D();

	glm::vec2 get_position() { return position; };
	glm::vec2 get_scale() { return scale; };
	glm::f32 get_rotation() { return rotation; };
	glm::f32 get_z() { return z; };
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


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

	//sprites!

	std::list<Sprite2D> sprites;

};
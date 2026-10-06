#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "GameMap.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <unordered_map>
#include <list>
#include <random>

GLuint plane_program = 0;
Load< MeshBuffer > plane_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("plane.pnct"));
	plane_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});


Load< Scene > main_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("plane.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = plane_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = plane_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

		// force set invisible for existing stuff
		drawable.blended = true;
		drawable.pipeline.set_uniforms = []() {
			glUniform4fv(lit_color_texture_program->TINT_vec4, 1, glm::value_ptr(glm::vec4(0.0f)));
		};

	});
});

Load< SharedTexture > rocket_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/rocket.png"));
});

Load< SharedTexture > asteroid_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/asteroid.png"));
});

Load< SharedTexture > pea_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/pea.png"));
});

Load< SharedTexture > star_yellow_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/star_yellow.png"));
});

Load< SharedTexture > big_star_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/star.png"));
});

Load< SharedTexture > star_blue_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/star_blue.png"));
});

Load< SharedTexture > goal_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/goal.png"));
});

Load< SharedTexture > flame_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/flame.png"));
});

Load< SharedTexture > smoke_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/smoke.png"));
});

Load< SharedTexture > congrat_texture(LoadTagDefault, []() -> SharedTexture const * {
	return new SharedTexture(data_path("assets/congrat.png"));
});

float cross(glm::vec2 a, glm::vec2 b) {
	return a.x * b.y - b.x * a.y;
}

const glm::vec2 GOAL_POS = glm::vec2(50.0f, -10.0f);

PlayMode::PlayMode() : scene(*main_scene), text_renderer("sample", data_path("assets/fonts/NotoSans.ttf")) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//construct level.
	{
		std::mt19937 mt(GameMap::LEVEL_SEED);
		for (size_t y = 0; y < GameMap::MAP_H; ++y) {
			for (size_t x = 0; x < GameMap::MAP_W; ++x) {
				if (!GameMap::LEVEL_1[y * GameMap::MAP_W + x]) continue;
				float sprsize = mt() / float(mt.max()) * 1.0f + 5.0f;
				Sprite2D *spr = add_sprite(*asteroid_texture.value, glm::vec2(0.0f), glm::vec2(sprsize));
				spr->tint.r = spr->tint.b = mt() / float(mt.max()) * 0.25f + 0.75f;
				RigidBody *rock = add_body(
					/* Rock name */ "Rock" + std::to_string(y) + "_" + std::to_string(x),
					/* Position */ glm::vec2(-60.0f +12.8f * x + 3.6f * mt() / float(mt.max()), -32.0f + 10.5f * y + 3.8f * mt() / float(mt.max())),
					/*weight*/ 10.5f + 3.0f * mt() / float(mt.max()),
					/*radius*/ sprsize * 0.75f,
					spr
				);
				rock->rotation = mt() / float(mt.max()) * PI * 2.0f;
			}
		}
	}

	// add a goal
	{
		add_sprite(*goal_texture.value, GOAL_POS, glm::vec2(5.0f));
	}

	//add one cute rocket
	{
		std::mt19937 mt(GameMap::LEVEL_SEED | 0x12b34);
		// const & brings mental damage
		Sprite2D *spr = add_sprite(*rocket_texture.value, glm::vec2(0.0f), glm::vec2(6.0f));
		rocket = add_body("Rocket", glm::vec2(-45.0f, 28.0f), 5.0f, 4.0f, spr);
	}

	//add some random decoration stars
	{
		std::mt19937 mt(GameMap::LEVEL_SEED | 0x12b34);
		for (size_t i = 0; i < 28; ++i) {
			Sprite2D *spr = add_sprite(
				*star_blue_texture.value,
				glm::vec2(-70.0f + mt() / float(mt.max()) * 130.0f, -45.0f + mt() / float(mt.max()) * 80.0f),
				glm::vec2(2.0f + glm::vec2(mt() / float(mt.max()) * 1.5f)));
			spr->tint.a = mt() / float(mt.max()) * 0.25f;
			spr->set_z(3.0f);
		}
		for (size_t i = 0; i < 11; ++i) {
			Sprite2D *spr = add_sprite(
				*star_yellow_texture.value,
				glm::vec2(-70.0f + mt() / float(mt.max()) * 130.0f, -45.0f + mt() / float(mt.max()) * 80.0f),
				glm::vec2(1.0f + glm::vec2(mt() / float(mt.max()) * 1.5f)));
			spr->tint.a = mt() / float(mt.max()) * 0.25f;
			spr->set_z(3.0f);
		}
	}

	{// initialize history
		history = std::unordered_map<std::string, std::list<RBRecord>>();
		for (RigidBody &body : bodies) {
			history[body.name] = std::list<RBRecord>();
			history[body.name].emplace_back(0, body.position, body.velocity, body.mass, body.radius, body.rotation, body.rotation_speed);
		}
	}

	// propulsor? flame
	flame = add_sprite(*flame_texture.value, glm::vec2(0.0f), glm::vec2(6.0f));

	// z-sort sprites
	scene.drawables.sort([](Scene::Drawable const &dr1, Scene::Drawable const &dr2) {return dr1.transform->position.z > dr2.transform->position.z;});
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_ESCAPE) {
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		} else if (evt.key.key == SDLK_A) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			shoot.downs += 1;
			shoot.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_R) {
			rewind.downs += 1;
			rewind.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_1) {
			framestepping_interval = TIMESTEP;
			physics_frame_delta = 0.0f;
			return true;
		} else if (evt.key.key == SDLK_2) {
			framestepping_interval = TIMESTEP * 2;
			physics_frame_delta = 0.0f;
			return true;
		} else if (evt.key.key == SDLK_3) {
			framestepping_interval = TIMESTEP * 5;
			physics_frame_delta = 0.0f;
			return true;
		} else if (evt.key.key == SDLK_4) {
			framestepping_interval = 0.5f;
			physics_frame_delta = 0.0f;
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_SPACE) {
			shoot.pressed = false;
			shoot.downs = 0;
			return true;
		} else if (evt.key.key == SDLK_R) {
			rewind.pressed = false;
			rewind.downs = 0;
			return true;
		} 
	}

	return false;
}

void PlayMode::update(float elapsed) {
	{ // run physics frame for each object in game.
		// elapsed is not stable... so there will be some condition for doing a frame.
		used_time += elapsed;
		physics_frame_delta += elapsed;
		shoot_cooldown = std::max(0.0f, shoot_cooldown - elapsed);

		while (physics_frame_delta > framestepping_interval) {
			physics_frame_delta -= framestepping_interval;

			if (current_frame == 23000) {
				std::cout << "Warning! The game has recorded a lot of frames, which uses a lot of memory.\n"
				             "To avoid burning down someone's computer, the game stops progressing after 24000 frames.\n";
			}
			if (current_frame > 24000 && !rewind.pressed) {

			} else if (!rewind.pressed) {
				shot = false;
				if (shoot_cooldown == 0.0f && shoot.downs) {
					shoot_held_frame++;
				} else if (shoot_cooldown == 0.0f && shoot_held_frame > 0) {
					shot = true;
				}

				step_physics_frame();

				// controls
				if (left.pressed) {
					rocket->rotation_speed += 0.5f * TIMESTEP;
					flaming = 1;

				} else if (right.pressed) {
					rocket->rotation_speed += -0.5f * TIMESTEP;
					flaming = -1;
				} else {
					flaming = 0;
				}
				rocket->rotation_speed = std::clamp(rocket->rotation_speed, -2.0f, 2.0f);

				if (shot) {
					glm::vec2 rocket_facing = glm::vec2(glm::cos(rocket->rotation), glm::sin(rocket->rotation));
					float shoot_factor = 0.4f + 0.6f * std::clamp(shoot_held_frame / 75.0f, 0.0f, 1.0f);
					rocket->velocity += rocket_facing * shoot_factor * SHOOT_MOMENTUM / rocket->mass;
					generate_bullet();
					shoot_cooldown = 0.5f;
					shoot_held_frame = 0;
					bullets_shot++;
				}
				current_frame++;
			} else {
				// rewinding
				if (current_frame > 0) current_frame--;

				for (auto iter = bodies.begin(); iter != bodies.end(); ++iter) {
					// for each list and name
					if (history.find(iter->name) == history.end()) {
						continue;
					}
					std::list<RBRecord> &list = history[iter->name];
					while (!list.empty()) {
						if (list.back().frame <= current_frame) {
							break;
						}
						list.pop_back();
						if (list.empty()) {
							// remove its sprite, O(N) because well anyway
							for (auto spr = sprites.begin(); spr != sprites.end(); ++spr) {
								if (&*spr == iter->sprite) {
									spr = sprites.erase(spr);
									break;
								}
							}
							// remove the object too
							iter = bodies.erase(iter);
						} else {
							// rewind
							RBRecord &record = list.back();
							iter->position = record.position;
							iter->rotation = record.rotation;
							iter->velocity = record.velocity;
							iter->rotation_speed = record.rotation_speed;
						}
					}
				}

			}

			// DO physics recording history
			
			for (RigidBody &body: bodies) {
				if (history.find(body.name) == history.end()) {
					history[body.name] = std::list<RBRecord>();
					history[body.name].emplace_back(current_frame, body.position, body.velocity, body.mass, body.radius, body.rotation, body.rotation_speed);
				} else {
					RBRecord &last = history[body.name].back();
					if (body.position != last.position || body.velocity != last.velocity || last.rotation != body.rotation || body.rotation_speed != last.rotation_speed) {
						history[body.name].emplace_back(current_frame, body.position, body.velocity, body.mass, body.radius, body.rotation, body.rotation_speed);
					}
				}
			}
		}

		// winning condition check
		if (glm::length(rocket->position - GOAL_POS) < 5.5f && !game_clear) {
			game_clear = true;
			add_sprite(*congrat_texture.value, glm::vec2(0.0f, 0.0f), glm::vec2(40.0f));

			text_renderer.text = "Time: " + std::to_string(static_cast<int>(used_time)) + "   #Shot: " + std::to_string(bullets_shot) + "   Damage: " + std::to_string(damage);
			auto texture = text_renderer.Rasterize(0, text_w, text_h);
			text_texture.overwrite(std::move(texture), static_cast<GLsizei>(text_w), static_cast<GLsizei>(text_h));
			win_sprite = add_sprite(text_texture, glm::vec2(5.0f, -20.0f), glm::vec2(text_w * 0.03f, text_h * 0.03f));
			win_sprite->tint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			if (used_time < 50.0f) {
				add_sprite(*big_star_texture.value, glm::vec2(-23.2f, 0.0f), glm::vec2(12.0f));
			}
			if (bullets_shot < 20) {
				add_sprite(*big_star_texture.value, glm::vec2(0.0f, 0.0f), glm::vec2(12.0f));
			}
			if (damage < 100) {
				add_sprite(*big_star_texture.value, glm::vec2(20.8f, 0.0f), glm::vec2(12.0f));
			}
		}
	}

	// --------------------------- AESTHETICS ----------------------------

	{ // update body sprites

		for (auto iter = bodies.begin(); iter != bodies.end(); ++iter) {
			iter->sprite->set_position(iter->position);
			iter->sprite->set_rotation(iter->rotation);
		}
	}

	{ // update smoke sprites
		for (auto smoke = smoke_list.begin(); smoke != smoke_list.end();) {
			(*smoke)->tint.a = (*smoke)->tint.a - elapsed * 0.2f;
			if ((*smoke)->tint.a <= 0.0f) {
				sprites.erase((*smoke));
				smoke = smoke_list.erase(smoke);
			} else {
				 ++smoke;
			}
		}
	}

	{ // update flame sprite
		if (flaming == 0) {
			flame->tint.a = 0.0f;
		} else {
			flame->tint.a = 1.0f;

			glm::vec2 rocket_dir = glm::vec2(glm::cos(rocket->rotation), glm::sin(rocket->rotation));
			glm::vec2 rocket_norm = glm::vec2(-rocket_dir.y, rocket_dir.x);

			flame->set_position(rocket->position - rocket_dir * 1.8f + 1.5f * rocket_norm * static_cast<float>(flaming));
			flame->set_rotation(flaming < 0 ? rocket->rotation : rocket->rotation + PI);
			
		}
	}

	{ // update squeezy rocket sprite
		if (shoot_held_frame > 0) {
			float actual_frame = std::clamp(shoot_held_frame / 1.0f, 0.0f, 75.0f);
			rocket->sprite->set_scale(glm::vec2(6.0f - std::sqrt(actual_frame / 75.0f) * 2.0f, 6.0f));
		} else if (shoot_cooldown > 0.0f) {
			float v = std::clamp(shoot_cooldown - 0.4f, 0.0f, 0.1f);
			rocket->sprite->set_scale(glm::vec2(6.0f - 20 * v, 6.0f + 10 * v));
		}

	}

	if (game_clear) { // if game beaten
		
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}


void PlayMode::step_physics_frame() {
	// first move, the deaccel

	// for every single one of the bodies, move and collide
	for (auto iter = bodies.begin(); iter != bodies.end(); ++iter) {

		float earliest_hit = INFINITY;
		std::list<RigidBody>::iterator eother = bodies.end(); 

		// good thing about circles and straight lines is that everything is easy to calculate
		for (auto other = bodies.begin(); other != bodies.end(); ++other) {
			// skip self
			if (other == iter) {
				continue;
			}
			glm::vec2 pq = other->position - iter->position;  // P--->Q

			// special condition. If already clipping badly, ignore it
			if (glm::length(pq) < (iter->radius + other->radius)) {
				continue;
			}
			
			// if collidible (1): dot(PQ, velocity) >= 0.0f, otherwise they are not approaching
			if (glm::dot(pq, iter->velocity) < 0.0f) {
				continue;
			}
			// if collidable (2): |vec(PQ) dot unit(velocity)| < r_P + r_Q
			float dist = glm::abs(glm::dot(pq, glm::normalize(glm::vec2(iter->velocity.y, -iter->velocity.x))));
			if (!(glm::length(iter->velocity) > 0.0f && dist < iter->radius + other->radius)) {
				continue;   // not satisfied, so continue
			}
			// calculate collision point
			float radius_sum = iter->radius + other->radius;
			glm::vec2 dir = glm::normalize(iter->velocity);   // unit(v)
			
			float phlen = glm::dot(pq, dir);   // P---->H  where dot(PH, HQ) = vec(0)
			//  (2prod - sqrt( 4prod^2 - 4*(QR^2 - PR^2) ) )   / 2
			float collide_length = (phlen - std::sqrt(phlen * phlen - glm::dot(pq, pq) +radius_sum * radius_sum));

			if (collide_length > glm::length(iter->velocity) * TIMESTEP) {
				// would not collide because not moving long enough this round
				continue;
			}
			// record this
			if (collide_length < earliest_hit) {
				earliest_hit = collide_length;
				eother = other;
			}
		}

		glm::vec2 old_velocity = iter->velocity;
		if (eother != bodies.end()) {
			// move by that hit length, reset own velocity and other's velocity
			// for now, use a bad TODO implementation (stop)

			iter->position += glm::normalize(iter->velocity) * earliest_hit;

			// momentum transfer

			glm::vec2 dir = glm::normalize(eother->position - iter->position);
			glm::vec2 norm = glm::vec2(dir.y, -dir.x);

			glm::vec2 v_iter = dot(iter->velocity, dir) * dir;
			glm::vec2 v_other = dot(eother->velocity, dir) * dir;

			// on PQ axis, recalculate velocity;
			// on perpendicular axis, keep velocity unchanged (elastic collision!!!)
			iter->velocity = ((iter->mass - eother->mass) * v_iter + (2 * eother->mass) * v_other ) / (iter->mass + eother->mass) + dot(iter->velocity, norm) * norm;
			eother->velocity = ((eother->mass - iter->mass) * v_other + (2 * iter->mass) * v_iter ) / (iter->mass + eother->mass) + dot(eother->velocity, norm) * norm;

			// as for rotation, we are coming up with arbitrary values for them.
			// rocket has an axis so there will be sort of rotation

			// apply rotational velocity first
			iter->rotation += iter->rotation_speed * TIMESTEP;

			// then, standard conversation of angular momentum
			float rv_iter = iter->rotation_speed;
			float rv_other = eother->rotation_speed;
			iter->rotation_speed = ((iter->mass - eother->mass) * rv_iter + (2 * eother->mass) * rv_other ) / (iter->mass + eother->mass);
			eother->rotation_speed = ((eother->mass - iter->mass) * rv_other + (2 * iter->mass) * rv_iter ) / (iter->mass + eother->mass);

			// then, special stuff related to rockets
			if (iter->name == "Rocket") {
				// direction Rocket is facing
				glm::vec2 iter_facing = glm::vec2(glm::cos(iter->rotation), glm::sin(iter->rotation));
				// cross product of iter_facing and dir(Q-P)
				float cross_product = cross(glm::normalize(dir), iter_facing);

				iter->rotation_speed += cross_product * COLL_ROT_ACCEL * TIMESTEP / iter->mass;
				eother->rotation_speed -= cross_product * COLL_ROT_ACCEL * TIMESTEP / eother->mass;

				rocket_collision_count++;

			} else if (eother->name == "Rocket") {
				// direction Rocket is facing
				glm::vec2 other_facing = glm::vec2(glm::cos(eother->rotation), glm::sin(eother->rotation));
				float cross_product = cross(glm::normalize(dir), other_facing);
				iter->rotation_speed += cross_product * COLL_ROT_ACCEL * TIMESTEP / iter->mass;
				eother->rotation_speed -= cross_product * COLL_ROT_ACCEL * TIMESTEP / eother->mass;

				rocket_collision_count++;
			}
			// apply deaccel: none

			// Aesthetics
			float strength = glm::length(iter->velocity - old_velocity) * iter->mass;
			if (iter->name == "Rocket" || eother->name == "Rocket")
				damage += static_cast<int>(strength);
			add_smoke(*smoke_texture.value, iter->position + dir * iter->radius, glm::vec2(std::sqrt(strength) * 1.0f), strength);

		} else {
			iter->position += iter->velocity * TIMESTEP;
			iter->rotation += iter->rotation_speed * TIMESTEP;
		}

		// friction
		if (glm::length(iter->velocity) > DEACCEL * TIMESTEP) {
			iter->velocity -= DEACCEL * TIMESTEP * glm::normalize(iter->velocity);
		} else {
			iter->velocity = glm::vec2(0.0f);
		}
	}
}


void PlayMode::generate_bullet() {
	if (rocket == nullptr) return;
	glm::vec2 rocket_facing = glm::vec2(glm::cos(rocket->rotation), glm::sin(rocket->rotation));
	Sprite2D *spr = add_sprite(*pea_texture.value, glm::vec2(0.0f), glm::vec2(3.0f));
	RigidBody *body = add_body("Bullet" + std::to_string(bullets_shot), rocket->position - rocket_facing * (rocket->radius + 2.4f), 0.75f, 2.4f, spr);
	float shoot_factor = 0.4f + 0.6f * std::clamp(shoot_held_frame / 75.0f, 0.0f, 1.0f);
	body->velocity = -rocket_facing * shoot_factor * SHOOT_MOMENTUM / body->mass + rocket->velocity;
	body->position += body->velocity * TIMESTEP;  // move away from rocket without collision
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.002f, 0.002f, 0.002f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		std::string text = "A/D to rotate; Hold SPACE then release to fire!";
		text += " shot=" + std::to_string(bullets_shot) + ", damage=" + std::to_string(damage);

		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}



// TEXTURE & SPRITE

SharedTexture::SharedTexture() {
	glGenTextures(1, &tex);
}

SharedTexture::SharedTexture(std::string file_path) {
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// get PNG!
	std::vector<glm::u8vec4> image(0);
	
	load_png(file_path, &image_size, &image,  LowerLeftOrigin);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, image_size.x, image_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

SharedTexture::SharedTexture(std::vector<glm::u8vec4> image, GLsizei w, GLsizei h) {
	image_size.x = w;
	image_size.y = h;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

 void SharedTexture::overwrite(std::vector<glm::u8vec4> image, GLsizei w, GLsizei h) {
	image_size.x = w;
	image_size.y = h;
	glBindTexture(GL_TEXTURE_2D, tex);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

SharedTexture::~SharedTexture() {
	if (tex != 0) {
		glDeleteTextures(1, &tex);
	}
}

Sprite2D::Sprite2D(Scene &scene): scene(scene) {
	add_drawable_and_transform();
	is_valid = true;
}

void Sprite2D::add_drawable_and_transform() {
	// initialize with scene, with texture
	// load mesh and add as drawable, and read sprite file
	static int sprite_count = 0;
	Mesh const &mesh = plane_meshes->lookup("Plane");

	// add a new transform
	scene.transforms.emplace_back();
	Scene::Transform &xform = scene.transforms.back();

	// Complete the transform and drawable init
	xform.name = "Sprite_" + std::to_string(sprite_count++);
	xform.parent = nullptr;
	xform.position = glm::vec3(0.0f);
	xform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	xform.rotation = glm::quat(glm::vec3{0.0f, 0.0f, 0.0f});

	scene.drawables.emplace_back(&xform);
	Scene::Drawable &drawable_ = scene.drawables.back();
	drawable = &drawable_;
	drawable_.pipeline = lit_color_texture_program_pipeline;
	drawable_.pipeline.vao = plane_program;
	drawable_.pipeline.type = mesh.type;
	drawable_.pipeline.start = mesh.start;
	drawable_.pipeline.count = mesh.count;
	drawable_.blended = true;

	// transform and drawable
	transform_iter = --scene.transforms.end();
	drawable_iter = --scene.drawables.end();
}

Sprite2D::Sprite2D(Scene &scene, std::string file_path): scene(scene) {
	add_drawable_and_transform();
	is_valid = true;

	// load texture from PNG file
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// get PNG!
	glm::uvec2 image_size;
	std::vector<glm::u8vec4> image(0);
	
	load_png(file_path, &image_size, &image,  LowerLeftOrigin);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, image_size.x, image_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	drawable->pipeline.textures[0].texture = tex;
	drawable->pipeline.textures[0].target = GL_TEXTURE_2D;

	// Set uniforms to modulate color
	drawable->pipeline.set_uniforms = [this]() {
		glUniform4fv(lit_color_texture_program->TINT_vec4, 1, glm::value_ptr(this->tint));
		glUniform3f(lit_color_texture_program->LIGHT_DIRECTION_vec3, 0.0f, 0.0f, 0.0f);
	};

	tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	is_valid = true;
}

Sprite2D::Sprite2D(Scene &scene, SharedTexture const &shared_texture): scene(scene) {
	add_drawable_and_transform();
	is_valid = true;

	tex = shared_texture.tex;

	drawable->pipeline.textures[0].texture = tex;
	drawable->pipeline.textures[0].target = GL_TEXTURE_2D;

	// Set uniforms to modulate color
	drawable->pipeline.set_uniforms = [this]() {
		glUniform4fv(lit_color_texture_program->TINT_vec4, 1, glm::value_ptr(this->tint));
		glUniform3f(lit_color_texture_program->LIGHT_DIRECTION_vec3, 0.0f, 0.0f, 0.0f);
	};

	tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	is_texture_shared = true;
}

Sprite2D::~Sprite2D() {
	if (!is_valid) return;
	// release texture...
	if (tex != 0 && !is_texture_shared) {
		glDeleteTextures(1, &tex);
	}
	scene.drawables.erase(drawable_iter);
	scene.transforms.erase(transform_iter);

}

void Sprite2D::set_position(glm::vec2 new_position) {
	position = new_position;
	drawable->transform->position.x = position.x;
	drawable->transform->position.y = position.y;
}

void Sprite2D::set_scale(glm::vec2 new_scale) {
	scale = new_scale;
	drawable->transform->scale.x = scale.x;
	drawable->transform->scale.y = scale.y;
}

void Sprite2D::set_rotation(glm::f32 new_rotation) {
	rotation = new_rotation;
	drawable->transform->rotation = glm::quat(glm::vec3(0.0f, 0.0f, rotation));
}

void Sprite2D::set_z(glm::f32 new_z) {
	z = new_z;
	drawable->transform->position.z = z;
}

RigidBody::RigidBody() { }

RigidBody::RigidBody(glm::vec2 position, float mass, float radius, Sprite2D *sprite)
	: position(position), mass(mass), radius(radius), sprite(sprite) {
	velocity = glm::vec2(0.0f);
}

RigidBody::~RigidBody() {
	/* Limitation: it is not cleaning up Sprite2D ref. */
	velocity = glm::vec2(0.0f);
}


/* Helpers */

Sprite2D* PlayMode::add_sprite(std::string file_path, glm::vec2 position, glm::vec2 scale) {
	sprites.emplace_back(scene, data_path("assets/rocket.png"));
	sprites.back().set_position(position);
	sprites.back().set_scale(scale);
	sprites.back().set_rotation(0.0f);
	return &sprites.back();
}

Sprite2D* PlayMode::add_sprite(SharedTexture const &shared_texture, glm::vec2 position, glm::vec2 scale) {
	sprites.emplace_back(scene, shared_texture);
	sprites.back().set_position(position);
	sprites.back().set_scale(scale);
	sprites.back().set_rotation(0.0f);
	return &sprites.back();
}

RigidBody* PlayMode::add_body(std::string name, glm::vec2 position, float mass, float radius, Sprite2D *sprite) {
	bodies.emplace_back(position, mass, radius, sprite);
	bodies.back().name = name;
	return &bodies.back();
}

Sprite2D* PlayMode::add_smoke(SharedTexture const &shared_texture, glm::vec2 position, glm::vec2 scale, float strength) {
	Sprite2D *spr = add_sprite(shared_texture, position, scale);
	
	auto iter = --sprites.end();
	smoke_list.emplace_back(iter);

	spr->tint.a = std::clamp(0.01f * strength, 0.0f, 1.0f);

	return spr;
}
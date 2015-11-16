#ifndef FLUIDSOLVER_H
#define FLUIDSOLVER_H

#include "../../TSBB11/src/common/glm/glm.hpp"
#include <vector>
#include <assert.h>
#include "../../TSBB11/src/fluidDataStructures.h"
#include "../../TSBB11/src/Voxelgrid.h"

#ifndef LIN_SOLVE
#define LIN_SOLVE 20
#endif

class FluidSolver
{
public:
	FluidSolver(Voxelgrid* grid) : m_grid(grid) {};
	~FluidSolver() {};

	void runSimulation(float dt);
private:
	void dens_step(float dt);
	void velocity_step(float dt);

	void diffuse_velocity(float dt);
	void diffuse_density(float dt);
	template <typename T>
	void linjear_solve_helper(float constantData, T& current_center, const T& prev_center, const T& c_left,
		const T& c_right, const T& c_above, const T& c_below, const T& c_near, const T& c_far);
	void diffuse_one_velocity(float constantData, NeighbourVoxels& vox);
	void diffuse_one_density(float constantData, NeighbourVoxels& vox);

	void advect_velocity(float dt);
	void advect_density(float dt);
	void advect_core_function(float someconstant, glm::ivec3 &prev_gridPosition, glm::ivec3 gridPosition, glm::vec3 &pointPosition, const glm::vec3& midVelocity);
	void advect_one_velocity(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox);
	void advect_one_density(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox);

	template <typename T>
	void advect_helper(glm::vec3 point_position, glm::vec3 prev_grid_position,
		T& searched, const T& prev_c_mid_center, const T& prev_near_mid_center, const T& prev_c_top_center,
		const T& prev_near_top_center, const T& prev_c_mid_right, const T& prev_near_mid_right, const T& prev_c_top_right,
		const T& prev_near_top_right);

	void project_velocity(float dt);

	void force_boundries_velocity();
	void force_boundries_density();
	void force_boundries_preassure();
	void force_boundries_divergence();

	Voxelgrid* m_grid;
};
























































/*
class fluidsolver
{
public:
	fluidsolver(int N);
	fluidsolver(int height, int width, int depth);
	~fluidsolver() {};

	void set_bnd(int b, float* x);
	void add_source(float *x, float *s);
	void diffuse(int b, float *x, float *x0, float diff);
	void advect (int b, float *d, float *d0, float *u, float *v, float *w);

	void project (float *u, float *v, float *w, float *p, float *div);

	void dens_step(float *x, float *x0, float *u, float *v, float *w, float diff);
	void vel_step(float *u, float *v, float *w, float *u0, float *v0, float *w0, float visc);

	void print(float* v);
private:
	
	std::vector<glm::vec3> _velocity;
	int _height;
	int _width;
	int _depth;
	int _size;

	float _dt;
};
*/
#endif //fluidsolver

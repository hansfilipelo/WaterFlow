#include "fluidsolver.h"

#include "iostream"

void FluidSolver::diffuse_one_velocity(NeighbourVoxels& vox, float constantData)
{
	//constantData =  a=dt*diff*N*N*N;
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity = 
		(vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity +
		constantData * (vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_velocity +
		vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity +
		vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_velocity +
		vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_velocity +
		vox.voxels[CUBEPOS::BACK_MID_CENTER]->prev_velocity +
		vox.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_velocity)) / (1 + 6 * constantData);
};

void FluidSolver::diffuse_one_density(NeighbourVoxels& vox, float constantData)
{
	//constantData =  a=dt*diff*N*N*N;
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density = 
		(vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->density + 
		constantData * (vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_density +
		vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density + 
		vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_density +
		vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_density + 
		vox.voxels[CUBEPOS::BACK_MID_CENTER]->prev_density + 
		vox.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_density)) / (1 + 6 * constantData);
};


void FluidSolver::diffuse_velocity(float dt)
{

	float someconstant = dt;  // 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getNeighbour(x,y,z);
				diffuse_one_velocity(temp,someconstant);
			}
		}
	}
};

void FluidSolver::diffuse_density(float dt)
{
	float someconstant = dt;  // 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getNeighbour(x,y,z);
				diffuse_one_density(temp,someconstant);
			}
		}
	}
};

//everything from here needs to be double checked
void FluidSolver::advect_velocity(float dt)
{
	float someconstant = dt;
	Voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for(unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getVoxel(x, y, z);
				advect_core_function(someconstant, glm::vec3(x, y, z), prev_gridPosition, pointPosition, temp->velocity);
				advect_one_velocity(prev_gridPosition,pointPosition,temp,someconstant);			
			}
		}
	}	
};

void FluidSolver::advect_one_velocity(glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox, float constantData)
{
	NeighbourVoxels origintemp = m_grid.getNeighbour(prev_grid_position.x, prev_grid_position.y, prev_grid_position.z);
	//pick out how far away from the voxel we stand in to the point
	float s1 = point_position.x - prev_grid_position.x;
	float s0 = 1 - s1;
	float t1 = point_position.y - prev_grid_position.y;
	float t0 = 1 - t1;
	float q1 = point_position.z - prev_grid_position.z;
	float q0 = 1 - q1;

	currentVox->velocity = 
		q0*(
			s0*(
				t0*origintemp.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_velocity)
			+
			s1*(
				t0*origintemp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_RIGHT]->prev_velocity))
		+
		q1*(
			s0*(
				t0*origintemp.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_CENTER]->prev_velocity)
			+
			s1*(
				t0*origintemp.voxels[CUBEPOS::FRONT_MID_RIGHT]->prev_velocity
				+
				t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_RIGHT]->prev_velocity));
};

void FluidSolver::advect_density(float dt)
{
	float someconstant = dt;
	Voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	for (unsigned int x = 1; x < m_grid.XLength - 1; x++)
	{
		for (unsigned int y = 1; y < m_grid.YLength - 1; y++)
		{
			for (unsigned int z = 1; z < m_grid.ZLength - 1; z++)
			{
				temp = m_grid.getVoxel(x, y, z);
				advect_core_function(someconstant,glm::vec3(x,y,z),prev_gridPosition, pointPosition, temp->velocity);
				advect_one_density(prev_gridPosition, pointPosition, temp, someconstant);
			}
		}
	}
};

void FluidSolver::advect_core_function(float someconstant, glm::ivec3 gridPosition, glm::ivec3 &prev_gridPosition, glm::vec3 &pointPosition, const glm::vec3 &midVelocity)
{
	pointPosition = (glm::vec3)gridPosition - someconstant*midVelocity; //might be prev_velocity

	//if we are outsidde the inside voxels clamp it to the edges
	if (pointPosition.x < 0.5) {
		pointPosition.x = 0.5f;
	}
	else if (pointPosition.x >(m_grid.XLength - 1) + 0.5){
		pointPosition.x = (float)(m_grid.XLength - 1) + 0.5f;
	}
	if (pointPosition.y < 0.5) {
		pointPosition.y = 0.5f;
	}
	else if (pointPosition.y >(m_grid.YLength - 1) + 0.5) {
		pointPosition.y = (float)(m_grid.YLength - 1) + 0.5f;
	}
	if (pointPosition.z < 0.5) {
		pointPosition.z = 0.5f;
	}
	else if (pointPosition.z >(m_grid.ZLength - 1) + 0.5) {
		pointPosition.z = (float)(m_grid.ZLength - 1) + 0.5f;
	}
	//finding the previous position on the grid by clamping it to integers
	prev_gridPosition = glm::floor(pointPosition);
};

void FluidSolver::advect_one_density(glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox, float constantData)
{
	NeighbourVoxels origintemp = m_grid.getNeighbour(prev_grid_position.x, prev_grid_position.y, prev_grid_position.z);
	//pick out how far away from the voxel we stand in to the point
	float s1 = point_position.x - prev_grid_position.x;
	float s0 = 1 - s1;
	float t1 = point_position.y - prev_grid_position.y;
	float t0 = 1 - t1;
	float q1 = point_position.z - prev_grid_position.z;
	float q0 = 1 - q1;

	currentVox->density =
		q0*(
		s0*(
		t0*origintemp.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density
		+
		t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_density)
		+
		s1*(
		t0*origintemp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density
		+
		t1*origintemp.voxels[CUBEPOS::CURRENT_BOTTOM_RIGHT]->prev_density))
		+
		q1*(
		s0*(
		t0*origintemp.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_density
		+
		t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_CENTER]->prev_density)
		+
		s1*(
		t0*origintemp.voxels[CUBEPOS::FRONT_MID_RIGHT]->prev_density
		+
		t1*origintemp.voxels[CUBEPOS::FRONT_BOTTOM_RIGHT]->prev_density));
};


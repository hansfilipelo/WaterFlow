#include "fluidsolver.h"

#include "iostream"

void FluidSolver::diffuse_one_velocity(float constantData, NeighbourVoxels& vox)
{
	//constantData =  a=dt*diff*N*N*N;
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity = 
		(vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity +
		constantData *
			(vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_velocity +
			vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_velocity +
			vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_velocity +
			vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_velocity +
			vox.voxels[CUBEPOS::BACK_MID_CENTER]->prev_velocity +
			vox.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_velocity)
		) / (1 + 6 * constantData);
};

void FluidSolver::diffuse_one_density(float constantData, NeighbourVoxels& vox)
{
	//constantData =  a=dt*diff*N*N*N;
	vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->prev_density = 
		(vox.voxels[CUBEPOS::CURRENT_MID_CENTER]->density + 
		constantData * 
			(vox.voxels[CUBEPOS::CURRENT_MID_LEFT]->prev_density +
			vox.voxels[CUBEPOS::CURRENT_MID_RIGHT]->prev_density + 
			vox.voxels[CUBEPOS::CURRENT_TOP_CENTER]->prev_density +
			vox.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->prev_density + 
			vox.voxels[CUBEPOS::BACK_MID_CENTER]->prev_density + 
			vox.voxels[CUBEPOS::FRONT_MID_CENTER]->prev_density)
		) / (1 + 6 * constantData);
};

void FluidSolver::diffuse_velocity(float dt)
{
	float someconstant = dt;  // 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int k = 0; k < LIN_SOLVE; k++)
	{
		for (unsigned int z = 1; z < m_grid.getZMaxSize() - 1; z++)
		{
			for (unsigned int y = 1; y < m_grid.getYMaxSize() - 1; y++)
			{
				for (unsigned int x = 1; x < m_grid.getXMaxSize() - 1; x++)
				{
					temp = m_grid.getNeighbour(x, y, z);
					diffuse_one_velocity(someconstant, temp);
				}
			}
		}
		//set bounds for velocity
		force_boundries_velocity();
	} //end of Lin solver
};

void FluidSolver::diffuse_density(float dt)
{
	float someconstant = dt;  // 
	NeighbourVoxels temp;
	//not including borders
	for (unsigned int k = 0; k < LIN_SOLVE; k++)
	{
		for (unsigned int z = 1; z < m_grid.getZMaxSize() - 1; z++)
		{
			for (unsigned int y = 1; y < m_grid.getYMaxSize() - 1; y++)
			{
				for (unsigned int x = 1; x < m_grid.getXMaxSize() - 1; x++)
				{
					temp = m_grid.getNeighbour(x, y, z);
					diffuse_one_density(someconstant, temp);
				}
			}
		}
		//set bounds for density
		force_boundries_density();
	} //end of lin solver
};

//Advect moves stuff around in the system
void FluidSolver::advect_velocity(float dt)
{
	float someconstant = dt;
	Voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	for (unsigned int z = 1; z < m_grid.getZMaxSize() - 1; z++)
	{
		for (unsigned int y = 1; y < m_grid.getYMaxSize() - 1; y++)
		{
			for (unsigned int x = 1; x < m_grid.getXMaxSize() - 1; x++)
			{
				temp = m_grid.getVoxel(x, y, z);
				advect_core_function(someconstant, prev_gridPosition, glm::ivec3(x, y, z), pointPosition, temp->velocity);
				advect_one_velocity(someconstant, prev_gridPosition, pointPosition, temp);
			}
		}
	}	
};

void FluidSolver::advect_density(float dt)
{
	float someconstant = dt;
	Voxel* temp;
	glm::ivec3 prev_gridPosition;
	glm::vec3 pointPosition;
	//not including borders
	for (unsigned int z = 1; z < m_grid.getZMaxSize() - 1; z++)
	{
		for (unsigned int y = 1; y < m_grid.getYMaxSize() - 1; y++)
		{
			for (unsigned int x = 1; x < m_grid.getXMaxSize() - 1; x++)
			{
				temp = m_grid.getVoxel(x, y, z);
				advect_core_function(someconstant, prev_gridPosition, glm::vec3(x, y, z), pointPosition, temp->velocity);
				advect_one_density(someconstant, prev_gridPosition, pointPosition, temp);
			}
		}
	}
};

void FluidSolver::advect_core_function(float someconstant, glm::ivec3 &prev_gridPosition, glm::ivec3 gridPosition, glm::vec3 &pointPosition, const glm::vec3 &midVelocity)
{
	pointPosition = (glm::vec3)gridPosition - someconstant*midVelocity; //might be prev_velocity

	//if we are outsidde the inside voxels clamp it to the edges
	if (pointPosition.x < 0.5) {
		pointPosition.x = 0.5f;
	}
	else if (pointPosition.x >(m_grid.getXMaxSize() - 1) + 0.5){
		pointPosition.x = (float)(m_grid.getXMaxSize() - 1) + 0.5f;
	}
	if (pointPosition.y < 0.5) {
		pointPosition.y = 0.5f;
	}
	else if (pointPosition.y >(m_grid.getYMaxSize() - 1) + 0.5) {
		pointPosition.y = (float)(m_grid.getYMaxSize() - 1) + 0.5f;
	}
	if (pointPosition.z < 0.5) {
		pointPosition.z = 0.5f;
	}
	else if (pointPosition.z >(m_grid.getZMaxSize() - 1) + 0.5) {
		pointPosition.z = (float)(m_grid.getZMaxSize() - 1) + 0.5f;
	}
	//finding the previous position on the grid by clamping it to integers
	prev_gridPosition = glm::floor(pointPosition);
};

void FluidSolver::advect_one_velocity(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox)
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

void FluidSolver::advect_one_density(float constantData, glm::ivec3 prev_grid_position, glm::vec3 point_position, Voxel* currentVox)
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

void FluidSolver::project_velocity(float dt)
{
	NeighbourVoxels temp;
	for (unsigned int z = 0; z < m_grid.getZMaxSize(); z++)
	{
		for (unsigned int y = 0; y < m_grid.getYMaxSize(); y++)
		{
			for (unsigned int x = 0; x < m_grid.getXMaxSize(); x++)
			{
				temp = m_grid.getNeighbour(x, y, z);
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->divergence = -0.5f*(
					temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->velocity.x - temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->velocity.x
					+ temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->velocity.y - temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->velocity.y
					+ temp.voxels[CUBEPOS::FRONT_MID_CENTER]->velocity.z - temp.voxels[CUBEPOS::BACK_MID_CENTER]->velocity.z); /* /N */
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure = 0.0f;
			}
		}
	}
	//force boundries for divergence
	force_boundries_divergence();
	//force boundries for preassure
	force_boundries_preassure();
	for (unsigned int k= 0; k < LIN_SOLVE; k++)
	{
		for (unsigned int z = 0; z < m_grid.getZMaxSize(); z++)
		{
			for (unsigned int y = 0; y < m_grid.getYMaxSize(); y++)
			{
				for (unsigned int x = 0; x < m_grid.getXMaxSize(); x++)
				{
					temp = m_grid.getNeighbour(x, y, z);
					temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->preassure =
						(
						temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->divergence
						+ temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure + temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure
						+ temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure + temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure
						+ temp.voxels[CUBEPOS::FRONT_MID_CENTER]->preassure + temp.voxels[CUBEPOS::BACK_MID_CENTER]->preassure
						) / 6;
				}
			}
		}
		//force boundries for preassure
		force_boundries_preassure();
	}

	for (unsigned int z = 0; z < m_grid.getZMaxSize(); z++)
	{
		for (unsigned int y = 0; y < m_grid.getYMaxSize(); y++)
		{
			for (unsigned int x = 0; x < m_grid.getXMaxSize(); x++)
			{
				temp = m_grid.getNeighbour(x, y, z);
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.x -= 0.5f*(
					temp.voxels[CUBEPOS::CURRENT_MID_LEFT]->preassure - temp.voxels[CUBEPOS::CURRENT_MID_RIGHT]->preassure); /* /N */
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.y -= 0.5f*(
					temp.voxels[CUBEPOS::CURRENT_BOTTOM_CENTER]->preassure - temp.voxels[CUBEPOS::CURRENT_TOP_CENTER]->preassure); /* /N */
				temp.voxels[CUBEPOS::CURRENT_MID_CENTER]->velocity.z -= 0.5f*(
					temp.voxels[CUBEPOS::BACK_MID_CENTER]->preassure - temp.voxels[CUBEPOS::FRONT_MID_CENTER]->preassure); /* /N */
			}
		}
	}
	//force boundries for velocity
	force_boundries_velocity();
};

//need to check how we define max sizes
void FluidSolver::force_boundries_velocity()
{
	unsigned int maxX = m_grid.getXMaxSize();
	unsigned int maxY = m_grid.getYMaxSize();
	unsigned int maxZ = m_grid.getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid.getGuaranteedVoxel(i, j, 0)->velocity = -m_grid.getGuaranteedVoxel(i, j, 1)->velocity;
			//far
			m_grid.getGuaranteedVoxel(i, j, m_grid.getZMaxSize())->velocity = -m_grid.getGuaranteedVoxel(i, j, maxZ -1)->velocity;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid.getGuaranteedVoxel(i, 0, j)->velocity = -m_grid.getGuaranteedVoxel(i, 1, j)->velocity;
			//top
			m_grid.getGuaranteedVoxel(i, maxY, j)->velocity = -m_grid.getGuaranteedVoxel(i, maxY - 1, j)->velocity;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid.getGuaranteedVoxel(0, i, j)->velocity = -m_grid.getGuaranteedVoxel(1, i, j)->velocity;
			//right
			m_grid.getGuaranteedVoxel(maxX, i, j)->velocity = -m_grid.getGuaranteedVoxel(maxX - 1, i, j)->velocity;
		}
	}
	//left bottom near
	m_grid.getGuaranteedVoxel(0, 0, 0)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, 0)->velocity
		+ m_grid.getGuaranteedVoxel(0, 1, 0)->velocity + m_grid.getGuaranteedVoxel(0, 0, 1)->velocity);

	//top-left-near
	m_grid.getGuaranteedVoxel(0,maxY + 1,0)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, 0)->velocity
		+ m_grid.getGuaranteedVoxel(0, maxY, 0)->velocity + m_grid.getGuaranteedVoxel(0, maxY + 1, 1)->velocity);
	//bottom-right-near
	m_grid.getGuaranteedVoxel(maxX+1,0,0)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX,0,0)->velocity
		+ m_grid.getGuaranteedVoxel(maxX+1,1,0)->velocity + m_grid.getGuaranteedVoxel(maxX+1,0,1)->velocity);
	//right-top-near
	m_grid.getGuaranteedVoxel(maxX+1,maxY+1,0)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX,maxY+1,0)->velocity
		+ m_grid.getGuaranteedVoxel(maxX+1,maxY,0)->velocity + m_grid.getGuaranteedVoxel(maxX+1,maxY+1,1)->velocity);
	//left-bottom-far
	m_grid.getGuaranteedVoxel(0,0,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1,0,maxZ+1)->velocity
		+ m_grid.getGuaranteedVoxel(0,1,maxZ+1)->velocity + m_grid.getGuaranteedVoxel(0,0,maxZ)->velocity);
	//left-top-far
	m_grid.getGuaranteedVoxel(0,maxY+1,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1,maxY+1,maxZ+1)->velocity
		+ m_grid.getGuaranteedVoxel(0,maxY,maxZ+1)->velocity + m_grid.getGuaranteedVoxel(0,maxY+1,maxZ)->velocity);
	//right-bottom-far
	m_grid.getGuaranteedVoxel(maxX+1,0,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX,0,maxZ+1)->velocity
		+ m_grid.getGuaranteedVoxel(maxX+1,1,maxZ+1)->velocity + m_grid.getGuaranteedVoxel(maxX+1,0,maxZ)->velocity);
	//right-top-far
	m_grid.getGuaranteedVoxel(maxX+1,maxY+1,maxZ+1)->velocity = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX,maxY+1,maxZ+1)->velocity
		+ m_grid.getGuaranteedVoxel(maxX+1,maxY,maxZ+1)->velocity + m_grid.getGuaranteedVoxel(maxX+1,maxY+1,maxZ)->velocity);
}

void FluidSolver::force_boundries_density()
{
	unsigned int maxX = m_grid.getXMaxSize();
	unsigned int maxY = m_grid.getYMaxSize();
	unsigned int maxZ = m_grid.getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid.getGuaranteedVoxel(i, j, 0)->density = m_grid.getGuaranteedVoxel(i, j, 1)->density;
			//far
			m_grid.getGuaranteedVoxel(i, j, m_grid.getZMaxSize())->density = m_grid.getGuaranteedVoxel(i, j, maxZ - 1)->density;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid.getGuaranteedVoxel(i, 0, j)->density = m_grid.getGuaranteedVoxel(i, 1, j)->density;
			//top
			m_grid.getGuaranteedVoxel(i, maxY, j)->density = m_grid.getGuaranteedVoxel(i, maxY - 1, j)->density;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid.getGuaranteedVoxel(0, i, j)->density = m_grid.getGuaranteedVoxel(1, i, j)->density;
			//right
			m_grid.getGuaranteedVoxel(maxX, i, j)->density = m_grid.getGuaranteedVoxel(maxX - 1, i, j)->density;
		}
	}
	//left bottom near
	m_grid.getGuaranteedVoxel(0, 0, 0)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, 0)->density
		+ m_grid.getGuaranteedVoxel(0, 1, 0)->density + m_grid.getGuaranteedVoxel(0, 0, 1)->density);

	//top-left-near
	m_grid.getGuaranteedVoxel(0, maxY + 1, 0)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, 0)->density
		+ m_grid.getGuaranteedVoxel(0, maxY, 0)->density + m_grid.getGuaranteedVoxel(0, maxY + 1, 1)->density);
	//bottom-right-near
	m_grid.getGuaranteedVoxel(maxX + 1, 0, 0)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, 0, 0)->density
		+ m_grid.getGuaranteedVoxel(maxX + 1, 1, 0)->density + m_grid.getGuaranteedVoxel(maxX + 1, 0, 1)->density);
	//right-top-near
	m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, 0)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, maxY + 1, 0)->density
		+ m_grid.getGuaranteedVoxel(maxX + 1, maxY, 0)->density + m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, 1)->density);
	//left-bottom-far
	m_grid.getGuaranteedVoxel(0, 0, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, maxZ + 1)->density
		+ m_grid.getGuaranteedVoxel(0, 1, maxZ + 1)->density + m_grid.getGuaranteedVoxel(0, 0, maxZ)->density);
	//left-top-far
	m_grid.getGuaranteedVoxel(0, maxY + 1, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, maxZ + 1)->density
		+ m_grid.getGuaranteedVoxel(0, maxY, maxZ + 1)->density + m_grid.getGuaranteedVoxel(0, maxY + 1, maxZ)->density);
	//right-bottom-far
	m_grid.getGuaranteedVoxel(maxX + 1, 0, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, 0, maxZ + 1)->density
		+ m_grid.getGuaranteedVoxel(maxX + 1, 1, maxZ + 1)->density + m_grid.getGuaranteedVoxel(maxX + 1, 0, maxZ)->density);
	//right-top-far
	m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ + 1)->density = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, maxY + 1, maxZ + 1)->density
		+ m_grid.getGuaranteedVoxel(maxX + 1, maxY, maxZ + 1)->density + m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ)->density);
	
}

void FluidSolver::force_boundries_preassure()
{
	unsigned int maxX = m_grid.getXMaxSize();
	unsigned int maxY = m_grid.getYMaxSize();
	unsigned int maxZ = m_grid.getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid.getGuaranteedVoxel(i, j, 0)->preassure = m_grid.getGuaranteedVoxel(i, j, 1)->preassure;
			//far
			m_grid.getGuaranteedVoxel(i, j, m_grid.getZMaxSize())->preassure = m_grid.getGuaranteedVoxel(i, j, maxZ - 1)->preassure;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid.getGuaranteedVoxel(i, 0, j)->preassure = m_grid.getGuaranteedVoxel(i, 1, j)->preassure;
			//top
			m_grid.getGuaranteedVoxel(i, maxY, j)->preassure = m_grid.getGuaranteedVoxel(i, maxY - 1, j)->preassure;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid.getGuaranteedVoxel(0, i, j)->preassure = m_grid.getGuaranteedVoxel(1, i, j)->preassure;
			//right
			m_grid.getGuaranteedVoxel(maxX, i, j)->preassure = m_grid.getGuaranteedVoxel(maxX - 1, i, j)->preassure;
		}
	}
	//left bottom near
	m_grid.getGuaranteedVoxel(0, 0, 0)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, 0)->preassure
		+ m_grid.getGuaranteedVoxel(0, 1, 0)->preassure + m_grid.getGuaranteedVoxel(0, 0, 1)->preassure);

	//top-left-near
	m_grid.getGuaranteedVoxel(0, maxY + 1, 0)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, 0)->preassure
		+ m_grid.getGuaranteedVoxel(0, maxY, 0)->preassure + m_grid.getGuaranteedVoxel(0, maxY + 1, 1)->preassure);
	//bottom-right-near
	m_grid.getGuaranteedVoxel(maxX + 1, 0, 0)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, 0, 0)->preassure
		+ m_grid.getGuaranteedVoxel(maxX + 1, 1, 0)->preassure + m_grid.getGuaranteedVoxel(maxX + 1, 0, 1)->preassure);
	//right-top-near
	m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, 0)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, maxY + 1, 0)->preassure
		+ m_grid.getGuaranteedVoxel(maxX + 1, maxY, 0)->preassure + m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, 1)->preassure);
	//left-bottom-far
	m_grid.getGuaranteedVoxel(0, 0, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, maxZ + 1)->preassure
		+ m_grid.getGuaranteedVoxel(0, 1, maxZ + 1)->preassure + m_grid.getGuaranteedVoxel(0, 0, maxZ)->preassure);
	//left-top-far
	m_grid.getGuaranteedVoxel(0, maxY + 1, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, maxZ + 1)->preassure
		+ m_grid.getGuaranteedVoxel(0, maxY, maxZ + 1)->preassure + m_grid.getGuaranteedVoxel(0, maxY + 1, maxZ)->preassure);
	//right-bottom-far
	m_grid.getGuaranteedVoxel(maxX + 1, 0, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, 0, maxZ + 1)->preassure
		+ m_grid.getGuaranteedVoxel(maxX + 1, 1, maxZ + 1)->preassure + m_grid.getGuaranteedVoxel(maxX + 1, 0, maxZ)->preassure);
	//right-top-far
	m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ + 1)->preassure = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, maxY + 1, maxZ + 1)->preassure
		+ m_grid.getGuaranteedVoxel(maxX + 1, maxY, maxZ + 1)->preassure + m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ)->preassure);
}

void FluidSolver::force_boundries_divergence()
{
	unsigned int maxX = m_grid.getXMaxSize();
	unsigned int maxY = m_grid.getYMaxSize();
	unsigned int maxZ = m_grid.getZMaxSize();
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j <= maxY; j++)
		{
			//near
			m_grid.getGuaranteedVoxel(i, j, 0)->divergence = m_grid.getGuaranteedVoxel(i, j, 1)->divergence;
			//far
			m_grid.getGuaranteedVoxel(i, j, m_grid.getZMaxSize())->divergence = m_grid.getGuaranteedVoxel(i, j, maxZ - 1)->divergence;
		}
	}
	for (unsigned int i = 0; i <= maxX; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//bottom
			m_grid.getGuaranteedVoxel(i, 0, j)->divergence = m_grid.getGuaranteedVoxel(i, 1, j)->divergence;
			//top
			m_grid.getGuaranteedVoxel(i, maxY, j)->divergence = m_grid.getGuaranteedVoxel(i, maxY - 1, j)->divergence;
		}
	}
	for (unsigned int i = 0; i < maxY; i++)
	{
		for (unsigned int j = 0; j < maxZ; j++)
		{
			//left
			m_grid.getGuaranteedVoxel(0, i, j)->divergence = m_grid.getGuaranteedVoxel(1, i, j)->divergence;
			//right
			m_grid.getGuaranteedVoxel(maxX, i, j)->divergence = m_grid.getGuaranteedVoxel(maxX - 1, i, j)->divergence;
		}
	}
	//left bottom near
	m_grid.getGuaranteedVoxel(0, 0, 0)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, 0)->divergence
		+ m_grid.getGuaranteedVoxel(0, 1, 0)->divergence + m_grid.getGuaranteedVoxel(0, 0, 1)->divergence);

	//top-left-near
	m_grid.getGuaranteedVoxel(0, maxY + 1, 0)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, 0)->divergence
		+ m_grid.getGuaranteedVoxel(0, maxY, 0)->divergence + m_grid.getGuaranteedVoxel(0, maxY + 1, 1)->divergence);
	//bottom-right-near
	m_grid.getGuaranteedVoxel(maxX + 1, 0, 0)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, 0, 0)->divergence
		+ m_grid.getGuaranteedVoxel(maxX + 1, 1, 0)->divergence + m_grid.getGuaranteedVoxel(maxX + 1, 0, 1)->divergence);
	//right-top-near
	m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, 0)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, maxY + 1, 0)->divergence
		+ m_grid.getGuaranteedVoxel(maxX + 1, maxY, 0)->divergence + m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, 1)->divergence);
	//left-bottom-far
	m_grid.getGuaranteedVoxel(0, 0, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, 0, maxZ + 1)->divergence
		+ m_grid.getGuaranteedVoxel(0, 1, maxZ + 1)->divergence + m_grid.getGuaranteedVoxel(0, 0, maxZ)->divergence);
	//left-top-far
	m_grid.getGuaranteedVoxel(0, maxY + 1, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(1, maxY + 1, maxZ + 1)->divergence
		+ m_grid.getGuaranteedVoxel(0, maxY, maxZ + 1)->divergence + m_grid.getGuaranteedVoxel(0, maxY + 1, maxZ)->divergence);
	//right-bottom-far
	m_grid.getGuaranteedVoxel(maxX + 1, 0, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, 0, maxZ + 1)->divergence
		+ m_grid.getGuaranteedVoxel(maxX + 1, 1, maxZ + 1)->divergence + m_grid.getGuaranteedVoxel(maxX + 1, 0, maxZ)->divergence);
	//right-top-far
	m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ + 1)->divergence = (1.0f / 3.0f)*(
		m_grid.getGuaranteedVoxel(maxX, maxY + 1, maxZ + 1)->divergence
		+ m_grid.getGuaranteedVoxel(maxX + 1, maxY, maxZ + 1)->divergence + m_grid.getGuaranteedVoxel(maxX + 1, maxY + 1, maxZ)->divergence);
}
#ifndef VOXELTESTING_H
#define VOXELTESTING_H

#include "voxelGrid.h"
#include "readData.h"
#include "../../../Simulation/C++/fluidsolver.h"
#include "glm.hpp"
#include <iostream>
#include <cstdlib>

namespace voxelTest{

  class VoxelTest{
  public:
    DataHandler* dataPtr;
    Voxelgrid* gridPtr;
	FluidSolver* solverPtr;

    VoxelTest(DataHandler* inDataPtr,Voxelgrid* inGridPtr, FluidSolver* inSolverPtr);
  };


  //Public loose functions in the testing namespace.
  void mainTest(VoxelTest*);
}




#endif

/// @file drawable.h
/// @brief Contains classes with draw methods.

#ifndef MYDRAWABLE_H
#define MYDRAWABLE_H

#include "loadobj.h"
#include "glm.hpp"
#include "LoadTGA.h"
#include <vector>

class myDrawable {
protected:
	glm::mat4 MTWMatrix;
	GLuint program;

public:
	myDrawable(GLuint program);
	virtual void draw() = 0;
};

class SkyCube : public myDrawable {
private:
	Model* model;
	GLuint textureID;

public:
	SkyCube(GLuint program);
	virtual void draw();
};

class Terrain : public myDrawable {
private:
	std::vector<Model*>* model;
	glm::mat3 inverseNormalMatrixTrans;
	GLuint textureID;

public:
	Terrain(GLuint program, std::vector<Model*>*, GLuint texID, glm::vec3 scale);
	virtual void draw();
};

class WaterBody : public myDrawable {
private:
	Model* model;
	glm::mat3 inverseNormalMatrixTrans;
	GLuint textureID;

public:
	WaterBody(GLuint program, Model* inModel, GLuint texID, glm::vec3 scale, int texCoordScaleW, int texCoordScaleH);
	virtual void draw();
};


#endif // DRAWABLE_H

/*
 * Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#ifndef EXAMPLEVRAPP_H_
#define EXAMPLEVRAPP_H_
#define GLM_FORCE_RADIANS

#include "GL/glew.h"
#include "MVRCore/AbstractMVRApp.H"
#include "MVRCore/AbstractCamera.H"
#include "MVRCore/AbstractWindow.H"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include "MVRCore/Event.H"
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include "MVRCore/Thread.h"


class ExampleVrApp : public MinVR::AbstractMVRApp {
public:
	ExampleVrApp();
	virtual ~ExampleVrApp();

	void doUserInputAndPreDrawComputation(const std::vector<MinVR::EventRef> &events, double synchronizedTime);
	void initializeContextSpecificVars(int threadId, MinVR::WindowRef window);
	void postInitialization();
	void drawGraphics(int threadId, MinVR::AbstractCameraRef camera, MinVR::WindowRef window);
    void initData();
private:
	void initGL();
	void initVBO(int threadId);
	void initLights();
    GLuint _vao;
    GLuint _vbo;

    GLuint _indexVbo;
    GLuint shaderProgram;
	std::map<int, GLuint> _vboId;
    MinVR::Mutex _mutex;
    
    std::vector<glm::vec3> verticies;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colors;
    std::vector<GLuint> indicies;


};

#endif /* EXAMPLEVRAPP_H_ */

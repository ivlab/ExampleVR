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

#define _USE_MATH_DEFINES 

#include "GL/glew.h"
#include <cmath>
#include "MVRCore/AbstractMVRApp.H"
#include "MVRCore/AbstractCamera.H"
#include "MVRCore/AbstractWindow.H"
#include "MVRCore/CameraOffAxis.H"

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
    
private:
    void initData();

	void initGL();
	void initVBO(int threadId);
    void initShader(int threadId);


	std::map<int, GLuint> _vboId;
    std::map<int, GLuint> _iboId;
    std::map<int, GLuint> _vaoId;
    std::map<int, GLuint> _shdId;

    MinVR::Mutex _mutex;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    std::vector<GLuint> indicies;
    
    glm::mat4 _model;
    GLuint _program;


};

#endif /* EXAMPLEVRAPP_H_ */

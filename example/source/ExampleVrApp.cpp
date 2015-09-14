/*
 * Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <example/include/ExampleVrApp.h>
#define DEBUG_ON 1

using namespace MinVR;
using glm::vec3;
using glm::vec2;


ExampleVrApp::ExampleVrApp() : MinVR::AbstractMVRApp() {
    initData();
}

ExampleVrApp::~ExampleVrApp() {
    for(std::map<int, GLuint>::iterator iterator = _vboId.begin(); iterator != _vboId.end(); iterator++) {
        glDeleteBuffers(1, &iterator->second);
    }
    for(std::map<int, GLuint>::iterator iterator = _iboId.begin(); iterator != _iboId.end(); iterator++) {
        glDeleteBuffers(1, &iterator->second);
    }
    for(std::map<int, GLuint>::iterator iterator = _vaoId.begin(); iterator != _vaoId.end(); iterator++) {
        glDeleteVertexArrays(1, &iterator->second);
    }
    for(std::map<int, GLuint>::iterator iterator = _shdId.begin(); iterator != _shdId.end(); iterator++) {
        glDeleteProgram( iterator->second);
    }
}



// Create a GLSL program object from inline vertex and fragment text
void ExampleVrApp::initShader(int threadId)
{
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    
    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    printf("%i, %i\n",vertex_shader, fragment_shader);
    
    // Specify source code with strings (these could be read from files)
    vs_text = (GLchar *)
    "#version 330\n"
    "in vec3 position, color, normal;\n"
    "uniform mat4 model, view, proj;\n"
    "out vec3 fragColor, fragPosition, fragNormal;\n"
    "void main() {\n"
    "  fragColor = color;\n"
    "  gl_Position = proj * view * model * vec4(position,1.0);\n"
    "  fragPosition = (model * vec4(position,1.0)).xyz;\n"
    "  fragNormal = normalize((transpose(inverse(model)) * vec4(normal,1.0)).xyz);\n"
    "}\n";
    
    fs_text = (GLchar *)
    "#version 330\n"
    "in vec3 fragColor, fragPosition, fragNormal;\n"
    "out vec4 outColor;\n"
    "uniform vec3 lightPositions[10];\n"
    "uniform vec3 lightColors[10];\n"
    "uniform int lightCount;\n"
    "vec3 lightAmount = vec3(0,0,0);\n"
    "vec3 D;\n"
    "vec3 L;\n"
    "vec4 Idiff;\n"
    "void main () {\n"
        "  for(int i = 0; i < lightCount; i++) {\n"
        "    D = lightPositions[i].xyz - fragPosition;\n"
        "    L = normalize(D);\n"
        "    Idiff = vec4(1,1,1,1)*max(dot(fragNormal,L),0)*1/pow(length(D),1);\n"
        "    lightAmount += lightColors[i]*Idiff.xyz;\n"
        "  }\n"
    "  outColor = vec4((lightAmount+ vec3(0.2,0.2,0.2))*(fragColor+vec3(0.2,0.2,0.2)),1);\n"
    "}\n";
    
    
    // error check
    if (vs_text == NULL) {
        exit(1);
    }
    if (fs_text == NULL) {
        exit(1);
    }    
    // Load Vertex Shader
    const char *vv = vs_text;
    glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
    glCompileShader(vertex_shader); // Compile shaders
    
    // Check for errors
    GLint  compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printf("Vertex shader failed to compile:\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }
    
    // Load Fragment Shader
    const char *ff = fs_text;
    glShaderSource(fragment_shader, 1, &ff, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    
    //Check for Errors
    if (!compiled) {
        printf("Fragment shader failed to compile\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }
    
    // Create the program
    _shdId[threadId] = glCreateProgram();
    
    // Attach shaders to program
    glAttachShader(_shdId[threadId], vertex_shader);
    glAttachShader(_shdId[threadId], fragment_shader);
    
    // Link and set program to use
    glLinkProgram(_shdId[threadId]);
    glUseProgram(_shdId[threadId]);
}



void ExampleVrApp::doUserInputAndPreDrawComputation(
		const std::vector<MinVR::EventRef>& events, double synchronizedTime) {
    
    // Calculate Frames Per Second every 3 seconds
    static int frameCount = 0;
    static int lastFrameCount = frameCount;
    static double lastTime = synchronizedTime;
    double fpsDelay = 3;
    
    if (synchronizedTime > lastTime + fpsDelay) {
        std::cout << "FPS: " << (frameCount - lastFrameCount)/(synchronizedTime - lastTime) << std::endl;
        lastTime = synchronizedTime;
        lastFrameCount = frameCount;
    }
    frameCount++;
   
    // Spin the cube over time by setting the _model matrix
    _model = glm::mat4(1);
    _model = glm::scale(_model, vec3(0.1,0.1,0.1));
    _model = glm::rotate(_model, (float)(M_PI_2*synchronizedTime*1.5), vec3(0,1,0));
    _model = glm::rotate(_model, (float)(M_PI_2*synchronizedTime), vec3(1,0,0));
    _model = glm::rotate(_model, (float)M_PI/4, vec3(0,1,0));
    
    // Rotate the third light around the y axis, orbiting the cube
    GLfloat light_a = synchronizedTime*10;
    GLfloat light_d = 0.15;
    _lightPositions.at(2) = vec3(light_d*cos(light_a),0.1,light_d*sin(light_a));
}

void ExampleVrApp::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    _mutex.lock();
    _vboId[threadId] = GLuint(0);
    _iboId[threadId] = GLuint(0);
    _vaoId[threadId] = GLuint(0);
    _shdId[threadId] = GLuint(0);
    _mutex.unlock();
    
    initShader(threadId);

	initVBO(threadId);
    initGL();

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}
}
void ExampleVrApp::postInitialization()
{
    
}
void ExampleVrApp::initGL()
{
    
}
void ExampleVrApp::initVBO(int threadId)
{
    glGenVertexArrays(1, &_vaoId[threadId]);
	glBindVertexArray(_vaoId[threadId]);

    glGenBuffers(1, & _vboId[threadId]);
    glGenBuffers(1, &_iboId[threadId]);

    glBindBuffer(GL_ARRAY_BUFFER,  _vboId[threadId]);

    
    glBufferData(GL_ARRAY_BUFFER,
                 _vertices.size()*sizeof(_vertices[0]) +
                 _normals.size()*sizeof(_normals[0])+
                 _colors.size()*sizeof(_colors[0]),
                 NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    _vertices.size()*sizeof(_vertices[0]),
                    _vertices.data());
    
    glBufferSubData(GL_ARRAY_BUFFER,
                    _vertices.size()*sizeof(_vertices[0]) ,
                    _normals.size()*sizeof(_normals[0]),
                    _normals.data());
    
    glBufferSubData(GL_ARRAY_BUFFER,
                    _vertices.size()*sizeof(_vertices[0]) +_normals.size()*sizeof(_normals[0]),
                    _colors.size()*sizeof(_colors[0]),
                    _colors.data());
    
    //Tell GPU how to intrepret VBO data
    
    glVertexAttribPointer(glGetAttribLocation(_shdId[threadId], "position"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
    glEnableVertexAttribArray(glGetAttribLocation(_shdId[threadId], "position"));
    
    glVertexAttribPointer(glGetAttribLocation(_shdId[threadId], "normal"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(_vertices.size()*sizeof(_vertices[0]) ));
    glEnableVertexAttribArray(glGetAttribLocation(_shdId[threadId], "normal"));
    
    glVertexAttribPointer(glGetAttribLocation(_shdId[threadId], "color"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(_vertices.size()*sizeof(_vertices[0]) +_normals.size()*sizeof(_normals[0])));
    glEnableVertexAttribArray(glGetAttribLocation(_shdId[threadId], "color"));
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboId[threadId]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size()*sizeof(_indices[0]), _indices.data(), GL_STATIC_DRAW);
    
    
	glClearColor(0.2, 0.2, 0.3, 1.0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    GLenum err;
    if((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "GLERROR initGL: "<<err<<std::endl;
    }
}

void ExampleVrApp::initData()
{
    // cube /////////////////////////////////////////////////////
    //////////////////
    //    v6----- v5
    //   /|      /|
    //  v1------v0|
    //  | |     | |
    //  | |v7---|-|v4
    //  |/      |/
    //  v2------v3
    
    // Specify unique values
    glm::vec3 v[] = {
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(-1.0f, 1.0f, 1.0f),
        glm::vec3(-1.0f,-1.0f, 1.0f),
        glm::vec3(1.0f,-1.0f, 1.0f),
        glm::vec3(1.0f,-1.0f,-1.0f),
        glm::vec3(1.0f, 1.0f,-1.0f),
        glm::vec3(-1.0f, 1.0f,-1.0f),
        glm::vec3(-1.0f,-1.0f,-1.0f)
    };
    glm::vec3 n[] = {
        glm::vec3(0,0,1),
        glm::vec3(1,0,0),
        glm::vec3(0,1,0),
        glm::vec3(-1,0,0),
        glm::vec3(0,-1,0),
        glm::vec3(0,0,-1)
    };
    

    glm::vec3 c[] = {
        glm::vec3(0, 0, 0), // Dark Gray
        glm::vec3(0, 0,   1), // Blue
        glm::vec3(0, 1,   0), // Green
		glm::vec3(0, 1,   1),   // Cyan
		glm::vec3(1,   0, 0), // Red
        glm::vec3(1,   0,   1), // Purple
		glm::vec3(1,   1,   0), // Yellow
		glm::vec3(1,   1,   1)    // White
    };
    
    // construct faces from
    glm::vec3 vertices_array[]  = {
        v[0], v[1], v[2], v[3],
        v[0], v[3], v[4], v[5],
        v[0], v[5], v[6], v[1],
        v[1], v[6], v[7], v[2],
        v[7], v[4], v[3], v[2],
        v[4], v[7], v[6], v[5]
    };
    
    glm::vec3 normals_array[] = {
        n[0], n[0], n[0], n[0],
        n[1], n[1], n[1], n[1],
        n[2], n[2], n[2], n[2],
        n[3], n[3], n[3], n[3],
        n[4], n[4], n[4], n[4],
        n[5], n[5], n[5], n[5]
    };
    
#define CORNER 1
#define FACE 2
#define WHITE 3

    // Pick a color scheme
#define COLORSCHEME WHITE
    
    glm::vec3 colors_array[] = {
#if COLORSCHEME ==  CORNER
        c[0], c[1], c[2], c[3],
        c[0], c[3], c[4], c[5],
        c[0], c[5], c[6], c[1],
        c[1], c[6], c[7], c[2],
        c[7], c[4], c[3], c[2],
        c[4], c[7], c[6], c[5]
#elif COLORSCHEME == FACE
        c[0], c[0], c[0], c[0],
        c[1], c[1], c[1], c[1],
        c[2], c[2], c[2], c[2],
        c[3], c[3], c[3], c[3],
        c[4], c[4], c[4], c[4],
        c[5], c[5], c[5], c[5]
#else /* COLORSCHEME == WHITE*/
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7]
        
#endif
    };
    
    // Define the triangles with index arrays
    GLuint indices_array[] = {
        0, 1, 2, // front
        2, 3, 0,
        4, 5, 6, // right
        6, 7, 4,
        8, 9, 10, // top
        10, 11, 8,
        12, 13, 14, // left
        14, 15, 12,
        16, 17, 18, // bottom
        18, 19, 16,
        20, 21, 22, // back
        22, 23, 20
    };

    // Transfer the data to vectors
    _indices.assign(indices_array,  indices_array+  sizeof(indices_array)/  sizeof(indices_array[0]));
    _vertices.assign(vertices_array, vertices_array+ sizeof(vertices_array)/ sizeof(vertices_array[0]));
    _normals.assign( normals_array,  normals_array+  sizeof(normals_array)/  sizeof(normals_array[0]));
    _colors.assign(  colors_array,   colors_array+   sizeof(colors_array)/   sizeof(colors_array[0]));
    
    // Add a white light above
    _lightPositions.push_back(glm::vec3(0,1.5,0));
    _lightColors.push_back(glm::vec3(1,1,1));

    // Add a red light below
    _lightPositions.push_back(glm::vec3(0,-0.7,0));
    _lightColors.push_back(glm::vec3(0.5,0.0,0.0));
    
    // Add a small orange light for rotation
    _lightPositions.push_back(glm::vec3(0,0,0));
    _lightColors.push_back(glm::vec3(0.00,0.000,0.00));
}


void ExampleVrApp::drawGraphics(int threadId,
                                MinVR::WindowRef window, int viewportIndex) {
    
    MinVR::CameraOffAxis* offAxisCamera = dynamic_cast<MinVR::CameraOffAxis*>(window->getCamera(viewportIndex).get());

    
    glm::mat4 proj;
    proj = offAxisCamera->getLastAppliedProjectionMatrix();
    
    glm::mat4 view = glm::mat4(offAxisCamera->getLastAppliedViewMatrix());


    glUniformMatrix4fv(glGetUniformLocation(_shdId[threadId], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(_shdId[threadId], "model"), 1, GL_FALSE, glm::value_ptr(_model));
    glUniformMatrix4fv(glGetUniformLocation(_shdId[threadId], "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    
    glUniform1i(glGetUniformLocation(_shdId[threadId], "lightCount"), _lightPositions.size());

    glUniform3fv(glGetUniformLocation(_shdId[threadId], "lightColors"), _lightPositions.size(), (float*)&_lightColors[0]);
    glUniform3fv(glGetUniformLocation(_shdId[threadId], "lightPositions"), _lightPositions.size(), (float*)&_lightPositions[0]);
    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(_vaoId[threadId]);
	//glDrawArrays(GL_TRIANGLES, 0, indicies.size());

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboId[threadId]);

    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, nullptr);
}

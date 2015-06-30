/*
 * Copyright Regents of the University of Minnesota, 2014.  This software is released under the following license: http://opensource.org/licenses/lgpl-3.0.html.
 * Source code originally developed at the University of Minnesota Interactive Visualization Lab (http://ivlab.cs.umn.edu).
 *
 * Code author(s):
 * 		Dan Orban (dtorban)
 */

#include <example/include/ExampleVrApp.h>
#define DEBUG_ON 1

typedef enum  {
    ATTRIB_POSITION = 0,
    ATTRIB_NORMAL = 1,
    ATTRIB_COLOR = 2,
    ATTRIB_WVP = 4
} ;
using namespace MinVR;
using glm::vec3;

using glm::vec2;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
    vec3 tangent;
    vec3 bitangent;
};

// Create a GLSL program object from vertex and fragment shader files
void ExampleVrApp::initShader(int threadId)
{
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    
    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    printf("%i, %i\n",vertex_shader, fragment_shader);
    
    // Read source code from shader files
    vs_text =
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
    
    fs_text =
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
    "  outColor = vec4((lightAmount+ vec3(0.2,0.2,0.2))*fragColor,1);\n"
    "}\n";
    
//    fs_text =
//    "#version 330\n"
//    "in vec3 fragColor;\n"
//    "out vec4 outColor;\n"
//    "void main () {\n"
//    "  outColor = vec4(fragColor,1);\n"
//    "}\n";
    
    
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


ExampleVrApp::ExampleVrApp() : MinVR::AbstractMVRApp() {
        initData();
}

ExampleVrApp::~ExampleVrApp() {
	for(std::map<int, GLuint>::iterator iterator = _vboId.begin(); iterator != _vboId.end(); iterator++) {
		glDeleteBuffersARB(1, &iterator->second);
	}
}

void ExampleVrApp::doUserInputAndPreDrawComputation(
		const std::vector<MinVR::EventRef>& events, double synchronizedTime) {
    
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
   
    _model = glm::mat4(1);
    _model = glm::scale(_model, vec3(0.1,0.1,0.1));
    _model = glm::rotate(_model, (float)(M_PI_2*synchronizedTime*1.5), vec3(0,1,0));

    _model = glm::rotate(_model, (float)(M_PI_2*synchronizedTime), vec3(1,0,0));
    _model = glm::rotate(_model, (float)M_PI/4, vec3(0,1,0));
    
    //lightPositions.at(2) = vec3(0.2*cos(synchronizedTime*20),-.1,0.2*sin(synchronizedTime*20));
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
                 vertices.size()*sizeof(vertices[0]) +
                 normals.size()*sizeof(normals[0])+
                 colors.size()*sizeof(colors[0]),
                 NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    vertices.size()*sizeof(vertices[0]),
                    vertices.data());
    
    glBufferSubData(GL_ARRAY_BUFFER,
                    vertices.size()*sizeof(vertices[0]) ,
                    normals.size()*sizeof(normals[0]),
                    normals.data());
    
    glBufferSubData(GL_ARRAY_BUFFER,
                    vertices.size()*sizeof(vertices[0]) +normals.size()*sizeof(normals[0]),
                    colors.size()*sizeof(colors[0]),
                    colors.data());
    
    //Tell GPU how to intrepret VBO data
    
    glVertexAttribPointer(glGetAttribLocation(_shdId[threadId], "position"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
    glEnableVertexAttribArray(glGetAttribLocation(_shdId[threadId], "position"));
    
    glVertexAttribPointer(glGetAttribLocation(_shdId[threadId], "normal"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(vertices.size()*sizeof(vertices[0]) ));
    glEnableVertexAttribArray(glGetAttribLocation(_shdId[threadId], "normal"));
    
    glVertexAttribPointer(glGetAttribLocation(_shdId[threadId], "color"), 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(vertices.size()*sizeof(vertices[0]) +normals.size()*sizeof(normals[0])));
    glEnableVertexAttribArray(glGetAttribLocation(_shdId[threadId], "color"));
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboId[threadId]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size()*sizeof(indicies[0]), indicies.data(), GL_STATIC_DRAW);
    
    
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
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.5,0.5,1),
        glm::vec3(0.5,0.6,0.5),
		glm::vec3(0.5, 0.75, 1),
		glm::vec3(1, 0.6, 0.5),
        glm::vec3(1,0.5,1),
		glm::vec3(1, 0.6, 0.5),
		glm::vec3(1, 0.6, 1)
    };
    
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
    
    glm::vec3 colors_array[] = {
#if 0
        c[0], c[1], c[2], c[3],
        c[0], c[3], c[4], c[5],
        c[0], c[5], c[6], c[1],
        c[1], c[6], c[7], c[2],
        c[7], c[4], c[3], c[2],
        c[4], c[7], c[6], c[5]
#elif 1
        c[0], c[0], c[0], c[0],
        c[1], c[1], c[1], c[1],
        c[2], c[2], c[2], c[2],
        c[3], c[3], c[3], c[3],
        c[4], c[4], c[4], c[4],
        c[5], c[5], c[5], c[5]
#else
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7],
        c[7], c[7], c[7], c[7]
        
#endif
    };
    
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

    
    indicies.assign(indices_array,  indices_array+  sizeof(indices_array)/  sizeof(indices_array[0]));
    vertices.assign(vertices_array, vertices_array+ sizeof(vertices_array)/ sizeof(vertices_array[0]));
    normals.assign( normals_array,  normals_array+  sizeof(normals_array)/  sizeof(normals_array[0]));
    colors.assign(  colors_array,   colors_array+   sizeof(colors_array)/   sizeof(colors_array[0]));
    
    lightPositions.push_back(glm::vec3(0,1,0));
    lightColors.push_back(glm::vec3(1,1,1));

    lightPositions.push_back(glm::vec3(0,-1,0));
    lightColors.push_back(glm::vec3(1,1,1));
    lightPositions.push_back(glm::vec3(0,0,0));
    lightColors.push_back(glm::vec3(0.1,0.1,0.1));
}


void ExampleVrApp::drawGraphics(int threadId, MinVR::AbstractCameraRef camera,
		MinVR::WindowRef window) {
    glm::mat4 proj;
    
    MinVR::CameraOffAxis* offAxisCamera = dynamic_cast<MinVR::CameraOffAxis*>(camera.get());
    proj = offAxisCamera->getLastAppliedProjectionMatrix();
    
    glm::mat4 view;
    view = glm::mat4(offAxisCamera->getLastAppliedViewMatrix());
    _mutex.lock();

    _mutex.unlock();
    


    glUniformMatrix4fv(glGetUniformLocation(_shdId[threadId], "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(_shdId[threadId], "model"), 1, GL_FALSE, glm::value_ptr(_model));
    glUniformMatrix4fv(glGetUniformLocation(_shdId[threadId], "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1i(glGetUniformLocation(_shdId[threadId], "lightCount"), lightPositions.size());

    glUniform3fv(glGetUniformLocation(_shdId[threadId], "lightColors"), lightPositions.size(), (float*)&lightColors[0]);
    glUniform3fv(glGetUniformLocation(_shdId[threadId], "lightPositions"), lightPositions.size(), (float*)&lightPositions[0]);
    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(_vaoId[threadId]);
	//glDrawArrays(GL_TRIANGLES, 0, indicies.size());

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboId[threadId]);

    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
}

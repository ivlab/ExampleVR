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

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile)
{
    FILE *fp;
    long length;
    char *buffer;
    
    // open the file containing the text of the shader code
    fp = fopen(shaderFile, "r");
    
    // check for errors in opening the file
    if (fp == NULL) {
        printf("can't open shader source file %s\n", shaderFile);
        return NULL;
    }
    
    // determine the file size
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);  // return the value of the current position
    
    // allocate a buffer with the indicated number of bytes, plus one
    buffer = new char[length + 1];
    
    // read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
    fread(buffer, 1, length, fp); // read all of the bytes
    
    // append a NULL character to indicate the end of the string
    buffer[length] = '\0';
    
    // close the file
    fclose(fp);
    
    // return the string
    return buffer;
}

GLuint LoadTexture( const char * filename, int w, int h )
{
    //    GLuint texture;
    //    unsigned char * data;
    //    FILE * file;
    //
    //    //The following code will read in our RAW file
    //    file = fopen( filename, "rb" );
    //
    //    if ( file == NULL ) return 0;
    //    data = (unsigned char *)malloc( width * height * 3 );
    //
    //    fread( data, width * height * 3, 1, file );
    //
    //    fclose( file );
    //
    //    glGenTextures( 1, &texture ); //generate the texture with the loaded data
    //    glBindTexture( GL_TEXTURE_2D, texture ); //bind the texture to it’s array
    //
    //    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ); //set texture environment parameters
    //
    //    //And if you go and use extensions, you can use Anisotropic filtering textures which are of an
    //    //even better quality, but this will do for now.
    //    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //
    //    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //
    //    //Here we are setting the parameter to repeat the texture instead of clamping the texture
    //    //to the edge of our shape.
    //    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    //    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
    //
    //    //Generate the texture
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    //
    //    free( data ); //free the texture
    //
    //    return texture; //return whether it was successfull
    
    // Data read from the header of the BMP file
    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    unsigned int width, height;
    unsigned int imageSize;   // = width*height*3
    // Actual RGB data
    unsigned char * data;
    
    FILE * file = fopen(filename,"rb");
    if (!file)
    {
        printf("Image could not be opened\n");
        return 0;
    }
    if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
        printf("Not a correct BMP file\n");
        return false;
    }
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file\n");
        return 0;
    }
    
    // Read ints from the byte array
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    
    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)
        imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)
        dataPos=54; // The BMP header is done that way
    
    // Create a buffer
    data = new unsigned char [imageSize];
    
    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);
    
    //Everything is in memory now, the file can be closed
    fclose(file);
    
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return textureID;
}



// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName)
{
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    GLuint program;
    
    // check GLSL version
    printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read source code from shader files
    vs_text = readShaderSource(vShaderFileName);
    fs_text = readShaderSource(fShaderFileName);
    
    // error check
    if (vs_text == NULL) {
        printf("Failed to read from vertex shader file %s\n", vShaderFileName);
        exit(1);
    } else if (DEBUG_ON) {
        printf("Vertex Shader:\n=====================\n");
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }
    if (fs_text == NULL) {
        printf("Failed to read from fragent shader file %s\n", fShaderFileName);
        exit(1);
    } else if (DEBUG_ON) {
        printf("\nFragment Shader:\n=====================\n");
        printf("%s\n", fs_text);
        printf("=====================\n\n");
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
    program = glCreateProgram();
    
    // Attach shaders to program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    // Link and set program to use
    glLinkProgram(program);
    glUseProgram(program);
    
    return program;
}


ExampleVrApp::ExampleVrApp() : MinVR::AbstractMVRApp() {

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
    double fps = 0;
    
    if (synchronizedTime > lastTime + fpsDelay) {
        fps = (frameCount - lastFrameCount)/(synchronizedTime - lastTime);
        std::cout << "FPS: " << fps << std::endl;
        lastTime = synchronizedTime;
        lastFrameCount = frameCount;
    }
    frameCount++;
   
	//for(int i=0; i < events.size(); i++) {
	//	std::cout << events[i]->getName() <<std::endl;
	//}
}

void ExampleVrApp::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {
	initGL();
	//initVBO(threadId);

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "openGL ERROR in initializeContextSpecificVars: "<<err<<std::endl;
	}
}

void ExampleVrApp::initVBO(int threadId)
{
}

void ExampleVrApp::initGL()
{
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_indexVbo);

    glGenVertexArrays(1, &_vao);
    shaderProgram = InitShader("Vertex.glsl", "Fragment.glsl");
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(_vao);
    int vertexCount = 0;
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
        glm::vec3(0,0,0),
        glm::vec3(0,0,0),
        glm::vec3(0,0,0),
        glm::vec3(0,0,0),
        glm::vec3(0,0,0),
        glm::vec3(0,0,0)
    };
    glm::vec3 c[] = {
        glm::vec3(0,0,0),
        glm::vec3(0,0,1),
        glm::vec3(0,1,0),
        glm::vec3(0,1,1),
        glm::vec3(1,0,0),
        glm::vec3(1,0,1),
        glm::vec3(1,1,0),
        glm::vec3(1,1,1)
    };

    glm::vec3 vertices[]  = {
        v[0], v[1], v[2], v[3],
        v[0], v[3], v[4], v[5],
        v[0], v[5], v[6], v[1],
        v[1], v[6], v[7], v[2],
        v[7], v[4], v[3], v[2],
        v[4], v[7], v[6], v[5]
    };
    
    glm::vec3 normals[] = {
        n[0], n[0], n[0], n[0],
        n[1], n[1], n[1], n[1],
        n[2], n[2], n[2], n[2],
        n[3], n[3], n[3], n[3],
        n[4], n[4], n[4], n[4],
        n[5], n[5], n[5], n[5]
    };
    
    glm::vec3 colors[] = {
#if 1
        c[0], c[1], c[2], c[3],
        c[0], c[3], c[4], c[5],
        c[0], c[5], c[6], c[1],
        c[1], c[6], c[7], c[2],
        c[7], c[4], c[3], c[2],
        c[4], c[7], c[6], c[5]
#else
        c[0], c[0], c[0], c[0],
        c[1], c[1], c[1], c[1],
        c[2], c[2], c[2], c[2],
        c[3], c[3], c[3], c[3],
        c[4], c[4], c[4], c[4],
        c[5], c[5], c[5], c[5]
#endif
    };
    
    GLuint indices_array[] = {
        0, 1, 2, // front *
        2, 3, 0, // *
        4, 5, 6, // right X
        6, 7, 4, // X
        8, 9, 10, // top ?
        10, 11, 8,
        12, 13, 14, // left
        14, 15, 12,
        16, 17, 18, // bottom
        18, 19, 16,
        20, 21, 22, // back
        22, 23, 20
    };
    indicies.assign(indices_array, indices_array+36);
    
    
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    
//        glBufferData(GL_ARRAY_BUFFER, geometry.vertices.size() * sizeof(Vertex), &geometry.vertices[0], GL_STATIC_DRAW);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors), NULL, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER,0                               ,sizeof(vertices),  &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER,sizeof(vertices)                ,sizeof(normals),   &normals[0]);
        glBufferSubData(GL_ARRAY_BUFFER,sizeof(vertices)+sizeof(normals),sizeof(colors),    &colors[0]);
        
        //Tell GPU how to intrepret VBO data

        glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, 0);
        glEnableVertexAttribArray(ATTRIB_POSITION);
        
        glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (void*)sizeof(vertices));
        glEnableVertexAttribArray(ATTRIB_NORMAL);
        
        glVertexAttribPointer(ATTRIB_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (void*)(sizeof(vertices)+sizeof(normals)));
        glEnableVertexAttribArray(ATTRIB_COLOR);


    glBindBuffer(GL_ARRAY_BUFFER, _indexVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_array), indices_array, GL_STATIC_DRAW);
    
    
        
            
    glm::mat4 view = glm::lookAt(
                                 vec3(3,3,3),  //Cam Position
                                 vec3(0,0,0) ,  //Look at point
                                 glm::vec3(0.0f, 1.0f, 0.0f)); //Up
    glm::mat4 model(1);
    
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    
    glm::mat4 proj = glm::perspective((float)M_PI/3, 1.0f, 0.01f, 1000.0f); //FOV, aspect, near, far
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initGL: "<<err<<std::endl;
	}

    

}

void ExampleVrApp::postInitialization() {
}

void ExampleVrApp::drawGraphics(int threadId, MinVR::AbstractCameraRef camera,
		MinVR::WindowRef window) {
    glClearColor(0.2, 0.2, 0.3, 1.0);
    glEnable(GL_DEPTH_TEST);
//    glCullFace(GL_BACK);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    
    




//    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexVbo);
//    glBindBuffer(GL_VERTEX_ARRAY, _vbo);
//    glDrawArrays(GL_LINE_STRIP, 0, 36);
    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, indicies.data());
   

    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "GLERROR: "<<err<<std::endl;
    }

    
}

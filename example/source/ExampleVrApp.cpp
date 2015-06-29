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
    ATTRIB_TEX_COORD = 2,
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

size_t loadOBJ(
               const char * path,
               std::vector < Vertex > & out_vertices
               );

struct GeometrySet {
    struct Geometry {
        size_t start;
        size_t size;
        std::string name;
    };
    std::vector< Vertex > vertices;
    std::vector<Geometry> models;
    size_t addGeometry(const char * name) {
        Geometry g;
        g.start =vertices.size();
        
        size_t size = loadOBJ(name, vertices);
        
        g.size = size;
        g.name = name;
        
        
        models.push_back(g);
        return models.size()-1;
    }
    
    
};
GeometrySet geometry;
std::vector<GLint> textures;

size_t loadOBJ(
               const char * path,
               std::vector < Vertex > & out_vertices
               ) {
    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;
    std::vector< glm::vec3 > temp_tangents;
    std::vector< glm::vec3 > temp_bitangents;
    
    
    FILE * file = fopen(path, "r");
    if( file == NULL ){
        printf("Impossible to open the file !\n");
        return false;
    }
    
    while( 1 ){
        
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        
        // else : parse lineHeader
        
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
            
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            temp_uvs.push_back(uv);
            
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
            
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
            
            
            
        }
    }
    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){
        
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];
        Vertex v;
        
        v.position = temp_vertices[ vertexIndex-1 ];
        v.normal = temp_normals[ normalIndex-1 ];
        v.tex_coord = temp_uvs[uvIndex-1];
        
        out_vertices.push_back(v);
        
        
    }
    
    for ( int i=0; i<out_vertices.size(); i+=3){
        
        // Shortcuts for vertices
        glm::vec3 & v0 = out_vertices[i+0].position;
        glm::vec3 & v1 = out_vertices[i+1].position;
        glm::vec3 & v2 = out_vertices[i+2].position;
        
        // Shortcuts for UVs
        glm::vec2 & uv0 = out_vertices[i+0].tex_coord;
        glm::vec2 & uv1 = out_vertices[i+1].tex_coord;
        glm::vec2 & uv2 = out_vertices[i+2].tex_coord;
        
        // Edges of the triangle : postion delta
        glm::vec3 deltaPos1 = v1-v0;
        glm::vec3 deltaPos2 = v2-v0;
        
        // UV delta
        glm::vec2 deltaUV1 = uv1-uv0;
        glm::vec2 deltaUV2 = uv2-uv0;
        
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
        
        
        out_vertices[i+0].tangent=
        out_vertices[i+1].tangent=
        out_vertices[i+2].tangent=tangent;
        
        out_vertices[i+0].bitangent=
        out_vertices[i+1].bitangent=
        out_vertices[i+2].bitangent=bitangent;
        
    }
    
    
    
    
    return vertexIndices.size();
}


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
    lightPositions.at(0) = glm::vec3(2*sin(synchronizedTime), 2, cos(synchronizedTime));

    lightPositions.at(1) = glm::vec3(2*sin(synchronizedTime+2*M_PI/3), 2, 2*cos(synchronizedTime+2*M_PI/3));
    
    lightPositions.at(2) = glm::vec3(2*sin(synchronizedTime+2*2*M_PI/3), 2, 2*cos(synchronizedTime+2*2*M_PI/3));
    player_position = vec3(4*(sin(synchronizedTime)), 0.5, 4*(cos(synchronizedTime)));
	//for(int i=0; i < events.size(); i++) {
	//	std::cout << events[i]->getName() <<std::endl;
	//}
}

void ExampleVrApp::initializeContextSpecificVars(int threadId,
		MinVR::WindowRef window) {
	initGL();
	//initVBO(threadId);
	//initLights();
    lightColors.push_back((vec3(0.8,0.7,0.9)));
    lightPositions.push_back(vec3(0,100,0));
    lightColors.push_back((vec3(0.7,0.7,0.5)));
    lightPositions.push_back(vec3(0,100,0));
    lightColors.push_back((vec3(1,1,1)));
    lightPositions.push_back(vec3(0,100,0));
	//glClearColor(0.f, 0.f, 0.f, 0.f);
    player_position = vec3(4, 0.5, 2);
//    player_direction = vec3(-1,-1,-1);
    
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
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo2);
    glGenVertexArrays(1, &_vao2);
    shaderProgram = InitShader("Vertex.glsl", "Fragment.glsl");
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(_vao);
    int vertexCount = 0;
    geometry.addGeometry("cube.obj");
    std::cout << "Vertex size: " << sizeof(Vertex) << "\n" << "list size: " << geometry.vertices.size() << " * " << sizeof(Vertex) << " = " << geometry.vertices.size() * sizeof(Vertex)<< "\n";
    std::cout << "floats in Vertex:" << sizeof(Vertex)/sizeof(float) << "\n";

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, geometry.vertices.size() * sizeof(Vertex), &geometry.vertices[0], GL_STATIC_DRAW);
    //Tell GPU how to intrepret VBO data
    std::cout << sizeof(Vertex) <<  '-' <<  sizeof(vec3) << " = " << sizeof(Vertex) - sizeof(vec3) << std::endl;
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    
    glVertexAttribPointer(ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)*2));
    glEnableVertexAttribArray(ATTRIB_TEX_COORD);

    textures.push_back(LoadTexture("cube.bmp", 1024, 1024)) ;
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo2);
    
    for (unsigned int i = 0; i < 4 ; i++) {
        glEnableVertexAttribArray(ATTRIB_WVP + i);
        glVertexAttribPointer(ATTRIB_WVP + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (const GLvoid*)(sizeof(GLfloat) * i * 4));
        glVertexAttribDivisor(ATTRIB_WVP + i, 1);
    }
    
   
    glm::mat4 model;
    for(int i = 0; i < 100000; i++)
    {
        model = glm::mat4(1);
        
        model = glm::translate(model, glm::vec3(-i/10.f,i/20.f,-i/10.f));
        model = glm::rotate(model, (float)(i*M_PI/64),glm::vec3(1,0,0));
        WVPs.push_back(model);

    }
    
    glBindBuffer(GL_ARRAY_BUFFER, ATTRIB_WVP);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * WVPs.size(), WVPs.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(ATTRIB_WVP);
    model = glm::mat4(1);

    model = glm::scale(model, glm::vec3(0.5,0.5,0.5));
    
    
    
    
    glm::mat4 proj = glm::perspective((float)M_PI/3, 1.0f, 0.01f, 1000.0f); //FOV, aspect, near, far
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
    
    GLint uniLightPositions = glGetUniformLocation(shaderProgram, "lightPositions");
    GLint uniLightColors = glGetUniformLocation(shaderProgram, "lightColors");
    GLint uniLightCount = glGetUniformLocation(shaderProgram, "lightCount");
    glUniform1i(uniLightCount, lightPositions.size());

	GLenum err;
	if((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "GLERROR initGL: "<<err<<std::endl;
	}
    glUniform3fv(uniLightColors, lightPositions.size(), (float*)&lightColors[0]);
    glUniform3fv(uniLightPositions, lightPositions.size(), (float*)&lightPositions[0]);

}

void ExampleVrApp::postInitialization() {
}

void ExampleVrApp::drawGraphics(int threadId, MinVR::AbstractCameraRef camera,
		MinVR::WindowRef window) {
    glClearColor(0.2, 0.2, 0.3, 1.0);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint uniCam = glGetUniformLocation(shaderProgram, "camPosition");
    //    cout << player_position.x << player_position.z << endl;
    glUniform3f(uniCam,  player_position.x, player_position.y, player_position.z);
    
    
    GLint uniView = glGetUniformLocation(shaderProgram, "view");

    glm::mat4 view = glm::lookAt(
                                 player_position,  //Cam Position
                                 vec3(0,0,0) ,  //Look at point
                                 glm::vec3(0.0f, 1.0f, 0.0f)); //Up

    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
    glm::mat4 model;
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));


    //    lightColors.push_back(vec3(4,4,2));
    //    lightPositions.push_back(vec3(-3,5,0));
    

    if (false) {
        for(int i = 0; i < WVPs.size(); i++)
        {
            glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(WVPs.at(i)));
            glDrawArrays(GL_TRIANGLES, geometry.models[0].start,geometry.models[0].size);
        }
    } else {
        glDrawArraysInstanced(GL_TRIANGLES,geometry.models[0].start,geometry.models[0].size, WVPs.size());

    }

   

    
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "GLERROR: "<<err<<std::endl;
    }

    
}

#include <glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "LoadShaders.h"
#include "linmath.h"
//#include <vgl.h>
#include <map>
#include <vector>
using namespace std;

#define BUFFER_OFFSET(x)  ((const void*) (x))

GLuint programID;
/*
* Arrays to store the indices/names of the Vertex Array Objects and
* Buffers.  Rather than using the books enum approach I've just
* gone out and made a bunch of them and will use them as needed.
*
* Not the best choice I'm sure.
*/

GLuint vertexBuffers[10], arrayBuffers[10], elementBuffers[10];
/*
* Global variables
*   The location for the transformation and the current rotation
*   angle are set up as globals since multiple methods need to
*   access them.
*/
float rotationAngle;
bool elements;
int nbrTriangles, materialToUse = 0;
int startTriangle = 0, endTriangle = 12;
bool rotationOn = false;
mat4x4 rotation, viewMatrix, projectionMatrix;
map<string, GLuint> locationMap;
GLuint textureID[4];
GLuint currentTextureMap = 0;
GLuint teapotVAO, teapotBAO, sphereVAO, sphereBAO, cubeVAO, cubeBAO;
int teapotTriangles, sphereTriangles, cubeTriangles;
// Prototypes
GLuint buildProgram(string vertexShaderName, string fragmentShaderName);
GLFWwindow * glfwStartUp(int& argCount, char* argValues[],
	string windowTitle = "No Title", int width = 800, int height = 800);
void setAttributes(float lineWidth = 1.0, GLenum face = GL_FRONT_AND_BACK,
	GLenum fill = GL_FILL);
void buildObjects();
void getLocations();
void init(string vertexShader, string fragmentShader);
void buildAndSetupTextures();
float* readOBJFile(string filename, int& nbrTriangles, float*& normalArray, float*& textureCoordArray);
/*
 * Error callback routine for glfw -- uses cstdio
 */
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

/*
 * keypress callback for glfw -- Escape exits...
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        mat4x4_look_at(viewMatrix, vec3{ 5.0f, 0.0f, 0.0f }, vec3 {0.0f, 0.0f, 0.0f}, vec3 {0.0, 1.0f, 0.0f});
    }
    else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        mat4x4_look_at(viewMatrix, vec3{ 0.0f, 10.0f, 0.0f }, vec3 {0.0f, 0.0f, 0.0f}, vec3 {0.0, 0.0f, -1.0f});
    }
    else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        mat4x4_look_at(viewMatrix, vec3{ 0.0f, 0.0f, -5.0f }, vec3 {0.0f, 0.0f, 0.0f}, vec3 {0.0, 1.0f, 0.0f});
    }
    else if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        mat4x4_look_at(viewMatrix, vec3{ 5.0f, 2.0f, -5.0f }, vec3 {0.0f, 0.0f, 0.0f}, vec3 {0.0, 1.0f, 0.0f});
    }
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		currentTextureMap = 0;
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		currentTextureMap = 1;
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		currentTextureMap = 2;
	}
	else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		currentTextureMap = 3;
	}
}

/*
 * Routine to encapsulate some of the startup routines for GLFW.  It returns the window ID of the
 * single window that is created.
 */
GLFWwindow* glfwStartUp(int& argCount, char* argValues[], string title, int width, int height) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);   // This is set to compliance for 4.1 -- if your system
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);   // supports 4.5 or 4.6 you may wish to modify it.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	return window;
}


/*
 * Use the author's routines to build the program and return the program ID.
 */
GLuint buildProgram(string vertexShaderName, string fragmentShaderName) {

	/*
	*  Use the Books code to load in the shaders.
	*/
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, vertexShaderName.c_str() },
		{ GL_FRAGMENT_SHADER, fragmentShaderName.c_str() },
		{ GL_NONE, NULL }
	};
	GLuint program = LoadShaders(shaders);
	if (program == 0) {
		cerr << "GLSL Program didn't load.  Error \n" << endl
			<< "Vertex Shader = " << vertexShaderName << endl
			<< "Fragment Shader = " << fragmentShaderName << endl;
	}
	glUseProgram(program);
	return program;
}

/*
 * Set up the clear color, lineWidth, and the fill type for the display.
 */
void setAttributes(float lineWidth, GLenum face, GLenum fill) {
	/*
	* I'm using wide lines so that they are easier to see on the screen.
	* In addition, this version fills in the polygons rather than leaving it
	* as lines.
	*/
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glLineWidth(lineWidth);
	glPolygonMode(face, fill);
	glEnable(GL_DEPTH_TEST);

}

/*
 * read and/or build the objects to be displayed.  Also sets up attributes that are
 * vertex related.
 */

void buildObjects() {
    // constants
    const int VERTICES_PER_TRIANGLE = 3;
    const int FLOATS_PER_VERTEX   = 4;
    const int FLOATS_PER_NORMAL   = 3;
    const int FLOATS_PER_TEXCOORD = 2;
    const int BYTES_PER_FLOAT     = 4;

    // Teapot
	float* teapotNormals, *teapotTextures;
	float *teapotVertices = readOBJFile("teapot.obj", teapotTriangles, teapotNormals, teapotTextures);
	// Print info
	cout << teapotTriangles << endl;
	glGenVertexArrays(1, &teapotVAO);
	glBindVertexArray(teapotVAO);
	glGenBuffers(1, &teapotBAO);
	glBindBuffer(GL_ARRAY_BUFFER, teapotBAO);

	glBufferData(GL_ARRAY_BUFFER,
		teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT +
		teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT +
		teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT,
		NULL, GL_STATIC_DRAW);
	//                               offset in bytes   size in bytes     ptr to data
	glBufferSubData(GL_ARRAY_BUFFER, 0, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, teapotVertices);
	glBufferSubData(GL_ARRAY_BUFFER, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, teapotNormals);
	glBufferSubData(GL_ARRAY_BUFFER, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT + teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT, teapotTextures);

	GLuint vPosition = glGetAttribLocation(programID, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(programID, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, FLOATS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT));

	GLuint vTexture = glGetAttribLocation(programID, "vTexture");
	glEnableVertexAttribArray(vTexture);
	glVertexAttribPointer(vTexture, FLOATS_PER_TEXCOORD, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT+teapotTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT));

    // Sphere
    float* sphereNormals, *sphereTextures;
    float *sphereVertices = readOBJFile("sphere001.obj", sphereTriangles, sphereNormals, sphereTextures);
    // Print info`
    cout << sphereTriangles << endl;
    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);
    glGenBuffers(1, &sphereBAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereBAO);

    glBufferData(GL_ARRAY_BUFFER,
                 sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT +
                 sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT +
                 sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT,
                 NULL, GL_STATIC_DRAW);
    //                               offset in bytes   size in bytes     ptr to data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, sphereVertices);
    glBufferSubData(GL_ARRAY_BUFFER, sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, sphereNormals);
    glBufferSubData(GL_ARRAY_BUFFER, sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT + sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT, sphereTextures);

    vPosition = glGetAttribLocation(programID, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    vNormal = glGetAttribLocation(programID, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, FLOATS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT));

    vTexture = glGetAttribLocation(programID, "vTexture");
    glEnableVertexAttribArray(vTexture);
    glVertexAttribPointer(vTexture, FLOATS_PER_TEXCOORD, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT+sphereTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT));

    // Cube
    float* cubeNormals, *cubeTextures;
    float *cubeVertices = readOBJFile("cube001.obj", cubeTriangles, cubeNormals, cubeTextures);
    // Print info`
    cout << cubeTriangles << endl;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
    glGenBuffers(1, &cubeBAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeBAO);

    glBufferData(GL_ARRAY_BUFFER,
                 cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT +
                 cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT +
                 cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT,
                 NULL, GL_STATIC_DRAW);
    //                               offset in bytes   size in bytes     ptr to data
    glBufferSubData(GL_ARRAY_BUFFER, 0, cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, cubeVertices);
    glBufferSubData(GL_ARRAY_BUFFER, cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT, cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, cubeNormals);
    glBufferSubData(GL_ARRAY_BUFFER, cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT + cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT, cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_TEXCOORD * BYTES_PER_FLOAT, cubeTextures);

    vPosition = glGetAttribLocation(programID, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, FLOATS_PER_VERTEX, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    vNormal = glGetAttribLocation(programID, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, FLOATS_PER_NORMAL, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT));

    vTexture = glGetAttribLocation(programID, "vTexture");
    glEnableVertexAttribArray(vTexture);
    glVertexAttribPointer(vTexture, FLOATS_PER_TEXCOORD, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_VERTEX * BYTES_PER_FLOAT+cubeTriangles * VERTICES_PER_TRIANGLE * FLOATS_PER_NORMAL * BYTES_PER_FLOAT));
}

/*
 * This fills in the locations of most of the uniform variables for the program.
 * there are better ways of handling this but this is good in going directly from
 * what we had.
 *
 * Revised to get the locations and names of the uniforms from OpenGL.  These
 * are then stored in a map so that we can look up a uniform by name when we
 * need to use it.  The map is still global but it is a little neater than the
 * version that used all the locations.  The locations are still there right now
 * in case that is more useful for you.
 *
 */

void getLocations() {
	/*
	 * Find out how many uniforms there are and go out there and get them from the
	 * shader program.  The locations for each uniform are stored in a global -- locationMap --
	 * for later retrieval.
	 */
	GLint numberBlocks;
	char uniformName[1024];
	int nameLength;
	GLint size;
	GLenum type;
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numberBlocks);
	for (int blockIndex = 0; blockIndex < numberBlocks; blockIndex++) {
		glGetActiveUniform(programID, blockIndex, 1024, &nameLength, &size, &type, uniformName);
		cout << uniformName << endl;
		locationMap[string(uniformName)] = blockIndex;
	}
}

/*
 * read in raw data file for image.
 */
unsigned char* getImageFromFile(string filename, int width, int height) {
	ifstream infile;
	unsigned char* image;

	infile.open(filename.c_str(), ios_base::binary);
	if (!infile.fail()) {
		image = new unsigned char[width * height * 3];
		//infile.get(); // get rid of the newline
		memset(image, 0, width * height * 3); // clear out the memory area
		infile.read((char*)image, width * height * 3);
		infile.close();
		return image;
	}
	else {
		return nullptr;
	}
}

void init(string vertexShader, string fragmentShader) {

	setAttributes(1.0f, GL_FRONT_AND_BACK, GL_FILL);

	programID = buildProgram(vertexShader, fragmentShader);
	mat4x4_identity(rotation);
    mat4x4_identity(viewMatrix);

    mat4x4_look_at(viewMatrix, vec3{ 5.0f, 2.0f, -5.0f }, vec3 {0.0f, 0.0f, 0.0f}, vec3 {0.0, 1.0f, 0.0f});
    mat4x4_perspective(projectionMatrix, M_PI_4, 1.0f, 0.01f, 100.0f);
	buildObjects();
	getLocations();

	buildAndSetupTextures();
}
/*
 * This function loads and sets up 4 textures for use by the program.  The
 * textures are in a raw format -- just a list of all the pixel values.
 */
void buildAndSetupTextures()
{
	/*
	*  Create and load the textures
	*/
	string filenames[] = { "image1.raw", "image2.raw", "image3.raw", "image4.raw" };

	unsigned char* imageData;
	glGenTextures(4, textureID);
    // using glTexImage instead of glTexSubImage
    // not using glTextStorage2D
	for (int i = 0; i < 4; i++) {
        glBindTexture(GL_TEXTURE_2D, textureID[i]);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // No padding between pixels/rows
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // load and generate the texture
        unsigned char *data = getImageFromFile(filenames[i], 256, 256);
        if (data) // check if the texture got loaded or not
        {
            // create the texture with the data
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        free(data);
	}
}

/*
 * The display routine is basically unchanged at this point.
 */
void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// needed
	glUseProgram(programID);
    GLuint viewMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, false, (const GLfloat*) viewMatrix);
    GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat*) projectionMatrix);

    glBindTexture(GL_TEXTURE_2D, textureID[currentTextureMap]);

    GLuint texLocation = glGetUniformLocation(programID, "tex");
	GLuint modelMatrixLocation = glGetUniformLocation(programID, "modelingMatrix");
//	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat *)rotation);
//	glBindVertexArray(vertexBuffers[0]);
//	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
//	glDrawArrays(GL_TRIANGLES, 0, nbrTriangles * 3);
	mat4x4 scaling, teapotModelingMatrix, cubeModelingMatrix, sphereModelingMatrix, translate;
    // texture 1
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
	mat4x4_identity(scaling);
	mat4x4_scale_aniso(scaling, scaling, 1.0f / 20.0f, 1.0f / 20.0f, 1.0f / 20.0f);
	mat4x4_mul(teapotModelingMatrix, rotation, scaling);
    mat4x4_translate_in_place(teapotModelingMatrix, 20.0, 0.f, 20.0f);
	glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)teapotModelingMatrix);
	glBindVertexArray(teapotVAO);
	glBindBuffer(GL_ARRAY_BUFFER, teapotBAO);
	glDrawArrays(GL_TRIANGLES, 0, teapotTriangles*3);

    // texture 2
    glBindTexture(GL_TEXTURE_2D, textureID[1]);
    mat4x4_identity(scaling);
    mat4x4_scale_aniso(scaling, scaling, 1.0f / 5.0f, 1.0f / 5.0f, 1.0f / 5.0f);
    mat4x4_mul(sphereModelingMatrix, rotation, scaling);
    mat4x4_translate_in_place(sphereModelingMatrix, 5.0f, 0.0f, -5.0f);
    glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)sphereModelingMatrix);
    glBindVertexArray(sphereBAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereTriangles * 3);

    // texture 3
    glBindTexture(GL_TEXTURE_2D, textureID[2]);
    mat4x4_identity(scaling);
    mat4x4_scale_aniso(scaling, scaling, 1.0f / 5.0f, 1.0f / 5.0f, 1.0f / 5.0f);
    mat4x4_mul(cubeModelingMatrix, rotation, scaling);
    mat4x4_translate_in_place(cubeModelingMatrix, -5.0f, 0.0f, -5.0f);
    glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)cubeModelingMatrix);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeBAO);
    glDrawArrays(GL_TRIANGLES, 0, cubeTriangles * 3);

    // texture 4
    glBindTexture(GL_TEXTURE_2D, textureID[3]);
    mat4x4_identity(scaling);
    mat4x4_scale_aniso(scaling, scaling, 1.0f / 5.0f, 1.0f / 5.0f, 1.0f / 5.0f);
    mat4x4_mul(cubeModelingMatrix, rotation, scaling);
    mat4x4_translate_in_place(cubeModelingMatrix, -5.0f, 0.0f, 5.0f);
    glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)cubeModelingMatrix);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeBAO);
    glDrawArrays(GL_TRIANGLES, 0, cubeTriangles * 3);

}

/*
* Handle window resizes -- adjust size of the viewport -- more on this later
*/

void reshapeWindow(GLFWwindow* window, int width, int height)
{
	float ratio;
	ratio = width / (float)height;

	glViewport(0, 0, width, height);

}
/*
* Main program with calls for many of the helper routines.
*/
int main(int argCount, char* argValues[]) {
	GLFWwindow* window = nullptr;
	window = glfwStartUp(argCount, argValues, "Textures on Objects");
	init("texture.vert", "texture.frag");
	glfwSetWindowSizeCallback(window, reshapeWindow);

	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
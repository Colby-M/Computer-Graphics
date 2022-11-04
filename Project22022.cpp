#include <glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include "LoadShaders.h"
#include "linmath.h"
#include <map>
#include <vector>
#include "Particles.h"
using namespace std;
/*
 * Computer Graphics I -- Project 2
 * Name: Colby McDonald
 *
 */

#define BUFFER_OFFSET(x)  ((const void*) (x))

GLuint programID;
/*
* Arrays to store the indices/names of the Vertex Array Objects and
* Buffers.  Rather than using the books enum approach I've just
* gone out and made a bunch of them and will use them as needed.
*
* This will change as we add things into our toolbox this term.
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
float currentT = 0.0f;
FountainParticles myParticleSystem;

// Prototypes
GLuint buildProgram(string vertexShaderName, string fragmentShaderName);
GLFWwindow* glfwStartUp(int& argCount, char* argValues[],
                        string windowTitle = "No Title", int width = 500, int height = 500);
void setAttributes(float lineWidth = 1.0, GLenum face = GL_FRONT_AND_BACK,
                   GLenum fill = GL_FILL);
void buildObjects();
void getLocations();
void init(string vertexShader, string fragmentShader);
float* readOBJFile(string filename, int& nbrTriangles, float*& normalArray);
/*
 * Error callback routine for glfw -- uses cstdio
 */
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

/*
 * keypress callbacks for glfw -- Escape exits...
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        mat4x4_rotate_Y(rotation, rotation, 0.31419);
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        mat4x4_rotate_Y(rotation, rotation, -0.31419);
    }
    else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        mat4x4_look_at(viewMatrix, vec3{ 1.0f, 0.0f, 0.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
    }
    else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        mat4x4_look_at(viewMatrix, vec3{ 0.0f, 1.0f, 0.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 0.0f, 1.0f });
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

    myParticleSystem.init(1000);
    glGenVertexArrays(1, vertexBuffers);
    glBindVertexArray(vertexBuffers[0]);

/*
 * Read object in from obj file.
 */
    GLfloat *cowVertices= nullptr, *cowNormals=nullptr;
    cowVertices = readOBJFile("roof.obj", nbrTriangles, cowNormals);
    glGenBuffers(1, &(arrayBuffers[0]));
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
    GLuint cowVerticesSize = nbrTriangles * 3.0 * 4.0 * sizeof(GLfloat);
    GLuint cowNormalsSize = nbrTriangles * 3.0 * 3.0 * sizeof(GLfloat);
    // The cow has no colors associated with the vertices.  I'm going to make it
    // gray for right now.
    GLfloat *cowColors = new GLfloat[nbrTriangles * 3.0 * 4.0];
    for (int i = 0; i < nbrTriangles * 3.0 * 4.0; i++) {
        cowColors[i] = 0.7f;
    }
    GLuint cowColorsSize = cowVerticesSize;
    glBufferData(GL_ARRAY_BUFFER,
                 cowVerticesSize + cowColorsSize + cowNormalsSize,
                 NULL, GL_STATIC_DRAW);
    //                               offset in bytes   size in bytes     ptr to data

    glBufferSubData(GL_ARRAY_BUFFER, 0, cowVerticesSize, cowVertices);
    glBufferSubData(GL_ARRAY_BUFFER, cowVerticesSize, cowColorsSize, cowColors);
    glBufferSubData(GL_ARRAY_BUFFER, cowVerticesSize+cowColorsSize, cowNormalsSize, cowNormals);
    /*
     * Set up variables into the shader programs (Note:  We need the
     * shaders loaded and built into a program before we do this)
     */
    GLuint vPosition = glGetAttribLocation(programID, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    GLuint vColors = glGetAttribLocation(programID, "vColor");
    glEnableVertexAttribArray(vColors);
    glVertexAttribPointer(vColors, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(cowVerticesSize));
    GLuint vNormal = glGetAttribLocation(programID, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(cowVerticesSize+cowColorsSize));
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

void init(string vertexShader, string fragmentShader) {

    setAttributes(1.0f, GL_FRONT_AND_BACK, GL_FILL);

    programID = buildProgram(vertexShader, fragmentShader);
    mat4x4_identity(rotation);
    mat4x4_identity(viewMatrix);
    mat4x4_look_at(viewMatrix, vec3{ 1.0f, -1.0f, 1.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f });
    mat4x4_perspective(projectionMatrix, (60 * M_PI)/180, 1.0f, -5.0f, 5.0f);
    buildObjects();
    getLocations();
}
void SetUpDirectionalLighting()
{
    GLfloat ambientLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLuint ambientLightLocation = glGetUniformLocation(programID, "ambientLight");
    glUniform4fv(ambientLightLocation, 1, ambientLight);
    GLuint directionalLightDirectionLoc = glGetUniformLocation(programID, "directionalLightDirection");
    GLuint directionalLightColorLoc = glGetUniformLocation(programID, "directionalLightColor");
    GLuint halfVectorLocation = glGetUniformLocation(programID, "halfVector");
    GLuint shininessLoc = glGetUniformLocation(programID, "shininess");
    GLuint strengthLoc = glGetUniformLocation(programID, "strength");
    GLfloat directionalLightDirection[] = { 0.0f, 0.7071f, 0.7071f };
    GLfloat directionalLightColor[] = { 1.0f, 1.0f, 1.0f };
    GLfloat shininess = 25.0f;
    GLfloat strength = 1.0f;
    GLfloat halfVector[] = { 0.0f, 0.45514f, 0.924f };
    glUniform1f(shininessLoc, shininess);
    glUniform1f(strengthLoc, strength);
    glUniform3fv(directionalLightDirectionLoc, 1, directionalLightDirection);
    glUniform3fv(directionalLightColorLoc, 1, directionalLightColor);
    glUniform3fv(halfVectorLocation, 1, halfVector);
}
/*
 * The display routine is basically unchanged at this point.
 */
void displayDirectional() {
    myParticleSystem.generate(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// needed
    GLuint modelMatrixLocation = glGetUniformLocation(programID, "modelingMatrix");
    mat4x4 translation, mTransform;
    mat4x4_translate(translation, 3.0f* sin(currentT * 2.0 * 3.14159f), 0.0f, 2.0f*cos(currentT * 2.0 * 3.14159f));
    mat4x4_mul(mTransform, translation, rotation);
    glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat*)mTransform);
    GLuint viewMatrixLocation = glGetUniformLocation(programID, "viewingMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, false, (const GLfloat*)viewMatrix);
    GLuint projectionMatrixLocation = glGetUniformLocation(programID, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, false, (const GLfloat*)projectionMatrix);
    mat4x4 MVMatrix, MVPMatrix;
    mat4x4_mul(MVMatrix, viewMatrix, rotation);
    mat4x4_mul(MVPMatrix, projectionMatrix, MVMatrix);
    /*
     *  This code is designed to invert and transpose the matrix
     *  needed to modify normals.
     */
    for (int i = 0; i < 3; i++) {  // zero out last row and column.
        MVMatrix[i][3] = 0;
        MVMatrix[3][i] = 0;
    }
    MVMatrix[3][3] = 1;
    mat4x4 inverted;
    mat4x4_invert(inverted, MVMatrix);  // inverting should give the proper
    // matrix in the upper 3x3.
    GLfloat normalMatrix[3][3];

    for (int row = 0; row < 3; row++) {  // copy it over and transpose it.
        for (int column = 0; column < 3; column++) {
            normalMatrix[row][column] = inverted[column][row];
        }
    }
    GLuint normalMatrixLoc = glGetUniformLocation(programID, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, false, (const GLfloat*)normalMatrix);
    SetUpDirectionalLighting();

    glDrawArrays(GL_TRIANGLES, 0, nbrTriangles * 3);

    mat4x4 scale;
    mat4x4 translate;
    mat4x4_identity(scale);
    mat4x4_scale_aniso(scale, scale, 0.1f, 0.1f, 0.1f);;
    mat4x4 modelMatrix;
    float* positions = myParticleSystem.getPositions();
    int nbrParticles = myParticleSystem.getNumberOfParticles();

    for (int currentParticle = 0; currentParticle < nbrParticles; currentParticle++) {
        mat4x4_identity(translate);
        mat4x4_translate(translate, positions[currentParticle * 3],
                         positions[currentParticle * 3 + 1],
                         positions[currentParticle * 3 + 2]);
        mat4x4_mul(modelMatrix, translate, scale);
        glUniformMatrix4fv(modelMatrixLocation, 1, false, (const GLfloat *)modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, nbrTriangles * 3);
    }
    myParticleSystem.update();
    myParticleSystem.compact();

    currentT = currentT + 0.01;
    if (currentT > 1.0f) {
        currentT -= 1.0f;
    }
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
    window = glfwStartUp(argCount, argValues, "Lighting Demo");
    init("passthrough.vert", "directional.frag");
//	init("pointsource.vert", "pointsource.frag");
    glfwSetWindowSizeCallback(window, reshapeWindow);
    while (!glfwWindowShouldClose(window))
    {
        displayDirectional();
        glfwSwapBuffers(window);
        glfwPollEvents();
    };


    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
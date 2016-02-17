#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
//#include <GL/glew.h>
//#include <GL/glu.h>
//include <GL/gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <bits/stdc++.h>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
    bool isObs;
    float obj_x;
    float obj_y;
    float r;
    int id;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;


void projectile();

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( std::max(InfoLogLength, (int)1) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );
    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

float rot_ang = 0;
int flag=0;
double x=0, y=0, u = 18, t=0;
double key_press_time = 0, key_release_time = 0, u_factor;
static float z = 1;
int obs1_flag = 0, obs2_flag = 0, obs3_flag = 0, obs4_flag = 0, obs5_flag = 0;
float a = -9.0, b = 9.0, c = -4.0, d = 4.0;
int obs_cir = 0, obs_rect = 0, obs_tri = 0;
float ux, uy;
float Y=0;
int Yflag = 0;
GLfloat zoom = 2.f;
float score = 0;
float pan = 0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_SPACE:
                key_release_time = glfwGetTime();
                u_factor = key_release_time - key_press_time;
                u = u*u_factor*2.5;
                cout << "\nSPEED: " << u << "\n";
                ux = u*cos(rot_ang*M_PI/180.0f);
                uy = u*sin(rot_ang*M_PI/180.0f);
                flag=1;
                // do something ..
                break;
                //    Matrices.projection = glm::ortho(-9.0f*z, 9.0f*z, -4.0f*z, 4.0f*z, 0.1f, 500.0f);
                //            z += 2;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_W:
                rot_ang += 10;
                break;
            case GLFW_KEY_S:
                rot_ang -= 10;
                break;
            case GLFW_KEY_SPACE:
                key_press_time = glfwGetTime();
            case GLFW_KEY_Z:
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
        quit(window);
        case 'z':
            z -= 0.05;
            break;
        case 'x':
            z += 0.05;
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS)
                pan -= 0.5;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                pan += 0.5;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /*glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0);*/
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //    glMatrixMode (GL_MODELVIEW);
    //  glLoadIdentity();
}

VAO *obj[50];

VAO *triangle, *rectangle, *line, *circle, *rectangle2, *triangle2, *rectangle3, *circle1, *circle2, *circle3, *circle4, *circle5, *circle6,*circle7, *movingObs, *circle8;

static int chance = 3;
void createMovingobs()
{
    static const GLfloat vertex_buffer_data [] = {
        2.8, 1.0, 0,
        2.8, -1.0, 0,
        3.0, -1.0, 0,

        3.0, -1.0, 0,
        3.0, 1.0, 0,
        2.8, 1.0, 0,
    };

    movingObs = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0);
}


void createRectangle3()
{
    static const GLfloat vertex_buffer_data [] = {
        -7.8, -2.3, 0,
        -7.8, -2.7, 0,
        -6.0, -2.7, 0,

        -6.0, -2.7, 0,
        -6.0, -2.3, 0,
        -7.8, -2.3, 0,
    };


    rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data,0,0,0);
}

void createLine()
{
    static const GLfloat vertex_buffer_data [] = {
        0, 0, 0,
        -5.0,-1.0,0,
    };

    static const GLfloat color_buffer_data [] = {
        0,0,0,
        0,0,0,
        0,0,0,
    };
    glLineWidth( 50 ); 
    line = create3DObject(GL_LINES, 2, vertex_buffer_data,1,1,1);
}



// Creates the triangle object used in this sample code
void createTriangle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        -1, 1,0, // vertex 0
        -1,-1,0, // vertex 1
        1.5,0,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 0
        1,0,0.0, // color 1
        1,0,0.0 // color 2
    };

    static const GLfloat color_t_2 [] = {
        0.3, 0, 0.5,
        0.3, 0, 0.5,
        0.3, 0, 0.5,
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
    triangle2 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_t_2, GL_FILL);
    triangle2->r = 0.24;
    triangle2->obj_x = 4.8;
    triangle2->obj_y = 1.0;

    if(obs_tri == 0)
    {
        obj[2] = triangle2;
        obs_tri = 1;
    }
}



void createRectangle2()
{
    static const GLfloat vertex_buffer_data [] = {
        7, 1.5, 0,
        7.5, 1.5, 0,
        7.5, 1.8, 0,

        7.5, 1.8, 0,
        7., 1.8, 0,
        7, 1.5, 0
    };

    static const GLfloat color_buffer_data [] = {
        1, 0.55, 0,
        1, 0.55, 0,
        1, 0.55, 0,

        1, 0.55, 0,
        1, 0.55, 0,
        1, 0.55, 0
    };

    rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    rectangle2->r = 0.583;
    rectangle2->obj_x = 7.25;
    rectangle2->obj_y = 1.65;

    if(obs_rect == 0)
    {
        obj[1] = rectangle2;
        obs_rect = 1;
    }


}



// Creates the rectangle object used in this sample code
void createRectangle (float x1,float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4)
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -100, -1, 0, // vertex 1
        100, -1, 0, // vertex 2
        100, -100,0, // vertex 3

        100, -100,0, // vertex 3
        -100, -100,0, // vertex 4
        -100, -1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        x1,y1,z1, // color 1
        x2,y2,z2, // color 2
        x3,y3,z3, // color 3

        x3,y3,z3, // color 3
        x4,y4,z4, // color 4
        z1,y1,z1 // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}



float camera_rotation_angle = 90;
float triangle_rotation = 0;

void drawCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides )
{
    int numberOfVertices = numberOfSides + 2;

    GLfloat twicePi = 2.0f * M_PI;

    GLfloat circleVerticesX[numberOfVertices];
    GLfloat circleVerticesY[numberOfVertices];
    GLfloat circleVerticesZ[numberOfVertices];

    circleVerticesX[0] = x;
    circleVerticesY[0] = y;
    circleVerticesZ[0] = z;



    for ( int i = 1; i < numberOfVertices; i++ )
    {
        circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
        circleVerticesY[i] = y + ( radius * sin( i * twicePi / numberOfSides ) );
        circleVerticesZ[i] = z;
    }

    GLfloat allCircleVertices[( numberOfVertices ) * 3];

    for ( int i = 0; i < numberOfVertices; i++ )
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
        allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
    }





    // create3DObject creates and returns a handle to a VAO that can be used later

    circle = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,1);
    circle1 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0,0,0);
    circle2 = create3DObject(GL_TRIANGLE_FAN, 360,allCircleVertices,0.3,0.5,0.3);
    circle2->r = 0.3;
    circle2->obj_x = 3.8;
    circle2->obj_y = 3.2;
    circle2->isObs = true;
    circle3 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,1);
    circle4 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,1);
    circle5 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0.7,0.1,0.1);
    circle5->r = 0.2;
    circle5->obj_x = 5.7;
    circle5->obj_y = -1.4;
    circle5->isObs = true;

    circle6 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0.5,0,0.5);
    circle6->r = 0.4;
    circle6->obj_x = 5.0;
    circle6->obj_y = -3.0;
    circle6->isObs = true;

    circle7 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1.0,0.84,0);
    circle8 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0.0,0.6,0.0);

    if(obs_cir == 0)
    {
        obj[0] = circle2;
        obj[3] = circle5;
        obj[4] = circle6;
        obs_cir = 1;
    }
}


/*void projectile()
  {

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );

  glm::vec3 target (0, 0, 0);

  glm::vec3 up (0, 1, 0);

  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  glm::mat4 VP = Matrices.projection * Matrices.view;

  glm::mat4 MVP;	// MVP = Projection * View * Model

  drawCircle(0, 0, 0, 0.2, 40); // create and initialize circle  
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCircle = glm::translate (glm::vec3(-5.0+x/10, -3.0+y/10, 0));        // glTranslatef
  Matrices.model *= (translateCircle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(circle);
  x = u*cos(M_PI/4)*t;
  y = u*sin(M_PI/4)*t - t*t;
  t += 0.05;
  cout << x << " " << y << "\n";
  }*/
int flagobs = 0;

void isCollide()
{

    float x1 = -7.8+x/10;
    float y1 = -2.5+y/10;
    float r1 = 0.2;
    float x2;
    float y2;
    float d1;
    float r2;
    float x3,y3,r3,d2,x4,y4,r4,d3,x5,y5,r5,d4,x6,y6,r6,d5;

    x2 = obj[0]->obj_x;
    y2 = obj[0]->obj_y;
    d1 = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    r2 = obj[0]->r;
    if(d1 < r1+r2)
    {
        obs1_flag = 1;
        score += 1.0;
    }

    x3 = obj[1]->obj_x;
    y3 = obj[1]->obj_y;
    r3 = obj[1]->r;
    d2 = sqrt((x1-x3)*(x1-x3) + (y1-y3)*(y1-y3));
    if(d2 < r1+r3)
    {
        obs2_flag = 1;
        score += 1.0;
    }

    x4 = obj[2]->obj_x;
    y4 = obj[2]->obj_y;
    r4 = obj[2]->r;
    d3 = sqrt((x1-x4)*(x1-x4) + (y1-y4)*(y1-y4));
    if(d3 < r1+r4)
    {
        obs3_flag = 1;
        score += 1.0;
    }

    x5 = obj[3]->obj_x;
    y5 = obj[3]->obj_y;
    r5 = obj[3]->r;
    d4 = sqrt((x1-x5)*(x1-x5) + (y1-y5)*(y1-y5));
    if(d4 < r1+r5)
    {
        obs4_flag = 1;
        score += 1.0;
    }

    x6 = obj[4]->obj_x;
    y6 = obj[4]->obj_y;
    r6 = obj[4]->r;
    d5 = sqrt((x1-x6)*(x1-x6) + (y1-y6)*(y1-y6));
    if(d5 < r1+r6)
    {
        obs5_flag = 1;
        score += 1.0;
    }

    // cout << "Y: " << Y << endl;
    //cout << "x1: " << -7.8+x/10 << " " << "y1: " << -2.5+y/10 << endl;
    if((x1>=2.5 && x1<=3.0) && (y1>=-1.0f+Y && y1<=Y+1.0f))
    {
        ux = -ux;
    }

}    

/*void obsCollide()
  {
  cout << "Y: " << Y << endl;
  cout << "x1: " << x1 << " " << "y1: " << y1 << endl;
  if((-7.8+x>=2.5 && x1<=3.0) && (y1>=-1.0f+Y && y1<=Y+1.0f))
  {
  cout << "in if**********************************************************************************\n";
  rot_ang = M_PI - atan((u*sin(rot_ang)-2*t)/u*cos(rot_ang));
  rot`_ang = rot_ang * (180.0f/M_PI);
  u = u/10.0;
  t = 0;

  }*/


void draw()
{
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0.0, 0.0, 0.0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    Matrices.projection = glm::ortho(a*z, b*z, c*z, d*z, 0.1f, 500.0f);
    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless youoglm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    // Load identity to model matrix
    /* Render your scene */


    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    Matrices.model = glm::mat4(1.0f);
    glm::mat4 Z = glm::translate(glm::vec3(0.0,0.0,-zoom));
    Matrices.model *= Z;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    // glPopMatrix ();
    Matrices.model = glm::mat4(1.0f);

    glm::mat4 translateRectangle = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    //Matrices.model *= (translateRectangle * rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    draw3DObject(rectangle);

    if(obs2_flag == 0)
    {
        MVP = VP * Matrices.model;
        Matrices.model = glm::mat4(1.0f);
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(rectangle2);
    }


    /*    Matrices.model = glm::mat4(1.0f);    
          glm::mat4 scaleRectangle2 = glm::scale (glm::vec3(0.8, 0.8, 0.0));
          glm::mat4 translateRectangle2 = glm::translate (glm::vec3(1.28, 0.31, 0));        // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= ( translateRectangle2*scaleRectangle2 );
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(rectangle21);


    Matrices.model = glm::mat4(1.0f);    
    glm::mat4 scaleRectangle3 = glm::scale (glm::vec3(0.6, 0.6, 0.0));
    glm::mat4 translateRectangle3 = glm::translate (glm::vec3(2.56, 0.4, 0));        // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= ( translateRectangle3*scaleRectangle3 );
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(rectangle22);


    Matrices.model = glm::mat4(1.0f);    
    glm::mat4 scaleRectangle4 = glm::scale (glm::vec3(0.4, 0.4, 0.0));
    glm::mat4 translateRectangle4 = glm::translate (glm::vec3(3.84, 0.31, 0));        // glTranslatef
    // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= ( translateRectangle4*scaleRectangle4 );
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(rectangle23);*/


    drawCircle(0, 0, 0, 1.1, 360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCircle1 = glm::translate (glm::vec3(-7.8, -2.5, 0));        // glTranslatef
    Matrices.model *= (translateCircle1);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);  

    drawCircle(-7.8, -2.5, 0, 0.6, 360);
    Matrices.model = glm::mat4(1.0f);
 //   glm::mat4 translateCircle1 = glm::translate (glm::vec3(-7.8, -2.5, 0));        // glTranslatef
    glm::mat4 shadow = glm::scale(glm::vec3(3.0,0.0,0));
    Matrices.model *= (shadow);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle8);  
    
    // draw3DObject draws the VAO given to it using current MVP matrix
    drawCircle(0, 0, 0, 0.5, 360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCircle2 = glm::translate (glm::vec3(-7.8, -1.0, 0));        // glTranslatef
    Matrices.model *= (translateCircle2);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle3);  

    if(obs1_flag == 1 && obs2_flag == 1 && obs3_flag == 1 && obs4_flag == 1 && obs5_flag == 1)
        cout << "YOU WIN\n";
    
    if(obs1_flag == 0)
    {
        drawCircle(3.8, 3.2, 0, 0.3, 360);
        Matrices.model = glm::mat4(1.0f);
        //  glm::mat4 translateCircle2 = glm::translate (glm::vec3(-7.8, -1.0, 0));        // glTranslatef
        //Matrices.model *= (translateCircle2);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(circle2);
    }

    if(obs4_flag == 0)
    {

        drawCircle(5.7, -1.4, 0, 0.2, 360);
        Matrices.model = glm::mat4(1.0f);
        //  glm::mat4 translateCircle2 = glm::translate (glm::vec3(-7.8, -1.0, 0));        // glTranslatef
        //Matrices.model *= (translateCircle2);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(circle5);
    }

    if(obs5_flag == 0)
    {
        drawCircle(5.0, -3.0, 0, 0.4, 360);
        Matrices.model = glm::mat4(1.0f);
        //  glm::mat4 translateCircle2 = glm::translate (glm::vec3(-7.8, -1.0, 0));        // glTranslatef
        //Matrices.model *= (translateCircle2);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(circle6);
    }

    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateTriangle = glm::translate (glm::vec3(-7.15f, -0.9f, 0.0f)); // glTranslatef
    glm::mat4 scaleTriangle = glm::scale (glm::vec3(0.2,0.2,0));
    //  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 triangleTransform = translateTriangle * scaleTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M 

    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(triangle);

    if(obs3_flag == 0)
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateTriangle2 = glm::translate (glm::vec3(5.0f, 1.0f, 0.0f)); // glTranslatef
        glm::mat4 scaleTriangle2 = glm::scale (glm::vec3(0.3,0.3,0));
        //  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        glm::mat4 triangleTransform2 = translateTriangle2 * scaleTriangle2;
        Matrices.model *= triangleTransform2; 
        MVP = VP * Matrices.model; // MVP = p * V * M 

        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(triangle2);
    }
    // Increment angles
    //  float increments = 1;
    //camera_rotation_angle++; // Simulating camera rotation
    //    triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    //  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;


    drawCircle(-7.5, 3.0, 0, 0.5, 360);
    Matrices.model = glm::mat4(1.0f);
    //  glm::mat4 translateCircle2 = glm::translate (glm::vec3(-7.8, -1.0, 0));        // glTranslatef
    //Matrices.model *= (translateCircle2);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle7);

    drawCircle(0, 0, 0, 0.06, 360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateCircle3 = glm::translate (glm::vec3(-7.65, -0.75, 0));        // glTranslatef
    Matrices.model *= (translateCircle3);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle1);  

    //  Matrices.model = glm::mat4(1.0f); 
    // MVP = VP * Matrices.model;
    //glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    //draw3DObject(line);

    Matrices.model = glm::mat4(1.0f);    
    //  glm::mat4 scaleRectangle5 = glm::scale (glm::vec3(1.4, 1.4, 0.0));
    glm::mat4 tr= glm::translate (glm::vec3(7.8,2.5,0));        // glTranslatef
    glm::mat4 rr = glm::rotate((float)(rot_ang*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    glm::mat4 tr1 = glm::translate (glm::vec3(-7.8,-2.5,0));
    Matrices.model *= (  tr1*rr*tr  );
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(rectangle3);



    Matrices.model = glm::mat4(1.0f);    
    //  glm::mat4 scaleRectangle5 = glm::scale (glm::vec3(1.4, 1.4, 0.0));
    glm::mat4 tr2= glm::translate (glm::vec3(0,0+Y,0));        // glTranslatef
    //glm::mat4 rr = glm::rotate((float)(rot_ang*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    //glm::mat4 tr1 = glm::translate (glm::vec3(-7.8,-2.5,0));
    Matrices.model *= (  tr2  );
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    if(Yflag==0)
    {
        Y += 0.02;
    }
    else if(Yflag==1)
    {
        Y-=0.02;
    }
    if(Y<=-3.0)
    {
        Yflag=0;
    }
    else if(Y>=3.0)
    {
        Yflag=1;
    }
    draw3DObject(movingObs);

    if(flag==1)
    {
        if(chance == 0)
        {
            u = 0;
            flag = 0;
            cout << "\nYOU LOSE\n";
        }
        if(-7.8+x/10 > 9.0 || -7.8+x/10 < -9.0 || -2.5+y/10 > 4.0 ||  -2.5+y/10 < -4.0)
        {
            x = 0;
            y = 0;
            u = 18;
            t = 0;
            flag = 0;
            chance--;
        }
        drawCircle(-7.8+x/10,-2.5+y/10, 0, 0.2, 360); // create and initialize circle  
        Matrices.model = glm::mat4(1.0f);
        //      glm::mat4 translateCircle = glm::translate (glm::vec3(-7.8+x/10, -2.5+y/10, 0));        // glTranslatef
        //        Matrices.model *= (translateCircle);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(circle4);
        x += ux*t;
        uy -= 2*t;
        y += uy*t - t*t;
        t = 0.08;
    }

    isCollide();

}


/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}



/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createRectangle (0,0.7,0,0,0.7,0,0,0.7,0,0,0.7,0);
    createLine ();	
    createRectangle2();
    createRectangle3();
    createMovingobs();
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.0f, 0.5f, 0.9f, 1.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    int width = 1366;
    int height = 600;

    GLFWwindow* window = initGLFW(width, height);

    initGL(window, width, height);
    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();



        glfwPollEvents();
        glfwSwapBuffers(window);


        // Poll for Keyboard and mouse events


        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds

        if ((current_time - last_update_time) >= 1.0) { // atleast 0.5s elapsed since last frame               
            cout << "\nLIFE: " << chance << "\n";            
            cout <<  "\nSCORE: " << score << "\n";
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

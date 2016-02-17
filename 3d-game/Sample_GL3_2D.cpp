#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>
#include <cstdlib>
#include <time.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <unistd.h>

#define RESOLUTION 64
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define X .525731112119133606 
#define Z .850650808352039932
using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

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
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
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
int isDragging;
float angle = 0, deltaAngle = 0;
float xDragStart;
float z = 1.0;
float Y = 0;
int state = 1;
float cx=1, cy=1, cz=1, cdx=0, cdy=0, cdz=0, lx=0, ly=0, lz=0, ldx=0, ldy=0, ldz=0, ux=0, uy=1, uz=0, udx=0, udy=0, udz=0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int gaps[10][10];
float player_x=0.0, player_z=0.0, player_y=0.0;
int jump = 0;
float t = 0;
int flag_fall = 0;
int cam_flag = 0;
/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}

/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'c':
        case 'C':
            rectangle_rot_status = !rectangle_rot_status;
        case 'w':
            player_z -= 0.2;
            break;
        case 's':
            player_z += 0.2;
            break;
        case 'a':
            player_x -= 0.2;
            break;
        case 'd':
            player_x += 0.2;
            break;
        case 't':
            cdx = -5-cx;
            cdy = 5-cy;
            cdz = -5-cz;
            ldx = -5-lx;
            ldy = 0-ly;
            ldz = -5-lz;
            udy = +1-uy;
            udz = +1-uz;
            break;
        case 'n':
            cdx = 0;
            cdy = 0;
            cdz = 0;
            ldx = 0;
            ldy = 0;
            ldz = 0;
            udy = 0;
            udz = 0;
            cx = 1;
            cy = 1;
            cz = 1;
            uy = 1;
            break;        
        case 'z':
            z -= 0.05;
            break;
        case 'x':
            z += 0.05;
            break;
        case 'j':
            jump = 1;
            break;  
        case 'p':
            cdx = -8+player_x-cx;
            cdy = -4+player_y-cy;
            cdz = 0+player_z-cz;
            ldx = cdx+10;
            ldy = cdy-cy;
            ldz = cdz-10;   
            cam_flag = 1;
        default:
            break;
    }

    if(player_z>-0.001)
        player_z = -0.001;
    if(player_z<-9.80)
        player_z = -9.78;
    if(player_x>9.80)
        player_x = 9.78;
    if(player_x<0.0)
        player_x = 0.009;

//    cout << "X: " << player_x << "Y: " << player_y << endl;

}
/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{

}

/* Executed when a mouse button 'button' is put into state 'state'
   at screen position ('x', 'y')
   */
void mouseClick (int button, int state, int lx, int ly)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
            {
                isDragging = 1;
                xDragStart = lx;
            }
            else
            {
                angle += deltaAngle;
                isDragging = 0;
            }
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int lx, int ly)
{
    if(isDragging)
    {
        deltaAngle = (lx - xDragStart-1)*0.002;
        cout << deltaAngle << endl;
        ldx = -sin(angle + deltaAngle);
        //        ldy = cos(angle + deltaAngle);
    }
}

void mouseWheel(int button, int dir, int x, int z)
{
    if(dir > 0)
        z -= 0.05;
    else
        z += 0.05;
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) width, (GLsizei) height);

    // set the projection matrix as perspective/ortho
    // Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
}

VAO *triangle, *rectangle, *grid, *tiles[11][11], *player, *circle, *circle1, *circle2;

/*void drawsphere(int ndiv, float radius=1.0) {
  glBegin(GL_TRIANGLES);
  for (int i=0;i<20;i++)
  drawtri(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], ndiv, radius);
  glEnd();
  }*/
/*void createGrid()
  {
  glColor3f(.3,.3,.3);
  glBegin(GL_QUADS);
  glVertex3f( 0,-0.001, 0);
  glVertex3f( 0,-0.001,10);
  glVertex3f(10,-0.001,10);
  glVertex3f(10,-0.001, 0);
  glEnd();

  glBegin(GL_LINES);
  for(int i=0;i<=10;i++) {
  if (i==0) { glColor3f(.6,.3,.3); } else { glColor3f(.25,.25,.25); };
  glVertex3f(i,0,0);
  glVertex3f(i,0,10);
  if (i==0) { glColor3f(.3,.3,.6); } else { glColor3f(.25,.25,.25); };
  glVertex3f(0,0,i);
  glVertex3f(10,0,i);
  };
  glEnd();
  }*/
// Creates the triangle object used in this sample code
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

    circle = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,1,1,GL_LINE);
    circle1 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0,0,GL_LINE);
    circle2 = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allCircleVertices,0,1,GL_LINE);
}

void createPlayer()
{
    static const GLfloat vertex_buffer_data [] = {
        -8,-4,0,
        -8,-6,0,
        -7.5,-6,0,

        -7.5,-5,0,
        -7.5,-4,0,
        -8,-3,0,

        -7.5,-4,0,
        -7.5,-6,0,
        -7.5,-6,-0.5,

        -7.5,-6,-0.5,
        -7.5,-4,-0.5,
        -7.5,-4,0,

        -7.5,-4,-0.5,
        -8,-4,-0.5,
        -7.5,-6,-0.5,

        -7.5,-6,-0.5,
        -8,-6,-0.5,
        -8,-4,-0.5,

        -8,-4,-0.5,
        -8,-6,-0.5,
        -8,-4,0,

        -8,-4,0,
        -8,-6,0,
        -8,-6,-0.5,

        -8,-4,0,
        -7.5,-4,0,
        -8,-4,-0.5,

        -8,-4,-0.5,
        -7.5,-4,-0.5,
        -7.5,-4,0,

        -8,-6,0,
        -7.5,-6,0,
        -7.5,-6,-0.5,

        -7.5,-9,-0.5,
        -8,-9,-0.5,
        -8,-9,0,
    };

    static const GLfloat color_buffer_data [] = {
        0.2, 0.8,0,
        0.2, 0.8,0, 
        0.2, 0.8,0, 

        0.2, 0.8,0, 
        0.2, 0.8,0, 
        0.2, 0.8,0, 

        0.4,0.5,0.2,
        0.4,0.5,0.2,
        0.4,0.5,0.2,

        0.4,0.5,0.2,
        0.4,0.5,0.2,
        0.4,0.5,0.2,

        0.2,0.4,0.6,
        0.2,0.4,0.6,
        0.2,0.4,0.6,

        0.2,0.4,0.6,
        0.2,0.4,0.6,
        0.2,0.4,0.6,

        0.8,0.2,0.6,
        0.8,0.2,0.6,
        0.8,0.2,0.6,

        0.8,0.2,0.6,
        0.8,0.2,0.6,
        0.8,0.2,0.6,

        0.3,0.7,0.3,
        0.3,0.7,0.3,
        0.3,0.7,0.3,

        0.3,0.7,0.3,
        0.3,0.7,0.3,
        0.3,0.7,0.3,

        0.4,0.4,0.4,
        0.4,0.4,0.4,
        0.4,0.4,0.4,

        0.4,0.4,0.4,
        0.4,0.4,0.4,
        0.4,0.4,0.4,
    };

    player = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, 0,0,0);
}

void createGrid()
{
    static const GLfloat vertex_buffer_data [] = {
        -8,-6,0,
        -8,-12,0,
        -7,-12,0,

        -7,-12,0,
        -7,-6,0,
        -8,-6,0,

        -7,-6,0,
        -7,-12,0,
        -7,-12,-1,

        -7,-12,-1,
        -7,-6,-1,
        -7,-6,0,

        -7,-6,-1,
        -8,-6,-1,
        -7,-12,-1,

        -7,-12,-1,
        -8,-9,-1,
        -8,-6,-1,

        -8,-6,-1,
        -8,-12,-1,
        -8,-6,0,

        -8,-6,0,
        -8,-12,0,
        -8,-12,-1,

        -8,-6,0,
        -7,-6,0,
        -8,-6,-1,

        -8,-6,-1,
        -7,-6,-1,
        -7,-6,0,

        -8,-12,0,
        -7,-12,0,
        -7,-12,-1,

        -7,-12,-1,
        -8,-12,-1,
        -8,-12,0,
    };

    static const GLfloat color_buffer_data [] = {
        0.2, 0.8,0,
        0.2, 0.8,0, 
        0.2, 0.8,0, 

        0.2, 0.8,0, 
        0.2, 0.8,0, 
        0.2, 0.8,0, 

        0.4,0.5,0.2,
        0.4,0.5,0.2,
        0.4,0.5,0.2,

        0.4,0.5,0.2,
        0.4,0.5,0.2,
        0.4,0.5,0.2,

        0.2,0.4,0.6,
        0.2,0.4,0.6,
        0.2,0.4,0.6,

        0.2,0.4,0.6,
        0.2,0.4,0.6,
        0.2,0.4,0.6,

        0.8,0.2,0.6,
        0.8,0.2,0.6,
        0.8,0.2,0.6,

        0.8,0.2,0.6,
        0.8,0.2,0.6,
        0.8,0.2,0.6,

        0.3,0.7,0.3,
        0.3,0.7,0.3,
        0.3,0.7,0.3,

        0.3,0.7,0.3,
        0.3,0.7,0.3,
        0.3,0.7,0.3,

        0.4,0.4,0.4,
        0.4,0.4,0.4,
        0.4,0.4,0.4,

        0.4,0.4,0.4,
        0.4,0.4,0.4,
        0.4,0.4,0.4,
    };

    grid = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data,color_buffer_data,GL_FILL);
    for(int i=0;i<10;i++)
    {
        for(int j=0;j<10;j++)
        {
            tiles[i][j] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data,color_buffer_data,GL_FILL);
        }
    }

}

void createTriangle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        0, 1,0, // vertex 0
        -1,-1,0, // vertex 1
        1,-1,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 0
        0,1,0, // color 1
        0,0,1, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported static
    const GLfloat vertex_buffer_data [] = {
        -1.2,-1,0, // vertex 1
        1.2,-1,0, // vertex 2
        1.2, 1,0, // vertex 3

        1.2, 1,0, // vertex 3
        -1.2, 1,0, // vertex 4
        -1.2,-1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0, // color 3

        0,1,0, // color 3
        0.3,0.3,0.3, // color 4
        1,0,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void drawsphere()
{
    glColor3f(1.0, 1.0, 1.0); // set drawing color to white
    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.75);
    glutSolidSphere(0.75, 20, 20);
    glPopMatrix();
}

void checkXpos()
{
    if(player_x>=1.4 && player_z<=-1.4 && player_x<=3.5 && player_z>=-3.2)
    {
        if(player_x < 1.4+0.5)
            player_x -= 0.2;
        else if(player_x > 3.5-0.5)
            player_x += 0.2;
    }
    
    if(player_x>=5.6 && player_z<=-2.6 && player_x<=7.3 && player_z>=-4.2)
    {
        if(player_x < 5.6+0.5)
            player_x -= 0.2;
        else if(player_x > 7.3-0.5)
            player_x += 0.2;
    }
    
    if(player_x>=7.6 && player_z<=-4.2 && player_x<=9.4 && player_z>=-6.2)
    {
        if(player_x < 7.6+0.5)
            player_x -= 0.2;
        else if(player_x > 9.4-0.5)
            player_x += 0.2;
    }
    
    if(player_x>=3.5 && player_z<=-8.6 && player_x<=5.3 && player_z>=-9.8)
    {
        if(player_x < 3.5+0.5)
            player_x -= 0.2;
        else if(player_x > 5.3-0.5)
            player_x += 0.2;
    }
    
}

void checkZpos()
{
    if(player_x>=1.4 && player_z<=-1.4 && player_x<=3.5 && player_z>=-3.2)
    {
        if(player_z > -1.4-0.5)
            player_z += 0.2;
        else
            player_z -= 0.2;
    }
    
    if(player_x>=5.6 && player_z<=-2.6 && player_x<=7.3 && player_z>=-4.2)
    {
        if(player_z > -2.6-0.5)
            player_z += 0.2;
        else
            player_z -= 0.2;
    }
    
    if(player_x>=7.6 && player_z<=-4.2 && player_x<=9.4 && player_z>=-6.2)
    {
        if(player_z > -4.2-0.5)
            player_z += 0.2;
        else
            player_z -= 0.2;
    }
    
    if(player_x>=3.5 && player_z<=-8.6 && player_x<=5.3 && player_z>=-9.8)
    {
        if(player_z > -8.6-0.5)
            player_z += 0.2;
        else
            player_z -= 0.2;
    }
}
    
void checkGaps()
{
    for(int i=0;i<10;i++)
    {
        for(int j=0;j<10;j++)
        {
            if(gaps[i][j]==1)
            {   
                if(player_x>=i && player_x<=i+1 && -player_z>=j && -player_z<=(j+1) && player_y<=0.1)
                {
                    cout << player_x << " " << player_z << " " << i << " " << j << " " << endl;
                    flag_fall = 1;
                }
            }
        }
    }
}

void checkObs()
{
    cout << player_x << " " << player_z << "\n";
    if(player_x<=0.1 && player_z<=-8.6 && player_z>=-10.0)
    {
        player_x = 0;
        player_z = 0;
    }
    if(player_x>=7.6 && player_x<= 8.5 && player_z<=-0.78 && player_z>=-2.4)
    {
        player_x = 0;
        player_z = 0;
    }
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
glm::mat4 transTile[11][11];


    
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);
    if(cam_flag == 1)
    {    
            cdx = -8+player_x;
            cdy = -4+player_y;
            cdz = 0+player_z;
            ldx = cdx+10;
            ldy = cdy;
            ldz = cdz-10;  
            cam_flag = 0; 
    }
    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(cx+cdx,cy+cdy,cz+cdz), glm::vec3(lx+ldx,ly+ldy,lz+ldz), glm::vec3(ux+udx,uy+udy,uz+udz)); // Fixed camera for 2D (ortho) in XY plane


    Matrices.projection = glm::ortho(-10.0f*z, 10.0f*z, -10.0f*z, 10.0f*z, 0.1f, 500.0f);
    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    // Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);

    /* Render your scene */

    glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
    glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
    //Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    // draw3DObject(triangle);
    drawCircle(-7,-3,-7,0.4,360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 R = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(-7,-3,-7));  // rotate about vector (1,0,0)
    glm::mat4 P = R;
    Matrices.model *= P; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);
    // draw3DObject draws the VAO given to it using current MVP matrix
    
    drawCircle(0,-3,-10,0.4,360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 R4 = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,-3,-10));  // rotate about vector (1,0,0)
    glm::mat4 P4 = R4;
    Matrices.model *= P4; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle2);


    drawCircle(-3,-3,-5,0.4,360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 R1 = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(-3,-3,-5));  // rotate about vector (1,0,0)
    glm::mat4 P1 = R1;
    Matrices.model *= P1; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);


    drawCircle(0,-3,0,0.4,360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 R2 = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,-3,0));  // rotate about vector (1,0,0)
    glm::mat4 P2 = R2;
    Matrices.model *= P2; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle);

    drawCircle(-8,-4,-5,0.4,360);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 R3 = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(1,-1,1));  // rotate about vector (1,0,0)
    glm::mat4 P3 = R3;
    Matrices.model *= P3; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(circle1);

    Matrices.model = glm::mat4(1.0f);
    if(jump == 1 && player_y >= 0)
    {
        player_y += 2*t*0.05-t*t*0.025;
        t += 0.05;
    }
    else
    {
        player_y = 0;
        jump = 0;
        t = 0;
    }

    checkXpos();
    checkZpos();
    checkGaps();

    if(flag_fall == 1)
    {
        while(player_y > -50.0)
        {
            player_y -= 0.5;
            sleep(0.0005);
        }
        flag_fall = -1;
    }
    if(flag_fall == -1)
    {
        player_y = 0;
        player_x = 0;
        player_z = 0;
        flag_fall = 0;
    }
    glm::mat4 translatePlayer = glm::translate (glm::vec3(-2+player_x, 0+player_y, 0+player_z));        // glTranslatef
    
    //   glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatePlayer);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(player);
    //
    for(int i=0;i<10;i++)
    {
        for(int j=0;j<10;j++)
        {
            if(gaps[i][j]==0)
            {

                Matrices.model = glm::mat4(1.0f);
                if(i==2 && j==2 || i==3 && j==6 || i==9 && j==4 || i==5 && j==8)
                {
                    transTile[i][j] = glm::translate (glm::vec3(-2+j*1.03, -Y, -i*1.03));
                }        // glTranslatef
                else
                {
                    transTile[i][j] = glm::translate (glm::vec3(-2+j*1.03, 0, -i*1.03));
                }
                //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
                Matrices.model *= (transTile[i][j]);
                MVP = VP * Matrices.model;
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

                // draw3DObject draws the VAO given to it using current MVP matrix
                draw3DObject(tiles[i][j]);
            }
        }
    }

    if(Y<=-4)
    {
        state = -1;
    }
    else if(Y>1)
    {
        state = +1;
    }

    if(state == 1)
        Y -= 0.04;
    else if(state == -1)
        Y += 0.04;

    /*glPushMatrix();
      glTranslatef(7.5,7.5,0);
      drawsphere();
      glPopMatrix();*/
    /*    glColor3f(1.0, 1.0, 1.0); // set drawing color to white
          glPushMatrix();
          glTranslatef(0.0, 0.0, 0.75);
          glutSolidSphere(0.75, 20, 20);
          glPopMatrix();*/
    checkObs();
    // Swap the frame buffers
    glutSwapBuffers ();

    // Increment angles
    float increments = 1;

    //camera_rotation_angle++; // Simulating camera rotation
    triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);
    glutMouseWheelFunc(mouseWheel);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)

    //    glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
    // Create the models
    createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createGrid();
    createPlayer();
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    for(int i=0;i<10;i++)
    {
        for(int j=0;j<10;j++)
        {
            gaps[i][j]=0;
        }
    }
    int j;
    for(int i=0;i<10;i++)
    {
        int count = rand()%2+2;
        while(count--)
        {
            j=rand()%10;
            if(i!=0 && j!=0)
                gaps[i][j] = 1;             
        }
    }

    reshapeWindow (width, height);

    // Background color of the scene
    glClearColor (0.39f, 0.58f, 0.92f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    createRectangle ();

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    int width = 1920;
    int height = 1080;

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

    initGL (width, height);

    glutMainLoop ();

    return 0;
}

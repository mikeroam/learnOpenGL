#include <iostream>
#include <glad/glad.h>  // mjr added
#include <math.h>    // we added this for trig
#include <time.h>     // we added this


#include <fstream>    // so we can read external shader text files
#include <sstream>    // so we can read external shader as string

/*
 This is source code from http://learnopengl.com/#!Getting-started/Hello-Triangle
 which I've spliced into a copy of my glitter xcode project
 and then I've spent hours getting it to compile on mac xcode.
 Using cmake and gala(?glad?) and glfw.
*/


/* following is header suggestion from
 http://stackoverflow.com/questions/12229714/building-glew-for-mac-osx
 answer by Brett Hale
 ...
 hmmm, is this necessary since glfw includes all necessary headers anyway??
 see  http://www.glfw.org/docs/latest/quick.html
 */
#if defined (__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl3.h>   /* assert OpenGL 3.2 core profile available. */
#endif

#define GLFW_INCLUDE_GL3    /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

/* Brett Hale had <GL/glfw.h> which isn't found on Mike's xcode. Either install it or ... */
#include <GLFW/glfw3.h>    /* simple and easy to use. omit legacy GL. */


/* First header from learnopengl.com with Mike's attempts...
 // GLEW
 #define GLEW_STATIC
 #include <OpenGL/GL.h>    //   mjr adding this
 //#include <glad/glad.h>    // mjr tried this to replace learnopengl.com's #include <GL/glew.h>
 
 // GLFW
 #include <GLFW/glfw3.h>   // mjr can't blank this
 */




// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode); // <== mjr was blanking



// Window dimensions

const GLuint WIDTH = 800, HEIGHT = 600;

// fun with rotation...
GLfloat piOver180 = M_PI/180.0; // for turning Xdeg to Radians ... xDeg * πrad/180deg for cross-cancel.
GLfloat deltaAngle =  30.0;   // trying to be howManyDegreesPerSecond of turn   // was good with 0.0001;
GLfloat length = 1.0;  // keys MINUS and PLUS to lower and raise


/*
 Now we'll try to load some shaders from external files
 source stackoverflow.com/questions... 
 
note: GLchar *   and  char *  are recognized by compiler as same thing ("aka")
 
  had some luck with:    const char * getAShader(std::string path ) {
  had some luck with:    std::string getAShader( std::string path ) {
*/
const GLchar* getAShader(std::string path ) {
    std::ifstream theFile(path);
    if (! theFile.is_open( ) ) { /* trouble! */
        std::cout << "tried to read from file at " << path << " but told file not is_open( )" << std::endl;
        return "";
    } else { /* Ok, proceed ... */
        std::stringstream ourShaderBuffer;
        ourShaderBuffer << theFile.rdbuf( );
        // conversion from string to GLchar *   is from
        //  http://stackoverflow.com/questions/34836454/neither-vertex-shader-nor-fragment-shader-are-compiling-after-loading-from-a-fil
        // char * vertShaderSource = ourVertexShaderBuffer.c_str( );//const_cast<const GLchar*>(ourVertexShaderBuffer.str( ));
        std::string shaderSourceStr = ourShaderBuffer.str( );
        /*std::cout << "tried to read from file at " << path << " and got:"  << std::endl;
        std::cout << shaderSourceStr << std::endl;
        std::cout << "......." << std::endl;
        std::cout << "Here's the c_str( ):" << std::endl;
        std::cout << shaderSourceStr.c_str( ) << std::endl;
        std::cout << "-=-=-=-=-=-=-=-=" << std::endl;*/
        
        
        // The following might be overkill, but has fixed our ability to get a null-terminated c string from a text file.
        // from http://www.cplusplus.com/reference/string/string/c_str/
        // which says that c_str( ) returns a "null-terminated sequence of characters (i.e., a C-string)"
        char * ourCString = new char [shaderSourceStr.length()+1];
        std::strcpy (ourCString, shaderSourceStr.c_str());
        
        return ourCString;
        // return shaderSourceStr.c_str( );  // is this getting \0 ??
        // return shaderSourceStr;  // is this getting \0 ??
    }
} // getAShader( )


// Shaders
/*
We can copy these shaders by hand to
~/Documents/learnOpenGL/learnopengl-for-cmake/learn/Build/learn/Debug
but now we've figured out how to tell xcode to put the files into the build destination:
-Make sure we're in standard editor and leftmost column ("navigator") is visible.
-In navigator, click outermost upper project name ("learn").
-Just above the name, choose folder icon ("Project navigator") for view.
-In central main view area, upper left has icon: use it to show a sub sidebar "Project and Targets List"
 xcdoc://?url=developer.apple.com/library/etc/redirect/xcode/devtools/1157/recipes/xcode_help-project_editor/Articles/CreatingaCopyFilesBuildPhase.html
-Now can follow the help file re "Copying Files While Building a Product"…
 -Choose a target in that tiny left sidebar of main central window (we're currently on 'learn' target).
 -If there is not a "Copy Files" rule then add one.
 -Can drag files onto the list of who gets copied.
 -Specify Destination "Products Directory".
 Voila!
*/

const GLchar* vertexShaderSource = getAShader("./vertexshader_mh1.vert");  // "./" isn't necessary, right?
const GLchar* fragmentShaderSource = getAShader("./fragshader_mh4.frag");
//const GLchar* fragmentShaderSource2 = getAShader("fragshader_mh2.frag");
// should this experiment use strcopy?? const GLchar * fragmentShaderSource = getAShader("fragshader_mh3.frag").c_str( );

/*
// Alternative old way: load these shaders as direct strings:
 const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"}\n\0";


const GLchar* fragmentShaderSource = "#version 330 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(1.0f, 0.9f, 0.7f, 1.0f);\n"  // was vec4(1.0f, 0.5f, 0.2f, 1.0f);
"}\n\0";
*/



double prevTimeD;
GLfloat angle = 0.0;

// rotates some triangles
void rotateVertices(GLfloat vertices[]){
    
    // originally lower left corner...
    vertices[0] = cos(angle * piOver180);   //   <== oops, we were saying angle * 180/π, should have been ang * π/180
    vertices[1] = sin(angle * piOver180);
    // vertices[2] is a z, which probably doesn't make any difference visually, just coming toward us out of screen??
    vertices[2] = 0.25f;
    
    // originally lower right corner...
    vertices[3] = length * cos((angle-120.0) * piOver180);  // was -120.0
    vertices[4] = length * sin((angle-120.0) * piOver180);  // was -120.0
    vertices[5] = 0.5f;   // I am the z
    
    // originally center top vertex...
    vertices[6] = cos((angle+120.0) * piOver180);    // was +120.0
    vertices[7] = sin((angle+120.0) * piOver180);    // was +120.0
    vertices[8] = 0.75f;  // I am the z
    
    // originally right shoulder vertex...
    // without the following two lines, the right shoulder is fixed in one place..
    vertices[9] = 0.5 + cos((angle-80.0) * piOver180);
    vertices[10] = 0.5 + sin((angle-80.0) * piOver180);
    vertices[11] = cos(angle * piOver180);  // I am the z
    
    // angle += deltaAngle;  // 0.0001 is nice, 'R' keys makes it negative for rotate other way;
    double secondsSincePrevMove = (glfwGetTime()-prevTimeD);
    angle += ( deltaAngle * secondsSincePrevMove );
    while (angle < 0.0) {
        angle += 360.0;
    }
    while (angle > 360.0) {
        angle -= 360.0;
    }
    
    // std::cout << secondsSincePrevMove << "sec, " << angle << "angle ";
    // std::cout << angle << "angle ";
    
    prevTimeD = glfwGetTime();   //time(NULL);
}



// will try to rotate a tetrahedron made up of many triangles
// for now it just fools with some of the vertices
void rotateTetrahedron(GLfloat vertices[]){
    
    // originally lower left corner...
    vertices[0] = cos(angle * piOver180);   //   <== oops, we were saying angle * 180/π, should have been ang * π/180
    vertices[1] = sin(angle * piOver180);
    // vertices[2] is a z, which probably doesn't make any difference visually, just coming toward us out of screen??
    vertices[2] = 0.25f;
    
    // originally lower right corner...
    vertices[3] = length * cos((angle-120.0) * piOver180);  // was -120.0
    vertices[4] = length * sin((angle-120.0) * piOver180);  // was -120.0
    vertices[5] = 0.5f;   // I am the z
    
    // originally center top vertex...
    vertices[6] = cos((angle+120.0) * piOver180);    // was +120.0
    vertices[7] = sin((angle+120.0) * piOver180);    // was +120.0
    vertices[8] = 0.75f;  // I am the z
    
    // originally right shoulder vertex...
    // without the following two lines, the right shoulder is fixed in one place..
    vertices[9] = 0.5 + cos((angle-80.0) * piOver180);
    vertices[10] = 0.5 + sin((angle-80.0) * piOver180);
    vertices[11] = cos(angle * piOver180);  // I am the z
    
    // angle += deltaAngle;  // 0.0001 is nice, 'R' keys makes it negative for rotate other way;
    double secondsSincePrevMove = (glfwGetTime()-prevTimeD);
    angle += ( deltaAngle * secondsSincePrevMove );
    while (angle < 0.0) {
        angle += 360.0;
    }
    while (angle > 360.0) {
        angle -= 360.0;
    }
    
    // std::cout << secondsSincePrevMove << "sec, " << angle << "angle ";
    // std::cout << angle << "angle ";
    
    prevTimeD = glfwGetTime();   //time(NULL);
}  // rotateTetrahedron



// re-factoring
GLfloat* setUpVertices(  ) {
    
    GLfloat vertLowerLeft[] = {  0.0f, 0.0f, 0.0f };
    GLfloat vertLowerRight[] = { 1.0f, 0.0f, 0.0f };
    GLfloat vertTop[] = {  0.5f,  1.0f, 0.0f };
    GLfloat vertNose[] = { 0.5f, 0.2f, 1.0f };
    static GLfloat vertices[] = {
        vertLowerLeft[0],vertLowerLeft[1],vertLowerLeft[2],    vertLowerRight[0],vertLowerRight[1],vertLowerRight[2],    vertTop[0],vertTop[1],vertTop[2],   // flat
        vertLowerLeft[0],vertLowerLeft[1],vertLowerLeft[2],    vertNose[0],vertNose[1],vertNose[2],                      vertTop[0],vertTop[1],vertTop[2],          // leftslant
        vertNose[0],vertNose[1],vertNose[2],                   vertLowerRight[0],vertLowerRight[1],vertLowerRight[2],    vertTop[0],vertTop[1],vertTop[2],   // rightslant
        vertLowerLeft[0],vertLowerLeft[1],vertLowerLeft[2],    vertLowerRight[0],vertLowerRight[1],vertLowerRight[2],    vertNose[0],vertNose[1],vertNose[2]  // botslant
    };
    return vertices; // does anybody know how many elements this has?
}


int min(int a, int b){
    if(a < b){
        return a;
    }else{
        return b;
    }
}
int max(int a, int b){
    if(a < b){
        return b;
    }else{
        return a;
    }

}

void testColors(){
    std::string colors[1530] = {};
    int prevInt = 254;
    for(int i = 0; i < 1530; i++){
        int r = max(min(510 - min(abs(i - 1530), i), 255), 0);
        int g = max(min(510 - abs(i - 510), 255),0);
        int b = max(min(510 - abs(i - 1020), 255), 0);
        for(int j = 0; j < i; j++){
            if(colors[j] == std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b)){
                std::cout<< i << " and " << j << " are the same!" << std::endl;
            }
        }
        if(abs(r + g + b - prevInt) != 1) std::cout << r << " " << g << " " << b << " " << r + g + b << " " << prevInt << std::endl;
        prevInt = r + g + b;
        colors[i] = std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
    }
}
// The MAIN function, from here we start the application and run the game loop
int main()
{
    testColors();
    //std::cout << vertexShaderSource << std::endl;
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // mjr added
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    
    // mjr added the following, copied from the original glitter project from github.
    // Check for Valid Context
    if (window == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();  // mjr added  ***** THIS MADE IT WORK (along with include glad/glad.h above
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));  // mjr added
    
    // Set the 'required'(?) callback functions
    glfwSetKeyCallback(window, key_callback );   // necessary??  mjr can't blank it
    
    
    // Define the viewport dimensions
    // glViewport(0, 0, WIDTH, HEIGHT);   <== mjr is blanking
    
    
    // Build and compile our shader program
    // Vertex shader
    
    // mjr: the glCreateShader( ) was getting EXC_BAD_ACCESS (code=1) which may be indicative
    // of releasing from memory an object that isn't mine to release. Somebody online with this
    // problem realized that they hadn't set up the openGL context prior to calling glCreateShader
    // but I already have context setup above, I think.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for compile time errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for compile time errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    
    // Set up vertex data  (note: these initial coords are immediately replaced by trig data down below
//    GLfloat vertices[] = {
//        -0.9f, -0.9f, 0.0f, // lower Left
//        -0.0f, -0.9f, 0.0f, // lower Right
//        -0.9f,  -0.0f, 0.0f,  // Top
//        0.9f, 0.9f, 0.0f,// upper right (shoulder)
//        0.9f, 0.0f, 0.0f,  // steady triangle trying to float above the rotating stuff
//        0.0f, 0.9f, 0.0f/*,
//        0.0f, 1.0f, 0.75f*/
//    };
    GLfloat verticesTemp[] = {
        0.0f, 0.0f, 0.0f, // lower Left
        1.0f, 0.0f, 0.0f, // lower Right
        0.0f,  1.0f, 0.0f,  // Top
        0.0f, 0.0f, 1.0f, // Nose
    };
    
    GLfloat vertLowerLeft[] = {  0.0f, 0.0f, 0.0f };
    GLfloat vertLowerRight[] = { 1.0f, 0.0f, 0.0f };
    GLfloat vertTop[] = {  0.5f,  1.0f, 0.0f };
    GLfloat vertNose[] = { 0.5f, 0.2f, 1.0f };
    GLuint objectStarts[] = {0, 5};
    GLuint howManyObjects = 1;
    GLuint howManyVertices = 36;  // was 3 until we added right shoulder
    // GLfloat * vertices; //[howManyVertices];
    // vertices = setUpVertices(); /* {
    GLfloat vertices[] = {
        vertLowerLeft[0],vertLowerLeft[1],vertLowerLeft[2],    vertLowerRight[0],vertLowerRight[1],vertLowerRight[2],    vertTop[0],vertTop[1],vertTop[2],   // flat
        vertLowerLeft[0],vertLowerLeft[1],vertLowerLeft[2],    vertNose[0],vertNose[1],vertNose[2],                      vertTop[0],vertTop[1],vertTop[2],          // leftslant
        vertNose[0],vertNose[1],vertNose[2],                   vertLowerRight[0],vertLowerRight[1],vertLowerRight[2],    vertTop[0],vertTop[1],vertTop[2],   // rightslant
        vertLowerLeft[0],vertLowerLeft[1],vertLowerLeft[2],    vertLowerRight[0],vertLowerRight[1],vertLowerRight[2],    vertNose[0],vertNose[1],vertNose[2]  // botslant
    };
//    GLfloat vertices[] = {
//        verticesTemp[0], verticesTemp[1], verticesTemp[2],
//        verticesTemp[1], verticesTemp[2], verticesTemp[3],
//        verticesTemp[2], verticesTemp[3], verticesTemp[0],
//        verticesTemp[3], verticesTemp[0], verticesTemp[1]
//    };
    //mark the indices of the last vertices in an object. It makes sense I promise
    
    GLuint VBO, VAO;  // buffer(s)) and attribute pointers
    
    //  glGenVertexArrays(1, &VAO); ... through glBindVertexArray(0); was here
    
    
    // let's play with a vertex (mjr, hb)
    GLfloat deltaV0 = 0.1;
    
    
    
    
  //  double weirdTime = time(NULL);
    prevTimeD = glfwGetTime();

        // Game loop…
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        
        // std::cout << ".";    // << yes, dots show up
        
        // Render
        // Clear the colorbuffer
        glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glad_glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // we're going to fool with the vertices
        /* fooling one...
        if (vertices[0] > 1.0) {
            deltaV0 = -0.1;
        } else if (vertices[0] < -1.0) {
            deltaV0 = 0.1;
        }
        vertices[0] += deltaV0;
        //std::cout << vertices[0] << ",";
         */
        
        //rotateVertices(vertices);
        rotateTetrahedron(vertices);
        /*
        vertices[0] += 0.01;
        vertices[1] += 0.01;
        vertices[3] += 0.01;
        vertices[5] -= 0.01;
        vertices[6] -= 0.01;*/
        // ---------
        glGenVertexArrays(1, &VAO);   // mjr tried glGenVertexArraysAPPLE(1, &VAO);
        glGenBuffers(1, &VBO);
        // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
        glBindVertexArray(VAO);   // mjr tried glBindVertexArrayAPPLE(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // fill VBO with data
        /* 
         Uh-oh; what are these '3's? Apparently NOT the size of our vertices array (which is now 4) but instead
         specifying that our vertices use 3 coords (x,y,z) rather than 4 (x,y,z,w)
         see   https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml
           size: Specifies the number of components per generic vertex attribute (must be 1,2,3,4).
           stride: Specifies the byte offset between consecutive generic vertex attributes.
        */
        
        
        
         /* Note that this is allowed, the call to glVertexAttribPointer
                                           registered VBO as the currently bound vertex buffer object so 
                                           afterwards we can safely unbind */
        
        //glBindVertexArray(0); // mjr earlier tried glBindVertexArrayAPPLE(0);
        // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
        // ---------
        glBindVertexArray(VAO);   // mjr earlier tried glBindVertexArrayAPPLE(VAO);
        
        // Draw our first triangle     // ??? how does this differ from glDrawArrays(GL_TRIANGLES,0,3) below??
        glUseProgram(shaderProgram);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, VAO);
        glVertexAttribPointer(/*index*/0, /*sizePerVertex*/3, /*type*/GL_FLOAT, /*normalized*/GL_FALSE, /*stride*/3 * sizeof(GLfloat), (GLvoid*)0);
        //glDrawArrays(GL_t)
        for(GLuint i = 0; i < howManyVertices; i += 3){
            //if(i + 1 == howManyObjects){
                glDrawArrays(GL_TRIANGLES, i, 3);
            //}
            //glDrawArrays(GL_TRIANGLES, 0, 3);
        }
       // worked: glDrawArrays(GL_TRIANGLES, 0, /*howManyVertices*/3);    //  ??? is this the actual drawing?
        /*glDrawArrays(GL_TRIANGLE_STRIP, 0, howManyVertices);
        for(GLuint i = 0; i < howManyObjects; i++){
            glBindVertexArray(VAO);
            if(i + 1 == howManyObjects){
                glDrawArrays(GL_TRIANGLE_STRIP, objectStarts[i], howManyVertices - objectStarts[i]);
            } else {
                glDrawArrays(GL_TRIANGLE_STRIP, objectStarts[i], objectStarts[i + 1] - objectStarts[i]);
            }
            glBindVertexArray(0);
            
        }    //  ??? is this the actual drawing?
        glBindVertexArray(0);   // mjr earlier tried glBindVertexArrayAPPLE(0);
        */
        // Swap the screen buffers
        glfwSwapBuffers(window);
        
        

    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);    // glDeleteVertexArraysAPPLE(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}


/* 
 OUR KEYS:
 esc    quit
 'r'    reverse spin direction
 '-'    shorten a leg
 '='    lengthen a leg
 
 This callback is called whenever a key is pressed/released via GLFW
   See http://www.glfw.org/docs/latest/group__input.html
 
 Arguments:
 'action' can also use GLFW_RELEASE and GLFW_REPEAT
 'modifiers' can also use GLFW_MOD_ALT or  GLFW_MOD_CONTROL  or  GLFW_MOD_SUPER
 'scancode' is platform specific
*/
void key_callback( GLFWwindow* window, int key, int scancode, int action, int modifiers )
{
     if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
        //std::cout << "esc!" << std::endl;
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if ((key == GLFW_KEY_R) && (action == GLFW_PRESS)) {
        deltaAngle *= -1;
        //std::cout << "R" << std::endl;
    }
    if ((key == GLFW_KEY_MINUS) && (action == GLFW_PRESS)) {
        length *= 0.8;
        //std::cout << "-" << std::endl;
    }
    if ((key == 333 /* mac number pad '-' key */) && (action == GLFW_PRESS)) {
        length *= 0.8;
        //std::cout << "-" << std::endl;
    }
    if ((key == GLFW_KEY_EQUAL) /* plus key */ && (action == GLFW_PRESS)) { //  && (modifiers == GLFW_MOD_SHIFT)) {
        length /= 0.8;
        //std::cout << "=" << std::endl;
    }
    if ((key == 334) /* mac number pad plus key */ && (action == GLFW_PRESS)) {
        length /= 0.8;
        //std::cout << "=" << std::endl;
    }
   // std::cout << key  << std::endl;
}




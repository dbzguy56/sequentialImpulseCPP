#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "cube.h"
#include "tetrahedron.h"

#define MAT_VALUE_LOC(mat) &mat[0][0]
#define IDENTITY_MATRIX glm::mat4(1.0f)
#define KEY_PRESSED(key) glfwGetKey(window, key) == GLFW_PRESS
#define REMOVE_ELEMENT(v, i) v.erase(v.begin() + i)

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920

using namespace std;

const int SIMPLEX_INDICES_NUM = 12;
unsigned int simplex_indices[SIMPLEX_INDICES_NUM] = {
  0, 1, 2,
  0, 1, 3,
  0, 2, 3,
  1, 2, 3
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

float to_rgb(int num) {
  return (float) num / 255.0f;
}

// TODO: Fix naming of variables/function to be more consistent (snake instead of camel case)
void checkShaderCompileErrors(unsigned int shader, string type) {
  int success;
  GLint maxLength = 0;
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

      // The maxLength includes the NULL terminating character
      vector<GLchar> errorLog(maxLength);
      glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
      cout << "ERROR::SHADER::COMPILATION_ERROR of type: " << type << "\n" << endl;
      for (GLchar i : errorLog) {
        cout << i;
      }
      cout << "\n -----" << endl;
      glDeleteShader(shader);
    }
  }
  else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

      vector<GLchar> infoLog(maxLength);
      glGetProgramInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
      cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << endl;
      for (GLchar i : infoLog) {
        cout << i;
      }
      cout << "\n -----" << endl;
      glDeleteProgram(shader);
    }
  }
}

void printVec3Vector(vector<glm::vec3> v) {
  cout << "printVec3Vector: " << endl;
  for (int i = 0; i < v.size(); i++){
    cout << "\ti = " << i << ", vec3 = " << glm::to_string(v[i]) << endl;
  }
}

enum EvolutionStage {
  NO_INTERSECTION,
  FOUND_INTERSECTION,
  STILL_EVOLVING
};

glm::vec3 support(Shape shape, glm::vec3 direction) {
  float furthestDistance = -FLT_MAX;
  glm::vec3 furthestVertex = glm::vec3(0.0f, 0.0f, 0.0f);

  for (int i = 0; i < *(shape.size); i++) {
    glm::vec3 v = shape.world_vertices[i];
    float distance = dot(v, direction);
    if (distance > furthestDistance) {
      furthestDistance = distance;
      furthestVertex = v;
    }
  }
  return furthestVertex;
}

glm::vec3 getSupport(Shape shapeA, Shape shapeB, glm::vec3 direction) {
  return support(shapeA, direction) - support(shapeB, -direction);
}

bool addSupport(vector<glm::vec3> &simplex, Shape shapeA, Shape shapeB,
                glm::vec3 direction) {
  glm::vec3 new_point = getSupport(shapeA, shapeB, direction);
  // Support termination conditions from Erin Catto's 2010 presentation advice
  if (find(simplex.begin(), simplex.end(), new_point) != simplex.end()) {
    return false;
  }
  simplex.push_back(new_point);
  return (dot(direction, new_point) >= 0.0f);
}

glm::vec3 averagePoint(glm::vec3 points[], int size) {
  glm::vec3 avg = glm::vec3(0.0f, 0.0f, 0.0f);
  for (int i = 0; i < size; i++){
    avg += points[i];
  }
  avg /= size;

  return avg;
}

EvolutionStage evolveSimplex(vector<glm::vec3> &simplex, Shape shapeA, Shape shapeB,
                             glm::vec3 &direction) {
  glm::vec3 avgPointDifference = averagePoint(shapeB.world_vertices, *shapeB.size)
        - averagePoint(shapeA.world_vertices, *shapeA.size);
  glm::vec3 ab = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 ac = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 a0 = glm::vec3(0.0f, 0.0f, 0.0f);

  switch(simplex.size()) {
    case 0:
      direction = avgPointDifference;
      break;

    case 1:
      // flip the direction
      direction = -avgPointDifference;
      break;

    case 2:
      // line ab is the line formed by the first 2 vertices
      ab = simplex[1] - simplex[0];
      // line a0 is the line from the first vertex to the origin
      a0 = -simplex[0];
      glm::vec3 temp = cross(ab, a0);
      direction = cross(temp, ab);
      break;

    case 3:
      ac = simplex[2] - simplex[0];
      ab = simplex[1] - simplex[0];
      direction = cross(ac, ab);

      // ensure that the direction points toward origin
      a0 = -simplex[0];
      if (dot(direction, a0) < 0) {
        direction = -direction;
      }
      break;
    case 4:
      // calculate the 3 edges of interest
      glm::vec3 da = simplex[3] - simplex[0];
      glm::vec3 db = simplex[3] - simplex[1];
      glm::vec3 dc = simplex[3] - simplex[2];

      // calculate direction to the origin
      glm::vec3 d0 = -simplex[3];

      // check triangles a-b-d, b-c-d, and c-a-d
      glm::vec3 abd_norm = cross(da, db);
      glm::vec3 bcd_norm = cross(db, dc);
      glm::vec3 cad_norm = cross(dc, da);

      if (dot(abd_norm, d0) > 0.0f) {
        // origin outside of a-b-d, eliminate c
        REMOVE_ELEMENT(simplex, 2);
        direction = abd_norm;
      }
      else if (dot(bcd_norm, d0) > 0.0f) {
        // origin is outside of b-c-d, elimante a
        REMOVE_ELEMENT(simplex, 0);
        direction = bcd_norm;
      }
      else if (dot(cad_norm, d0) > 0.0f) {
        // origin is outside of c-a-d, eliminate b
        REMOVE_ELEMENT(simplex, 1);
        direction = cad_norm;
      }
      else {
        cout << "found intersection!" << endl;
        // the origin is inside of all of the triangles
        return FOUND_INTERSECTION;
      }
      break;

    default:
      cout << "Can't have a simplex with " << simplex.size() << " vertices!"  << endl;
      break;
  }

  EvolutionStage evolveResult;
  if (addSupport(simplex, shapeA, shapeB, direction)) {
    evolveResult = STILL_EVOLVING;
  }
  else {
    evolveResult = NO_INTERSECTION;
  }

  return evolveResult;
}

bool gjk(Shape shapeA, Shape shapeB, vector<glm::vec3> &simplex) {
  EvolutionStage evolveResult = STILL_EVOLVING;
  glm::vec3 direction = glm::vec3(1.0f, 0.0f, 0.0f);

  // NOTE: GJK is very sensitive to numerical issues, termination problems may
  // occur (see Gino's "Ill-conditioned error bounds"). I am going with
  // a max number of loop iterations and declaring no intersection. I ran
  // into this problem when one of the object's is either barely in or out
  // (colliding) with the other object. I decided with "no intersection"
  // because the simplex is needed for EPA, if I go that route. (Also at
  // the moment I am thinking if we have gravity/other physics, the collision
  // will either get closer or further anyways). I might look more into this
  // in the future.
  int loopIterations = 0;
  while (evolveResult == STILL_EVOLVING && loopIterations != 15) {
    cout << "pos = " << glm::to_string(shapeA.pos) << endl;
    evolveResult = evolveSimplex(simplex, shapeA, shapeB, direction);
    loopIterations++;
  }

  return ((evolveResult == FOUND_INTERSECTION) ? true : false);
}

int wmain(int argc, char *argv[]) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "C++ OpenGL", NULL, NULL);
  if (window == NULL) {
    cout << "Failed to create GLFW window!" << endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to initialize GLAD" << endl;
    return -1;
  }

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  string vertexCode;
  string fragmentCode;
  ifstream vShaderFile;
  ifstream fShaderFile;
  vShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
  fShaderFile.exceptions (ifstream::failbit | ifstream::badbit);
  try {
    vShaderFile.open("code/shaders/vertexShader.vs");
    fShaderFile.open("code/shaders/fragmentShader.fs");
    stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vShaderFile.close();
    fShaderFile.close();
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  }
  catch (ifstream::failure &e) {
    cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << endl;
  }

  const char* vertexShaderSource = vertexCode.c_str();
  const char* fragmentShaderSource = fragmentCode.c_str();

  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const GLint vertexShaderCount = vertexCode.length();
  glShaderSource(vertexShader, 1, &vertexShaderSource, &vertexShaderCount);
  glCompileShader(vertexShader);
  checkShaderCompileErrors(vertexShader, "VERTEX");

  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const GLint fragmentShaderCount = fragmentCode.length();
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, &fragmentShaderCount);
  glCompileShader(fragmentShader);
  checkShaderCompileErrors(fragmentShader, "FRAGMENT");

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  checkShaderCompileErrors(shaderProgram, "PROGRAM");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // cube
  Cube cube(glm::vec3(0.0f, 0.0f, 0.0f));

  // tetrahedron
  Tetrahedron tetrahedron(glm::vec3(-2.0f, 0.0f, 0.0f));

  // Simplex
  unsigned int simplex_vao, simplex_vbo, simplex_ebo;
  glGenVertexArrays(1, &simplex_vao);
  glGenBuffers(1, &simplex_vbo);
  glGenBuffers(1, &simplex_ebo);

  glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

  float pitch = 0.0f;
  float yaw = -90.0f;
  //float roll = 0.0f;
  float fov = 45.0f;

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;


  double mouseXPos, mouseYPos, lastMouseXPos, lastMouseYPos;
  double mouseSensitivity = 0.5f;
  glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
  glfwGetCursorPos(window, &lastMouseXPos, &lastMouseYPos);

  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check for exit program key
    if (KEY_PRESSED(GLFW_KEY_BACKSPACE)
        || KEY_PRESSED(GLFW_KEY_ESCAPE)) {
      // backspace because I have the ergodox ez keyboard (my backspace is where CAPSLOCK key would usually be)
      glfwSetWindowShouldClose(window, true);
    }

    // Move tetrahedron
    float tetrahedronSpeed = 0.01f;
    glm::vec3 adjustedInputPos = glm::vec3(0, 0, 0);
    bool tetrahedron_key_pressed = false;
    if (KEY_PRESSED(GLFW_KEY_UP)) {
      adjustedInputPos.y = tetrahedronSpeed;
      tetrahedron_key_pressed = true;
    }
    if (KEY_PRESSED(GLFW_KEY_DOWN)) {
      adjustedInputPos.y = -tetrahedronSpeed;
      tetrahedron_key_pressed = true;
    }
    if (KEY_PRESSED(GLFW_KEY_LEFT)) {
      adjustedInputPos.x = -tetrahedronSpeed;
      tetrahedron_key_pressed = true;
    }
    if (KEY_PRESSED(GLFW_KEY_RIGHT)) {
      adjustedInputPos.x = tetrahedronSpeed;
      tetrahedron_key_pressed = true;
    }
    if (tetrahedron_key_pressed) {
      tetrahedron.update_pos(adjustedInputPos);
    }

    // Update camera pos based on WASD keys
    float cameraSpeed = 2.5f * deltaTime;
    if (KEY_PRESSED(GLFW_KEY_W)) {
      cameraPos += cameraSpeed * cameraFront;
    }
    if (KEY_PRESSED(GLFW_KEY_S)) {
      cameraPos -= cameraSpeed * cameraFront;
    }
    if (KEY_PRESSED(GLFW_KEY_A)) {
      cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (KEY_PRESSED(GLFW_KEY_D)) {
      cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    // Update camera front based on mouse input
    float xOffset = (mouseXPos - lastMouseXPos) * mouseSensitivity;
    float yOffset = (mouseYPos - lastMouseYPos) * mouseSensitivity;
    lastMouseXPos = mouseXPos;
    lastMouseYPos = mouseYPos;

    yaw += xOffset;
    pitch -= yOffset;

    if (pitch > 89.0f) {
      pitch = 89.0f;
    }
    if (pitch < -89.0f) {
      pitch = -89.0f;
    }

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    glEnable(GL_DEPTH_TEST);
    glClearColor(to_rgb(71), to_rgb(78), to_rgb(104), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vector<glm::vec3> simplex;
    bool collision = gjk(tetrahedron, cube, simplex);
    if (collision) {
      cout << "There is a collision!" << endl;
    }
    else {
      cout << "NO COLLISION" << endl;
    }

    glUseProgram(shaderProgram);

    // Upload uniforms to vertex shader
    glm::mat4 viewMatrix = glm::lookAt(cameraPos,
                                       cameraPos + cameraFront,
                                       cameraUp);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, MAT_VALUE_LOC(viewMatrix));

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov),
                                                  (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
                                                  0.1f,
                                                  100.0f);
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, MAT_VALUE_LOC(projectionMatrix));

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLint colorLoc = glGetUniformLocation(shaderProgram, "shapeColor");
    glm::mat4 modelMatrix;

    // Draw cube
    glBindVertexArray(cube.vao);
    glUniform3f(colorLoc, 0.0, 0.0, 1.0);
    modelMatrix = glm::translate(IDENTITY_MATRIX, cube.pos);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, MAT_VALUE_LOC(modelMatrix));
    glDrawArrays(GL_TRIANGLES, 0, cube.VERTICES_NUM_VEC3);

    // Draw tetrahedron
    glBindVertexArray(tetrahedron.vao);
    glUniform3f(colorLoc, 1.0, 0.0, 0.0);
    modelMatrix = glm::translate(IDENTITY_MATRIX, tetrahedron.pos);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, MAT_VALUE_LOC(modelMatrix));
    glDrawArrays(GL_TRIANGLES, 0, tetrahedron.VERTICES_NUM_VEC3);

    if (collision) {
      // Draw simplex
      cout << "------------------------- Draw simplex" << endl;
      printVec3Vector(simplex);
      vector<float> simplexVector;
      for (int i = 0; i < simplex.size(); i++) {
        simplexVector.push_back(simplex[i].x);
        simplexVector.push_back(simplex[i].y);
        simplexVector.push_back(simplex[i].z);
      }
      glBindVertexArray(simplex_vao);
      glBindBuffer(GL_ARRAY_BUFFER, simplex_vbo);
      glBufferData(GL_ARRAY_BUFFER, simplexVector.size() * sizeof(float), &simplexVector[0], GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simplex_ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, SIMPLEX_INDICES_NUM * sizeof(unsigned int), &simplex_indices[0], GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

      glUniform3f(colorLoc, 0.0, 1.0, 0.0);
      modelMatrix = glm::translate(IDENTITY_MATRIX, glm::vec3(0.0f, 0.0f, 0.0f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, MAT_VALUE_LOC(modelMatrix));
      glDrawElements(GL_TRIANGLES, simplexVector.size(), GL_UNSIGNED_INT, NULL);
    }

    glBindVertexArray(cube.vao);
    glUniform3f(colorLoc, 0.0, 0.0, 0.0);
    float scale_factor = 0.05f;
    glm::mat4 m_scaled = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, MAT_VALUE_LOC(m_scaled));
    glDrawArrays(GL_TRIANGLES, 0, cube.VERTICES_NUM_VEC3);

    if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
      glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

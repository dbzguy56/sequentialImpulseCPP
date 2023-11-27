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

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

float to_rgb(int num) {
  return (float) num / 255.0f;
}

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

struct Simplex {
  glm::vec3 points[4];
  int size = 0;
};

enum EvolutionStage {
  NO_INTERSECTION,
  FOUND_INTERSECTION,
  STILL_EVOLVING
};

glm::vec3 averagePoint(glm::vec3 points[], int size) {
  glm::vec3 avg = {0.0f, 0.0f, 0.0f};
  for (int i = 0; i < size; i++){
    avg += points[i];
  }
  avg /= size;

  return avg;
}
/*
EvolutionStage evolveSimplex(Simplex &simplex, float verticesA[], float verticesB[],
                              glm::vec3 &direction]) {
  switch(simplex.size) {
    case 0:
      direction = averagePoint
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    default:
      cout << "blah" << endl;
  }
  return STILL_EVOLVING;
}
*/

bool gjk(float verticesA[], float verticesB[], Simplex &simplex) {
  EvolutionStage evolveResult = STILL_EVOLVING;
  float direction[3] = {0, 0, 0};
  return true;
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
  double mouseSensitivity = 0.5;
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
    if (KEY_PRESSED(GLFW_KEY_UP)) {
      tetrahedron.pos.y += tetrahedronSpeed;
    }
    if (KEY_PRESSED(GLFW_KEY_DOWN)) {
      tetrahedron.pos.y -= tetrahedronSpeed;
    }
    if (KEY_PRESSED(GLFW_KEY_LEFT)) {
      tetrahedron.pos.x -= tetrahedronSpeed;
    }
    if (KEY_PRESSED(GLFW_KEY_RIGHT)) {
      tetrahedron.pos.x += tetrahedronSpeed;
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

    //const int numElementsCube = (sizeof(cubeVertices) / sizeof(float));
    //glm::vec3 cubeWorldVertices[CUBE_VERTICES_SIZE];
    //toWorldPos(cubePos, cubeVertices, CUBE_VERTICES_SIZE, cubeWorldVertices);

    //glm::vec3 tetrahedronWorldVertices[TETRAHEDRON_VERTICES_SIZE];
    //toWorldPos(tetrahedron.pos, tetrahedronVertices, TETRAHEDRON_VERTICES_SIZE, tetrahedronWorldVertices);

    //Simplex simplex;
    //gjk(tetrahedronWorldVertices, cubeWorldVertices, simplex);

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

    if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
      glfwGetCursorPos(window, &mouseXPos, &mouseYPos);
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

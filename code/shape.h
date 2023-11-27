#ifndef SHAPE_H_
#define SHAPE_H_
#include <iostream>

class Shape {
public:
  unsigned int vao;
  unsigned int vbo;
  glm::vec3 pos;
  glm::vec3* model_vertices;
  glm::vec3* world_vertices;
  const int* size;

protected:
  virtual void gen_and_bind_vao_vbo() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
  }

  void to_world_vertices(int size) {
    for (int i = 0; i < size; i++) {
      world_vertices[i] = model_vertices[i] + pos;
    }
  }

  void fill_vec3_arr(float model_vertices_floats[], int size) {
    for (int i = 0; i < size; i += 3) {
      model_vertices[i/3] = glm::vec3(model_vertices_floats[i], model_vertices_floats[i+1], model_vertices_floats[i+2]);
    }
  }
};

#endif

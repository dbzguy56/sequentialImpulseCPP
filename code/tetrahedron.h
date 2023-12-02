#ifndef TETRAHEDRON_H_
#define TETRAHEDRON_H_

#include <iostream>
#include "shape.h"

class Tetrahedron : public Shape {
public:
  // TODO: Better naming?
  static const int VERTICES_NUM_VEC3 = 12;
  static const int VERTICES_NUM_FLOAT = VERTICES_NUM_VEC3 * 3;

  Tetrahedron(glm::vec3 tetrahedronPos) {
    pos = tetrahedronPos;
    size = &VERTICES_NUM_VEC3;
    model_vertices = new glm::vec3[VERTICES_NUM_VEC3];
    world_vertices = new glm::vec3[VERTICES_NUM_VEC3];
    fill_vec3_arr(model_vertices_floats, VERTICES_NUM_FLOAT);
    gen_and_bind_vao_vbo();
    to_world_vertices(VERTICES_NUM_VEC3);
  }
  void gen_and_bind_vao_vbo() {
    Shape::gen_and_bind_vao_vbo();
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_vertices_floats), model_vertices_floats, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
  }


private:
  float model_vertices_floats[VERTICES_NUM_FLOAT] = {
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.0f,
     0.5f,  0.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f,  0.0f,
     0.5f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.0f,
    -0.5f,  0.5f, -0.5f
  };
};
#endif

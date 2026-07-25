#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 theColor;

uniform vec2 resolution;

void main() {

  float aspect = resolution.x / resolution.y;

  vec3 p = aPos;

  if (aspect > 1.0) {
    p.x = p.x / aspect;
  } else {
    p.y = p.y * aspect;
  }


  gl_Position = vec4(p, 1.0);
  theColor = aColor;
}

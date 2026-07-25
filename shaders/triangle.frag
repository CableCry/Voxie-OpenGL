#version 460 core
out vec4 FragColor;

in vec3 theColor;

void main() {
  FragColor = vec4(theColor, 1.0);
}

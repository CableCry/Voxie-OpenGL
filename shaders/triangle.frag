#version 460 core
out vec4 FragColor;

in vec3 theColor;
in vec2 theUV;

uniform sampler2D tex;

void main() {
  FragColor = texture(tex, theUV) * vec4(theColor, 1.0);
}

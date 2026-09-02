#version 450

layout (location = 0) in vec3 position;

void main()
{
  //...pode ter mais linhas de c�digo aqui!
  gl_Position = vec4(position.x, position.y, position.z, 1.0);
}
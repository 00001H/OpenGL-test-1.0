#version 420 core
#define proc(x) (x/2.0+0.5)
layout(location = 0) in vec3 vpos;
layout(location = 1) in vec2 vtc;
layout(location = 2) in vec3 vnormal;
out vec2 tc;
out vec3 normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main(){
    gl_Position = proj*view*model*vec4(vpos,1.0);
    normal = vnormal;
    tc = vtc;
}

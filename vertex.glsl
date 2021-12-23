#version 420 core
layout(location = 0) in vec3 vpos;
layout(location = 1) in vec2 vtc;
layout(location = 2) in vec3 vnormal;

layout(location = 0) out vec4 modelpos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 tc;
uniform mat4 model;
uniform mat3 normalmat;
uniform mat4 view;
uniform mat4 proj;
void main(){
    modelpos = model*vec4(vpos,1.0);
    gl_Position = proj*view*model*vec4(vpos,1.0);
    normal = normalmat*vnormal;
    tc = vtc;
}

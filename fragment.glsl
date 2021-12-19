#version 420 core
struct Light{
    vec3 direction;
    vec3 color;
    int diffusestr;
    int specularstr;
};
out vec4 FragColor;
in vec4 color;
in vec3 normal;
in vec2 tc;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform Light light;
uniform vec3 ambient;
vec4 blend(vec4 a,vec4 b){
    return vec4((a.rgb*a.a+b.rgb*b.a)/(a.a+b.a),(1-(1-a.a)*(1-b.a)));
}
void main(){
    vec4 s1 = texture(tex1,tc);
    vec4 s2 = texture(tex2,tc);
    vec3 lighting = vec3(0.0);
    lighting += ambient;
    FragColor = vec4(lighting,1.0)*blend(s1,s2);
}

#version 420 core
#define double float//I like precision but can't implic cast from dvec to vec and
//probably we don't need that much of precision... and float color is already
//MUCH better than 1/256 8-bit colors.
struct Light{
    int kind;//0:point light
    vec3 direction;
    vec3 color;
    vec3 pos;
    double diffusestr;
    double specularstr;
};
struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    double shininess;
};
out vec4 FragColor;

layout(location = 0) in vec3 fpos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tc;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform Light light;
uniform Material material;
uniform vec3 ambient;
uniform vec3 campos;
uniform double ambstr;
vec4 blend(vec4 a,vec4 b){
    return vec4((a.rgb*a.a+b.rgb*b.a)/(a.a+b.a),(1-(1-a.a)*(1-b.a)));
}
vec3 calculateLighting(Light lgt,Material mater,in vec3 normx){
    vec3 norm = normalize(normx);
    if(lgt.kind==0){
        vec3 ldir = normalize(lgt.pos-fpos);
        double diffstr = max(dot(norm,ldir),0.0);
        vec3 diffuse = lgt.diffusestr*light.color*(diffstr*mater.diffuse);
        vec3 viewdir = normalize(campos-fpos);
        vec3 rfdir = reflect(-ldir,norm);
        double spec = pow(max(dot(viewdir,rfdir),0.0),mater.shininess);
        vec3 specular = lgt.specularstr*light.color*(spec*mater.specular);
        return diffuse+specular;
    }
}
void main(){
    vec4 s1 = texture(tex1,tc);
    vec4 s2 = texture(tex2,tc);
    vec3 lighting = vec3(0.0);
    lighting += ambient*ambstr;
    lighting += calculateLighting(light,material,normal);
    FragColor = vec4(lighting,1.0)*blend(s1,s2);

}

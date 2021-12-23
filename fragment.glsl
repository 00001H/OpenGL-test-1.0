#version 420 core
#define POINTLIGHT 0
#define DIRLIGHT 1
#define DPLIGHT 2
#define pointlight(x) ((x==POINTLIGHT)||(x==DPLIGHT))
#define double float//I like precision but can't implic cast from dvec to vec and
//probably we don't need that much of precision... and float color is already
//MUCH better than 1/256 8-bit colors.
struct Light{
    int kind;//0:point light, 1:directional light, 2:directional point light
    vec3 direction;
    vec3 color;
    vec3 pos;
    double diffusestr;
    double specularstr;

    double constant;
    double linear;
    double quadratic;

    double dpfull;
    double dpzero;
};
struct Material{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    bool hasemission;
    double shininess;
};
out vec4 FragColor;

layout(location = 0) in vec3 fpos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tc;
uniform Light light;
uniform Material material;
uniform vec3 ambient;
uniform vec3 campos;
uniform double ambstr;
vec4 blend(vec4 a,vec4 b){
    return vec4((a.rgb*a.a+b.rgb*b.a)/(a.a+b.a),(1-(1-a.a)*(1-b.a)));
}
vec3 calculateLighting(Light lgt,Material mater,vec2 texc, vec3 normx){
    vec3 norm = normalize(normx),ldir;
    if(pointlight(lgt.kind)){
        ldir = normalize(lgt.pos-fpos);
    }else if(lgt.kind==DIRLIGHT){
        ldir = normalize(-lgt.direction);
    }
    double attenu = 1.0;
    if(pointlight(lgt.kind)){
        double dist = length(lgt.pos-fpos);
        attenu = 1.0/(lgt.constant+(lgt.linear+lgt.quadratic*dist)*dist);
    }
    double diffstr = max(dot(norm,ldir),0.0);
    vec3 diffuse = lgt.diffusestr*light.color*(diffstr*texture(mater.diffuse,texc).rgb);
    vec3 viewdir = normalize(campos-fpos);
    vec3 rfdir = reflect(-ldir,norm);
    double spec = pow(max(dot(viewdir,rfdir),0.0),mater.shininess);
    vec3 specular = lgt.specularstr*light.color*(spec*texture(mater.specular,texc).rgb);
    vec3 emmis = vec3(0.0);
    if(mater.hasemission){
        emmis = texture(mater.emission,texc).rgb;
    }
    if(lgt.kind==DPLIGHT){
        double fragangle = dot(ldir,normalize(-lgt.direction));
        double thresh = lgt.dpfull-lgt.dpzero;
        attenu *= clamp((fragangle-lgt.dpzero)/thresh,0.0,1.0);
    }
    return (diffuse+specular)*attenu+emmis;
}
void main(){
    vec3 lighting = vec3(0.0);
    lighting += (ambient+texture(material.diffuse,tc).rgb)*ambstr;
    lighting += calculateLighting(light,material,tc,normal);
    FragColor = vec4(lighting,1.0);

}

#include<iostream>
#include<fstream>
#include<cmath>
#include<vector>
#include<any>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include"glutils.hpp"
#include"cam.hpp"
using namespace std;
using glm::transpose;
using glm::inverse;
using glm::lookAt;
using glm::fract;
using glm::cross;
using glm::normalize;
using glm::perspective;
using glm::value_ptr;
using glm::scale;
using glm::rotate;
#define rotate(x,y,z) rotate(x,(float)(y),z)
using glm::translate;
using glm::radians;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
#define cstr const char*
#define uint unsigned int
#define keyDown(win,ky) ((glfwGetKey(win,ky)==GLFW_PRESS)?true:false)
GLFWwindow* win;
bool first_cursor_capture = true;
double mouse_sensit = 0.2;
double lastmousex,lastmousey;
bool grab = false;
int SW,SH;
#define KEY         0xfc0000
#define MOUSE       0xfc0001
#define MOUSE_MOVE  0xfafc01
struct Event{
    int type;
    int action;
    any value;
};
vector<Event> events;
vector<Event> getEvents(){
    vector<Event> _events = events;
    events = vector<Event>();
    return _events;
}
struct MouseClick{
    double x;
    double y;
    int button;
};
void setMouseGrab(bool value){
    grab = value;
    glfwSetInputMode(win,GLFW_CURSOR,(value?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL));
    if(!grab){
        glfwSetCursorPos(win,SW/2,SH/2);
    }
}
void onkeydown(GLFWwindow* win,int ky,int scancode,int action,int mods){
    events.push_back({KEY,action,ky});
}
void onclick(GLFWwindow* win,int button,int action,int mods){
    double x,y;
    glfwGetCursorPos(win,&x,&y);
    events.push_back({MOUSE,action,MouseClick{x,y,button}});
}
void oncursormove(GLFWwindow* win,double x,double y){
    if(!grab){
        first_cursor_capture = true;
        return;
    }
    events.push_back({MOUSE,MOUSE_MOVE,make_pair(x,y)});
}
string replace(string st,string src,string dst){
    int start = 0;
    size_t loc;
    while((loc = st.find(src,start))!=string::npos){
        st = st.replace(loc,src.length(),dst);
        start = loc;
    }
    return st;
}
string ucat(cstr x,cstr y){
    char dest[strlen(x)+strlen(y)+10];
    strcpy(dest,x);
    strcat(dest,y);
    return dest;
}
#define within(tgt,bound1,bound2) ((bound1<tgt&&tgt<bound2)||(bound2<tgt&&tgt<bound1))
struct Light{//copy-pasted from fragment shader
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
struct Cube{
    vec3 pos,scale;
};
int main(){
    int lcount = 2;
    initGLFW(3,3,GLFW_OPENGL_CORE_PROFILE);
    atexit(glfwTerminate);
    GLFWmonitor* primarymon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primarymon);
    SW = mode->width;
    SH = mode->height;
    int DSW=SW,DSH=SH;
    win = mkwin(SW,SH,"Window!");//,primarymon);
    glfwMakeContextCurrent(win);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        exit(-1);
    };
    glViewport(0,0,DSW,DSH);
    //Load VBO/VAO
    float vtx[] = {
        -0.5, -0.5, -0.5,  0.0, 0.0, 0.0, 0.0, -1.0,
         0.5, -0.5, -0.5,  1.0, 0.0, 0.0, 0.0, -1.0,
         0.5,  0.5, -0.5,  1.0, 1.0, 0.0, 0.0, -1.0,
         0.5,  0.5, -0.5,  1.0, 1.0, 0.0, 0.0, -1.0,
        -0.5,  0.5, -0.5,  0.0, 1.0, 0.0, 0.0, -1.0,
        -0.5, -0.5, -0.5,  0.0, 0.0, 0.0, 0.0, -1.0,

        -0.5, -0.5,  0.5,  0.0, 0.0, 0.0, 0.0,  1.0,
         0.5, -0.5,  0.5,  1.0, 0.0, 0.0, 0.0,  1.0,
         0.5,  0.5,  0.5,  1.0, 1.0, 0.0, 0.0,  1.0,
         0.5,  0.5,  0.5,  1.0, 1.0, 0.0, 0.0,  1.0,
        -0.5,  0.5,  0.5,  0.0, 1.0, 0.0, 0.0,  1.0,
        -0.5, -0.5,  0.5,  0.0, 0.0, 0.0, 0.0,  1.0,

        -0.5,  0.5,  0.5,  1.0, 0.0, -1.0, 0.0, 0.0,
        -0.5,  0.5, -0.5,  1.0, 1.0, -1.0, 0.0, 0.0,
        -0.5, -0.5, -0.5,  0.0, 1.0, -1.0, 0.0, 0.0,
        -0.5, -0.5, -0.5,  0.0, 1.0, -1.0, 0.0, 0.0,
        -0.5, -0.5,  0.5,  0.0, 0.0, -1.0, 0.0, 0.0,
        -0.5,  0.5,  0.5,  1.0, 0.0, -1.0, 0.0, 0.0,

         0.5,  0.5,  0.5,  1.0, 0.0, 1.0, 0.0,  0.0,
         0.5,  0.5, -0.5,  1.0, 1.0, 1.0, 0.0,  0.0,
         0.5, -0.5, -0.5,  0.0, 1.0, 1.0, 0.0,  0.0,
         0.5, -0.5, -0.5,  0.0, 1.0, 1.0, 0.0,  0.0,
         0.5, -0.5,  0.5,  0.0, 0.0, 1.0, 0.0,  0.0,
         0.5,  0.5,  0.5,  1.0, 0.0, 1.0, 0.0,  0.0,

        -0.5, -0.5, -0.5,  0.0, 1.0, 0.0, -1.0, 0.0,
         0.5, -0.5, -0.5,  1.0, 1.0, 0.0, -1.0, 0.0,
         0.5, -0.5,  0.5,  1.0, 0.0, 0.0, -1.0, 0.0,
         0.5, -0.5,  0.5,  1.0, 0.0, 0.0, -1.0, 0.0,
        -0.5, -0.5,  0.5,  0.0, 0.0, 0.0, -1.0, 0.0,
        -0.5, -0.5, -0.5,  0.0, 1.0, 0.0, -1.0, 0.0,

        -0.5,  0.5, -0.5,  0.0, 1.0, 0.0, 1.0, 0.0,
         0.5,  0.5, -0.5,  1.0, 1.0, 0.0, 1.0, 0.0,
         0.5,  0.5,  0.5,  1.0, 0.0, 0.0, 1.0, 0.0,
         0.5,  0.5,  0.5,  1.0, 0.0, 0.0, 1.0, 0.0,
        -0.5,  0.5,  0.5,  0.0, 0.0, 0.0, 1.0, 0.0,
        -0.5,  0.5, -0.5,  0.0, 1.0, 0.0, 1.0, 0.0
    };

    //glEnable() calls
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    //Blending
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    //Load shaders
    Shader shader = loadshaderfiles("vertex.glsl","fragment.glsl",false);
    Shader lssh = loadshaderfiles("lsrcvertex.glsl","lsrcfragment.glsl");
    //Load Textures
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    stbi_set_flip_vertically_on_load(true);
    uint texture = loadTexture2D("container2.png");
    uint texture2 = loadTexture2D("cont2spec.png");
    uint texture3 = loadTexture2D("matrix.jpg");
    bind2DTo(texture,0);
    bind2DTo(texture2,1);
    bind2DTo(texture3,2);
    shader.uniform1i("material.diffuse",0);
    shader.uniform1i("material.specular",1);
    shader.uniform1i("material.emission",2);
//    shader.uniform1i("material.hasemission",1);
    //Mode Settings
//    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    //Load VBO/VAO(/EBO)
    size_t sff = sizeof(float);
    uint vbo;
    uint vao;
    glGenBuffers(1,&vbo);
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vtx),vtx,GL_STATIC_DRAW);
    //Setup attribute pointers
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sff,(void*)0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sff,(void*)(3*sff));
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sff,(void*)(5*sff));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    //Camera settings
    Camera cam = Camera(vec3(0,0,0),0,-90);
    cam.pos = vec3(0.0,0.0,5.0);
    //Game settings
    double gravity = -0.003;
    double yvel = 0;
    float clearr = 0.2;
    mat3 nmat;
    mat4 proj,model,view;
    proj = perspective(radians(45.0),(double)DSW/(double)DSH,0.1,100.0);
    float camspd = 0.035;
    double lastFrame = glfwGetTime();
    int fps = 60;
    vec3 ambient = vec3(1.0,1.0,1.0);
    //Input settings
    glfwSetKeyCallback(win,onkeydown);
    setMouseGrab(true);
    glfwSetCursorPosCallback(win,oncursormove);
    glfwSetMouseButtonCallback(win,onclick);
    events.push_back({KEY,GLFW_PRESS,GLFW_KEY_ENTER});
    cstr lpre = "lights[";
    cstr lpost = "].";
    Light lgts[lcount];
    int frame=0;
    Cube cubes[20];
    for(int i=0;i<20;i++){
        cubes[i].pos = vec3(0.0,0.0,i);
        cubes[i].scale = vec3(1.0);
    }
    while(!glfwWindowShouldClose(win)){
        glfwPollEvents();
        for(Event event:getEvents()){
            if(event.type==KEY){
                if(event.action==GLFW_PRESS){
                    switch(any_cast<int>(event.value)){
                        case GLFW_KEY_F1:{
                            system("start https://learnopengl.com/");
                            break;
                        }
                        case GLFW_KEY_ESCAPE:{
                            setMouseGrab(false);
                            break;
                        }
                        case GLFW_KEY_ENTER:{
                            reloadshaderfiles(shader,false);
                            shader.fshad = replace(shader.fshad,"$$LIGHT_COUNT",to_string(lcount));
                            shader.recompile();
                            break;
                        }
                    }
                }
            }else if(event.type==MOUSE){
                if(event.action==MOUSE_MOVE){
                    auto value = any_cast<pair<double,double> >(event.value);
                    double x = value.first;
                    double y = value.second;
                    if(first_cursor_capture){
                        lastmousex = x;
                        lastmousey = y;
                        first_cursor_capture = false;
                    }
                    double movex=x-lastmousex,movey=y-lastmousey;
                    lastmousex = x;
                    lastmousey = y;
                    movex *= mouse_sensit;
                    movey *= mouse_sensit;
                    cam.yaw += movex;
                    cam.pitch -= movey;
                    cam.pitch = max(-90.0,min(cam.pitch,90.0));
                }else if(event.action==GLFW_PRESS){
                    MouseClick click = any_cast<MouseClick>(event.value);
                    if(click.button==GLFW_MOUSE_BUTTON_LEFT){
                        setMouseGrab(true);
                    }
                }
            }
        }
        if(keyDown(win,GLFW_KEY_W)){
            cam.pos += cam.xzfront()*camspd;
        }
        if(keyDown(win,GLFW_KEY_S)){
            cam.pos -= cam.xzfront()*camspd;
        }
        if(keyDown(win,GLFW_KEY_D)){
            cam.pos += cam.xzright()*camspd;
        }
        if(keyDown(win,GLFW_KEY_A)){
            cam.pos -= cam.xzright()*camspd;
        }
        if(keyDown(win,GLFW_KEY_SPACE)){
            if(true){//cam.pos[1]==0){
                yvel = 0.05;
            }
        }
        if(cam.pos[1]>0){
            yvel += gravity;
        }
        cam.pos[1] += yvel;
        bool floorcollide = false;
        double floorpos = 0;
        for(int i=0;i<20;i++){
            Cube *c = cubes+i;
            double x0,x1,y0,y1,z0,z1;
            x0 = c->pos.x;
            y0 = c->pos.y;
            z0 = c->pos.z;
            x0 -= c->scale.x/2;
//            y0 -= c->scale.y/2;
            z0 += c->scale.z/2;
            x1 = x0+c->scale.x;
            y1 = y0+c->scale.y;
            z1 = z0-c->scale.z;
            if(within(cam.pos.x,x0,x1)&&within(cam.pos.y,y0-0.0001,y1)&&within(cam.pos.z,z0,z1)){
                floorcollide = true;
                floorpos = max(floorpos,max(y0,y1));
            }
        }
        if((cam.pos[1]<0)||(floorcollide)){
            yvel = 0;
            cam.pos[1] = floorpos;
        }
        view = cam.viewmatrix();
        glBindVertexArray(vao);
        model = mat4(1.0);
        shader.use();
        shader.uniformMatrix4fv("proj",proj);
        shader.uniformMatrix4fv("view",view);
        shader.uniformMatrix4fv("model",model);
        shader.uniform3fv("ambient",ambient);
        shader.uniform1f("ambstr",0.06);
        lgts[0].color = vec3(1.0);
        lgts[0].pos = cam.pos;//vec3(0.0,1.0,0.0);
        lgts[0].constant = 1.0;
        lgts[0].linear = 0.09;
        lgts[0].quadratic = 0.032;
        lgts[0].dpfull = cos(radians(5.0));
        lgts[0].dpzero = cos(radians(7.5));
        lgts[0].direction = cam.direction();
        lgts[0].kind = 2;
        lgts[0].diffusestr = 1.0;
        lgts[0].specularstr = 0.75;
        lgts[0].color = vec3(1.0);
        lgts[1].pos = vec3(0.0,1.0,abs(fract(frame/100.0)*2.0-1.0)*20.0);
        lgts[1].constant = 1.0;
        lgts[1].linear = 0.09;
        lgts[1].quadratic = 0.032;
        lgts[1].direction = vec3(0.0,-1.0,0.0);
        lgts[1].kind = 0;
        lgts[1].diffusestr = 1.0;
        lgts[1].specularstr = 0.75;
        lgts[1].color = vec3(1.0);
        for(int i=0;i<lcount;i++){
            char prx[100];
            strcpy(prx,lpre);
            strcat(prx,to_string(i).c_str());
            strcat(prx,lpost);
            shader.uniform3fv(ucat(prx,"color").c_str(),lgts[i].color);
            shader.uniform3fv(ucat(prx,"pos").c_str(),lgts[i].pos);
            shader.uniform1f(ucat(prx,"constant").c_str(),lgts[i].constant);
            shader.uniform1f(ucat(prx,"linear").c_str(),lgts[i].linear);
            shader.uniform1f(ucat(prx,"quadratic").c_str(),lgts[i].quadratic);
            shader.uniform1f(ucat(prx,"dpfull").c_str(),lgts[i].dpfull);
            shader.uniform1f(ucat(prx,"dpzero").c_str(),lgts[i].dpzero);
            shader.uniform3fv(ucat(prx,"direction").c_str(),lgts[i].direction);
            shader.uniform1i(ucat(prx,"kind").c_str(),lgts[i].kind);
            shader.uniform1f(ucat(prx,"diffusestr").c_str(),lgts[i].diffusestr);
            shader.uniform1f(ucat(prx,"specularstr").c_str(),lgts[i].specularstr);
        }
        shader.uniform1f("material.shininess",32);
        shader.uniform3fv("material.specular",vec3(1.0));
        shader.uniform3fv("campos",cam.pos);
        glClearColor(clearr,0.3,0.3,1.0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        for(int i=0;i<20;i++){
//            mat4 modell = translate(model,vec3(4.0*sin(radians(i*60.0)),0,4.0*cos(radians(i*60.0))));
            mat4 modell = translate(scale(model,cubes[i].scale),cubes[i].pos);
            shader.uniformMatrix4fv("model",modell);
            nmat = mat3(transpose(inverse(modell)));
            shader.uniformMatrix3fv("normalmat",nmat);
            glDrawArrays(GL_TRIANGLES,0,36);
        }
        lssh.use();
        model = mat4(1.0);
        model = translate(model,lgts[1].pos);
        model = scale(model,vec3(0.2));
        lssh.uniformMatrix4fv("proj",proj);
        lssh.uniformMatrix4fv("view",view);
        lssh.uniformMatrix4fv("model",model);
        lssh.uniform3fv("lightcolor",ambient);
        glDrawArrays(GL_TRIANGLES,0,36);
        glfwSwapBuffers(win);
        while((glfwGetTime()-lastFrame)<1.0/fps);
        lastFrame = glfwGetTime();
        frame++;
    }
    glfwTerminate();
    return 0;
}

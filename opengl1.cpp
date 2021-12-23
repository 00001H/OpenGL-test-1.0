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
#include"glutils.h"
#include"cam.h"
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
int main(){
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
    Shader shader = Shader("vertex.glsl","fragment.glsl");
    Shader lssh = Shader("lsrcvertex.glsl","lsrcfragment.glsl");
    //Load Textures
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    stbi_set_flip_vertically_on_load(true);
    uint texture = loadTexture2D("container.jpg");
    uint texture2 = loadTexture2D("awesomeface.png");
    shader.uniform1i("tex1",0);
    shader.uniform1i("tex2",1);
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
    vec3 lightPos = vec3(1.2,1.0,2.0);
    vec3 ambient = vec3(1.0,1.0,1.0);
    //Input settings
    glfwSetKeyCallback(win,onkeydown);
    setMouseGrab(true);
    glfwSetCursorPosCallback(win,oncursormove);
    glfwSetMouseButtonCallback(win,onclick);
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
                            shader.reload();
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
            if(cam.pos[1]==0){
                yvel = 0.05;
            }
        }
        if(cam.pos[1]>0){
            yvel += gravity;
        }
        cam.pos[1] += yvel;
        if(cam.pos[1]<0){
            yvel = 0;
            cam.pos[1] = 0;
        }
        shader.uniform1i("tex1",0);
        shader.uniform1i("tex2",1);
        view = cam.viewmatrix();
        glBindVertexArray(vao);
        model = mat4(1.0);
        shader.use();
        shader.uniformMatrix4fv("proj",proj);
        shader.uniformMatrix4fv("view",view);
        shader.uniformMatrix4fv("model",model);
        shader.uniform3fv("ambient",ambient);
        shader.uniform1f("ambstr",0.12);
        shader.uniform3fv("light.color",ambient);
        shader.uniform1f("light.diffusestr",1.0);
        shader.uniform1f("light.specularstr",0.5);
        shader.uniform1f("material.shininess",32);
        shader.uniform3fv("material.ambient",vec3(1.0));
        shader.uniform3fv("material.diffuse",vec3(1.0));
        shader.uniform3fv("material.specular",vec3(1.0));
        shader.uniform3fv("campos",cam.pos);
        glClearColor(clearr,0.3,0.3,1.0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        bind2DTo(texture,0);
        bind2DTo(texture2,1);
        for(double i=0;i<20;i++){
            mat4 modell = translate(model,vec3(4.0*sin(radians(i*60.0)),0,4.0*cos(radians(i*60.0))));
            shader.uniformMatrix4fv("model",modell);
            nmat = mat3(transpose(inverse(modell)));
            shader.uniformMatrix3fv("normalmat",nmat);
            glDrawArrays(GL_TRIANGLES,0,36);
        }
        lssh.use();
        model = mat4(1.0);
        model = translate(model,lightPos);
        model = scale(model,vec3(0.2));
        lssh.uniformMatrix4fv("proj",proj);
        lssh.uniformMatrix4fv("view",view);
        lssh.uniformMatrix4fv("model",model);
        lssh.uniform3fv("lightcolor",ambient);
        glDrawArrays(GL_TRIANGLES,0,36);
        glfwSwapBuffers(win);
        while((glfwGetTime()-lastFrame)<1.0/fps);
        lastFrame = glfwGetTime();
    }
    glfwTerminate();
    return 0;
}

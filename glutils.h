#ifndef _glutils_
#define _glutils_
#include<iostream>
#include"utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
using namespace std;
//Textures
unsigned int loadTexture2D(string filename){
    int w,h,chnls;
    unsigned char *data = stbi_load(filename.c_str(),&w,&h,&chnls,0);
    if(!data){
        cerr << "Texture " << filename << " failed to load." << endl;
    }
    unsigned int tex;
    unsigned int datchmode;
    if(chnls==3){
        datchmode = GL_RGB;
    }else if(chnls==4){
        datchmode = GL_RGBA;
    }else{
        cerr << "Error: unsupported color channel count of " << chnls << " while loading " << filename << endl;
        return -1;
    }
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,datchmode,GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}
void inline activeTexture(int texunit){
    glActiveTexture(GL_TEXTURE0+texunit);
}
void inline bind2D(unsigned int tex){
    glBindTexture(GL_TEXTURE_2D,tex);
}
void inline bind2DTo(unsigned int tex,int texunit){
    activeTexture(texunit);
    bind2D(tex);
}
//GLFW
void initGLFW(int major,int minor,int profile){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE,profile);
}
GLFWwindow inline *mkwin(int width,int height,const char* title,GLFWmonitor* monitor=NULL,GLFWwindow* share=NULL){
    return glfwCreateWindow(width,height,title,monitor,share);
}
//Shaders
unsigned int loadvshad(const char* filename){
    string vssource;
    unsigned int vshad = glCreateShader(GL_VERTEX_SHADER);
    try{
        vssource = loadStringFile(filename);
    }catch(logic_error&){
        cerr << "Failed to load vertex shader file:" << filename << endl;
        return -1;
    }
    {//cstrcode gets automatically destroyed after exiting this block
        const char* cstrcode = vssource.c_str();
        glShaderSource(vshad,1,&cstrcode,NULL);
    }
    glCompileShader(vshad);
    int success;
    char errormsg[1024];
    glGetShaderiv(vshad,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(vshad,1024,NULL,errormsg);
        cerr << "Vertex shader compilation failed:\n" << errormsg << endl;
        cerr << "---shader source--" << endl;
        cerr << vssource << endl;
        return -1;
    }
    return vshad;
}
unsigned int loadfshad(const char* filename){
    string fssource;
    unsigned int fshad = glCreateShader(GL_FRAGMENT_SHADER);
    try{
        fssource = loadStringFile(filename);
    }catch(logic_error&){
        cerr << "Failed to load vertex shader file:" << filename << endl;
        return -1;
    }
    {//cstrcode gets automatically destroyed after exiting this block
        const char* cstrcode = fssource.c_str();
        glShaderSource(fshad,1,&cstrcode,NULL);
    }
    glCompileShader(fshad);
    int success;
    char errormsg[1024];
    glGetShaderiv(fshad,GL_COMPILE_STATUS,&success);
    if(!success){
        glGetShaderInfoLog(fshad,1024,NULL,errormsg);
        cerr << "Fragment shader compilation failed:\n" << errormsg << endl;
        cerr << "---shader source--" << endl;
        cerr << fssource << endl;
        return -1;
    }
    return fshad;
}
unsigned int mkProgram(unsigned int vshad,unsigned int fshad){
    unsigned int program = glCreateProgram();
    glAttachShader(program,vshad);
    glAttachShader(program,fshad);
    glLinkProgram(program);
    int success;
    char errormsg[1024];
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success){
        glGetProgramInfoLog(program,1024,NULL,errormsg);
        cerr << "Program linking failed:\n" << errormsg << endl;
        return -1;
    }
    return program;
}
//OOP implementation
class Shader{
    private:
        int prepareUniform(const char* name){
            use();
            return glGetUniformLocation(id,name);
        }
    public:
        unsigned int id;
        string vshadfile,fshadfile;
        Shader(){
            id = -1;
        }
        Shader (string vshadfile,string fshadfile){
            modify(vshadfile,fshadfile);
        }
        void modify(string newvshadfile,string newfshadfile,bool reload=true){
            vshadfile = newvshadfile;
            fshadfile = newfshadfile;
            if(reload)this->reload();
        }
        void reload(){
            unsigned int vshad = loadvshad(vshadfile.c_str());
            unsigned int fshad = loadfshad(fshadfile.c_str());
            id = mkProgram(vshad,fshad);
            glDeleteShader(vshad);
            glDeleteShader(fshad);
        }
        void use(){
            glUseProgram(id);
        }
        void uniform1i(const char* name,int x){
            int location = prepareUniform(name);
            glUniform1i(location,x);
        }
        void uniform1f(const char* name,float x){
            int location = prepareUniform(name);
            glUniform1f(location,x);
        }
        void uniform3f(const char* name,float x,float y,float z){
            int location = prepareUniform(name);
            glUniform3f(location,x,y,z);
        }
        void uniform3fv(const char* name,glm::vec3 vect){
            uniform3f(name,vect.x,vect.y,vect.z);
        }
        void uniform4f(const char* name,float x,float y,float z,float w){
            int location = prepareUniform(name);
            glUniform4f(location,x,y,z,w);
        }
        void uniformMatrix3fv(const char* name,glm::mat3 matrix){
            int location = prepareUniform(name);
            glUniformMatrix3fv(location,1,GL_FALSE,glm::value_ptr(matrix));
        }
        void uniformMatrix4fv(const char* name,glm::mat4 matrix){
            int location = prepareUniform(name);
            glUniformMatrix4fv(location,1,GL_FALSE,glm::value_ptr(matrix));
        }
};
#endif

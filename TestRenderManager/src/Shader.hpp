// Author: Xavier Corbillon
// IMT Atlantique
//
// Description:
// Shader class
#pragma once

//internal includes
#include "stb_image.h"

//standard includes
#include <iostream>
#include <vector>

// Library/third-party includes
#include <GL/glew.h>

// This must come after we include <GL/gl.h> so its pointer types are defined.
#include <osvr/RenderKit/GraphicsLibraryOpenGL.h>

namespace IMT
{
// normally you'd load the shaders from a file, but in this case, let's
// just keep things simple and load from memory.
static const GLchar* vertexShader =
    "#version 330 core\n"
    "layout(location = 0) in vec3 position;\n"
    //"layout(location = 1) in vec3 vertexColor;\n"
    "layout(location = 1) in vec2 vertexUV;\n"
    //"out vec3 fragmentColor;\n"
    "out vec2 UV;\n"
    "uniform mat4 modelView;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * modelView * vec4(position,1);\n"
    //"   fragmentColor = vertexColor;\n"
    "   UV = vertexUV;\n"
    "}\n";

static const GLchar* fragmentShader = "#version 330 core\n"
                                      //"in vec3 fragmentColor;\n"
                                      "// Interpolated values from the vertex shaders\n"
                                      "in vec2 UV;\n"
                                      "out vec3 color;\n"
                                      "uniform sampler2D myTextureSampler;\n"
                                      "void main()\n"
                                      "{\n"
                                      //"    color = fragmentColor;\n"
                                      "    color = texture( myTextureSampler, UV ).rgb;\n"
                                      "}\n";

class Shader {
  public:
    Shader(std::string pathToTexture): m_pathToTexture(pathToTexture) {}

    ~Shader() {
        if (initialized) {
            glDeleteProgram(programId);
        }
    }

    void init() {
        if (!initialized) {
            GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
            GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

            // vertex shader
            glShaderSource(vertexShaderId, 1, &vertexShader, NULL);
            glCompileShader(vertexShaderId);
            checkShaderError(vertexShaderId,
                             "Vertex shader compilation failed.");

            // fragment shader
            glShaderSource(fragmentShaderId, 1, &fragmentShader, NULL);
            glCompileShader(fragmentShaderId);
            checkShaderError(fragmentShaderId,
                             "Fragment shader compilation failed.");

            // linking program
            programId = glCreateProgram();
            glAttachShader(programId, vertexShaderId);
            glAttachShader(programId, fragmentShaderId);
            glLinkProgram(programId);
            checkProgramError(programId, "Shader program link failed.");

            // once linked into a program, we no longer need the shaders.
            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);

            projectionUniformId = glGetUniformLocation(programId, "projection");
            modelViewUniformId = glGetUniformLocation(programId, "modelView");
            myTextureUniformId = glGetUniformLocation(programId, "myTextureSampler");
            initialized = true;
        }
    }

    void useProgram(const GLdouble projection[], const GLdouble modelView[]) {
        init();
        glUseProgram(programId);
        GLfloat projectionf[16];
        GLfloat modelViewf[16];
        convertMatrix(projection, projectionf);
        convertMatrix(modelView, modelViewf);
        glUniformMatrix4fv(projectionUniformId, 1, GL_FALSE, projectionf);
        glUniformMatrix4fv(modelViewUniformId, 1, GL_FALSE, modelViewf);

        if(texture == 0)
        {
          std::cout << "Load texture " << std::endl;
          int w;
          int h;
          int comp;
          unsigned char* image = stbi_load(m_pathToTexture.c_str(), &w, &h, &comp, 0);
          if(image == nullptr)
          {
            throw(std::string("Failed to load texture"));
          }
          glGenTextures(1, &texture);
          glBindTexture(GL_TEXTURE_2D, texture);

          std::cout << "Loaded with " << comp << " comp" << std::endl;

          if(comp == 3)
          {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
          }
          else if(comp == 4)
          {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
          }

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        	glGenerateMipmap(GL_TEXTURE_2D);

          glBindTexture(GL_TEXTURE_2D, 0);
          stbi_image_free(image);
          std::cout << "Texture loaded" << std::endl;
        }
        glActiveTexture(GL_TEXTURE0);
    		glBindTexture(GL_TEXTURE_2D, texture);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glUniform1i(myTextureUniformId,0);
    }

  private:
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    bool initialized = false;
    GLuint programId = 0;
    GLuint projectionUniformId = 0;
    GLuint modelViewUniformId = 0;
    GLuint myTextureUniformId = 0;
    GLuint texture = 0;
    std::string m_pathToTexture;

    void checkShaderError(GLuint shaderId, const std::string& exceptionMsg) {
        GLint result = GL_FALSE;
        int infoLength = 0;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLength);
        if (result == GL_FALSE) {
            std::vector<GLchar> errorMessage(infoLength + 1);
            glGetProgramInfoLog(programId, infoLength, NULL, &errorMessage[0]);
            std::cerr << &errorMessage[0] << std::endl;
            throw std::runtime_error(exceptionMsg);
        }
    }

    void checkProgramError(GLuint programId, const std::string& exceptionMsg) {
        GLint result = GL_FALSE;
        int infoLength = 0;
        glGetProgramiv(programId, GL_LINK_STATUS, &result);
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLength);
        if (result == GL_FALSE) {
            std::vector<GLchar> errorMessage(infoLength + 1);
            glGetProgramInfoLog(programId, infoLength, NULL, &errorMessage[0]);
            std::cerr << &errorMessage[0] << std::endl;
            throw std::runtime_error(exceptionMsg);
        }
    }

    void convertMatrix(const GLdouble source[], GLfloat dest_out[]) {
        if (nullptr == source || nullptr == dest_out) {
            throw new std::logic_error("source and dest_out must be non-null.");
        }
        for (int i = 0; i < 16; i++) {
            dest_out[i] = (GLfloat)source[i];
        }
    }
};
}

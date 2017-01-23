// Author: Xavier Corbillon
// IMT Atlantique
//
// Description:
// Virtual class that implement a shader to read color for one texture (with UVs)
// And apply viewport rotations
#pragma once

//standard includes
#include <iostream>
#include <vector>
#include <chrono>

// Library/third-party includes
#include <GL/glew.h>
#include "DisplayFrameInfo.hpp"

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

class ShaderTexture {
  public:
    ShaderTexture(void): m_initialized(false), m_programId(0),
      m_projectionUniformId(0), m_modelViewUniformId(0), m_myTextureUniformId(0),
      m_textureId(0) {}

    virtual ~ShaderTexture()
    {
        if (m_initialized)
        {
            glDeleteProgram(m_programId);
        }
        if (m_textureId != 0)
        {
          glDeleteTextures(1, &m_textureId);
        }
    }

    void init()
    {
        if (!m_initialized)
        {
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
            m_programId = glCreateProgram();
            glAttachShader(m_programId, vertexShaderId);
            glAttachShader(m_programId, fragmentShaderId);
            glLinkProgram(m_programId);
            checkProgramError(m_programId, "Shader program link failed.");

            // once linked into a program, we no longer need the shaders.
            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);

            m_projectionUniformId = glGetUniformLocation(m_programId, "projection");
            m_modelViewUniformId = glGetUniformLocation(m_programId, "modelView");
            m_myTextureUniformId = glGetUniformLocation(m_programId, "myTextureSampler");
            m_initialized = true;
        }
    }

    virtual void InitAudio(void) {}
    virtual void SetStartTime(std::chrono::system_clock::time_point startTime) {}

    DisplayFrameInfo useProgram(const GLdouble projection[], const GLdouble modelView[], std::chrono::system_clock::time_point deadline)
    {
        init();
        glUseProgram(m_programId);
        GLfloat projectionf[16];
        GLfloat modelViewf[16];
        convertMatrix(projection, projectionf);
        convertMatrix(modelView, modelViewf);
        glUniformMatrix4fv(m_projectionUniformId, 1, GL_FALSE, projectionf);
        glUniformMatrix4fv(m_modelViewUniformId, 1, GL_FALSE, modelViewf);

        auto frameInfo = UpdateTexture(std::move(deadline));
        glActiveTexture(GL_TEXTURE0);
    		glBindTexture(GL_TEXTURE_2D, m_textureId);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glUniform1i(m_myTextureUniformId,0);
        return std::move(frameInfo);
    }

  protected:
    const auto& GetTextureId(void) const {return m_textureId;}
    auto& GetTextureId(void) {return m_textureId;}

  private:
    ShaderTexture(const ShaderTexture&) = delete;
    ShaderTexture& operator=(const ShaderTexture&) = delete;
    bool m_initialized;
    GLuint m_programId;
    GLuint m_projectionUniformId = 0;
    GLuint m_modelViewUniformId = 0;
    GLuint m_myTextureUniformId = 0;
    GLuint m_textureId = 0;

    //Update content of openGl m_textureId object and return the current displayed frame id
    virtual DisplayFrameInfo UpdateTexture(std::chrono::system_clock::time_point deadline) = 0;

    void checkShaderError(GLuint shaderId, const std::string& exceptionMsg)
    {
        GLint result = GL_FALSE;
        int infoLength = 0;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLength);
        if (result == GL_FALSE)
        {
            std::vector<GLchar> errorMessage(infoLength + 1);
            glGetProgramInfoLog(m_programId, infoLength, NULL, &errorMessage[0]);
            std::cerr << &errorMessage[0] << std::endl;
            throw std::runtime_error(exceptionMsg);
        }
    }

    void checkProgramError(GLuint m_programId, const std::string& exceptionMsg)
    {
        GLint result = GL_FALSE;
        int infoLength = 0;
        glGetProgramiv(m_programId, GL_LINK_STATUS, &result);
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &infoLength);
        if (result == GL_FALSE)
        {
            std::vector<GLchar> errorMessage(infoLength + 1);
            glGetProgramInfoLog(m_programId, infoLength, NULL, &errorMessage[0]);
            std::cerr << &errorMessage[0] << std::endl;
            throw std::runtime_error(exceptionMsg);
        }
    }

    void convertMatrix(const GLdouble source[], GLfloat dest_out[])
    {
        if (nullptr == source || nullptr == dest_out)
        {
            throw new std::logic_error("source and dest_out must be non-null.");
        }
        for (int i = 0; i < 16; i++)
        {
            dest_out[i] = (GLfloat)source[i];
        }
    }
};
}

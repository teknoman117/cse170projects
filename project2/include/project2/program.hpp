#ifndef __program_hpp__
#define __program_hpp__

#include <vector>
#include <map>

#include <project2/common.hpp>
#include <project2/shader.hpp>

class Program
{
    GLuint handle;
    bool   linked;
    
    std::vector<std::shared_ptr<Shader>> shaders;
    
    std::map<std::string, GLuint> uniforms;
    std::map<std::string, GLuint> attributes;
    
public:
    Program();
    virtual ~Program();
    
    bool IsValid() const;
    bool IsLinked() const;
    
    Program& Attach(const std::string& path, const GLenum type);
    Program& Attach(const std::shared_ptr<Shader>& shader);
    Program& Link();
    void     Bind();
    
    GLuint   GetUniform(const std::string& uniformName);
    GLuint   GetAttribute(const std::string& attributeName);
};

#endif

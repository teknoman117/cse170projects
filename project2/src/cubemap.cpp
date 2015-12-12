#include <project2/cubemap.hpp>
#include <project2/lodepng.hpp>

namespace
{
    // The string contants representing the suffixes of the cubemap textures
    const std::string cubemapTextureNameSuffixes[] =
    {
        "+x.png", "-x.png", "+y.png", "-y.png", "+z.png", "-z.png"
    };

    // The string contants representing the suffixes of the cubemap textures
    const GLenum cubemapTextureTypes[] =
    {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };

    // The count of important things to load
    const size_t cubemapTextureCount = 6;
}

Cubemap::Cubemap(const std::string& path)
{
    glGenTextures(1, &handle);

    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    
    // Loop through and load all the cubemap textures
    unsigned width, height;
    for(size_t i = 0; i < cubemapTextureCount; i++)
    {
        // Load the texture from disk
        std::vector<unsigned char> textureData;
        textureData.clear();
        std::string textureName = path + cubemapTextureNameSuffixes[i];
        unsigned error = lodepng::decode(textureData, width, height, textureName);
        if(error)
        {
            // Error and cleanup
            std::cout << "[FATAL] [CUBEMAP \"" << path << "] Texture: " << textureName << " Failed to load" << std::endl;
            glDeleteTextures(1, &handle);
            throw std::runtime_error("resource not found");
        }
        
        // Upload the texture to the GPU
        glTexImage2D(cubemapTextureTypes[i], 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) textureData.data());
    }

    std::cout << "[INFO] [CUBEMAP \"" << path << "\" ==> " << handle << "] Loaded 6 (" << width << "," << height << ") faces" << std::endl;;
}

Cubemap::~Cubemap()
{
    glDeleteTextures(1, &handle);
    std::cout << "[INFO] [CUBEMAP " << handle << "] Destroyed" << std::endl;;
}

void Cubemap::Bind(GLenum unit)
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
}

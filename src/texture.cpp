#include "texture.h"

#include <string>

using namespace std;

namespace Splash {

/*************/
Texture::Texture()
{
    _type = "texture";
}

/*************/
Texture::~Texture()
{
}

/*************/
Texture& Texture::operator=(const ImageBuf& pImg)
{
    if (!pImg.initialized())
        return *this;
    if (pImg.localpixels() == NULL)
        return *this; // Pixels are not loaded in RAM, not yet supported

    ImageSpec spec = pImg.spec();

    if (spec.width != _spec.width || spec.height != _spec.width
        || spec.nchannels != _spec.nchannels || spec.format != _spec.format
        || !glIsTexture(_glTex))
    {
        glDeleteTextures(1, &_glTex);
        glGenTextures(1, &_glTex);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _glTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (spec.nchannels == 3 && spec.format == TypeDesc::UINT8)
        {
            SLog::log << Log::DEBUG << "Texture::" <<  __FUNCTION__ << " - Creating a new texture of type GL_UNSIGNED_BYTE, format GL_BGR" << Log::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, spec.width, spec.height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImg.localpixels());
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        _spec = spec;
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _glTex);
        if (spec.nchannels == 3 && spec.format == TypeDesc::UINT8)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, spec.width, spec.height, GL_RGB, GL_UNSIGNED_BYTE, pImg.localpixels());
        }

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return *this;
}

/*************/
void Texture::generateMipmap() const
{
    glBindTexture(GL_TEXTURE_2D, _glTex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*************/
ImageBuf Texture::getBuffer() const
{
}

/*************/
void Texture::reset(GLenum target, GLint pLevel, GLint internalFormat, GLsizei width, GLsizei height,
                    GLint border, GLenum format, GLenum type, const GLvoid* data)
{
    if (width == 0 || height == 0)
    {
        SLog::log << Log::WARNING << "Texture::" << __FUNCTION__ << " - Texture size is null" << Log::endl;
        return;
    }

    if (_glTex == 0)
    {
        glGenTextures(1, &_glTex);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _glTex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(target, pLevel, internalFormat, width, height, border, format, type, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _glTex);
        glTexImage2D(target, pLevel, internalFormat, width, height, border, format, type, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    _spec.width = width;
    _spec.height = height;
    if (format == GL_RGB && type == GL_UNSIGNED_BYTE)
    {
        _spec.nchannels = 3;
        _spec.format = TypeDesc::UINT8;
        _spec.channelnames = {"R", "G", "B"};
    }
    else if (format == GL_RGBA && type == GL_UNSIGNED_BYTE)
    {
        _spec.nchannels = 4;
        _spec.format == TypeDesc::UINT8;
        _spec.channelnames = {"R", "G", "B", "A"};
    }
}

} // end of namespace
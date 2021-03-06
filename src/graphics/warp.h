/*
 * Copyright (C) 2016 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @warp.h
 * The Warp class, designed to allow for projection warping
 */

#ifndef SPLASH_WARP_H
#define SPLASH_WARP_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "./core/constants.h"

#include "./core/attribute.h"
#include "./graphics/camera.h"
#include "./graphics/framebuffer.h"
#include "./graphics/object.h"
#include "./graphics/texture.h"
#include "./graphics/texture_image.h"
#include "./mesh/mesh_bezierpatch.h"

namespace Splash
{

/*************/
class Warp : public Texture
{
  public:
    /**
     * \brief Constructor
     * \param root Root object
     */
    Warp(RootObject* root);

    /**
     * \brief Destructor
     */
    ~Warp() final;

    /**
     * No copy constructor, but a move one
     */
    Warp(const Warp&) = delete;
    Warp(Warp&&) = default;
    Warp& operator=(const Warp&) = delete;

    /**
     * \brier Bind this warp
     */
    void bind() final;

    /**
     * \brier Unbind this warp
     */
    void unbind() final;

    /**
     * \brief Get the shader parameters related to this warp. Texture should be locked first.
     * \return Return the shader uniforms
     */
    std::unordered_map<std::string, Values> getShaderUniforms() const;

    /**
     * \brief Get the texture the warp is rendered to
     * \return Return the rendered texture
     */
    std::shared_ptr<Texture_Image> getTexture() const { return _fbo->getColorTexture(); }

    /**
     * Get the output texture GL id
     * \return Return the id
     */
    GLuint getTexId() const { return _fbo->getColorTexture()->getTexId(); }

    /**
     * Get the timestamp
     * \return Return the timestamp in us
     */
    virtual int64_t getTimestamp() const final { return _spec.timestamp; }

    /**
     * \brief Get the coordinates of the closest vertex to the given point
     * \param p Point around which to look
     * \param v Closest vertex coordinates
     * \return Return the index of the point
     */
    int pickControlPoint(glm::vec2 p, glm::vec2& v);

    /**
     * \brief Warps should always be saved as it hold user-modifiable parameters. This method has no effect.
     */
    void setSavable(bool /*savable*/) { _savable = true; }

    /**
     * \brief Update the warp
     */
    void render() final;

    /**
     * \brief Update for a warp does nothing, it is the render() job
     */
    void update() final {}

  protected:
    /**
     * \brief Try to link the given GraphObject to this object
     * \param obj Shared pointer to the (wannabe) child object
     */
    bool linkIt(const std::shared_ptr<GraphObject>& obj) final;

    /**
     * \brief Try to unlink the given GraphObject from this object
     * \param obj Shared pointer to the (supposed) child object
     */
    void unlinkIt(const std::shared_ptr<GraphObject>& obj) final;

  private:
    std::weak_ptr<Camera> _inCamera;

    std::unique_ptr<Framebuffer> _fbo{nullptr};
    std::shared_ptr<Mesh_BezierPatch> _screenMesh{nullptr};
    std::shared_ptr<Object> _screen{nullptr};

    // Render options
    bool _showControlPoints{false};
    int _selectedControlPointIndex{-1};

    // Mipmap capture
    int _grabMipmapLevel{-1};
    Value _mipmapBuffer{};
    Values _mipmapBufferSpec{};

    /**
     * \brief Init function called in constructors
     */
    void init();

    /**
     * \brief Load some defaults models
     */
    void loadDefaultModels();

    /**
     * \brief Setup the output texture
     */
    void setupFBO();

    /**
     * \brief Register new functors to modify attributes
     */
    void registerAttributes();
};

} // end of namespace

#endif // SPLASH_WARP_H

/*
 * Copyright (C) 2013 Emmanuel Durand
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
 * @scene.h
 * The Scene class
 */

#ifndef SPLASH_SCENE_H
#define SPLASH_SCENE_H

#include <atomic>
#include <cstddef>
#include <future>
#include <list>
#include <mutex>
#include <vector>

#include "./config.h"

#include "./core/attribute.h"
#include "./core/coretypes.h"
#include "./core/factory.h"
#include "./core/root_object.h"
#include "./graphics/gl_window.h"
#include "./graphics/object_library.h"

namespace Splash
{

class ControllerObject;
class Gui;
class Scene;

/*************/
//! Scene class, which does the rendering on a given GPU
class Scene : public RootObject
{
    friend ControllerObject;
    friend UserInput;

  public:
    /**
     * \brief Constructor
     * \param name Scene name
     * \param autoRun If true, the Scene will start without waiting for a start message from the World
     */
    Scene(const std::string& name = "Splash", const std::string& socketPrefix = "");

    /**
     * \brief Destructor
     */
    ~Scene() override;

    /**
     * \brief Add an object of the given type, with the given name
     * \param type Object type
     * \param name Object name
     * \return Return a shared pointer to the created object
     */
    std::shared_ptr<GraphObject> addObject(const std::string& type, const std::string& name = "");

    /**
     * \brief Add an object ghosting one in another Scene. Used in master Scene for controlling purposes
     * \param type Object type
     * \param name Object name
     */
    void addGhost(const std::string& type, const std::string& name = "");

    /**
     * \brief Get a glfw window sharing the same context as _mainWindow
     * \param name Window name
     * \return Return a shared pointer to the new window
     */
    std::shared_ptr<GlWindow> getNewSharedWindow(const std::string& name = "");

    /**
     * Get the found OpenGL version
     * \return Return the version as a vector of {MAJOR, MINOR}
     */
    static std::vector<int> getGLVersion();

    /**
     * Get whether NV swap groups are available
     * \return Return true if they are
     */
    static bool getHasNVSwapGroup();

    /**
     * Get a reference to the object library
     * \return Return a reference to the object library
     */
    ObjectLibrary& getObjectLibrary() { return _objectLibrary; }

    /**
     * \brief Get the status of the scene
     * \return Return true if all is well
     */
    bool getStatus() const { return _status; }

    /**
     * Get the swap interval for this whole scene
     * \return Return the swap interval
     */
    int getSwapInterval() const { return _swapInterval; }

    /**
     * \brief Check whether it is initialized
     * \return Return true if the Scene is initialized
     */
    bool isInitialized() const { return _isInitialized; }

    /**
     * \brief Ask whether the scene is Master or not
     * \return Return true if the Scene is master
     */
    bool isMaster() const { return _isMaster; }

    /**
     * \brief Check wether the scene is running
     * \return Return true if the scene runs
     */
    bool isRunning() const { return _isRunning; }

    /**
     * \brief Link an object to another, base on their types
     * \param first Child object
     * \param second Parent object
     * \return Return true if the linking succeeded
     */
    bool link(const std::string& first, const std::string& second);
    bool link(const std::shared_ptr<GraphObject>& first, const std::shared_ptr<GraphObject>& second);

    /**
     * \brief Unlink two objects. This always succeeds
     * \param first Child object
     * \param second Parent object
     */
    void unlink(const std::string& first, const std::string& second);
    void unlink(const std::shared_ptr<GraphObject>& first, const std::shared_ptr<GraphObject>& second);

    /**
     * \brief Remove an object
     * \param name Object name
     */
    void remove(const std::string& name);

    /**
     * \brief Render everything
     */
    void render();

    /**
     * \brief Main loop for the scene
     */
    void run();

    /**
     * \brief Set the Scene as the master one
     * \param configFilePath File path for the loaded configuration
     */
    void setAsMaster(const std::string& configFilePath = "");

    /**
     * \brief Set a message to be sent to the world
     * \param message Message type to send, which should correspond to a World attribute
     * \param value Message content
     */
    void sendMessageToWorld(const std::string& message, const Values& value = {});

    /**
     * \brief Set a message to be sent to the world, and wait for the World to send an answer
     * \param message Message type to send, which should correspond to a World attribute
     * \param value Message content
     * \param timeout Timeout in microseconds
     * \return Return the answer from the World
     */
    Values sendMessageToWorldWithAnswer(const std::string& message, const Values& value = {}, const unsigned long long timeout = 0);

  protected:
    std::shared_ptr<GlWindow> _mainWindow;
    std::atomic_bool _isRunning{false};

    // Gui exists in master scene whatever the configuration
    std::shared_ptr<Gui> _gui;
    bool _guiLinkedToWindow{false};

    // Default input objects
    std::shared_ptr<GraphObject> _keyboard{nullptr};
    std::shared_ptr<GraphObject> _mouse{nullptr};
    std::shared_ptr<GraphObject> _joystick{nullptr};
    std::shared_ptr<GraphObject> _dragndrop{nullptr};
    std::shared_ptr<GraphObject> _blender{nullptr};

// Objects in charge of calibration
#if HAVE_GPHOTO and HAVE_OPENCV
    std::shared_ptr<GraphObject> _colorCalibrator{nullptr};
#endif
#if HAVE_SLAPS
    std::shared_ptr<GraphObject> _geometricCalibrator{nullptr};
#endif

  private:
    ObjectLibrary _objectLibrary; //!< Library of 3D objects used by multiple GraphObjects

    static bool _hasNVSwapGroup; //!< If true, NV swap groups have been detected and are used
    static std::vector<int> _glVersion;
    static std::string _glVendor;
    static std::string _glRenderer;

    bool _runInBackground{false}; //!< If true, no window will be created
    bool _threadedTextureUpload{false}; //!< If true, texture upload is done in a separate thread
    std::atomic_bool _started{false};

    bool _isMaster{false}; //!< Set to true if this is the master Scene of the current config
    bool _isInitialized{false};
    bool _status{false};                        //!< Set to true if an error occured during rendering
    int _swapInterval{1};                       //!< Global value for the swap interval, default for all windows
    unsigned long long _targetFrameDuration{0}; //!< Duration in microseconds of a frame at the refresh rate of the
                                                //!< primary monitor

    // Texture upload context
    std::future<void> _textureUploadFuture;
    std::shared_ptr<GlWindow> _textureUploadWindow;
    std::atomic_bool _textureUploadDone{false};
    std::mutex _textureMutex; //!< Sync between texture and render loops
    GLsync _textureUploadFence{nullptr}, _cameraDrawnFence{nullptr};

    std::atomic_bool _doUploadTextures{false};
    std::condition_variable _doUploadTexturesCondition{};
    std::mutex _doUploadTexturesMutex{};

    // NV Swap group specific
    GLuint _maxSwapGroups{0};
    GLuint _maxSwapBarriers{0};

    static std::vector<std::string> _ghostableTypes;

    /**
     * \brief Find which OpenGL version is available (from a predefined list)
     * \return Return MAJOR and MINOR
     */
    std::vector<int> findGLVersion();

    /**
     * \brief Set up the context and everything
     * \param name Scene name
     */
    void init(const std::string& name);

    /**
     * \brief Computes and store the duration of a frame at the refresh rate of the primary monitor
     * \return The duration of a frame at the refresh rate of the primary monitor in microseconds
     */
    unsigned long long updateTargetFrameDuration();

    /**
     * \brief Callback for GLFW errors
     * \param code Error code
     * \param msg Associated error message
     */
    static void glfwErrorCallback(int code, const char* msg);

    /**
     * \brief Callback for GL errors and warnings
     */
    static void glMsgCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, void*);

    /**
     * \brief Texture update loop
     */
    void textureUploadRun();

    /**
     * \brief Register new attributes
     */
    void registerAttributes();

    /**
     * Initialize the tree
     */
    void initializeTree();

    /**
     * \brief Update the various inputs (mouse, keyboard...)
     */
    void updateInputs();
};

} // namespace Splash

#endif // SPLASH_SCENE_H

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
 * @image.h
 * The Image class
 */

#ifndef SPLASH_IMAGE_H
#define SPLASH_IMAGE_H

#include <chrono>
#include <mutex>

#include "config.h"

#include "coretypes.h"
#include "basetypes.h"
#include "imageBuffer.h"

namespace Splash {

class Image : public BufferObject
{
    public:
        /**
         * Constructor
         */
        Image();
        Image(bool linked); //< This constructor is used if the object is linked to a World counterpart
        Image(ImageBufferSpec spec);

        /**
         * Destructor
         */
        virtual ~Image();

        /**
         * No copy constructor, but a copy operator
         */
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image& operator=(Image&&) = default;

        /**
         * Get a pointer to the data
         */
        const void* data() const;

        /**
         * Lock the image, useful while reading. Use with care
         * Note that only write mutex is needed, as it also disables reading
         */
        void lock() {_writeMutex.lock();}
        void unlock() {_writeMutex.unlock();}

        /**
         * Get the image buffer
         */
        ImageBuffer get() const;

        /**
         * Get the file path
         */
        std::string getFilepath() const {return _filepath;}

        /**
         * Get the image buffer specs
         */
        ImageBufferSpec getSpec() const;

        /**
         * Get the timestamp for the current mesh
         */
        std::chrono::high_resolution_clock::time_point getTimestamp() const {return _timestamp;}

        /**
         * Set the image from an ImageBuffer
         */
        void set(const ImageBuffer& img);

        /**
         * Set the image as a empty with the given size / channels / typedesc
         */
        void set(unsigned int w, unsigned int h, unsigned int channels, ImageBufferSpec::Type type);

        /**
         * Serialize the image
         */
        std::shared_ptr<SerializedObject> serialize() const;

        /**
         * Update the Image from a serialized representation
         */
        bool deserialize(std::shared_ptr<SerializedObject> obj);

        /**
         * Set the path to read from
         */
        virtual bool read(const std::string& filename);

        /**
         * Set all pixels in the image to the specified value
         */
        void setTo(float value);

        /**
         * Update the content of the image
         */
        virtual void update();

        /**
         * Write the current buffer to the specified file
         */
        bool write(const std::string& filename);

    protected:
        std::unique_ptr<ImageBuffer> _image;
        std::unique_ptr<ImageBuffer> _bufferImage;
        std::string _filepath;
        bool _flip {false};
        bool _flop {false};
        bool _imageUpdated {false};
        bool _srgb {true};
        bool _benchmark {false};
        bool _linkedToWorldObject {false};

        void createDefaultImage(); //< Create a default black image
        void createPattern(); //< Create a default pattern

        /**
         * Read the specified image file
         */
        bool readFile(const std::string& filename);
        
    private:
        // Deserialization is done in this buffer, to avoid realloc
        ImageBuffer _bufferDeserialize;

        /**
         * Base init for the class
         */
        void init();
        
        /**
         * Register new functors to modify attributes
         */
        void registerAttributes();
};

typedef std::shared_ptr<Image> ImagePtr;

} // end of namespace

#endif // SPLASH_IMAGE_H

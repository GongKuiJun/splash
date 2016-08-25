/*
 * Copyright (C) 2015 Emmanuel Durand
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
 * @ltcclock.h
 * The LtcClock class, which reads a clock on an audio input
 */

#ifndef SPLASH_LTCCLOCK_H
#define SPLASH_LTCCLOCK_H

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <ltc.h>
#include <portaudio.h>

#include "config.h"
#include "coretypes.h"
#include "basetypes.h"
#include "listener.h"

namespace Splash {

/*************/
class LtcClock : public BaseObject
{
    public:
        struct Clock
        {
            uint8_t years {0};
            uint8_t months {0};
            uint8_t days {0};
            uint8_t hours {0};
            uint8_t mins {0};
            uint8_t secs {0};
            uint8_t frame {0};
            bool paused {false};
        };

        /**
         * \brief Constructor
         */
        LtcClock(bool masterClock = false, const std::string& deviceName = "");

        /**
         * \brief Destructor
         */
        ~LtcClock();

        /**
         * \brief Safe bool idiom
         */
        explicit operator bool() const
        {
            return _ready;
        }

        /**
         * \brief Get the clock as a Clock struct
         * \return Return a Clock struct
         */
        Clock getClock();

        /**
         * \brief Get the clock as as values (from years to frame, starting with a bool set to true if clock is running)
         */
        void getClock(Values& clockValues);

    private:
        bool _ready {false};
        bool _masterClock {false};
        bool _continue {false};
        std::thread _ltcThread;

        bool _framerateChanged {false};
        uint8_t _previousFrame {0};
        uint8_t _maximumFramePerSec {30};

        Clock _clock;
        std::unique_ptr<Listener> _listener;

        /**
         * \brief Register new functors to modify attributes
         */
        void registerAttributes();
};

} // end of namespace

#endif

/*
 * Copyright (C) 2013 Emmanuel Durand
 *
 * This file is part of Log.
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
 * @log.h
 * The Log class
 */

#ifndef SPLASH_LOG_H
#define SPLASH_LOG_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <utility>
#include <mutex>
#include <string>
#include <tuple>
#include <deque>

#include "config.h"

#include "coretypes.h"

namespace Splash
{

/*************/
class Log
{
    public:
        enum Priority
        {
            DEBUGGING = 0,
            MESSAGE,
            WARNING,
            ERROR,
            NONE
        };

        enum Action
        {
            endl = 0
        };

        /**
         * Get the singleton
         */
        static Log& get()
        {
            static auto instance = new Log;
            return *instance;
        }

        /**
         * Shortcut for any type of log
         */
        template<typename ... T>
        void operator()(Priority p, T ... args)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            rec(p, args...);
        }

        /**
         * Shortcut for setting MESSAGE log
         */
        template <typename T>
        Log& operator<<(const T& msg)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            addToString(_tempString, msg);
            return *this;
        }

        Log& operator<<(const Value& v)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            addToString(_tempString, v.asString());
            return *this;
        }

        Log& operator<<(Log::Action action)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (action == endl)
            {
                if (_tempPriority >= _verbosity)
                    rec(_tempPriority, _tempString);
                _tempString.clear();
                _tempPriority = MESSAGE;
            }
            return *this;
        }

        Log& operator<<(Log::Priority p)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _tempPriority = p;
            return *this;
        }

        /**
         * Get the full logs
         */
        std::deque<std::pair<std::string, Priority>> getFullLogs()
        {
            return _logs;
        }

        /**
         * Get the logs by priority
         */
        template<typename ... T>
        std::vector<std::string> getLogs(T ... args)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<Log::Priority> priorities {args...};
            std::vector<std::string> logs;
            for (auto log : _logs)
                for (auto p : priorities)
                    if (log.second == p)
                        logs.push_back(log.first);

            return logs;
        }

        /**
         * Get the new logs (from last call to this method)
         */
        std::vector<std::pair<std::string, Priority>> getNewLogs()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<std::pair<std::string, Priority>> logs;
            for (int i = _logPointer; i < _logs.size(); ++i)
                logs.push_back(_logs[i]);
            _logPointer = _logs.size();
            return logs;
        }

        /**
         * Get the verbosity of the console output
         */
        Priority getVerbosity()
        {
            return _verbosity;
        }

        /**
         * Set the verbosity of the console output
         */
        void setVerbosity(Priority p)
        {
            _verbosity = p;
        }

        /**
         * Add new logs from an outside source, i.e. another process
         */
        void setLog(std::string log, Priority priority)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _logs.push_back(std::pair<std::string, Priority>(log, priority));

            if (_logs.size() > _logLength)
            {
                _logPointer = _logPointer > 0 ? _logPointer - 1 : _logPointer;
                _logs.pop_front();
            }
        }

    private:
        /**
         * Constructor
         */
        Log() {}

        /**
         * Destructor
         */
        ~Log() {}

        /**
         * Delete some constructors
         */
        Log(const Log&) = delete;
        const Log& operator=(const Log&) = delete;

    private:
        mutable std::mutex _mutex;
        std::deque<std::pair<std::string, Priority>> _logs;
        int _logLength {500};
        int _logPointer {0};
        Priority _verbosity {MESSAGE};

        std::string _tempString;
        Priority _tempPriority {MESSAGE};

        /*****/
        template <typename T, typename... Ts>
        void addToString(std::string& str, const T& t, Ts& ... args) const
        {
            str += std::to_string(t);
            addToString(str, args...);
        }

        template <typename... Ts>
        void addToString(std::string& str, const std::string& s, Ts& ...args) const
        {
            str += s;
            addToString(str, args...);
        }

        template <typename... Ts>
        void addToString(std::string& str, const char* s, Ts& ...args) const
        {
            str += std::string(s);
            addToString(str, args...);
        }

        void addToString(std::string& str) const
        {
            return;
        }

        /**
         * Set a new log message
         */
        template<typename ... T>
        void rec(Priority p, T ... args)
        {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            char time_c[64];
            strftime(time_c, 64, "%T", std::localtime(&now_c));

            std::string timedMsg;
            std::string type;
            if (p == Priority::MESSAGE)
                type = std::string("[MESSAGE]");
            else if (p == Priority::DEBUGGING)
                type = std::string(" [DEBUG] ");
            else if (p == Priority::WARNING)
                type = std::string("[WARNING]");
            else if (p == Priority::ERROR)
                type = std::string(" [ERROR] ");

            timedMsg = std::string(time_c) + std::string(" / ") + type + std::string(" / ");

            addToString(timedMsg, args...);

            if (p >= _verbosity)
                toConsole(timedMsg);

            _logs.push_back(std::pair<std::string, Priority>(timedMsg, p));
            if (_logs.size() > _logLength)
            {
                _logPointer = _logPointer > 0 ? _logPointer - 1 : _logPointer;
                _logs.erase(_logs.begin());
            }
        }

        /*****/
        void toConsole(std::string msg)
        {
            if (msg.find("[MESSAGE]") != std::string::npos)
                msg.replace(msg.find("[MESSAGE]"), 9, "\033[32;1m[MESSAGE]\033[0m");
            else if (msg.find("[DEBUG]") != std::string::npos)
                msg.replace(msg.find("[DEBUG]"), 7, "\033[36;1m[DEBUG]\033[0m");
            else if (msg.find("[WARNING]") != std::string::npos)
                msg.replace(msg.find("[WARNING]"), 9, "\033[33;1m[WARNING]\033[0m");
            else if (msg.find("[ERROR]") != std::string::npos)
                msg.replace(msg.find("[ERROR]"), 7, "\033[31;1m[ERROR]\033[0m");

            std::cout << msg << std::endl;
        }
};

} // end of namespace

#endif // SPLASH_LOG_H

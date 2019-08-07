/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GTLIB2_LOGGINGH
#define GTLIB2_LOGGINGH

#include <chrono>
#include <ctime>
#include <thread>
#include <iomanip>
#include <iostream>

namespace GTLib2 {

constexpr unsigned int LOGLEVEL_CRITICAL = 50;
constexpr unsigned int LOGLEVEL_ERROR = 40;
constexpr unsigned int LOGLEVEL_WARNING = 30;
constexpr unsigned int LOGLEVEL_INFO = 20;
constexpr unsigned int LOGLEVEL_DEBUG = 10;
constexpr unsigned int LOGLEVEL_NOTSET = 0;

/**
 * Levels of logging (inspired by python)
 */
#ifndef NDEBUG
inline unsigned int log_level = LOGLEVEL_DEBUG;
#else
unsigned int log_level = LOGLEVEL_INFO;
#endif

inline auto lastMeasure = std::chrono::system_clock::now();
inline auto time_diff() {
    using namespace std::chrono;
    using std::to_string;

    auto now = system_clock::now();
    auto diff = duration_cast<microseconds>(now - lastMeasure).count();
    lastMeasure = now;

    std::stringstream ss;
    ss.fill(' ');
    ss.precision(2);
    ss << std::setw(4);
    if (diff > 1000000 * 60) {
        ss << diff / 1000000. * 60 << "min";
    } else if (diff > 1000000) {
        ss << diff / 1000000. << " s";
    } else if (diff > 1000) {
        ss << diff / 1000. << "ms";
    } else {
        ss << diff << "us";
    }
    return ss.str();
}


enum Color {
    NONE = 0,
    BLACK, RED, GREEN,
    YELLOW, BLUE, MAGENTA,
    CYAN, GRAY, WHITE

};

inline static std::string set_color(Color foreground = NONE, Color background = NONE) {
    std::stringstream s;
    s << "\033[";
    if (!foreground && !background) {
        s << "0"; // reset colors if no params
    }
    if (foreground) {
        s << 29 + foreground;
        if (background) s << ";";
    }
    if (background) {
        s << 39 + background;
    }
    s << "m";
    return s.str();
}

//@formatter:off
#define LOG_TIME_DIFF    time_diff()
#define LOG_THREAD       std::this_thread::get_id()

#define LOG_DEBUG(x)     if(log_level <= LOGLEVEL_DEBUG)    { cerr << set_color(GRAY) << "[DEBUG][thread-" << LOG_THREAD << "][" << LOG_TIME_DIFF << "] " << x << set_color() << endl; }
#define LOG_INFO(x)      if(log_level <= LOGLEVEL_INFO)     { cerr << set_color(WHITE) << "[INFO ][thread-" << LOG_THREAD << "][" << LOG_TIME_DIFF << "] " << x << set_color() << endl; }
#define LOG_WARN(x)      if(log_level <= LOGLEVEL_WARNING)  { cerr << set_color(GREEN) << "[WARN ][thread-" << LOG_THREAD << "][" << LOG_TIME_DIFF << "] " << x << set_color() << endl; }
#define LOG_ERROR(x)     if(log_level <= LOGLEVEL_ERROR)    { cerr << set_color(RED) << "[ERROR][thread-" << LOG_THREAD << "][" << LOG_TIME_DIFF << "] " << x << set_color() << endl; }
#define LOG_CRITICAL(x)  if(log_level <= LOGLEVEL_CRITICAL) { cerr << set_color(RED, GRAY) << "[CRIT ][thread-" << LOG_THREAD << "][" << LOG_TIME_DIFF << "] " << x << set_color() << endl; }
//@formatter:on

}

#endif // GTLIB2_LOGGINGH

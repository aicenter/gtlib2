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

namespace GTLib2::CLI {

// Levels of logging (inspired by python)
constexpr unsigned int LOGLEVEL_CRITICAL = 50;
constexpr unsigned int LOGLEVEL_ERROR = 40;
constexpr unsigned int LOGLEVEL_WARNING = 30;
constexpr unsigned int LOGLEVEL_INFO = 20;
constexpr unsigned int LOGLEVEL_DEBUG = 10;
constexpr unsigned int LOGLEVEL_VERBOSE = 5;
constexpr unsigned int LOGLEVEL_NOTSET = 0;

inline unsigned int log_level = LOGLEVEL_DEBUG;
inline bool log_thread = false;

inline auto runStartTime = std::chrono::system_clock::now();
inline auto lastLogMeasure = std::chrono::system_clock::now();

inline auto time_diff(std::chrono::system_clock::time_point &lastTime) {
    using namespace std::chrono;
    using std::to_string;

    auto now = system_clock::now();
    auto diff = duration_cast<microseconds>(now - lastTime).count();
    lastTime = now;

    std::stringstream ss;
    ss.fill(' ');
    ss.precision(1);
    ss << std::setw(5) << std::fixed;
    if (diff > 1000000 * 60) {
        ss << diff / 1000000. / 60 << "min";
    } else if (diff > 1000000) {
        ss << diff / 1000000. << " s";
    } else if (diff > 1000) {
        ss << diff / 1000. << "ms";
    } else {
        ss << diff / 1. << "us";
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
inline std::string thread() {
    if (CLI::log_thread) {
        std::stringstream ss;
        ss << "[thread-" << std::this_thread::get_id() << "]";
        return ss.str();
    } else {
        return "";
    }
}

//@formatter:off
#define LOG_TIME_DIFF    CLI::time_diff(CLI::lastLogMeasure)
#define LOG_THREAD       CLI::thread()

#define LOG_VAR(x)           cerr << __FILE__ << ":" << __LINE__ << " |  " << #x"=" << x << std::endl;
#define LOG_VAL(name, value) cerr << __FILE__ << ":" << __LINE__ << " |  " << name << "=" << value << std::endl;

#define LOG_VERBOSE(x)   if(CLI::log_level <= CLI::LOGLEVEL_VERBOSE)  { cerr << CLI::set_color(CLI::GRAY)           << "[VERBS]" << LOG_THREAD << " " << LOG_TIME_DIFF << " | " << x << CLI::set_color() << endl; }
#define LOG_DEBUG(x)     if(CLI::log_level <= CLI::LOGLEVEL_DEBUG)    { cerr << CLI::set_color(CLI::GRAY)           << "[DEBUG]" << LOG_THREAD << " " << LOG_TIME_DIFF << " | " << x << CLI::set_color() << endl; }
#define LOG_INFO(x)      if(CLI::log_level <= CLI::LOGLEVEL_INFO)     { cerr << CLI::set_color(CLI::WHITE)          << "[INFO ]" << LOG_THREAD << " " << LOG_TIME_DIFF << " | " << x << CLI::set_color() << endl; }
#define LOG_WARN(x)      if(CLI::log_level <= CLI::LOGLEVEL_WARNING)  { cerr << CLI::set_color(CLI::GREEN)          << "[WARN ]" << LOG_THREAD << " " << LOG_TIME_DIFF << " | " << x << CLI::set_color() << endl; }
#define LOG_ERROR(x)     if(CLI::log_level <= CLI::LOGLEVEL_ERROR)    { cerr << CLI::set_color(CLI::RED)            << "[ERROR]" << LOG_THREAD << " " << LOG_TIME_DIFF << " | " << x << CLI::set_color() << endl; }
#define LOG_CRITICAL(x)  if(CLI::log_level <= CLI::LOGLEVEL_CRITICAL) { cerr << CLI::set_color(CLI::RED, CLI::GRAY) << "[CRIT ]" << LOG_THREAD << " " << LOG_TIME_DIFF << " | " << x << CLI::set_color() << endl; }
//@formatter:on

}

#endif // GTLIB2_LOGGINGH

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

#ifndef GTLIB2_GLOBAL_ARGSH
#define GTLIB2_GLOBAL_ARGSH

#include "base/includes.h"
#include "utils/args.hpp"
#include "utils/logging.h"
#include "utils/utils.h"

#include <fstream>
#include <filesystem>

// Specify all global CLI arguments here
namespace args {

args::Group arguments("Global arguments");
args::ValueFlag<std::string>
    domain(arguments, "DOMAIN", "See constructDomain()", {'d', "domain"}, "IIGS_3");
args::ValueFlagList<std::string>
    alg(arguments, "ALG_NAME", "See constructAlgWithData()", {'a', "alg"}, {"RND", "RND"});
args::ValueFlagList<std::string>
    algcfg(arguments, "CFG_PATH", "Config files to use", {'c', "algcfg"}, {"", ""});
args::ValueFlagList<std::string>
    extra1(arguments, "PARAM=VAL", "Extra params to pass for alg1", {"x1"}, {""});
args::ValueFlagList<std::string>
    extra2(arguments, "PARAM=VAL", "Extra params to pass for alg2", {"x2"}, {""});
args::ValueFlag<unsigned int>
    log_level(arguments, "", "Logging level (see logging.h)", {'l', "log_level"},
#ifndef NDEBUG
    GTLib2::CLI::LOGLEVEL_DEBUG
#else
              GTLib2::CLI::LOGLEVEL_INFO
#endif
);
args::Flag log_thread(arguments, "", "Print thread number in logs", {"log_thread"}, false);
args::Flag tag(arguments, "", "Print params to stdout (CSV-like)", {'t', "tag"}, false);
args::Flag tag_header(arguments, "", "Print header to stdout (CSV-like)", {"tag_header"}, false);
args::Flag run_time(arguments, "", "Measure runtime of command", {'r', "run_time"}, false);

struct Header {}; // throw Header once tag_header is parsed
}

namespace GTLib2::CLI {

typedef unordered_map<string, string> AlgParams;
inline vector<AlgParams> algParams = vector<AlgParams>(2);

/**
 * Read .ini file (without support for [] sections), just key=value
 */
inline AlgParams readIni(const string &file) {
    std::fstream fs(file, std::fstream::in);
    if (!fs) {
        LOG_ERROR("Could not open " << std::filesystem::current_path() << "/" << file)
        exit(1);
    }

    unordered_map<string, string> params;
    char c;
    bool isKey = true;
    bool isComment = false;
    string key = "", val = "";
    while (!fs.eof()) {
        fs.get(c);

        if (c == ' ') {
            continue;
        } else if (c == ';') {
            isComment = true;
        } else if (c == '\n') {
            if(!key.empty()) params.emplace(key, val);
            key="";
            val="";
            isKey = true;
            isComment = false;
        } else if (isComment) {
            continue;
        } else if (c == '=') {
            isKey = false;
        } else if (isKey) {
            key += c;
        } else {
            val += c;
        }
    }
    fs.close();
    return params;
}

inline pair<string, string> parseIniKV(const string &kv) {
    string k, v;
    bool appendKey = true;
    for (char c : kv) {
        if (c == '=') appendKey = false;
        else if (appendKey) k += c;
        else v += c;
    }
    return make_pair(k, v);
}

void initializeParser(args::Subparser &parser) {
    parser.Parse();
    CLI::log_level = args::get(args::log_level);
    CLI::log_thread = args::get(args::log_thread);

    if (args::get(args::tag_header)) {
        cout << "domain" << ",";
        for (unsigned int i = 0; i < args::get(args::alg).size(); ++i) cout << "alg" << i << ",";
        for (unsigned int i = 0; i < args::get(args::algcfg).size(); ++i) cout << "algcfg" << i << ",";
    } else if (args::get(args::tag)) {
        cout << args::get(args::domain) << ",";
        for (auto &alg : args::get(args::alg)) cout << alg << ",";
        for (auto &algcfg : args::get(args::algcfg)) cout << algcfg << ",";
    }

    CLI::runStartTime = std::chrono::system_clock::now();

    // parse alg configs
    int numAlgs = args::get(args::algcfg).size();
    numAlgs = max(numAlgs, args::get(args::extra1).empty() ? 0 : 1);
    numAlgs = max(numAlgs, args::get(args::extra2).empty() ? 0 : 2);
    CLI::algParams = vector<AlgParams>(numAlgs);

    if (args::algcfg) {
        const auto algcfgs = args::get(args::algcfg);
        for (unsigned int i = 0; i < algcfgs.size(); ++i) {
            CLI::algParams.at(i) = readIni(algcfgs.at(i));
        }
    }
    if (args::extra1) {
        const auto kvs = args::get(args::extra1);
        for (const auto &kv : kvs) {
            auto[k, v] = parseIniKV(kv);
            CLI::algParams.at(0)[k] = v;
        }
    }
    if (args::extra2) {
        const auto kvs = args::get(args::extra2);
        for (const auto &kv : kvs) {
            auto[k, v] = parseIniKV(kv);
            CLI::algParams.at(1)[k] = v;
        }
    }

}

}

#endif // GTLIB2_GLOBAL_ARGSH

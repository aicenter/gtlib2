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

#ifndef GTLIB2_CLI_HELPERSH
#define GTLIB2_CLI_HELPERSH

#include <iomanip>
#include <fstream>

#include "external/cereal/archives/json.hpp"
#include "utils/args.hpp"
#include "utils/global_args.h"

#include "base/algorithm.h"

#include "algorithms/cfr.h"
#include "algorithms/oos.h"
#include "algorithms/mccr.h"

#include "domains/goofSpiel.h"
#include "domains/oshiZumo.h"
#include "domains/genericPoker.h"
#include "domains/liarsDice.h"
#include "domains/randomGame.h"
#include "domains/simple_games.h"
#include "domains/kriegspiel.h"
#include "domains/stratego.h"

namespace GTLib2::CLI {
using namespace domains;
using namespace algorithms;

unique_ptr<Domain> constructDomain(const string &description) {
    string buf;
    std::stringstream ss(description);
    string domain;
    vector<string> params;

    ss >> domain;
    while (ss >> buf) params.push_back(buf);

    const auto parseGoofspiel = [](vector<string> p, GoofSpielVariant v) {
        const auto numCards = static_cast<uint32>(stoi(p.at(0)));
        if (p.size() == 1) {
            return make_unique<GoofSpielDomain>(GoofSpielSettings{
                variant: v,
                numCards: numCards,
                fixChanceCards: false, // chance cards are not fixed !! (in constrast to IIGS_#)
                chanceCards: {},
                binaryTerminalRewards: false
            });
        }

        // chance cards are specified
        vector<int> cards(p.size() - 1);
        std::transform(p.begin() + 1, p.end(), cards.begin(), [](const std::string &val) {
            return std::stod(val);
        });

        return make_unique<GoofSpielDomain>(GoofSpielSettings{
            variant: v,
            numCards: numCards,
            fixChanceCards: true,
            chanceCards: cards,
            binaryTerminalRewards: false
        });
    };

    const auto parseOshizumo = [](vector<string> p, OshiZumoVariant v) {
        const auto startingCoins = p.size() >= 1 ? static_cast<uint32>(stoi(p.at(0))) : 3;
        const auto startingLocation = p.size() >= 2 ? static_cast<uint32>(stoi(p.at(1))) : 3;
        const auto minBid = p.size() >= 3 ? static_cast<uint32>(stoi(p.at(2))) : 1;

        return make_unique<OshiZumoDomain>(OshiZumoSettings{
            variant: v,
            startingCoins: startingCoins,
            startingLocation: startingLocation,
            minBid: minBid,
        });
    };

    // @formatter:off
    unordered_map<string, function<unique_ptr<Domain>(vector<string>)>> domainsTable = {
        {"IIGS",       [&](vector<string> p) { return parseGoofspiel(p, GoofSpielVariant::IncompleteObservations); }},
        {"IIGS_2",     [ ](vector<string> p) { return GoofSpielDomain::IIGS(2); }},
        {"IIGS_3",     [ ](vector<string> p) { return GoofSpielDomain::IIGS(3); }},
        {"IIGS_4",     [ ](vector<string> p) { return GoofSpielDomain::IIGS(4); }},
        {"IIGS_5",     [ ](vector<string> p) { return GoofSpielDomain::IIGS(5); }},
        {"IIGS_6",     [ ](vector<string> p) { return GoofSpielDomain::IIGS(6); }},
        {"IIGS_13",    [ ](vector<string> p) { return GoofSpielDomain::IIGS(13); }},
        {"IIGS_small", [ ](vector<string> p) { return GoofSpielDomain::IIGS(5); }},
        {"IIGS_large", [ ](vector<string> p) { return GoofSpielDomain::IIGS(13); }},
        {"GS",         [&](vector<string> p) { return parseGoofspiel(p, GoofSpielVariant::CompleteObservations); }},
        {"GS_2",       [ ](vector<string> p) { return GoofSpielDomain::GS(2); }},
        {"GS_3",       [ ](vector<string> p) { return GoofSpielDomain::GS(3); }},
        {"GS_4",       [ ](vector<string> p) { return GoofSpielDomain::GS(4); }},
        {"GS_5",       [ ](vector<string> p) { return GoofSpielDomain::GS(5); }},
        {"GS_6",       [ ](vector<string> p) { return GoofSpielDomain::GS(6); }},
        {"IIOZ",       [&](vector<string> p) { return parseOshizumo(p, OshiZumoVariant::IncompleteObservation); }},
        {"IIOZ_2",     [ ](vector<string> p) { return OshiZumoDomain::IIOZ(2); }},
        {"IIOZ_3",     [ ](vector<string> p) { return OshiZumoDomain::IIOZ(3); }},
        {"IIOZ_4",     [ ](vector<string> p) { return OshiZumoDomain::IIOZ(4); }},
        {"IIOZ_5",     [ ](vector<string> p) { return OshiZumoDomain::IIOZ(5); }},
        {"IIOZ_6",     [ ](vector<string> p) { return OshiZumoDomain::IIOZ(6); }},
        {"0Z",         [&](vector<string> p) { return parseOshizumo(p, OshiZumoVariant::CompleteObservation); }},
        {"0Z_2",       [ ](vector<string> p) { return OshiZumoDomain::OZ(2); }},
        {"0Z_3",       [ ](vector<string> p) { return OshiZumoDomain::OZ(3); }},
        {"0Z_4",       [ ](vector<string> p) { return OshiZumoDomain::OZ(4); }},
        {"0Z_5",       [ ](vector<string> p) { return OshiZumoDomain::OZ(5); }},
        {"0Z_6",       [ ](vector<string> p) { return OshiZumoDomain::OZ(6); }},
        {"MP",         [ ](vector<string> p) { return make_unique<MatchingPenniesDomain>(); }},
        {"RPS",        [ ](vector<string> p) { return make_unique<RPSDomain>(); }},
        {"BRPS",       [ ](vector<string> p) { return make_unique<BiasedRPSDomain>(stod(p.at(0))); }},
        {"PD",         [ ](vector<string> p) { return make_unique<PrisonnersDilemmaDomain>(); }},
        {"GP_322221",  [ ](vector<string> p) { return make_unique<GenericPokerDomain>(3,2,2,2,2,1); }},
        {"GP_464441",  [ ](vector<string> p) { return make_unique<GenericPokerDomain>(4,6,4,4,4,1); }},
        {"GP_small",   [ ](vector<string> p) { return make_unique<GenericPokerDomain>(3,2,2,2,2,1); }},
        {"GP_large",   [ ](vector<string> p) { return make_unique<GenericPokerDomain>(4,6,4,4,4,1); }},
        {"LD_116",     [ ](vector<string> p) { return make_unique<LiarsDiceDomain>(vector<int>{1, 1}, 6); }},
        {"LD_226",     [ ](vector<string> p) { return make_unique<LiarsDiceDomain>(vector<int>{2, 2}, 6); }},
        {"LD_small",   [ ](vector<string> p) { return make_unique<LiarsDiceDomain>(vector<int>{1, 1}, 6); }},
        {"LD_large",   [ ](vector<string> p) { return make_unique<LiarsDiceDomain>(vector<int>{2, 2}, 6); }},
        {"STRAT2x2",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{2,2,{},{'1', '2'}}); }},
        {"STRAT2x3",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{2,3,{},{'1', '2', '3'}});}},
        {"STRAT3x2",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{3,2,{},{'1', '2'}});}},
        {"STRAT3x3",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{3,3,{{1,1,1,1}},{'1', '2', '3'}});}},
        {"STRAT4x4",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{4,4,{{1,1,2,2}}, {'3','2','2','1'}});}},
        {"STRAT6x6",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{6,6,{{2,2,2,2}}, {'B','4','3','3','2','2', '2','1','1','1','1','F'}});}},
        {"STRAT10x10", [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{10,10, {{3,5,2,2}, {7,5,2,2}}}); }},
        {"KS",         [ ](vector<string> p) { return make_unique<KriegspielDomain>(1000, 1000, chess::BOARD::STANDARD); }},
        {"KS_STANDARD",      [ ](vector<string> p) { return make_unique<KriegspielDomain>(1000, 1000, chess::BOARD::STANDARD); }},
        {"KS_SILVERMAN4BY4", [ ](vector<string> p) { return make_unique<KriegspielDomain>(1000, 1000, chess::BOARD::SILVERMAN4BY4); }},
        {"KS_MINIMAL3x3",    [ ](vector<string> p) { return make_unique<KriegspielDomain>(1000, 1000, chess::BOARD::MINIMAL3x3); }},
        {"KS_MICROCHESS",    [ ](vector<string> p) { return make_unique<KriegspielDomain>(1000, 1000, chess::BOARD::MICROCHESS); }},
        {"KS_DEMICHESS ",    [ ](vector<string> p) { return make_unique<KriegspielDomain>(1000, 1000, chess::BOARD::DEMICHESS ); }},
    };
    // @formatter:on

    if(domainsTable.find(domain) == domainsTable.end()) {
        LOG_ERROR("Domain not found: " << domain);
        LOG_ERROR("List of available domains:")
        for(const auto &[domain, cb] : domainsTable) {
            cerr << domain << endl;
        }
        abort();
    }
    return domainsTable.at(domain)(params);
}

std::unique_ptr<GTLib2::AlgorithmWithData> constructAlgWithData(const GTLib2::Domain &d,
                                                                const std::string &algName,
                                                                std::string settingFile) {

    struct WrapperCFR: AlgorithmWithData {
        CFRData data;
        CFRSettings cfg;
        inline WrapperCFR(const Domain &d, CFRSettings _cfg) :
            data(CFRData(d, _cfg.cfrUpdating)), cfg(_cfg) {}
        PreparedAlgorithm prepare() override { return createInitializer<CFRAlgorithm>(data, cfg); }
    };
    struct WrapperOOS: AlgorithmWithData {
        OOSData data;
        OOSSettings cfg;
        inline WrapperOOS(const Domain &d, OOSSettings _cfg) : data(OOSData(d)), cfg(_cfg) {}
        PreparedAlgorithm prepare() override { return createInitializer<OOSAlgorithm>(data, cfg); }
    };
    struct WrapperMCCR: AlgorithmWithData {
        OOSData data;
        MCCRSettings cfg;
        inline WrapperMCCR(const Domain &d, MCCRSettings _cfg) : data(OOSData(d)), cfg(_cfg) {}
        PreparedAlgorithm prepare() override { return createInitializer<MCCRAlgorithm>(data, cfg); }
    };
    struct WrapperRND: AlgorithmWithData {
        PreparedAlgorithm prepare() override { return createInitializer<RandomPlayer>(); }
    };

    std::fstream fs;
    unique_ptr<cereal::JSONInputArchive> deserialize;

    // @formatter:off
    unordered_map<string, string> configsTable = {
        {"CFR", "settings/cfr.json"},
        {"OOS", "settings/oos.json"},
        {"MCCR", "settings/mccr.json"},
        {"RND", "settings/rnd.json"},
    };

    unordered_map<string, function<unique_ptr<AlgorithmWithData>()>> algorithmsTable = {
        {"CFR",   [&]() {
            CFRSettings settings;
            (*deserialize)(settings);
            return make_unique<WrapperCFR>(d, settings);
        }},
        {"OOS",   [&]() {
            OOSSettings settings;
            (*deserialize)(settings);
            return make_unique<WrapperOOS>(d, settings);
        }},
        {"MCCR",   [&]() {
            MCCRSettings settings;
            (*deserialize)(settings);
            return make_unique<WrapperMCCR>(d, settings);
        }},
        {"RND",   [&]() { return make_unique<WrapperRND>(); }},
    };
    // @formatter:on

    if(algorithmsTable.find(algName) == algorithmsTable.end()) {
        LOG_ERROR("Algorithm not found: " << algName);
        LOG_ERROR("List of available algorithms:")
        for(const auto &[alg, cb] : algorithmsTable) {
            cerr << alg << endl;
        }
        abort();
    }

    if (settingFile.empty()) {
        string defaultConfig = configsTable.at(algName);
        LOG_INFO("No config file was supplied for " << algName << ", using " << defaultConfig)
        settingFile = defaultConfig;
    }

    fs.open(settingFile, std::fstream::in);
    if (!fs) {
        LOG_ERROR("Could not open " << settingFile)
        exit(1);
    }

    deserialize = make_unique<cereal::JSONInputArchive>(fs);
    return algorithmsTable.at(algName)();
}

}

#endif

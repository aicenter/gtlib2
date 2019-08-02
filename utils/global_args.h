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

#include <domains/stratego.h>
#include "utils/args.hpp"

#ifndef GTLIB2_GLOBAL_ARGSH
#define GTLIB2_GLOBAL_ARGSH

// Specify all global arguments here
namespace args {
args::Group arguments("global arguments");
args::ValueFlag<std::string> domain(arguments,
                                    "DOMAIN",
                                    "Domain name to use, see constructDomain function for details.",
                                    {"d", "domain"}, "IIGS_3");
}

#include "domains/goofSpiel.h"
#include "domains/oshiZumo.h"
#include "domains/randomGame.h"
#include "domains/simple_games.h"

std::unique_ptr<GTLib2::Domain> constructDomain(const std::string &description) {
    using namespace GTLib2;
    using domains::GoofSpielDomain;
    using domains::GoofSpielSettings;
    using domains::GoofSpielVariant;
    using domains::OshiZumoDomain;
    using domains::OshiZumoSettings;
    using domains::OshiZumoVariant;
    using domains::RPSDomain;
    using domains::BiasedRPSDomain;
    using domains::MatchingPenniesDomain;
    using domains::PrisonnersDilemmaDomain;
    using domains::StrategoDomain;
    using domains::StrategoSettings;


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
                .variant =  v,
                .numCards =  numCards,
                .fixChanceCards =  false, // chance cards are not fixed !! (in constrast to IIGS_#)
                .chanceCards =  {},
                .binaryTerminalRewards =  false
            });
        }

        // chance cards are specified
        vector<int> cards(p.size() - 1);
        std::transform(p.begin() + 1, p.end(), cards.begin(), [](const std::string &val) {
            return std::stod(val);
        });

        return make_unique<GoofSpielDomain>(GoofSpielSettings{
            .variant =  v,
            .numCards =  numCards,
            .fixChanceCards =  true,
            .chanceCards =  cards,
            .binaryTerminalRewards =  false
        });
    };

    const auto parseOshizumo = [](vector<string> p, OshiZumoVariant v) {
        const auto startingCoins = p.size() >= 1 ? static_cast<uint32>(stoi(p.at(0))) : 3;
        const auto startingLocation = p.size() >= 2 ? static_cast<uint32>(stoi(p.at(1))) : 3;
        const auto minBid = p.size() >= 3 ? static_cast<uint32>(stoi(p.at(2))) : 1;

        return make_unique<OshiZumoDomain>(OshiZumoSettings{
            .variant = v,
            .startingCoins = startingCoins,
            .startingLocation = startingLocation,
            .minBid = minBid,
        });
    };

    // @formatter:off
    unordered_map<string, function<unique_ptr<Domain>(vector<string>)>> domainsTable = {
        {"IIGS",   [&](vector<string> p) { return parseGoofspiel(p, GoofSpielVariant::IncompleteObservations); }},
        {"IIGS_2", [ ](vector<string> p) { return GoofSpielDomain::IIGS(2); }},
        {"IIGS_3", [ ](vector<string> p) { return GoofSpielDomain::IIGS(3); }},
        {"IIGS_4", [ ](vector<string> p) { return GoofSpielDomain::IIGS(4); }},
        {"IIGS_5", [ ](vector<string> p) { return GoofSpielDomain::IIGS(5); }},
        {"IIGS_6", [ ](vector<string> p) { return GoofSpielDomain::IIGS(6); }},
        {"GS",     [&](vector<string> p) { return parseGoofspiel(p, GoofSpielVariant::CompleteObservations); }},
        {"GS_2",   [ ](vector<string> p) { return GoofSpielDomain::GS(2); }},
        {"GS_3",   [ ](vector<string> p) { return GoofSpielDomain::GS(3); }},
        {"GS_4",   [ ](vector<string> p) { return GoofSpielDomain::GS(4); }},
        {"GS_5",   [ ](vector<string> p) { return GoofSpielDomain::GS(5); }},
        {"GS_6",   [ ](vector<string> p) { return GoofSpielDomain::GS(6); }},
        {"IIOZ",   [&](vector<string> p) { return parseOshizumo(p, OshiZumoVariant::IncompleteObservation); }},
        {"IIOZ_2", [ ](vector<string> p) { return OshiZumoDomain::IIOZ(2); }},
        {"IIOZ_3", [ ](vector<string> p) { return OshiZumoDomain::IIOZ(3); }},
        {"IIOZ_4", [ ](vector<string> p) { return OshiZumoDomain::IIOZ(4); }},
        {"IIOZ_5", [ ](vector<string> p) { return OshiZumoDomain::IIOZ(5); }},
        {"IIOZ_6", [ ](vector<string> p) { return OshiZumoDomain::IIOZ(6); }},
        {"0Z",     [&](vector<string> p) { return parseOshizumo(p, OshiZumoVariant::CompleteObservation); }},
        {"0Z_2",   [ ](vector<string> p) { return OshiZumoDomain::OZ(2); }},
        {"0Z_3",   [ ](vector<string> p) { return OshiZumoDomain::OZ(3); }},
        {"0Z_4",   [ ](vector<string> p) { return OshiZumoDomain::OZ(4); }},
        {"0Z_5",   [ ](vector<string> p) { return OshiZumoDomain::OZ(5); }},
        {"0Z_6",   [ ](vector<string> p) { return OshiZumoDomain::OZ(6); }},
        {"MP",     [ ](vector<string> p) { return make_unique<MatchingPenniesDomain>(); }},
        {"RPS",    [ ](vector<string> p) { return make_unique<RPSDomain>(); }},
        {"BRPS",   [ ](vector<string> p) { return make_unique<BiasedRPSDomain>(stod(p.at(0))); }},
        {"PD",     [ ](vector<string> p) { return make_unique<PrisonnersDilemmaDomain>(); }},
        {"STRAT2x2",     [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{2,2,{},{'1', '2'}}); }},
        {"STRAT2x3",     [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{2,3,{},{'1', '2', '3'}});}},
        {"STRAT3x2",     [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{3,2,{},{'1', '2'}});}},
        {"STRAT3x3",     [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{3,3,{{1,1,1,1}},{'1', '2', '3'}});}},
        {"STRAT10x10",   [ ](vector<string> p) { return make_unique<StrategoDomain>(StrategoSettings{10,10,{{3,5,2,2}, {7,5,2,2}},{'B','B','B','B','B','B','9','8','7','7','6','6','6','5','5','5','5','4','4','4','4','3','3','3','3','2','2','2','2','2','1','1','1','1','1','1','1','1','0','F' }});}}
    };
    // @formatter:on

    return domainsTable.at(domain)(params);
}

#endif // GTLIB2_GLOBAL_ARGSH

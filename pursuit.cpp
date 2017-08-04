//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"

Random::Random():gen(std::random_device()()) {}
Random* Random::instance = nullptr;
Random *Random::getInstance() {
    if (instance == nullptr)   // Only allow one instance of class to be generated.
        instance = new Random();
    return instance;
}

double Random::getRandom() {
    return dis(gen);
}

Domain::Domain(int w, int h, State &r): width(w), height(h), root(r) {}

PursuitDomain::PursuitDomain(int w, int h,int max, State &r): Domain(w,h,r), maxdepth(max){}

int PursuitDomain::getMaxDepth() const{
    return maxdepth;
}

Action::Action(int t_id): id(t_id) {}

Action::Action(int t_id,  const std::string& t_s): id(t_id), s(t_s){}

Observation::Observation(int t_id): id(t_id){}

Observation::Observation(int t_id,  const std::string& t_s):id(t_id), s(t_s){}

Outcome::Outcome( const std::vector<Observation> &t_ob, const std::vector<int> &t_rew):  ob(t_ob), rew(t_rew){}

PursuitOutcome::PursuitOutcome(const PursuitState &s, const std::vector<Observation> &t_ob,
                               const std::vector<int> &t_rew): st(s), Outcome(t_ob, t_rew){}

Chance::Chance(const PursuitState &s,const std::vector<Observation> &t_ob, const std::vector<int> &t_rew) : PursuitOutcome(s,t_ob, t_rew) {}

State::State() = default;

void State::getActions(std::vector<Action> &list, int player) {};

void PursuitState::getActions(std::vector<Action> &list,int player, int t_i, int t_j){
    for (int i = 1; i < 5; ++i) {
        if((place[player-1].x + m[i].x) >= 0 && (place[player-1].x + m[i].x) < t_j
           &&(place[player-1].y + m[i].y) >= 0 && (place[player-1].y + m[i].y) < t_i){
            list.emplace_back(i);
        }
    }
}

PursuitState::PursuitState(std::vector<Pos> &p):place(p){
    setProb();
}

void PursuitState::setProb() {
    probDis = {0.2,0.8};
}


PursuitOutcome PursuitState::PerformAction(std::vector<Action>& t_actions){
    std::vector<Pos> moves = std::vector<Pos>(t_actions.size());
    for(int i = 0; i < t_actions.size();  ++i) {
     //   Random *r = r->getInstance();
        double number =(rand()%100)/100.0; //r->getRandom();//
        if(number > probDis[0]) {
            moves[i].x = m[t_actions[i].getID()].x + place[i].x;
            moves[i].y = m[t_actions[i].getID()].y + place[i].y;
        }
        else{
            moves[i].x = place[i].x;
            moves[i].y = place[i].y;
        }
    }
    PursuitState s = PursuitState(moves);
    std::vector<int> rew = std::vector<int>(s.place.size());

    for (int i = 1; i < s.getPlace().size(); ++i) {
        if((s.place[0].x == place[i].x && s.place[0].y == place[i].y && s.place[i].x == place[0].x && s.place[i].y == place[0].y) ||
           (s.place[0].x == s.place[i].x && s.place[0].y == s.place[i].y)){
            ++rew[0];
            --rew[i];
        }
    }
    std::vector<Pos> eight = {{-2,-2},{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}, {0,0}};
    std::vector<Observation> obs = std::vector<Observation>();
    int index;
    obs.emplace_back(0);
    for(int i = 1; i < s.place.size(); ++i) {
        index = 0;
        for (int j = 1; j < eight.size(); ++j) {
            if(s.place[0].x + eight[j].x == s.place[i].x && s.place[0].y + eight[j].y == s.place[i].y) {
                index = j;
            }
        }
        obs[0] = Observation(index);
        obs.emplace_back(eight.size()-1-index);
    }

    PursuitOutcome o = PursuitOutcome(s, obs, rew);
    return o;
}


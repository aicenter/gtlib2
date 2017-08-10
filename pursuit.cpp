//
// Created by rozliv on 02.08.2017.
//

#include "pursuit.h"

PursuitAction::PursuitAction(int id, int move): Action(id), move_(move) {}

std::string PursuitAction::ToString() {
    return std::to_string(id_);
}


PursuitObservation::PursuitObservation(int id, int move): Observation(id), move_(move){}

std::string PursuitObservation::ToString() {
    return std::to_string(id_);
}


PursuitState::PursuitState(std::vector<Pos> &p):place_(p){
    probdis_ = {0.1,0.9};
}

PursuitState::PursuitState(std::vector<Pos> &p, double prob):place_(p), prob_(prob){
    probdis_ = {0.1,0.9};
}

std::vector<std::shared_ptr<Action>> PursuitState::getActions(int player) {
    std::vector<std::shared_ptr<Action>> list = std::vector<std::shared_ptr<Action>>();
    getActions(list,player);
    return list;
}

void PursuitState::getActions(std::vector<std::shared_ptr<Action>> &list,int player)const {
    int count = 0;
    for (int i = 1; i < 5; ++i) {
        if((place_[player].x + m_[i].x) >= 0 && (place_[player].x + m_[i].x) < PursuitDomain::width_
           &&(place_[player].y + m_[i].y) >= 0 && (place_[player].y + m_[i].y) < PursuitDomain::height_){
      //      std::shared_ptr<PursuitAction> a =
            list.push_back(std::make_shared<PursuitAction>(count,i));
        //    list.emplace_back(new PursuitAction(count, i));
            count++;
        }
    }

}

ProbDistribution PursuitState::PerformAction(std::vector<std::shared_ptr<Action>> actions) {
    //std::vector<std::shared_ptr<PursuitAction>> actions(actions2.begin(), actions2.end());// = std::vector<double> doubleVec(intVec.begin(), intVec.end());//static_cast<std::vector<std::shared_ptr<PursuitAction>>> (actions2);
    //TODO: for loop na pretypovani
//    std::vector<std::shared_ptr<PursuitAction>> actions = std::vector<std::shared_ptr<PursuitAction>>();// = std::vector<std::shared_ptr<PursuitAction>>(actions2.size());
//    for (int j = 0; j < actions2.size(); ++j) {
//        PursuitAction *b = dynamic_cast< PursuitAction* >( actions2[j].get() );
//        actions.push_back(std::make_shared<PursuitAction>((b->getID(),b->getMove())));
//      //  actions.push_back(std::make_shared<PursuitAction>(dynamic_cast<PursuitAction*>(actions2[j].get())));
//    }
    std::vector<Pos> moves = std::vector<Pos>();
    moves.reserve(actions.size());
    auto count = static_cast<unsigned int>(pow(2, actions.size()));
    std::vector<std::pair<Outcome, double>> pairs;
    for (int k = 0; k < count; ++k) {
        std::vector<double> rew = std::vector<double>(actions.size());
        double probability = prob_;
        for (int i = 0; i < actions.size(); ++i) {
            if (((k >>i) & 1) == 1) {
                moves.push_back({m_[actions[i]->getMove()].y + place_[i].y, m_[actions[i]->getMove()].x + place_[i].x});
                probability *= probdis_[1];
            } else {
                moves.push_back({place_[i].y, place_[i].x});
                probability *= probdis_[0];
            }
        }
        std::unique_ptr<PursuitState>s = make_unique<PursuitState>(moves, probability);
        moves.clear();
        for (int i = 1; i < s->getPlace().size(); ++i) {
            if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
                 s->place_[i].x == place_[0].x &&
                 s->place_[i].y == place_[0].y) ||
                (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
                ++rew[0];
                --rew[i];
            }
        }

        int index;
        std::vector<std::unique_ptr<Observation>> obs = std::vector<std::unique_ptr<Observation>>();
        obs.push_back(make_unique<PursuitObservation>(0,2));
        for(int i = 1; i < s->place_.size(); ++i) {
            index = 0;
            for (int l = 1; l < eight.size(); ++l) {
                if (s->place_[0].x + eight[l].x == s->place_[i].x && s->place_[0].y + eight[l].y == s->place_[i].y) {
                    index = l;
                }
            }
     //       obs[0] = PursuitObservation(0,index); //TODO: zeptat se jak je to s tim vektorem observaci
            obs.push_back(make_unique<PursuitObservation>(k,eight.size()-1-index));
          //  obs.emplace_back(k,eight.size()-1-index);
        }
        double p2 = s->prob_/prob_;
        Outcome p (std::move(s), std::move(obs), rew);
      //  cout << s.use_count() << "   ";
        pairs.emplace_back(std::move(p), p2);
    }
    ProbDistribution prob(std::move(pairs));
    return prob;
}

PursuitState::PursuitState() = default;

PursuitDomain::PursuitDomain(int max,const std::unique_ptr<State> &r): Domain(r),maxdepth_(max){}

int PursuitDomain::height_ = 0;
int PursuitDomain::width_ = 0;

std::string PursuitDomain::GetInfo() {
    return "Rozmery pole jsou: " + std::to_string(PursuitDomain::height_) + " x " +
           std::to_string(PursuitDomain::height_) + " maximalni hloubka grafu je: " +  std::to_string(maxdepth_) +
           " a pocatecni stav je: ...";
}

int count = 0;
std::vector<double> rewards;

void pursuit(const Domain& domain,const std::unique_ptr<State> &state, int depth) {
   for (int i = 0; i < domain.getMaxDepth() - depth; ++i) {
        cout << "    ";
    }

    string s = std::to_string(state->getPlace()[0].x) + " " + std::to_string(state->getPlace()[0].y) + "    " +
               std::to_string(state->getPlace()[1].x) + " " + std::to_string(state->getPlace()[1].y) + "   ";
  //             std::to_string(state.getPro());
      cout << s<< '\n';
  //  cout << state.use_count()<<'\n';
    vector<std::shared_ptr<Action>> actions = vector<std::shared_ptr<Action>>();
    vector<std::shared_ptr<Action>> actions2 = vector<std::shared_ptr<Action>>();
    ++count;
    if (depth == 0) {
        return;
    }
    state->getActions(actions, 0);
  //  cout << actions.size() << " ";
    state->getActions(actions2, 1);
//    for (int k = 0; k < actions.size(); ++k) {


    for (auto &action : actions) {
        for (auto &j : actions2) {
            vector<std::shared_ptr<Action>> base ={action,j}; //{std::move(actions.back()), std::move(actions2.back())}; //{make_shared<PursuitAction>(action.get()) , make_shared<PursuitAction>(j.get())};
          //  actions.pop_back();
          //  actions2.pop_back();
            ProbDistribution prob = state->PerformAction(base);//{std::move(action),std::move(j)});
            for (Outcome &o : prob.GetOutcomes()) {
                for (int i = 0; i < rewards.size(); ++i) {
                    rewards[i] += o.getReward()[i];
                }
                pursuit(domain, o.getState(), depth - 1);
            }
        }
    }
}

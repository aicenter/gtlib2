//
// Created by Jakub Rozlivek on 02.08.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_H_
#define PURSUIT_H_

#include <utility>
#include <numeric>

#include "../base/base.h"


using namespace GTLib2;

namespace GTLib2 {

    struct Pos {
        int y;
        int x;
    };

// moves description
    const vector<string> movedes_ = {"stay", "right", "down", "left", "up"};

// eight surrounding description -  MooreNeighborhood
    const vector<string> eightdes_ = {"nowhere", "top left", "top", "top right",
                                      "left", "right", "bottom left", "bottom",
                                      "bottom right", "same"};

/**
 * PursuitAction is a class that represents pursuit actions,
 * which are identified by their id and contain where to move.
 */
    class PursuitAction : public Action {
    public:
        // constructor
        PursuitAction(int id, int move);

        // Returns move description.
        inline string toString() const final {
          if (id == -1)
            return "NoA";
          return movedes_[move_];
        }

        // Returns index to array, which describes where to move.
        inline int GetMove() const {
          return move_;
        }

    private:
        int move_;
    };


/**
 * PursuitObservation is a class that represents pursuit observation,
 * which are identified by their id and
 * contain vector of mini-observations to others.
 */
    class PursuitObservation : public Observation {
    public:
        // constructor
        PursuitObservation(int id, vector<int> values);

        // Returns description.
        string toString() const final;

        // Returns vector of mini-observations to others.
        inline const vector<int> &GetValues() const {
          return values_;
        }

    private:
        vector<int> values_;
    };

/**
 * PursuitObservationLoc is a class that represents pursuit observation,
 * which are identified by their id and contain vector of others' locations.
 */
    class PursuitObservationLoc : public Observation {
    public:
        // constructor
        PursuitObservationLoc(int id, vector<Pos> values);

        // Returns description.
        string toString() const final;

        // Returns vector of mini-observations to others (others' locations).
        inline const vector<Pos> &GetValues() const {
          return values_;
        }

    private:
        vector<Pos> values_;
    };


/**
 * PursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 */
    class PursuitState : public State {
    public:
        // Constructor
        explicit PursuitState(Domain* domain, const vector<Pos> &p);

        // Constructor
        PursuitState(Domain* domain, const vector<Pos> &p, double prob);

        // Destructor
        ~PursuitState() override = default;

        // GetActions returns possible actions for a player in the state.
        vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

        OutcomeDistribution
        performActions(const vector<pair<int, shared_ptr<Action>>> &actions2) const override;

        inline vector<int> getPlayers() const final {
          return players_;
        }

        // ToString returns state description.
        inline string toString() const override {
          string s;
          for(auto player = 0; player < place_.size(); ++player) {
            s += "player: " + to_string(player) + ", location: " +
                 to_string(place_[player].x) + " " + to_string(place_[player].y) +
                 strings_[player] + "\n";
          }
          return s;
        }

      inline bool operator==(const State &rhs) const override {
        auto State = dynamic_cast<const PursuitState&>(rhs);

        return  place_[0].x== State.place_[0].x &&
                place_[1].x== State.place_[1].x &&
                place_[0].y== State.place_[0].y &&
                place_[1].y== State.place_[1].y &&
                players_ == State.players_ &&
                prob_ == State.prob_;
        }

      inline size_t getHash() const override {
        size_t seed = 0;
        for (auto &i : place_) {
          boost::hash_combine(seed, i.x);
          boost::hash_combine(seed, i.y);
        }
        for (auto &i : players_) {
          boost::hash_combine(seed, i);
        }
        boost::hash_combine(seed, prob_);
        return seed;
      }


    protected:
        vector<Pos> place_;  // locations of all players
        // eight surrounding
        vector<Pos> eight_ = {{-2, -2},
                              {-1, -1},
                              {0,-1},
                              {1, -1},
                              {-1,  0},
                              {1,  0},
                              {-1,  1},
                              {0,  1},
                              {1,  1},
                              {0,  0}};
        vector<Pos> m_ = {{0,  0},
                          {1,  0},
                          {0,  1},
                          {-1, 0},
                          {0,  -1}};  // moves
        vector<string> strings_;
        vector<int> players_;
        double prob_ = 1;  // state probability
    };


/**
 * MMPursuitState is a class that represents multiple-move pursuit states,
 * which contains vector of all players and a move count of player on turn.
 */
    class MMPursuitState : public PursuitState {
    public:
        // Constructor
        MMPursuitState(Domain* domain, const vector<Pos> &p, const vector<int> &players,
                       vector<int> numberOfMoves);

        // Constructor
        MMPursuitState(Domain* domain, const vector<Pos> &p, const vector<int> &players,
                       vector<int> numberOfMoves, int currentNOM, int currentPlayer);

        // Constructor
        MMPursuitState(Domain* omain, const vector<Pos> &p, double prob,
                       const vector<int> &players, vector<int> numberOfMoves);

        // Constructor
        MMPursuitState(Domain* domain, const vector<Pos> &p, double prob, const vector<int> &players,
                       vector<int> numberOfMoves, int currentNOM, int currentPlayer);

        OutcomeDistribution
        performActions(const vector<pair<int, shared_ptr<Action>>> &actions2) const override;

      inline bool operator==(const State &rhs) const override {
        auto State = dynamic_cast<const MMPursuitState&>(rhs);

        return  place_[0].x== State.place_[0].x &&
                place_[1].x== State.place_[1].x &&
                place_[0].y== State.place_[0].y &&
                place_[1].y== State.place_[1].y &&
                players_ == State.players_ &&
                prob_ == State.prob_ &&
                numberOfMoves_ == State.numberOfMoves_ &&
                currentNOM_ == State.currentNOM_ &&
                currentPlayer_ == State.currentPlayer_;
      }

      inline size_t getHash() const override {
        size_t seed = 0;
        for (auto &i : place_) {
          boost::hash_combine(seed, i.x);
          boost::hash_combine(seed, i.y);
        }
        for (auto &i : players_) {
          boost::hash_combine(seed, i);
        }
        for (auto &i : numberOfMoves_) {
          boost::hash_combine(seed, i);
        }
        boost::hash_combine(seed, prob_);
        boost::hash_combine(seed, currentPlayer_);
        boost::hash_combine(seed, currentNOM_);
        return seed;
      }


    private:
        vector<int> numberOfMoves_;
        vector<int> players_;
        int currentNOM_;
        int currentPlayer_;
    };


/**
 * ObsPursuitState is a class that represents pursuit states,
 * which contains eight surrounding, moves,
 * vector of locations of all players and state probability.
 * A difference is that it uses PursuitObservationLoc.
 */
    class ObsPursuitState : public PursuitState {
    public:
        // Constructor
        explicit ObsPursuitState(Domain* domain, const vector<Pos> &p);

        // Constructor
        ObsPursuitState(Domain* domain, const vector<Pos> &p, double prob);

        OutcomeDistribution
        performActions(const vector<pair<int, shared_ptr<Action>>> &actions2) const override;
    };

/**
 * PursuitDomain is a class that represents pursuit domain,
 * which contain static height and static width.
 */
    class PursuitDomain : public Domain {
    public:
        // constructor
        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                      const vector<Pos> &loc, int height, int width);

        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                    const vector<Pos> &loc, int height, int width, vector<double> probability);

        // constructor
        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                      const shared_ptr<MMPursuitState> &state, int height, int width);

        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                    const shared_ptr<MMPursuitState> &state, int height, int width, vector<double> probability);

        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                      const shared_ptr<ObsPursuitState> &state, int height, int width);
        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                    const shared_ptr<ObsPursuitState> &state, int height, int width, vector<double> probability);

        // constructor
        PursuitDomain(unsigned int max, int height, int width);

        // destructor
        ~PursuitDomain() override = default;

        // GetInfo returns string containing domain information.
        string getInfo() const final;

        vector<int> getPlayers() const final;

        vector<double> probability;  // probability of stay or move
        int height;
        int width;

    };

/**
 * PursuitDomainChance is a class that represents pursuit domain,
 * it starts with a ChanceNode, so it can have more first states.
 */
    class PursuitDomainChance : public PursuitDomain {
    public:
        // constructor
        PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                            const vector<Pos> &firstPlayerLocation,
                            const vector<Pos> &secondPlayerLocation,
                            int height, int width);

        PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                            const vector<Pos> &firstPlayerLocation,
                            const vector<Pos> &secondPlayerLocation,
                            int height, int width, vector<double> probability);

        // constructor
        PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                            const shared_ptr<MMPursuitState> &state,
                            int height, int width);

        PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                            const shared_ptr<MMPursuitState> &state,
                            int height, int width, vector<double> probability);

    };
}


#endif  // PURSUIT_H_

#pragma clang diagnostic pop
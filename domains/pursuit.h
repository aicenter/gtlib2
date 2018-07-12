//
// Created by rozliv on 02.08.2017.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_H_
#define PURSUIT_H_

#include <utility>
#include <numeric>

#include "../base/base.h"
#include "../base/efg.h"

using std::string;

using namespace GTLib2;

namespace GTLib2 {

    struct Pos {
        int y;
        int x;
    };

// moves description
    const vector<string> movedes_ = {"stay", "right", "down", "left", "up"};

// eight surrounding description
    const vector<string> eightdes_ = {"nowhere", "top left", "top", "top right",
                                      "left", "right", "bottom left", "bottom",
                                      "bottom right", "same"};

    enum EightSurrounding {
        NOWHERE, TOP_LEFT, TOP_CENTER, TOP_RIGHT, LEFT, RIGHT, BOTTOM_LEFT,
        BOTTOM_CENTER, BOTTOM_RIGHT, SAME
    }; // MooreNeighborhood

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
        explicit PursuitState(const vector<Pos> &p);

        // Constructor
        PursuitState(const vector<Pos> &p, double prob);

        // Destructor
        ~PursuitState() override = default;

        // GetActions returns possible actions for a player in the state.
        vector<shared_ptr<Action>> getAvailableActionsFor(int player) const override;

        OutcomeDistribution
        performActions(const unordered_map<int, shared_ptr<Action>> &actions) const override;

        // GetNumPlayers returns number of players who can play in this state.
        inline int getNumberOfPlayers() const override {
          return place_.size();
        }

        inline vector<int> getPlayers() const final {
          return players_;
        }

        // AddString adds string s to a string in vector of strings. //TODO: vyresit jinak -> AddString deprecated
        inline void AddString(const string &s, int player) override {
          strings_[player].append(s);
        }

        // ToString returns state description.
        inline string toString(int player) const override {
          return "player: " + to_string(player) + ", location: " +
                 to_string(place_[player].x) + " " + to_string(place_[player].y) +
                 strings_[player] + "\n";
        }

    protected:
        vector<Pos> place_;  // locations of all players
        double prob_ = 1;  // state probability
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
        vector<double> probdis_ = {0.1, 0.9};  // TODO(rozlijak): temporary
        vector<string> strings_;
        vector<int> players_;
    };


/**
 * MMPursuitState is a class that represents multiple-move pursuit states,
 * which contains vector of all players and a move count of player on turn.
 */
    class MMPursuitState : public PursuitState {
    public:
        // Constructor
        MMPursuitState(const vector<Pos> &p, const vector<int> &players,
                       vector<int> numberOfMoves);

        // Constructor
        MMPursuitState(const vector<Pos> &p, const vector<int> &players,
                       vector<int> numberOfMoves, int currentNOM, int currentPlayer);

        // Constructor
        MMPursuitState(const vector<Pos> &p, double prob,
                       const vector<int> &players, vector<int> numberOfMoves);

        // Constructor
        MMPursuitState(const vector<Pos> &p, double prob, const vector<int> &players,
                       vector<int> numberOfMoves, int currentNOM, int currentPlayer);

        OutcomeDistribution
        performActions(const unordered_map<int, shared_ptr<Action>> &actions) const override;

        // GetNumPlayers returns number of players who can play in this state.
        inline int getNumberOfPlayers() const override {
          return players_.size();
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
        explicit ObsPursuitState(const vector<Pos> &p);

        // Constructor
        ObsPursuitState(const vector<Pos> &p, double prob);

        OutcomeDistribution
        performActions(const unordered_map<int, shared_ptr<Action>> &actions) const override;
    };

/**
 * PursuitDomain is a class that represents pursuit domain,
 * which contain static height and static width.  */
    class PursuitDomain : public Domain {
    public:
        // constructor
        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                      const vector<Pos> &loc, int height, int width);

        // constructor
        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                      const shared_ptr<MMPursuitState> &state, int height, int width);

        PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                      const shared_ptr<ObsPursuitState> &state, int height, int width);

        // constructor
        PursuitDomain(unsigned int max, int height, int width);

        // destructor
        ~PursuitDomain() override = default;

        // GetInfo returns string containing domain information.
        string getInfo() const final;

        vector<int> getPlayers() const final;

        static int height_;
        static int width_;
    protected:
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
                            int height, int weight);

        // constructor
        PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                            const shared_ptr<MMPursuitState> &state,
                            int height, int weight);

    };

}
// Domain independent treewalk algorithm
[[deprecated]]
    void Pursuit(const shared_ptr<Domain> &domain, State *state,
                 unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
[[deprecated]]
    void PursuitStart(const shared_ptr<Domain> &domain, unsigned int depth = 0);


#endif  // PURSUIT_H_

#pragma clang diagnostic pop
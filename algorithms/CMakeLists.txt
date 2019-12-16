add_library(algorithms OBJECT
        bestResponse.h
        bestResponse.cpp
        cfr.h
        cfr.cpp
        cfr_cr.cpp cfr_cr.h
        common.h
        common.cpp
        continualResolving.h
        equilibrium.h
        equilibrium.cpp
        evaluation.cpp evaluation.h
        mccr.h
        mccr.cpp
        normalFormLP.h
        normalFormLP.cpp
        oos.h
        oos.cpp
        stats.h
        stats.cpp
        strategy.cpp strategy.h
        utility.h
        utility.cpp
        MCTS/ISMCTS.h
        MCTS/ISMCTS.cpp
        MCTS/ISMCTS_settings.h
        MCTS/CPW_ISMCTS.h
        MCTS/CPW_ISMCTS.cpp
        MCTS/selectors/selector.h
        MCTS/selectors/selectorFactory.h
        MCTS/selectors/UCTSelector.h
        MCTS/selectors/UCTSelector.cpp
        MCTS/selectors/UCTSelectorFactory.h
        MCTS/selectors/UCTSelectorFactory.cpp
        MCTS/selectors/RMSelector.h
        MCTS/selectors/RMSelector.cpp
        MCTS/selectors/RMSelectorFactory.h
        MCTS/selectors/RMSelectorFactory.cpp
        MCTS/selectors/Exp3Selector.h
        MCTS/selectors/Exp3Selector.cpp
        MCTS/selectors/Exp3LSelector.h
        MCTS/selectors/Exp3LSelector.cpp
        MCTS/selectors/Exp3SelectorFactory.h
        MCTS/selectors/Exp3SelectorFactory.cpp
        )
target_include_directories(algorithms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


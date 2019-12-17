add_library(domains OBJECT
        chessboard_factory.h
        chessboard_factory.cpp
        gambit.h
        gambit.cpp
        genericPoker.h
        genericPoker.cpp
        goofSpiel.h
        goofSpiel.cpp
        kriegspiel.h
        kriegspiel.cpp
        matching_pennies.h
        matching_pennies.cpp
        normal_form_game.cpp normal_form_game.h
        simple_games.h
        liarsDice.h
        liarsDice.cpp
        phantomTTT.h
        phantomTTT.cpp
        pursuit.h
        pursuit.cpp
        RhodeIslandPoker.h
        RhodeIslandPoker.cpp
        oshiZumo.h
        oshiZumo.cpp
        randomGame.h
        randomGame.cpp
        stratego.h
        stratego.cpp
        )
target_include_directories (domains PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_FILES ${TEST_FILES}
        domains/domainsTest.h
        domains/domainsTest.cpp
        domains/gambitTest.cpp
        domains/genericPokerTest.cpp
        domains/goofSpielTest.cpp
        domains/kriegSpielTest.cpp
        domains/liarsDiceTest.cpp
        domains/matchingPenniesTest.cpp
        domains/normalFormGameTest.cpp
        domains/oshiZumoTest.cpp
        domains/randomGameTest.cpp
        domains/strategoTest.cpp
        PARENT_SCOPE)

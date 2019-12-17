add_library(cli OBJECT
        benchmark_cfr.h
        calc_expl.h
        export_domain.h
        export_ring.h
        cfr_regrets.h
        play_match.h
        cli_helpers.h
        global_args.h
        )
target_include_directories(cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

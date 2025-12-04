add_executable( EngineNetTest main.c cwalk.c )
target_include_directories(EngineNetTest PRIVATE .)
target_include_directories(EngineNetTest PRIVATE ../engine/src/vendor/netcode-1.2.3)
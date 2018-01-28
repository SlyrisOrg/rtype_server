set(GAME_SERV ${CMAKE_SOURCE_DIR}/Project/rtype/gameserver/)

set(GAME_SERV_HEADERS
        ${GAME_SERV}/ConfigManager.hpp
        ${GAME_SERV}/ConfigManager.cpp
        ${GAME_SERV}/IOThread.hpp
        ${GAME_SERV}/ServerIOThread.hpp
        ${GAME_SERV}/Server.hpp
        PARENT_SCOPE)

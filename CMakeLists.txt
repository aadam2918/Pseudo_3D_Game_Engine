cmake_minimum_required(VERSION 3.28)
project (2DGame VERSION 1.0)

find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.c src/game.c)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2_mixer SDL2_image SDL2_ttf m)
target_include_directories(${PROJECT_NAME} PUBLIC include/)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/util/"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/util/"
    COMMENT "Copying util directory to executable location")
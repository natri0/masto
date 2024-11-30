project(Masto)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DBUS REQUIRED dbus-1)

add_executable(masto main.cpp platform.cpp
        textfield.cpp
        mastoapi.cpp)

target_link_libraries(masto PRIVATE CURL::libcurl nlohmann_json::nlohmann_json ${DBUS_LIBRARIES} raylib_static)
target_include_directories(masto PRIVATE ${DBUS_INCLUDE_DIRS})

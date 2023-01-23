include(FetchContent)

FetchContent_Declare(
        aquarium-lionfish
        GIT_REPOSITORY https://github.com/pretore/aquarium-lionfish.git
        GIT_TAG v1.0.0
        GIT_SHALLOW 1
)

FetchContent_MakeAvailable(aquarium-lionfish)

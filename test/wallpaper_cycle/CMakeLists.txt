add_executable(WallpaperCycleTest
  WallpaperCycleTest.cpp
)

target_include_directories(WallpaperCycleTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(WallpaperCycleTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(WallpaperCycleTest)

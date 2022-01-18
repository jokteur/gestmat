
add_executable(item_test item_test.cpp)
target_link_libraries(item_test PRIVATE Tempo ${Python_LIBRARIES} ${PROJECT_NAME}_lib)
target_include_directories(Tempo PUBLIC "${PROJECT_SOURCE_DIR}/vendors/TempoApp/include")
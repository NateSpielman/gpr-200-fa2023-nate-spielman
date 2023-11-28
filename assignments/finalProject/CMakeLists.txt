#Transformations example

file(
 GLOB_RECURSE FINAL_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE FINAL_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment7_lighting's asset folder to bin when it is built
add_custom_target(copyAssetsFinal ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${FINAL_INC} DESTINATION include/finalProject)
add_executable(finalProject ${FINAL_SRC} ${FINAL_INC} ${FINAL_ASSETS})
target_link_libraries(finalProject PUBLIC core IMGUI)
target_include_directories(finalProject PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment7_lighting is built
add_dependencies(finalProject copyAssetsFinal)
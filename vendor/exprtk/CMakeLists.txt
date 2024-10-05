set(EXPRTK_SOURCE_DIR "./Source")
set(EXPRTK_INCLUDE_DIR "./Include")
add_library(exprtk STATIC ${EXPRTK_SOURCE_DIR}/exprtk_wrapper.cpp
                          ${EXPRTK_INCLUDE_DIR}/exprtk_wrapper.h
                          ${EXPRTK_INCLUDE_DIR}/exprtk.hpp
                          )

target_include_directories(exprtk PUBLIC ${EXPRTK_INCLUDE_DIR})
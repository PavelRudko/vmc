cmake_minimum_required(VERSION 3.10)

project(vmc LANGUAGES C CXX)

set(VMC_FILES
    main.cpp)

set(VMC_VK_FILES
    vk/VulkanInstance.h
    vk/VulkanDevice.h
    vk/VulkanSwapchain.h
    vk/VulkanInstance.cpp
    vk/VulkanDevice.cpp
    vk/VulkanSwapchain.cpp)

set(VMC_COMMON_FILES
    common/Log.h
    common/Utils.h
    common/Log.cpp
    common/Utils.cpp)

source_group("\\" FILES ${VMC_FILES})
source_group("vk\\" FILES ${VMC_VK_FILES})
source_group("common\\" FILES ${VMC_COMMON_FILES})

add_executable(vmc 
    ${VMC_FILES} 
    ${VMC_VK_FILES} 
    ${VMC_COMMON_FILES})

target_link_libraries(vmc
    volk
    glm
    glfw
    vma)

target_compile_definitions(vmc PUBLIC NOMINMAX)

target_include_directories(vmc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    set_property(TARGET vmc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vmc)
endif()

foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG_TYPE} SUFFIX)
    string(TOLOWER ${CONFIG_TYPE} CONFIG_DIR)
    set_target_properties(vmc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${SUFFIX} ${CMAKE_BINARY_DIR}/bin/${CONFIG_DIR})
endforeach()
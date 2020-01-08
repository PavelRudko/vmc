cmake_minimum_required(VERSION 3.10)

project(vmc LANGUAGES C CXX)

set(VMC_FILES
    main.cpp)

set(VMC_VK_FILES
    vk/VulkanInstance.h
    vk/VulkanDevice.h
    vk/Swapchain.h
    vk/ShaderModule.h
    vk/VulkanBuffer.h
    vk/StagingManager.h
    vk/DescriptorPool.h
    vk/DynamicUniform.h
    vk/DescriptorSetLayout.h
    vk/VulkanInstance.cpp
    vk/VulkanDevice.cpp
    vk/Swapchain.cpp
    vk/ShaderModule.cpp
    vk/VulkanBuffer.cpp
    vk/StagingManager.cpp
    vk/DescriptorPool.cpp
    vk/DynamicUniform.cpp
    vk/DescriptorSetLayout.cpp)

set(VMC_COMMON_FILES
    common/Log.h
    common/Utils.h
    common/Log.cpp
    common/Utils.cpp)

set(VMC_CORE_FILES
    core/Application.h
    core/Window.h
    core/View.h
    core/GameView.h
    core/Application.cpp
    core/Window.cpp
    core/View.cpp
    core/GameView.cpp)

set(VMC_RENDERING_FILES
    rendering/RenderContext.h
    rendering/RenderPass.h
    rendering/RenderPipeline.h
    rendering/RenderContext.cpp
    rendering/RenderPass.cpp
    rendering/RenderPipeline.cpp)

set(VMC_SHADER_FILES
    shaders/default.vert
    shaders/default.frag)

source_group("\\" FILES ${VMC_FILES})
source_group("vk\\" FILES ${VMC_VK_FILES})
source_group("common\\" FILES ${VMC_COMMON_FILES})
source_group("core\\" FILES ${VMC_CORE_FILES})
source_group("rendering\\" FILES ${VMC_RENDERING_FILES})
source_group("shaders\\" FILES ${VMC_SHADER_FILES})

add_executable(vmc 
    ${VMC_FILES} 
    ${VMC_VK_FILES} 
    ${VMC_COMMON_FILES}
    ${VMC_CORE_FILES}
    ${VMC_RENDERING_FILES}
    ${VMC_SHADER_FILES})

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
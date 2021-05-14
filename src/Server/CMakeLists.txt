
find_package(CUDA REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
list(
        APPEND
        SERVER_SRCS
        main.cpp
        NeuronAnnotaterApplication.cpp
        RequestHandlerFactory.cpp
        MyHTTPRequestHandler.cpp
        UploadRequestHandler.cpp
        DownloadRequestHandler.cpp
        WebSocketRequestHandler.cpp
        VolumeRenderer.cpp
        AnnotationDS.cpp
        Image.cpp
        DataBase.cpp
)

# add_library( AnnotationDS SHARED AnnotationDS.cpp )
# add_library( DataBase SHARED DataBase.cpp )

add_executable(NeuronAnnotateServer ${SERVER_SRCS})
target_include_directories(NeuronAnnotateServer PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party)
target_link_libraries(NeuronAnnotateServer PRIVATE
        Poco::Net
        Poco::Util
        Poco::JSON
        Poco::Foundation
        Poco::MongoDB
        spdlog::spdlog
        Threads::Threads
        glm::glm
        seria::seria
        ${OPENGL_LIBRARIES}
        ${CUDA_CUDA_LIBRARY}
        ${CMAKE_DL_LIBS}
        )

#ifndef NEURONANNOTATION_LINESRENDERER_H
#define NEURONANNOTATION_LINESRENDERER_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SWCP.hpp>
#include <UserInfo.hpp>
#ifdef _WINDOWS
#include <windows.h>
#endif

#include <Common/utils.hpp>
#include <unordered_set>
#include <Common/boundingbox.hpp>
#include <Common/help_cuda.hpp>

#include "ShaderProgram.hpp"

class LinesRenderer final: public IRenderer{
public:
    LinesRenderer(int w=1200,int h=900);
    LinesRenderer(const LinesRenderer&)=delete;
    LinesRenderer(LinesRenderer&&)=delete;
    ~LinesRenderer();

private:
    int SCR_WIDTH;
    int SCR_HEIGHT;

public:
    void set_volume(const char* path) override;
    void set_camera(Camera camera) noexcept override;
    void set_transferfunc(TransferFunction tf) noexcept override;
    void set_mousekeyevent(MouseKeyEvent event) noexcept override;
    void render_frame() override;
    auto get_frame()->const Image& override;
    void clear_scene() override;

private:
    std::unique_ptr<sv::Shader> raycasting_shader;
public:
    void createGLShader();
    void initResourceContext();
    void initGL();
    void initCUDA();
private:
    uint32_t window_width,window_height;
    sv::OBB view_obb;

	glm::mat4 viewport;
	glm::mat4 projection;
	glm::mat4 model;

private: //用户相关
    Camera camera;
    Tools selected_tool;
    long selected_line_index;
    long selected_vertex_index;
    UserInfo *cur_user_info;
    NeuronTree *neuron;
private:    
    CUcontext cu_context;

#ifdef _WINDOWS
    HDC window_handle;
    HGLRC gl_context;
#elif LINUX

#endif
};
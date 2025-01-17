#include"LinesRenderer.hpp"
#include<Camera.hpp>

#ifdef _WINDOWS
#include <Common/wgl_wrap.hpp>
#define WGL_NV_gpu_affinity
#else
#include<EGL/egl.h>
#include<EGL/eglext.h>
const EGLint egl_config_attribs[] = {EGL_SURFACE_TYPE,
                                     EGL_PBUFFER_BIT,
                                     EGL_BLUE_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_RED_SIZE,
                                     8,
                                     EGL_DEPTH_SIZE,
                                     8,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_BIT,
                                     EGL_NONE};



void EGLCheck(const char *fn) {
    EGLint error = eglGetError();

    if (error != EGL_SUCCESS) {
        throw runtime_error(fn + to_string(error));
    }
}
#endif

#include <cudaGL.h>

#include <Common/transferfunction_impl.h>
#include <Common/help_gl.hpp>
#include <random>
#include <SWCP.hpp>
#include <list>

void LinesRenderer::createGLShader() {
    raycasting_shader=std::make_unique<sv::Shader>("../../src/Render/Shaders/markedpath_v.glsl",
                                                   "../../src/Render/Shaders/markedpath_f.glsl");
}

LinesRenderer::LinesRenderer(int w=1200,int h=900)
:window_width(w),window_height(h)
{
    if(w>2048 || h>2048 || w<1 || h<1){
        throw std::runtime_error("bad width or height");
    }
    initResourceContext();
    setupSystemInfo();
}

void LinesRenderer::initResourceContext() {
    initGL();
    initCUDA();
}

void LinesRenderer::initGL() {
#ifdef _WINDOWS
    auto ins=GetModuleHandle(NULL);
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f);
    std::string idx=std::to_string(dist(rng));
    HWND window=create_window(ins,("wgl_invisable"+idx).c_str(),window_width,window_height);
    this->window_handle=GetDC(window);
    this->gl_context=create_opengl_context(this->window_handle);
#else
    static const int MAX_DEVICES = 4;
    EGLDeviceEXT egl_devices[MAX_DEVICES];
    EGLint num_devices;

    auto eglQueryDevicesEXT =
            (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
    eglQueryDevicesEXT(4, egl_devices, &num_devices);

    auto eglGetPlatformDisplayEXT =
            (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress(
                    "eglGetPlatformDisplayEXT");

    auto m_egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
                                                  egl_devices[0], nullptr);
    EGLCheck("eglGetDisplay");

    EGLint major, minor;
    eglInitialize(m_egl_display, &major, &minor);
    EGLCheck("eglInitialize");

    EGLint num_configs;
    EGLConfig egl_config;

    eglChooseConfig(m_egl_display, egl_config_attribs, &egl_config, 1,
                    &num_configs);
    EGLCheck("eglChooseConfig");

    const EGLint pbuffer_attribs[] = {
            EGL_WIDTH, (EGLint)window_width, EGL_HEIGHT, (EGLint)window_height, EGL_NONE,
    };
    EGLSurface egl_surface =
            eglCreatePbufferSurface(m_egl_display, egl_config, pbuffer_attribs);
    EGLCheck("eglCreatePbufferSurface");

    eglBindAPI(EGL_OPENGL_API);
    EGLCheck("eglBindAPI");

    const EGLint context_attri[] = {EGL_CONTEXT_MAJOR_VERSION,
                                    4,
                                    EGL_CONTEXT_MINOR_VERSION,
                                    6,
                                    EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                    EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                    EGL_NONE};
    EGLContext egl_context = eglCreateContext(m_egl_display, egl_config,
                                              EGL_NO_CONTEXT, context_attri);
    EGLCheck("eglCreateContext");

    eglMakeCurrent(m_egl_display, egl_surface, egl_surface, egl_context);
    EGLCheck("eglMakeCurrent");

    if (!gladLoadGLLoader((void *(*)(const char *))(&eglGetProcAddress))) {
        throw runtime_error("Failed to load gl");
    }

    std::cout << "OpenGLVolumeRenderer: Detected " << std::to_string(num_devices)
              << " devices, using first one, OpenGL "
                 "version "
              << std::to_string(GLVersion.major) << "."
              << std::to_string(GLVersion.minor) << std::endl;
    glEnable(GL_DEPTH_TEST);

#endif
}


void LinesRenderer::setupShaderUniform(){
    raycasting_shader->setInt("window_width",window_width);
    raycasting_shader->setInt("window_height",window_height);
    glm::vec3 camera_pos={camera.pos[0],camera.pos[1],camera.pos[2]};
    glm::vec3 view_direction={camera.front[0],camera.front[1],camera.front[2]};
    glm::vec3 up={camera.up[0],camera.up[1],camera.up[2]};
    glm::vec3 right=glm::normalize(glm::cross(view_direction,up));
    raycasting_shader->setVec3("camera_pos",camera_pos);
    raycasting_shader->setVec3("view_pos",camera_pos+view_direction*camera.n);
    raycasting_shader->setVec3("view_direction",view_direction);
    raycasting_shader->setVec3("view_right",right);
    raycasting_shader->setVec3("view_up",up);
    raycasting_shader->setFloat("view_depth",camera.f);
    float space=camera.f*tanf(glm::radians(camera.zoom/2))*2/window_height;
    raycasting_shader->setFloat("view_right_space",space);
    raycasting_shader->setFloat("view_up_space",space);
    raycasting_shader->setFloat("step",1.f);
}


void LinesRenderer::set_camera(Camera camera) noexcept {
    this->camera=camera;
    glm::vec3 camera_pos={camera.pos[0],camera.pos[1],camera.pos[2]};
    glm::vec3 view_direction={camera.front[0],camera.front[1],camera.front[2]};
    glm::vec3 up={camera.up[0],camera.up[1],camera.up[2]};
    glm::vec3 right=glm::normalize(glm::cross(view_direction,up));
    auto center_pos=camera_pos+view_direction*(camera.f+camera.n)/2.f;
    this->view_obb=sv::OBB(center_pos,right,up,view_direction,
                           camera.f*tanf(glm::radians(camera.zoom/2))*window_width/window_height,
                           camera.f*tanf(glm::radians(camera.zoom/2)) ,
                           (camera.f-camera.n)/2.f);
    auto fov =
      2 * atan(tan(45.0f * glm::pi<float>() / 180.0f / 2.0f) / camera.zoom);
    this->projection = glm::perspective(
      (double)fov, (double)SCR_WIDTH / (double)SCR_HEIGHT, 0.0001, 5.0);
    this->model = glm::mat4(1.0f);
    this->viewport = glm::lookAt(camera_pos,view_direction,up);
}

//找到屏幕上拾取的一点距离最近的已标注的点
long LinesRenderer::findNearestNeuronNode_WinXY(int cx, int cy, NeuronTree * ptree, double &best_dist) //find the nearest node in a neuron in XY project of the display window
{
	if (!ptree) return -1;
	list <NeuronSWC> *p_listneuron = &(ptree->listNeuron);
	if (!p_listneuron) return -1;

	GLdouble px, py, pz, ix, iy, iz;
	long best_ind=-1; best_dist=-1;
	for (long i=0;i<p_listneuron->size();i++)
	{
		ix = p_listneuron->at(i).x, iy = p_listneuron->at(i).y, iz = p_listneuron->at(i).z;
		GLint res = gluProject(ix, iy, iz, model, projection, viewport, &px, &py, &pz);// note: should use the saved modelview,projection and viewport matrix
		py = viewport[3]-py; //the Y axis is reversed
		if (res==GL_FALSE) {
           std::cout <<"gluProject() fails for NeuronTree [" << i << "] node";
           return -1;
        }
		double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);

#ifdef _NEURON_ASSEMBLER_
		if (cur_dist < this->radius * this->radius) this->indices.insert(i);
#endif

		if (i==0) {	best_dist = cur_dist; best_ind=0; }
		else 
		{	
			if (cur_dist<best_dist) 
			{
				best_dist=cur_dist; 
				best_ind = i;
			}
		}
	}

	return best_ind; 
}

void LinesRenderer::initCUDA(){
    CUDA_DRIVER_API_CALL(cuInit(0));
    CUdevice cuDevice=0;
    CUDA_DRIVER_API_CALL(cuDeviceGet(&cuDevice, 0));
    CUDA_DRIVER_API_CALL(cuCtxCreate(&cu_context,0,cuDevice));
}

void LinesRenderer::render_frame(){
    raycasting_shader->use();
    raycasting_shader->setMat4("MVPMatrix", model*projection*viewport);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.5,0.5,0);
    for (int i = 0; i < marking_maps.size() ; i ++ ){
        if(marking_maps[marking_map_index].IsVisible() ){
            glLineWidth(3);
            glBindVertexArray(marking_maps[i].vao);
            glDrawElements(GL_LINES, 2 * (marking_maps[i].getVexNum() - 1),
                            GL_UNSIGNED_INT, nullptr);
        }
    }
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
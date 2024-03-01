#include "UStarForgeApplication.hpp"
#include "UStarForgeContext.hpp"
#include "UInput.hpp"
#include "UCamera.hpp"
#include "../lib/glfw/deps/glad/gl.h"

#include <J3D/Material/J3DUniformBufferObject.hpp>
#include <J3D/Picking/J3DPicking.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "ResUtil.hpp"

void DealWithGLErrors(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cout << "GL CALLBACK: " << message << std::endl;
}

static UStarForgeContext* ResizeContext = nullptr;

void HandleFramebufferResize(GLFWwindow* window, int w, int h){
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	if(ResizeContext != nullptr){
		ResizeContext->GetCamera()->SetWindowSize(width, height);
		J3D::Picking::ResizeFramebuffer(width, height);
	}
}

UStarForgeApplication::UStarForgeApplication() {
	mWindow = nullptr;
	mContext = nullptr;
}

bool UStarForgeApplication::Setup() {
	// Initialize GLFW
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_SAMPLES, 4);

	mWindow = glfwCreateWindow(1280, 720, "StarForge", nullptr, nullptr);
	if (mWindow == nullptr) {
		glfwTerminate();
		return false;
	}

	UInput::SetWindow(mWindow);

	glfwSetKeyCallback(mWindow, UInput::GLFWKeyCallback);
	glfwSetCursorPosCallback(mWindow, UInput::GLFWMousePositionCallback);
	glfwSetMouseButtonCallback(mWindow, UInput::GLFWMouseButtonCallback);
	glfwSetScrollCallback(mWindow, UInput::GLFWMouseScrollCallback);
	glfwSetFramebufferSizeCallback(mWindow, HandleFramebufferResize);

	glfwMakeContextCurrent(mWindow);
	gladLoadGL(glfwGetProcAddress);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glfwSwapInterval(1);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DealWithGLErrors, nullptr);

	// Initialize imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_None;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	glEnable(GL_MULTISAMPLE);

	GCResourceManager.Init();
	
	J3D::Picking::InitFramebuffer(1280,720); //resize as a todo

	// Create viewer context
	mContext = new UStarForgeContext();
	ResizeContext = mContext;

	return true;
}

bool UStarForgeApplication::Teardown() {
	J3D::Picking::DestroyFramebuffer();
	J3DUniformBufferObject::DestroyUBO();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	glfwDestroyWindow(mWindow);
	glfwTerminate();

	delete mContext;

	return true;
}

bool UStarForgeApplication::Execute(float deltaTime) {
	// Try to make sure we return an error if anything's fucky
	if (mContext == nullptr || mWindow == nullptr || glfwWindowShouldClose(mWindow))
		return false;

	// Update viewer context
	mContext->Update(deltaTime);
	
	// Begin actual rendering
	glfwMakeContextCurrent(mWindow);
	glfwPollEvents();

	UInput::UpdateInputState();

	// The context renders both the ImGui elements and the background elements.
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Update buffer size
	int width, height;
	glfwGetFramebufferSize(mWindow, &width, &height);
	glViewport(0, 0, width, height);

	glDepthMask(true);
	// Clear buffers
	glClearColor(0.100f, 0.261f, 0.402f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render viewer context
	mContext->Render(deltaTime);
	
	// Render imgui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers
	glfwSwapBuffers(mWindow);

	return true;
}

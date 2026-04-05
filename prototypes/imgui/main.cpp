#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imfilebrowser.h>
#include <cstdio>
#include <string>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Image texture wrapper
struct ImageTexture {
    GLuint texture_id = 0;
    int width = 0;
    int height = 0;

    bool LoadFromFile(const char* path) {
        // Load image data
        int channels;
        unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
        if (!data) {
            fprintf(stderr, "Failed to load image: %s\n", path);
            return false;
        }

        // Generate OpenGL texture
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
        return true;
    }

    void Destroy() {
        if (texture_id) {
            glDeleteTextures(1, &texture_id);
            texture_id = 0;
        }
    }

    bool IsLoaded() const {
        return texture_id != 0;
    }
};

class Application {
public:
    Application() : window_(nullptr), progress_(0.0f), show_about_(false) {}

    bool Initialize(const char* title, int width, int height) {
        // Initialize GLFW
        if (!glfwInit()) {
            fprintf(stderr, "Failed to initialize GLFW\n");
            return false;
        }

        // OpenGL 3.3 Core Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // Create window
        window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window_) {
            fprintf(stderr, "Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            fprintf(stderr, "Failed to initialize GLAD\n");
            glfwDestroyWindow(window_);
            glfwTerminate();
            return false;
        }

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // Enable docking
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup style - dark theme
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // Initialize file dialog
        file_dialog_.SetTitle("Open Image File");
        file_dialog_.SetTypeFilters({".png", ".jpg", ".jpeg", ".bmp", ".tga"});

        return true;
    }

    void Run() {
        while (!glfwWindowShouldClose(window_)) {
            // Poll events
            glfwPollEvents();

            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Build UI
            BuildUI();

            // Update progress animation
            UpdateProgress();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window_, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window_);
        }
    }

    void Shutdown() {
        // Cleanup image texture
        image_texture_.Destroy();

        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup GLFW
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

private:
    GLFWwindow* window_;
    ImGui::FileBrowser file_dialog_;
    ImageTexture image_texture_;
    std::string current_image_path_;
    float progress_;
    bool show_about_;

    void UpdateProgress() {
        // Animate progress bar
        static double last_time = 0.0;
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;

        if (delta_time > 0.016) { // ~60fps update
            progress_ += 0.01f;
            if (progress_ > 1.0f) {
                progress_ = 0.0f;
            }
            last_time = current_time;
        }
    }

    void BuildUI() {
        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    file_dialog_.Open();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    glfwSetWindowShouldClose(window_, true);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    show_about_ = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // File dialog
        file_dialog_.Display();

        if (file_dialog_.HasSelected()) {
            current_image_path_ = file_dialog_.GetSelected().string();

            // Reload image
            image_texture_.Destroy();
            if (image_texture_.LoadFromFile(current_image_path_.c_str())) {
                printf("Loaded image: %s (%dx%d)\n",
                       current_image_path_.c_str(),
                       image_texture_.width,
                       image_texture_.height);
            } else {
                fprintf(stderr, "Failed to load: %s\n", current_image_path_.c_str());
            }

            file_dialog_.ClearSelected();
        }

        // Main content window
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
        ImVec2 display_size;
        int fb_width, fb_height;
        glfwGetFramebufferSize(window_, &fb_width, &fb_height);
        display_size.x = static_cast<float>(fb_width);
        display_size.y = static_cast<float>(fb_height) - ImGui::GetFrameHeight();
        ImGui::SetNextWindowSize(display_size);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("MainWindow", nullptr, window_flags);

        // File open button
        if (ImGui::Button("Open Image...")) {
            file_dialog_.Open();
        }

        ImGui::Separator();

        // Progress bar with animation
        ImGui::Text("Progress:");
        char progress_label[32];
        snprintf(progress_label, sizeof(progress_label), "%.0f%%", progress_ * 100.0f);
        ImGui::ProgressBar(progress_, ImVec2(-1, 0), progress_label);

        ImGui::Separator();

        // Image display
        ImGui::Text("Image Display:");
        if (image_texture_.IsLoaded()) {
            // Calculate display size maintaining aspect ratio
            float max_width = ImGui::GetContentRegionAvail().x - 20;
            float max_height = 400.0f;

            float display_w = static_cast<float>(image_texture_.width);
            float display_h = static_cast<float>(image_texture_.height);

            // Scale to fit
            if (display_w > max_width) {
                float scale = max_width / display_w;
                display_w *= scale;
                display_h *= scale;
            }
            if (display_h > max_height) {
                float scale = max_height / display_h;
                display_w *= scale;
                display_h *= scale;
            }

            ImGui::Image(
                reinterpret_cast<ImTextureID>(static_cast<intptr_t>(image_texture_.texture_id)),
                ImVec2(display_w, display_h)
            );

            ImGui::Text("Size: %dx%d", image_texture_.width, image_texture_.height);
            ImGui::Text("Path: %s", current_image_path_.c_str());
        } else {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 text_pos(
                avail.x * 0.5f - 60,
                100
            );
            ImGui::SetCursorPos(text_pos);
            ImGui::Text("No image loaded");
            ImGui::Text("Click 'Open Image...' to load an image");
        }

        ImGui::End();

        // About dialog
        if (show_about_) {
            ImGui::OpenPopup("About");
        }

        if (ImGui::BeginPopupModal("About", &show_about_, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Image Processing Prototype");
            ImGui::Separator();
            ImGui::Text("Built with Dear ImGui");
            ImGui::Text("OpenGL 3.3 + GLFW");
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                show_about_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
};

int main() {
    Application app;
    if (!app.Initialize("Image Processing Prototype", 1280, 720)) {
        return -1;
    }
    app.Run();
    app.Shutdown();
    return 0;
}

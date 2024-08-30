#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <session.h>
#include <string>
#include <vector>

#include "CSerialPort/SerialPortInfo.h"

using namespace itas109;

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

using CfgValKeyId = cc_ublox::field::CfgValKeyIdCommon::ValueType;

void appendText(const char* str);

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int countRead = 0;

static ImVec2 windowSize = ImVec2(1600, 800);
static int selectedIndex = -1;

static std::vector<SerialPortInfo> availablePortsList = CSerialPortInfo::availablePortInfos();
static ImVector<int> lineOffsets;
static ImGuiTextBuffer textBuffer;

static Session session;

// cc_ublox::field::CfgValKeyIdCommon::ValueType::CFG_MSGOUT_UBX_NAV_POSLLH_UART1

void appendText(const char* str) {
    int old_size = textBuffer.size();

    textBuffer.append(str);

    for (const int new_size = textBuffer.size(); old_size < new_size; old_size++)
        if (textBuffer[old_size] == '\n')
            lineOffsets.push_back(old_size + 1);
}

void openConnection() {
    if (selectedIndex > -1) {
        const char* portName = availablePortsList[selectedIndex].portName;

        session.start(portName);
    }
}

void closeConnection() {
    session.stop();
}

void clearOutput() {
    textBuffer.clear();
    lineOffsets.clear();
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::SetNextWindowSize(windowSize);
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::PushID("port-selector");
            ImGui::PushItemWidth(200.0f);
            if (ImGui::BeginCombo("COM port", selectedIndex > -1 ? availablePortsList[selectedIndex].portName : "", ImGuiComboFlags_None)) {
                const auto optionsLength = availablePortsList.size();
                for (int n = 0; n < optionsLength; n++)
                {
                    const bool is_selected = (selectedIndex == n);
                    if (ImGui::Selectable(availablePortsList[n].portName, is_selected)) {
                        selectedIndex = n;
                    }

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopID();

            ImGui::SameLine();

            if (ImGui::Button("Connect")) {
                openConnection();
            }

            ImGui::SameLine();

            if (ImGui::Button("Disconnect")) {
                closeConnection();
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear Output")) {
                clearOutput();
            }

            ImGui::SameLine();

            if (ImGui::Button("Enable NAV-POSLLH")) {
                session.enableMessage(CfgValKeyId::CFG_MSGOUT_UBX_NAV_POSLLH_UART1);
            }

            ImGui::SameLine();

            if (ImGui::Button("Disable NAV-POSLLH")) {
                session.disableMessage(CfgValKeyId::CFG_MSGOUT_UBX_NAV_POSLLH_UART1);
            }

            ImGui::SameLine();

            if (ImGui::Button("Enable NAV-PVT")) {
                session.enableMessage(CfgValKeyId::CFG_MSGOUT_UBX_NAV_PVT_UART1);
            }

            ImGui::SameLine();

            if (ImGui::Button("Disable NAV-PVT")) {
                session.disableMessage(CfgValKeyId::CFG_MSGOUT_UBX_NAV_PVT_UART1);
            }

            ImGui::SameLine();

            if (ImGui::Button("Enable NAV-SAT")) {
                session.enableMessage(CfgValKeyId::CFG_MSGOUT_UBX_NAV_SAT_UART1);
            }

            ImGui::SameLine();

            if (ImGui::Button("Disable NAV-SAT")) {
                session.disableMessage(CfgValKeyId::CFG_MSGOUT_UBX_NAV_SAT_UART1);
            }

            ImGui::Text("bytes emitted: %d", (int)textBuffer.size());

            const char* buf = textBuffer.begin();
            const char* buf_end = textBuffer.end();
            ImGuiListClipper clipper;
            clipper.Begin(lineOffsets.Size);

            // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            // ImGui::TextUnformatted(buf, buf_end);

            ImGui::PushID("output");
            while (clipper.Step()) {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
                    const char* line_start = buf + lineOffsets[line_no];
                    const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();

            // ImGui::PopStyleVar();

            ImGui::Text("%d lines", lineOffsets.Size);

            ImGui::PopID();

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

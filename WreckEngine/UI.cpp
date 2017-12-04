#include "UI.h"
#include "GLstate.h"
#include "GLError.h"
#include "External.h"
#if defined(_WIN32) || defined(_WIN64)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Event.h"

namespace UI
{
    // NOTE - Shaders and globals are copied from ImGui's GLFW example

    static const char* const ImGuiVertexShader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "   Frag_UV = UV;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0.01, 1);\n"
        "}\n";

    static const char* const ImGuiFragmentShader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    static constexpr GLenum ImGuiDrawType = (sizeof(ImDrawIdx) == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

    static double time = 0.0;

    static GLtexture fontTex;
    static GLprogram shader;

    static GLuniform<GLsampler> texLoc;
    static GLuniform<mat4> projLoc;

    static GLbuffer buffer, elements;
    static GLVAO vao;

    static void GlfwKeyCallback(GLFWwindow*, int key, int, int action, int /*mods*/);
    static void KeyCallback(Event::Handler::param_t e) {
        auto params = e.data.extract<int, int, int, int>();
        GlfwKeyCallback(Window::window, std::get<0>(params), std::get<1>(params), std::get<2>(params), std::get<3>(params));
    };

    static Event::Handler keyHandler = Event::make_handler<Keyboard::KeyHandler>(&KeyCallback);

    /**
     * \brief The implementation for retrieving the clipboard's text for ImGui.
     *
     * \param userData ImGui user data.
     * \return The clipboard's contents.
     */
    static const char* ImGuiGetClipboardImpl(void* userData)
    {
        return glfwGetClipboardString(Window::window);
    }

    /**
     * \brief The implementation for setting the clipboard's text for ImGui.
     *
     * \param userData ImGui user data.
     * \param contents The clipboard's new contents.
     */
    static void ImGuiSetClipboardImpl(void* userData, const char* contents)
    {
        glfwSetClipboardString(Window::window, contents);
    }

    /**
     * \brief Renders the given IMGUI draw data.
     *
     * \param dd The draw data.
     */
    void ImGuiRenderImpl(ImDrawData* dd)
    {
        // NOTE - This function is basically the same as ImGui's GLFW example

        GLsavestate_broad glStateHelper;

        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        auto& io = ImGui::GetIO();
        
        int fb_width  = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
        int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
        if (fb_width == 0 || fb_height == 0)
            return;
        dd->ScaleClipRects(io.DisplayFramebufferScale);

        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
        GLstate<GL_BLEND, GL_ENABLE_BIT>{ true }.apply();
        GLstate<GL_BLEND, GL_BLEND_EQUATION>{ GL_FUNC_ADD }.apply();
        GLstate<GL_BLEND, GL_BLEND_FUNC>{
            GLstate_comp{ GL_SRC_ALPHA }
          , GLstate_comp{ GL_ONE_MINUS_SRC_ALPHA }
        }.apply();

        GLstate<GL_CULL_FACE, GL_ENABLE_BIT>{ false }.apply();
        GLstate<GL_DEPTH_TEST, GL_ENABLE_BIT>{ false }.apply();
        GLstate<GL_SCISSOR_TEST, GL_ENABLE_BIT>{ true }.apply();
        GLstate<GL_BINDING, GL_ACTIVE_TEXTURE>{ GL_TEXTURE0 }.apply();

        // Setup viewport, orthographic projection matrix
        Window::viewport(fb_width, fb_height);

        shader.use();
        texLoc.update(0);
        projLoc.update(glm::ortho(0.f, io.DisplaySize.x, io.DisplaySize.y, 0.f));
        vao.bind();

        for (int n = 0, cmdListCount = dd->CmdListsCount; n < cmdListCount; n++) {
            const auto* cmd_list = dd->CmdLists[n];
            const ImDrawIdx* idx_buffer_offset = nullptr;

            buffer.bind();
            buffer.data((GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), cmd_list->VtxBuffer.Data);

            elements.bind();
            elements.data((GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), cmd_list->IdxBuffer.Data);

            for (int cmd_i = 0, cmdBufferSize = cmd_list->CmdBuffer.Size; cmd_i < cmdBufferSize; cmd_i++) {
                const auto pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback) {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
                else {
                    GLstate<GL_BINDING, GL_TEXTURE_BINDING_2D>{ (GLint)pcmd->TextureId }.apply();
                    GLstate<GL_SCISSOR_TEST, GL_SCISSOR_BOX>{ GLstate_dims{
                        (int)pcmd->ClipRect.x
                      , (int)(fb_height - pcmd->ClipRect.w)
                      , (int)(pcmd->ClipRect.z - pcmd->ClipRect.x)
                      , (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)
                    } }.apply();
                    GL_CHECK(glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, ImGuiDrawType, idx_buffer_offset));
                }
                idx_buffer_offset += pcmd->ElemCount;
            }
        }
    }

    /**
     * \brief Creates the ImGui font texture.
     */
    static bool ImGuiCreateFontTexture()
    {
        // Build texture atlas
        auto& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;

        // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because
        // it is more likely to be compatible with user's existing shaders. If your ImTextureId
        // represent a higher-level concept than just a GL texture id, consider calling
        // GetTexDataAsAlpha8() instead to save on GPU memory.
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        // Upload texture to graphics system
        GLsavestate<GL_BINDING, GL_TEXTURE_BINDING_2D> lastTexture;
        
        fontTex.create(GL_TEXTURE_2D);
        if (!fontTex()) return false;
        
        fontTex.bind();
        fontTex.param(GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        fontTex.set2D<GLubyte>(pixels, width, height);

        // Store our identifier
        io.Fonts->TexID = (void *)(intptr_t)fontTex();

        return true;
    }

    /**
     * \brief The key callback for GLFW.
     */
    static void GlfwKeyCallback(GLFWwindow*, int key, int, int action, int /*mods*/)
    {
        auto& io = ImGui::GetIO();
        io.KeysDown[key] = action != GLFW_RELEASE;

        io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT]   || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT]     || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER]   || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
    }

    /**
     * \brief The text callback for GLFW.
     */
    static void GlfwCharCallback(GLFWwindow*, unsigned int c)
    {
        auto& io = ImGui::GetIO();
        if (c > 0 && c < 0x10000)
            io.AddInputCharacter((ImWchar)c);
    }

    /**
     * \brief Initializes the graphics components of ImGui.
     *
     * \return 
     */
    static bool InitializeGraphics()
    {
        GLsavestate_broad glStateHelper;

        shader.create();
        shader.vertex.create(ImGuiVertexShader, GLshader::Type::Vertex);
        shader.fragment.create(ImGuiFragmentShader, GLshader::Type::Fragment);
        shader.link();

        texLoc  = shader.getUniform<GLsampler>("Texture");
        projLoc = shader.getUniform<mat4>("ProjMtx");

        vao.create();
        vao.bind();

        buffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
        elements.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);

        GLattrarr attrSetup;

        buffer.bind();
        attrSetup.add<vec2>(2);
        attrSetup.add<GLubyte>(4, 0, GL_TRUE);
        attrSetup.apply();

        return ImGuiCreateFontTexture();
    }

    /**
     * \brief Initializes the system components of ImGui.
     *
     * \return True if initialization was successful, otherwise false.
     */
    static bool InitializeSystem()
    {
        // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
        auto& io = ImGui::GetIO();
        io.KeyMap[ImGuiKey_Tab]        = (int) Keyboard::Key::Code::Tab;
        io.KeyMap[ImGuiKey_LeftArrow]  = (int) Keyboard::Key::Code::Left;
        io.KeyMap[ImGuiKey_RightArrow] = (int) Keyboard::Key::Code::Right;
        io.KeyMap[ImGuiKey_UpArrow]    = (int) Keyboard::Key::Code::Up;
        io.KeyMap[ImGuiKey_DownArrow]  = (int) Keyboard::Key::Code::Down;
        io.KeyMap[ImGuiKey_PageUp]     = (int) Keyboard::Key::Code::PageUp;
        io.KeyMap[ImGuiKey_PageDown]   = (int) Keyboard::Key::Code::PageDown;
        io.KeyMap[ImGuiKey_Home]       = (int) Keyboard::Key::Code::Home;
        io.KeyMap[ImGuiKey_End]        = (int) Keyboard::Key::Code::End;
        io.KeyMap[ImGuiKey_Delete]     = (int) Keyboard::Key::Code::Delete;
        io.KeyMap[ImGuiKey_Backspace]  = (int) Keyboard::Key::Code::Backspace;
        io.KeyMap[ImGuiKey_Enter]      = (int) Keyboard::Key::Code::Enter;
        io.KeyMap[ImGuiKey_Escape]     = (int) Keyboard::Key::Code::Escape;
        io.KeyMap[ImGuiKey_A]          = (int) Keyboard::Key::Code::A;
        io.KeyMap[ImGuiKey_C]          = (int) Keyboard::Key::Code::C;
        io.KeyMap[ImGuiKey_V]          = (int) Keyboard::Key::Code::V;
        io.KeyMap[ImGuiKey_X]          = (int) Keyboard::Key::Code::X;
        io.KeyMap[ImGuiKey_Y]          = (int) Keyboard::Key::Code::Y;
        io.KeyMap[ImGuiKey_Z]          = (int) Keyboard::Key::Code::Z;

        // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
        io.RenderDrawListsFn = ImGuiRenderImpl;
        io.SetClipboardTextFn = ImGuiSetClipboardImpl;
        io.GetClipboardTextFn = ImGuiGetClipboardImpl;
        io.ClipboardUserData = Window::window;
#ifdef _WIN32
        io.ImeWindowHandle = glfwGetWin32Window(Window::window);
#endif

        Keyboard::charCallback(GlfwCharCallback);

        return true;
    }

    /**
     * \brief Attempts to initialize the UI system.
     *
     * \return True if the UI system was successfully initialized, otherwise false.
     */
    bool Initialize()
    {
        // Let's attempt to load a better looking font
        auto& io = ImGui::GetIO();
        ImFont* font = nullptr;
        io.Fonts->ClearFonts();
        //font = io.Fonts->AddFontDefault();
        //font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Cousine-Regular.ttf", 15.0f);
        font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/DroidSans.ttf", 12.0f);
        //font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/ProggyClean.ttf", 13.0f);
        //font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/ProggyTiny.ttf", 10.0f);

        if (!font)
        {
            return false;
        }

        return InitializeSystem() && InitializeGraphics();
    }
    
    /**
     * \brief Prepares to draw a new frame's worth of UI.
     */
    void PrepareFrame()
    {
        auto& io = ImGui::GetIO();

        // Setup display size
        io.DisplaySize = Window::size;
        io.DisplayFramebufferScale = Window::frameScale;

        // Setup time step
        double current_time = Time::elapsed;
        io.DeltaTime = time > 0.0 ? (float)(current_time - time) : (float)(1.0f / 60.0f);
        time = current_time;

        // Setup inputs
        // (We've already got mouse wheel, keyboard keys & characters from GLFW callbacks polled in glfwPollEvents())
        if (Window::isInFocus) {
            // Mouse position in screen coordinates
            io.MousePos = ImVec2((float)Mouse::info.currPixel.x, (float)Mouse::info.currPixel.y);
        }
        else {
            io.MousePos = ImVec2(-1, -1);
        }

        for (int i = 0; i < 3; i++) {
            // If a mouse press event came, always pass it as "mouse held this frame", so we don't
            // miss click-release events that are shorter than 1 frame.
            io.MouseDown[i] = Mouse::info.getButtonState(Mouse::Button{ i });
        }

        io.MouseWheel = Mouse::info.wheel.vertical.accum;

        // Hide OS mouse cursor if ImGui is drawing it
        Thread::Main::run([&io] { glfwSetInputMode(Window::window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : Window::cursorMode); });

        // Start the frame
        ImGui::NewFrame();
    }

    /**
     * \brief Draws all of the queued UI.
     */
    void Draw()
    {
        ImGui::Render();
    }
}

#include "UI.h"
#include "gl_structs.h"
#include "GLError.h"
#include "External.h"
#if defined(_WIN32) || defined(_WIN64)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#if defined(offsetof)
#define OFFSETOF(TYPE, ELEMENT) offsetof(TYPE, ELEMENT)
#else
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif

namespace UI
{
    // NOTE - Shaders and globals are copied from ImGui's GLFW example

    static const char* const ImGuiVertexShader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "   Frag_UV = UV;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    static const char* const ImGuiFragmentShader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "   Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    static double   g_Time = 0.0;
    static bool     g_MousePressed[3] = { false, false, false };
    static float    g_MouseWheel = 0.0f;
    static GLuint   g_FontTexture = 0;
    static int      g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
    static int      g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
    static int      g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
    static GLuint   g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

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

        GLstatehelper glStateHelper;

        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        ImGuiIO& io = ImGui::GetIO();
        int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
        int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
        if (fb_width == 0 || fb_height == 0)
            return;
        dd->ScaleClipRects(io.DisplayFramebufferScale);

        // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendEquation(GL_FUNC_ADD));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GL_CHECK(glDisable(GL_CULL_FACE));
        GL_CHECK(glDisable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_SCISSOR_TEST));
        GL_CHECK(glActiveTexture(GL_TEXTURE0));

        // Setup viewport, orthographic projection matrix
        GL_CHECK(glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height));
        const float ortho_projection[4][4] =
        {
            {2.0f / io.DisplaySize.x, 0.0f,                     0.0f, 0.0f},
            {0.0f,                    2.0f / -io.DisplaySize.y, 0.0f, 0.0f},
            {0.0f,                    0.0f,                    -1.0f, 0.0f},
            {-1.0f,                   1.0f,                     0.0f, 1.0f},
        };
        GL_CHECK(glUseProgram(g_ShaderHandle));
        GL_CHECK(glUniform1i(g_AttribLocationTex, 0));
        GL_CHECK(glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]));
        GL_CHECK(glBindVertexArray(g_VaoHandle));

        for (int n = 0; n < dd->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = dd->CmdLists[n];
            const ImDrawIdx* idx_buffer_offset = 0;

            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle));
            GL_CHECK(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW));

            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle));
            GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW));

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback)
                {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
                else
                {
                    GL_CHECK(glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId));
                    GL_CHECK(glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
                    GL_CHECK(glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset));
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
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;

        // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because
        // it is more likely to be compatible with user's existing shaders. If your ImTextureId
        // represent a higher-level concept than just a GL texture id, consider calling
        // GetTexDataAsAlpha8() instead to save on GPU memory.
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        // Upload texture to graphics system
        GLint last_texture;
        GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
        GL_CHECK(glGenTextures(1, &g_FontTexture));
        if (!g_FontTexture) return false;
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, g_FontTexture));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

        // Store our identifier
        io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

        // Restore state
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, last_texture));

        return true;
    }

    /**
     * \brief The mouse button callback for GLFW.
     */
    static void GlfwMouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/)
    {
        if (action == GLFW_PRESS && button >= 0 && button < 3)
            g_MousePressed[button] = true;
    }

    /**
     * \brief The scroll callback for GLFW.
     */
    static void GlfwScrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset)
    {
        g_MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
    }

    /**
     * \brief The key callback for GLFW.
     */
    static void GlfwKeyCallback(GLFWwindow*, int key, int, int action, int mods)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;
        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;

        (void)mods; // Modifiers are not reliable across systems
        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
    }

    /**
     * \brief The text callback for GLFW.
     */
    static void GlfwCharCallback(GLFWwindow*, unsigned int c)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (c > 0 && c < 0x10000)
            io.AddInputCharacter((unsigned short)c);
    }

    /**
     * \brief Initializes the graphics components of ImGui.
     *
     * \return 
     */
    static bool InitializeGraphics()
    {
        GLstatehelper glStateHelper;

        GL_CHECK(g_ShaderHandle = glCreateProgram());
        GL_CHECK(g_VertHandle = glCreateShader(GL_VERTEX_SHADER));
        GL_CHECK(g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER));
        GL_CHECK(glShaderSource(g_VertHandle, 1, &ImGuiVertexShader, 0));
        GL_CHECK(glShaderSource(g_FragHandle, 1, &ImGuiFragmentShader, 0));
        GL_CHECK(glCompileShader(g_VertHandle));
        GL_CHECK(glCompileShader(g_FragHandle));
        GL_CHECK(glAttachShader(g_ShaderHandle, g_VertHandle));
        GL_CHECK(glAttachShader(g_ShaderHandle, g_FragHandle));
        GL_CHECK(glLinkProgram(g_ShaderHandle));

        GL_CHECK(g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture"));
        GL_CHECK(g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx"));
        GL_CHECK(g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position"));
        GL_CHECK(g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV"));
        GL_CHECK(g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color"));

        GL_CHECK(glGenBuffers(1, &g_VboHandle));
        GL_CHECK(glGenBuffers(1, &g_ElementsHandle));

        GL_CHECK(glGenVertexArrays(1, &g_VaoHandle));
        GL_CHECK(glBindVertexArray(g_VaoHandle));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle));
        GL_CHECK(glEnableVertexAttribArray(g_AttribLocationPosition));
        GL_CHECK(glEnableVertexAttribArray(g_AttribLocationUV));
        GL_CHECK(glEnableVertexAttribArray(g_AttribLocationColor));

        GL_CHECK(glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos)));
        GL_CHECK(glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv)));
        GL_CHECK(glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col)));

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
        ImGuiIO& io = ImGui::GetIO();
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

        // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
        io.RenderDrawListsFn = ImGuiRenderImpl;
        io.SetClipboardTextFn = ImGuiSetClipboardImpl;
        io.GetClipboardTextFn = ImGuiGetClipboardImpl;
        io.ClipboardUserData = Window::window;
#ifdef _WIN32
        io.ImeWindowHandle = glfwGetWin32Window(Window::window);
#endif

        glfwSetMouseButtonCallback(Window::window, GlfwMouseButtonCallback);
        glfwSetScrollCallback(Window::window, GlfwScrollCallback);
        glfwSetKeyCallback(Window::window, GlfwKeyCallback);
        glfwSetCharCallback(Window::window, GlfwCharCallback);

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
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->ClearFonts();
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("Assets/Fonts/Cousine-Regular.ttf", 15.0f);
        io.Fonts->AddFontFromFileTTF("Assets/Fonts/DroidSans.ttf", 12.0f);
        //io.Fonts->AddFontFromFileTTF("Assets/Fonts/ProggyClean.ttf", 13.0f);
        //io.Fonts->AddFontFromFileTTF("Assets/Fonts/ProggyTiny.ttf", 10.0f);

        return InitializeSystem() && InitializeGraphics();
    }
    
    /**
     * \brief Prepares to draw a new frame's worth of UI.
     */
    void PrepareFrame()
    {
        ImGuiIO& io = ImGui::GetIO();

        // Setup display size (every frame to accommodate for window resizing)
        int w, h;
        int display_w, display_h;
        glfwGetWindowSize(Window::window, &w, &h);
        glfwGetFramebufferSize(Window::window, &display_w, &display_h);
        io.DisplaySize = ImVec2((float)w, (float)h);
        io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

        // Setup time step
        double current_time = glfwGetTime();
        io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
        g_Time = current_time;

        // Setup inputs
        // (We've already got mouse wheel, keyboard keys & characters from GLFW callbacks polled in glfwPollEvents())
        if (glfwGetWindowAttrib(Window::window, GLFW_FOCUSED))
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(Window::window, &mouse_x, &mouse_y);

            // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
            io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }
        else
        {
            io.MousePos = ImVec2(-1, -1);
        }

        for (int i = 0; i < 3; i++)
        {
            // If a mouse press event came, always pass it as "mouse held this frame", so we don't
            // miss click-release events that are shorter than 1 frame.
            io.MouseDown[i] = g_MousePressed[i] || glfwGetMouseButton(Window::window, i) != 0;
            g_MousePressed[i] = false;
        }

        io.MouseWheel = g_MouseWheel;
        g_MouseWheel = 0.0f;

        // Hide OS mouse cursor if ImGui is drawing it
        glfwSetInputMode(Window::window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : Window::cursorMode);

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

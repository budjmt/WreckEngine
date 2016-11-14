#include "UiTest.h"
#include "UI.h"

struct UiTestEntity : public Entity
{
    ImVec4 clear_color = ImColor(100, 149, 237);
    bool show_test_window = false;
    bool show_another_window = false;

    void draw() override
    {
        UI::PrepareFrame();

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            //ImGui::Begin("Debug");
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Test Window"))
                show_test_window ^= 1;
            if (ImGui::Button("Another Window"))
                show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::End();
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        UI::Draw();
        GL_CHECK(glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
    }
};

struct UiTestState : public State
{
    UiTestState()
        : State("UiTestState")
    {
        addEntity(make_shared<UiTestEntity>());
    }
};

/**
 * \brief Creates a new UI test game.
 */
UiTest::UiTest()
{
    drawDebug = false;
    addState(make_shared<UiTestState>());
}

/**
 * \brief Destroys the UI test game.
 */
UiTest::~UiTest()
{
    ImGui::Shutdown();
}

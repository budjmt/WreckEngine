#include "UiTest.h"
#include "UI.h"

namespace {
    void glob(Event::Handler::param_t e) {
        static uint32_t mouse_button = Event::Message::get("mouse_button");
        if (e.id == mouse_button) {
            printf("Global: Mods %d\n", e.data.peek<int>(2));
        }
    }

    ImVec4 clear_color = ImColor(100, 149, 237);
}

struct UiTestEntity : public Entity
{
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
    }

    void testHandler(Event::Handler::param_t e) {
        static uint32_t mouse_button = Event::Message::get("mouse_button");
        if (e.id == mouse_button) {
            printf("UiTestEntity: Action %s\n", e.data.peek<bool>(1) ? "PRESS" : "RELEASE");
        }
    }

    Event::Handler button_handler = Event::make_handler<Mouse::ButtonHandler>(Event::Handler::wrap_member_func(this, &UiTestEntity::testHandler));
    Event::Handler glob_button_handler = Event::make_handler<Mouse::ButtonHandler>(&glob);
};

/**
 * \brief Creates a new UI test game.
 */
UiTest::UiTest() : Game(1)
{
    drawDebug = false;
    auto s = make_shared<State>("UiTestState");
    s->addEntity(make_shared<UiTestEntity>());
    addState(s);
}

/**
 * \brief Destroys the UI test game.
 */
UiTest::~UiTest()
{
    ImGui::Shutdown();
}

void UiTest::draw() {
    GLframebuffer::setClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    Game::draw();
    GLframebuffer::unbind(GL_FRAMEBUFFER);
    GLframebuffer::clear();
    UI::Draw();
}

void UiTest::testHandler(Event::Handler::param_t e) {
    static uint32_t mouse_button = Event::Message::get("mouse_button");
    if (e.id == mouse_button) {
        printf("UiTest: Button %d\n", e.data.peek<int>(0));
    }
}
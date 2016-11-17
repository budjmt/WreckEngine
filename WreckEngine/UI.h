#pragma once

#include <imgui.h>

#include "gl_structs.h"

namespace UI
{
    /**
     * \brief Attempts to initialize the UI system.
     *
     * \return True if the UI system was successfully initialized, otherwise false.
     */
    bool Initialize();

    /**
     * \brief Prepares to draw a new frame's worth of UI.
     */
    void PrepareFrame();

    /**
     * \brief Draws all of the queued UI.
     */
    void Draw();
}

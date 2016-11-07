#pragma once

#include "Game.h"

/**
 * Defines a UI test game.
 */
class UiTest : public Game
{
public:
    /**
     * \brief Creates a new UI test game.
     */
    UiTest();

    /**
     * \brief Destroys the UI test game.
     */
    ~UiTest();

    /**
     * \brief Updates the UI test.
     *
     * \param dt The time since the last frame.
     */
    void update(double dt) override;

    /**
     * \brief Draws the UI test.
     */
    void draw() override;
};

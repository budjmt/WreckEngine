#pragma once

#include "Game.h"

#include "Event.h"

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

    void draw() override;

    void testHandler(Event::Handler::param_t e);

    Event::Handler button_handler = Event::make_handler<Mouse::ButtonHandler>(Event::Handler::wrap_member_func(this, &UiTest::testHandler));
};

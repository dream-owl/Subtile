#ifndef SUBTILE_SUBUI_H
#define SUBTILE_SUBUI_H

#include "subcore.h"

class stMesh //debug only, proper implementation (particle effects and blended lighting) will be present on separate  client
{
public:
    void reset();
    void line(stVector const& origin, stVector const& target);
    void rect(stVector const& center, stVector const& radius);

    std::vector<stVector> const& vertices() const { return m_vertices; }
    std::vector<uint32_t> const& indices() const { return m_indices; }

private:
    std::vector<stVector> m_vertices;
    std::vector<uint32_t> m_indices;
};

class stUI
{
public:
    stUI();
   ~stUI();

    bool step();
    bool draw(stMesh const& mesh);

    stVector const& screen() const { return m_screen; }
    stVector const& cursor() const { return m_cursor; }

    float delta() const { return m_delta; }
    float stamp() const { return m_stamp; }

    bool press(char code, bool repeat);
    bool click(char code, bool repeat);

private:
    void* m_window;

    stVector m_screen;
    stVector m_cursor;

    float m_delta;
    float m_stamp;

    std::array<bool, 128> m_keys;
    std::array<bool, 3> m_buttons;

    std::unique_ptr<class stGraphics> m_graphics;
};

#endif // SUBTILE_SUBUI_H

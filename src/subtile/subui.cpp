#include "subui.h"
#include "subtile.h"

#include <glad/glad.h>
#include <glad/glad.c>

#include <glfw/glfw3.h>

void stMesh::reset()
{
    m_vertices.clear();
    m_indices.clear();
}

void stMesh::line(stVector const& origin, stVector const& target)
{
    m_indices.push_back(m_vertices.size()+0);
    m_indices.push_back(m_vertices.size()+1);

    m_vertices.push_back(origin);
    m_vertices.push_back(target);
}

void stMesh::rect(stVector const& center, stVector const& radius)
{

}

stUI::stUI() : m_screen(1024.0f, 768.0f) , m_cursor(0.0f) , m_delta(0.0f) , m_stamp(0.0f)
{
    if(glfwInit() != GLFW_TRUE)
        throw stException("Couldn't initialize GLFW libary.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    glfwWindowHint(GLFW_RESIZABLE, 0);

    m_window = glfwCreateWindow(m_screen.x, m_screen.y, "Subtile Project", nullptr, nullptr);

    if(!m_window)
        throw stException("Couldn't create system.");

    glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_window));
    glfwSwapInterval(1);

    std::fill(m_keys.begin(), m_keys.end(), false);
    std::fill(m_buttons.begin(), m_buttons.end(), false);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw stException("Couldn't initialize OpenGL extensions libary.");

    glClearDepth(1.0f);
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    m_stamp = static_cast<float>(glfwGetTime());
}

stUI::~stUI()
{
    if(m_window)
    {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_window));
        glfwTerminate();
    }
}

bool stUI::step()
{
    GLFWwindow* system = static_cast<GLFWwindow*>(m_window);

    float timestamp = static_cast<float>(glfwGetTime());
    m_delta = timestamp - m_stamp;
    m_stamp = timestamp;

    glfwSwapBuffers(system);
    glClear(GL_COLOR_BUFFER_BIT);

    double mx = 0.0f, my = 0.0f;

    glfwPollEvents();
    glfwGetCursorPos(system, &mx, &my);

    m_cursor.x = mx;
    m_cursor.y = my;

    return !glfwWindowShouldClose(system);
}

bool stUI::draw(stMesh const& mesh)
{

}

bool stUI::press(char code, bool repeat)
{
    code = std::toupper(code);

    bool state = glfwGetKey(static_cast<GLFWwindow*>(m_window), code);
    bool first = state && !m_keys[code];

    m_keys[code] = state;

    return repeat ? state : first;
}

bool stUI::click(char code, bool repeat)
{
    code = std::toupper(code);

    switch(code)
    {
        case 'L': code = 0; break;
        case 'M': code = 2; break;
        case 'R': code = 1; break;
        default: return false;
    }

    bool state = glfwGetMouseButton(static_cast<GLFWwindow*>(m_window), static_cast<int>(code));
    bool first = state && !m_buttons[code];

    m_buttons[code] = state;

    return repeat ? state : first;
}

int main()
{
    try
    {
        stUI ui;
        stSubtile os("universe");

        while(ui.step())
        {

        }
    }
    catch(std::exception catched)
    {
        std::cerr << catched.what() << "\n";
    }
}

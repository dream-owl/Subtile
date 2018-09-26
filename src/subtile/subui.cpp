#include "subui.h"
#include "subtile.h"

#include <glad/glad.h>
#include <glad/glad.c>

#include <glfw/glfw3.h>

class stGraphics
{
public:
    stGraphics(stGraphics const&) = delete;
    stGraphics()
    {
        static char const* const VertexSource = {
            "#version 330\n"
            "uniform mat4 uProjection;"
            "in vec2 iPosition;"
            "in vec2 iTexCoord;"
            "out vec2 vTexCoord;"
            "void main() {"
            "   vTexCoord = iTexCoord;"
            "   gl_Position = uProjection * vec4(iPosition.xy, 0.0, 1.0);"
            "}"
        };

        static char const* const FragmentSource = {
            "#version 330\n"
            "in vec2 vTexCoord;"
            "out vec4 oColor;"
            "void main() {"
            "   oColor = vec4(1.0, 0.0, 0.0, 1.0);"
            "}"
        };

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

        glBindVertexArray(0);

        m_vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_vertex, 1, &VertexSource, nullptr);
        glCompileShader(m_vertex);

        m_fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_fragment, 1, &FragmentSource, nullptr);
        glCompileShader(m_fragment);

        m_program = glCreateProgram();
        glAttachShader(m_program, m_vertex);
        glAttachShader(m_program, m_fragment);

        glBindAttribLocation(m_program, 0, "iPosition");
        glBindAttribLocation(m_program, 1, "iTexCoord");

        glBindFragDataLocation(m_program, 0, "oColor");

        glLinkProgram(m_program);
        glUseProgram(m_program);

        m_projectionUniform = glGetUniformLocation(m_program, "uProjection");

        glUseProgram(0);
    }

    void draw(stMesh const& mesh)
    {
        glUseProgram(m_program);
        glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, m_projectionMatrix);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(stVector) * mesh.vertices().size(), mesh.vertices().data(), GL_STREAM_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(stVector), reinterpret_cast<void const*>(0));
        glEnableVertexAttribArray(0);

        glVertexAttrib2f(1, 1.0f, 1.0f);
        glDisableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh.indices().size(), mesh.indices().data(), GL_STREAM_DRAW);

        glDrawElements(GL_LINES, mesh.indices().size(), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void ortho(stVector const& center, stVector radius, float aspect)
    {
        float const left = center.x - radius.x * aspect;
        float const right = center.x + radius.x * aspect;
        float const top = center.y + radius.y;
        float const bottom = center.y - radius.y;

        std::fill(&m_projectionMatrix[0], &m_projectionMatrix[15], 0.0f);

        m_projectionMatrix[0] = 2.0f / (right - left);
        m_projectionMatrix[5] = 2.0f / (top - bottom);
        m_projectionMatrix[10] = -1.0f;
        m_projectionMatrix[12] = (right + left) / (left - right);
        m_projectionMatrix[13] = (top + bottom) / (bottom - top);
        m_projectionMatrix[15] = 1.0f;
    }

private:
    uint32_t m_vao;
    uint32_t m_vbo;
    uint32_t m_ibo;

    uint32_t m_program;
    uint32_t m_vertex;
    uint32_t m_fragment;

    int32_t m_projectionUniform;
    float m_projectionMatrix[16];
};

class stSubtileMesh : public stMesh , public stVisitor
{
public:
    void onTile(stTransform const& transform, stMaterial const& material, stBehavior const& behavior) override
    {
        line(transform.position, stVector(transform.position.x + 0.5f, transform.position.y + 0.5f));
    }
};

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

    m_graphics.reset(new stGraphics());
    m_graphics->ortho(stVector(0.0f), stVector(5.0f), m_screen.x / m_screen.y);

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
    m_graphics->draw(mesh);
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
        os.parse(stRequest(0, 2.0f, -1.0f, 1.0f));

        while(ui.step())
        {
            stSubtileMesh mesh;
            os.visit(mesh, stBounds(0, -2.0f, -2.0f, 0, 2.0f, 2.0f));;
            ui.draw(mesh);
        }
    }
    catch(std::exception const& catched)
    {
        std::cerr << catched.what() << "\n";
    }
}

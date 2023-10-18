#include "window.hpp"
#include <cmath>

void Window::onCreate() {
  auto const *vertexShader{R"gl(#version 300 es

    layout(location = 0) in vec2 inPosition;
    layout(location = 1) in vec4 inColor;

    uniform vec2 translation;
    uniform float scale;

    out vec4 fragColor;

    void main() {
      vec2 newPosition = inPosition * scale + translation;
      gl_Position = vec4(newPosition, 0, 1);
      fragColor = inColor;
    }
  )gl"};

  auto const *fragmentShader{R"gl(#version 300 es

    precision mediump float;

    in vec4 fragColor;

    out vec4 outColor;

    void main() { outColor = fragColor; }
  )gl"};

  m_program = abcg::createOpenGLProgram(
      {{.source = vertexShader, .stage = abcg::ShaderStage::Vertex},
       {.source = fragmentShader, .stage = abcg::ShaderStage::Fragment}});

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glClear(GL_COLOR_BUFFER_BIT);

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());
}

double mapRange(double inValue, double minInRange, double maxInRange, double minOutRange, double maxOutRange){
  double x = (inValue - minInRange) / (maxInRange - minInRange);
  double result = minOutRange + (maxOutRange - minOutRange) * x;

  return result;
}

bool firstExec = true;
bool changeFlagX = true;
bool changeFlagY = true;
float scaleFactor = 0.2f;
float translationX = 0.0f;
float translationY = 0.0f;
float translationModifierX = 0.01f;
float translationModifierY = 0.01f;
double m_delay;

glm::vec3 color1;
glm::vec3 color2;

int colorOption;
int recoilOption;


void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT);
  auto const sides{256};
  std::uniform_real_distribution rd(0.0f, 1.0f);
  std::uniform_real_distribution tm_rd(0.005f, 0.012f);

  //Initialization settings using the firstExec flag;
  if (firstExec){
    color1 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
    color2 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
    translationModifierX = tm_rd(m_randomEngine);
    translationModifierY = tm_rd(m_randomEngine);
    firstExec = false;
  }
  
  //Transform the delay variable using this mapRange function created in this file
  m_delay = round(mapRange(m_speed, 1, 100, 30, 0));

  if (m_timer.elapsed() <= m_delay / 1000.0){

  }
  else{ //Control the translation over the time
    translationX = translationX+translationModifierX;
    translationY = translationY+translationModifierY;
    m_timer.restart();
  }
  
  if (abs(translationX) > 0.8f){ //In case of X border impact, changes the color and the translation direction
    if(changeFlagX){
      if (recoilOption == 1)
        translationModifierX = (translationModifierX / -abs(translationModifierX)) * tm_rd(m_randomEngine);
      else
        translationModifierX *= -1;
      color1 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
      color2 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
      changeFlagX = false; 
    }
  } 
  else if (abs(translationX) < 0.75f) {
    changeFlagX = true;
  }

  if (abs(translationY) > 0.8f){ //In case of Y border impact, changes the color and the translation direction
    if(changeFlagY){
      if (recoilOption == 1)
        translationModifierY = (translationModifierY / -abs(translationModifierY)) * tm_rd(m_randomEngine);
      else
        translationModifierY *= -1;
      color1 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
      color2 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
      changeFlagY = false;
    }
  } 
  else if (abs(translationY) < 0.75f) {
    changeFlagY = true;
  }

  // Create a regular polygon with with a high number os sides, aproximating a circle

  //In case of Single Color option selected
  if (colorOption == 1){
    color2 = color1;
  }

  setupModel(sides, color1, color2);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Use the X and Y positions based on the behavior above
  glm::vec2 const translation{translationX,translationY};
  auto const translationLocation{
  abcg::glGetUniformLocation(m_program, "translation")};
  abcg::glUniform2fv(translationLocation, 1, &translation.x);

  // Keep a single scale factor for every object
  auto const scale{scaleFactor};
  auto const scaleLocation{abcg::glGetUniformLocation(m_program, "scale")};
  abcg::glUniform1f(scaleLocation, scale);

  // Render
  abcg::glBindVertexArray(m_VAO);
  abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    auto const widgetSize{ImVec2(330, 150)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - widgetSize.x - 5,
                                   m_viewportSize.y - widgetSize.y - 5));
    ImGui::SetNextWindowSize(widgetSize);
    auto const windowFlags{ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoTitleBar};
    ImGui::Begin(" ", nullptr, windowFlags);
    ImGui::Text("X: %g Y: %g", translationX, translationY);
    {
      static auto currentIndex{0U};
      std::vector<std::string> const comboItems{"Stable", "Randomized"};

      if (ImGui::BeginCombo("Recoil Mode",
                            comboItems.at(currentIndex).c_str())) {
        for (auto const index : iter::range(comboItems.size())) {
          bool const isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected)){
            currentIndex = index;
            recoilOption = currentIndex;
          }
          // Set the initial focus when opening the combo (scrolling + keyboard
          // navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }
    {
      static auto currentIndex{0U};
      std::vector<std::string> const comboItems{"Mixed Colors", "Single Color"};

      if (ImGui::BeginCombo("Color Mode",
                            comboItems.at(currentIndex).c_str())) {
        for (auto const index : iter::range(comboItems.size())) {
          bool const isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected)){
            currentIndex = index;
            colorOption = currentIndex;
          }
          // Set the initial focus when opening the combo (scrolling + keyboard
          // navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }

    ImGui::PushItemWidth(230);
    ImGui::SliderInt("Speed", &m_speed, 1, 100, "%d %");
    ImGui::PopItemWidth();

    if (ImGui::Button("Restart", ImVec2(-1, 30))) {
      abcg::glClear(GL_COLOR_BUFFER_BIT);
      firstExec = true;
      scaleFactor = 0.2f;
      translationX = 0.0f;
      translationY = 0.0f;
    }

    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
  firstExec = true;
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_VBOPositions);
  abcg::glDeleteBuffers(1, &m_VBOColors);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Window::setupModel(int sides, glm::vec3 color1, glm::vec3 color2) {
  // Release previous resources, if any
  abcg::glDeleteBuffers(1, &m_VBOPositions);
  abcg::glDeleteBuffers(1, &m_VBOColors);
  abcg::glDeleteVertexArrays(1, &m_VAO);

  std::vector<glm::vec2> positions;
  std::vector<glm::vec3> colors;

  // Polygon center
  positions.emplace_back(0, 0);
  colors.push_back(color1);

  // Border vertices
  auto const step{M_PI * 2 / sides};
  for (auto const angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(std::cos(angle), std::sin(angle));
    colors.push_back(color2);
  }

  // Duplicate second vertex
  positions.push_back(positions.at(1));
  colors.push_back(color2);

  // Generate VBO of positions
  abcg::glGenBuffers(1, &m_VBOPositions);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate VBO of colors
  abcg::glGenBuffers(1, &m_VBOColors);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
  abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                     colors.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  auto const colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glEnableVertexAttribArray(colorAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
  abcg::glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}


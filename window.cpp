#include "headers/window.hpp"

#include <SDL_mouse.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void Window::onEvent(SDL_Event const &event) {
  // if (event.type == SDL_KEYDOWN) {
  //   if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
  //     m_dollySpeed = 1.0f;
  //   if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
  //     m_dollySpeed = -1.0f;
  //   if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
  //     m_panSpeed = -1.0f;
  //   if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
  //     m_panSpeed = 1.0f;
  //   if (event.key.keysym.sym == SDLK_q)
  //     m_truckSpeed = -2.0f;
  //   if (event.key.keysym.sym == SDLK_e)
  //     m_truckSpeed = 2.0f;
  // }
  // if (event.type == SDL_KEYUP) {
  //   if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
  //   &&
  //       m_dollySpeed > 0)
  //     m_dollySpeed = 0.0f;
  //   if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
  //   &&
  //       m_dollySpeed < 0)
  //     m_dollySpeed = 0.0f;
  //   if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
  //   &&
  //       m_panSpeed < 0)
  //     m_panSpeed = 0.0f;
  //   if ((event.key.keysym.sym == SDLK_RIGHT ||
  //        event.key.keysym.sym == SDLK_d) &&
  //       m_panSpeed > 0)
  //     m_panSpeed = 0.0f;
  //   if (event.key.keysym.sym == SDLK_q && m_truckSpeed < 0)
  //     m_truckSpeed = 0.0f;
  //   if (event.key.keysym.sym == SDLK_e && m_truckSpeed > 0)
  //     m_truckSpeed = 0.0f;
  // }

  if (event.type == SDL_MOUSEBUTTONUP) {
    if (event.button.button == SDL_BUTTON_LEFT) {
      for (auto &alvo : m_alvos) {
        if (alvo.m_mouseInside) {
          alvo.m_hit = true;
        }
      }
    }
  }
  if (event.type == SDL_MOUSEMOTION) {
    SDL_GetMouseState(&m_mousePosition.x, &m_mousePosition.y);
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  m_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "depth.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "depth.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  m_model.loadObj(assetsPath + "objmodels/geosphere.obj");
  m_model.setupVAO(m_program);

  m_modelSphere.loadObj(assetsPath + "objmodels/sphere.obj");
  m_modelSphere.setupVAO(m_program);
  m_trianglesToDraw = m_modelSphere.getNumTriangles();

  m_modelSquare.loadObj(assetsPath + "objmodels/chamferbox.obj");
  m_modelSquare.setupVAO(m_program);
  m_trianglesToDraw = m_modelSquare.getNumTriangles();

  // Camera at (0,0,0) and looking towards the negative z

  m_viewMatrix = m_camera.getViewMatrix();
  // Setup stars
  for (auto &star : m_stars) {
    randomizeStar(star);
  }

  for (auto &alvo : m_alvos) {
    alvo.m_rotationAxis = glm::sphericalRand(1.0f);
  }

  auto const filename{abcg::Application::getAssetsPath() +
                      "Inconsolata-Medium.ttf"};
  m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 20.0f);
  if (m_font == nullptr) {
    throw abcg::RuntimeError{"Cannot load font file"};
  }
}

void Window::randomizeStar(Star &star) {
  // Random position: x and y in [-20, 20), z in [-100, 0)
  std::uniform_real_distribution<float> distPosXY(-20.0f, 20.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);
  star.m_position =
      glm::vec3(distPosXY(m_randomEngine), distPosXY(m_randomEngine),
                distPosZ(m_randomEngine));

  // Random rotation axis
  star.m_rotationAxis = glm::sphericalRand(1.0f);
}

void Window::onUpdate() {
  // Increase angle by 90 degrees per second
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};
  
  m_timeAcc += deltaTime;
  // Troca de forma a cada 2 segundos
  if (m_timeAcc >= 2.0f) {
    m_actualTargForm = (m_actualTargForm == Forms::SPHERE) ? Forms::SQUARE : Forms::SPHERE;
    m_timeAcc = 0.0f; // Reinicia o acumulador
  }

  // control camera movement
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
  m_camera.computeProjectionMatrix(m_viewportSize);

  m_angle = glm::wrapAngle(m_angle + glm::radians(90.0f) * deltaTime);

  // Update stars
  for (auto &star : m_stars) {
    // Increase z by 10 units per second
    star.m_position.z += deltaTime * 10.0f;

    // If this star is behind the camera, select a new random position &
    // orientation and move it back to -100
    if (star.m_position.z > 0.1f) {
      randomizeStar(star);
      star.m_position.z = -100.0f; // Back to -00
    }
  }

  detectTargetPosition();
}

void Window::detectTargetPosition() {

  for (auto &alvo : m_alvos) {
    // Transformar a posição do alvo para o espaço da câmera
    glm::vec4 worldPos = glm::vec4(alvo.m_positionTarget, 1.0f);
    glm::vec4 clipSpacePos =
        m_camera.getProjMatrix() * m_camera.getViewMatrix() * worldPos;

    if (clipSpacePos.w != 0.0f) {
      // Normalizar para NDC
      glm::vec3 ndcPos = glm::vec3(clipSpacePos) / clipSpacePos.w;

      // Converter NDC para coordenadas de tela
      glm::vec2 screenPos;
      screenPos.x = (ndcPos.x * 0.5f + 0.5f) * m_viewportSize.x;
      screenPos.y = (1.0f - (ndcPos.y * 0.5f + 0.5f)) * m_viewportSize.y;

      // Calcular o raio aparente com base na escala do alvo
      glm::vec3 scaleOffset =
          glm::vec3(alvo.m_scaleTarget.x * 0.5f); // Raio no espaço do mundo
      glm::vec4 scaleWorldPos = worldPos + glm::vec4(scaleOffset, 0.0f);
      glm::vec4 scaleClipSpacePos =
          m_camera.getProjMatrix() * m_camera.getViewMatrix() * scaleWorldPos;

      float apparentRadius = 0.0f;
      if (scaleClipSpacePos.w != 0.0f) {
        glm::vec3 scaleNDC = glm::vec3(scaleClipSpacePos) / scaleClipSpacePos.w;
        glm::vec2 scaleScreenPos;
        scaleScreenPos.x = (scaleNDC.x * 0.5f + 0.5f) * m_viewportSize.x;
        scaleScreenPos.y =
            (1.0f - (scaleNDC.y * 0.5f + 0.5f)) * m_viewportSize.y;

        apparentRadius = glm::distance(screenPos, scaleScreenPos);
      }

      // Verificar a distância do mouse até o centro do alvo
      glm::vec2 mousePos = glm::vec2(m_mousePosition);
      float dist = glm::distance(mousePos, screenPos);

      // Determinar se o mouse está dentro do alvo
      if (dist <= apparentRadius * 0.85f) {
        alvo.m_mouseInside = true;
      } else {
        alvo.m_mouseInside = false;
      }
    }
  }
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  auto const modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  auto const colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE,
                           &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE,
                           &m_camera.getProjMatrix()[0][0]);

  renderTargets(colorLoc, modelMatrixLoc);

  // Render each star
  for (auto &star : m_stars) {
    // Compute model matrix of the current star
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, star.m_position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, star.m_rotationAxis);

    // Set uniform variable
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

    m_model.render();
  }

  abcg::glUseProgram(0);
}

void Window::renderTargets(GLint colorLoc, GLint modelMatrixLoc){
  for (auto i = 0; i < (int)m_alvos.size(); i++) {
    auto &alvo = m_alvos[i];
    glm::mat4 model{1.0f};
    glm::vec3 targetPosition{};

    if (i == 0) {
      targetPosition = {0.25f, 0.25f, -1.0f};
      abcg::glUniform4f(colorLoc, 1.0f, 0.0f, 0.00f, 1.0f); // White

      alvo.m_positionTarget = targetPosition;
      model = glm::translate(model, alvo.m_positionTarget);
      model = glm::rotate(model, m_angle, alvo.m_rotationAxis);
      model = glm::scale(model, alvo.m_scaleTarget);

      // atribui o modelo na matriz de modelo do vertex shader
      abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
      if (!alvo.m_hit)
        m_modelSphere.render(m_trianglesToDraw);

    } else if (i == 1) {
      targetPosition = {-0.25f, 0.25f, -1.0f};
      alvo.m_positionTarget = targetPosition;
      model = glm::translate(model, alvo.m_positionTarget);
      model = glm::rotate(model, m_angle, alvo.m_rotationAxis);
      model = glm::scale(model, alvo.m_scaleTarget);

      // atribui o modelo na matriz de modelo do vertex shader
      abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White
      
      if (!alvo.m_hit)
        m_modelSquare.render(m_trianglesToDraw);

    } else if (i == 2) {
      targetPosition = {-0.25f, -0.25f, -1.0f};
      alvo.m_positionTarget = targetPosition;
      model = glm::translate(model, alvo.m_positionTarget);
      model = glm::rotate(model, m_angle, alvo.m_rotationAxis);
      model = glm::scale(model, alvo.m_scaleTarget);

      // atribui o modelo na matriz de modelo do vertex shader
      abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White
      
      if (!alvo.m_hit)
        m_modelSquare.render(m_trianglesToDraw);

    } else {
      targetPosition = {0.25f, -0.25f, -1.0f};
      alvo.m_positionTarget = targetPosition;
      model = glm::translate(model, alvo.m_positionTarget);
      model = glm::rotate(model, m_angle, alvo.m_rotationAxis);
      model = glm::scale(model, alvo.m_scaleTarget);

      // atribui o modelo na matriz de modelo do vertex shader
      abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White
      if (!alvo.m_hit)
        m_modelSphere.render(m_trianglesToDraw);
    }
  }
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {// Tamanho e posição do widget
    auto const pointsWidget{ImVec2(210, 40)};
    ImGui::PushFont(m_font);
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - pointsWidget.x - 10,  10));
    ImGui::SetNextWindowSize(pointsWidget);
    ImGui::Begin("Points", nullptr, ImGuiWindowFlags_NoDecoration);

    // Mostrando os pontos
    ImGui::Text("Pontos: %d", m_totalPoints);
    ImGui::End();
  }
  {
    // Tamanho e posição do widget
    auto const widgetSize{ImVec2(210, 150)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - widgetSize.x - 10, m_viewportSize.y - widgetSize.y - 10));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Forms", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::PushFont(m_font);

    ImGui::Text("Alvo");
    // Obter o contexto de desenho
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    
    // Posição inicial para desenhar dentro do widget
    ImVec2 widgetPos = ImGui::GetCursorScreenPos();

    if (m_actualTargForm == Forms::SPHERE) {
      // Desenhar um círculo preenchido
      drawList->AddCircleFilled(
          ImVec2(widgetPos.x + 105, widgetPos.y + 55), // Centro
          50.0f,                                       // Raio
          IM_COL32(0, 255, 0, 255)                     // Cor (verde)
      );
    } else {
      drawList->AddRectFilled(
          ImVec2(widgetPos.x + 55, widgetPos.y + 10),  // Canto superior esquerdo
          ImVec2(widgetPos.x + 160, widgetPos.y + 105), // Canto inferior direito
          IM_COL32(0, 255, 0, 255)                     // Cor (verde)
      );
    }

    // Desenhar um quadrado preenchido

    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_camera.computeProjectionMatrix(size);
}

void Window::onDestroy() {
  m_model.destroy();
  m_modelSphere.destroy();
  m_modelSquare.destroy();
  abcg::glDeleteProgram(m_program);
}

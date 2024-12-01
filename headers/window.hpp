#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <array>
#include <random>

#include "abcgOpenGL.hpp"
#include "model.hpp"
#include "camera.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onCreate() override;
  void onEvent(SDL_Event const &event) override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  std::default_random_engine m_randomEngine;

  ImFont *m_font{};
  glm::ivec2 m_viewportSize{};

  enum class Forms {SQUARE, SPHERE};
  enum class Colors {GREEN, RED};

  Forms m_actualTargForm{Forms::SPHERE};
  float m_timeAcc{1.0f};

  Model m_model;
  Model m_modelSphere;
  Model m_modelSquare;
  Camera m_camera;

  glm::ivec2 m_mousePosition{};

  //Model m_modelTargets;
  int m_trianglesToDraw{};

  float m_dollySpeed{};
  float m_truckSpeed{};
  float m_panSpeed{};

  struct Star {
    glm::vec3 m_position{};
    glm::vec3 m_rotationAxis{};
  };
  // cena 3D formada por 500 estrelas
  std::array<Star, 500> m_stars;
  
  struct Alvos {
    glm::vec3 m_positionTarget{};
    glm::vec3 m_scaleTarget{0.25f};
    glm::vec3 m_rotationAxis{};
    bool m_mouseInside{false};
    bool m_hit{false};
  };

  std::array<Alvos, 4> m_alvos;

  int m_totalPoints{0};
  struct Fases{
    Forms m_form;
    Colors m_color;
    std::array<Alvos, 4> m_alvos;
    int m_points;
  };

  std::array<Fases, 4> m_fases;


  // angulo em radianos que rotaciona cada cubo (cada cubo tem seu proprio eixo
  // de rotacao)
  float m_angle{};

  // iniciadas como identidade, mas alteradas em onCreate e onPaint
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  GLint m_viewMatrixLocation{};
  GLint m_projMatrixLocation{};
    
  // angulo de abertura vertical de visao, slider vai alterar
  float *m_FOV = &m_camera.m_FOV;

  GLuint m_program{};

  // utilizado para gerar aleatoriamente a posicao e eixo de rotacao de cada
  // estrela passada como parametro
  void randomizeStar(Star &star);
  void detectTargetPosition();
  void renderTargets(GLint colorLoc, GLint modelMatrixLoc);
};

#endif

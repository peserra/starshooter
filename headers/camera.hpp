#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera{
public:
    void computeViewMatrix();
    void computeProjectionMatrix(glm::vec2 const &size);

    // modificam m_eye e m_at a partir de inputs do teclado
    void dolly(float speed);
    void truck(float speed);
    void pan(float speed);

    void setEye(glm::vec3 const &eye);
    void setAt(glm::vec3 const &at);
    void setUp(glm::vec3 const &up);
    void setFov(float fovValue);

    float m_FOV{70.0f};

    glm::mat4 const &getViewMatrix() const {return m_viewMatrix; }
    glm::mat4 const &getProjMatrix() const {return m_projMatrix; }

private:
    glm::vec3 m_eye{0.0f, 0.0f, 0.0f};  // posicao da camera
    glm::vec3 m_at{0.0f, 0.0f, -1.0f};   // pra onde a camera esta olhando
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};   // indica qual direcao é 'para cima' 
    
    // matriz que transforma 'espaco real' em 'espaco da camera'
    glm::mat4 m_viewMatrix;
    // matriz que transforma 'espaco da camera' em 'espaco da projecao' 
    glm::mat4 m_projMatrix;

};

#endif

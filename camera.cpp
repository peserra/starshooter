#include "headers/camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


void Camera::computeProjectionMatrix(glm::vec2 const &size){
    m_projMatrix = glm::mat4(1.0f);
    auto const aspect{size.x/size.y};
    m_projMatrix = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 100.0f);
}

void Camera::computeViewMatrix(){
    m_viewMatrix = glm::lookAt(m_eye, m_at, m_up);
}

void Camera::dolly(float speed){
    // computa o vetor 'para frente'
    auto const forward{glm::normalize(m_at - m_eye)};

    // move 'eye' e 'center' para frente (speed > 0) e para tras (speed < 0 )
    m_eye += speed * forward;
    m_at += speed * forward;

    // computa a matriz da view toda vez
    computeViewMatrix();
}

void Camera::truck(float speed){
    // computa vetor 'para frente' (baseado em onde estou olhando)
    auto const forward{glm::normalize(m_at - m_eye)};
    // vomputa vetor 'esquerda', produto vetorial entre os dois vetores. 
    auto const left{glm::cross(m_up, forward)};

    // move eye e center para a esquerda (speed < 0) ou direita (speed > 0)
    m_eye -= left * speed;
    m_at -= left * speed;
    // computa a matriz da view toda vez
    computeViewMatrix();
}

void Camera::pan(float speed){
    glm::mat4 transform{1.0};

    // rotaciona a camera em volta da sua posicao y local
    transform = glm::translate(transform, m_eye);
    transform = glm::rotate(transform, -speed, m_up);
    transform = glm::translate(transform, -m_eye);

    m_at = transform * glm::vec4(m_at, 1.0f);
    computeViewMatrix();
}

void Camera::setEye(glm::vec3 const &eye) {
    m_eye = eye;
    computeViewMatrix();  // Atualiza a matriz de visão
}

void Camera::setAt(glm::vec3 const &at) {
    m_at = at;
    computeViewMatrix();  // Atualiza a matriz de visão
}

void Camera::setUp(glm::vec3 const &up) {
    m_up = up;
    computeViewMatrix();  // Atualiza a matriz de visão
}


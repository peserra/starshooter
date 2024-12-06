# Star Shooter

**Star Shooter** é um jogo interativo desenvolvido em C++ utilizando a biblioteca ABCg. Neste jogo, cubos e esferas são gerados aleatoriamente na tela, e o jogador deve clicar nas formas corretas para acumular pontos. O cenário de fundo apresenta um efeito de starfield com esferas que se movem em direção à câmera, criando uma experiência imersiva e dinâmica. A proposta desse jogo é ser um jogo de atenção, onde o jogador precisa clicar nas formas corretas. A ideia é exercitar a atenção, uso do mouse e
coordenação.

## Funcionalidades

- **Geração Aleatória de Formas:** Cubos e esferas aparecem aleatoriamente na tela em diferentes posições e tamanhos, em cada fase.
- **Interação do Jogador:** Clique nas formas corretas para ganhar pontos.
- **Efeito de Starfield:** Fundo animado com esferas que se movem em direção à câmera, simulando um campo de estrelas.
- **Pontuação:** Sistema de pontuação que recompensa acertos e penalidade em erros.
- **Gráficos 3D:** Renderização de formas em 3D para uma melhor experiência visual.

## Tecnologias Utilizadas

- **Linguagem de Programação:** C++
- **Biblioteca Gráfica:** [ABCg](https://github.com/cx-org/abcg)
- **Renderização 3D:** OpenGL (integrado com ABCg)
- **Sistema de Controle de Versão:** Git

## Detalhes do código

Para a janela temos que os seguintes métodos foram sobrescritos:

   - onCreate()
   - onEvent()
   - onUpdate()
   - onPaint()
   - onPaintUI()
   - onResize()
   - onDestroy()

O Window::onCreate inicializa os recursos e configurações necessários para o funcionamento do jogo. Ele configura o OpenGL com fundo preto e teste de profundidade, carrega e prepara os shaders e modelos 3D (como esferas e cubos), define a matriz de visão da câmera e inicializa elementos da cena, como estrelas e alvos com eixos de rotação aleatórios. Além disso, prepara as fases do jogo, gerando formas e cores-alvo de maneira aleatória, e carrega uma fonte para a interface gráfica. Esse método garante que todos os elementos estejam configurados antes do início da execução.

```cpp
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
  std::uniform_int_distribution<int> distribuicao(0, 1000);
  // para cada fase
  for (auto i = 0; i < (int)m_fases.size(); i++) {
    auto &fase = m_fases[i];
    fase.m_points = 0; // pontos iniciais adquiridos naquela fase
    
    // preenche quais formas terao naquela fase
    for (auto j = 0; j < (int)fase.m_targetForms.size(); j++){
      int numero = distribuicao(m_randomEngine) % 2;
      if (numero == 0) {
        fase.m_targetForms[j] = Forms::SQUARE;
      } else {
        fase.m_targetForms[j] = Forms::SPHERE;
      }
    }
    
    // alvo é uma forma aleatoria dos alvos criados na fase
    // garante que sempre vai ter um alvo valido
    int selecAlvo = distribuicao(m_randomEngine) % (int)fase.m_targetForms.size();
    fase.m_targetForm = fase.m_targetForms[selecAlvo];

    // seleciona cor aleatoria entre verde e vermelho
    int selecCor = distribuicao(m_randomEngine) % 2;
    fase.m_targetColor = m_colors[selecCor];
  }

  m_camera.m_FOV = 170.0f;
}

```

O onEvent() é chamado para lidar com eventos do SDL, no nosso caso foi sobrescrito para lidar com movimento do mouse e clicks do mouse. Atribuindo a uma variável global m_mousePosition os valores das coordenadas do mouse, sempre que o mesmo for movido. Caso o evento seja um click do mouse então se checa o mouse está dentro do hitbox dos alvos (as esferas ou os cubos) para se setar o valor de alvo.m_hit como true, marcando que o alvo foi acertado.

```cpp
void Window::onEvent(SDL_Event const &event) {

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
```

O Window::onUpdate é responsável por atualizar o estado da janela e seus elementos a cada quadro. Ele incrementa o tempo acumulado para alternar a fase do jogo a cada 5 segundos, atualiza a posição da câmera com base nos comandos de movimento e calcula a nova matriz de projeção. As estrelas na cena têm suas posições incrementadas ao longo do eixo Z, retornando a uma posição aleatória com coordenada z = -100 quando passam pela câmera, criando um efeito de movimento contínuo. Além disso, ele chama o método detectTargetPosition, que calcula as coordenadas dos alvos na tela com base nas transformações da câmera, determina o raio aparente dos alvos, e verifica se o cursor do mouse está dentro dos limites de cada alvo, atualizando o estado correspondente (m_mouseInside). Esse método garante a atualização contínua da lógica e dos elementos gráficos do jogo.

```cpp
void Window::onUpdate() {
  // Increase angle by 90 degrees per second
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};
  
  if (m_reduceFOV) {
    m_timeAccFOV += deltaTime; // Acumula tempo
    float const duration = 1.0f; // Duração da transição em segundos
    if (m_timeAccFOV <= duration) {
      // Interpola o FOV de 170 para 70
      float const t = m_timeAccFOV / duration; // Normaliza entre 0 e 1
      m_camera.m_FOV = glm::mix(170.0f, 70.0f, t);
    } else {
      // Garante que o FOV finalize em 70 e termina a transição
      m_camera.m_FOV = 70.0f;
      m_reduceFOV = false; // Finaliza a transição
    }
  }


  if (m_gameStatus == GameStatus::PLAYING) {
    if (m_faseAtual == (int)m_fases.size()){
      m_faseAtual = 0;
      m_camera.m_FOV = 170.0f;
      m_gameStatus = GameStatus::ON_MENU;
    }
    m_timeAcc += deltaTime;
    if (m_timeAcc >= 6.0f) {
      fmt::print("fase {}\n", m_faseAtual);  
      for (auto &a: m_alvos){
        // reseta os alvos quando muda de fase
        a.m_hit = false; 
        a.m_alreadyComputePoint = false;
      }
      m_faseAtual++;
      m_timeAcc = 0.0f; // Reinicia o acumulador
    }
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
  if (m_gameStatus == GameStatus::PLAYING){
    detectTargetPosition();
    computePoints();
  }
}
```

O Window::onPaint é responsável por renderizar a cena na tela a cada quadro. Ele limpa o buffer de cor e profundidade, configura o viewport e ativa o programa de shaders usado para desenhar os objetos. Matrizes de visão e projeção da câmera são enviadas como variáveis uniformes para os shaders. Em seguida, o método renderiza os alvos da fase atual por meio de renderTargets, e cada estrela na cena é renderizada individualmente, com sua matriz de modelo calculada para aplicar transformações de posição, escala e rotação. Finalmente, o programa de shaders é desativado, completando o ciclo de renderização.

```cpp
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

  abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White

  if (m_gameStatus == GameStatus::PLAYING) {
    renderTargets(colorLoc, modelMatrixLoc, m_fases[m_faseAtual]);
  }

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
```

O método `Window::onPaintUI` é responsável por renderizar a interface gráfica do jogo utilizando a biblioteca ImGui. Ele exibe dois widgets: um no topo direito para mostrar a pontuação total do jogador e outro no canto inferior direito para indicar a forma e a cor do alvo atual. O método configura o tamanho e a posição de cada widget, utiliza uma fonte personalizada e, no caso do widget do alvo, desenha dinamicamente uma esfera ou um retângulo preenchido, dependendo do formato-alvo da fase atual, com a cor associada. Isso proporciona ao jogador informações visuais claras sobre o objetivo da fase e sua pontuação.

```cpp
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

  renderTargets(colorLoc, modelMatrixLoc, m_fases[m_faseAtual]);

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
```

Além da classe da janela, temos a classe do modelo. Que ficou responsável por encapsular todas as lógicas de VAO, VBO, EBO e renderização dos modelos de objetos (dos polígonos usados). Para essa classe temos os seguintes métodos:

   - loadObj()
   - render()
   - setupVAO()
   - destroy()
   - createBuffers()
   - standardize()

O método `Model::loadObj` carrega um modelo 3D no formato OBJ de um arquivo especificado pelo caminho `path` e converte seus dados em uma estrutura utilizável para renderização. Ele utiliza a biblioteca `tinyobjloader` para processar o arquivo OBJ, extrair atributos como posições dos vértices e organizá-los em listas de vértices e índices (`m_vertices` e `m_indices`). Para evitar duplicatas, um mapa de hash associa vértices únicos a índices, otimizando o uso de dados. Caso a opção `standardize` seja ativada, o modelo é padronizado para ajustar sua escala e posição. Finalmente, o método chama `createBuffers` para configurar os buffers necessários para renderização. Ele também trata erros e avisos relacionados ao carregamento do arquivo.

```cpp
void Model::loadObj(std::string_view path, bool standardize) {
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<VertexBG, GLuint> hash{};

  // Loop over shapes
  for (auto const &shape : shapes) {
    // Loop over indices
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      auto const index{shape.mesh.indices.at(offset)};

      // Vertex position
      auto const startIndex{3 * index.vertex_index};
      auto const vx{attrib.vertices.at(startIndex + 0)};
      auto const vy{attrib.vertices.at(startIndex + 1)};
      auto const vz{attrib.vertices.at(startIndex + 2)};

      VertexBG const vertex{.position = {vx, vy, vz}};

      // If hash doesn't contain this vertex
      if (!hash.contains(vertex)) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  if (standardize) {
    Model::standardize();
  }

  createBuffers();
}
```

O método Model::render é responsável por renderizar o modelo 3D utilizando os dados de vértices e índices armazenados no VAO (Vertex Array Object). Ele vincula o VAO correspondente ao modelo, calcula o número de índices a ser desenhado (com base no número de triângulos fornecido ou no total de índices do modelo), e em seguida, realiza a renderização com a função glDrawElements para desenhar os triângulos. Após a renderização, o VAO é desvinculado, finalizando o processo de renderização.

```cpp
void Model::render(int numTriangles) const {
  abcg::glBindVertexArray(m_VAO);

  auto const numIndices{(numTriangles < 0) ? m_indices.size()
                                           : numTriangles * 3};

  abcg::glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);
}
```

O método `Model::setupVAO` configura o Vertex Array Object (VAO) do modelo para preparar os dados de vértices e índices para renderização. Primeiro, ele remove qualquer VAO anterior e cria um novo. Em seguida, vincula os buffers de índices (EBO) e vértices (VBO) ao VAO. O método também vincula o atributo de posição dos vértices ao shader program usando a localização do atributo no programa de shaders. Após configurar o atributo de posição, ele termina o processo de vinculação dos buffers e do VAO, preparando o modelo para ser renderizado.

```cpp
void Model::setupVAO(GLuint program) {
  // Release previous VAO
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  auto const positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(VertexBG), nullptr);
  }

  // End of binding
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}
```

A lógica de câmera look at é feita pala classe câmera. Que contêm os seguintes métodos:

   - dolly()
   - truck()
   - pan()
   - setEye()
   - setAt()
   - setUp()
   - setFov()

Essa implementação de uma câmera "LookAt" utiliza a biblioteca GLM para calcular e manipular as matrizes de projeção e visão de uma câmera 3D. A câmera é configurada com uma posição (`eye`), um ponto de interesse (`at`) e um vetor para cima (`up`). O método `computeProjectionMatrix` calcula a matriz de projeção perspectiva com base no campo de visão (FOV) e a proporção da janela, enquanto o método `computeViewMatrix` cria a matriz de visão com a função `glm::lookAt`, que orienta a câmera para o ponto de interesse. A câmera permite três tipos de movimentação: `dolly`, que move a câmera para frente ou para trás; `truck`, que move a câmera lateralmente (para esquerda ou direita); e `pan`, que rotaciona a câmera em torno do seu eixo local, ajustando o ponto de interesse. Cada movimento atualiza a matriz de visão da câmera. Além disso, os métodos `setEye`, `setAt` e `setUp` são usados para definir esses parâmetros e recalcular a matriz de visão automaticamente após cada alteração.

## Instalação

### Pré-requisitos

- **C++ Compiler:** Compatível com C++17 ou superior.
- **CMake:** Versão 3.10 ou superior.
- **Biblioteca ABCg:** Certifique-se de ter a biblioteca ABCg instalada. Você pode cloná-la a partir do [repositório oficial](https://github.com/cx-org/abcg).

### Passos de Instalação

1. **Clone o Repositório:**
    Clone o repositorio do projeto dentro da pasta "examples" da abcg
   ```bash
   git clone https://github.com/peserra/starshooter
   cd starshooter
   ```

2. **Crie o Diretório de Build:**
   Altere o arquivo CmakeList.txt para compilar o projeto 
   ```bash
   mkdir build
   cd build
   ```

3. **Compile o Projeto:**
   No diretorio abcg, execute
   ```bash
   ./build.sh
   ```

4. **Execute o Jogo:**
   No diretorio abcg, execute
   ```bash
    ./build/bin/starshooter/starshooter
   ```
   
## Instruções de como jogar:
- O visor no lado direito inferior indica qual figura é necessário clicar:
     -  Quadrado verde: Clique nos cubos;
     -  Círculo verde: Clique nas esferas;
     -  Quadrado vermelho: Clique nas esferas;
     -  Círculo vermelho: Clique nos cubos.



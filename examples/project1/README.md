Rafael Riciardi Barreira - 11056916

# Projeto 1 - DVD Animation

Link para a aplicação: https://rafaelriciardi.github.io/abcg/

## Motivação e ideia principal

No decorrer das aulas, ao ver alguns exemplos, tive a ideia de fazer algo muito semelhante e inspirado naquelas animações da tela de espera dos DVDs que tinham antigamente, como o exemplo da imagem abaixo:

<p align="center">
  <img src="https://repository-images.githubusercontent.com/344610266/246c3a80-7cf0-11eb-92d0-fe1d20e11982"/>
</p>

Para facilitar um pouco a implementação, ao invés de realizar a animação utilizando o logo do DVD, escolhi por fazer algo com a mesma ideia, porém utilizando um circulo. Para isso, usei como base inicial o código do **regularpolygons** e fiz todas as alterações e implementações necessárias para chegar aos resultados esperados. A ideia de utilizar esse código como base, era justamente reaproveitar a função de desenhar poligonos e desenhar um poligono de muitos lados, de forma que visualmente se aproximasse de um circulo. No caso do projeto, o circulo animado é um poligono de **256** lados.

A ideia principal do código é que o atributo de translação do poligono criado seja atualizado a cada ciclo de execução do código, fazendo justamente com que o objeto pareça se mover. Para que isso funcione, foi preciso configurar alguns comportamentos no código.

## Comportamento geral

Primeiramente, para a parte de inicialização, utilizei uma flag *firstExec* para identificar esse momento, no qual são geradas as duas cores aleatorias que o circulo irá receber e são instanciadas também de maneira aleatória o modificador de translação de ambos os eixos, que basicamente é o quanto o objeto irá se mover em X e Y a cada ciclo.
```
  //Initialization settings using the firstExec flag;
  if (firstExec){
    color1 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
    color2 = {rd(m_randomEngine), rd(m_randomEngine),rd(m_randomEngine)};
    translationModifierX = tm_rd(m_randomEngine);
    translationModifierY = tm_rd(m_randomEngine);
    firstExec = false;
  }
```

Para conseguir controlar uma variação de velocidade na execução, utilizei um slides muito semelhante ao do código base, mas criei a função de transformação *mapRange*, que mapeia os valores de 0 a 100 do slides, para uma faixa que ficou interessante de aplicar ao delay do timer (30 a 0). Por meio desse temporizador, que já havia sido implementado no código padrão, conseguimos controlar o intervalo que algum comportamento deve se repetir. Nesse caso, é o invervalo em que estamos atualizado a posição de transação dos eixos X e Y com o modificador definido na inicialização acima.
```
  //Transform the delay variable using this mapRange function created in this file
  m_delay = round(mapRange(m_speed, 1, 100, 30, 0));

  if (m_timer.elapsed() <= m_delay / 1000.0){

  }
  else{ //Control the translation over the time
    translationX = translationX+translationModifierX;
    translationY = translationY+translationModifierY;
    m_timer.restart();
  }
```
Esse próximo trecho justamente rege o comportamento de quando o circulo de choca com alguma das extremidades da tela. Para identificar esse momento, basicamente é verificado se a posição de translação absoluta no eixo é maior do que 0.8. Esse valor foi encontrado puramente por meio de testes de maneira que o toque a borda parecesse mais natural.

Ao tocar as bordas, dependendo da seleção da UI que será explicado mais abaixo, ou o sentido do deslocamento no eixo é apenas invertido (Recoil Mode = Stable) ou além do sentido ser invertido, também é gerada uma nova taxa de translação de maneira aleatória (Recoil Mode = Randomized). Além disso, ao tocar nas bordas, novas cores também são sorteadas para o objeto.

Para evitar um bug que estava ocorrendo, também foram criadas flags de controle para cada eixo, de modo que esse comportamento acima do pode ser executado uma vez quando o circulo estiver dentro do valor de 0.8 e isso é resetado quando ele volta para pelo menos 0.75. Isso evita um bug do objeto ficar preso nesse ciclo de comportamento quando chegava próximo a borda em alguns momentos.
```
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
```

Finalmente o objeto é criado com a função setupModel, que foi ligeiramente modificada do código de aula, recebendo agora como parâmetro as cores que o poligono criado vai utilizar e não mais gerando-as internamente. Em seguida, são atualizados os atributos de translação com base na lógica acima, e a escala, que na verdade utiliza uma constante de 0.2f definida na variavel global *scaleFactor*. Esse valor também foi obtido por testes de modo a ficar um tamanho interessante para a aplicação.

As cores também são uma opção controlada por UI com uma simples condicional no código. No caso do seletor *Color Mode* estar em *Single Color*, a cor 2 recebe o mesmo valor da cor 1, resultando em um poligono de cor única. Caso o seletor esteja na opção *Mixed Colors* ele não faz nada na lógica, mantendo as duas cores geradas aleatoriamente.

```
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
```

## UI
Para a UI foi criada uma pequena janela no lado inferior direito que exibe algumas informações e é possíveis alterernar algumas opções. A primeira informação que é mostrada a todo tempo são as coordenadas nos eixos X e Y do circulo, que podem ser acompanhadas durante a animação / translação.

Logo a seguir temos dois seletores do tipo drop down para escolher o modo do ricochete quando o circulo toda as bordas e o comportamento das cores, ambos descritos em detalhes nos trechos acima. Para ajustar também a velocidade da animação, também há um seletor do tipo slides, no qual pode-se variar a velocidade entre os valores de 1 e 100.

Por fim, adicionei um botão de *Restart*, que permite reinicializar a animação e que, ao ser clicado, reestabelece as variáveis importantes para inicialização para seus valores iniciais.
```
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
```

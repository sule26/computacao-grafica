## Documentação do Jogo "Penguin Adventure"

**Versão:** 1.0 (Conforme desenvolvimento atual)

**Autor:** Rafael Liberatori e João Pedro

**Linguagem:** C++ com OpenGL (GLUT)

---

### 1. Visão Geral do Jogo
![gif do jogo](https://i.imgur.com/s0jcTwI.gif)

"Penguin Adventure" é um jogo 2D onde o jogador controla um pinguim com o objetivo de manter seu filhote alimentado e, consequentemente, o tempo do jogo positivo, enquanto sobrevive por até 5 minutos. O pinguim pode se mover na terra e nadar na água para capturar peixes. Esses peixes podem ser levados ao filhote para adicionar tempo bônus ao contador principal do jogo. Um pássaro inimigo sobrevoa o cenário e pode realizar mergulhos parabólicos, representando uma ameaça ao pinguim quando este está em terra.

**Objetivos:**
1.  Capturar peixes na água.
2.  Alimentar o filhote com os peixes para adicionar tempo ao contador de "Vida do pinguim".
3.  Evitar que o contador "Vida do pinguim" chegue a zero.
4.  Sobreviver por 5 minutos com o contador "Vida do pinguim" positivo para ganhar o jogo.
5.  Evitar ser atingido pelo pássaro inimigo enquanto estiver em terra.

**Condições de Fim de Jogo:**
*   **Derrota (Game Over):**
    *   O contador "Vida do pinguim" chega a zero.
    *   O pinguim é atingido pelo pássaro inimigo enquanto está em terra.
*   **Vitória:**
    *   O contador "Vitória em:" (que representa a duração da sessão de 5 minutos) chega a zero E o contador "Vida do pinguim" ainda é maior que zero.

Ao final do jogo (vitória ou derrota), uma mensagem é exibida, e o jogador pode reiniciar pressionando a tecla 'R'.

---

### 2. Estrutura do Código

O código é organizado em várias seções principais:

*   **Includes e Constantes Globais:** Bibliotecas necessárias e valores fixos que definem aspectos do jogo (tamanho da tela, limites do mundo, velocidades, etc.).
*   **Variáveis Globais:** Variáveis que mantêm o estado do jogo, como posição dos personagens, timers, flags de estado (e.g., `isGameOver`, `playerWon`, `pinguinHasFish`).
*   **Estruturas de Dados:**
    *   `struct Fish`: Define as propriedades de um peixe (posição, visibilidade, velocidade, direção).
    *   `enum EnemyState`: Define os estados do pássaro inimigo (voando normal, mergulhando, retornando à altitude).
*   **Protótipos de Funções:** Declarações antecipadas das funções utilizadas.
*   **Funções Utilitárias:**
    *   `formatTime()`: Formata segundos em uma string "MM:SS".
*   **Funções de Inicialização:**
    *   `initializeGame()`: Reseta todas as variáveis de estado para iniciar um novo jogo.
    *   `init()`: Configurações iniciais do OpenGL (cor de fundo, projeção).
*   **Funções de Lógica e Atualização:**
    *   `timer()`: Chamada periodicamente para atualizar a lógica do jogo (movimento de peixes, pássaro, nuvens, timers).
    *   `updateEnemy()`: Controla o comportamento e movimento do pássaro inimigo.
    *   `checkCollisions()`: Gerencia a detecção de colisões (pinguim-peixe, pinguim-filhote para alimentação, pinguim-pássaro).
    *   `isPinguinInWater()`: Verifica se o pinguim está na área da água.
*   **Funções de Desenho (`draw...`)**: Responsáveis por renderizar os diferentes elementos visuais do jogo.
    *   Primitivas: `drawSquare()`, `drawTriangle()`, `drawDisk()`, `drawElipse()`.
    *   Cenário: `drawCloud()`, `drawFloor()`, `drawWater()`.
    *   Personagens e Objetos: `drawFishModel()`, `drawPenguin()`, `drawEnemyWing()`, `drawEnemy()`.
*   **Funções de Callback do GLUT:**
    *   `display()`: Função principal de desenho, chamada sempre que a tela precisa ser atualizada.
    *   `keyboard()`: Processa input de teclas especiais (setas).
    *   `keyboardNormal()`: Processa input de teclas normais (R para reiniciar, ESC para sair).
*   **`main()`:** Ponto de entrada do programa, inicializa o GLUT, registra callbacks e inicia o loop principal do jogo.

---

### 3. Detalhes das Principais Mecânicas

#### 3.1. Pinguim (Jogador)
*   **Movimento:** Controlado pelas teclas de seta. Pode se mover para esquerda/direita na terra e na água. Na água, pode também se mover para cima/baixo para nadar e mergulhar.
*   **Animação:** Ao entrar na água, o pinguim "deita" (rotação de 270 graus). A direção do seu corpo (esquerda/direita) e a inclinação do bico (cima/baixo durante o nado) são refletidas por escalonamento.
*   **Captura de Peixe:** Ao colidir com um peixe na água, o pinguim o captura. Um peixe é então desenhado em seu bico. Só pode carregar um peixe por vez.
*   **Alimentar Filhote:** Se o pinguim está carregando um peixe e se aproxima do filhote (que está na terra), o peixe é entregue. Isso adiciona `TIME_BONUS_FROM_FEEDING` (60 segundos) ao `gameTimeRemaining`.

#### 3.2. Pinguim Filhote
*   Posicionado estaticamente à esquerda da tela, na terra.
*   Sua "vida" está diretamente ligada ao `gameTimeRemaining`.
*   Visualmente, ele pode virar para olhar na direção do pinguim principal quando este se aproxima com um peixe.

#### 3.3. Peixes
*   Quatro peixes se movem horizontalmente na água, em alturas variadas.
*   Ao atingir as bordas da área da água, invertem a direção.
*   Quando um peixe é capturado, ele "desaparece" e um novo peixe (o mesmo, na verdade) reaparece em uma posição e com velocidade aleatórias na água.

#### 3.4. Pássaro Inimigo
*   **Movimento Normal:** Voa horizontalmente de um lado para o outro da tela em uma altitude fixa (`ENEMY_NORMAL_ALTITUDE`).
*   **Mergulho:** A cada travessia da tela, há uma chance de o pássaro iniciar um mergulho em um ponto X aleatório.
    *   A trajetória do mergulho é uma parábola.
    *   O ponto inicial é a posição atual do pássaro.
    *   O ponto final horizontal (`diveEndX`) é uma distância fixa (`DIVE_HORIZONTAL_DISTANCE`) à frente.
    *   A altura final da fase parabólica (`diveEndY`) é sorteada, ficando entre o nível do chão e um pouco abaixo da altitude normal.
    *   O ponto mais baixo que a parábola atinge (`targetLowestPointY`) também é sorteado, com o chão (`WATER_SURFACE_Y_LEVEL`) como limite inferior.
    *   A profundidade extra da curva (`calculatedDiveExtraDepth`) é calculada para que o vértice da parábola atinja `targetLowestPointY`.
*   **Retorno:** Após completar a fase parabólica do mergulho (atingindo `diveEndX`), o pássaro voa horizontalmente na altura `diveEndY` por um instante e então começa a subir gradualmente de volta para `ENEMY_NORMAL_ALTITUDE`, continuando seu voo horizontal. Ao atingir a altitude normal, volta ao estado `FLYING_NORMAL`.
*   **Colisão:** Se o pássaro colidir com o pinguim enquanto este está em terra, o jogo termina (Game Over).

#### 3.5. Nuvens
*   Três nuvens se movem horizontalmente no fundo do cenário, em diferentes alturas e velocidades.
*   Quando uma nuvem sai da tela, ela é reposicionada do outro lado para criar um efeito contínuo.
*   São desenhadas antes de todos os outros elementos do jogo para aparecerem ao fundo.

#### 3.6. Timers e Condições de Fim
*   **`gameTimeRemaining` ("Vida do pinguim"):** Começa com `INITIAL_GAME_TIME` (60 segundos). Diminui a cada segundo. Se chegar a 0, é Game Over. Aumenta em `TIME_BONUS_FROM_FEEDING` (60 segundos) quando o filhote é alimentado.
*   **`sessionTimeRemainingForWin` ("Vitória em:"):** Começa com `MAX_SESSION_DURATION` (300 segundos / 5 minutos). Diminui a cada segundo.
    *   Se este timer chegar a 0 **E** `gameTimeRemaining` ainda for `> 0`, o jogador ganha.
    *   Se este timer chegar a 0 e `gameTimeRemaining` também for `0` (ou menos), é Game Over.
*   **Exibição:** Ambos os timers são exibidos no formato "MM:SS".

---

### 4. Funções Principais de Desenho

*   **`drawPenguin(...)`:** Desenha o pinguim (principal ou filhote). Inclui lógica para desenhar o peixe no bico se `isCarryingFish` for verdadeiro e ajusta a orientação do peixe com base no estado do pinguim (em pé ou nadando, e direção).
*   **`drawFishModel()`:** Desenha a aparência de um peixe.
*   **`drawEnemy()`:** Desenha o pássaro inimigo, incluindo o espelhamento horizontal baseado na direção `enemyGoingRight` e a animação da asa.
*   **`drawEnemyWing()`:** Desenha a forma da asa do pássaro.
*   **`drawCloud()`:** Desenha uma nuvem individual usando múltiplos discos.
*   **`drawFloor()` e `drawWater()`:** Desenham os elementos básicos do cenário.

---

### 5. Controles

*   **Setas Direcionais:**
    *   **Esquerda/Direita:** Movem o pinguim horizontalmente.
    *   **Cima/Baixo:** Movem o pinguim verticalmente quando está na água (nadando/mergulhando).
*   **Tecla 'R' ou 'r':** Reinicia o jogo quando estiver na tela de "Game Over" ou "Você Ganhou".
*   **Tecla 'ESC':** Fecha o jogo.

---

### 6. Compilação e Execução

*   **Dependências:** Biblioteca GLUT.
*   **Compilação (Exemplo Linux/g++):**
    `g++ seu_arquivo.cpp -o nome_do_executavel -lGL -lglut -lm`
*   **Execução:**
    `./nome_do_executavel`

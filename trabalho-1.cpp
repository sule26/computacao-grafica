#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

// Constantes Globais de Configuração
const int WIDTH = 800;                           // Largura da janela
const int HEIGHT = 600;                          // Altura da janela
const int FPS = 60;                              // Frames por segundo desejados
const int DELAY = 1000 / FPS;                    // Atraso entre frames em milissegundos
const int COORDINATES_X = 20;                    // Limite do sistema de coordenadas em X
const int COORDINATES_Y = 20;                    // Limite do sistema de coordenadas em Y
const int COORDINATES_Z = 20;                    // Limite do sistema de coordenadas em Z (para glOrtho)
const float PINGUIN_COLLISION_RADIUS = 1.5f;     // Raio de colisão para o pinguim principal
const float FISH_COLLISION_RADIUS = 1.0f;        // Raio de colisão para os peixes
const float CHICK_COLLISION_RADIUS = 2.0f;       // Raio de colisão para alimentar o filhote
const float ENEMY_EFFECTIVE_COLLISION_RADIUS = 2.0f; // Raio de colisão efetivo para o pássaro inimigo
const float WATER_SURFACE_Y_LEVEL = -COORDINATES_Y / 2.0f; // Nível Y da superfície da água/terra
const float WATER_SWIM_CEILING = WATER_SURFACE_Y_LEVEL + 1.5f; // Limite superior para o nado do pinguim
const float WATER_BOTTOM_Y = -COORDINATES_Y + 1.0f;      // Limite inferior para o nado do pinguim e peixes
const float WATER_LEFT_BOUNDARY_X = 0.0f;        // Borda esquerda da área da água
const float WATER_RIGHT_BOUNDARY_X = COORDINATES_X; // Borda direita da área da água

// Constantes de Gameplay
const int TIME_BONUS_FROM_FEEDING = 60;     // Segundos adicionados ao alimentar o filhote
const int INITIAL_GAME_TIME = 60;           // Tempo inicial de "vida do pinguim" em segundos
const int MAX_SESSION_DURATION = 300;       // Duração máxima da sessão para condição de vitória (5 minutos)

// Variáveis Globais para o Pássaro Inimigo
enum EnemyState { FLYING_NORMAL, DIVING, RETURNING_TO_ALTITUDE }; // Estados possíveis do pássaro
EnemyState enemyState = FLYING_NORMAL;      // Estado inicial do pássaro
float enemyX = -COORDINATES_X + 1.0f;       // Posição X inicial do pássaro
float enemyY = COORDINATES_Y - 5.0f;        // Posição Y inicial do pássaro (altitude de voo)
const float ENEMY_NORMAL_ALTITUDE = COORDINATES_Y - 5.0f; // Altitude de voo padrão do pássaro
const float ENEMY_SPEED_X = 0.2f;           // Velocidade horizontal do pássaro
bool enemyGoingRight = true;                // Direção horizontal atual do pássaro
bool enemyHasDivedThisTurn = false;         // Flag para controlar um mergulho por travessia

// Variáveis para o Mergulho Parabólico do Pássaro
float diveStartX, diveStartY;               // Coordenadas de início do mergulho
float diveEndX, diveEndY;                   // Coordenadas de fim da fase parabólica do mergulho
float targetLowestPointY;                   // Altura mais baixa que a parábola deve atingir
float diveProgress = 0.0f;                  // Progresso do mergulho (0.0 a 1.0)
const float DIVE_HORIZONTAL_DISTANCE = 8.0f;// Distância horizontal do mergulho
float calculatedDiveExtraDepth;             // Profundidade extra da parábola (calculada dinamicamente)

// Variáveis Globais para o Pinguim Principal
float pinguinPositionX = -12.0f;            // Posição X atual do pinguim
float pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Posição Y atual do pinguim
float deltaX = 0.5f;                        // Incremento de movimento horizontal do pinguim
float deltaY = 0.5f;                        // Incremento de movimento vertical do pinguim
bool isGoingRight = true;                   // Direção horizontal lógica do pinguim
bool isGoingDown = true;                    // Direção vertical lógica do pinguim (ao nadar/mergulhar)

// Variáveis Globais de Estado do Jogo
int gameTimeRemaining;                      // Tempo restante para "vida do pinguim"
int sessionTimeRemainingForWin;             // Tempo restante para atingir a condição de vitória
int framesSinceLastSecond = 0;              // Contador de frames para lógica de 1 segundo
bool isGameOver = false;                    // Flag: jogo terminou por derrota
bool playerWon = false;                     // Flag: jogo terminou por vitória

// Variáveis Globais para o Pinguim Filhote
const float CHICK_POSITION_X = -18.0f;      // Posição X fixa do filhote
const float CHICK_POSITION_Y = WATER_SURFACE_Y_LEVEL; // Posição Y fixa do filhote

// Estrutura e Vetor para os Peixes
struct Fish {
    float x, y;                             // Posição do peixe
    bool isVisible;                         // Se o peixe deve ser desenhado
    float speedX;                           // Velocidade horizontal do peixe
    bool movingRight;                       // Direção horizontal do peixe
    Fish(float _x, float _y, float _speedX, bool _movingRight) : x(_x), y(_y), isVisible(true), speedX(_speedX), movingRight(_movingRight) {}
    void move() {                           // Lógica de movimento do peixe
        if (movingRight) { x += speedX; if (x > WATER_RIGHT_BOUNDARY_X - 0.5f) { x = WATER_RIGHT_BOUNDARY_X - 0.5f; movingRight = false; }
        } else { x -= speedX; if (x < WATER_LEFT_BOUNDARY_X + 0.5f) { x = WATER_LEFT_BOUNDARY_X + 0.5f; movingRight = true; } }
    }
    void respawn() {                        // Lógica de ressurgimento do peixe
        isVisible = true;
        y = WATER_BOTTOM_Y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WATER_SURFACE_Y_LEVEL - 1.0f - WATER_BOTTOM_Y)));
        if (rand() % 2 == 0) { x = WATER_LEFT_BOUNDARY_X + 0.5f; movingRight = true; } else { x = WATER_RIGHT_BOUNDARY_X - 0.5f; movingRight = false; }
        speedX = 0.1f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.2f));
    }
};
std::vector<Fish> fishes;                   // Vetor para armazenar todos os peixes
bool pinguinHasFish = false;                // Flag: se o pinguim principal está carregando um peixe

// Variáveis Globais para as Nuvens
float cloudPos1 = -12.0f;                   // Posição X da nuvem 1
float cloudPos2 = 5.0f;                     // Posição X da nuvem 2
float cloudPos3 = 18.0f;                    // Posição X da nuvem 3
float cloudSpeed = 0.03f;                   // Velocidade base de movimento das nuvens

// Protótipos das Funções
void init(void);
void display(void);
void timer(int value);
void keyboard(int key, int x, int y);
void keyboardNormal(unsigned char key, int x, int y);
void updateEnemy();
void checkCollisions();
bool isPinguinInWater();
void drawFishModel();
void drawPenguin(bool isCarryingFish, bool isInWaterFlag, bool pinguinCurrentlyGoingRight, bool pinguinCurrentlyGoingDownIfInWater);
void drawEnemy();
void drawEnemyWing();
void drawCloud();
void drawDisk(double radius);
void drawSquare(void);
void drawTriangle(void);
void drawElipse(double radiusX, double radiusY);

// Função Utilitária para Formatar Tempo (MM:SS)
std::string formatTime(int totalSeconds) {
    if (totalSeconds < 0) totalSeconds = 0;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

// Inicializa ou Reinicia o Estado do Jogo
void initializeGame() {
    fishes.clear();
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < 4; ++i) { // Cria e posiciona os peixes
        float startY = WATER_BOTTOM_Y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WATER_SURFACE_Y_LEVEL - 1.0f - WATER_BOTTOM_Y)));
        float startX = WATER_LEFT_BOUNDARY_X + 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WATER_RIGHT_BOUNDARY_X - 1.0f - (WATER_LEFT_BOUNDARY_X + 0.5f) )));
        float speed = 0.1f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.2f));
        bool startsRight = rand() % 2;
        fishes.push_back(Fish(startX, startY, speed, startsRight));
    }
    pinguinHasFish = false; // Pinguim começa sem peixe
    isGameOver = false;     // Jogo não está em game over
    playerWon = false;      // Jogador ainda não ganhou
    gameTimeRemaining = INITIAL_GAME_TIME; // Reseta tempo de vida do pinguim
    sessionTimeRemainingForWin = MAX_SESSION_DURATION; // Reseta tempo para condição de vitória
    framesSinceLastSecond = 0; // Reseta contador de frames para o timer
    pinguinPositionX = -12.0f; // Reseta posição X do pinguim
    pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Reseta posição Y do pinguim
    isGoingRight = true;    // Pinguim começa olhando para a direita
    isGoingDown = true;     // Estado inicial de mergulho (se entrar na água)

    // Inicializa estado do pássaro inimigo
    enemyX = -COORDINATES_X + 1.0f;
    enemyY = ENEMY_NORMAL_ALTITUDE;
    enemyGoingRight = true;
    enemyState = FLYING_NORMAL;
    enemyHasDivedThisTurn = false;

    // Inicializa posição das nuvens
    cloudPos1 = -12.0f;
    cloudPos2 = 5.0f;
    cloudPos3 = 18.0f;
}

// Função Chamada Periodicamente pelo GLUT para Atualizações de Lógica
void timer(int value)
{
  if (!isGameOver && !playerWon) { // Se o jogo está em andamento
    for (Fish &fish : fishes) { // Atualiza cada peixe
        if (fish.isVisible) {
            fish.move();
        }
    }
    updateEnemy(); // Atualiza lógica do pássaro
    checkCollisions(); // Verifica colisões e alimentação

    // Atualiza posição das nuvens
    cloudPos1 += cloudSpeed;
    if (cloudPos1 > COORDINATES_X + 4) // Se nuvem 1 saiu da tela pela direita
        cloudPos1 = -COORDINATES_X - 4 - (rand() % 3); // Reposiciona na esquerda com variação

    cloudPos2 += cloudSpeed * 0.8f; // Nuvem 2 se move um pouco mais lenta
     if (cloudPos2 > COORDINATES_X + 3)
        cloudPos2 = -COORDINATES_X - 3 - (rand() % 3);

    cloudPos3 += cloudSpeed * 1.2f; // Nuvem 3 se move um pouco mais rápida
     if (cloudPos3 > COORDINATES_X + 5)
        cloudPos3 = -COORDINATES_X - 5 - (rand() % 3);
  }

  if (!isGameOver && !playerWon) { // Lógica dos timers do jogo
    framesSinceLastSecond++;
    if (framesSinceLastSecond >= FPS) { // Passou um segundo
        if (gameTimeRemaining > 0) { // Decrementa vida do pinguim
            gameTimeRemaining--;
        } else {
            isGameOver = true; // Vida do pinguim acabou
        }

        if (sessionTimeRemainingForWin > 0) { // Decrementa tempo para vitória
            sessionTimeRemainingForWin--;
        } else { // Tempo da sessão acabou
            if (gameTimeRemaining > 0) { // Se pinguim ainda tem vida
                playerWon = true; // Jogador ganhou
            } else { // Se pinguim também não tem vida
                isGameOver = true;
            }
        }
        framesSinceLastSecond = 0; // Reseta contador de frames
    }
  }
  glutPostRedisplay(); // Solicita que a tela seja redesenhada
  glutTimerFunc(DELAY, timer, 0); // Reagenda o timer
}

// Função de Inicialização do OpenGL
void init(void)
{
  glClearColor(0.6f, 0.8f, 1.0f, 0); // Define a cor de fundo padrão (céu azul claro)
  glMatrixMode(GL_PROJECTION);        // Seleciona a matriz de projeção
  glLoadIdentity();                   // Reseta a matriz de projeção
  // Define uma projeção ortográfica 2D
  glOrtho(-COORDINATES_X, COORDINATES_X, -COORDINATES_Y, COORDINATES_Y, -COORDINATES_Z, COORDINATES_Z);
  initializeGame();                   // Inicializa as variáveis de estado do jogo
}

// Verifica se o Pinguim Está na Água
bool isPinguinInWater()
{
  // Considera na água se X >= borda da água E Y <= superfície da água
  return pinguinPositionX >= WATER_LEFT_BOUNDARY_X && pinguinPositionY <= WATER_SURFACE_Y_LEVEL;
}

// Desenha um Quadrado Centrado na Origem de Tamanho 2x2
void drawSquare()
{
  glBegin(GL_POLYGON); // Inicia desenho de polígono
    glVertex3f(-1, -1, 0); // Vértice inferior esquerdo
    glVertex3f(1, -1, 0);  // Vértice inferior direito
    glVertex3f(1, 1, 0);   // Vértice superior direito
    glVertex3f(-1, 1, 0);  // Vértice superior esquerdo
  glEnd(); // Finaliza desenho de polígono
}

// Desenha um Triângulo Isósceles Centrado na Base com Altura 1 e Base 2
void drawTriangle()
{
  glBegin(GL_POLYGON); // Inicia desenho de polígono
    glVertex3f(-1, 0, 0); // Vértice da base esquerda
    glVertex3f(0, 1, 0);  // Vértice do topo
    glVertex3f(1, 0, 0);  // Vértice da base direita
  glEnd(); // Finaliza desenho de polígono
}

// Desenha um Disco (Círculo Preenchido) Centrado na Origem
void drawDisk(double radius)
{
  glBegin(GL_POLYGON); // Inicia desenho de polígono
  for (int i = 0; i < 32; i++) // Aproxima círculo com 32 segmentos
  {
    double disk_angle = (2 * M_PI / 32) * i; // Ângulo para o vértice atual
    // Calcula coordenadas X e Y do vértice na circunferência
    glVertex3f(radius * cos(disk_angle), radius * sin(disk_angle), 0);
  }
  glEnd(); // Finaliza desenho de polígono
}

// Desenha uma Elipse Centrada na Origem
void drawElipse(double radiusX, double radiusY)
{
  glBegin(GL_POLYGON); // Inicia desenho de polígono
  for (int i = 0; i < 32; i++){ // Aproxima elipse com 32 segmentos
    double ellipse_angle = (2 * M_PI / 32) * i; // Ângulo para o vértice atual
    // Calcula coordenadas X e Y do vértice na elipse
    glVertex3f(radiusX * cos(ellipse_angle), radiusY * sin(ellipse_angle), 0);
  }
  glEnd(); // Finaliza desenho de polígono
}

// Desenha uma Nuvem Composta por Vários Discos Sobrepostos
void drawCloud()
{
    glColor3f(1.0f, 1.0f, 1.0f); // Define a cor da nuvem como branca

    glPushMatrix(); // Salva a matriz atual para o disco central
        drawDisk(1.0); // Disco central/inferior da nuvem
    glPopMatrix(); // Restaura a matriz

    glPushMatrix(); // Salva a matriz para a parte esquerda da nuvem
        glTranslatef(-0.8f, 0.1f, 0.0f); // Desloca para a esquerda e um pouco para cima
        glScalef(0.8f, 0.8f, 1.0f);      // Torna um pouco menor
        drawDisk(1.0); // Desenha parte esquerda
    glPopMatrix(); // Restaura a matriz

    glPushMatrix(); // Salva a matriz para a parte direita da nuvem
        glTranslatef(0.7f, 0.2f, 0.0f); // Desloca para a direita e um pouco mais para cima
        glScalef(0.9f, 0.9f, 1.0f);      // Tamanho ligeiramente diferente
        drawDisk(1.0); // Desenha parte direita
    glPopMatrix(); // Restaura a matriz

    glPushMatrix(); // Salva a matriz para a parte superior da nuvem
        glTranslatef(0.1f, 0.5f, 0.0f); // Desloca para cima
        glScalef(0.6f, 0.6f, 1.0f);     // Torna menor ainda
        drawDisk(1.0); // Desenha parte superior
    glPopMatrix(); // Restaura a matriz
}

// Desenha a Porção de Terra do Cenário
void drawFloor()
{
  glColor3f(0.13f, 0.55f, 0.13f); // Define a cor do chão (verde escuro)
  glBegin(GL_POLYGON); // Inicia desenho do polígono do chão
    glVertex3f(0, WATER_SURFACE_Y_LEVEL, 0);                 // Canto superior direito da terra (encontra a água)
    glVertex3f(-COORDINATES_X, WATER_SURFACE_Y_LEVEL, 0);    // Canto superior esquerdo da terra
    glVertex3f(-COORDINATES_X, -COORDINATES_Y, 0);           // Canto inferior esquerdo da terra
    glVertex3f(0, -COORDINATES_Y, 0);                        // Canto inferior direito da terra
  glEnd(); // Finaliza desenho do polígono
}

// Desenha a Porção de Água do Cenário
void drawWater()
{
  glColor3f(0.4, 0.4, 1); // Define a cor da água (azul)
  glBegin(GL_POLYGON); // Inicia desenho do polígono da água
    glVertex3f(WATER_LEFT_BOUNDARY_X, WATER_SURFACE_Y_LEVEL, 0); // Canto superior esquerdo da água
    glVertex3f(WATER_RIGHT_BOUNDARY_X, WATER_SURFACE_Y_LEVEL, 0); // Canto superior direito da água
    glVertex3f(WATER_RIGHT_BOUNDARY_X, -COORDINATES_Y, 0);        // Canto inferior direito da água
    glVertex3f(WATER_LEFT_BOUNDARY_X, -COORDINATES_Y, 0);         // Canto inferior esquerdo da água
  glEnd(); // Finaliza desenho do polígono
}

// Desenha o Modelo Base de um Peixe
void drawFishModel() {
  glPushMatrix(); // Salva matriz para o rabo do peixe
    glColor3f(0.8f, 0.4f, 0.0f); // Define cor do rabo (laranja)
    glTranslatef(-1.5f * 0.5f, 0, 0.5f); // Posiciona o rabo
    glRotatef(270, 0, 0, 1);             // Rotaciona o rabo
    glScalef(1.0f * 0.5f, 0.5f * 0.5f, 1); // Escala o rabo
    drawTriangle();                        // Desenha o rabo como um triângulo
  glPopMatrix(); // Restaura matriz

  glPushMatrix(); // Salva matriz para o corpo do peixe
    glColor3f(0.8f, 0.4f, 0.0f); // Define cor do corpo (laranja)
    glTranslatef(0, 0, 0.5f);    // Posiciona o corpo (Z para ficar na frente)
    glScalef(1.0f * 0.5f, 1.0f * 0.5f, 1); // Escala o corpo
    drawElipse(1, 0.5);            // Desenha o corpo como uma elipse
  glPopMatrix(); // Restaura matriz
}

// Desenha o Personagem Pinguim
void drawPenguin(bool isCarryingFish, bool isInWaterFlag, bool pinguinCurrentlyGoingRight, bool pinguinCurrentlyGoingDownIfInWater)
{
  glPushMatrix(); // Pata de trás
    glColor3f(1, 0, 0); // Cor vermelha
    glTranslatef(-0.5, 0, 0.2f); // Posiciona
    glScalef(0.5, 1.5, 1);    // Escala
    drawTriangle();           // Desenha
  glPopMatrix();

  glPushMatrix(); // Pata da frente
    glColor3f(1, 0, 0); // Cor vermelha
    glTranslatef(0.5, 0, 0.2f);  // Posiciona
    glScalef(0.5, 1.5, 1);     // Escala
    drawTriangle();            // Desenha
  glPopMatrix();

  glPushMatrix(); // Corpo principal
    glColor3f(0, 0, 0); // Cor preta
    glTranslatef(0, 3.7, 0); // Posiciona (mais alto)
    glScalef(2, 2.5, 1);   // Escala (largo e alto)
    drawElipse(0.5, 1);    // Desenha como elipse vertical
  glPopMatrix();

  glPushMatrix(); // Barriga
    glColor3f(1, 1, 1); // Cor branca
    glTranslatef(0.0, 3.7, 0.1f); // Posição similar ao corpo, um pouco à frente (Z)
    glScalef(0.8, 1.5, 1);    // Escala (mais fina que o corpo)
    drawElipse(0.3, 1);       // Desenha como elipse vertical
  glPopMatrix();

  glPushMatrix(); // Cabeça
    glColor3f(0, 0, 0); // Cor preta
    glTranslatef(0, 7, 0.1f); // Posiciona acima do corpo
    glScalef(1, 1, 1);      // Escala
    drawDisk(0.8);          // Desenha como disco
  glPopMatrix();

  glPushMatrix(); // Olho (parte branca)
    glColor3f(1, 1, 1); // Cor branca
    glTranslatef(0.25, 7, 0.2f); // Posição na cabeça
    glScalef(1, 1, 1);       // Escala
    drawDisk(0.3);           // Desenha
  glPopMatrix();
      glPushMatrix(); // Pupila
        glColor3f(0,0,0); // Cor preta
        glTranslatef(0.25, 7, 0.25f); // Posição sobre o olho branco, um pouco à frente (Z)
        glScalef(1,1,1);        // Escala
        drawDisk(0.1);            // Desenha (menor)
      glPopMatrix();

  // Variáveis locais para o bico, para clareza
  float beakBaseX = 0.7f;
  float beakBaseY = 6.5f;
  float beakBaseZ = 0.2f;
  float beakRotation = 20.0f;
  float beakScale = 0.6f;

  glPushMatrix(); // Bico
    glColor3f(0.8, 0.8, 0); // Cor amarela
    glTranslatef(beakBaseX, beakBaseY, beakBaseZ); // Posição base do bico
    glRotatef(beakRotation, 0, 0, 1);       // Rotaciona o bico
    glScalef(beakScale, beakScale, 1);        // Escala o bico
    drawTriangle();                         // Desenha como triângulo
  glPopMatrix();


  if (isCarryingFish) { // Se o pinguim está carregando um peixe
      glPushMatrix(); // Matriz de transformação para o peixe no bico
          if (isInWaterFlag) { // Se pinguim está na água (nadando)
            // Define a posição local da ponta do bico no modelo do pinguim em pé
            float pinguinModel_beakTipX = beakBaseX + 0.7f; // Um pouco à frente da base do bico
            float pinguinModel_beakTipY = beakBaseY;       // Na mesma altura Y da base do bico
            float pinguinModel_beakTipZ = beakBaseZ + 0.1f; // Um pouco à frente no Z

            glTranslatef(pinguinModel_beakTipX, pinguinModel_beakTipY, pinguinModel_beakTipZ); // Move para a ponta do bico (no sistema já rotacionado/escalado do pinguim)
            glRotatef(90, 0, 0, 1); // Deita o peixe para alinhar com a direção de nado
            if (!pinguinCurrentlyGoingRight) { // Se o pinguim nada para a esquerda
                 glRotatef(180, 0, 1, 0); // Vira o modelo do peixe para que ele também aponte para a esquerda
            }
          } else { // Se pinguim está em pé
            float pinguinModel_beakTipX = beakBaseX + 0.7f;
            float pinguinModel_beakTipY = beakBaseY;
            float pinguinModel_beakTipZ = beakBaseZ + 0.1f;
            glTranslatef(pinguinModel_beakTipX, pinguinModel_beakTipY, pinguinModel_beakTipZ); // Move para a ponta do bico
            glRotatef(beakRotation, 0,0,1); // Alinha o peixe com a rotação original do bico
            // A direção (esquerda/direita) do pinguim em pé já foi tratada pela escala em display()
          }
          glScalef(0.7f, 0.7f, 0.7f); // Escala o peixe para ser menor no bico
          drawFishModel(); // Desenha o modelo do peixe
      glPopMatrix(); // Restaura matriz após desenhar o peixe
  }
}

// Desenha uma Asa para o Pássaro Inimigo
void drawEnemyWing() {
    glColor3f(0.35f, 0.35f, 0.85f); // Cor da asa (azul um pouco mais claro)
    glBegin(GL_POLYGON); // Inicia desenho da asa como polígono
        glVertex2f(0.0f, 0.0f);    // Vértice 1: Ponto de articulação da asa (ombro), origem local da asa
        glVertex2f(-0.2f, 0.3f);  // Vértice 2: Canto superior interno da asa (próximo ao corpo)
        glVertex2f(-2.0f, 0.1f);   // Vértice 3: Ponta externa superior da asa (mais longa, para trás)
        glVertex2f(-1.8f, -0.1f);  // Vértice 4: Borda inferior da ponta da asa
        glVertex2f(-0.1f, -0.2f); // Vértice 5: Canto inferior interno da asa
    glEnd(); // Finaliza desenho da asa
}

// Desenha o Pássaro Inimigo
void drawEnemy(){
  glPushMatrix(); // Salva matriz de transformação global (do translate do pássaro em display)
    if (!enemyGoingRight) { // Se o pássaro está voando para a esquerda
        glScalef(-1.0f, 1.0f, 1.0f); // Espelha todo o desenho do pássaro horizontalmente
    }

    // Corpo do pássaro
    glPushMatrix(); // Salva matriz atual (já espelhada, se for o caso)
      glColor3f(0.3f, 0.3f, 0.8f); // Cor do corpo (azul escuro)
      glScalef(2.0f, 1.5f, 1.0f);   // Escala o corpo para ser uma elipse mais larga que alta
      drawElipse(1.0f, 1.0f);       // Desenha o corpo base como um círculo (que vira elipse com a escala)
    glPopMatrix(); // Restaura matriz (remove a escala do corpo)

    // Asa do pássaro
    glPushMatrix(); // Salva matriz atual
      glTranslatef(-0.5f, 0.3f, -0.1f); // Posiciona o "ombro" da asa em relação ao corpo
      // Calcula o ângulo de batida da asa baseado no tempo, para animação
      float wingFlapAngle = 40.0f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.020) - 10.0f;
      glRotatef(wingFlapAngle, 0, 0, 1); // Rotaciona a asa para cima/baixo (em torno do eixo Z local)
      drawEnemyWing(); // Desenha a asa usando sua função dedicada
    glPopMatrix(); // Restaura matriz

    // Olho do pássaro
    glPushMatrix(); // Salva matriz atual
      glColor3f(1, 1, 1); // Cor do olho (branco)
      glTranslatef(1.4f, 0.3f, 0.1f); // Posiciona o olho na "frente" do corpo
      drawDisk(0.3); // Desenha a parte branca do olho
      // Pupila
      glColor3f(0.0f, 0.0f, 0.0f); // Cor da pupila (preta)
      glTranslatef(0.05f, 0.0f, 0.01f); // Pequeno deslocamento para a pupila, e Z para ficar na frente
      drawDisk(0.15); // Desenha a pupila (menor)
    glPopMatrix(); // Restaura matriz

    // Bico do pássaro
    glPushMatrix(); // Salva matriz atual
      glColor3f(0.8, 0.8, 0); // Cor do bico (amarelo)
      glTranslatef(2.3f, 0.0f, 0.05f); // Posiciona o bico bem à frente do corpo
      glRotatef(50, 0, 0, 1);    // Rotaciona o bico para cima
      glScalef(0.6, 0.6, 1);   // Escala o bico
      drawTriangle();            // Desenha o bico como um triângulo
    glPopMatrix(); // Restaura matriz

  glPopMatrix(); // Restaura matriz de transformação global (remove espelhamento)
}

// Manipulador de Teclas Especiais (Setas, etc.)
void keyboard(int key, int x, int y)
{
  if (isGameOver || playerWon) { return; } // Ignora input se o jogo terminou
  float oldPinguinX = pinguinPositionX; // Guarda posição X antes do movimento
  bool wasInWater = isPinguinInWater(); // Verifica se estava na água antes

  switch (key) { // Verifica qual tecla foi pressionada
    case GLUT_KEY_LEFT: // Seta para esquerda
      if (pinguinPositionX > -COORDINATES_X + deltaX/2.0f) { // Limite esquerdo da tela
          pinguinPositionX -= deltaX;
      } else {
          pinguinPositionX = -COORDINATES_X; // Trava na borda
      }
      isGoingRight = false; // Define direção lógica
    break;
    case GLUT_KEY_RIGHT: // Seta para direita
    if (pinguinPositionX < COORDINATES_X - deltaX/2.0f ){ // Limite direito da tela
      pinguinPositionX += deltaX;
    } else {
        pinguinPositionX = COORDINATES_X; // Trava na borda
    }
    isGoingRight = true; // Define direção lógica
    break;
    case GLUT_KEY_DOWN: // Seta para baixo (mergulhar)
    if (isPinguinInWater() && pinguinPositionY > WATER_BOTTOM_Y + deltaY){ // Só se estiver na água e acima do fundo
      pinguinPositionY -= deltaY;
      isGoingDown = true; // Define direção lógica de mergulho
    }
    break;
    case GLUT_KEY_UP: // Seta para cima (subir na água)
    if (isPinguinInWater() && pinguinPositionY < WATER_SWIM_CEILING - deltaY) { // Só se estiver na água e abaixo do teto de nado
      pinguinPositionY += deltaY;
      isGoingDown = false; // Define direção lógica de mergulho
    }
    break;
    default: break; // Ignora outras teclas especiais
  }

  bool currentlyInWater = isPinguinInWater(); // Verifica se está na água APÓS o movimento

  // Lógica de transição Terra <-> Água para ajustar a altura Y do pinguim
  if (!wasInWater && currentlyInWater) { // Se acabou de entrar na água
      pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Ajusta Y para a superfície da água
  } else if (wasInWater && !currentlyInWater) { // Se acabou de sair da água
      pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Ajusta Y para a superfície da terra
  } else if (!currentlyInWater) { // Se continua na terra (ou foi para a terra)
      pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Mantém/Ajusta Y para a superfície da terra
  }
  glutPostRedisplay(); // Solicita redesenho
}

// Manipulador de Teclas Normais (ASCII)
void keyboardNormal(unsigned char key, int x, int y)
{
    if (isGameOver || playerWon) { // Se o jogo terminou (perdeu ou ganhou)
        if (key == 'r' || key == 'R') { // Tecla 'R' ou 'r' para reiniciar
            initializeGame(); // Reinicia o estado do jogo
            glutPostRedisplay(); // Solicita redesenho
        }
    }
    if (key == 27) { // Tecla ESC para sair
        exit(0);
    }
}

// Atualiza o Estado e Posição do Pássaro Inimigo
void updateEnemy() {
    if (isGameOver || playerWon) return; // Não atualiza se o jogo terminou

    switch (enemyState) { // Máquina de estados para o comportamento do pássaro
        case FLYING_NORMAL: // Estado: voando normalmente na horizontal
            // Movimento horizontal
            if (enemyGoingRight) {
                enemyX += ENEMY_SPEED_X;
                if (enemyX > COORDINATES_X + 2.0f) { // Se passou da borda direita (com buffer)
                    enemyX = COORDINATES_X + 2.0f; // Trava na borda estendida
                    enemyGoingRight = false;       // Inverte direção
                    enemyHasDivedThisTurn = false; // Permite novo mergulho no próximo percurso
                }
            } else { // Indo para a esquerda
                enemyX -= ENEMY_SPEED_X;
                if (enemyX < -COORDINATES_X - 2.0f) { // Se passou da borda esquerda (com buffer)
                    enemyX = -COORDINATES_X - 2.0f; // Trava na borda estendida
                    enemyGoingRight = true;        // Inverte direção
                    enemyHasDivedThisTurn = false;  // Permite novo mergulho
                }
            }
            enemyY = ENEMY_NORMAL_ALTITUDE; // Mantém altitude de voo normal

            // Lógica para iniciar um mergulho
            if (!enemyHasDivedThisTurn) { // Se ainda não mergulhou neste percurso
                float diveTriggerPoint; // Ponto X aleatório para iniciar o mergulho
                if (enemyGoingRight) { diveTriggerPoint = -5.0f + static_cast<float>(rand() % 150) / 10.0f; } // Sorteia ponto
                else { diveTriggerPoint = 5.0f - static_cast<float>(rand() % 150) / 10.0f; }

                bool shouldDive = false; // Flag para decidir se mergulha
                // Verifica se o pássaro está na zona de gatilho do mergulho
                if (enemyGoingRight && enemyX > diveTriggerPoint && enemyX < COORDINATES_X - DIVE_HORIZONTAL_DISTANCE -1.0f ) shouldDive = true;
                if (!enemyGoingRight && enemyX < diveTriggerPoint && enemyX > -COORDINATES_X + DIVE_HORIZONTAL_DISTANCE + 1.0f) shouldDive = true;

                if (shouldDive && (rand() % 100 < 2) ) { // Pequena chance (2%) de mergulhar por frame na zona
                    enemyState = DIVING; // Muda estado para mergulhando
                    enemyHasDivedThisTurn = true; // Marca que já mergulhou neste percurso
                    // Salva posições iniciais e finais do mergulho
                    diveStartX = enemyX; diveStartY = enemyY; diveProgress = 0.0f;
                    if (enemyGoingRight) { diveEndX = diveStartX + DIVE_HORIZONTAL_DISTANCE; } else { diveEndX = diveStartX - DIVE_HORIZONTAL_DISTANCE; }
                    // Calcula altura final da fase parabólica do mergulho
                    float minDiveEndY = WATER_SURFACE_Y_LEVEL; float maxDiveEndY = ENEMY_NORMAL_ALTITUDE - 2.0f;
                    if (maxDiveEndY < minDiveEndY) maxDiveEndY = minDiveEndY + 0.1f; // Garante intervalo válido
                    diveEndY = minDiveEndY + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxDiveEndY - minDiveEndY + 0.001f)));
                    // Calcula a altura mais baixa que a parábola atingirá
                    targetLowestPointY = WATER_SURFACE_Y_LEVEL + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (diveEndY - WATER_SURFACE_Y_LEVEL + 0.001f)));
                    if (targetLowestPointY > diveEndY) targetLowestPointY = diveEndY; if (targetLowestPointY < WATER_SURFACE_Y_LEVEL) targetLowestPointY = WATER_SURFACE_Y_LEVEL;
                    // Calcula a deflexão da parábola
                    float y_mid_linear = diveStartY + (diveEndY - diveStartY) * 0.5f;
                    calculatedDiveExtraDepth = targetLowestPointY - y_mid_linear;
                    if (diveEndY <= WATER_SURFACE_Y_LEVEL + 0.1f || (calculatedDiveExtraDepth > 0 && targetLowestPointY > y_mid_linear) ) { calculatedDiveExtraDepth = 0; }
                }
            }
            break;
        case DIVING: // Estado: pássaro está mergulhando
            if (enemyGoingRight) { enemyX += ENEMY_SPEED_X; } else { enemyX -= ENEMY_SPEED_X; } // Move horizontalmente
            diveProgress = fabs(enemyX - diveStartX) / DIVE_HORIZONTAL_DISTANCE; if (diveProgress > 1.0f) diveProgress = 1.0f; // Atualiza progresso do mergulho
            // Calcula Y usando a equação da parábola
            enemyY = diveStartY + (diveEndY - diveStartY) * diveProgress + calculatedDiveExtraDepth * (4.0f * diveProgress * (1.0f - diveProgress));
            if (enemyY < WATER_SURFACE_Y_LEVEL) enemyY = WATER_SURFACE_Y_LEVEL; // Garante que não passe do chão

            // Verifica se o mergulho terminou
            if ((enemyGoingRight && enemyX >= diveEndX) || (!enemyGoingRight && enemyX <= diveEndX) || diveProgress >= 1.0f) {
                enemyState = RETURNING_TO_ALTITUDE; // Muda estado para subir
                enemyY = diveEndY; if (enemyY < WATER_SURFACE_Y_LEVEL) enemyY = WATER_SURFACE_Y_LEVEL; // Garante altura final correta
            }
            break;
        case RETURNING_TO_ALTITUDE: // Estado: pássaro está subindo de volta à altitude normal
            // Movimento horizontal e inversão de direção nas bordas da tela
            if (enemyGoingRight) {
                enemyX += ENEMY_SPEED_X;
                if (enemyX > COORDINATES_X) { enemyX = COORDINATES_X; enemyGoingRight = false; enemyHasDivedThisTurn = false; }
            } else {
                enemyX -= ENEMY_SPEED_X;
                if (enemyX < -COORDINATES_X) { enemyX = -COORDINATES_X; enemyGoingRight = true; enemyHasDivedThisTurn = false; }
            }
            // Lógica de subida
            if (enemyY < ENEMY_NORMAL_ALTITUDE) { enemyY += 0.15f; if (enemyY >= ENEMY_NORMAL_ALTITUDE) { enemyY = ENEMY_NORMAL_ALTITUDE; enemyState = FLYING_NORMAL; }
            } else { enemyY = ENEMY_NORMAL_ALTITUDE; enemyState = FLYING_NORMAL; }
            break;
    }
}

// Verifica Colisões e Lógica de Alimentação
void checkCollisions() {
    if (isGameOver || playerWon) return; // Não verifica se o jogo terminou

    // 1. Lógica para alimentar o filhote
    if (pinguinHasFish) { // Se o pinguim está carregando um peixe
        float dx_to_chick = pinguinPositionX - CHICK_POSITION_X;
        float dy_to_chick = pinguinPositionY - CHICK_POSITION_Y;
        float distance_to_chick = sqrt(dx_to_chick * dx_to_chick + dy_to_chick * dy_to_chick);
        // Se perto do filhote E na terra
        if (distance_to_chick < (PINGUIN_COLLISION_RADIUS + CHICK_COLLISION_RADIUS) && pinguinPositionX < WATER_LEFT_BOUNDARY_X) {
            pinguinHasFish = false; // Pinguim solta o peixe
            gameTimeRemaining += TIME_BONUS_FROM_FEEDING; // Adiciona tempo
        }
    }

    // 2. Lógica para capturar peixes
    if (!pinguinHasFish) { // Se o pinguim não está carregando peixe
        for (Fish &fish : fishes) { // Itera sobre os peixes
            if (fish.isVisible) { // Se o peixe está visível
                float dx_col = pinguinPositionX - fish.x;
                float dy_col = pinguinPositionY - fish.y;
                float distance = sqrt(dx_col * dx_col + dy_col * dy_col);
                if (distance < (PINGUIN_COLLISION_RADIUS + FISH_COLLISION_RADIUS)) { // Se colidiu
                    if (isPinguinInWater()) { // E pinguim está na água
                        pinguinHasFish = true; // Pinguim pega o peixe
                        fish.respawn();        // Peixe reaparece em outro lugar
                        break; // Para de verificar outros peixes
                    }
                }
            }
        }
    }

    // 3. Colisão Pássaro-Pinguim
    if (!isPinguinInWater()) { // Se o pinguim está na terra
        float pinguinCenterX = pinguinPositionX;
        float pinguinEffectiveY = pinguinPositionY + 3.5f; // Estimativa do centro vertical do pinguim
        float dx_enemy_pinguin = enemyX - pinguinCenterX;
        float dy_enemy_pinguin = enemyY - pinguinEffectiveY;
        float distance_enemy_pinguin = sqrt(dx_enemy_pinguin * dx_enemy_pinguin + dy_enemy_pinguin * dy_enemy_pinguin);
        if (distance_enemy_pinguin < (ENEMY_EFFECTIVE_COLLISION_RADIUS + PINGUIN_COLLISION_RADIUS)) { // Se colidiu
            isGameOver = true; // Fim de jogo
        }
    }
}

// Função Principal de Desenho (Callback do GLUT)
void display()
{
  glClear(GL_COLOR_BUFFER_BIT); // Limpa o buffer de cor com a cor definida por glClearColor
  glMatrixMode(GL_MODELVIEW);   // Seleciona a matriz de modelview
  glLoadIdentity();             // Reseta a matriz de modelview

  if (playerWon) { // Se o jogador ganhou
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Define fundo branco para tela de vitória
      glClear(GL_COLOR_BUFFER_BIT);         // Limpa com a nova cor
      glColor3f(0.0f, 0.5f, 0.0f);          // Cor do texto de vitória (verde)
      std::string winMsgText = "VOCE GANHOU!";
      float textWidthEstimate = winMsgText.length() * 1.0f; // Estimativa da largura do texto
      glRasterPos2f(-textWidthEstimate / 2.0f, 2.0f);    // Posiciona o texto
      for (char c : winMsgText) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); } // Desenha texto
      std::string restartMsgText = "Press 'R' to Restart";
      float restartTextWidthEstimate = restartMsgText.length() * 0.4f;
      glRasterPos2f(-restartTextWidthEstimate / 2.0f, -2.0f);
      for (char c : restartMsgText) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); }
  } else if (isGameOver) { // Se o jogo acabou (derrota)
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Define fundo branco para tela de game over
      glClear(GL_COLOR_BUFFER_BIT);         // Limpa com a nova cor
      glColor3f(0.0f, 0.0f, 0.0f);          // Cor do texto de game over (preto)
      std::string gameOverMsgText = "GAME OVER";
      float textWidthEstimate = gameOverMsgText.length() * 0.8f;
      glRasterPos2f(-textWidthEstimate / 2.0f, 2.0f);
      for (char c : gameOverMsgText) { glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); }
      std::string restartMsgText = "Press 'R' to Restart";
      float restartTextWidthEstimate = restartMsgText.length() * 0.4f;
      glRasterPos2f(-restartTextWidthEstimate / 2.0f, -2.0f);
      for (char c : restartMsgText) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); }
  } else { // Se o jogo está em andamento
    glClearColor(0.6f, 0.8f, 1.0f, 0); // Restaura cor do céu para o jogo

    // Desenha as nuvens no fundo
    glPushMatrix(); glTranslatef(cloudPos1, COORDINATES_Y - 8.0f, 0.0f); glScalef(2.5f, 1.8f, 1.0f); drawCloud(); glPopMatrix();
    glPushMatrix(); glTranslatef(cloudPos2, COORDINATES_Y - 5.0f, 0.0f); glScalef(2.0f, 1.5f, 1.0f); drawCloud(); glPopMatrix();
    glPushMatrix(); glTranslatef(cloudPos3, COORDINATES_Y - 10.0f, 0.0f); glScalef(3.0f, 2.0f, 1.0f); drawCloud(); glPopMatrix();

    // Desenha o cenário e os personagens
    drawFloor(); // Chão
    drawWater(); // Água
    for (const Fish &fish : fishes) { if (fish.isVisible) { glPushMatrix(); glTranslated(fish.x, fish.y, 0.4f); if (!fish.movingRight) { glRotatef(180, 0, 1, 0); } glScalef(0.8f, 0.8f, 1.0f); drawFishModel(); glPopMatrix(); } } // Peixes
    
    // Pinguim Principal
    glPushMatrix();
      glTranslated(pinguinPositionX, pinguinPositionY, 0); // Posiciona pinguim
      bool currentIsInWater = isPinguinInWater(); // Verifica se está na água
      float scaleX_for_pinguin_body, scaleY_for_pinguin_body; // Escalas a serem aplicadas
      if (currentIsInWater) { // Se na água
          glRotatef(270, 0, 0, 1); // Deita o pinguim
          scaleX_for_pinguin_body = isGoingDown ? 0.8f : -0.8f; // Escala X local (bico cima/baixo)
          scaleY_for_pinguin_body = isGoingRight ? 0.8f : -0.8f;// Escala Y local (corpo esq/dir)
      } else { // Se na terra
          scaleX_for_pinguin_body = isGoingRight ? 0.8f : -0.8f; // Escala X (direção horizontal)
          scaleY_for_pinguin_body = 0.8f;                       // Escala Y padrão
      }
      glScalef(scaleX_for_pinguin_body, scaleY_for_pinguin_body, 1); // Aplica escalas
      drawPenguin(pinguinHasFish, currentIsInWater, isGoingRight, isGoingDown); // Desenha pinguim
    glPopMatrix();
    
    // Pinguim Filhote
    glPushMatrix();
      glTranslated(CHICK_POSITION_X, CHICK_POSITION_Y, 0); // Posiciona filhote
      float filhoteScaleFactor = 0.5f; // Fator de escala para o filhote
      glScalef(filhoteScaleFactor, filhoteScaleFactor, 1); // Aplica escala geral
      bool filhoteLooksRight = true; // Direção do filhote
      if (pinguinPositionX < WATER_LEFT_BOUNDARY_X && pinguinHasFish) { // Se pinguim principal está na terra com peixe
          if (pinguinPositionX < CHICK_POSITION_X) filhoteLooksRight = false; else filhoteLooksRight = true; // Filhote olha para o pinguim
      }
      glScalef(filhoteLooksRight ? 1.0f : -1.0f, 1.0f, 1.0f); // Aplica escala de direção
      drawPenguin(false, false, true, false); // Desenha filhote (nunca carrega peixe, sempre em pé, olhando para dir por padrão)
    glPopMatrix();
    
    // Pássaro Inimigo
    glPushMatrix();
      glTranslated(enemyX, enemyY, 0); // Posiciona pássaro
      drawEnemy(); // Desenha pássaro
    glPopMatrix();

    // Desenha a UI (textos de tempo)
    glColor3f(0.0f, 0.0f, 0.0f); // Cor preta para o texto
    glRasterPos2f(-COORDINATES_X + 1.0f, COORDINATES_Y - 2.0f); // Posição para "Vida do pinguim"
    std::string pinguinLifeStr = "Vida do pinguim: " + formatTime(gameTimeRemaining);
    for (char c : pinguinLifeStr) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); }
    glRasterPos2f(-COORDINATES_X + 1.0f, COORDINATES_Y - 4.0f); // Posição para "Vitória em"
    std::string winConditionStr = "Vitoria em: " + formatTime(sessionTimeRemainingForWin);
    for (char c : winConditionStr) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); }
  }
  glutSwapBuffers(); // Troca os buffers (necessário para GLUT_DOUBLE)
}

// Função Principal do Programa
int main(int argc, char **argv)
{
  glutInit(&argc, argv); // Inicializa GLUT
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // Define modo de display (double buffer, cores RGB)
  glutInitWindowSize(WIDTH, HEIGHT);           // Define tamanho da janela
  glutInitWindowPosition(200, 200);        // Define posição inicial da janela
  glutCreateWindow("Penguin Adventure - Trabalho 1 - Computacao Grafica - 2025.1");        // Cria janela com título
  init();                                      // Chama função de inicialização
  glutDisplayFunc(display);                    // Registra callback de desenho
  glutSpecialFunc(keyboard);                   // Registra callback de teclas especiais
  glutKeyboardFunc(keyboardNormal);            // Registra callback de teclas normais
  glutTimerFunc(DELAY, timer, 0);              // Registra callback do timer para animação/lógica
  glutMainLoop();                              // Inicia o loop de eventos do GLUT
  return 0; // Retorna 0 ao finalizar (teoricamente nunca alcançado devido ao glutMainLoop)
}

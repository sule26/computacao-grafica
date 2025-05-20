#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

const int WIDTH = 800;
const int HEIGHT = 600;
const int FPS = 60;
const int DELAY = 1000 / FPS;
const int COORDINATES_X = 20;
const int COORDINATES_Y = 20;
const int COORDINATES_Z = 20;
const float PINGUIN_COLLISION_RADIUS = 1.5f;
const float FISH_COLLISION_RADIUS = 1.0f;
const float CHICK_COLLISION_RADIUS = 2.0f;
const float WATER_SURFACE_Y_LEVEL = -COORDINATES_Y / 2.0f;
const float WATER_SWIM_CEILING = WATER_SURFACE_Y_LEVEL + 1.5f;
const float WATER_BOTTOM_Y = -COORDINATES_Y + 1.0f;
const float WATER_LEFT_BOUNDARY_X = 0.0f;
const float WATER_RIGHT_BOUNDARY_X = COORDINATES_X;

const int TIME_BONUS_FROM_FEEDING = 60;
const int INITIAL_GAME_TIME = 60;
const int MAX_SESSION_DURATION = 300;

float angle = 0.0f;
float pinguinPositionX = -12.0f;
float pinguinPositionY = WATER_SURFACE_Y_LEVEL;
float pinguinScaleX = 0.8f;
float pinguinScaleY = 0.8f;
float deltaX = 0.5f;
float deltaY = 0.5f;
bool isGoingRight = true;
bool isGoingDown = true;

int gameTimeRemaining;
int sessionTimeRemainingForWin;
int framesSinceLastSecond = 0;
bool isGameOver = false;
bool playerWon = false;

const float CHICK_POSITION_X = -18.0f;
const float CHICK_POSITION_Y = WATER_SURFACE_Y_LEVEL;

struct Fish {
    float x, y;
    bool isVisible;
    float speedX;
    bool movingRight;

    Fish(float _x, float _y, float _speedX, bool _movingRight) :
        x(_x), y(_y), isVisible(true), speedX(_speedX), movingRight(_movingRight) {}

    void move() {
        if (movingRight) {
            x += speedX;
            if (x > WATER_RIGHT_BOUNDARY_X - 0.5f) {
                x = WATER_RIGHT_BOUNDARY_X - 0.5f;
                movingRight = false;
            }
        } else {
            x -= speedX;
            if (x < WATER_LEFT_BOUNDARY_X + 0.5f) {
                x = WATER_LEFT_BOUNDARY_X + 0.5f;
                movingRight = true;
            }
        }
    }

    void respawn() {
        isVisible = true;
        y = WATER_BOTTOM_Y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WATER_SURFACE_Y_LEVEL - 1.0f - WATER_BOTTOM_Y)));
        if (rand() % 2 == 0) {
            x = WATER_LEFT_BOUNDARY_X + 0.5f;
            movingRight = true;
        } else {
            x = WATER_RIGHT_BOUNDARY_X - 0.5f;
            movingRight = false;
        }
        speedX = 0.1f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.2f));
    }
};

std::vector<Fish> fishes;
bool pinguinHasFish = false;

void init(void);
void display(void);
void timer(int value);
void keyboard(int key, int x, int y);

void checkCollisionsAndFeeding();
bool isPinguinInWater();
void drawFishModel();
void drawPenguin(bool isCarryingFish, float currentScaleX, float currentScaleY, bool inWater, bool goingDownIfInWater);
void keyboardNormal(unsigned char key, int x, int y);

std::string formatTime(int totalSeconds) {
    if (totalSeconds < 0) totalSeconds = 0;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
    return oss.str();
}

void initializeGame() {
    fishes.clear();
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < 4; ++i) {
        float startY = WATER_BOTTOM_Y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WATER_SURFACE_Y_LEVEL - 1.0f - WATER_BOTTOM_Y)));
        float startX = WATER_LEFT_BOUNDARY_X + 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (WATER_RIGHT_BOUNDARY_X - 1.0f - (WATER_LEFT_BOUNDARY_X + 0.5f) )));
        float speed = 0.1f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.2f));
        bool startsRight = rand() % 2;
        fishes.push_back(Fish(startX, startY, speed, startsRight));
    }
    pinguinHasFish = false;
    isGameOver = false;
    playerWon = false;
    gameTimeRemaining = INITIAL_GAME_TIME;
    sessionTimeRemainingForWin = MAX_SESSION_DURATION;
    framesSinceLastSecond = 0;
    pinguinPositionX = -12.0f;
    pinguinPositionY = WATER_SURFACE_Y_LEVEL;
    isGoingRight = true;
    isGoingDown = true;
}

void timer(int value)
{
  angle += 5.0f;
  if (angle > 360.0f)
    angle -= 360.f;

  if (!isGameOver && !playerWon) {
    for (Fish &fish : fishes) {
        if (fish.isVisible) {
            fish.move();
        }
    }
    checkCollisionsAndFeeding();
  }

  if (!isGameOver && !playerWon) {
    framesSinceLastSecond++;
    if (framesSinceLastSecond >= FPS) {
        if (gameTimeRemaining > 0) {
            gameTimeRemaining--;
        } else {
            isGameOver = true;
        }

        if (sessionTimeRemainingForWin > 0) {
            sessionTimeRemainingForWin--;
        } else {
            if (gameTimeRemaining > 0) {
                playerWon = true;
            } else {
                isGameOver = true;
            }
        }
        framesSinceLastSecond = 0;
    }
  }
  glutPostRedisplay();
  glutTimerFunc(DELAY, timer, 0);
}

void init(void)
{
  glClearColor(1, 1, 1, 0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-COORDINATES_X, COORDINATES_X, -COORDINATES_Y, COORDINATES_Y, -COORDINATES_Z, COORDINATES_Z);
  initializeGame();
}

bool isPinguinInWater()
{
  return pinguinPositionX >= WATER_LEFT_BOUNDARY_X && pinguinPositionY <= WATER_SURFACE_Y_LEVEL;
}

void drawSquare()
{
  glBegin(GL_POLYGON);
  glVertex3f(-1, -1, 0);
  glVertex3f(1, -1, 0);
  glVertex3f(1, 1, 0);
  glVertex3f(-1, 1, 0);
  glEnd();
}

void drawTriangle()
{
  glBegin(GL_POLYGON);
  glVertex3f(-1, 0, 0);
  glVertex3f(0, 1, 0);
  glVertex3f(1, 0, 0);
  glEnd();
}

void drawDisk(double radius)
{
  glBegin(GL_POLYGON);
  for (int i = 0; i < 32; i++)
  {
    double disk_angle = (2 * M_PI / 32) * i;
    glVertex3f(radius * cos(disk_angle), radius * sin(disk_angle), 0);
  }
  glEnd();
}

void drawElipse(double radiusX, double radiusY)
{
  glBegin(GL_POLYGON);
  for (int i = 0; i < 32; i++){
    double ellipse_angle = (2 * M_PI / 32) * i;
    glVertex3f(radiusX * cos(ellipse_angle), radiusY * sin(ellipse_angle), 0);
  }
  glEnd();
}

void drawFloor()
{
  glColor3f(0, 1, 0);
  glBegin(GL_POLYGON);
    glVertex3f(0, WATER_SURFACE_Y_LEVEL, 0);
    glVertex3f(-COORDINATES_X, WATER_SURFACE_Y_LEVEL, 0);
    glVertex3f(-COORDINATES_X, -COORDINATES_Y, 0);
    glVertex3f(0, -COORDINATES_Y, 0);
  glEnd();
}

void drawWater()
{
  glColor3f(0.4, 0.4, 1);
  glBegin(GL_POLYGON);
    glVertex3f(WATER_LEFT_BOUNDARY_X, WATER_SURFACE_Y_LEVEL, 0);
    glVertex3f(WATER_RIGHT_BOUNDARY_X, WATER_SURFACE_Y_LEVEL, 0);
    glVertex3f(WATER_RIGHT_BOUNDARY_X, -COORDINATES_Y, 0);
    glVertex3f(WATER_LEFT_BOUNDARY_X, -COORDINATES_Y, 0);
  glEnd();
}

void drawFishModel() {
  glPushMatrix();
    glColor3f(0.8f, 0.4f, 0.0f);
    glTranslatef(-1.5f * 0.5f, 0, 0.5f);
    glRotatef(270, 0, 0, 1);
    glScalef(1.0f * 0.5f, 0.5f * 0.5f, 1);
    drawTriangle();
  glPopMatrix();

  glPushMatrix();
    glColor3f(0.8f, 0.4f, 0.0f);
    glTranslatef(0, 0, 0.5f);
    glScalef(1.0f * 0.5f, 1.0f * 0.5f, 1);
    drawElipse(1, 0.5);
  glPopMatrix();
}

void drawPenguin(bool isCarryingFish, float currentPinguinScaleX, float currentPinguinScaleYWhenSwimming, bool isInWaterFlag, bool isGoingDownWhenSwimming)
{
  glPushMatrix();
    glColor3f(1, 0, 0);
    glTranslatef(-0.5, 0, 0.2f);
    glScalef(0.5, 1.5, 1);
    drawTriangle();
  glPopMatrix();

  glPushMatrix();
    glColor3f(1, 0, 0);
    glTranslatef(0.5, 0, 0.2f);
    glScalef(0.5, 1.5, 1);
    drawTriangle();
  glPopMatrix();

  glPushMatrix();
    glColor3f(0, 0, 0);
    glTranslatef(0, 3.7, 0);
    glScalef(2, 2.5, 1);
    drawElipse(0.5, 1);
  glPopMatrix();

  glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(0.0, 3.7, 0.1f);
    glScalef(0.8, 1.5, 1);
    drawElipse(0.3, 1);
  glPopMatrix();

  glPushMatrix();
    glColor3f(0, 0, 0);
    glTranslatef(0, 7, 0.1f);
    glScalef(1, 1, 1);
    drawDisk(0.8);
  glPopMatrix();

  glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(0.25, 7, 0.2f);
    glScalef(1, 1, 1);
    drawDisk(0.3);
  glPopMatrix();
      glPushMatrix();
        glColor3f(0,0,0);
        glTranslatef(0.25, 7, 0.25f);
        glScalef(1,1,1);
        drawDisk(0.1);
      glPopMatrix();

  glPushMatrix();
    glColor3f(0.8, 0.8, 0);
    glTranslatef(0.7, 6.5, 0.2f);
    glRotatef(20, 0, 0, 1);
    glScalef(0.6, 0.6, 1);
    drawTriangle();
  glPopMatrix();

  if (isCarryingFish) {
      glPushMatrix();
          if (isInWaterFlag) {
             glTranslatef(0.0f, 1.5f * (currentPinguinScaleYWhenSwimming > 0 ? 1.0f : -1.0f) , 0.5f);
             glRotatef(90, 0, 0, 1);
             if (currentPinguinScaleYWhenSwimming < 0) {
                glRotatef(180, 0,1,0);
             }
          } else {
            glTranslatef(0.7f + 0.6f, 6.5f, 0.5f);
            glRotatef(20, 0, 0, 1);
            if (currentPinguinScaleX < 0){
                glRotatef(180.0f, 0.0f,1.0f,0.0f);
                glTranslatef(-0.8f,0.0f,0.0f);
            } else {
                glTranslatef(0.4f, 0.0f, 0.0f);
            }
          }
          glScalef(0.8f, 0.8f, 0.8f);
          drawFishModel();
      glPopMatrix();
  }
}


void drawEnemy()
{
  glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.8f);
    glTranslatef(0, 0, 0);
    glScalef(2, 1.5, 1);
    drawElipse(1, 1);
  glPopMatrix();

  glPushMatrix();
    glColor3f(0.4f, 0.4f, 0.8f);
    glTranslatef(-1, 1, 0);
    glRotatef(-40, 0, 0, 1);
    glScalef(2, 1, 1);
    drawTriangle();
  glPopMatrix();

  glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(1.4, 0.3, 0);
    glScalef(1, 1, 1);
    drawDisk(0.3);
  glPopMatrix();

  glPushMatrix();
    glColor3f(0.8, 0.8, 0);
    glTranslatef(2.3, 0, 0);
    glRotatef(50, 0, 0, 1);
    glScalef(0.6, 0.6, 1);
    drawTriangle();
  glPopMatrix();
}


void keyboard(int key, int x, int y)
{
  if (isGameOver || playerWon) {
      return;
  }

  float oldPinguinX = pinguinPositionX;
  bool wasInWater = isPinguinInWater(); // Verifica se estava na água ANTES do movimento

  switch (key)
  {
    case GLUT_KEY_LEFT:
      if (pinguinPositionX > -COORDINATES_X + deltaX/2.0f) { // Pode se mover até a borda esquerda da tela
          pinguinPositionX -= deltaX;
      } else {
          pinguinPositionX = -COORDINATES_X; // Trava na borda
      }
      isGoingRight = false;
    break;
  case GLUT_KEY_RIGHT:
    if (pinguinPositionX < COORDINATES_X - deltaX/2.0f ){ // Pode se mover até a borda direita da tela
      pinguinPositionX += deltaX;
    } else {
        pinguinPositionX = COORDINATES_X; // Trava na borda
    }
    isGoingRight = true;
    break;
  case GLUT_KEY_DOWN:
    if (isPinguinInWater() && pinguinPositionY > WATER_BOTTOM_Y + deltaY){
      pinguinPositionY -= deltaY;
      isGoingDown = true;
    }
    break;
  case GLUT_KEY_UP:
    if (isPinguinInWater() && pinguinPositionY < WATER_SWIM_CEILING - deltaY) {
      pinguinPositionY += deltaY;
      isGoingDown = false;
    }
    break;
  default:
    break;
  }

  bool currentlyInWater = isPinguinInWater(); // Verifica se está na água DEPOIS do movimento

  // Lógica de transição Terra <-> Água
  if (!wasInWater && currentlyInWater) { // Entrou na água
      pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Ajusta para a superfície
  } else if (wasInWater && !currentlyInWater) { // Saiu da água
      pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Ajusta para a superfície da terra
  } else if (!currentlyInWater) { // Se não está na água (está na terra)
      pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Mantém na superfície da terra
  }
  // Se estiver exatamente na borda da água (X = WATER_LEFT_BOUNDARY_X) e Y for maior que a superfície,
  // força Y para a superfície. Isso pode acontecer se ele subir muito e depois tentar sair.
  if (pinguinPositionX >= WATER_LEFT_BOUNDARY_X && pinguinPositionY > WATER_SURFACE_Y_LEVEL && currentlyInWater) {
    //   pinguinPositionY = WATER_SURFACE_Y_LEVEL; // Comentado por enquanto, pode causar "teleporte"
  }


  glutPostRedisplay();
}

void keyboardNormal(unsigned char key, int x, int y)
{
    if (isGameOver || playerWon) {
        if (key == 'r' || key == 'R') {
            initializeGame();
            glutPostRedisplay();
        }
    }
    if (key == 27) {
        exit(0);
    }
}


void checkCollisionsAndFeeding() {
    if (isGameOver || playerWon) return;

    if (pinguinHasFish) {
        float dx_to_chick = pinguinPositionX - CHICK_POSITION_X;
        float dy_to_chick = pinguinPositionY - CHICK_POSITION_Y;
        float distance_to_chick = sqrt(dx_to_chick * dx_to_chick + dy_to_chick * dy_to_chick);

        // Pinguim só pode alimentar se estiver na terra (X < WATER_LEFT_BOUNDARY_X)
        if (distance_to_chick < (PINGUIN_COLLISION_RADIUS + CHICK_COLLISION_RADIUS) && pinguinPositionX < WATER_LEFT_BOUNDARY_X) {
            pinguinHasFish = false;
            gameTimeRemaining += TIME_BONUS_FROM_FEEDING;
            return;
        }
    }

    if (!pinguinHasFish) {
        for (Fish &fish : fishes) {
            if (fish.isVisible) {
                float dx_col = pinguinPositionX - fish.x;
                float dy_col = pinguinPositionY - fish.y;
                float distance = sqrt(dx_col * dx_col + dy_col * dy_col);

                if (distance < (PINGUIN_COLLISION_RADIUS + FISH_COLLISION_RADIUS)) {
                    if (isPinguinInWater()) {
                        pinguinHasFish = true;
                        fish.respawn();
                        break;
                    }
                }
            }
        }
    }
}


void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (!isGameOver && !playerWon) {
    drawFloor();
    drawWater();

    for (const Fish &fish : fishes) {
        if (fish.isVisible) {
            glPushMatrix();
              glTranslated(fish.x, fish.y, 0.4f);
              if (!fish.movingRight) {
                  glRotatef(180, 0, 1, 0);
              }
              glScalef(0.8f, 0.8f, 1.0f);
              drawFishModel();
            glPopMatrix();
        }
    }

    glPushMatrix();
      glTranslated(pinguinPositionX, pinguinPositionY, 0);
      bool currentIsInWater = isPinguinInWater();
      float currentPlayerScaleX, currentPlayerScaleYForSwimming;

      if (currentIsInWater) {
          glRotatef(270, 0, 0, 1);
          currentPlayerScaleX = isGoingDown ? 0.8f : -0.8f;
          currentPlayerScaleYForSwimming = isGoingRight ? 0.8f : -0.8f;
          glScalef(currentPlayerScaleX, currentPlayerScaleYForSwimming, 1);
      } else {
          currentPlayerScaleX = isGoingRight ? 0.8f : -0.8f;
          glScalef(currentPlayerScaleX, 0.8f, 1);
      }
      drawPenguin(pinguinHasFish, currentPlayerScaleX, currentPlayerScaleYForSwimming, currentIsInWater, isGoingDown);
    glPopMatrix();


    glPushMatrix();
      glTranslated(CHICK_POSITION_X, CHICK_POSITION_Y, 0);
      float filhoteScaleFactor = 0.5f;
      glScalef(filhoteScaleFactor, filhoteScaleFactor, 1);

      bool filhoteLooksRight = true;
      if (pinguinPositionX < WATER_LEFT_BOUNDARY_X && pinguinHasFish) {
          if (pinguinPositionX < CHICK_POSITION_X) filhoteLooksRight = false;
          else filhoteLooksRight = true;
      }
      
      float filhoteRenderScaleX = filhoteLooksRight ? 0.8f : -0.8f;
      glScalef(filhoteRenderScaleX / 0.8f, 1.0f, 1.0f); 

      drawPenguin(false, filhoteRenderScaleX, 0.8f, false, false);
    glPopMatrix();


    glPushMatrix();
      glTranslated(0, 10, 0);
      glScalef(1, 1, 1);
      drawEnemy();
    glPopMatrix();

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-COORDINATES_X + 1.0f, COORDINATES_Y - 2.0f);
    std::string pinguinLifeStr = "Vida do pinguim: " + formatTime(gameTimeRemaining);
    for (char c : pinguinLifeStr) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glRasterPos2f(-COORDINATES_X + 1.0f, COORDINATES_Y - 4.0f);
    std::string winConditionStr = "Vitoria em: " + formatTime(sessionTimeRemainingForWin);
     for (char c : winConditionStr) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }


  } else if (playerWon) {
      glColor3f(0.0f, 0.5f, 0.0f);
      std::string winMsgText = "VOCE GANHOU!";
      float textWidthEstimate = winMsgText.length() * 1.0f;
      glRasterPos2f(-textWidthEstimate / 2.0f, 2.0f);
      for (char c : winMsgText) {
          glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
      }
      std::string restartMsgText = "Press 'R' to Restart";
      float restartTextWidthEstimate = restartMsgText.length() * 0.4f;
      glRasterPos2f(-restartTextWidthEstimate / 2.0f, -2.0f);
      for (char c : restartMsgText) {
          glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
      }

  } else {
      glColor3f(0.0f, 0.0f, 0.0f);
      std::string gameOverMsgText = "GAME OVER";
      float textWidthEstimate = gameOverMsgText.length() * 0.8f;
      glRasterPos2f(-textWidthEstimate / 2.0f, 2.0f);
      for (char c : gameOverMsgText) {
          glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
      }
      std::string restartMsgText = "Press 'R' to Restart";
      float restartTextWidthEstimate = restartMsgText.length() * 0.4f;
      glRasterPos2f(-restartTextWidthEstimate / 2.0f, -2.0f);
      for (char c : restartMsgText) {
          glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
      }
  }
  glFlush();
}


int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(200, 200);
  glutCreateWindow("Trabalho 1 - Computação Gráfica UERJ 2025/1");
  init();
  glutDisplayFunc(display);
  glutSpecialFunc(keyboard);
  glutKeyboardFunc(keyboardNormal);
  glutTimerFunc(DELAY, timer, 0);
  glutMainLoop();
  return 0;
}

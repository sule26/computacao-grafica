#include <GL/glut.h>
#include <math.h>

const int WIDTH = 800;
const int HEIGHT = 600;
const int DELAY = 100;
const int COORDINATES_X = 20;
const int COORDINATES_Y = 20;
const int COORDINATES_Z = 20;

float angle = 0.0f; // angulo de rotação do moinho
float pinguinPositionX = -12.0f;
float pinguinPositionY = -COORDINATES_Y / 2;
float deltaX = 0.5f;
float deltaY = 0.5f;

void init(void);
void display(void);
void timer(int value);
void keyboard(int key, int x, int y);

void drawDisk(double radius);
void drawSquare(void);
void drawTriangle(void);

void timer(int value)
{

  angle += 5.0f;
  if (angle > 360.0f)
    angle -= 360.f;

  glutPostRedisplay();
  glutTimerFunc(DELAY, timer, 0);
}

void init(void)
{
  // define a cor de background da janela
  glClearColor(1, 1, 1, 0); // cor do céu

  // define o sistema de visualização - tipo de projeção
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-COORDINATES_X, COORDINATES_X, -COORDINATES_Y, COORDINATES_Y, -COORDINATES_Z, COORDINATES_Z);
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
    double angle = (2 * M_PI / 32) * i;
    glVertex3f(radius * cos(angle), radius * sin(angle), 0);
  }
  glEnd();
}

void drawElipse(double radiusX, double radiusY){
  glBegin(GL_POLYGON);
  for (int i = 0; i < 32; i++){
    double angle = (2 * M_PI / 32) * i;
    glVertex3f(radiusX * cos(angle), radiusY * sin(angle), 0);
  }
  glEnd();
}

void drawFloor(){
  glColor3f(0, 1, 0);
  glBegin(GL_POLYGON);
    glVertex3f(0, -COORDINATES_Y / 2, 0);
    glVertex3f(-COORDINATES_X, -COORDINATES_Y / 2, 0);
    glVertex3f(-COORDINATES_X, -COORDINATES_Y, 0);
    glVertex3f(0, -COORDINATES_Y, 0);
  glEnd();
}

void drawWater(){
  glColor3f(0.4, 0.4, 1);
  glBegin(GL_POLYGON);
    glVertex3f(0, -COORDINATES_Y / 2, 0);
    glVertex3f(COORDINATES_X, -COORDINATES_Y / 2, 0);
    glVertex3f(COORDINATES_X, -COORDINATES_Y, 0);
    glVertex3f(0, -COORDINATES_Y, 0);
  glEnd();
}

void drawPenguin(){

  
  

  // desenha pata de trás
  glPushMatrix();
    glColor3f(1, 0, 0);
    glTranslatef(-0.5, 0, 1);
    glScalef(0.5, 1.5, 1);
    drawTriangle();
  glPopMatrix();

  // desenha pata da frente

  glPushMatrix();
    glColor3f(1, 0, 0);
    glTranslatef(0.5, 0, 1);
    glScalef(0.5, 1.5, 1);
    drawTriangle();
  glPopMatrix();

  // desenha corpo
  glPushMatrix();
    glColor3f(0, 0, 0);
    glScalef(2, 2.5, 1);
    glTranslatef(0, 1.5, 0);
    drawElipse(0.5, 1);
  glPopMatrix();


  // desenha barriga
  glPushMatrix();
    glColor3f(1, 1, 1);
    glScalef(1, 1.5, 1);
    glTranslatef(0.5, 2.5, 1);
    drawElipse(0.3, 1);
  glPopMatrix();

  // desenha cabeça
  glPushMatrix();
    glColor3f(0, 1, 0);
    glTranslatef(0, 7, 1);
    glScalef(1, 1, 1);
    drawDisk(0.8);
  glPopMatrix();

  // desenha olho
  glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(0.25, 7, 1);
    glScalef(1, 1, 1);
    drawDisk(0.3);
  glPopMatrix();

  // desenha bico
  glPushMatrix();
    glColor3f(0.8, 0.8, 0);
    glTranslatef(1, 6.5, 1);
    glScalef(0.6, 0.6, 1);
    glRotatef(20, 0, 0, 1);
    drawTriangle();
  glPopMatrix();
}

// void keyboard(unsigned char key, int x, int y) // para teclas normais
void keyboard(int key, int x, int y) // para teclas especiais
{
  switch (key)
  {
  case GLUT_KEY_LEFT:
    if (pinguinPositionX >= 0){

    }
    if ((pinguinPositionY >= -COORDINATES_Y / 2) && (pinguinPositionX >= 0)){
      pinguinPositionX -= deltaX;
    }
    break;
  case GLUT_KEY_RIGHT:
    if (pinguinPositionX <= COORDINATES_X){
      pinguinPositionX += deltaX;
    }
    break;
  case GLUT_KEY_DOWN:
    if (pinguinPositionX >= 0 && pinguinPositionY > -COORDINATES_Y){
      pinguinPositionY -= deltaY;
    }
    break;
  case GLUT_KEY_UP:
    if (pinguinPositionX >= 0 && pinguinPositionY < -COORDINATES_Y / 2){
      pinguinPositionY += deltaY;
    }
    break;
  default:
    break;
  }

  glutPostRedisplay();
}

void display()
{
  // Limpa a janela, colocando na tela a cor definida pela função glClearColor
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // desenha o chão
  drawFloor();

  // desenha a água
  drawWater();

  // desenha pinguin
  glPushMatrix();
    glTranslated(pinguinPositionX, pinguinPositionY, 0);
    glScalef(0.8, 0.8, 1);
    drawPenguin();
  glPopMatrix();

  // Libera o buffer de comando de desenho para fazer o desenho acontecer o mais rápido possível.
  glFlush();

  // glutSwapBuffers();
}

int main(int argc, char **argv)
{

  // Inicializa a biblioteca GLUT e negocia uma seção com o gerenciador de janelas.
  // É possível passar argumentos para a função glutInit provenientes da linha de execução, tais como informações sobre a geometria da tela
  glutInit(&argc, argv);

  // Informa à biblioteca GLUT o modo do display a ser utilizado quando a janela gráfica for criada.
  //  O flag GLUT_SINGLE força o uso de uma janela com buffer simples, significando que todos os desenhos serão feitos diretamente nesta janela.
  //  O flag GLUT_RGB determina que o modelo de cor utilizado será o modelo RGB.

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  // Define o tamanho inicial da janela, 256x256 pixels, e a posição inicial do seu canto superior esquerdo na tela, (x, y)=(100, 100).
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(200, 200);

  // Cria uma janela e define seu título
  glutCreateWindow("Animation");

  // Nesta função é definido o estado inicial do OpenGL. Ajustes podem ser feitos para o usuário nessa função.
  init();

  // Define display() como a função de desenho (display callback) para a janela corrente.
  // Quando GLUT determina que esta janela deve ser redesenhada, a função de desenho é chamada.
  glutDisplayFunc(display);

  // glutKeyboardFunc(keyboard); // para teclas normais (ex: 'a', 'd', 'w', etc.)
  glutSpecialFunc(keyboard); // para teclas especiais (ex: setas)

  glutTimerFunc(DELAY, timer, 0);

  // Inicia o loop de processamento de desenhos com GLUT.
  //  Esta rotina deve ser chamada pelo menos uma vez em um programa que utilize a biblioteca GLUT.
  glutMainLoop();

  return 0;
}

// hello_world.cpp
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include "vec.hpp"
#include "mat.hpp"
#include "operator.hpp"
#include "transform.hpp"

#define LEVEL 7

using namespace std;

void init();
void mydisplay();
void myreshape(int width, int height);
GLuint create_shader_from_file(const std::string& filename, GLuint shader_type);
void draw_a_rectangle(float cx, float cy, float size);
void draw_a_rectangle_group(float group_size, unsigned int num_rects);
void Hilbert_vertex();
void createGLUTMenus();
void offsettingMenuEvents(int option);
void minmaxMenuEvents(int option);
void ratioMenuEvents(int option);
void processMenuEvents(int option);

GLuint program; // 쉐이더 프로그램 객체의 레퍼런스 값
GLint  loc_a_position;
GLint  loc_a_color;
GLint  loc_u_Model;
GLint loc_u_projection;

struct Point{
  float x;
  float y;
};
Point *parr[LEVEL];

int Min=15, Max=20;
float offset = 0.04f;
int ratio = 1;
float aspect_ratio = 1.0f;

int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitWindowSize(1100, 1100);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutCreateWindow("Hello FreeGLUT");

  init();
  createGLUTMenus();
  glutDisplayFunc(mydisplay);
  glutReshapeFunc(myreshape);

  glutMainLoop();

  return 0;
}

void createGLUTMenus() {

	int menu, submenu_offset, submenu_minmax, submenu_ratio;

	submenu_offset = glutCreateMenu(offsettingMenuEvents);
	glutAddMenuEntry("0.08f",1);
	glutAddMenuEntry("0.04f",2);
	glutAddMenuEntry("0.03f",3);

  submenu_minmax = glutCreateMenu(minmaxMenuEvents);
	glutAddMenuEntry("1/5",1);
	glutAddMenuEntry("5/10",2);
	glutAddMenuEntry("20/25",3);

  submenu_ratio = glutCreateMenu(ratioMenuEvents);
	glutAddMenuEntry("1",1);
	glutAddMenuEntry("2",2);
	glutAddMenuEntry("3",3);

	menu = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("offsetting",submenu_offset);
	glutAddSubMenu("min/max",submenu_minmax);
  glutAddSubMenu("ratio", submenu_ratio);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void processMenuEvents(int option){
}

void offsettingMenuEvents(int option){
  switch (option) {
  case 1:
    offset = 0.08f;
    break;
  case 2:
    offset = 0.04f;
    break;
  case 3:
    offset = 0.03f;
    break;
  }
  glutPostRedisplay();
}

void minmaxMenuEvents(int option){
  switch (option){
  case 1:
    Min = 1;
    Max = 5;
    break;
  case 2:
    Min = 5;
    Max = 10;
    break;
  case 3:
    Min = 20;
    Max = 25;
    break;
  }
  glutPostRedisplay();
}

void ratioMenuEvents(int option){
  switch (option) {
  case 1:
    ratio = 1;
    break;
  case 2:
    ratio = 2;
    break;
  case 3:
    ratio = 3;
    break;
  }
  glutPostRedisplay();
}


// GLSL 파일을 읽어서 컴파일한 후 쉐이더 객체를 생성하는 함수
GLuint create_shader_from_file(const std::string& filename, GLuint shader_type)
{
  GLuint shader = 0;
  shader = glCreateShader(shader_type);
  std::ifstream shader_file(filename.c_str());
  std::string shader_string;
  shader_string.assign(
    (std::istreambuf_iterator<char>(shader_file)),
    std::istreambuf_iterator<char>());

  const GLchar* shader_src = shader_string.c_str();

  glShaderSource(shader, 1, (const GLchar**)&shader_src, NULL);
  glCompileShader(shader);

  return shader;
}


void init()
{
  glewInit();

  // 정점 쉐이더 객체를 파일로부터 생성
  GLuint vertex_shader
    = create_shader_from_file("./shader/vertex.glsl", GL_VERTEX_SHADER);

  // 프래그먼트 쉐이더 객체를 파일로부터 생성
  GLuint fragment_shader
    = create_shader_from_file("./shader/fragment.glsl", GL_FRAGMENT_SHADER);

  // 쉐이더 프로그램 생성 및 컴파일
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  loc_a_position = glGetAttribLocation(program, "a_position");
  loc_a_color    = glGetAttribLocation(program, "a_color");
  loc_u_Model    = glGetUniformLocation(program, "u_Model");
  loc_u_projection    = glGetUniformLocation(program, "u_Projection");

  glClearColor(0.5f, 0.5f, 0.5f, 1.0f); //회색
}

void draw_a_rectangle(float cx, float cy, float size, float r, float g, float b){
  kmuvcl::math::mat4x4f model_matrix;
  kmuvcl::math::mat4x4f S, T;

  float position[] = {
    //right-down triangle
    0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.0f, 1.0f,
    //left-down triangle
    0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 1.0f,
  };

  float color[] = {
    0.0f, 0.7f, 0.0f, 1.0f,
    0.0f, 0.7f, 0.0f, 1.0f,
    0.0f, 0.7f, 0.0f, 1.0f,
    0.0f, 0.7f, 0.0f, 1.0f,
    0.0f, 0.7f, 0.0f, 1.0f,
    0.0f, 0.7f, 0.0f, 1.0f,
  };

  for(unsigned int i=0; i<6; ++i){
    color[4*i + 0]= r;
    color[4*i + 1]= g;
    color[4*i + 2]= b;
    color[4*i + 3]= 1.0f;
  }

  //T: 원점에 있어라
  T = kmuvcl::math::translate(cx, cy, 0.0f); //d(dx,dy,dz)를 0으로! translate하는 matrix 참고하기.
  S = kmuvcl::math::scale(size, size, 1.0f);
  model_matrix = T*S;

  glUniformMatrix4fv(loc_u_Model, 1, GL_FALSE, model_matrix); //1은 matrix개수, GL_FALSEL transform안한다.

  glVertexAttribPointer(loc_a_position, 4, GL_FLOAT, GL_FALSE, 0, position);
  glVertexAttribPointer(loc_a_color, 4, GL_FLOAT, GL_FALSE, 0, color);

  glEnableVertexAttribArray(loc_a_position);
  glEnableVertexAttribArray(loc_a_color);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisableVertexAttribArray(loc_a_position);
  glDisableVertexAttribArray(loc_a_color);

}

void draw_a_rectangle_group(float group_size, unsigned int num_rects){ //center값을 parameter로
  float size = group_size;
  Hilbert_vertex();
  srand(2);
  float r, g, b;

  int random_size = rand()%(Max-Min+1) + Min; //그릴 사각형 개수 Min~Max

  for(int l=0; l<LEVEL; l++){ //레벨
    int point_num = pow(4,l+1);
    for(int i=0; i<random_size; i++){ //사각형 그룹 개수

      if(i>=point_num) break;

      int random = rand() % (point_num);

      float x = parr[l][random].x;
      float y = parr[l][random].y;
      size = group_size;
      for (unsigned int j=0; j<num_rects; j++){ //사각형 하나에 그리는 사각형들
        if((j+i)%5 == 0){ //빨강
          r = 0.91f;
          g = 0.17f;
          b = 0.11f;
        }
        else if((j+i)%5 == 1){ //청록
          r = 0.0f;
          g = 0.29f;
          b = 0.22f;
        }
        else if((j+i)%5 == 2){ //상아색
          r = 1.0f;
          g = 0.9f;
          b = 0.35f;
        }
        else if((j+i)%5 == 3){ //노랑
          r = 1.0f;
          g = 0.78f;
          b = 0.0f;
        }
        else if((i+j)%5 == 4){ //하늘
          r = 0.44f;
          g = 0.61f;
          b = 0.6f;
        }
        draw_a_rectangle(x, y, size, r, g, b);
        if(size > 0.08f) size -= offset; //안에 그리는 사각형 크기가 0.08보다 작으면 break
        else break;
      }
      if(group_size > 0.05f) group_size -= 0.01f; //그룹사이즈가 0.05보다 작으면 break
      else break;
    }
    num_rects = num_rects * ratio; // 사각형 개수
  }

}

void Hilbert_vertex(){
  int coef=2;

  for(int l=0; l<LEVEL; l++){
    parr[l] = new Point[coef*coef];
    int idx=0;
    for (int y=1; y<=coef; y++){
      for (int x=1; x<=coef; x++){
        parr[l][idx].x = (float)x/coef * 2.0f - 1.0f/coef -1.0f;
        parr[l][idx].y = (float)y/coef * 2.0f - 1.0f/coef -1.0f;
        idx++;
      }
    }
    coef *= 2;
  }
}

void mydisplay()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 캔버스를 지움.

  glUseProgram(program);

  kmuvcl::math::mat4x4f proj_matrix;
  proj_matrix = kmuvcl::math::ortho(-2.0f, 2.0f, aspect_ratio * -2.0f, aspect_ratio * 2.0f, -1.0f, 1.0f);
  glUniformMatrix4fv(loc_u_projection, 1, GL_FALSE, proj_matrix);

  draw_a_rectangle_group(0.6f, 5);

  glUseProgram(0);

  glutSwapBuffers();
}

void myreshape(int width, int height){
  glViewport(0,0,width, height);
  aspect_ratio = (float)height / width;
}

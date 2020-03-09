
#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"

int main() {
	glfwInit();
	GLFWwindow* w = glfwCreateWindow(640, 480, "test", NULL, NULL);
	glfwMakeContextCurrent(w);
	glewInit();

	glViewport(0, 0, 640, 480);
	glTranslatef(-1, 1, 0);
	glScalef(1 / 640.0f, -1 / 480.0f, 1);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS);
	glColor4f(1, 0, 0, 0); glVertex2i(0, 0);
	glColor4f(0, 0, 1, 0); glVertex2i(100, 100);
	glEnd();

	glfwSwapBuffers(w);

	while (!glfwWindowShouldClose(w)) { glfwPollEvents(); }
	glfwTerminate();
}

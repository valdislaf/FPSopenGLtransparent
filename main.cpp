//#define _CRT_SECURE_NO_WARNINGS
//#define _USE_MATH_DEFINES
#include <cstdio>

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

#include <locale.h>

#include <ft2build.h>
#include FT_FREETYPE_H
GLuint VBO, VAO;
GLuint vao_text;
GLuint CompileShaders();
GLuint CompileShadersCube();
float aspectRatio=1.0f;
int w = 800;
int h = 800;
// Создание вершинных данных
GLfloat vertices[] = {

	// Верхняя грань
	-0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f,
	-0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f,
	0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f,
	0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f,
	0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f,
	-0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f,

   // Нижняя грань
	-0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f,
	0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f,
	0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f,
	0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f,
	-0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f,   
	-0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f,

  // Левая грань
  -0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f,
   -0.5f, -0.5f,  0.5f, 0.f, 0.f, 1.f,
	 -0.5f,  0.5f,  0.5f, 0.f, 0.f, 1.f,
  -0.5f,  0.5f,  0.5f, 0.f, 0.f, 1.f,
  -0.5f,  0.5f, -0.5f, 0.f, 0.f, 1.f, 
  -0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f,

  // Правая грань
	0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f,
		0.5f,  0.5f, -0.5f, 1.f, 1.f, 0.f,
			0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f,
	0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f,
	0.5f, -0.5f, 0.5f, 1.f, 1.f, 0.f,
	0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f,

	// Задняя грань
	-0.5f, -0.5f, -0.5f, 1.f, 0.f, 1.f,
		-0.5f,  0.5f, -0.5f, 1.f, 0.f, 1.f,
			 0.5f,  0.5f, -0.5f, 1.f, 0.f, 1.f,
	 0.5f,  0.5f, -0.5f, 1.f, 0.f, 1.f,
	 0.5f, -0.5f, -0.5f, 1.f, 0.f, 1.f,
	-0.5f, -0.5f, -0.5f, 1.f, 0.f, 1.f,

	// Передняя грань
	-0.5f, -0.5f, 0.5f, 0.f, 1.f, 1.f,
		 0.5f, -0.5f, 0.5f, 0.f, 1.f, 1.f,
			 0.5f,  0.5f, 0.5f, 0.f, 1.f, 1.f,
	  0.5f,  0.5f, 0.5f, 0.f, 1.f, 1.f,
	-0.5f,  0.5f, 0.5f, 0.f, 1.f, 1.f,
	-0.5f, -0.5f, 0.5f, 0.f, 1.f, 1.f,
};

struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data) {
	printf("%d: %s\n", id, msg);
	printf("OpenGL сообщение: %s (тип: 0x%x, источник: 0x%x, ID: %u, серьезность: 0x%x)\n", msg, type, source, id, severity);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	glfwGetFramebufferSize(window, &width, &height);
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	w = width;
	h = height;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scaleX, GLfloat scaleY, std::map<GLchar, Character>& Characters, GLuint& buffer) {

	//glActiveTexture(GL_TEXTURE0);
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];
		GLfloat xpos = x + ch.Bearing.x * scaleX;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scaleY;

		GLfloat w = ch.Size.x * scaleX;
		GLfloat h = ch.Size.y * scaleY;
		// Update VBO for each character
		GLfloat vertices[6 * 4] = {
			 xpos,     ypos + h,   0.0f, 0.0f ,
			 xpos,     ypos,       0.0f, 1.0f ,
			 xpos + w, ypos,       1.0f, 1.0f ,

			 xpos,     ypos + h,   0.0f, 0.0f ,
			 xpos + w, ypos,       1.0f, 1.0f ,
			 xpos + w, ypos + h,   1.0f, 0.0f
		};

		glNamedBufferSubData(buffer, 0, sizeof(GLfloat) * 6 * 4, vertices);
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scaleX;

	}

}

void initcube() {	

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

}

int init(GLuint& buffer) {


	FT_Library ft;
	FT_Init_FreeType(&ft);

	FT_Face face;
	//FT_New_Face(ft, "C:/Windows/Fonts/arial.ttf", 0, &face);
	// открываем шрифт
	if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face)) {
		fprintf(stderr, "Не удалось загрузить шрифт\n");
		return 0;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);



	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++) {
		if (c == 32) continue; // Пропускать пробелы

		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			fprintf(stderr, "Ошибка загрузки символа: %c\n\n", c);
			continue;
		}

		if (face->glyph->bitmap.width == 0 || face->glyph->bitmap.rows == 0) {
			fprintf(stderr, "Ошибка создания текстуры: некорректный размер битмапа символа\n\n");
			continue;
		}

		GLuint texture;
		glCreateTextures(GL_TEXTURE_2D, 1, &texture);

		glTextureStorage2D(texture, 1, GL_RGBA8, face->glyph->bitmap.width, face->glyph->bitmap.rows);

		if (glGetError() != GL_NO_ERROR) {
			fprintf(stderr, "Ошибка создания текстуры\n\n");
			continue;
		}

		glTextureSubImage2D(texture, 0, 0, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		Character character = {
		texture,
		glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		static_cast<GLuint>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));

	
	glCreateVertexArrays(1, &vao_text);
	glBindVertexArray(vao_text);

	
	glCreateBuffers(1, &buffer);

	glNamedBufferStorage(buffer, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayVertexBuffer(vao_text, 0, buffer, 0, sizeof(GLfloat) * 4);
	glVertexArrayAttribFormat(vao_text, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_text, 0, 0);
	glEnableVertexArrayAttrib(vao_text, 0);
	glUniform4f(7, 0.0f, 1.0f, 0.0f, 0.0f);
	glUniform3f(6, 0.0f, 1.0f, 0.0f);

    return 0;
}

void angspd(float &angle,float &speed) {
    angle += speed;
    if (angle >= 1.0f) {
        angle = 1.0f;
        speed = -speed;
    }
    else if (angle <= 0.0f) {
        angle = 0.0f;
        speed = -speed;
    }
}

int main(int argc, char* argv[]) {

	//setlocale(LC_ALL, "Russian");
	setlocale(LC_ALL, "en_US.UTF-8");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);;

	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	framebuffer_size_callback(window, w, h);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// Определяем время начала отсчета
	double startTime = glfwGetTime();
	// Количество кадров
	int frameCount = 0;
	int frameCount60 = 0;
	std::string frameCountStr = std::to_string(frameCount60);


	glewInit();
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(GLDebugMessageCallback, NULL);
	//glViewport(0, 0, 800, 600);


	GLuint shader = CompileShaders();
	glUseProgram(shader);
	GLuint buffer;
	init(buffer);


	GLuint shadercube = CompileShadersCube();
	glUseProgram(shadercube);
	initcube();



	float anglef = 0.99f;
	float speedf = 0.01f;
	float angle = 0.01f;
		float speed = 0.01f;

		int widthinit, heightinit;
		glfwGetFramebufferSize(window, &widthinit, &heightinit);

		float px_w = 2.0f / (float)widthinit;
		float px_h = 2.0f / (float)heightinit;


	while (!glfwWindowShouldClose(window)) {
		
		float r_w = (float)w / (float)widthinit;
		float r_h = (float)h / (float)heightinit;
		float scale = 0.3f;
		int deltaw = w - widthinit;
		int deltah = h - heightinit;


		angspd(angle, speed);
		angspd(anglef, speedf);
		// Обработка ввода
		processInput(window);
		//glClearColor(0.0f, angle, 1.0f, 1.0f); // установка цвета фона в сине-голубой
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // установка цвета фона в сине-голубой
		//glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shadercube);
		//glViewport(0, 0, height, height);

		// Матрица преобразования
		glm::mat4 transform = glm::mat4(1.0f);
		float ze = -1.7f - 12.1f * angle;
		transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, ze));
		transform = glm::rotate(transform, (float)glfwGetTime() / 1, glm::vec3(1.0f*anglef, 1.0f, 1.0f* angle));
		transform = glm::scale(transform, glm::vec3(0.7f, 0.7f, 0.7f));

		// Матрица проекции
		float fov =  45.0f;
		glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f , 1000.0f );
		//glm::mat4 projection = glm::ortho(glm::radians(30.0f), 1.0f, 0.1f, 100.0f);
	//где:
	//	    fov - угол обзора в градусах(field of view)
	//		aspect - соотношение ширины к высоте экрана
	//		near - расстояние до ближней плоскости отсечения(near clipping plane)
	//		far - расстояние до дальней плоскости отсечения(far clipping plane)
	
		// Установка матриц в шейдер
		GLuint transformLoc = glGetUniformLocation(shadercube, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		GLuint projectionLoc = glGetUniformLocation(shadercube, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Отрисовка треугольника	
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);





		// Отрисовка треугольника	
		//glViewport(0, 0, 900, 900); // устанавливаем новые пропорции области вывода

		//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),  // позиция камеры
		//	glm::vec3(0.0f, 0.0f, 0.0f),  // точка, на которую смотрит камера
		//	glm::vec3(0.0f, 0.0f, 1.0f)); // направление "вверх"

		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		//glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 4.0f);
		//// Умножение матриц преобразования модели и вида
		//glm::mat4 mvp = projection * view * model;
		//// Установка матрицы преобразования в шейдере
		// transformLoc = glGetUniformLocation(shadercube, "transform");
		//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));





		// настройка глубины для текста
		glDepthFunc(GL_ALWAYS);
		glViewport(0, 0, w , h);
		glUseProgram(shader);
		/*init(buffer);*/
		glBindVertexArray(vao_text);
		frameCountStr = std::to_string(frameCount60);
		
		RenderText("fps: " +frameCountStr, ((widthinit-50) + deltaw) / r_w, ((heightinit - 15)  + deltah) / r_h, scale / r_w, scale / r_h, Characters, buffer);
		RenderText( std::to_string(w), 10 / r_w, ((heightinit - 15)  + deltah) / r_h, scale/ r_w, scale / r_h, Characters, buffer);
		RenderText(std::to_string(h), 10 / r_w, ((heightinit - 35)  + deltah) / r_h, scale / r_w, scale / r_h, Characters, buffer);
		//RenderText(frameCountStr, 0.0f, 0.0f, 10.3f / r_w, 10.3f / r_h, Characters, buffer);
		RenderText("angle: "+std::to_string(angle), 10 / r_w, ((heightinit - 55) + deltah) / r_h, scale / r_w, scale / r_h, Characters, buffer);
		
		RenderText("fov: " + std::to_string(fov), 10 / r_w, ((heightinit - 75) + deltah) / r_h, scale / r_w, scale / r_h, Characters, buffer);
		RenderText("ze: " + std::to_string(ze), 10 / r_w, ((heightinit - 95) + deltah) / r_h, scale / r_w, scale / r_h, Characters, buffer);

		// возврат настройки глубины
		glDepthFunc(GL_LESS);


		glfwSwapBuffers(window);
		glfwPollEvents();

		// Увеличиваем количество кадров
		frameCount++;

		// Если прошла секунда с момента начала отсчета
		if (glfwGetTime() - startTime >= 1.0)
		{
			// Выводим количество кадров
			//std::cout << "FPS: " << frameCount << std::endl;
			frameCount60 = frameCount;
			// Сбрасываем количество кадров и время начала отсчета
			frameCount = 0;
			startTime = glfwGetTime();
		}
	}
	printf("%d", glGetError());
	return 0;
}



GLuint CompileShaders() {

	GLuint shader_programm = glCreateProgram();

	GLuint vs,  fs;

	const char* vertexShaderSource = R"glsl(
	#version 460 core
	layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
	layout (location = 1) uniform mat4 projection;

	out vec2 TexCoords;

	void main()
	{
		gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
		TexCoords = vertex.zw;
	}
)glsl";

	const char* fragment_shader_source = R"glsl(
#version 460 core
in vec2 TexCoords;
out vec4 color;

layout (binding = 0) uniform sampler2D text;
layout (location = 6) uniform vec3 textColor;
layout (location = 7) uniform vec4 backgroundColor;

void main()
{  
    vec4 textColorWithAlpha = vec4(textColor, 1.0);
    vec4 backgroundColorWithAlpha = vec4(backgroundColor.rgb, 0.0);

    color = mix(backgroundColorWithAlpha, textColorWithAlpha, texture(text, TexCoords).r);
}


)glsl";

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShaderSource, nullptr);
	glCompileShader(vs);

	GLint isCompiled = 0;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* error = (char*)malloc(maxLength);
		glGetShaderInfoLog(vs, maxLength, &maxLength, error);
		printf("Vertex shader error: ");
        printf("%s", error);

        free(error);
	}

	glAttachShader(shader_programm, vs);

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader_source, NULL);
	glCompileShader(fs);

	isCompiled = 0;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* error = (char*)malloc(maxLength);
		glGetShaderInfoLog(fs, maxLength, &maxLength, error);
		printf("Fragment shader error: ");
        printf("%s", error);

        free(error);
	}

	glAttachShader(shader_programm, fs);

	glLinkProgram(shader_programm);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return shader_programm;

}

GLuint CompileShadersCube() {

	GLuint shader_programm = glCreateProgram();

	GLuint vs, fs;

	const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 color;

uniform mat4 transform;
uniform mat4 projection;

void main()
{
    gl_Position = projection * transform * vec4(aPos, 1.0);
    color = aColor;
}


)glsl";

	const char* fragment_shader_source = R"(
        #version 330 core
        in vec3 color;
        out vec4 FragColor;
        void main()
        {
           FragColor = vec4(color, 1.0f);
        }

)";

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShaderSource, nullptr);
	glCompileShader(vs);

	GLint isCompiled = 0;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* error = (char*)malloc(maxLength);
		glGetShaderInfoLog(vs, maxLength, &maxLength, error);
		printf("Vertex shader error: ");
		printf("%s", error);

		free(error);
	}

	glAttachShader(shader_programm, vs);

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader_source, NULL);
	glCompileShader(fs);

	isCompiled = 0;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		char* error = (char*)malloc(maxLength);
		glGetShaderInfoLog(fs, maxLength, &maxLength, error);
		printf("Fragment shader error: ");
		printf("%s", error);

		free(error);
	}

	glAttachShader(shader_programm, fs);

	glLinkProgram(shader_programm);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return shader_programm;

}

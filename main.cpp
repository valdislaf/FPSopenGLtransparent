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

GLuint CompileShaders();

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
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void RenderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, std::map<GLchar, Character>& Characters, GLuint& buffer) {

	//glActiveTexture(GL_TEXTURE0);
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];
		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
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
		x += (ch.Advance >> 6) * scale;

	}

}

int init() {
	GLuint shader = CompileShaders();
	glUseProgram(shader);

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
	glViewport(0, 0, 800, 600);

	init();

	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glCreateBuffers(1, &buffer);

	glNamedBufferStorage(buffer, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(GLfloat) * 4);
	glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);
	glUniform4f(7, 0.0f, 1.0f, 0.0f, 0.0f);
	glUniform3f(6, 0.0f, 1.0f, 0.0f);


	int width, height;
	glfwGetWindowSize(window, &width, &height);

	float angle = 0.01f;
		float speed = 0.01f;
	

	while (!glfwWindowShouldClose(window)) {
		angspd(angle, speed);
		// Обработка ввода
		processInput(window);
		glClearColor(angle, angle, angle, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_MULTISAMPLE);
		
		frameCountStr = std::to_string(frameCount60);
		RenderText(frameCountStr, width-20.0f, height-15.0f, 0.3f, Characters, buffer);
		RenderText(frameCountStr,0.0f, 0.0f, 10.3f, Characters, buffer);

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

	const char* fragment_shader_source = R"(
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


//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <iostream>
//#include <string>
//#include <map>
//
//// Font path
//std::string fontPath = "C:/Windows/Fonts/arial.ttf";
//
//// Screen dimensions
//const GLuint WIDTH = 800, HEIGHT = 600;
//
//// Vertex Shader source code
//const GLchar* vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
//"out vec2 TexCoords;\n"
//"uniform mat4 projection;\n"
//"void main()\n"
//"{\n"
//"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
//"    TexCoords = vertex.zw;\n"
//"}\0";
//
//// Fragment Shader source code
//const GLchar* fragmentShaderSource = "#version 330 core\n"
//"in vec2 TexCoords;\n"
//"out vec4 color;\n"
//"uniform sampler2D text;\n"
//"uniform vec3 textColor;\n"
//"void main()\n"
//"{\n"
//"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
//"    color = vec4(textColor, 1.0) * sampled;\n"
//"}\n\0";
//
//GLFWwindow* window;
//GLuint VAO, VBO, texture;
//FT_Library ft;
//FT_Face face;
//GLuint shaderProgram;
//
//struct Character {
//    GLuint textureID;   // ID handle of the glyph texture
//    glm::ivec2 size;    // Size of glyph
//    glm::ivec2 bearing; // Offset from baseline to left/top of glyph
//    GLuint advance;     // Horizontal offset to advance to next glyph
//};
//
//std::map<GLchar, Character> characters;
//
//void loadFont(std::string fontPath) {
//    if (FT_Init_FreeType(&ft)) {
//        throw std::runtime_error("Failed to initialize FreeType library");
//    }
//
//    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
//        throw std::runtime_error("Failed to load font");
//    }
//
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//    for (unsigned char c = 0; c < 128; c++) {
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//            std::cerr << "Failed to load glyph " << c << std::endl;
//            continue;
//        }
//
//        GLuint texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face->glyph->bitmap.width,
//            face->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face->glyph->bitmap.buffer
//        );
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        Character character = {
//            texture,
//            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//            static_cast<GLuint>(face->glyph->advance.x)
//        };
//        characters.insert(std::pair<GLchar, Character>(c, character));
//    }
//
//    glBindTexture(GL_TEXTURE_2D, 0);
//
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//}
//
//void renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(VAO);
//
//    std::string::const_iterator c;
//    for (c = text.begin(); c != text.end(); c++) {
//        Character ch = characters[*c];
//
//        GLfloat xpos = x + ch.bearing.x * scale;
//        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;
//
//        GLfloat w = ch.size.x * scale;
//        GLfloat h = ch.size.y * scale;
//
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//
//        glBindTexture(GL_TEXTURE_2D, ch.textureID);
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        x += (ch.advance >> 6) * scale;
//    }
//
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//int main() {
//    if (!glfwInit()) {
//        return -1;
//    }
//
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    window = glfwCreateWindow(WIDTH, HEIGHT, "FreeType and OpenGL", nullptr, nullptr);
//    if (!window) {
//        glfwTerminate();
//        return -1;
//    }
//
//    glfwMakeContextCurrent(window);
//    glewExperimental = GL_TRUE;
//    if (glewInit() != GLEW_OK) {
//        return -1;
//    }
//
//    glViewport(0, 0, WIDTH, HEIGHT);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
//    glCompileShader(vertexShader);
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
//    glCompileShader(fragmentShader);
//
//    shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//
//
//    // Check for shader compilation errors
//    GLint success;
//    GLchar infoLog[512];
//    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
//        std::cout << "Error compiling vertex shader: " << infoLog << std::endl;
//    }
//
//    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
//        std::cout << "Error compiling fragment shader: " << infoLog << std::endl;
//    }
//
//    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//    if (!success) {
//        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
//        std::cout << "Error linking shader program: " << infoLog << std::endl;
//    }
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    loadFont("C:/Windows/Fonts/arial.ttf");
//
//    while (!glfwWindowShouldClose(window)) {
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        renderText("Hello, World!", 100.0f, 100.0f, 1.0f, glm::vec3(1.0f, 0.5f, 0.2f));
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteProgram(shaderProgram);
//
//    glfwTerminate();
//
//    return 0;
//}

// end of code.





//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <iostream>
//#include <string>
//#include <map>
//GLuint VAO, VBO, texture;
//FT_Library ft;
//FT_Face face;
//GLuint shaderProgram;
//
//
//struct Character {
//    GLuint textureID;     // идентификатор текстуры символа
//    glm::ivec2 size;      // размер символа в пикселях
//    glm::ivec2 bearing;      // смещение символа от начала строки в пикселях
//    GLuint advance;        // расстояние до начала следующего символа в пикселях
//    glm::vec2 texCoords;   // текстурные координаты левого верхнего угла символа
//};
//
//std::map<GLchar, Character> characters;
//
//void loadFont(std::string fontPath) {
//    FT_Library ft;
//    if (FT_Init_FreeType(&ft)) {
//        throw std::runtime_error("Failed to initialize FreeType library");
//    }
//
//    FT_Face face;
//    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
//        throw std::runtime_error("Failed to load font");
//    }
//
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//    for (unsigned char c = 0; c < 128; c++) {
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//            std::cerr << "Failed to load glyph " << c << std::endl;
//            continue;
//        }
//
//        GLuint texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face->glyph->bitmap.width,
//            face->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face->glyph->bitmap.buffer
//        );
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        Character    character = {
//            texture,
//            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//            face->glyph->advance.x,
//            glm::vec2(face->glyph->bitmap_left / (float)face->glyph->bitmap.width,
//                face->glyph->bitmap_top / (float)face->glyph->bitmap.rows)
//        };
//
//        characters.insert(std::pair<GLchar, Character>(c, character));
//    }
//
//    glBindTexture(GL_TEXTURE_2D, 0);
//
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//
//}
//
//
//// Utility function to compile shader source code
//GLuint compileShader(const char* source, GLenum type) {
//    GLuint shader = glCreateShader(type);
//    glShaderSource(shader, 1, &source, NULL);
//    glCompileShader(shader);
//
//    // Check for shader compile errors
//    GLint success;
//    GLchar infoLog[512];
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        glGetShaderInfoLog(shader, 512, NULL, infoLog);
//        std::cout << "Shader compilation failed\n" << infoLog << std::endl;
//    }
//
//    return shader;
//}
//
//// Utility function to create shader program
//GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
//    GLuint program = glCreateProgram();
//    glAttachShader(program, vertexShader);
//    glAttachShader(program, fragmentShader);
//    glLinkProgram(program);
//
//    // Check for program link errors
//    GLint success;
//    GLchar infoLog[512];
//    glGetProgramiv(program, GL_LINK_STATUS, &success);
//    if (!success) {
//        glGetProgramInfoLog(program, 512, NULL, infoLog);
//        std::cout << "Program linking failed\n" << infoLog << std::endl;
//    }
//
//    // Delete shaders
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    return program;
//}
//
//void renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
//    // Activate corresponding render state	
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(VAO);
//
//   
//    // Iterate through all characters
//    std::string::const_iterator c;
//    for (c = text.begin(); c != text.end(); c++) {
//        Character ch = characters[*c];
//
//        GLfloat xpos = x + ch.bearing.x * scale;
//        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;
//
//        GLfloat w = ch.size.x * scale;
//        GLfloat h = ch.size.y * scale;
//
//        // Update VBO for each character
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 }
//        };
//
//        // Render glyph texture over quad
//        glBindTexture(GL_TEXTURE_2D, ch.textureID);
//
//        // Update content of VBO memory
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//
//        // Render quad
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        // Now advance cursors for next glyph
//        x += (ch.advance >> 6) * scale;
//    }
//
//    // Unbind VAO and shader program
//    glBindVertexArray(0);
//    glUseProgram(0);
//}
//
//int main() {
//    // Initialize GLFW and OpenGL version
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    // Create window and OpenGL context
//    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Text Rendering", NULL, NULL);
//    if (window == NULL) {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    // Initialize GLEW
//    glewExperimental = GL_TRUE;
//    if (glewInit() != GLEW_OK) {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        return -1;
//    }
//
//    // Set viewport and clear color
//    glViewport(0, 0, 800, 600);
//    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//    const char* vertexShaderSource = "#version 330 core\n"
//                "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
//                "out vec2 TexCoords;\n"
//                "uniform mat4 projection;\n"
//                "void main()\n"
//                "{\n"
//                "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
//                "    TexCoords = vertex.zw;\n"
//                "}";
//            const char* fragmentShaderSource = "#version 330 core\n"
//                "in vec2 TexCoords;\n"
//                "out vec4 color;\n"
//                "uniform sampler2D text;\n"
//                "uniform vec3 textColor;\n"
//                "void main()\n"
//                "{\n"
//                "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
//                "    color = vec4(textColor, 1.0) * sampled;\n"
//                "}";
//    // Initialize shaders and program
//    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
//    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
//    shaderProgram = createShaderProgram(vertexShader, fragmentShader);
//
//    //// Initialize FreeType and character map
//    //init();
//    loadFont("C:/Windows/Fonts/arial.ttf");
//    // Render loop
//    while (!glfwWindowShouldClose(window)) {
//        // Clear screen
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Draw text    
//        renderText("Hello, World!", 10.0f, 10.0f, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
//        // Swap buffers and poll events
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // Clean up resources
//
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//    // Terminate GLFW
//    glfwTerminate();
//
//    return 0;
//}

//
//// Utility function to load a font face
//void loadFontFace(const char* fontPath) {
//    FT_Library ft;
//    if (FT_Init_FreeType(&ft)) {
//        std::cout << "Failed to initialize FreeType library" << std::endl;
//        return;
//    }
//
//    FT_Face face;
//    if (FT_New_Face(ft, fontPath, 0, &face)) {
//        std::cout << "Failed to load font face: " << fontPath << std::endl;
//        FT_Done_FreeType(ft);
//        return;
//    }
//
//    // Set size to load glyphs as
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    // Disable byte-alignment restriction
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//    // Load characters into the character map
//    for (GLubyte c = 0; c < 128; c++) {
//        // Load character glyph 
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//            std::cout << "Failed to load glyph: " << c << std::endl;
//            continue;
//        }
//
//        // Generate texture for glyph
//        GLuint texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face->glyph->bitmap.width,
//            face->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face->glyph->bitmap.buffer
//        );
//        // Set texture options
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        // Store character in character map
//        Character character = {
//            texture,
//            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//            face->glyph->advance.x
//        };
//        Characters[c] = character;
//    }
//
//    // Clean up resources
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//}
//
//// Utility function to render text
//void renderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
//    // Activate corresponding render state
//    shader.use();
//    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(VAO);
//
//    // Iterate through all characters
//    std::string::const_iterator c;
//    for (c = text.begin(); c != text.end(); c++) {
//        Character ch = Characters[*c];
//
//        GLfloat xpos = x + ch.Bearing.x * scale;
//        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
//        GLfloat w = ch.Size.x * scale;
//        GLfloat h = ch.Size.y * scale;
//
//        // Update VBO for each character
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//
//        // Render glyph texture over quad
//        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
//
//        // Update content of VBO memory
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//
//        // Render quad
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
//        x += (ch.Advance >> 6) * scale;
//    }
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//int main() {
//    // Initialize GLFW
//    if (!glfwInit()) {
//        std::cout << "Failed to initialize GLFW" << std::endl;
//        return -1;
//    }
//
//    // Create a GLFW window
//    GLFWwindow* window = glfwCreateWindow(800, 600, "Text Rendering", NULL, NULL);
//    if (!window) {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    // Initialize GLEW
//    glewExperimental = true;
//    if (glewInit() != GLEW_OK) {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Enable blending
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    // Set up vertex data and buffers
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    // Load shader program
//    Shader shader("shader.vert", "shader.frag");
//
//    // Load font
//    loadFont("arial.ttf");
//
//    // Render loop
//    while (!glfwWindowShouldClose(window)) {
//        // Clear the screen
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Render text
//        renderText(shader, "Hello, World!", 200.0f, 300.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
//
//        // Swap buffers and poll events
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // Clean up resources
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glfwTerminate();
//
//    return 0;
//}

/* Output:
 * A window should appear with white text saying "Hello, World!"
 */




















//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <iostream>
//#include <string>
//
//GLuint VAO, VBO;
//FT_Library ft;
//FT_Face face;
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//    glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}
//
//void checkCompileErrors(GLuint shader, std::string type) {
//    GLint success;
//    GLchar infoLog[1024];
//    if (type != "PROGRAM") {
//        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//        if (!success) {
//            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
//            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//        }
//    }
//    else {
//        glGetProgramiv(shader, GL_LINK_STATUS, &success);
//        if (!success) {
//            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
//            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//        }
//    }
//}
//
//GLuint createTexture(FT_Face face) {
//    GLuint texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    return texture;
//}
//
//void drawText(FT_Face face, GLuint shaderProgram, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//
//    GLfloat x2 = x;
//    GLfloat y2 = y;
//
//    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
//        if (*c == '\n') {
//            y2 += face->size->metrics.height >> 6;
//            x2 = x;
//            continue;
//        }
//        FT_UInt glyph_index = FT_Get_Char_Index(face, *c);
//        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))
//            continue;
//        glBindTexture(GL_TEXTURE_2D, createTexture(face));
//        GLfloat xpos = x2 + face->glyph->bitmap_left * scale;
//        GLfloat ypos = y2 - (face->glyph->bitmap_top -face->glyph->bitmap.rows) * scale;
//        GLfloat w = face->glyph->bitmap.width * scale;
//        GLfloat h = face->glyph->bitmap.rows * scale;
//
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//
//        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//        glDisableVertexAttribArray(0);
//
//        x2 += (face->glyph->advance.x >> 6) * scale;
//        y2 += (face->glyph->advance.y >> 6) * scale;
//    }
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//
//int main() {
//    // Initialize GLFW and GLEW
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    // Create a window
//    GLFWwindow* window = glfwCreateWindow(800, 600, "FreeType OpenGL", NULL, NULL);
//    if (window == NULL) {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    // Initialize GLEW
//    glewExperimental = true;
//    if (glewInit() != GLEW_OK) {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        return -1;
//    }
//
//    // Define the viewport dimensions
//    glViewport(0, 0, 800, 600);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    // Compile shaders
//    const char* vertexShaderSource = "#version 330 core\n"
//        "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
//        "out vec2 TexCoords;\n"
//        "uniform mat4 projection;\n"
//        "void main()\n"
//        "{\n"
//        "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
//        "    TexCoords = vertex.zw;\n"
//        "}";
//    const char* fragmentShaderSource = "#version 330 core\n"
//        "in vec2 TexCoords;\n"
//        "out vec4 color;\n"
//        "uniform sampler2D text;\n"
//        "uniform vec3 textColor;\n"
//        "void main()\n"
//        "{\n"
//        "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
//        "    color = vec4(textColor, 1.0) * sampled;\n"
//        "}";
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//    glCompileShader(vertexShader);
//    checkCompileErrors(vertexShader, "VERTEX");
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//    glCompileShader(fragmentShader);
//    checkCompileErrors(fragmentShader, "FRAGMENT");
//
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//    checkCompileErrors(shaderProgram, "PROGRAM");
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    // Load fonts
//    FT_Library ft;
//    if (FT_Init_FreeType(&ft)) {
//        std::cout << "Failed to initialize FreeType" << std::endl;
//        return -1;
//    }
//    FT_Face face;
//    if (FT_New_Face(ft, "C:/Windows/Fonts/arial.ttf", 0, &face)) {
//        std::cout << "Failed to load font" << std::endl;
//        return -1;
//    }
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    // Set up texture atlas
//    GLuint texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//    // Render loop
//    while (!glfwWindowShouldClose(window)) {
//        // Check for events
//        processInput(window);
//
//        // Clear the screen
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Activate shader program
//        glUseProgram(shaderProgram);
//        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(glm::ortho(0.0f, static_cast<GLfloat>(800), 0.0f, static_cast<GLfloat>(600))));
//
//        // Render text
//     //   renderText("Hello, World!", face, 300.0f, 300.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
//
//        drawText(face, shaderProgram, "Hello, World!", 20.0f, 20.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
//        // Swap buffers and poll for events
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // Clean up
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//    glDeleteProgram(shaderProgram);
//    glDeleteTextures(1, &texture);
//    glfwTerminate();
//    return 0;
//}
//

















//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <iostream>
//#include <string>
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//    glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}
//
//void checkCompileErrors(GLuint shader, std::string type) {
//    GLint success;
//    GLchar infoLog[1024];
//    if (type != "PROGRAM") {
//        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//        if (!success) {
//            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
//            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//        }
//    }
//    else {
//        glGetProgramiv(shader, GL_LINK_STATUS, &success);
//        if (!success) {
//            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
//            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//        }
//    }
//}
//
//GLuint createTexture(FT_Face face) {
//    GLuint texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    return texture;
//}
//
//void drawText(FT_Face face, GLuint shaderProgram, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(0);
//
//    GLfloat x2 = x;
//    GLfloat y2 = y;
//
//    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
//        if (*c == '\n') {
//            y2 += face->size->metrics.height >> 6;
//            x2 = x;
//            continue;
//        }
//        FT_UInt glyph_index = FT_Get_Char_Index(face, *c);
//        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
//        GLuint texture = createTexture(face);
//
//        GLfloat xpos = x2 + face->glyph->bitmap_left * scale;
//        GLfloat ypos = y2 - (face->glyph->bitmap_top - face->glyph->bitmap.rows) * scale;
//
//        GLfloat w = face->glyph->bitmap.width * scale;
//        GLfloat h = face->glyph->bitmap.rows * scale;
//
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glBindVertexArray(VAO);
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        x2 += (face->glyph->advance.x >> 6) * scale;
//        y2 += (face->glyph->advance.y >> 6) * scale;
//        glDeleteTextures(1, &texture);
//    }
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//int main()
//{
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    GLFWwindow* window = glfwCreateWindow(800, 600, "Text Rendering with FreeType", NULL, NULL);
//    if (window == NULL) {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    glfwMakeContextCurrent(window);
//
//    glewExperimental = GL_TRUE;
//    if (glewInit() != GLEW_OK) {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        return -1;
//    }
//
//    glViewport(0, 0, 800, 600);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    FT_Library ft;
//    if (FT_Init_FreeType(&ft)) {
//        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
//        return -1;
//    }
//
//    FT_Face face;
//    if (FT_New_Face(ft, "arial.ttf", 0, &face)) {
//        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
//        return -1;
//    }
//
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    GLuint shaderProgram = glCreateProgram();
//    const char* vertexShaderSource = "#version 330 core\n"
//        "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
//        "out vec2 TexCoords;\n"
//        "uniform mat4 projection;\n"
//        "void main()\n"
//        "{\n"
//        "   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0); \n"
//        "   TexCoords = vertex.zw;\n"
//        "}\0";
//    const char* fragmentShaderSource = "#version 330 core\n"
//        "in vec2 TexCoords;\n"
//        "out vec4 color;\n"
//        "uniform sampler2D text;\n"
//        "uniform vec3 textColor;\n"
//        "void main()\n"
//        "{\n"
//        "   vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
//        "   color = vec4(textColor, 1.0) * sampled;\n"
//        "}\n\0";
//
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//    glCompileShader(vertexShader);
//
//    GLint success;
//    GLchar infoLog[512];
//    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//    }
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//    glCompileShader(fragmentShader);
//    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
//    }
//
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//    if (!success) {
//        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//    }
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    GLuint VAO, VBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(800), 0.0f, static_cast<GLfloat>(600));
//
//    glUseProgram(shaderProgram);
//    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);
//
//    while (!glfwWindowShouldClose(window))
//    {
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        renderText(shaderProgram, "Hello, World!", 25.0f, 25.0f, 1.0f, glm::vec3(0.5f, 0.8f, 0.2f));
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteProgram(shaderProgram);
//
//    glfwTerminate();
//    return 0;
//}
//
//void renderText(GLuint shaderProgram, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
//{
//    // Activate corresponding render state	
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(VAO);
//
//    // Iterate through all characters
//    std::string::const_iterator c;
//    for (c = text.begin(); c != text.end(); c++)
//    {
//        Character ch = Characters[*c];
//
//        GLfloat xpos = x + ch.Bearing.x * scale;
//        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
//
//        GLfloat w = ch.Size.x * scale;
//        GLfloat h = ch.Size.y * scale;
//        // Update VBO for each character
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//        // Render glyph texture over quad
//        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
//        // Update content of VBO memory
//        glBindBuffer(GL_ARRAY_BUFFER, VBO);
//        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        // Render quad
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//        // Now advance cursors for next glyph
//        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
//    }
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//

























//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <iostream>
//#include <string>
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//	glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}
//
//void checkCompileErrors(GLuint shader, std::string type) {
//    GLint success;
//    GLchar infoLog[1024];
//    if (type != "PROGRAM") {
//        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//        if (!success) {
//            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
//            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//        }
//    }
//    else {
//        glGetProgramiv(shader, GL_LINK_STATUS, &success);
//        if (!success) {
//            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
//            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
//        }
//    }
//}
//
//GLuint createTexture(FT_Face face) {
//    GLuint texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    return texture;
//}
//
//void drawText(FT_Face face, GLuint shaderProgram, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindVertexArray(0);
//
//    GLfloat x2 = x;
//    GLfloat y2 = y;
//
//    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c) {
//        if (*c == '\n') {
//            y2 += face->size->metrics.height >> 6;
//            x2 = x;
//            continue;
//        }
//        FT_UInt glyph_index = FT_Get_Char_Index(face, *c);
//        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
//        GLuint texture = createTexture(face);
//
//        GLfloat xpos = x2 + face->glyph->bitmap_left * scale;
//        GLfloat ypos = y2 - (face->glyph->bitmap_top - face->glyph->bitmap.rows) * scale;
//        GLfloat w = face->glyph->bitmap.width * scale;
//        GLfloat h = face->glyph->bitmap.rows * scale;
//
//        GLfloat vertices[6][4] = {
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos,     ypos,       0.0, 1.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//
//            { xpos,     ypos + h,   0.0, 0.0 },
//            { xpos + w, ypos,       1.0, 1.0 },
//            { xpos + w, ypos + h,   1.0, 0.0 }
//        };
//
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glBindVertexArray(0);
//        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        x2 += (face->glyph->advance.x >> 6) * scale;
//        y2 += (face->glyph->advance.y >> 6) * scale;
//
//        glDeleteTextures(1, &texture);
//    }
//}
//
//GLuint createShaderProgram() {
//    const GLchar* vertexShaderSource =
//        "#version 330 core\n"
//        "layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>\n"
//        "out vec2 TexCoords;\n"
//        "uniform mat4 projection;\n"
//        "void main()\n"
//        "{\n"
//        "   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
//        "   TexCoords = vertex.zw;\n"
//        "}\0";
//
//    const GLchar* fragmentShaderSource =
//        "#version 330 core\n"
//        "in vec2 TexCoords;\n"
//        "out vec4 color;\n"
//        "uniform sampler2D text;\n"
//        "uniform vec3 textColor;\n"
//        "void main()\n"
//        "{\n"
//        "   vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
//        "   color = vec4(textColor, 1.0) * sampled;\n"
//        "}\n\0";
//
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//    glCompileShader(vertexShader);
//    checkCompileErrors(vertexShader, "VERTEX");
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//    glCompileShader(fragmentShader);
//    checkCompileErrors(fragmentShader, "FRAGMENT");
//
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//    checkCompileErrors(shaderProgram, "PROGRAM");
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    return shaderProgram;
//}
//
//int main()
//{
//    glfwInit();
//    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Text Rendering", NULL, NULL);
//    glfwMakeContextCurrent(window);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    glewExperimental = GL_TRUE;
//    glewInit();
//
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    FT_Library ftLibrary;
//    if (FT_Init_FreeType(&ftLibrary)) {
//        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
//        return -1;
//    }
//
//    FT_Face face;
//    if (FT_New_Face(ftLibrary, "C:/Windows/Fonts/arial.ttf", 0, &face)) {
//        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
//        return -1;
//    }
//
//    FT_Set_Pixel_Sizes(face, 0, 48);
//
//    GLuint VAO, VBO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
//
//    GLuint shaderProgram = createShaderProgram();
//    glUseProgram(shaderProgram);
//    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(800), 0.0f, static_cast<GLfloat>(600));
//    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//
//    while (!glfwWindowShouldClose(window)) {
//        processInput(window);
//
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        drawText(face, shaderProgram, "Hello, World!", 200, 300, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    glDeleteProgram(shaderProgram);
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//
//    FT_Done_Face(face);
//    FT_Done_FreeType(ftLibrary);
//
//    glfwTerminate();
//    return 0;
//}














//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
//
//#include <iostream>
//#include <string>
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//    glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow* window) {
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//        glfwSetWindowShouldClose(window, true);
//    }
//}
//
//// Функция для загрузки шрифта
//void loadFont(FT_Library& ft, FT_Face& face, const char* fontPath, int fontSize) {
//    FT_Init_FreeType(&ft);
//    FT_New_Face(ft, fontPath, 0, &face);
//    FT_Set_Pixel_Sizes(face, 0, fontSize);
//}
//
//// Функция для создания текстуры OpenGL из шрифта
//void createTexture(FT_Face& face, GLuint& texture) {
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glActiveTexture(GL_TEXTURE0);
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    for (unsigned char c = 0; c < 128; c++) {
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//            continue;
//        }
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
//    }
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//// Функция для вывода текстуры на экран
//void drawText(const char* text, FT_Face& face, GLuint texture, int x, int y, int fontSize) {
//    glUseProgram(0);
//    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//    glPushMatrix();
//    glTranslatef(x, y, 0);
//    glScalef(fontSize, fontSize, 1.0f);
//    float x2, y2;
//    FT_GlyphSlot g = face->glyph;
//    for (const char* p = text; *p; p++)
//    {
//        if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
//            continue;
//        }
//        x2 = x + g->bitmap_left * fontSize / g->bitmap.width;
//        y2 = -y - g->bitmap_top * fontSize / g->bitmap.width;
//        float w = g->bitmap.width * fontSize / g->bitmap.width;
//        float h = g->bitmap.rows * fontSize / g->bitmap.width;
//        glBegin(GL_QUADS);
//        glTexCoord2f(0, 0);
//        glVertex2f(x2, -y2);
//        glTexCoord2f(1, 0);
//        glVertex2f(x2 + w, -y2);
//        glTexCoord2f(1, 1);
//        glVertex2f(x2 + w, -y2 - h);
//        glTexCoord2f(0, 1);
//        glVertex2f(x2, -y2 - h);
//        glEnd();
//        x += (g->advance.x >> 6) * fontSize / g->bitmap.width;
//        y += (g->advance.y >> 6) * fontSize / g->bitmap.width;
//    }
//    glPopMatrix();
//    glBindTexture(GL_TEXTURE_2D, 0);
//    glDisable(GL_BLEND);
//    glDisable(GL_TEXTURE_2D);
//}
//
//int main() {
//    // Инициализация GLFW
//    if (!glfwInit()) {
//        std::cerr << "Failed to initialize GLFW" << std::endl;
//        return -1;
//    }
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
//    if (!window) {
//        std::cerr << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    // Инициализация GLEW
//    if (glewInit() != GLEW_OK) {
//        std::cerr << "Failed to initialize GLEW" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Загрузка шрифта
//    FT_Library ft;
//    FT_Face face;
//    loadFont(ft, face, "C:/Windows/Fonts/arial.ttf", 48);
//
//    // Создание текстуры из шрифта
//    GLuint texture;
//    createTexture(face, texture);
//
//    // Цикл отрисовки окна
//    while (!glfwWindowShouldClose(window)) {
//        processInput(window);
//
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Вывод текста
//        drawText("Hello, World!",face, texture, 100, 100, 48);
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // Освобождение ресурсов
//    glDeleteTextures(1, &texture);
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//    glfwTerminate();
//    return 0;
//}















//#include <iostream>
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <GL/glut.h>
//
//#include <ft2build.h>
//#include FT_FREETYPE_H
//
//
//
//
//
//#include <string>
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//    glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow *window) {
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//        glfwSetWindowShouldClose(window, true);
//    }
//}
//
//// Функция для загрузки шрифта
//int loadFont(FT_Library& ft, FT_Face& face, const char* fontPath, int fontSize) {
//    FT_Init_FreeType(&ft);
//    //FT_New_Face(ft, fontPath, 0, &face);
//    if (FT_New_Face(ft, fontPath, 0, &face)) {
//        fprintf(stderr, "Error setting pixel size for font\n");
//        return -1;
//    }
//    FT_Set_Pixel_Sizes(face, 0, fontSize);
//
//  /*  if (FT_Set_Pixel_Sizes(face, 0, fontSize)) {
//        fprintf(stderr, "Error setting pixel size for font\n");
//        return -1;
//    }*/
//
//
//}
//
//
//
//// Функция для создания текстуры OpenGL из шрифта
//void createTexture(FT_Face& face, GLuint& texture) {
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glActiveTexture(GL_TEXTURE0);
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    for (unsigned char c = 0; c < 128; c++) {
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//            continue;
//        }
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
//    }
//    glBindTexture(GL_TEXTURE_2D, 0);
//}
//
//// Функция для вывода текстуры на экран
//void drawText(const char* text,FT_Face& face, GLuint texture, int x, int y, int fontSize) {
//    glUseProgram(0);
//    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//    glPushMatrix();
//    glTranslatef(x, y, 0);
//    glScalef(fontSize, fontSize, 1.0f);
//    float x2, y2;
//    FT_GlyphSlot g = face->glyph;
//    for (const char* p = text; *p; p++){
//        if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
//            continue;
//        }
//     
//            
//       
//
//        x2 = x + g->bitmap_left * fontSize / g->bitmap.width;
//        y2 = -y - g->bitmap_top * fontSize / g->bitmap.width;
//        float w = g->bitmap.width * fontSize / g->bitmap.width;
//        float h = g->bitmap.rows * fontSize / g->bitmap.width;
//      
//        glBegin(GL_QUADS);
//        glTexCoord2f(0, 0);
//        glVertex2f(x2, -y2);
//        glTexCoord2f(1, 0);
//        glVertex2f(x2 + w, -y2);
//        glTexCoord2f(1, 1);
//        glVertex2f(x2 + w, -y2 - h);
//        glTexCoord2f(0, 1);
//        glVertex2f(x2, -y2 - h);
//        glEnd();
//        x += (g->advance.x >> 6) * fontSize / g->bitmap.width;
//        y += (g->advance.y >> 6) * fontSize / g->bitmap.width;
//       
//    }
//    glPopMatrix();
//    glBindTexture(GL_TEXTURE_2D, 0);
//    glDisable(GL_BLEND);
//    glDisable(GL_TEXTURE_2D);
//}
//
//
///*// Обработчик изменения размера окна
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height);
//}
//
//// Обработчик ввода клавиатуры
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}*/
//
//void angspd(float &angle,float &speed) {
//    angle += speed;
//    if (angle >= 1.0f) {
//        angle = 1.0f;
//        speed = -speed;
//    }
//    else if (angle <= 0.0f) {
//        angle = 0.0f;
//        speed = -speed;
//    }
//}
//
///*void renderText(const char* text, float x, float y) {
//    glRasterPos2f(x, y);
//
//    for (const char* c = text; *c != '\0'; c++) {
//        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
//    }
//}*/
//
//int main(int argc, char** argv)
//{
///*    // Инициализация GLUT
//    glutInit(&argc, argv);
//
//    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
//    glutCreateWindow("My OpenGL Window");*/
//    // Инициализация GLFW
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    // Создание окна
//    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
//    if (!window)
//    {
//        std::cerr << "Ошибка при создании GLFW окна" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    // Инициализация GLAD
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cerr << "Ошибка при инициализации GLAD" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    // Установка размера окна и обработчика изменения размера окна
//    glViewport(0, 0, 800, 600);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    // Создание шейдеров
//    const char* vertexShaderSource =
//            "#version 330 core\n"
//            "layout (location = 0) in vec3 aPos;\n"
//            "layout (location = 1) in vec3 aColor;\n"
//            "uniform mat4 transform;\n"
//            "out vec3 color;\n"
//            "void main()\n"
//            "{\n"
//            "   gl_Position = transform * vec4(aPos, 1.0);\n"
//            "   color = aColor;\n"
//            "}\n";
//    // Создание вершинных данных
//    float vertices[] = {
//
//
//            // Верхняя грань
//            -0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f,
//            0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f,
//            0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f,
//            -0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f,
//            0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f,
//            -0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f,
//
//            // Нижняя грань
//            -0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f,
//            0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f,
//            0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f,
//            -0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f,
//            0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f,
//            -0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f,
//
//            // Левая грань
//            -0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f,
//            -0.5f,  0.5f, -0.5f, 0.f, 0.f, 1.f,
//            -0.5f,  0.5f,  0.5f, 0.f, 0.f, 1.f,
//            -0.5f, -0.5f,  0.5f, 0.f, 0.f, 1.f,
//            -0.5f,  0.5f,  0.5f, 0.f, 0.f, 1.f,
//            -0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f,
//
//            // Правая грань
//            0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f,
//            0.5f,  0.5f, -0.5f, 1.f, 1.f, 0.f,
//            0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f,
//            0.5f, -0.5f, 0.5f, 1.f, 1.f, 0.f,
//            0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f,
//            0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f,
//
//            // Задняя грань
//            -0.5f, -0.5f, -0.5f, 1.f, 0.f, 1.f,
//            0.5f, -0.5f, -0.5f, 1.f, 0.f, 1.f,
//            0.5f,  0.5f, -0.5f, 1.f, 0.f, 1.f,
//            -0.5f,  0.5f, -0.5f, 1.f, 0.f, 1.f,
//            0.5f,  0.5f, -0.5f, 1.f, 0.f, 1.f,
//            -0.5f, -0.5f, -0.5f, 1.f, 0.f, 1.f,
//
//            // Передняя грань
//            -0.5f, -0.5f, 0.5f, 0.f, 1.f, 1.f,
//            0.5f, -0.5f, 0.5f, 0.f, 1.f, 1.f,
//            0.5f,  0.5f, 0.5f, 0.f, 1.f, 1.f,
//            -0.5f,  0.5f, 0.5f, 0.f, 1.f, 1.f,
//            0.5f,  0.5f, 0.5f, 0.f, 1.f, 1.f,
//            -0.5f, -0.5f, 0.5f, 0.f, 1.f, 1.f,
//    };
//
//
//
//    const char* fragmentShaderSource =
//            "#version 330 core\n"
//            "in vec3 color;\n"
//            "out vec4 FragColor;\n"
//            "void main()\n"
//            "{\n"
//            "   FragColor = vec4(color, 1.0f);\n"
//            "}\n";
//
//
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
//    glCompileShader(vertexShader);
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
//    glCompileShader(fragmentShader);
//
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    GLuint VBO, VAO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//
//    glBindVertexArray(VAO);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//    glBindVertexArray(0);
//    float angle = 0.0f;
//    float speed = 0.01f;
//    // Включаем отсечение задних граней
//    // glEnable(GL_CULL_FACE);
//    // glEnable(GL_DEPTH_TEST);
//    // Задаем направление отсечения на лицевые грани
//    // glCullFace(GL_BACK);
//
//// Загрузка шрифта
//    FT_Library ft;
//    FT_Face face;
//    loadFont(ft, face, "C:/Windows/Fonts/arial.ttf", 48);
//// Создание текстуры из шрифта
//    GLuint texture;
//    createTexture(face, texture);
//
//
//    // Цикл отрисовки
//    while (!glfwWindowShouldClose(window))
//    {
//        // Обработка ввода клавиатуры
//        processInput(window);
//        glEnable(GL_DEPTH_TEST);
//
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        // Очистка экрана
//        glClearColor(0.0f, angle, 1.0f, 1.0f); // установка цвета фона в сине-голубой
//        glClear(GL_COLOR_BUFFER_BIT);
//        angspd(angle, speed);
//        // Вывод текста
//        drawText("Hello, World!",face, texture, 100, 100, 48);
//
//
//
//        // Получение матрицы преобразования
//        glm::mat4 transform = glm::mat4(1.0f);
//        transform = glm::rotate(transform, (float)glfwGetTime()/1, glm::vec3(1.0f, 1.0f, 0.0f));
//
//        // Установка шейдера
//        glUseProgram(shaderProgram);
//
//        // Установка матрицы преобразования в шейдере
//        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
//        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
//
//        // Отрисовка треугольника
//        glBindVertexArray(VAO);
//        glDrawArrays(GL_TRIANGLES, 0, 36);
//
///*        // Отображение текста
//        renderText("Hello, world!", -0.1, 0.1);*/
//
//
//        // Проверка и обработка событий окна
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // Очистка ресурсов
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteProgram(shaderProgram);
//
//// Освобождение ресурсов
//   glDeleteTextures(1, &texture);
//    FT_Done_Face(face);
//    FT_Done_FreeType(ft);
//
//    // Завершение работы GLFW
//    glfwTerminate();
//
//    return 0;
//}

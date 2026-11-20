// Shader.h: prepara los programas que le dicen a la tarjeta gráfica cómo dibujar y pintar los objetos.
// Se encarga de leerlos desde sus archivos, compilarlos y activarlos.
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

// Un programa de dibujo ya listo para usarse.
class Shader
{
public:
	// Identificador del programa y lugar donde se guarda el color.
	GLuint Program;
	GLuint uniformColor;
	// Crea el shader: lee sus dos archivos, los compila y los une en un solo programa.
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// Paso 1: se leen los dos archivos del shader.
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// Se prepara la lectura para que avise si hay un error.
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		try
		{
			// Se abren los archivos.
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// Se lee su contenido.
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// Se cierran los archivos.
			vShaderFile.close();
			fShaderFile.close();
			// Se guarda el contenido como texto.
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		// Paso 2: se compilan los shaders.
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];
		// Primer shader: el que ubica los puntos del modelo en pantalla.
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Si la compilación falla, se muestra el error en la consola.
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Segundo shader: el que decide el color de cada punto de la imagen.
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Si la compilación falla, se muestra el error en la consola.
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Se unen los dos shaders en un solo programa.
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// Si la unión falla, se muestra el error en la consola.
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Se guarda dónde está el dato "color" dentro del programa.
		uniformColor = glGetUniformLocation(this->Program, "color");
		// Los shaders sueltos ya no se necesitan, porque quedaron dentro del programa.
		glDeleteShader(vertex);
		glDeleteShader(fragment);

	}
	// Activa este shader para que se use al dibujar.
	void Use()
	{
		glUseProgram(this->Program);
	}

	// Regresa dónde está el dato "color" del programa.
	GLuint getColorLocation()
	{
		return uniformColor;
	}
};

#endif
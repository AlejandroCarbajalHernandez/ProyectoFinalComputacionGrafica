#pragma once

// Librerías estándar de C++
#include <vector>

// Librerías de OpenGL y de matemáticas para gráficos
#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Movimientos posibles de la cámara: adelante, atrás, izquierda y derecha.
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Valores iniciales de la cámara: hacia dónde mira, qué tan rápido se mueve, qué tan sensible es el mouse y cuánto abre la vista.
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 6.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;

// La cámara con la que se recorre el escenario: guarda dónde está y hacia dónde mira, y se mueve con el teclado y el mouse.
class Camera
{
public:
	// Crea la cámara indicando su posición, su "arriba" y hacia dónde mira.
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// Crea la cámara igual que la anterior, pero dando cada número por separado.
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVTY), zoom(ZOOM)
	{
		this->position = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	// Coloca la camara en una posicion y orientacion concretas (usado por las teclas de ubicacion)
	void SetView(glm::vec3 newPosition, GLfloat newYaw, GLfloat newPitch)
	{
		this->position = newPosition;
		this->yaw = newYaw;
		this->pitch = newPitch;
		this->updateCameraVectors();
	}

	// Entrega lo que la cámara está viendo en este momento, para dibujar la escena desde ahí.
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->front, this->up);
	}

	// Mueve la cámara en la dirección pedida; entre más tiempo pasa, más avanza.
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->movementSpeed * deltaTime;

		if (direction == FORWARD)
		{
			this->position += this->front * velocity;
		}

		if (direction == BACKWARD)
		{
			this->position -= this->front * velocity;
		}

		if (direction == LEFT)
		{
			this->position -= this->right * velocity;
		}

		if (direction == RIGHT)
		{
			this->position += this->right * velocity;
		}
	}

	// Gira la cámara según cuánto se movió el mouse.
	void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
	{
		xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;

		// Se limita la mirada hacia arriba y abajo para que la imagen no se voltee.
		if (constrainPitch)
		{
			if (this->pitch > 89.0f)
			{
				this->pitch = 89.0f;
			}

			if (this->pitch < -89.0f)
			{
				this->pitch = -89.0f;
			}
		}

		// Con el nuevo giro se actualizan las direcciones de la cámara.
		this->updateCameraVectors();
	}

	// Rueda del mouse: por ahora no hace nada.
	void ProcessMouseScroll(GLfloat yOffset)
	{

	}

	// Regresa cuánto abre la vista la cámara.
	GLfloat GetZoom()
	{
		return this->zoom;
	}

	// Regresa dónde está la cámara.
	glm::vec3 GetPosition()
	{
		return this->position;
	}

	// Regresa hacia dónde mira la cámara.
	glm::vec3 GetFront()
	{
		return this->front;
	}

private:
	// Dónde está la cámara y hacia dónde mira.
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	// Cuánto ha girado la cámara a los lados y hacia arriba o abajo.
	GLfloat yaw;
	GLfloat pitch;

	// Qué tan rápido se mueve, qué tan sensible es el mouse y cuánto abre la vista.
	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	// Recalcula hacia dónde mira la cámara cada vez que cambia su giro.
	void updateCameraVectors()
	{
		// Se calcula la nueva dirección hacia el frente.
		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);
		// También se recalculan las direcciones de la derecha y de arriba.
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));  
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}
};
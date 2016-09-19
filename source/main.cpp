/*
Title: 2D Cameras
File Name: main.cpp
Copyright � 2016
Author: David Erbelding
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>
#include "../header/shape.h"
#include "../header/transform2d.h"
#include "../header/shader.h"
#include <iostream>

Shape* square;

// The transform being used to draw our shape
Transform2D transform;

// These shader objects wrap the functionality of loading and compiling shaders from files.
Shader vertexShader;
Shader fragmentShader;

// GL index for shader program
GLuint shaderProgram;

// Index of the world matrix in the vertex shader.
GLuint worldMatrixUniform;

// Index of the camera matrix in the vertex shader.
GLuint cameraMatrixUniform;

// Here we store the position, of the camera.
glm::vec2 cameraPosition;

// Window resize callback
void resizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Initialize window
	GLFWwindow* window = glfwCreateWindow(800, 600, "2D Cameras", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	//set resize callback
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	// Initializes the glew library
	glewInit();


	// Indices for square (-1, -1)[2] to (1, 1)[1]
	// [0]------[1]
	//	|		 |
	//	|		 |
	//	|		 |
	// [2]------[3]

	// Create square vertex data.
	std::vector<glm::vec2> vertices;
	vertices.push_back(glm::vec2(-1, 1));
	vertices.push_back(glm::vec2(1, 1));
	vertices.push_back(glm::vec2(-1, -1));
	vertices.push_back(glm::vec2(1, -1));

	std::vector<unsigned int> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(2);
	indices.push_back(1);


	// Create shape object
	square = new Shape(vertices, indices);
	

	transform.SetScale(.25f);
	transform.SetPosition(glm::vec2(.25, .25));
    



	// SHADER STUFF
	
	// 
    std::string vertexShaderCode =
        "#version 400 core \n"

        // vertex position attribute
        "layout(location = 0) in vec2 in_position;"

        // uniform variables
        "uniform mat3 worldMatrix;"
        "uniform mat3 cameraView;"

		"void main(void)"
		"{"
			// Multiply the position by the world matrix to convert from local to world space.
	        "vec3 worldPosition = worldMatrix * vec3(in_position, 1);"
            
            // Now, we multiply by the view matrix to get everything in view space.
            "vec3 viewPosition = cameraView * worldPosition;"
            
			// output the transformed vector as a vec4.
			"gl_Position = vec4(viewPosition, 1);"
		"}";



	// Compile the vertex shader.
	vertexShader.InitFromString(vertexShaderCode, GL_VERTEX_SHADER);

	// Load and compile the fragment shader.
	fragmentShader.InitFromFile("../shaders/fragment.glsl", GL_FRAGMENT_SHADER);





	// Create a shader program.
	shaderProgram = glCreateProgram();
	
	// Attach the vertex and fragment shaders to our program.
	vertexShader.AttachTo(shaderProgram);
	fragmentShader.AttachTo(shaderProgram);

	// Build shader program.
	glLinkProgram(shaderProgram);

	// After the program has been linked, we can ask it where it put our world matrix and camera matrix
	worldMatrixUniform = glGetUniformLocation(shaderProgram, "worldMatrix");

    cameraMatrixUniform = glGetUniformLocation(shaderProgram, "cameraView");


    std::cout << "Use WASD to move the camera!" << std::endl;



	// Main Loop
	while (!glfwWindowShouldClose(window))
	{
        // Calculate delta time.
        float dt = glfwGetTime();
        // Reset the timer.
        glfwSetTime(0);

		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 0.0);


		// rotate square
		transform.Rotate(1.0f * dt);
		
        // Here we get some input, and use it to move the camera
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPosition.y += dt;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cameraPosition.x -= dt;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPosition.y -= dt;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cameraPosition.x += dt;
        }


        // Cameras use a transformations matrix just like other renderable objects.
        // When we multiply by a world matrix, we are moving an object from local space to world space.
        // When using a camera, we do the exact opposite. We move everything else from world space into camera local space.
        // To do this we make a matrix that does the inverse of what a world matrix does.
        glm::mat3 cameraMatrix;
        
        // For this example, we'll 
        // The inverse of a translation matrix is the same matrix with negative translation values.
        glm::mat3 translation = glm::mat3(
            1, 0, 0,
            0, 1, 0,
            -cameraPosition.x, -cameraPosition.y, 1
            );


        cameraMatrix = translation;


		// Set the current shader program.
		glUseProgram(shaderProgram);

        // Send the camera matrix to the shader
        glUniformMatrix3fv(cameraMatrixUniform, 1, GL_FALSE, &(cameraMatrix[0][0]));
		
		// Draw using the worldMatrixUniform
		square->Draw(transform.GetMatrix(), worldMatrixUniform);



		// Stop using the shader program.
		glUseProgram(0);

		// Swap the backbuffer to the front.
		glfwSwapBuffers(window);

		// Poll input and window events.
		glfwPollEvents();

	}

	// Free memory from shader program and individual shaders
	glDeleteProgram(shaderProgram);


	// Free memory from shape object
	delete square;

	// Free GLFW memory.
	glfwTerminate();


	// End of Program.
	return 0;
}

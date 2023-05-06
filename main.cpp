/*
Author: <Qing Yu>
Class: ECE6122
Last Date Modified: <2021/12/7>
Description:
    FinalProject;
    No collision detection;
	10 extra bonus points for using a bitmap file called ff.bmp;
	extra 20 extra bonus points place a texture map on a 3D object;
*/


// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

#include<Windows.h>
#include <GL/glut.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include "uavs.h"

#include <iostream>


int main( void )
{

	ECE_UAV uav[15];
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1920, 1080, "FinalProject", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
    
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1920/2, 1080/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 




	

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
	GLuint programID1 = LoadShaders("StandardShading.vertexshader", "StandardTransparentShading.fragmentshader");

	//One program for court and uavs
	glUseProgram(programID);

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	mat4 ProjectionMatrix = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
	mat4 ViewMatrix = lookAt(
		vec3(0, -150, 100), 
		vec3(0, 0, 0),
		vec3(0, 1, 0)  
	);

	// Load the texture
	GLuint Texture[2];
	Texture[0] = loadDDS("uvmap.DDS");
	Texture[1] = loadBMP_custom("ff.bmp");
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our uav obj file
	vector<vec3> vertices;
	vector<vec2> uvs;
	vector<vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	vector<unsigned short> indices;
	vector<vec3> indexed_vertices;
	vector<vec2> indexed_uvs;
	vector<vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Read our sphere obj file
	vector<vec3> sphereVertices;
	vector<vec2> sphereUvs;
	vector<vec3> sphereNormals;
	bool res1 = loadOBJ("sphere.obj", sphereVertices, sphereUvs, sphereNormals);

	vector<unsigned short> sphereIndices;
	vector<vec3> sphere_indexed_vertices;
	vector<vec2> sphere_indexed_uvs;
	vector<vec3> sphere_indexed_normals;
	indexVBO(sphereVertices, sphereUvs, sphereNormals, sphereIndices, sphere_indexed_vertices, sphere_indexed_uvs, sphere_indexed_normals);

	
	//bind to uavs
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Load it into 15 VBO
	GLuint vertexbuffer[15];
	
	GLuint uvbuffer[15];
	

	GLuint normalbuffer[15];
	

	// Generate a buffer for the indices as well
	GLuint elementbuffer[15];
	

	for(int i = 0; i < 15; ++i )
	{
		
		glGenBuffers(1, &vertexbuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

		
		glGenBuffers(1, &uvbuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	
		glGenBuffers(1, &normalbuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);


		glGenBuffers(1, &elementbuffer[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
	}
	glBindVertexArray(0);

	//bind to football court
	GLuint VertexArrayID1;
	glGenVertexArrays(1, &VertexArrayID1);
	glBindVertexArray(VertexArrayID1);

	static const GLfloat g_vertex_buffer_data[] = {
		50.0f,  35.0f, 0.0f,
		50.0f, -35.0f, 0.0f,
		-50.0f, -35.0f, 0.0f,
		-50.0, -35.0f, 0.0f,
		-50.0f, 35.0f, 0.0f ,
		50.0f, 35.0f, 0.0f


	};

	static const GLfloat g_uv_buffer_data[] = {
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

	GLuint vertexbuffer1;
	glGenBuffers(1, &vertexbuffer1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer1;
	glGenBuffers(1, &uvbuffer1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	glBindVertexArray(0);


	// Get a handle for our "LightPosition" uniform
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	vec3 lightPos = vec3(0, -200, 100);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"


	glUseProgram(0);

	//Another program for semitransparent sphere
	glUseProgram(programID1);

	GLuint MatrixID1 = glGetUniformLocation(programID1, "MVP");
	GLuint ViewMatrixID1 = glGetUniformLocation(programID1, "V");
	GLuint ModelMatrixID1 = glGetUniformLocation(programID1, "M");

	mat4 ProjectionMatrix1 = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
	mat4 ViewMatrix1 = lookAt(
		vec3(0, -150, 100), 
		vec3(0, 0, 0), 
		vec3(0, 1, 0) 
	);


	GLuint Texture1;
	Texture1 = loadDDS("uvmap.DDS");
	GLuint TextureID1 = glGetUniformLocation(programID1, "myTextureSampler");

	//bind to sphere obj
	GLuint sphere_VertexArrayID;
	glGenVertexArrays(1, &sphere_VertexArrayID);
	glBindVertexArray(sphere_VertexArrayID);

	GLuint sphere_vertexbuffer;
	glGenBuffers(1, &sphere_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sphere_indexed_vertices.size() * sizeof(vec3), &sphere_indexed_vertices[0], GL_STATIC_DRAW);

	GLuint sphere_uvbuffer;
	glGenBuffers(1, &sphere_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sphere_indexed_uvs.size() * sizeof(vec2), &sphere_indexed_uvs[0], GL_STATIC_DRAW);

	GLuint sphere_normalbuffer;
	glGenBuffers(1, &sphere_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sphere_indexed_normals.size() * sizeof(vec3), &sphere_indexed_normals[0], GL_STATIC_DRAW);


	GLuint sphere_elementbuffer;
	glGenBuffers(1, &sphere_elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned short), &sphereIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	GLuint LightID1 = glGetUniformLocation(programID1, "LightPosition_worldspace");
	vec3 lightPos1 = vec3(0, -200, 100);
	glUniform3f(LightID1, lightPos1.x, lightPos1.y, lightPos1.z);
	glUniformMatrix4fv(ViewMatrixID1, 1, GL_FALSE, &ViewMatrix1[0][0]); 


	glUseProgram(0);



	//set initial posotion
	int ii = 0;
	for (ii = 0; ii < 5; ++ii)
	{
		uav[ii].iniPosition[0] = -39 + ii * 19;
		uav[ii].iniPosition[1] = 30;
		uav[ii].iniPosition[2] = 0.5;
	}
	for (ii = 5; ii < 10; ++ii)
	{
		uav[ii].iniPosition[0] = -39 + (ii - 5) * 19;
		uav[ii].iniPosition[1] = 0;
		uav[ii].iniPosition[2] = 0.5;
	}
	for (ii = 10; ii < 15; ++ii)
	{
		uav[ii].iniPosition[0] = -39 + (ii - 10) * 19;
		uav[ii].iniPosition[1] = -30;
		uav[ii].iniPosition[2] = 0.5;
	}

	//start the threads
	for (int jj = 0; jj < 15; ++jj)
	{
		uav[jj].start();
	}

	int colorInterval = 0;
	bool begin = true;
	double beginTime = 0;
	double currentTime = 0;
	double elapsedTime = 0;

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
		
		////// Start of the rendering of the uav objects //////
		
		// Use our shader
		glUseProgram(programID);

		//In the fragmentshader, vec3 MaterialDiffuseColor = factor * texture( myTextureSampler, UV ).rgb, change factor in order to change the color of uavs
		GLint color_location = glGetUniformLocation(programID, "factor");
		if(((colorInterval / 1500) % 2) == 0) // Change to half color every 1.5s
		    glUniform1f(color_location, 0.5f);
	
		glDisable(GL_BLEND);
		glBindVertexArray(VertexArrayID);
	
		//loop 15 uavs
		for (int j = 0; j < 15; ++j)
		{
			mat4 ModelMatrix = mat4(1.0);
			ModelMatrix = translate(ModelMatrix, vec3(uav[j].position[0], uav[j].position[1], uav[j].position[2]));
			ModelMatrix = scale(ModelMatrix, vec3(1.2f, 1.2f, 1.2f));
			mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;


			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);


			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture[0]);
			// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(TextureID, 0);


			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[j]);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[j]);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[j]);
			glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer[j]);

			// Draw the triangles !
			glDrawElements(
				GL_TRIANGLES,      // mode
				indices.size(),    // count
				GL_UNSIGNED_SHORT,   // type
				(void*)0           // element array buffer offset
			);
			////// End of rendering of the uav objects //////
		}



		//stop simulation when all the uavs have flown for 60s
	    //according to the initial position, uav[0] is the last one to reach the sphere
		if (uav[0].start_timer)
		{
			if (begin)
			{
			    beginTime = glfwGetTime();
				begin = false;
			}			
		}
		currentTime = glfwGetTime();
		elapsedTime = currentTime - beginTime;
		if (elapsedTime >= 60)
		{
			for (int ii = 0; ii < 15; ++ii)
			{
				uav[ii].m_stop = true;
			}
		}

		//draw the court
		glUniform1f(color_location, 1.0f);
		glBindVertexArray(VertexArrayID1);

		mat4 ModelMatrix3 = mat4(1.0);
		mat4 MVP3 = ProjectionMatrix * ViewMatrix * ModelMatrix3;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix3[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture[1]);
		glUniform1i(TextureID, 0);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		glUseProgram(0);

		//draw the sphere
		glUseProgram(programID1);

		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(sphere_VertexArrayID);
		mat4 ModelMatrix2 = mat4(1.0);
		ModelMatrix2 = translate(ModelMatrix2, vec3(-10, 0, 50));//This is the coordinate of the most left vertex of the sphere, so the coordinate of the center is (0, 0, 50)
		ModelMatrix2 = scale(ModelMatrix2, vec3(3.0f, 3.0f, 3.0f));
		mat4 MVP2 = ProjectionMatrix1 * ViewMatrix1 * ModelMatrix2;

		glUniformMatrix4fv(MatrixID1, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID1, 1, GL_FALSE, &ModelMatrix2[0][0]);



		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture1);
		glUniform1i(TextureID1, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sphere_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, sphere_uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, sphere_normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_elementbuffer);

		glDrawElements(
			GL_TRIANGLES,      // mode
			sphereIndices.size(),    // count
			GL_UNSIGNED_SHORT, // type
			(void*)0           // element array buffer offset
		);

		glBindVertexArray(0);
		glUseProgram(0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		colorInterval += 30;

		// polls the UAV once every 30 msec
		this_thread::sleep_for(chrono::milliseconds(30));

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer[15]);
	glDeleteBuffers(1, &uvbuffer[15]);
	glDeleteBuffers(1, &vertexbuffer1);
	glDeleteBuffers(1, &uvbuffer1);
	glDeleteBuffers(1, &normalbuffer[15]);
	glDeleteBuffers(1, &elementbuffer[15]);
	glDeleteBuffers(1, &sphere_vertexbuffer);
	glDeleteBuffers(1, &sphere_uvbuffer);
	glDeleteBuffers(1, &sphere_normalbuffer);
	glDeleteBuffers(1, &sphere_elementbuffer);
	glDeleteProgram(programID);
	glDeleteProgram(programID1);
	glDeleteTextures(1, &Texture[2]);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteVertexArrays(1, &VertexArrayID1);
	glDeleteVertexArrays(1, &sphere_VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


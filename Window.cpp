﻿#include "Window.h"

extern glm::mat4 P;
extern glm::mat4 V;
extern GLuint Shader_Geometry; 
extern GLuint Shader_Skybox;
extern GLuint Shader_Coordinate;
extern GLuint Shader_Model;
extern GLuint Shader_SimplePointer;
extern GLuint Shader_BoundBox;
extern GLuint Shader_DisplayLight;
extern GLuint Shader_Water;

std::vector<Cube *> cubeList;
std::vector<Camera *> cameraList;
std::vector<Model *> domino;
std::vector<Model *> otherModels;

int pickType = 0;
int pickStyle = 0;
int idCount = 0;
int leftMousePressed,rightMousePressed;
int disableFog;
int pickObjectFace;
int showCoordinate;
int goRight, goLeft, goUp, goDown, goForward, goBackward;
int displayLightOnCube;
int disableWater;
float mouseX, mouseY;
glm::vec3 ray_dir;
Object* pickObject = NULL;

Cube *cube;
Skybox *skybox;
Camera *currentCam;
Coordinate *coordinate;
SimplePointer *centerRouter;
Fog *fog;
Water *water;

std::vector<std::string> faces
{
	"../assets/night skybox/2/Right.png",
	"../assets/night skybox/2/Left.png",
	"../assets/night skybox/2/Up.png",
	"../assets/night skybox/2/Down.png",
	"../assets/night skybox/2/Back.png",
	"../assets/night skybox/2/Front.png"
};



const char* window_title = "GLFW Starter Project";

//Model * model;
Light lights;
LightDisplay * lightDisplay;

//Model * model1;
//Model * model2;
Sound * sound;
ALuint bgmSource;
Util util;

int keyPressed;
int shiftPressed;
// On some systems you need to change this to the absolute path

// Default camera parameters
glm::vec3 cam_pos(0.0f, 0.0f, 10.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;

bool begin_domino = false;
bool show_boudingbox = false;
bool done_collision = false;
int count = 0;
int count_before = 0;
int domino_index = 0;


void Window::placeObject(int type, int *style)
{
	if (type == 0) return;
	if (pickObject != NULL) {
		glm::vec3 npos = pickObject->getObjectBiasPosition(pickObjectFace);
		switch (type) {
		case 1:
			if (*style >= CUBE_TEXTURE_NUM) *style = 0;
			if (*style < 0) *style = CUBE_TEXTURE_NUM - 1;
			cube = new Cube(idCount++, 1, *style);
			cube->setPosition(npos.x, npos.y, npos.z);
			cubeList.push_back(cube);
			break;
		case 2:break;
		default:break;
		}
		
	}
}

Object* Window::testForCollision(int *face) {
	Object *obj = NULL;
	float value = 999999;
	int fff = 0;
	for (int i = 0; i < cubeList.size(); ++i) {
		//cubeList[i]->getId();
		float j = cubeList[i]->getCollisionValue(currentCam->camera_pos, ray_dir,&fff);
		if (j > 0 && j < value) {
			*face = fff;
			value = j;
			obj = cubeList[i];
		}
	}
	if (obj) {
		obj->setSelected();
	}
	return obj;
}

void Window::loadAllShader() {
	Shader_Geometry = LoadShaders(GEOMETRY_VERTEX_SHADER_PATH, GEOMETRY_FRAGMENT_SHADER_PATH);
	Shader_Skybox = LoadShaders(SKYBOX_VERTEX_SHADER_PATH, SKYBOX_FRAGMENT_SHADER_PATH);
	Shader_Coordinate = LoadShaders(COORDINATE_VERTEX_SHADER_PATH, COORDINATE_FRAGMENT_SHADER_PATH);
	Shader_Model = LoadShaders(MODEL_VERTEX_SHADER_PATH, MODEL_FRAGMENT_SHADER_PATH);
	Shader_DisplayLight = LoadShaders(DISPLAYLIGHT_VERTEX_SHADER_PATH, DISPLAYLIGHT_FRAGMENT_SHADER_PATH);
	Shader_SimplePointer = LoadShaders(SIMPLE_POINTER_VERTEX_SHADER_PATH, SIMPLE_POINTER_FRAGMENT_SHADER_PATH);
	Shader_BoundBox = LoadShaders(BOUNDBOX_VERTEX_SHADER_PATH,BOUNDBOX_FRAGMENT_SHADER_PATH);
	Shader_Water = LoadShaders(WATER_VERTEX_SHADER_PATH, WATER_FRAGMENT_SHADER_PATH);
}

void Window::initialize_objects()
{
	leftMousePressed = 0;
	mouseX = mouseY = MOUSEPOS_INIT_VALUE;
	goRight = goLeft = goUp = goDown = goForward = goBackward = 0;
	showCoordinate = 0;
	disableFog = 1;
	disableWater = 1;
	loadAllShader();

	//	printf("LoadShaders Finished!2 %d\n", Shader_Geometry);
	int min = -2, max = 2;
	for (int i = min; i <= max; ++i)
	{
		for (int j = min; j <= max; ++j) 
		{
			//cube = new Cube(idCount++, 1, glm::vec3(0.94, 1, 1));
			cube = new Cube(idCount++, 1, 0);
			cube->setPosition(i, GROUND_LEVEL -1, j);
			cubeList.push_back(cube);
		}
	}
	bindedCubeVAO = false;

	// toggle them to on
	displayLightOnCube = 1;
	glUseProgram(Shader_Geometry);
	GLuint temp = glGetUniformLocation(Shader_Geometry, "disableLight");
	glUniform1i(temp, displayLightOnCube);
	glUseProgram(Shader_Water);
	temp = glGetUniformLocation(Shader_Water, "disableLight");
	glUniform1i(temp, displayLightOnCube);
	// init skybox
	skybox = new Skybox(idCount++, 1000, &faces);

	// init cam;
	currentCam = new Camera(idCount++, glm::vec3(0, 1, 0), glm::vec3(0, 1, -1), glm::vec3(0, 1, 0));

	// init cor;
	coordinate = new Coordinate(idCount++, 100);

	// init pointer
	centerRouter = new SimplePointer(idCount++, 0, 0, glm::vec3(1, 0, 0));
	ray_dir = glm::vec3(0, 0.5, -1);

	// init fog
	fog = new Fog(glm::vec3(0.2, 0.2, 0.2), currentCam, 2, 15, 0.1);
	fog->addShader(Shader_Geometry);
	fog->addShader(Shader_DisplayLight);
	fog->addShader(Shader_Model);
	fog->addShader(Shader_Water);
	fog->fogUpdate(disableFog);
	printf("Init All Done\n PLEASE TYPE 1-4 to select Object, and use I O to select Texture\n");


	// init water;
	
	water = new Water(idCount++, 300, 200, 0.2, 0, 1, 0,skybox ->getTexture(), currentCam);
	water->setPosition(-150, GROUND_LEVEL - 2, 150);
	// Enables backface culling
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// ------------------FOR TESTING ONLY ---------------------
	for (int i = 0; i < 100; i++)
	{
		Model * model1 = new Model("../Gun/handgun_obj.obj");
		model1->setModelMatrix(glm::translate(glm::mat4(1.0f), { 0,0.5f,0.5f*i })*model1->getUModelMatrix());
		model1->domino_position = glm::vec3(0,0,0.5*i);
		model1->centerAndScale(1.0f);
		model1->setBoundingBox();
		model1->bounding_box->update();
		model1->setCamera(currentCam);
		model1->initShader(Shader_Model);
		domino.push_back(model1);
	}
	
	domino[0]->bounding_box->collision = true;

	// Enable depth buffering
	// glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Related to shaders and z value comparisons for the depth buffer
	glDepthFunc(GL_LEQUAL);
	// Set polygon drawing mode to fill front and back of each polygon
	// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// create a test light
	lights = Light();
	lights.randInit();
	lights.initializeShader(Shader_Model);
	lights.initializeShader(Shader_Geometry);
	lights.initializeShader(Shader_Water);
	//lights.turnAllLightOn();
	lights.updateAllShader();


	lightDisplay = new LightDisplay(&lights, currentCam);
	lightDisplay->initShader(Shader_DisplayLight);
	lightDisplay->update(Shader_DisplayLight);

	sound = new Sound(currentCam);

	// generate a sound buffer to store sound files
	ALuint buf = sound->generateBuffer("../assets/sounds/song_mono.wav");

	// generate a source that plays the buffer
	bgmSource = sound->generateSource(glm::vec3(0, 0, 0));

	// bind the buffer to the source
	sound->bindSourceToBuffer(bgmSource, buf);

	// make the bgm loop (don't use this if you just want it to play once)
	sound->setSourceLooping(bgmSource, true);	// Only if you want the sound to keep on looping!

	// play the sound now
	sound->playSourceSound(bgmSource);
	std::cerr << (sound->isSourcePlaying(bgmSource) ? "Playing BGM!" : "Error, BGM is not playing!") << std::endl;
	// --------------------------------------------------------------
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 2000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
}

void Window::idle_callback()
{
	// Call the update function the cube
	if (goForward) currentCam->cameraMove(0);
	if (goBackward) currentCam->cameraMove(1);
	if (goRight) currentCam->cameraMove(2);
	if (goLeft) currentCam->cameraMove(3);
	if (goUp) currentCam->cameraMove(4);
	if (goDown) currentCam->cameraMove(5);

	// update Camera
	V = currentCam->getCameraV();

	// init intersection
	pickObject = testForCollision(&pickObjectFace);

	//fake the domino collision
	
	if (begin_domino == true) {
		
		glm::mat4 m1, m2, m3;
		
		for (int i = 0; i < domino.size()-1; ++i) {
			if (domino[i]->bounding_box->collision && domino[i]->bounding_box->count_rotate <=70) {
					m1 = glm::translate(glm::mat4(1.0f), -1.0f * domino[i]->domino_position);
					m2 = glm::rotate(glm::mat4(1.0f), 1.0f*glm::pi<float>() / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
					m3 = glm::translate(glm::mat4(1.0f), domino[i]->domino_position);
					domino[i]->setModelMatrix(m3 * m2 * m1 * domino[i]->getUModelMatrix());
					domino[i]->bounding_box->count_rotate++;
			}
			domino[i]->bounding_box->check_collision(domino[i+1]->bounding_box);
		}

		if (domino[domino.size() - 1]->bounding_box->collision == true && count<=90) {

				m1 = glm::translate(glm::mat4(1.0f), -1.0f * domino[domino.size() - 1]->domino_position);
				m2 = glm::rotate(glm::mat4(1.0f), 1.0f*glm::pi<float>() / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				m3 = glm::translate(glm::mat4(1.0f), domino[domino.size() - 1]->domino_position);
				domino[domino.size() - 1]->setModelMatrix(m3 * m2 * m1 * domino[domino.size() - 1]->getUModelMatrix());
				count++;
			
		}
	}


	//water 
	if(!disableWater) water->waterUpdate();

	// fog
	fog->fogUpdate(disableFog);
	/* ---------Test only ----------------*/
	//model->setModelMatrix(glm::rotate(model->getUModelMatrix(), 1.0f*glm::pi<float>() / 180.0f, glm::vec3(1.0f, 1.0f, 0)));
	//model2->setModelMatrix(glm::rotate(model2->getUModelMatrix(), 1.0f*glm::pi<float>() / 180.0f, glm::vec3(1.0f, 1.0f, 0)));
	/* ----------------------------------- */
	sound->updateListener();
}

void Window::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	
	// draw coordinate
	if (showCoordinate) coordinate->draw(glm::mat4(1.0f));

	//draw a cursor
	centerRouter->draw(glm::mat4(1.0f));

	//draw skybox
	skybox->draw(glm::mat4(1.0f));


	// draw cube!!!!!
	drawingCube = true;
	for (int i = 0; i < cubeList.size();++i)
		cubeList[i]->draw(glm::mat4(1.0f));
	bindedCubeVAO = false;
	drawingCube = false;
	

	// draw water;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if(!disableWater) water->draw(glm::mat4(1.0f));
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// test draw model

	//draw domino
	for (int i = 0; i < domino.size(); i++)
	{
		domino[i]->render(Shader_Model);
	}

	lightDisplay->render(Shader_DisplayLight);
	
	if (show_boudingbox == true) {
		glUseProgram(Shader_BoundBox);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i < domino.size(); i++)
		{
			domino[i]->bounding_box->draw(Shader_BoundBox);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glfwPollEvents();

	// Swap buffers
	glfwSwapBuffers(window);
	
}

void Window::mouseButton_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		rightMousePressed = 1;
		if (pickObject != NULL) {
			for (int i = 0; i < cubeList.size(); ++i) {
				if (cubeList[i]->getId() == pickObject->getId()) {
					Cube * ptr = cubeList[i];
					cubeList.erase(cubeList.begin() + i);
					delete ptr;
					break;
				}
			}
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		leftMousePressed = 1;
		placeObject(pickType, &pickStyle);
		
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		rightMousePressed = 0;
		//mouseX = mouseY = MOUSEPOS_INIT_VALUE;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		leftMousePressed = 0;
		mouseX = mouseY = MOUSEPOS_INIT_VALUE;
	}
}

void Window::mouseScroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}
/*
float compute_angle(float x1,float y1,float x2,float y2)  
{
	float r = MAX(Window::width, Window::height);
	r /= 2;
	x1 -= sphereX;
	x2 -= sphereX;
	y1 -= sphereY;
	y2 -= sphereY;
	float len1 = x1 * x1 + y1 * y1;
	float len2 = x2 * x2 + y2 * y2;
	len1 = len1 > r * r - 1 ? r * r : len1;
	len2 = len2 > r * r - 1 ? r * r : len2;

	float z1 = sqrtf(r * r - len1);
	float z2 = sqrtf(r * r - len2);
	glm::vec3 v1(x1, y1, z1);
	glm::vec3 v2(x2, y2, z2);
	v1 = glm::normalize(v1);
	v2 = glm::normalize(v2);
	
	//printf("%f %f %f %f %f %f\n", v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
	return glm::acos(glm::dot(v1,v2)) * 180.0f / glm::pi<float>();
}

glm::vec3 compute_axis(float x1, float y1, float x2, float y2) {
	float r = MAX(Window::width, Window::height);
	r /= 2;
	x1 -= sphereX;
	x2 -= sphereX;
	y1 -= sphereY;
	y2 -= sphereY;
	float len1 = x1 * x1 + y1 * y1;
	float len2 = x2 * x2 + y2 * y2;
	len1 = len1 > r * r - 1 ? r * r : len1;
	len2 = len2 > r * r - 1 ? r * r : len2;

	float z1 = sqrtf(r * r - len1);
	float z2 = sqrtf(r * r - len2);
	//printf("%f %f\n", z1, z2);
	glm::vec3 v1(x1, y1, z1);
	glm::vec3 v2(x2, y2, z2);
	v1 = glm::normalize(v1);
	v2 = glm::normalize(v2);
	glm::vec3 re = glm::cross(v1, v2);
	//printf("%f %f %f %f %f %f\n", v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
	return re;
}
*/

void Window::mousePos_callback(GLFWwindow* window, double xpos, double ypos) {

	if (leftMousePressed == 0) {
		if (mouseX == MOUSEPOS_INIT_VALUE) {
			mouseX = xpos;
			mouseY = ypos;
		}
		else {
			glm::vec2 delta = glm::vec2((xpos - mouseX) * 7 / Window::width, (mouseY - ypos) * 7 / Window::height);
			//printf("%f %f\n", delta.x, delta.y);
			currentCam->cameraRotate(delta);
			mouseX = xpos;
			mouseY = ypos;
			float x = 0; // (2.0f * xpos) / Window::width - 1.0f;
			float y = 0; // 1.0f - (2.0f * ypos) / Window::height;
			float z = 1.0f;
			//printf("%f %f %f\n", x, y, z);
			glm::vec4 ray = glm::vec4(x, y, z, 1.0);
			glm::vec4 ray_eye = glm::inverse(P) * ray;
			glm::vec4 ray_world = glm::inverse(V) * ray_eye;
			//printf("%f %f %f %f\n", ray_world.x / ray_world.w, ray_world.y / ray_world.w, ray_world.z / ray_world.w, ray_world.w);
			ray_dir = glm::normalize(glm::vec3(ray_world.x / ray_world.w, ray_world.y / ray_world.w, ray_world.z / ray_world.w) - currentCam->camera_pos);
		}

		// sets the cursor to the center of the screen
		//glfwSetCursorPos(window, Window::width / 2, Window::height / 2);

		// hides cursor away from screen
		// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}


}

static bool light_toggle = false;
void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
		{
			shiftPressed = 1;
		}
		if (key == GLFW_KEY_W) {
			goForward = 1;
			goBackward = 0;
		}
		if (key == GLFW_KEY_S) {
			goForward = 0;
			goBackward = 1;
		}
		if (key == GLFW_KEY_A) {
			goLeft = 1;
			goRight = 0;
		}
		if (key == GLFW_KEY_D) {
			goRight = 1;
			goLeft = 0;
		}
		if (key == GLFW_KEY_E) {
			goUp = 1;
			goDown = 0;
		}
		if (key == GLFW_KEY_Q) {
			goDown = 1;
			goUp = 0;
		}
		if (key == GLFW_KEY_C) {
			showCoordinate = -showCoordinate + 1;
		}
		if (key == GLFW_KEY_T) {
			glUseProgram(Shader_Geometry);
			displayLightOnCube = (displayLightOnCube == 1 ? 0 : 1);
			GLuint temp = glGetUniformLocation(Shader_Geometry, "disableLight");
			glUniform1i(temp, displayLightOnCube);
		}
		if (key == GLFW_KEY_F) {
			disableFog = -disableFog + 1;
		}
		if (key == GLFW_KEY_I) {
			pickStyle--;
		}
		if (key == GLFW_KEY_O) {
			pickStyle++;
		}
		if (key == GLFW_KEY_J) {
			disableWater = -disableWater + 1;
		}
		if (key == GLFW_KEY_0) {
			if (!light_toggle)
			{
				pickType = 0;
				pickStyle = 0;
			}
		}
		if (key == GLFW_KEY_1) {
			if (!light_toggle)
			{
				pickType = 1;
				pickStyle = 0;
			}
		}
		if (key == GLFW_KEY_L)
		{
			light_toggle = !light_toggle;
			std::cerr << (light_toggle ? "Enter light selection mode!" : "Exiting light selection mode!") << std::endl;
		}
		if (key == GLFW_KEY_Z)
		{
			begin_domino = !begin_domino;
		}
		if (key == GLFW_KEY_X)
		{
			show_boudingbox = !show_boudingbox;
		}
		// p for pausing or resuming
		if (key == GLFW_KEY_P)
		{
			if (sound->isSourcePlaying(bgmSource))
				sound->pauseSourceSound(bgmSource);
			else
				sound->playSourceSound(bgmSource);
		}
		if (light_toggle && key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
		{
			int index = key - GLFW_KEY_0;
			lights.toggleLight(index);
			lights.updateAllShader();
			lightDisplay->update(Shader_DisplayLight);
			std::cerr << "Light " << index << " is turned " << ((lights.getLightStatus(index)==1)?"on":"off") << "!" << std::endl;
		}
		if (light_toggle && key >= GLFW_KEY_F1 && key <= GLFW_KEY_F6)
		{
			int index = key - GLFW_KEY_F1 + 10;
			lights.toggleLight(index);
			lights.updateAllShader();
			lightDisplay->update(Shader_DisplayLight);
			std::cerr << "Light " << index << " is turned " << ((lights.getLightStatus(index) == 1) ? "on" : "off") << "!" << std::endl;
		}

		// use ctrl + f1 to f12 to save to 12 different file locations
		// disabled when light mode is on
		if ( (mods & GLFW_MOD_CONTROL) != 0)
		{
			if (!light_toggle && key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12)
			{
				int file = key - GLFW_KEY_F1 + 1;
				std::stringstream str;
				str << "#IDCOUNT" << "," << idCount << std::endl;
				str << "#STARTCUBE" << std::endl;
				for (int i = 0; i < cubeList.size(); i++)
				{
					CubeInfo info = cubeList[i]->getCubeInfo();
					str << info.id << ","
						<< info.position.x << ","
						<< info.position.y << ","
						<< info.position.z << ","
						<< info.size << ","
						<< info.textureId << ","
						<< info.color.x << ","
						<< info.color.y << ","
						<< info.color.z << std::endl;
				}
				str << "#ENDCUBE" << std::endl;

				std::stringstream filename ;
				filename << OUTPUTFILELOCATION << file;
				std::ofstream out(filename.str(), std::ofstream::out);
				out << str.str();

				// For recording the light status
				str << "#STARTLIGHT" << std::endl;
				lights.writeToFile(out);
				str << "#ENDLIGHT" << std::endl;

				out.close();
				std::cout << "Successfully saved to " << filename.str() << "!" << std::endl;
			}
		}

		// use shift + f1 to f12 to load from 12 different locations
		// disabled in light mode
		if ((mods & GLFW_MOD_SHIFT) != 0)
		{
			if (!light_toggle && key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12)
			{
				int file = key - GLFW_KEY_F1 + 1;
				std::stringstream filename;
				filename << OUTPUTFILELOCATION << file;
				std::ifstream in(filename.str(), std::ofstream::in);
				if (!in.is_open())
				{
					std::cerr << "Invalid file! Please make a save file first" << std::endl;
					return;
				}

				for (int i = 0; i < cubeList.size(); i++)
				{
					delete cubeList[i];
				}
				cubeList.clear();
				std::cerr << (cubeList.size() == 0 ? "Cleared cube list!" : "Failed to clear cube list!") << std::endl;

				std::string line;
				bool readingCube = false;
				bool readingLight = false;
				Cube * cube_temp;
				while (std::getline(in, line))
				{
					std::vector<std::string> tokens = util.split(line, ',');
					if (tokens[0] == "#IDCOUNT")
					{
						int count = std::stoi(tokens[1], nullptr);
						idCount = count;
						std::cerr << "load id count: " << idCount << std::endl;
					}
					else if (tokens[0] == "#STARTCUBE")
					{
						std::cerr << "start to read cube lines..." << std::endl;
						readingCube = true;
					}
					else if (tokens[0] == "#ENDCUBE")
					{
						std::cerr << "stop to read cube lines..." << std::endl;
						readingCube = false;
					}
					else if (tokens[0] == "#STARTLIGHT")
					{
						readingLight = true;
					}
					else if (tokens[0] == "#ENDLIGHT")
					{
						readingLight = false;
					}
					else if (readingCube)
					{
						int id = std::stoi(tokens[FILE_INDEX_ID], nullptr);
						//if (id == -1) continue;
						float pos_x = std::stof(tokens[FILE_INDEX_POS_X], nullptr);
						float pos_y = std::stof(tokens[FILE_INDEX_POS_Y], nullptr);
						float pos_z = std::stof(tokens[FILE_INDEX_POS_Z], nullptr);
						float size = std::stof(tokens[FILE_INDEX_SIZE], nullptr);
						int texId = std::stoi(tokens[FILE_INDEX_TEXTUREID], nullptr);
						float color_x = std::stof(tokens[FILE_INDEX_COLOR_X], nullptr);
						float color_y = std::stof(tokens[FILE_INDEX_COLOR_Y], nullptr);
						float color_z = std::stof(tokens[FILE_INDEX_COLOR_Z], nullptr);
						if (texId != -1)
						{
							cube_temp = new Cube(id, size, texId);
							cube_temp->setPosition(pos_x, pos_y, pos_z);
							cubeList.push_back(cube_temp);
						}
						else
						{
							cube_temp = new Cube(id, size, glm::vec3(color_x, color_y, color_z));
							cube_temp->setPosition(pos_x, pos_y, pos_z);
							cubeList.push_back(cube_temp);
						}
					}
					else if (readingLight)
					{
						lights.readFromFile(in);
					}
					else
					{
						std::cerr << "Unknown file line: " << line << std::endl;
					}
				}

				std::cout << "Successfully loaded from " << filename.str() << "!" << std::endl;
				
			}
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
		{
			shiftPressed = 0;
		}
		if (key == GLFW_KEY_W) {
			goForward = 0;
		}
		if (key == GLFW_KEY_S) {
			goBackward = 0;
		}
		if (key == GLFW_KEY_A) {
			goLeft = 0;
		}
		if (key == GLFW_KEY_D) {
			goRight = 0;
		}
		if (key == GLFW_KEY_E) {
			goUp = 0;
		}
		if (key == GLFW_KEY_Q) {
			goDown = 0;
		}
	}
	if (action == GLFW_REPEAT) {
		
	}
}

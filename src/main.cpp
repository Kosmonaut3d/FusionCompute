#include "ofApp.h"
#include "ofMain.h"

#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

// OpenGL Error Callback
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar* message, const void* userParam)
{
	// Add breakpoints here for debugging the GPU
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", "** GL ERROR **", type,
		        severity, message);
	}
	// 0x8250 = pixel path perf warning when taking screenshots, 131186 = perf warning when reading atomic counter
	else if (severity != GL_DEBUG_SEVERITY_NOTIFICATION && id != 131186 && id != 131218 && id != 0x8250)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", "** NO GL ERROR **", type,
		        severity, message);
	}
}

//========================================================================
int main()
{
	ofGLFWWindowSettings settings;
	// 4.6 is the latest version in 2022
	settings.setGLVersion(4, 6);

	// Window Resolution
	settings.setSize(1280, 960);
	ofCreateWindow(settings);

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	ofSetDataPathRoot("../resources/");

	// Log the OpenGL version
	std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;

	// Log the amount of threads we can run on the GPU
	int workGroupCount[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCount[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n", workGroupCount[0], workGroupCount[1],
	       workGroupCount[2]);

	int workGroupSize[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSize[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n", workGroupSize[0], workGroupSize[1],
	       workGroupSize[2]);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
}

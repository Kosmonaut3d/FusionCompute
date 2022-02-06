#include "ofMain.h"
#include "ofApp.h"

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			 "** GL ERROR **",
			type, severity, message);
	}
	else if(severity != GL_DEBUG_SEVERITY_NOTIFICATION && id != 131218)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			"** NO GL ERROR **",
			type, severity, message);
	}
}


//========================================================================
int main() {
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4, 4); //we define the OpenGL version we want to use
	//ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context

	settings.setSize(1280, 960);
	ofCreateWindow(settings);

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}

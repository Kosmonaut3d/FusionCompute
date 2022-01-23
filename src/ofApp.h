#pragma once

#include "ofMain.h"
#include "datastructures/pointCloud.h"
#include "datastructures/sdf.h"

class ofApp : public ofBaseApp{
	enum class RenderMode
	{
		DepthImage = 0,
		PointCloud = 1,
		SDF = 2,
		Max = 3,
	};

	public:
		ofApp();

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofImage img;
		RenderMode renderMode;
		PointCloud pointCloud; 
		ofEasyCam m_camera;
		SignedDistanceField sdf;
		float depthMultipy;
		float minDepthGrid;
		bool computeSDF;
		float m_buildProgress;
};

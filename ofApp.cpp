#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	draw_p = false;
	speed = 0;

	//put in all the pumpkins
	cat.load("pumpkin1.png");
	images.push_back(cat);
	cat.load("pumpkin2.png");
	images.push_back(cat);
	cat.load("pumpkin3.png");
	images.push_back(cat);
	cat.load("pumpkin4.png");
	images.push_back(cat);
	cat.load("pumpkin5.png");
	images.push_back(cat);

	cat.load("broken1.png");
	pumpkins.push_back(cat);
	cat.load("broken2.png");
	pumpkins.push_back(cat);
	cat.load("broken3.png");
	pumpkins.push_back(cat);

	speed = 0.05;

	camWidth = 640;
	camHeight = 480;

	p.set(camWidth / 2, camHeight / 2, 0);

	vidGrabber.setDeviceID(0);
	vidGrabber.initGrabber(camWidth, camHeight);

	//finder.setup("haarcascade_frontalface_default.xml");

	rgb.allocate(camWidth, camHeight);
	hsb.allocate(camWidth, camHeight);
	hue.allocate(camWidth, camHeight);
	sat.allocate(camWidth, camHeight);
	bri.allocate(camWidth, camHeight);
	filter1.allocate(camWidth, camHeight);
	filter2.allocate(camWidth, camHeight);
	finalImageL.allocate(camWidth, camHeight);
	finalImageR.allocate(camWidth, camHeight);
}

//--------------------------------------------------------------
void ofApp::update(){
	vidGrabber.update();
	if (vidGrabber.isFrameNew())
	{
		//Find left hand blob
		rgb.setFromPixels(vidGrabber.getPixels());
		hsb = rgb;
		hsb.convertRgbToHsv();
		hsb.convertToGrayscalePlanarImages(hue, sat, bri);

		int hueRange = 10;
		for (int i = 0; i < camWidth * camHeight; ++i)
		{
			filter1.getPixels()[i] = ofInRange(hue.getPixels()[i], findHueL - hueRange, findHueL + hueRange) ? 255 : 0;
		}
		filter1.flagImageChanged();

		int satRange = 10;
		for (int i = 0; i < camWidth *camHeight; ++i)
		{
			filter2.getPixels()[i] = ofInRange(sat.getPixels()[i], findSatL - satRange, findSatL + satRange) ? 255 : 0;
		}
		filter2.flagImageChanged();

		cvAnd(filter1.getCvImage(), filter2.getCvImage(), finalImageL.getCvImage());
		finalImageL.flagImageChanged();

		//Find right hand blob
		hueRange = 20;
		for (int i = 0; i < camWidth * camHeight; ++i)
		{
			filter1.getPixels()[i] = ofInRange(hue.getPixels()[i], findHueR - hueRange, findHueR + hueRange) ? 255 : 0;
		}
		filter1.flagImageChanged();
		satRange = 20;
		for (int i = 0; i < camWidth *camHeight; ++i)
		{
			filter2.getPixels()[i] = ofInRange(sat.getPixels()[i], findSatR - satRange, findSatR + satRange) ? 255 : 0;
		}
		filter2.flagImageChanged();

		cvAnd(filter1.getCvImage(), filter2.getCvImage(), finalImageR.getCvImage());
		finalImageR.flagImageChanged();

		contoursL.findContours(finalImageL, 50, (camWidth*camHeight) / 3, 1, false);
		contoursR.findContours(finalImageR, 50, (camWidth*camHeight) / 3, 1, false);
		
		//finder.findHaarObjects(vidGrabber.getPixels());

	}

	//calculate 'upper' point for lazer
	upper.set(righty.x + ((righty.x - lefty.x) * 2), righty.y + ((righty.y - lefty.y) * 2), 0);

	//find formula of points within lazer
	slope = (upper.y - righty.y) / (upper.x - righty.x);
	b = lefty.y - (slope*lefty.x);

	if (draw_p)
	{
		//check if pumpkin is touching 'lazer' and should be destroyed
		for (vector<pair<ofVec3f, int>>::iterator it = past.begin(); it != past.end(); ++it)
		{
			y = (it->first.x*slope) + b;
			cout << y << ' ' << it->first.y << endl;
			if (it->first.x > MIN(upper.x, righty.x) && it->first.x < MAX(upper.x, righty.x))
			{
				//tolerance for pumpkins close to line
				//if (it->first.x < (ofGetMouseX()+10) && it->first.x > (ofGetMouseX() - 10) && it->first.y > (ofGetMouseY() - 10) && it->first.y < (ofGetMouseY() + 10))
				if ((y - 40) < it->first.y && it->first.y < (y + 40))
				{
					temp.set(it->first.x, it->first.y);
					//store pumpkins to be broken
					broken.push_back(temp);
					it = past.erase(it);
					if (it == past.end())
					{
						break;
					}
				}
			}

		}
	}



	/*  //ball repulsion (scrapped)
	upper.set(righty.x + ((righty.x - lefty.x) * 2), righty.y + ((righty.y - lefty.y) * 2), 0);

	slope = (upper.y - righty.y) / (upper.x - righty.x);
	b = lefty.y - (slope*lefty.x);

	s = MIN(lefty.x, upper.x);

	for (int i = 0; i < abs(upper.x - righty.x); ++i)
	{
		x = s + i;
		y = (slope*x) + b;
		//cout << y << endl;
		ofVec2f c(x, y); // Point of repulsion
		ofVec2f v = p - c; // Vector between c and p
		float d = v.length(); // Distance between c and p
		float f = 10000 / d / d; // Intensity of the repulsion (quick search on the web, found http://www.physicsclassroom.com/class/estatics/Lesson-3/Coulomb-s-Law)
		v = v.getNormalized() * f; // Force vector

		v.x = ofClamp(v.x, -speed, speed);
		v.y = ofClamp(v.y, -speed, speed);

		temp = p + v;
		if (0 < temp.x < camWidth)
		{
			v.x = -v.x;
		}
		if (0 < temp.y < camHeight)
		{
			v.y = -v.y;
		}
		p += v; // Move the point with this vector

				// Just to bound the movement


	}

	//mouse can guide ball back
	ofVec2f c(ofGetMouseX(), ofGetMouseY()); // Point of repulsion
	ofVec2f v = p - c; // Vector between c and p
	float d = v.length(); // Distance between c and p
	float f = 10000 / d / d; // Intensity of the repulsion (quick search on the web, found http://www.physicsclassroom.com/class/estatics/Lesson-3/Coulomb-s-Law)
	v = v.getNormalized() * f; // Force vector
	p += v; // Move the point with this vector

			// Just to bound the movement
	p.x = ofClamp(p.x, 0, camWidth);
	p.y = ofClamp(p.y, 0, camHeight);


	past.erase(past.begin());
	past.push_back(p);*/
}


//--------------------------------------------------------------
void ofApp::draw(){
	//draw a not cat
	//ofTranslate(camWidth, camHeight);
	//cat.setAnchorPoint(cat.getWidth() / 2, 0);
	//ofRotate(40);
	//ofEnableAlphaBlending();
	//cat.draw(0, 0);
	//ofDisableAlphaBlending();
	//ofTranslate(0, 0);

	//pick random pumpkin placement
	if (draw_p)
	{
		if (speed == 5)
		{
			p.x = ofRandom(camWidth);
			p.y = ofRandom(camHeight);
			//store pumpkin placement
			past.push_back(make_pair(p, ofRandom(5)));
			speed = 0;
		}
		else
		{
			speed += 1;
		}

		
	}
	


	ofSetColor(ofColor::white);
	vidGrabber.draw(0, 0, camWidth, camHeight);

	font.load("franklinGothic.otf", 15);
	ofSetColor(ofColor::blueSteel);
	font.drawString("Destroy these pumpkins!\nUse the provided stick and hit the pumpkins with the red line to make them disappear.\nPress 'c' to clear the screen.\nKeep the bows directed at and in view of the camera!", 25, camHeight + 35);

	ofSetLineWidth(0.1f);
	
	contoursL.draw(0, 0);
	contoursR.draw(0, 0);

	ofSetColor(ofColor::lawnGreen);
	//ofNoFill();
	for (int i = 0; i < contoursL.blobs.size(); ++i)
	{
		//ofDrawRectangle(finder.blobs[i].boundingRect);
		//finder.blobs[i].centroid();
		lefty = contoursL.blobs.at(i).centroid;
		ofDrawCircle(lefty, 5);
	}

	for (int i = 0; i < contoursR.blobs.size(); ++i)
	{
		righty = contoursR.blobs.at(i).centroid;
		ofDrawCircle(righty, 5);
	}

	//draw lazer!
	ofSetColor(ofColor::red);
	ofSetLineWidth(10);
	ofDrawLine(righty, upper);

/*  //drawball (scrapped)
	ofPushMatrix();
	for (int i = 0; i < 5; ++i)
	{
		ofEnableAlphaBlending();
		//ofSetColor(ofColor::plum, (51 * i));
		//ofDrawCircle(past[i].x, past[i].y, 10);
		ofSetColor(255, 255, 255, (51 * i));
		cat.draw(past[i].x, past[i].y, 50, 50);
		ofDisableAlphaBlending();
	}
	ofPopMatrix();*/

	//draw all existing pumpkins
	for (int i = 0; i < past.size(); ++i)
	{
		ofSetColor(255, 255, 255);
		images[past[i].second].draw(past[i].first.x, past[i].first.y, 86, 51);
	}

	//draw destroyed pumpkins dying
	for (vector<ofVec2f>::iterator it = broken.begin(); it != broken.end(); ++it)
	{
		if (it->y > camHeight)
		{
			it = broken.erase(it);
			if (it == broken.end())
			{
				break;
			}
		}
		else
		{
			ofSetColor(255, 255, 255);
			it->y += 20;
			pumpkins[ofRandom(3)].draw(it->x, it->y, 86, 51);
		}

	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	int mx = ofGetMouseX() % camWidth;
	int my = ofGetMouseY() % camHeight;
	//set lefty color
	if (key == 'l')
	{
		findHueL = hue.getPixels()[my*camWidth + mx];
		findSatL = sat.getPixels()[my*camWidth + mx];
	}
	//set righty color
	if (key == 'r')
	{
		findHueR = hue.getPixels()[my*camWidth + mx];
		findSatR = sat.getPixels()[my*camWidth + mx];
	}
	/*
	if (key == 's')
	{
		speed -= 0.01;
	}
	if (key == 'f')
	{
		speed += 0.01;
	}

	if (key == 'p')
	{
		cat.load(images[ofRandom(4)]);
	}*/
	//draw pumpkins toggle on/off
	if (key == 'd')
	{
		draw_p = !draw_p;
	}
	//clear screen
	if (key == 'c')
	{
		past.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "RMSContext.h"
#include "RMSImage.h"
#include <stdexcept>
#include <iostream>
#include "opencv/cv.hpp"
#include "FaceTracker.h"
#include "dlib/opencv/cv_image.h"
#include "RMSMakeTech.h"
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"


using namespace std;


int main()
{
	//int width = 639;
	//int height = 480;

	int width = 1024;
	int height = 768;

	cv::VideoCapture capture;
	if (!capture.open(0))
		throw std::runtime_error("capture init failed");
	
	cv::Mat mat;
	capture.read(mat);

	RMSCVImage image;
	image.Load(mat, width, height);

	std::string path = "./";
	FaceTracker ft(path);

	auto result = glfwInit();
	if (result != GLFW_TRUE)
		throw std::runtime_error("glfwInit failed");

	//glfwDefaultWindowHints();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	auto window = glfwCreateWindow(width, height, "test", nullptr, nullptr);
	if (window == nullptr) 
		throw std::runtime_error("glfwCreateWindow failed");

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	result = glewInit();
	if (result != GLEW_OK)
		throw std::runtime_error("glewInit failed");

	RMSContext *context = RMSContext::sharedContext();

	//auto img = cv::imread("test.jpg");

	//int width = img.cols;
	//int height = img.rows;

	//uchar* camData = new uchar[img.total() * 4];
	//cv::Mat continuousRGBA(img.size(), CV_8UC4, camData);
	//cv::cvtColor(img, continuousRGBA, CV_BGR2RGBA, 4);

	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	//glEnable(GL_TEXTURE_2D);
	//glViewport(0, 0, width, height);
	//glLoadIdentity();
	//gluLookAt((GLfloat)0, (GLfloat)0, (GLfloat)5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	//GLuint texture;
	//glGenTextures(1, &texture);
	// Set texture clamping method
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0, width, height,
		GL_RGB, GL_UNSIGNED_BYTE, img->imageData);
	*/
	
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	//RMSRenderer renderer(false);

	//context->setFilter("extreme.xml");
	//context->setMakeTech("test.xml", kRMSLayerFound);
	context->setMakeTech("test.xml", kRMSLayerCheek);
	context->setMakeTech("test.xml", kRMSLayerEye);
	context->setMakeTech("test.xml", kRMSLayerLip);
	//context->setMakeTech("test.xml", kRMSLayerLipGloss);
	//auto makeTech = context->prepareMakeTech("37801_MJECJ_BL381neD.xml", kRMSLayerEye);
	//context->setPreparedMakeTech(makeTech, kRMSLayerEye);
	/*
	auto tech1 = context->prepareMakeTech(".", 0);
	auto tech2 = context->prepareMakeTech(".", 1);
	auto tech3 = context->prepareMakeTech(".", 2);
	*/
	
	//context->setDrawFlags(kRMSDrawMake|kRMSDrawFilter);
	context->setDrawFlags(kRMSDrawMake);


	float tmp_parts[RMSFeatures::NativeCount*2];
	int indexi = 0;
	double lastdraw = 0;
	
	auto vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
	auto font = nvgCreateFont(vg, "sans", "Roboto-Regular.ttf");
	int fbWidth, fbHeight;

	//auto fb = nvgluCreateFramebuffer(vg, width, height, 0);

	bool drawOriginal = true;
	bool drawPoints = true;

	while (!glfwWindowShouldClose(window)) {

		//glClearStencil(0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//glViewport(0, 0, fbWidth, fbHeight);

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//renderer.UpdateFrame(image);
		//renderer.Render(0);
		//ft.trackWithPixels(image.Pixels(), image.Width(), image.Height(), image.Stride());

		glfwGetWindowSize(window, &width, &height);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

		image.setVWidth(width);
		image.setVHeight(height);

		ft.trackWithCV(image.cvImage());
		CGRect rect;
		auto hasFace = ft.fetchFaceRect(&rect, 0);
		ft.fetchFaceParts(context->features(), 0);
		auto parts = context->features();
		memcpy(tmp_parts, parts, sizeof(tmp_parts));

		for (int i = 0; i < RMSFeatures::NativeCount; i++) {
			parts[i*2] = tmp_parts[i*2] / image.Width() * width;
			parts[i*2 + 1] = tmp_parts[i*2 + 1] / image.Height() * height;
		}

		context->updateFeatures(hasFace);
		context->updateFrame(image);

		drawOriginal = glfwGetKey(window, GLFW_KEY_SPACE);

		context->render(drawOriginal || !hasFace);
		
		drawPoints = glfwGetKey(window, GLFW_KEY_TAB);
	
		if (drawPoints) {
			nvgBeginFrame(vg, width, height, (float)fbWidth / (float)width);
			int pcount = RMSFeatures::Count;
			//int pcount = indexi % RMSFeatures::Count + 1;
			
			nvgFontFace(vg, "sans");
			nvgFontSize(vg, 15.0f);
			nvgFillColor(vg, nvgRGB(255, 0, 0));
			nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);

			GLfloat *v2 = new GLfloat[pcount * 2];
			for (int i = 0; i < pcount * 2; i += 2) {
				v2[i] = (float)parts[i] / width * 2.0 - 1;
				v2[i + 1] = 1.0 - (float)parts[i + 1] / height * 2.0;

				stringstream ss;;
				ss << i / 2;
				nvgText(vg, parts[i] + 15, parts[i+1] + 5, ss.str().c_str(), NULL);
			}
			nvgFill(vg);
			nvgEndFrame(vg);


			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			float x1 = (float)rect.origin.x / image.Width() * 2.0 - 1;
			float x2 = (float)(rect.origin.x + rect.size.width - 1) / image.Width() * 2.0 - 1;
			float y1 = 1.0 - (float)rect.origin.y / image.Height() * 2.0;
			float y2 = 1.0 - (float)(rect.origin.y + rect.size.height - 1) / image.Height() * 2.0;

			GLfloat vertices[] = { x1, y1,
								 x2, y1,
								 x1, y2,
								 x2, y2 };
			GLubyte indices[] = { 0, 1,
								  1, 3,
								  3, 2,
								  2, 0 };

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glLineWidth(1.0);
			//glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 0.0);
			glDrawElements(GL_LINES, sizeof(indices), GL_UNSIGNED_BYTE, indices);
			glDisableClientState(GL_VERTEX_ARRAY);

			GLubyte *indices2 = new GLubyte[pcount];
			for (int idi = 0; idi < pcount; ++idi) {
				indices2[idi] = idi;
			}
			glPointSize(5.0);
			glColor3f(0.5, 1.0, 0.5);
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, v2);
			glDrawElements(GL_POINTS, pcount, GL_UNSIGNED_BYTE, indices2);
			glDisableClientState(GL_VERTEX_ARRAY);

			delete[] v2;
			delete[] indices2;

			glFinish();
		}

		double current = glfwGetTime();
		if (current - lastdraw > 0.5) {
			indexi++;
			lastdraw = current;
		}

		/*
		glVertex2f(x1, y1);
		glVertex2f(x1, y2);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		*/
		//glEnd();
		
		/*
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA, image.Width(), image.Height(), 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image.Pixels());

		glViewport(0, 0, (GLsizei)image.Width(), (GLsizei)image.Height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, image.Width(), image.Height(), 0);

		glEnable(GL_TEXTURE_2D);    //允许使用纹理
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_POLYGON);    //设置为多边形纹理贴图方式并开始贴图
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);    //纹理左上角对应窗口左上角
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, image.Height());    //纹理左下角对应窗口左下角
		glTexCoord2f(1.0f, 1.0f); glVertex2f(image.Width(), image.Height());    //纹理右下角对应窗口右下角
		glTexCoord2f(1.0f, 0.0f); glVertex2f(image.Width(), 0);    //纹理右上角对应窗口右上角
		glEnd();    //结束贴图
		glDisable(GL_TEXTURE_2D);
		*/

		/*
		glEnable(GL_TEXTURE_2D);    //允许使用纹理
		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_POLYGON);    //设置为多边形纹理贴图方式并开始贴图
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);    //纹理左上角对应窗口左上角
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, image.Height());    //纹理左下角对应窗口左下角
		glTexCoord2f(1.0f, 1.0f); glVertex2f(image.Width(), image.Height());    //纹理右下角对应窗口右下角
		glTexCoord2f(1.0f, 0.0f); glVertex2f(image.Width(), 0);    //纹理右上角对应窗口右上角
		glEnd();    //结束贴图
		glDisable(GL_TEXTURE_2D);
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();

		capture.read(mat);
		image.Load(mat, width, height);
	}

	image.Dispose();
	delete context;

	glfwTerminate();

	return 0;
}
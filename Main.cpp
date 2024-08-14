#include <stdio.h>
#include <windows.h>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include "glsl.h"
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include <GLFW/glfw3.h>
#include "glut.h"
#include "ILMBase.h"
#include "Basic.h"
#include "Colors.h"
#include "Mesh.h"
#include "ceres/ceres.h"
#include "Camera.h"

using namespace std;

char g_filename[260] = { NULL };
char g_foldername[260] = { NULL };

//io state from ImGui
bool g_WantCaptureKeyboard = false;
bool g_WantCaptureMouse = false;
bool g_WantCaptureTextInput = false;
//global stuff
bool g_minimize_menu = false;  //minimize imGui menu drawing?
Vec4i g_viewport(0, 0, 800, 600);  //(x,y,w,h)
GLFWwindow* g_window = NULL;
int g_window_w = 1900;
int g_window_h = 1060;
Camera* g_camera = NULL;
bool g_draw_registration = true;  //draw registered points? (training and testing points)
//mouse/keyboard parameters
int g_mouse_button = -1;  //GLFW_MOUSE_BUTTON_1, GLFW_MOUSE_BUTTON_2, ...
int g_mouse_state = -1;  //GLFW_PRESS, GLFW_RELEASE
int g_mouse_x = 0;
int g_mouse_y = 0;
int g_mouse_modifier = 0;  //GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT, GLFW_MOD_SUPER, ...
int g_down_x = 0;
int g_down_y = 0;

//point clouds
vector<vector<Vec3f>> g_points;

void ResetLighting()
{	
	//default lighting coefficients
	GLfloat light_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat mat_diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_shininess[] = { 10.0f };
	GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
}

void DrawSphere(float x, float y, float z, Vec3f& color, float size)
{
	int divide = 12;

	//scale, translate then draw the sphere
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);

	float scale = size;
	glScalef(scale, scale, scale);

	glColor3f(color.x, color.y, color.z);

	//draw a solid sphere:
	GLUquadricObj* quadric;
	quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluSphere(quadric, 1, divide, divide);
	gluDeleteQuadric(quadric);

	glPopMatrix();
}

//main render function
void Display(GLFWwindow* window)
{
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);  // always re-normalize normal (due to scale transform)
	ResetLighting();
	glEnable(GL_LIGHT0);

	glClearColor(1, 1, 1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//apply camera
	g_camera->ApplyMODELView();
	g_camera->ApplyProjection();

	//draw the point clouds
	for (int i = 0; i < g_points.size(); i++)
	{
		if (!g_draw_registration && i > 1)
			continue;

		vector<Vec3f>& points = g_points[i];

		Vec3f color(0);
		if (i == 0)  //target
			color = Vec3f(1, 0, 0);
		else if (i == 1)  //source original
			color = Vec3f(0, 0, 1);
		else if (i == 2)  //registered, training
			color = Vec3f(0, 1, 0);
		else if (i == 3)  //registered, testing
			color = Vec3f(0, 1, 1);

		float size = 0.001;
		if (i == 2)  //training set
			size = 0.0018;
		else if (i == 3)  //testing set
			size = 0.0014;

		for (int j = 0; j < points.size(); j++)
		{
			DrawSphere(points[j].x, points[j].y, points[j].z, color, size);
		}

	}
}

////imgui callbacks

static void ErrorCallback(int Error, const char* Description)
{
	fprintf(stderr, "[ImGui_ErrorCallback] Error %d: %s\n", Error, Description);
}

static void FramebufferSizeCallback(GLFWwindow* window, int w, int h)
{
	g_window_w = w;
	g_window_h = h;
	g_viewport[2] = w;
	g_viewport[3] = h;
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mod)
{
	if (g_WantCaptureMouse)
		return;

	g_mouse_button = button;
	g_mouse_state = action;
	g_mouse_modifier = mod;

	//remember x/y at mouse down
	if (g_mouse_state == GLFW_PRESS)
	{
		g_down_x = g_mouse_x;
		g_down_y = g_mouse_y;		
	}	
}

static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (g_WantCaptureMouse)
		return;
	
	//camera control
	if (g_mouse_state == GLFW_PRESS)
	{
		if (g_mouse_button == GLFW_MOUSE_BUTTON_1)
		{
			if (g_mouse_modifier & GLFW_MOD_ALT)
			{
				//rolling
				g_camera->RotateRoll((float(g_mouse_x - x) / g_viewport[2]) * (float)360.0);
			}
			else
			{
				// compute the angle (0..360) around x axis, and y-axis
				g_camera->RotateHorizotal((float(x - g_mouse_x) / g_viewport[2]) * (float)360.0);
				g_camera->RotateVertical((float(y - g_mouse_y) / g_viewport[3]) * (float)360.0);
			}
		}
		else if (g_mouse_button == GLFW_MOUSE_BUTTON_2)  //scaling
		{
			g_camera->Zoom((float)(g_mouse_y - y) / 100);
		}
		else if (g_mouse_button == GLFW_MOUSE_BUTTON_3)  //translation
		{
			g_camera->MoveHorizontal((float)(x - g_mouse_x) * 0.1);
			g_camera->MoveVertical((float)(g_mouse_y - y) * 0.1);
		}
	}

	g_mouse_x = (int)x;
	g_mouse_y = (int)y;
}

void KeyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	if (g_WantCaptureKeyboard)
		return;

	g_mouse_modifier = mods;

	if (action == GLFW_PRESS)
	{
		if (key == 256)  //ESC
		{
		}		
	}
}

//rigid registration's least-square term: Ra + t - b
//R is rotation matrix, t is translation. {a, b} is a pair of matched 3D points
//NOTE: x and y are constants, R and t are variables
//NOTE: each matching constitutes 3 terms (x,y,z)
struct FunctorRigid
{
	FunctorRigid(double weight, double a0, double a1, double a2, 
		double b0, double b1, double b2) : Weight(weight), A0(a0), A1(a1), A2(a2),
		B0(b0), B1(b1), B2(b2) {}

	//R is 3x3 matrix (row-major), t is 3d vector
	template <typename T> bool operator()(const T* const R, const T* const t, T* residual) const
	{
		//residuals for the x,y,z components:		
		/*residual[0] = T(Weight) * (A0 + t[0] - B0);
		residual[1] = T(Weight) * (A1 + t[1] - B1);
		residual[2] = T(Weight) * (A2 + t[2] - B2);*/

		residual[0] = T(Weight) * (R[0] * A0 + R[1] * A1 + R[2] * A2 + t[0] - B0);
		residual[1] = T(Weight) * (R[3] * A0 + R[4] * A1 + R[5] * A2 + t[1] - B1);
		residual[2] = T(Weight) * (R[6] * A0 + R[7] * A1 + R[8] * A2 + t[2] - B2);
		return true;
	}

private:
	double Weight;
	double A0, A1, A2, B0, B1, B2;  //a and b vectors
};

//3x3 rotation matrix constraints:
//1.orthogonality condition: RTR = I
//2.det(R) = +1
struct FunctorOrthogonality
{
	FunctorOrthogonality(double weight) : Weight(weight) {}

	template <typename T> bool operator()(const T* const R, T* residual) const
	{
		//9 residuals for each element of a 3x3 (indentify) matrix in row major:
		//residual[0] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[1] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[2] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[3] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[4] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[5] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[6] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[7] = T(Weight) * (T(0)/*TODO: implement this*/);
		//residual[8] = T(Weight) * (T(0)/*TODO: implement this*/);

		//9 residuals for each element of a 3x3 (indentify) matrix in row major:
		residual[0] = T(Weight) * (R[0] * R[0] + R[3] * R[3] + R[6] * R[6] -T(1));
		residual[1] = T(Weight) * (R[0] * R[1] + R[3] * R[4] + R[6] * R[7]);
		residual[2] = T(Weight) * (R[0] * R[2] + R[3] * R[5] + R[6] * R[8]);
		residual[3] = T(Weight) * (R[0] * R[1] + R[3] * R[4] + R[6] * R[7]);
		residual[4] = T(Weight) * (R[1] * R[1] + R[4] * R[4] + R[7] * R[7] -T(1));
		residual[5] = T(Weight) * (R[1] * R[2] + R[4] * R[5] + R[7] * R[8]);
		residual[6] = T(Weight) * (R[0] * R[2] + R[3] * R[5] + R[6] * R[8]);
		residual[7] = T(Weight) * (R[1] * R[2] + R[4] * R[5] + R[7] * R[8]);
		residual[8] = T(Weight) * (R[2] * R[2] + R[5] * R[5] + R[8] * R[8] -T(1));

		

		return true;
	}

private:
	double Weight;
};
struct FunctorDet
{
	FunctorDet(double weight) : Weight(weight) {}

	template <typename T> bool operator()(const T* const R, T* residual) const
	{
		//residual[0] = T(Weight) * (T(0)/*TODO: implement this*/);
		//1 residuals for det(R) = +1
		residual[0] = T(Weight) * (R[0] * R[4] * R[8] + R[1] * R[5] * R[6] + R[2] * R[3] * R[7] - R[2] * R[4] * R[6] - R[0] * R[5] * R[7] - R[1] * R[3] * R[8] - T(1));
		return true;
	}

private:
	double Weight;
};


//Project3: rigid registration optimization by Ceres Solver
//we want to register points1 to points0 by a rotation and then a translation
//return the solved 3x3 rotation matrix R and translation t
bool RigidRegistration(vector<Vec3f> &points0, vector<Vec3f> &points1, Matrix33f &R, Vec3f &t,
	bool constrain_rotation_matrix)
{
	if (points0.size() != points1.size())
	{
		cout << "two point clouds shall have the same size" << endl;
		return false;
	}

	//variables: 9 elements of a 3d rotation matrix R (row-major)
	//and 3 elements of a 3d translation t

	double* vars = new double[12];
	//initialize R to identity and t to zero. row-major
	vars[0] = 1;
	vars[1] = 0;
	vars[2] = 0;
	vars[3] = 0;
	vars[4] = 1;
	vars[5] = 0;
	vars[6] = 0;
	vars[7] = 0;
	vars[8] = 1;
	//t:
	vars[9] = 0;
	vars[10] = 0;
	vars[11] = 0;

	ceres::Problem problem;

	//create the rigid registration data terms (one matching one term)
	ceres::CostFunction** functor_rigids = new ceres::CostFunction * [points0.size()];
	for (int i = 0; i < points0.size(); i++)
	{
		functor_rigids[i] = new ceres::AutoDiffCostFunction<FunctorRigid, 3/*residual size*/,
			9/*R matrix*/, 3/*t vector*/>(new FunctorRigid(1/*weight*/, points0[i].x, points0[i].y, points0[i].z, points1[i].x, points1[i].y, points1[i].z));
	}

	//create the orthogonality and determinant terms:
	ceres::CostFunction* functor_ortho = new ceres::AutoDiffCostFunction<FunctorOrthogonality, 9/*residual size*/,
		9/*R matrix*/>(new FunctorOrthogonality(1000/*weight*/));
	ceres::CostFunction* functor_det = new ceres::AutoDiffCostFunction<FunctorDet, 1/*residual size*/,
		9/*R matrix*/>(new FunctorDet(1000/*weight*/));

	////AddResidualBlocks:

	//rigid registration data terms:
	for (int i = 0; i < points0.size(); i++)
	{
		problem.AddResidualBlock(functor_rigids[i], NULL, &vars[0]/*R*/, &vars[9]/*t*/);
	}

	//orthogonality and determinant terms:
	if (constrain_rotation_matrix)
	{
		problem.AddResidualBlock(functor_ortho, NULL, &vars[0]/*R*/);
		problem.AddResidualBlock(functor_det, NULL, &vars[0]/*R*/);
	}

	// Run the solver!
	ceres::Solver::Options options;
	options.max_num_iterations = 1000;
	options.minimizer_progress_to_stdout = true;
	ceres::Solver::Summary summary;
	ceres::Solve(options, &problem, &summary);
	cout << summary.BriefReport() << endl;

	//get the results
	R[0][0] = vars[0];
	R[0][1] = vars[1];
	R[0][2] = vars[2];
	R[1][0] = vars[3];
	R[1][1] = vars[4];
	R[1][2] = vars[5];
	R[2][0] = vars[6];
	R[2][1] = vars[7];
	R[2][2] = vars[8];
	t = Vec3f(vars[9], vars[10], vars[11]);

	return true;
}

int main()
{
	//first, get module folder location	
	if (GetCurrentDirectoryA(sizeof(g_foldername), g_foldername) > 0)
	{
		//add "/" to the end
		strcpy(g_foldername, (string(g_foldername) + "/").c_str());
	}	

	//imgui pipeline
	{
		// Setup window
		if (!glfwInit())
			return 1;

		//windowed full-screen mode
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		g_window_w = mode->width - 15;  //note: leave a bit padding
		g_window_h = mode->height - 15;

		g_window = glfwCreateWindow(g_window_w, g_window_h, "Epipolar", NULL, NULL);
		glfwMakeContextCurrent(g_window);

		// Setup ImGui binding
		ImGui_ImplGlfw_Init(g_window, false/*don't install callbacks for me*/);

		//set font scale (size)				
		ImGui::GetIO().FontGlobalScale = (float)g_window_w / (float)2560;

		//setup own callbacks
		glfwSetErrorCallback(ErrorCallback);
		glfwSetFramebufferSizeCallback(g_window, FramebufferSizeCallback);
		glfwSetMouseButtonCallback(g_window, MouseButtonCallback);
		glfwSetCursorPosCallback(g_window, CursorPosCallback);
		glfwSetKeyCallback(g_window, KeyCallback);
		glfwSetCharCallback(g_window, ImGui_ImplGlfw_CharCallback);

		ImGui::CaptureMouseFromApp(false);

		//use global window size as viewport
		g_viewport[0] = 0;
		g_viewport[1] = 0;
		glfwGetWindowSize(g_window, &g_viewport[2], &g_viewport[3]);

		//(offline) create point clouds and same them to files
		if(false)
		{
			Mesh mesh;
			mesh.LoadObj("mesh/bunny.obj");

			//sample parts of its vertices for the first point cloud
			vector<Vec3f> points0;
			vector<Vertex*> vertices = mesh.GetVertices();
			for (int i = 0; i < vertices.size(); i++)
			{
				//sample only a subset?
				//if (i % 2 != 0)
				//	continue;

				points0.push_back(vertices[i]->pos);
			}

			//the second point cloud is the first one after a rotation and then a translation
			//and a bit perturb
			Vec3f rot_axis = Vec3f(-3, 2, 1).normalize();
			float rot_angle = R2D(37);
			Vec3f t(-0.08, 0.027, 0.064);

			vector<Vec3f> points1;
			for (int i = 0; i < points0.size(); i++)
			{
				Vec3f P = rotate(points0[i], rot_axis, rot_angle);
				P += t;

				Vec3f perturb((float)(rand() % 1000) / 1000.0f, (float)(rand() % 1000) / 1000.0f, (float)(rand() % 1000) / 1000.0f);
				perturb *= 0.001;
				P += perturb;

				points1.push_back(P);
			}

			//save to files!
			ofstream outfile0("points/points0.txt");
			for (int i = 0; i < points0.size(); i++)
			{
				outfile0 << points0[i].x << "," << points0[i].y << "," << points0[i].z << endl;
			}
			outfile0.close();
			ofstream outfile1("points/points1.txt");
			for (int i = 0; i < points1.size(); i++)
			{
				outfile1 << points1[i].x << "," << points1[i].y << "," << points1[i].z << endl;
			}
			outfile1.close();
		}

		//load point clouds from files:
		for (int Case = 0; Case < 2; Case++)
		{
			string filename;
			if (Case == 0)
				filename = "points/points0.txt";
			else
				filename = "points/points1.txt";
			
			ifstream infile(filename);
			if (infile.fail())
			{
				cout << "[load_obj] Error opening file " << filename;
				return false;
			}

			vector<Vec3f> points;
			string current_line;
			while (getline(infile, current_line))
			{
				Vec3f p;
				sscanf(current_line.c_str(), "%f,%f,%f", &p.x, &p.y, &p.z);

				points.push_back(p);
			}
			infile.close();

			g_points.push_back(points);
		}

		////project3: rigid registration
		if(true)
		{
			//separate the point clouds into training and testing sets
			//note: we want to register point cloud#1 (source) to point cloude#0 (target)
			vector<Vec3f> points_train_source;
			vector<Vec3f> points_train_target;
			vector<Vec3f> points_test_source;
			vector<Vec3f> points_test_target;
			float training_ratio = 0.05;
			for (int i = 0; i < g_points[0].size(); i++)
			{
				//random sampling:
				bool is_training = ((float)(rand() % 1000) / 1000.0f)/*0~1 random var*/ <= training_ratio;

				//or arbitrary-order sampling?
				if (i <= g_points[0].size() * training_ratio)
					is_training = true;
				else
					is_training = false;

				if (is_training)
				{
					points_train_source.push_back(g_points[1][i]);
					points_train_target.push_back(g_points[0][i]);
				}
				else
				{
					points_test_source.push_back(g_points[1][i]);
					points_test_target.push_back(g_points[0][i]);
				}
			}
			cout << "training:" << points_train_source.size() << " testing:" << points_test_source.size() << endl;

			//solve R and t on the training set:
			Matrix33f R;
			Vec3f t;
			//bool constrain_rotation_matrix = true;  //constrain the R to be a rotation matrix?
			bool constrain_rotation_matrix = false;  //constrain the R to be a rotation matrix?
			RigidRegistration(points_train_source, points_train_target, R, t, constrain_rotation_matrix);
			cout << "constrain_rotation_matrix:" << constrain_rotation_matrix << endl;

			//now test the avg L2 error on the training and testing tests
			//also put the registered training and testing points into the g_points for visualization
			float training_err = 0;
			{
				vector<Vec3f> points_reg;  //registered points
				for (int i = 0; i < points_train_source.size(); i++)
				{
					//calculate the rigid transformed new position
					Vec3f p = points_train_source[i];
					Vec3f P;
					P.x = R[0][0] * p.x + R[0][1] * p.y + R[0][2] * p.z + t[0];
					P.y = R[1][0] * p.x + R[1][1] * p.y + R[1][2] * p.z + t[1];
					P.z = R[2][0] * p.x + R[2][1] * p.y + R[2][2] * p.z + t[2];

					training_err += (points_train_target[i] - P).length();
					points_reg.push_back(P);
				}
				training_err /= points_train_source.size();
				cout << "training avg. L2 error:" << training_err << endl;

				g_points.push_back(points_reg);
			}
			float test_err = 0;
			{
				vector<Vec3f> points_reg;  //registered points
				for (int i = 0; i < points_test_source.size(); i++)
				{
					//calculate the rigid transformed new position
					Vec3f p = points_test_source[i];
					Vec3f P;
					P.x = R[0][0] * p.x + R[0][1] * p.y + R[0][2] * p.z + t[0];
					P.y = R[1][0] * p.x + R[1][1] * p.y + R[1][2] * p.z + t[1];
					P.z = R[2][0] * p.x + R[2][1] * p.y + R[2][2] * p.z + t[2];

					test_err += (points_test_target[i] - P).length();
					points_reg.push_back(P);
				}
				test_err /= points_test_source.size();
				cout << "testing avg. L2 error:" << test_err << endl;
				cout << "R:" << R << endl;
				cout << "t:" << t << endl;

				g_points.push_back(points_reg);
			}
			cout << "train-to-test gap:" << test_err - training_err << endl;
		}

		//setup camera:
		{
			//fit the center and bounding box of all point clouds
			Vec3f center(0);
			int count = 0;
			Vec3f BB_min(FLT_MAX), BB_max(-FLT_MAX);
			for (int i = 0; i < g_points.size(); i++)
			{
				vector<Vec3f>& points = g_points[i];
				for (int i = 0; i < points.size(); i++)
				{
					Vec3f p = points[i];
					if (p.x < BB_min.x)
						BB_min.x = p.x;
					if (p.y < BB_min.y)
						BB_min.y = p.y;
					if (p.z < BB_min.z)
						BB_min.z = p.z;
					if (p.x > BB_max.x)
						BB_max.x = p.x;
					if (p.y > BB_max.y)
						BB_max.y = p.y;
					if (p.z > BB_max.z)
						BB_max.z = p.z;

					center += p;
				}
				count += points.size();
			}
			center /= count;
			Vec3f volume = BB_max - BB_min;

			g_camera = new Camera();
			g_camera->m_default_distance = 2.0f * MAX3(volume.x, volume.y, volume.z);
			g_camera->m_center = center;
			g_camera->m_nearPlane = 0.01;
			g_camera->m_farPlane = 5.0f * volume.length();
			g_camera->m_scale = 1;
			g_camera->m_zenith = 0;
			g_camera->m_azimuth = 0;
			g_camera->m_roll = 0;
			g_camera->m_fieldofview = 45.0;
			g_camera->m_translation = Vec3f(0);
		}
		
		//dimension spec for windows
		const int W = g_window_w * 0.2;		
		const int WItem = (W / 2) * 0.4;  //width for "item" (such as input or slider)
		const int Padding = 2;
		const int MainWindowH = g_window_h * 0.08;
		const int MainWindowX = Padding;
		const int MainWindowY = Padding;
		
		// Main loop
		bool FirstLoop = true;
		while (!glfwWindowShouldClose(g_window))
		{
			glfwPollEvents();
			ImGui_ImplGlfw_NewFrame();

			if (g_minimize_menu)
			{
				//minimized menus
				ImGui::SetNextWindowPos(ImVec2(MainWindowX, MainWindowY));
				ImGui::SetNextWindowSize(ImVec2(W, 70));
				ImGui::Begin("Main");
				{
					ImGui::Checkbox("HideUI", &g_minimize_menu);
				}
				ImGui::End();
			}
			else
			{
				// main dialog
				ImGui::SetNextWindowPos(ImVec2(MainWindowX, MainWindowY));
				ImGui::SetNextWindowSize(ImVec2(W, MainWindowH));
				ImGui::Begin("Main");
				{
					ImGui::Checkbox("DrawRegistration", &g_draw_registration);					
							
				}
				ImGui::End();
			}

			//these flags would become true if ImGui UIs want to hijack inputs
			g_WantCaptureKeyboard = ImGui::GetIO().WantCaptureKeyboard;
			g_WantCaptureMouse = ImGui::GetIO().WantCaptureMouse;
			g_WantCaptureTextInput = ImGui::GetIO().WantTextInput;

			// main render function
			Display(g_window);

			//render ImGui UIs
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			ImGui::Render();

			glfwSwapBuffers(g_window);

			FirstLoop = false;
		}

		// Cleanup
		ImGui_ImplGlfw_Shutdown();
		glfwTerminate();
	}

    return 0;
}
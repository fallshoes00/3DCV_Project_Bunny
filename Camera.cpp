#include <iostream>
#include <fstream>
#include "glut.h"
#include "Basic.h"
#include "Camera.h"

using namespace std;

extern Vec4i g_viewport;

void Camera::ApplyMODELView()
{
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	//NOTE: OpenGL specify matrix in the left-to-right order, so the first specified matrix is applied last

	//VIEW
	gluLookAt(0, 0, m_default_distance, 0, 0, 0, 0, 1, 0);

	//MODEL
	//step3: translation
	glTranslatef(m_translation.x, m_translation.y, m_translation.z);
	//step2: scale
	glScalef(m_scale, m_scale, m_scale);
	//step1: rotate
	glRotatef(m_zenith, 1.0f, 0.0f, 0.0f);
	glRotatef(m_azimuth, 0.0f, 1.0f, 0.0f);
	glRotatef(m_roll, 0.0f, 0.0f, 1.0f);	
	//step0: re-center everything (usually in the object center)
	glTranslatef(-m_center.x, -m_center.y, -m_center.z);
}

void Camera::ApplyProjection()
{
	// setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	

	//perspective projection?
	if (true)
	{
		GLfloat aspect = (float)g_viewport[2] / (float)g_viewport[3];
		gluPerspective(m_fieldofview/aspect, aspect, m_nearPlane, m_farPlane);
	}

	//orthogonal projection?
	else
	{
		glOrtho(-1,1,-1,1, m_nearPlane, m_farPlane);
	}
}

void Camera::ApplyPickProjection(double pick_x, double pick_y, double w, double h)
{
	// setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//do pick matrix
	GLint viewport[4] = { g_viewport[0], g_viewport[1], g_viewport[2], g_viewport[3] };
	gluPickMatrix( pick_x, pick_y, w, h, viewport );	
	
	//perspective projection?
	if(true)
	{
		GLfloat aspect = (float)g_viewport[2]/(float)g_viewport[3];
		gluPerspective(m_fieldofview/aspect, aspect, m_nearPlane, m_farPlane);		
	}
	//orthogonal projection?
	else
	{
		glOrtho(-1,1,-1,1, m_nearPlane, m_farPlane);
	}
}

void Camera::RotateHorizotal(float degree)
{
	m_azimuth += degree;
}

void Camera::RotateVertical(float degree)
{
	m_zenith += degree;
}

void Camera::RotateRoll(float degree)
{
	m_roll += degree;	
}

void Camera::Zoom(float scale)
{
	m_scale += scale;

	if(m_scale < 0.01)
		m_scale = (float)0.01;
	
	if(m_scale > 100)
		m_scale = 100;
}

void Camera::MoveHorizontal(float amount)
{
	m_translation.x += ( amount * (m_default_distance * 0.01) );	
}

void Camera::MoveVertical(float amount)
{
	m_translation.y += ( amount * (m_default_distance * 0.01) );
}

bool Camera::Save( const char* filename )
{
	ofstream outfile(filename);
	if(outfile.fail())
	{
		cout<<"Error opening camera output file " << filename;
		return false;
	}

	//zenith
	outfile << "z " << m_zenith << endl;	
	//azimuth
	outfile << "a " << m_azimuth << endl;	
	//scale
	outfile << "s " << m_scale << endl;
	//roll
	outfile << "r " << m_roll << endl;
	//translate
	outfile << "t " << m_translation.x << " " << m_translation.y << " " << m_translation.z << endl;	
	//camera_default_distance
	outfile << "d " << m_default_distance << endl;
	//center
	outfile << "c " << m_center.x << " " << m_center.y << " " << m_center.z << endl;
	//projection parameters
	outfile << "p " << m_fieldofview << " " << m_nearPlane << " " << m_farPlane << endl;

	outfile.close();
	return true;
}

bool Camera::Load( const char* filename)
{
	Reset();

	ifstream infile(filename);
	if(infile.fail())
	{
		cout<<"Error opening camera file" << filename;
		return false;
	}

	char current_line[1024];
	while (!infile.eof())
	{
		infile.getline(current_line, 1024);

		switch (current_line[0])
		{
		case 'z':  //zenith
			sscanf(current_line, "z %f", &m_zenith);
			break;

		case 'a':  //azimuth
			sscanf(current_line, "a %f", &m_azimuth);
			break;

		case 's':  //scale
			sscanf(current_line, "s %f", &m_scale);
			break;

		case 'r':  //roll
			sscanf(current_line, "r %f", &m_roll);
			break;

		case 't':  //translate
			sscanf(current_line, "t %f %f %f", &m_translation.x, &m_translation.y, &m_translation.z);
			break;

		case 'd':  //m_camera_default_distance
			sscanf(current_line, "d %f", &m_default_distance);			
			break;			

		case 'c':  //center
			sscanf(current_line, "c %f %f %f", &m_center.x, &m_center.y, &m_center.z);
			break;

		case 'p':  //projection parameters
			sscanf(current_line, "p %f %f %f", &m_fieldofview, &m_nearPlane, &m_farPlane);
			break;

		break;
		}
	}

	infile.close();
	return true;
}
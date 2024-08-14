#pragma once

#include "ILMBase.h"

//each camera manages: 1.viewing (in MODELVIEW matrix) 2.projection (in PROJECTION matrix) and 3. viewport
class Camera
{
public:
	//view parameters
	//the camera moves on a sphere centered at the model
	float m_default_distance;
	Vec3f m_center;
	float m_scale;
	float m_zenith;  //rotation along (1,0,0), in degree
	float m_azimuth;  //rotation along (0,1,0)
	float m_roll;  //rotation along (0,0,1)
	Vec3f m_translation;

	//projection parameters
	float m_fieldofview;
	float m_nearPlane;
	float m_farPlane;	
	
	Camera()
	{
		Reset();
	}

	void Reset()
	{
		m_default_distance = 0;
		m_center = Vec3f(0, 0, 0);
		m_scale = 0;
		m_zenith = 0;
		m_azimuth = 0;
		m_roll = 0;
		m_translation = Vec3f(0, 0, 0);
		m_fieldofview = 0;
		m_nearPlane = 0;
		m_farPlane = 0;
	}

	//apply viewing transformation in MODELVIEW matrix
	void ApplyMODELView();

	//apply projection (perspective) transformation 	
	void ApplyProjection();

	//apply picking projection	
	//pick_x,pick_y: picking location
	//window_size: size of the picking window
	void ApplyPickProjection(double pick_x, double pick_y, double w, double h);

	//camera movement
	void RotateHorizotal(float degree);
	void RotateVertical(float degree);
	void RotateRoll(float degree);
	void Zoom(float scale);
	void MoveHorizontal(float amount);
	void MoveVertical(float amount);
	
	//save current camera parameters to a file
	//filename: the file name
	//return : success
	bool Save(const char* filename);
	//load camera parameters from a file
	//filename: the file name
	//return : success
	bool Load(const char* filename); 
};
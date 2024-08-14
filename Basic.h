#pragma once

#include <windows.h>
#include <vector>
#include <iostream>
#include "ILMBase.h"

#define MIN2(a,b) (((a) < (b))?(a):(b))
#define MAX2(a,b) (((a) > (b))?(a):(b))
#define MAX3(a,b,c) ( ( (MAX2(a,b)) > (c) ) ? (MAX2(a,b)) : (c) )
#define MIN3(a,b,c) ( ( (MIN2(a,b)) > (c) ) ? (MIN2(a,b)) : (c) )

#define MYPI 3.14159265359

#define MYE 2.71828182845

//Degree / Radian conversions
#define D2R(a) ((a)/180.0*MYPI)
#define R2D(a) ((a)/MYPI*180.0)

#define IS_NAN(x) ((x) != (x))

using namespace std;

//create a dialog for open file
bool OpenFileName(char* pszFilename, wchar_t* pszFilter = NULL, wchar_t* title = NULL);

//split a string
//str: input string
//separators: separating tokens
//stlv_string: split result
void split_string(const string &str,const string &separators, vector<string> &stlv_string);

//find two orthogonal vectors (out1&out2) to one given vector (int)
//return false when it is not possible (when in is (0,0,0))
bool FindOrthogonalVectors(Vec3f &in, Vec3f &out1, Vec3f &out2);
//2d version (much simpler)
bool FindOrthogonalVectors(Vec2f &in, Vec2f &out1, Vec2f &out2);

//produce rotation matrix R by a normal and angle phi
void rotation_matrix(Vec3f n, float phi, Matrix33f &R);
void rotation_matrix(Vec3f n, float phi, Matrix33d &R);

//rotate a 3D vector by a normal and angle phi
Vec3f rotate(Vec3f vector, Vec3f normal, float phi);
#include <windows.h>
#include "Shlwapi.h"
#include "Basic.h"

// Function to open file using dialog.
bool OpenFileName(char* pszFilename, wchar_t* pszFilter, wchar_t* title)
{
	OPENFILENAME	ofn;	// The common open filename dialog by Win32
	wchar_t			szFilename_wchar[MAX_PATH];
	size_t			iConvertedChars;

	mbstowcs_s(&iConvertedChars, szFilename_wchar, sizeof(pszFilename)+1, pszFilename, _TRUNCATE);
	// Initialization
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = (LPWSTR)szFilename_wchar;
	ofn.nMaxFile = 256;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;
	if(pszFilter)
	{
		ofn.lpstrFilter = (LPCWSTR)pszFilter;
	}
	else
	{
		ofn.lpstrFilter = (LPCWSTR)TEXT("All\0*.*\0");
	}

	if(title == NULL)
		ofn.lpstrTitle = (LPCWSTR)TEXT("Open File");
	else
		ofn.lpstrTitle = title;
	
	// Display the Open dialog box. 
	if(GetOpenFileName(&ofn)==TRUE)
	{
		wcstombs_s(&iConvertedChars, pszFilename, wcslen(szFilename_wchar)+1, szFilename_wchar, _TRUNCATE);
		return true;
	}
	else
	{
		DWORD err = CommDlgExtendedError();
		printf("[OpenFileName] error: 0x%x(%d)", err, err);
		return false;
	}
}

//a utility function to split a string into a vector of strings, with specified separators
void split_string(const string& str,const string& separators, vector<string> &stlv_string) 
{
	stlv_string.clear();

	string part_string("");
	string::size_type i;
	
	i=0;
	while(i < str.size()) 
	{
		if(separators.find(str[i]) != string::npos) 
		{
			stlv_string.push_back(part_string);
			part_string="";
			while(separators.find(str[i]) != string::npos) 
			{
				++i;
			}
		}
		else 
		{
			part_string += str[i];
			++i;
		}
	}
	if (!part_string.empty())
	{
		stlv_string.push_back(part_string);
	}
}

bool FindOrthogonalVectors(Vec3f &in, Vec3f &out1, Vec3f &out2)
{
	//find one non-zero component of in:
	int nz1 = -1;
	for(int i=0; i<3; i++ )
	{
		if( in[i] != 0 )
		{
			nz1 = i;
			break;
		}
	}
	if(nz1 == -1 )					
		return false;  //in is (0,0,0) !!! don't bother	

	//find another non-zero component:
	int nz2 = -1;
	int i = nz1;
	while(true)
	{
		i = (i+1)%3;

		if(i == nz1 )
			break;  //again

		if(in[i] != 0 )
		{
			nz2 = i;
			break;
		}
	}

	out1 = Vec3f(0,0,0);
	//case1: if both nz1 & nz2 are found, out1 can be formed as: out1[nz1] = -in[nz2] ; out1[nz2] = -in[nz1] ; another component is zero
	if(nz2 != -1)
	{
		out1[ nz1 ] = -in[ nz2 ];
		out1[ nz2 ] = -in[ nz1 ];
		
		if(out1 == in || out1 == -in)
			out1[nz1] += 0.012345f;  //avoid out1 become linear to in
	}
	//case2: otherwise is even simpler: out1[nz1] = 0 and other 2 components are set to 1
	else
	{
		out1[ (nz1+1)%3 ] = 1;
		out1[ (nz1+2)%3 ] = 1;
	}

	//out2 is in cross out1
	out2 = in.cross( out1 );
	
	//out1 may be ill-oriented so need to re-oriented it: out1 is out2 cross in
	out1 = out2.cross( in );
	
	return true;
}

bool FindOrthogonalVectors(Vec2f &in, Vec2f &out1, Vec2f &out2)
{
	if( in == Vec2f(0,0))
		return false;

	//out1 is in's x, y swapped and negative
	out1.x = -in.y;
	out1.y = in.x;
	//out2 is just -1 * out1
	out2 = -out1;
	return true;
}


void rotation_matrix(Vec3f n, float phi, Matrix33f &R)
{
	R.makeIdentity();

	float cosphi = cos(phi);
	float sinphi = sin(phi);

	R[0][0] = cosphi + n.x*n.x*(1-cosphi);
	R[0][1] = n.x*n.y*(1-cosphi)-n.z*sinphi;
	R[0][2] = n.x*n.z*(1-cosphi)+n.y*sinphi;
	R[1][0] = n.y*n.x*(1-cosphi)+n.z*sinphi;
	R[1][1] = cosphi + n.y*n.y*(1-cosphi);
	R[1][2] = n.y*n.z*(1-cosphi)-n.x*sinphi;
	R[2][0] = n.z*n.x*(1-cosphi)-n.y*sinphi;
	R[2][1] = n.z*n.y*(1-cosphi)+n.x*sinphi;
	R[2][2] = cosphi + n.z*n.z*(1-cosphi);
}
void rotation_matrix(Vec3f n, float phi, Matrix33d &R)
{
	R.makeIdentity();

	double cosphi = cos(phi);
	double sinphi = sin(phi);

	R[0][0] = cosphi + n.x*n.x*(1 - cosphi);
	R[0][1] = n.x*n.y*(1 - cosphi) - n.z*sinphi;
	R[0][2] = n.x*n.z*(1 - cosphi) + n.y*sinphi;
	R[1][0] = n.y*n.x*(1 - cosphi) + n.z*sinphi;
	R[1][1] = cosphi + n.y*n.y*(1 - cosphi);
	R[1][2] = n.y*n.z*(1 - cosphi) - n.x*sinphi;
	R[2][0] = n.z*n.x*(1 - cosphi) - n.y*sinphi;
	R[2][1] = n.z*n.y*(1 - cosphi) + n.x*sinphi;
	R[2][2] = cosphi + n.z*n.z*(1 - cosphi);
}

Vec3f rotate(Vec3f vector, Vec3f normal, float phi)
{
	Matrix33f R;
	rotation_matrix(normal, phi, R);
	return vector * R.transpose();
}
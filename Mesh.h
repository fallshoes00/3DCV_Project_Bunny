#pragma once

#include <vector>
#include <map>
#include "ILMBase.h"

using namespace std;

////half-edge data stucture:

class Vertex;
class Halfedge;
class Face;

class Vertex
{
public:
	int serial;
	bool valid;
	Vec3f pos;  //the 3d position
	Vec2f uv;  //per-vertex uv?
	Halfedge *e;  //the pointing-to halfedge

	Vertex()
	{
		Reset();
	}
	~Vertex()
	{
	}

	void Reset()
	{
		serial = 0;
		valid = true;
		pos = Vec3f(0);
		uv = Vec2f(0);
		e = NULL;
	}

	int Degree();  //return the valence (# of surrounding edges)

	//return the surrounding inward pointing edges (in CW order)
	vector<Halfedge*> Edges();	
	vector<Halfedge*> EdgesCCW();  //or in CCW order

	//return the surrounding faces (in CW order)
	vector<Face*> Faces();
	vector<Face*> FacesCCW();  //or in CCW order

	bool Border();  //touching border edges?

	Vec3f Normal();  //return vertex normal, e.g., avg of neighbor face normals
};

class Halfedge
{
public:
	int serial;
	bool valid;
	Halfedge *o;  //the opposite halfedge
	Halfedge *next;  //next halfedge in CCW order.	
	Halfedge *prev;  //previous halfedge in CCW order.
	Vertex *v;  //pointing to vertex
	Face *f;  //the facing face (NULL for border edge)

	Halfedge()
	{
		Reset();
	}

	void Reset()
	{
		serial = 0;
		valid = true;
		o = NULL;
		next = NULL;
		prev = NULL;
		v = NULL;
		f = NULL;
	}

	bool Border()
	{
		return f == NULL;
	}
	bool Border2()
	{
		return (f == NULL) || (o->f == NULL);
	}

	float Length()
	{
		return (v->pos - o->v->pos).length();
	}
	float LengthUV()
	{
		return (v->uv - o->v->uv).length();
	}

	Vec3f Dir()
	{
		return (v->pos - o->v->pos).normalize();
	}
	Vec2f Dir_UV()
	{
		return (v->uv - o->v->uv).normalize();
	}

};

class Face
{
public:
	int serial;
	bool valid;
	Halfedge *e;  //a circulating halfedge
	Vec3f normal;  //the avg. normal	

	//raw data from obj:
	vector<int> texcoordIndices;  //"ti"
	vector<int> normalIndices;  //"ni"

	Face()
	{
		Reset();
	}

	void Reset()
	{
		serial = 0;
		valid = true;
		e = NULL;
		texcoordIndices.clear();
		normalIndices.clear();
	}

	int Degree();  //return the face degree (# of circulating edges)

	//return the circulating halfedges or vertices / vertex positions. CCW
	vector<Halfedge*> Edges();

	vector<Vertex*> Vertices();  //CCW
	vector<Vec3f> VertexPoss();  //vertex positions		

	void CalculateNormal();  //calculate and store avg. normal
};

struct ViTiNi
{
	vector<int> Vis;
	vector<int> Tis;
	vector<int> Nis;

	ViTiNi(vector<int> &vis, vector<int> &tis, vector<int> &nis)
	{
		Vis = vis;
		Tis = tis;
		Nis = nis;
	}

	ViTiNi() {}
};

class Mesh
{
public:

	//reserve size of vertices (has a large default value)
	int m_reserve_size;

	//half-edge data structure (ordered by serials)
	vector<Vertex> m_vertices;
	vector<Halfedge> m_edges;  //half-edges actually!
	vector<Face> m_faces;

	//raw data from obj
	vector<Vec2d> m_texcoords;	//vt
	vector<Vec3f> m_normals;  //vn

	//texture stuff:
	unsigned char* m_tex_data;
	int m_tex_width;
	int m_tex_height;
	int m_tex_channels;
	unsigned int m_tex_id;

	//bounding box info
	Vec3f m_BB_min;
	Vec3f m_BB_max;
	Vec3f m_BB_volume;

	float m_avg_edge_len;  //avg edge length. calculated at Refresh()

	//(shader programming) vertex and triangle-face arrays for array-based draw
	float* m_varray;  //vertex poss array (size = |vertices|)
	float* m_narray;  //vertex normals array (size = |vertices|)
	float* m_tarray;  //vertex texcoord array (size = |verices|)
	unsigned int* m_tri_array;
	int m_tri_array_size;

	Mesh()
	{		
		m_reserve_size = 30000;
		m_tex_data = NULL;
		m_tex_width = 0;
		m_tex_height = 0;
		m_tex_channels = 0;
		m_tex_id = 9999;  //assumed invalid value
		m_varray = NULL;
		m_narray = NULL;
		m_tarray = NULL;
		m_tri_array = NULL;

		Reset();

	}
	Mesh(float ReserveRatio)
	{
		Reset();
	}
	~Mesh()
	{
		Reset();
	}

	void Reset()
	{
		m_vertices.clear();
		m_edges.clear();
		m_faces.clear();
		m_texcoords.clear();
		m_normals.clear();
		m_avg_edge_len = 0;
		
		if (m_varray)
		{
			delete m_varray;
			m_varray = NULL;
		}
		if (m_narray)
		{
			delete m_narray;
			m_narray = NULL;
		}
		if (m_tarray)
		{
			delete m_tarray;
			m_tarray = NULL;
		}
		if (m_tri_array)
		{
			delete m_tri_array;
			m_tri_array = NULL;
		}
		m_tri_array_size = 0;
	}

	//get certain element by serial
	Vertex* GetVertex(int Serial);
	Halfedge* GetHalfedge(int Serial);
	Face* GetFace(int Serial);

	//get a half-edge by v-serial / vo-serial
	Halfedge* GetHalfedge(int v_serial, int vo_serial);

	//add elements
	Vertex* AddVertex();
	Halfedge* AddHalfedge();
	Face* AddFace();

	//remove (just make invalid) certain element by serial
	void RemoveVertex(int serial);
	void RemoveEdge(int serial);  //actually remove a E/EO pair
	void RemoveFace(int serial);

	//directly retrieve valid elements
	vector<Vertex*> GetVertices();
	vector<Halfedge*> GetHalfedges();
	vector<Halfedge*> GetFulledges();  //get one edge per e/eo
	vector<Face*> GetFaces();

	//directly retrieve positions of all valid vertices
	vector<Vec3f> GetVertexPoss();

	//load/save from a obj file
	bool LoadObj(const char* Filename);
	bool SaveObj(const char* Filename);

	//actual implementation of LoadObj
	bool LoadObj(istream &infile);
	
	//build itself by vertices and faces and the optional per-vertex UV (the same order as Vertices)
	//Vertices: the positions of vertices
	//Faces: the vertex indieces (vi) of the faces
	//TexCoords (optional): tex coordinates data
	//Normals (optional): normals data
	//FaceTis (optional): the tex coordinate indices of the faces
	//FaceNis (optional): the normalindices of the faces
	bool Build(vector<Vec3f> &Vertices, vector<vector<int>> &Faces, vector<Vec2d> *TexCoords = NULL, vector<Vec3f> *Normals = NULL,
		vector<vector<int>> *FaceTis = NULL, vector<vector<int>> *FaceNis = NULL);	
	
	//calculate the bounding box of this mesh
	void CalculateBoundingBox();

	//whenever the mesh data structure is updated, do refresh
	void Refresh();

	//setup the "arrays" for rendering
	void CreateArrays();	
	
	//array-based draw functions
	void DrawArray();	

	void DrawWireframe(float size, Vec3f color);  //draw the mesh wireframes	

private:

	//actual function to build a mesh
	//Vertices: the positions of vertices
	//ViTiNis: the Vi/Ti/Ni indices of faces
	//TexCoords (optional): tex coordinates data
	//Normals (optional): normals data
	bool Build(vector<Vec3f> &Vertices, vector<ViTiNi> &ViTiNis, vector<Vec2d> *Texcoords = NULL, vector<Vec3f> *Normals = NULL);	
};
#include <iostream>
#include <fstream>
#include <sstream>      // std::istringstream
#include <algorithm>
#include <queue>
#include <list>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include "glut.h"
#include <time.h>
#include "Mesh.h"
#include "Colors.h"
#include "Basic.h"

//for loading textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float ReserveRatio = 3.0f;  //the ratio for reserve memory for element vectors (vertices, edges, m_Faces)

void TriangularNormal(Halfedge *e, Vec3f &normal)
{
	Vec3f e1 = e->v->pos - e->o->v->pos;
	Vec3f e2 = e->next->v->pos - e->v->pos;
	//grasp e1 and e2 in CCW order gives normal
	normal = (e1.cross(e2)).normalize();
}

int Vertex::Degree()
{
	if (e == NULL)
		return 0;  //???

	int valence = 0;
	Halfedge *cur = e;
	for (;;)
	{
		valence++;

		cur = cur->next->o;
		if (cur == e)
			break;
	}

	return valence;
}

vector<Halfedge*> Vertex::Edges()
{
	vector<Halfedge*> es;
	if (!e)
		return es;

	es.resize(10);
	Halfedge *cur = e;
	int index = 0;
	for (;;)
	{
		//es.push_back(cur);
		if (index >= es.size())
			es.resize(es.size() + 5);

		es[index] = cur;

		index++;
		cur = cur->next->o;
		if (cur == e)
			break;
	}	
	es.resize(index);

	return es;
}

vector<Halfedge*> Vertex::EdgesCCW()
{
	vector<Halfedge*> es = Edges();
	vector<Halfedge*> es2;
	for (int i = es.size() - 1; i >= 0; i--)
		es2.push_back(es[i]);
	return es2;
}

vector<Face*> Vertex::Faces()
{
	vector<Face*> Fs;

	vector<Halfedge*> Es = Edges();
	for (unsigned int i = 0; i < Es.size(); i++)
	{
		if (Es[i]->f)
			Fs.push_back(Es[i]->f);
	}

	return Fs;
}

vector<Face*> Vertex::FacesCCW()
{
	vector<Face*> fs = Faces();
	vector<Face*> fs2;
	for (int i = fs.size() - 1; i >= 0; i--)
		fs2.push_back(fs[i]);
	return fs2;	
}

bool Vertex::Border()
{
	if (!e)
		return false;  //valence-0!

	Halfedge *cur = e;
	for (;;)
	{
		if (cur->Border() || cur->o->Border())
			return true;

		cur = cur->next->o;
		if (cur == e)
			break;
	}

	return false;
}

Vec3f Vertex::Normal()
{
	Vec3f normal = Vec3f(0);
	vector<Halfedge*> Es = Edges();
	for (int i = 0; i < Es.size(); i++)
	{
		if (Es[i]->f)
			normal += Es[i]->f->normal;
	}
	normal.normalize();
	return normal;
}

int Face::Degree()
{
	vector<Halfedge*> edges;
	if (e == NULL)
		return 0;  //???

	int deg = 0;
	Halfedge *cur = e;
	for (;;)
	{
		deg++;
		cur = cur->next;
		if (cur == e)
			break;
	}

	return deg++;
}

vector<Halfedge*> Face::Edges()
{
	vector<Halfedge*> es;

	Halfedge *cur = e;
	for (;;)
	{
		es.push_back(cur);

		cur = cur->next;
		if (cur == e)
			break;
	}

	return es;
}

vector<Vertex*> Face::Vertices()
{
	vector<Vertex*> vs;

	Halfedge *cur = e;
	for (;;)
	{
		vs.push_back(cur->v);

		cur = cur->next;
		if (cur == e)
			break;
	}

	return vs;
}

vector<Vec3f> Face::VertexPoss()
{
	vector<Vec3f> poss;

	Halfedge *cur = e;
	for (;;)
	{
		poss.push_back(cur->v->pos);

		cur = cur->next;
		if (cur == e)
			break;
	}

	return poss;
}

void Face::CalculateNormal()
{	
	//we need to store each normal because we check the alignments	
	vector<Halfedge*> es = Edges();	
	vector< Vec3f > normals(es.size());

	size_t normal_half_size = normals.size() / 2;

	Vec3f *normal_ptr, *normal_ptr2;

	Halfedge **es_ptr;

	normal_ptr = &normals[0];
	es_ptr = &es[0];
	for (unsigned int i = 0; i<es.size(); i++, normal_ptr++, es_ptr++)
	{
		Vec3f n(0);
		TriangularNormal(*es_ptr, (*normal_ptr));		
	}
 
	normal = Vec3f(0);  //as the avg. normal

	normal_ptr = &normals[0];
	for (unsigned int i = 0; i<normals.size(); i++, normal_ptr++)
	{
		//find if this normal is more aligned or less aligned to all other normals
		unsigned int aligned = 0;
		normal_ptr2 = &normals[0];
		for (unsigned int j = 0; j<normals.size(); j++, normal_ptr2++)
		{
			if (j != i && (*normal_ptr).dot(*normal_ptr2) > 0)
			{
				aligned++;
			}

			if (aligned >= normal_half_size)
				break;
		}

		if (aligned >= normal_half_size)  //more aligned case		
			normal += (*normal_ptr);
		else  //less aligned case: revert it (making it more aligned to all other normals)		
			normal += (*normal_ptr)*-1;
	}
	normal.normalize();	
}

Vertex* Mesh::GetVertex(int serial)
{
	if (serial < m_vertices.size())
		return &m_vertices[serial];
	else
		return NULL;
}

Halfedge* Mesh::GetHalfedge(int serial)
{
	if (serial < m_edges.size())
		return &m_edges[serial];
	else
		return NULL;
}

Face* Mesh::GetFace(int serial)
{
	if (serial < m_faces.size())
		return &m_faces[serial];
	else
		return NULL;
}

Halfedge * Mesh::GetHalfedge(int v_serial, int vo_serial)
{
	vector<Halfedge*> es = GetHalfedges();
	for (int i = 0; i < es.size(); i++)
	{
		if (es[i]->v->serial == v_serial && es[i]->o->v->serial == vo_serial)
		{
			return es[i];
		}
	}

	return NULL;
}

Vertex* Mesh::AddVertex()
{
	if (m_vertices.capacity() < m_vertices.size() + 1)
	{
		//error! capacity is not enough...
		//TODO: rebuild the mesh?
		cout << "[AddVertex] Error: m_vertices capcacity not enough!" << endl;
		return NULL;
	}

	Vertex V;
	V.serial = m_vertices.size();
	m_vertices.push_back(V);
	return &m_vertices.back();
}

Halfedge* Mesh::AddHalfedge()
{
	if (m_edges.capacity() < m_edges.size() + 1)
	{
		//error! capacity is not enough...
		//TODO: rebuild the mesh?
		cout << "[AddHalfedgesPair] Error: m_selected_edges capcacity not enough!" << endl;
		return NULL;
	}

	Halfedge E;
	E.serial = m_edges.size();
	m_edges.push_back(E);
	return &m_edges.back();
}

Face* Mesh::AddFace()
{
	if (m_faces.capacity() < m_faces.size() + 1)
	{
		//error! capacity is not enough...
		//TODO: rebuild the mesh?
		cout << "[AddFace] Error: m_Faces capcacity not enough!" << endl;
		return NULL;
	}

	Face F;
	F.serial = m_faces.size();
	m_faces.push_back(F);
	return &m_faces.back();
}

void Mesh::RemoveVertex(int serial)
{
	if (m_vertices.size()>serial)
		m_vertices[serial].valid = false;
}

void Mesh::RemoveEdge(int serial)
{
	//delete edge and its opposite!
	if (m_edges.size()>serial)
	{
		int o_serial = GetHalfedge(serial)->o->serial;
		m_edges[serial].valid = false;
		m_edges[o_serial].valid = false;
	}
}

void Mesh::RemoveFace(int serial)
{
	if (m_faces.size()>serial)
		m_faces[serial].valid = false;
}

vector<Vertex*> Mesh::GetVertices()
{
	vector<Vertex*> Vertices;
	Vertices.reserve(m_vertices.size());
	for (int i = 0; i < m_vertices.size(); i++)
	{
		if (m_vertices[i].valid)
			Vertices.push_back(&m_vertices[i]);
	}
	return Vertices;
}

vector<Vec3f> Mesh::GetVertexPoss()
{
	vector<Vec3f> Poss;
	Poss.reserve(m_vertices.size());
	for (int i = 0; i < m_vertices.size(); i++)
	{
		if (m_vertices[i].valid)
			Poss.push_back(m_vertices[i].pos);
	}
	return Poss;
}

vector<Halfedge*> Mesh::GetHalfedges()
{
	vector<Halfedge*> Halfedges;
	Halfedges.reserve(m_edges.size());
	for (int i = 0; i < m_edges.size(); i++)
	{
		if (m_edges[i].valid)
			Halfedges.push_back(&m_edges[i]);
	}
	return Halfedges;
}

vector<Halfedge*> Mesh::GetFulledges()
{
	vector<Halfedge*> edges;
	edges.reserve(m_edges.size());

	map<int, bool> visited;
	for (int i = 0; i < m_edges.size(); i++)
	{
		if (m_edges[i].valid && visited.count(m_edges[i].serial) == 0)
		{
			edges.push_back(&m_edges[i]);
			visited[m_edges[i].serial] = true;
			visited[m_edges[i].o->serial] = true;
		}
	}
	return edges;
}

vector<Face*> Mesh::GetFaces()
{
	vector<Face*> Faces;
	Faces.reserve(m_faces.size());
	for (int i = 0; i < m_faces.size(); i++)
	{
		if (m_faces[i].valid)
			Faces.push_back(&m_faces[i]);
	}
	return Faces;
}

bool Mesh::SaveObj(const char* filename)
{
	//(special case) flip face's order (default is CCW)?
	const bool flip_face_order = false;

	ofstream outfile(filename);
	if (outfile.fail())
	{
		cout << "[save_obj] Error opening output file " << filename << endl;
		return false;
	}

	//header
	outfile << "#obj file." << endl;
	outfile << "#note: all indices are 1-based." << endl;
	time_t rawtime;
	time(&rawtime);
	outfile << "#time created: " << ctime(&rawtime) << endl;

	//print vertices
	map<int, int> vertex_index_map;  //first: vertex serial, second: 1-based index
	int v_index = 1;  //1-based!
	for (int i = 0; i < m_vertices.size(); i++)
	{
		Vertex &v = m_vertices[i];
		if (!v.valid)
			continue;

		vertex_index_map[v.serial] = v_index;
		v_index++;

		outfile << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << endl;
	}
	outfile << "#vertices:" << v_index << endl << endl;

	//print texcoords
	for (unsigned int i = 0; i<m_texcoords.size(); i++)
	{
		outfile << "vt " << m_texcoords[i].x << " " << m_texcoords[i].y << endl;
	}
	outfile << "#vt:" << m_texcoords.size() << endl << endl;

	//print normals
	for (unsigned int i = 0; i<m_normals.size(); i++)
	{
		outfile << "vn " << m_normals[i].x << " " << m_normals[i].y << " " << m_normals[i].z << endl;
	}
	outfile << "#vn:" << m_normals.size() << endl << endl;

	//print faces
	for (int i = 0; i < m_faces.size(); i++)
	{
		Face &f = m_faces[i];
		if (!f.valid)
			continue;

		//print the serial ids of its vertices
		outfile << "f ";
		vector<Halfedge*> es = f.Edges();

		if (!flip_face_order)
		{
			for (int i = 0; i < es.size(); i++)
			{
				//print vi
				outfile << vertex_index_map[es[i]->v->serial];

				outfile << "/";

				//print vt?
				if (i < f.texcoordIndices.size())
					outfile << f.texcoordIndices[i] + 1/*to become 1-based*/;

				outfile << "/";

				//print vn?
				if (i < f.normalIndices.size())
					outfile << f.normalIndices[i] + 1/*to become 1-based*/;

				outfile << " ";
			}
		}
		//flip face's circulating order!
		else
		{
			for (int i = es.size()-1;  i>= 0; i--)
			{
				//print vi
				outfile << vertex_index_map[es[i]->v->serial];

				outfile << "/";

				//print vt?
				if (i < f.texcoordIndices.size())
					outfile << f.texcoordIndices[i] + 1/*to become 1-based*/;

				outfile << "/";

				//print vn?
				if (i < f.normalIndices.size())
					outfile << f.normalIndices[i] + 1/*to become 1-based*/;

				outfile << " ";
			}
		}
		outfile << endl;
	}
	outfile << "#faces:" << m_faces.size() << endl;

	outfile.close();

	return true;
}

bool Mesh::LoadObj(const char* filename)
{
	ifstream infile(filename);
	if (infile.fail())
	{
		cout << "[Mesh::load_obj] Error opening file " << filename;
		return false;
	}

	bool ret = LoadObj(infile);
	infile.close();
	return ret;
}

bool Mesh::LoadObj(istream &infile)
{
	vector<Vec3f> Poss;  //(stored temporarily) the x-y-z of vertices
	Poss.reserve(m_reserve_size);
	vector< ViTiNi > ViTiNis;  //(stored temporarily) the face vis/tis/nis
	ViTiNis.reserve(20000);
	vector<Vec2d> Texcoords;	//vt
	vector<Vec3f> Normals;  //vn

	string current_line;
	while (getline(infile, current_line))
	{
		if (current_line.empty())
			continue;
		
		switch (current_line[0])
		{
		case 'v':
		{
			switch (current_line[1])
			{
			case 'n':  //create a new normal record
			{
				Vec3f n(0);
				sscanf(current_line.c_str(), "vn %f %f %f", &n.x, &n.y, &n.z);
				Normals.push_back(n);
			}
			break;

			case 't':  //create a new texcoord record
			{
				Vec2f t(0);
				sscanf(current_line.c_str(), "vt %f %f", &t.x, &t.y);
				Texcoords.push_back(t);
			}
			break;

			default:  //create a new vertex
			{
				float x = 0, y = 0, z = 0;
				sscanf(current_line.c_str(), "v %f %f %f", &x, &y, &z);
				Poss.push_back(Vec3f(x, y, z));
			}
			break;
			}
		}
		break;

		case 'f':
		{
			vector<string> face_indices;
			split_string(current_line, " \t.\r\n", face_indices);

			//erase the first token because it's 'f'
			if (face_indices.size()>0 && face_indices[0].compare("f") == 0)
				face_indices.erase(face_indices.begin() + 0);

			vector<int> vis(12);  //the 0-based vi of this face
			vector<int> tis(12);  //the 0-based ti of this face
			vector<int> nis(12);  //the 0-based ni of this face

			size_t vis_count = 0, tis_count = 0, nis_count = 0;

			//loop each "1/2/3/..." token
			string::size_type begin, end;
			for (unsigned int i = 0; i< face_indices.size(); i++)
			{
				int vi = 0, ni = 0, ti = 0;

				begin = 0;
				int j = 0;  //loop index within a "1/2/3/..." token
				do
				{
					//begin~end should be one token cut before "/" , ex: "1", "2", "3"...
					//of course end can be npos (when there is no "/")
					end = face_indices[i].find_first_of("/", begin);
					if (end == string::npos || begin<end)
					{
						//j=0: vertex index, j=1: normal index, j=2: texc index
						if (j == 0)
						{
							sscanf(face_indices[i].substr(begin, end - begin).c_str(), "%d", &vi);
							vi--;
						}
						else if (j == 1)
						{
							sscanf(face_indices[i].substr(begin, end - begin).c_str(), "%d", &ti);
							ti--;
						}
						else if (j == 2)
						{
							sscanf(face_indices[i].substr(begin, end - begin).c_str(), "%d", &ni);
							ni--;
						}
					}

					begin = end + 1;
					j++;
				} while (j <= 2 && end != string::npos);

				if (vis_count >= vis.size())
					vis.resize(vis_count * 2);
				vis[vis_count++] = vi;

				if (tis_count >= tis.size())
					tis.resize(tis_count * 2);
				tis[tis_count++] = ti;

				if (nis_count >= nis.size())
					nis.resize(nis_count * 2);
				nis[nis_count++] = ni;
			}

			vis.resize(vis_count);
			tis.resize(tis_count);
			nis.resize(nis_count);

			//if a face has repeated vertex indices, skip it
			bool have_repeated_vertices = false;
			for (unsigned int i = 0; i<vis.size(); i++)
			{
				for (unsigned int j = i + 1; j<vis.size(); j++)
				{
					if (vis[i] == vis[j])
					{
						have_repeated_vertices = true;
						break;
					}
				}
				if (have_repeated_vertices)
					break;
			}
			if (have_repeated_vertices)
			{
				cout << "[load_obj] Warning: skip face#" << m_faces.size() << " w/ repeated vertices:";
				for (unsigned int i = 0; i<vis.size(); i++)
					cout << vis[i] << ",";
				cout << endl;
				continue;
			}

			//save this vis and tis pair, we will handle them later				
			ViTiNis.push_back(ViTiNi(vis, tis, nis));
		}
		break;

		default:
			break;
		}
	}

	//(special treatment) merge vertices at close-by positions?
	if (false)
	{
		const float dist_threshold = 0.12;
		map<int, int> merge_list;  //fist: old vertex index, second: new (merged) vertex index		

		map<int, vector<Vec3f>> merged_vertices;  //first: new (merged) vertex index, second: the corresponding old vertices

												  //add new Poss one by one
		vector<Vec3f> Poss_new;
		for (int i = 0; i < Poss.size(); i++)
		{
			Vec3f pos = Poss[i];
			int new_index = -1;
			for (int j = 0; j < Poss_new.size(); j++)
			{
				if ((pos - Poss_new[j]).length() < dist_threshold)
				{
					//gotcha!
					new_index = j;
					break;
				}
			}
			if (new_index < 0)
			{
				new_index = Poss_new.size();
				Poss_new.push_back(pos);
			}

			merge_list[i] = new_index;
			merged_vertices[new_index].push_back(pos);

		}

		//update vertex poss to the avg of merged ones
		map<int, Vec3f> avg_poss;
		for (map<int, vector<Vec3f>>::iterator itr = merged_vertices.begin(); itr != merged_vertices.end(); itr++)
		{
			Vec3f avg(0);
			vector<Vec3f> &list = (*itr).second;
			for (int i = 0; i < list.size(); i++)
			{
				avg += list[i];
			}
			avg /= (float)list.size();

			avg_poss[(*itr).first] = avg;
		}

		for (int i = 0; i < Poss.size(); i++)
		{
			if (merge_list.count(i) > 0)
				Poss[i] = avg_poss[merge_list[i]];
		}
	}

	return Build(Poss, ViTiNis, &Texcoords, &Normals);
}

bool Mesh::Build(vector<Vec3f> &Vertices, vector<ViTiNi> &ViTiNis, vector<Vec2d> *Texcoords, vector<Vec3f> *Normals)
{
	Reset();

	//create the vertices!
	m_vertices.reserve(MAX2(m_reserve_size, Vertices.size() * ReserveRatio));
	m_vertices.resize(Vertices.size());
	for (unsigned int i = 0; i < Vertices.size(); i++)
	{
		m_vertices[i].serial = i;
		m_vertices[i].pos = Vertices[i];
	}

	//create the normals and texcoords!
	if (Normals)
		m_normals = *Normals;
	if (Texcoords)
		m_texcoords = *Texcoords;
	
	//actually create the faces and halfedges
	m_faces.reserve(MAX2(m_reserve_size, ViTiNis.size() * ReserveRatio));
	m_faces.resize(ViTiNis.size());

	//we need to know the total sum of sides (halfedges) of all the faces in advance...
	int NumHalfedge = 0;
	for (unsigned int i = 0; i < ViTiNis.size(); i++)
		NumHalfedge += ViTiNis[i].Vis.size();
	NumHalfedge *= 2;
	m_edges.reserve(MAX2(m_reserve_size*2, NumHalfedge * ReserveRatio));	

	unordered_map<int, vector<Halfedge*>> inward_edges_map;  //first: v's serial, second: the list of v's inward edges
	for (unsigned int i = 0; i<ViTiNis.size(); i++)
	{
		vector<int> &vis = ViTiNis[i].Vis;
		vector<int> &tis = ViTiNis[i].Tis;
		vector<int> &nis = ViTiNis[i].Nis;

		Face *F = &m_faces[i];
		F->serial = i;

		F->texcoordIndices = tis;
		F->normalIndices = nis;

		////setup half-edge DS:

		//create the circulating halfedges
		vector<Halfedge*> edges;
		for (unsigned int i = 0; i<vis.size(); i++)
		{
			Halfedge *NewE = AddHalfedge();
			NewE->f = F;
			NewE->v = &m_vertices[vis[i]];

			edges.push_back(NewE);
			inward_edges_map[NewE->v->serial].push_back(NewE);

			//set the v's e pointer if it is currently null!
			if (NewE->v->e == NULL)
				NewE->v->e = NewE;
		}

		//setup new_f's e pointer
		F->e = edges[0];

		//setup prev/next pointers:
		for (unsigned int i = 0; i<edges.size(); i++)
		{
			edges[i]->next = edges[(i + 1) % edges.size()];
			edges[i]->prev = edges[(i + edges.size() - 1) % edges.size()];
		}
	}

	//setup the o pointers:	
	for (unsigned int i = 0; i < m_edges.size(); i++)
	{
		Halfedge &E = m_edges[i];

		if (E.o)
			continue;

		//try to find the o edge according to inward_edges_map!		
		vector<Halfedge*> &vo_inward_edges = inward_edges_map[E.prev->v->serial];
		for (unsigned int i = 0; i<vo_inward_edges.size(); i++)
		{
			//see if this inward edge's prev->v is e->v:
			Halfedge* EO = vo_inward_edges[i];
			if (EO->serial == E.serial)
				continue;  //skip self!
			if (EO->prev->v->serial == E.v->serial)
			{
				//gotcha!
				E.o = EO;
				EO->o = &E;
				break;
			}
		}
	}

	//for edges w/o o, create their o edges (border ones)
	int count = 0;
	for (unsigned int i = 0; i < m_edges.size(); i++)
	{
		Halfedge &E = m_edges[i];

		if (E.o == NULL)
		{
			count++;
			Halfedge *EO = AddHalfedge();
			EO->v = E.prev->v;
			E.o = EO;
			EO->o = &E;
		}
	}

	//for border edges, setup prev/next pointers
	for (unsigned int i = 0; i < m_edges.size(); i++)
	{
		Halfedge &E = m_edges[i];

		if (E.Border())
		{
			//find prev:
			Halfedge *Cur = E.o->next->o;
			for (;;)
			{
				if (Cur->Border())
				{
					//gotcha!
					E.prev = Cur;
					break;
				}
				Cur = Cur->next->o;
			}

			//find next
			Cur = E.o->prev->o;
			for (;;)
			{
				if (Cur->Border())
				{
					//gotcha!
					E.next = Cur;
					break;
				}
				Cur = Cur->prev->o;
			}
		}
	}	

	Refresh();
	return true;
}

bool Mesh::Build(vector<Vec3f> &Vertices, vector<vector<int>> &Faces,
	vector<Vec2d> *TexCoords, vector<Vec3f> *Normals, vector<vector<int>> *FaceTis, vector<vector<int>> *FaceNis)
{
	//the goal here is to prepare ViTiNis and UVs for calling Build()

	vector<ViTiNi> ViTiNis;
	ViTiNis.resize(Faces.size());
	for (unsigned int i = 0; i < Faces.size(); i++)
	{
		ViTiNi VTN;

		VTN.Vis = Faces[i];
		if (FaceTis)
			VTN.Tis = (*FaceTis)[i];
		if (FaceNis)
			VTN.Nis = (*FaceNis)[i];

		ViTiNis[i] = VTN;
	}

	return Build(Vertices, ViTiNis, TexCoords, Normals);
}

void Mesh::CalculateBoundingBox()
{
	bool firstTime = true;
	for (unsigned int i = 0; i<m_vertices.size(); i++)
	{
		Vertex &v = m_vertices[i];
		if (!v.valid)
			continue;

		if (firstTime)
		{
			m_BB_min = v.pos;
			m_BB_max = v.pos;
			firstTime = false;
		}
		else
		{
			m_BB_min.x = min(m_BB_min.x, v.pos.x);
			m_BB_min.y = min(m_BB_min.y, v.pos.y);
			m_BB_min.z = min(m_BB_min.z, v.pos.z);
			m_BB_max.x = max(m_BB_max.x, v.pos.x);
			m_BB_max.y = max(m_BB_max.y, v.pos.y);
			m_BB_max.z = max(m_BB_max.z, v.pos.z);
		}
	}

	m_BB_volume = m_BB_max - m_BB_min;
}

void Mesh::Refresh()
{
	CalculateBoundingBox();

	//calculate average edge length
	m_avg_edge_len = 0;
	int edge_count = 0;
	vector<Halfedge*> edges = GetFulledges();
	for (int i = 0; i<edges.size(); i++)
	{
		Halfedge *e = edges[i];
		m_avg_edge_len += (e->v->pos - e->o->v->pos).length();
		edge_count++;

		
	}
	m_avg_edge_len /= (float)edge_count;
	
	//update face normals
	vector<Face*> faces = GetFaces();
	for (unsigned int i = 0; i<faces.size(); i++)
	{
		faces[i]->CalculateNormal();
	}	

	//setup arrays of vertex poss/normal/... for rendering
	CreateArrays();
}

void Mesh::CreateArrays()
{
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

	vector<Vertex*> vertices = GetVertices();
	vector<Face*> faces = GetFaces();

	//setup vertex poss/normals/texcoord array	
	map<int, GLuint> vertices_map;  //key: serial, value: 0-based index
	m_varray = new float[vertices.size() * 3];
	m_narray = new float[vertices.size() * 3];
	for (int i = 0; i < vertices.size(); i++)
	{
		Vertex* v = vertices[i];
		vertices_map[v->serial] = i;
		m_varray[i * 3] = v->pos.x;
		m_varray[i * 3 + 1] = v->pos.y;
		m_varray[i * 3 + 2] = v->pos.z;

		Vec3f n = v->Normal();
		m_narray[i * 3] = n.x;
		m_narray[i * 3 + 1] = n.y;
		m_narray[i * 3 + 2] = n.z;
	}

	//texcoords array?
	if (m_texcoords.size() > 0)
	{
		m_tarray = new float[vertices.size() * 2];

		//to calculate avg texcoords per vertex (TODO: per-face texcoords?)
		vector<vector<Vec2f>> per_vertex_texcoords(vertices.size());
		for (int i = 0; i < faces.size(); i++)
		{
			Face* f = faces[i];
			vector<Vertex*> vs = faces[i]->Vertices();
			if (f->texcoordIndices.size() == vs.size())
			{
				for (int j = 0; j < vs.size(); j++)
				{
					int ti = f->texcoordIndices[j];
					per_vertex_texcoords[vertices_map[vs[j]->serial]].push_back(m_texcoords[ti]);
				}
			}
		}
		for (int i = 0; i < per_vertex_texcoords.size(); i++)
		{
			Vec2f avg_tex_coord(0);
			if (per_vertex_texcoords[i].size() > 0)
			{
				for (int j = 0; j < per_vertex_texcoords[i].size(); j++)
				{
					avg_tex_coord += per_vertex_texcoords[i][j];
				}
				avg_tex_coord /= per_vertex_texcoords[i].size();
			}
			m_tarray[i * 2] = avg_tex_coord.x;
			m_tarray[i * 2 + 1] = 1 - avg_tex_coord.y;  //note: inverted

			//also write to vertex's uv
			vertices[i]->uv = avg_tex_coord;
		}
	}

	//setup face indices array
	vector<vector<unsigned int>> tris;  //to individual triangles of vertex indices
	for (int i = 0; i < faces.size(); i++)
	{
		vector<Vertex*> vs = faces[i]->Vertices();
		for (int offset = 0; offset <= vs.size() - 2; offset += 2)
		{
			vector<unsigned int> tri;
			tri.push_back(vertices_map[vs[offset]->serial]);
			tri.push_back(vertices_map[vs[offset + 1]->serial]);
			tri.push_back(vertices_map[vs[(offset + 2) % vs.size()]->serial]);
			tris.push_back(tri);
		}
	}
	m_tri_array = new unsigned int[tris.size() * 3];
	for (int i = 0; i < tris.size(); i++)
	{
		m_tri_array[i * 3] = tris[i][0];
		m_tri_array[i * 3 + 1] = tris[i][1];
		m_tri_array[i * 3 + 2] = tris[i][2];
	}
	m_tri_array_size = tris.size() * 3;
}

void Mesh::DrawWireframe(float size, Vec3f color)
{
	glColor3f(color.x, color.y, color.z);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth((int)size);
	glDepthMask(GL_FALSE);  //disable depth value write	

	//iterate faces and draw them as wireframe
	for (int i = 0; i<m_faces.size(); i++)
	{
		Face &f = m_faces[i];
		if (!f.valid)
			continue;

		vector<Halfedge*> Es = f.Edges();

		//front facing
		glBegin(GL_POLYGON);
		for (unsigned int i = 0; i<Es.size(); i++)
		{
			glVertex3fv(Es[i]->v->pos.getValue());
		}
		glEnd();

		//back facing
		glBegin(GL_POLYGON);
		for (int i = Es.size() - 1; i >= 0; i--)
		{
			glVertex3fv(Es[i]->v->pos.getValue());
		}
		glEnd();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
	glLineWidth(1);
}

void Mesh::DrawArray()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(3, 2);  //shall be a bit far away

	//if texture is not yet generated by GL, do it now:
	if (m_tarray)
	{
		if (m_tex_id == 9999)
		{
			//generate the texture by GL:
			glGenTextures(1, &m_tex_id);
			glBindTexture(GL_TEXTURE_2D, m_tex_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_tex_width, m_tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_tex_data);
			//glGenerateMipmap(GL_TEXTURE_2D);  //???

			//can release tex data for stb now
			stbi_image_free(m_tex_data);
		}
	}

	glEnableClientState(GL_NORMAL_ARRAY);
	if(m_tarray)
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, m_varray);
	glNormalPointer(GL_FLOAT, 0, m_narray);
	if (m_tarray)
		glTexCoordPointer(2, GL_FLOAT, 0, m_tarray);
	glDrawElements(GL_TRIANGLES, m_tri_array_size, GL_UNSIGNED_INT, m_tri_array);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (m_tarray)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_LIGHTING);
}
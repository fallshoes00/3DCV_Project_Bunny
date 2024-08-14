#pragma once

//draw elements
#define g_color_background Vec3f(1,1,1)
#define g_color_background_param Vec3f(1)
#define g_color_wireframe Vec3f(0.2,0.2,0.2)
#define g_color_mesh Vec3f(0.5,0.6,0.7)
#define g_color_border Vec3f(1,0,0)
#define g_color_sharpedge Vec3f(0.99,0.99,0.51)
//colors of valence (as triangle meshes)
#define g_color_tri_v2m Vec3f(0,0,0.7)
#define g_color_tri_v3 Vec3f(0,0,1.0)
#define g_color_tri_v4 Vec3f(0,0.4,1.0)
#define g_color_tri_v5 Vec3f(0,0.7,1.0)
#define g_color_tri_v6 Vec3f(0.2,0.2,0.2)
#define g_color_tri_v7 Vec3f(1,0.7,0)
#define g_color_tri_v8 Vec3f(1,0.4,0)
#define g_color_tri_v9 Vec3f(1,0.2,0)
#define g_color_tri_v10p Vec3f(1,0,0)
//colors of valence (as hybrid meshes)
#define g_color_h_5m Vec3f(0,0,0.6)
#define g_color_h_6 Vec3f(0,0,1.0)
#define g_color_h_7 Vec3f(0,0.2,1.0)
#define g_color_h_8 Vec3f(0,0.4,1.0)
#define g_color_h_9 Vec3f(0,0.45,1.0)
#define g_color_h_10 Vec3f(0,0.7,1.0)
#define g_color_h_11 Vec3f(0,0.9,1.0)
#define g_color_h_12 Vec3f(0.2,0.2,0.2)
#define g_color_h_13 Vec3f(1.0,0.9,0)
#define g_color_h_14 Vec3f(1.0,0.7,0)
#define g_color_h_15 Vec3f(1.0,0.45,0)
#define g_color_h_16 Vec3f(1.0,0.4,0)
#define g_color_h_17 Vec3f(1.0,0.2,0)
#define g_color_h_18 Vec3f(1.0,0,0)
#define g_color_h_19p Vec3f(0.6,0,0)
//colors of different types of hybrid mesh regular vertices
#define g_color_h_v0 Vec3f(0.3, 0.3, 0.5)
#define g_color_h_v1 Vec3f(0.3, 0.5, 0.3)
#define g_color_h_v2 Vec3f(0.3, 0.5, 0.5)
#define g_color_h_v3 Vec3f(0.5, 0.3, 0.5)
//colors of face types
#define g_color_triangle Vec3f(0.8, 0.9, 0.8)
#define g_color_quad Vec3f(0.80, 0.91, 1)
//colors of selected elements
//#define g_color_selected_face Vec3f(0.63,0.8,1)  //light blue
#define g_color_selected_face Vec3f(0.678,0.980,0.678)  //light green
#define g_color_selected_edge Vec3f(0,0.5,1)
#define g_color_selected_vertex Vec3f(1,0.25,0.25)
//colors for control graph / quadrangulation
#define g_color_CGVertex Vec3f(0)
#define g_color_CGVertex_selected Vec3f(0,0,0.6)
#define g_color_CGVertex_custom Vec3f(1,0,0)
#define g_color_CGEdge Vec3f(0)
#define g_color_CGEdge_strip Vec3f(1.0,0.1,0.1)
#define g_color_CGEdge_beginedge Vec3f(1.0, 0.7, 0.7)
#define g_color_CG_wireframe Vec3f(0.5,0.5,0.5)
#define g_color_CG_patch_background Vec4f(0.793, 0.82, 0.82)
//colors for grid view stuff
#define g_color_grid_0 Vec3f(112.0/255.0, 48.0/255.0, 160.0/255.0)
#define g_color_grid_1 Vec3f(56.0/255.0, 87.0/255.0, 35.0/255.0)
#define g_color_grid_2 Vec3f(192.0/255.0, 0/255.0, 0/255.0)
#define g_color_grid_line_type0 Vec3f(0.2, 0.2, 0.2)
#define g_color_grid_line_type1 Vec3f(91.0/255.0, 155.0/255.0, 213.0/255.0)
#define g_color_grid_line_type2 Vec3f(191.0/255.0, 144.0/255.0, 0/255.0)
#define g_color_grid_line_type3 Vec3f(255.0/255.0, 192.0/255.0, 0/255.0)
//colors for checkerboard stuff
#define g_color_tokyo_blue Vec3f(2/255.0, 34/255.0, 105/255.0)
#define g_color_stuart_green Vec3f(74/255.0, 118/255.0, 93/255.0)
#define g_color_fiverings_1 Vec3f(48.0f / 255.0f, 134.0f / 255.0f, 194.0f / 255.0f)
#define g_color_fiverings_2 Vec3f(244.0f / 255.0f, 179.0f / 255.0f, 55.0f / 255.0f)
#define g_color_fiverings_3 Vec3f(0 / 255.0f, 0 / 255.0f, 0 / 255.0f)
#define g_color_fiverings_4 Vec3f(49.0f / 255.0f, 140.0f / 255.0f, 57.0f / 255.0f)
#define g_color_fiverings_5 Vec3f(227.0f / 255.0f, 52 / 255.0f, 81.0f / 255.0f)
#define g_color_Someity Vec3f(218.0f / 255.0f, 21 / 255.0f, 133.0f / 255.0f)

//generate custom color
static Vec3f CustomColor(int id)
{
	Vec3f color(0, 0, 0);

	switch (id)
	{
	case 1:  //black
		color.x = 0 / 255.0;
		color.y = 0 / 255.0;
		color.z = 0 / 255.0;
		break;
	case 2:  //blue
		color.x = 0 / 255.0;
		color.y = 0 / 255.0;
		color.z = 255 / 255.0;
		break;
	case 3:	 //cyan
		color.x = 0 / 255.0;
		color.y = 255 / 255.0;
		color.z = 255 / 255.0;
		break;
	case 4:  //dark grey
		color.x = 51 / 255.0;
		color.y = 51 / 255.0;
		color.z = 51 / 255.0;
		break;
	case 5:  //grey
		color.x = 128 / 255.0;
		color.y = 128 / 255.0;
		color.z = 128 / 255.0;
		break;
	case 6:  //green
		color.x = 0 / 255.0;
		color.y = 255 / 255.0;
		color.z = 0 / 255.0;
		break;
	case 7:  //light grey
		color.x = 212 / 255.0;
		color.y = 212 / 255.0;
		color.z = 212 / 255.0;
		break;
	case 8:  //magenta
		color.x = 192 / 255.0;
		color.y = 64 / 255.0;
		color.z = 192 / 255.0;
		break;
	case 9:  //orange 
		color.x = 255 / 255.0;
		color.y = 192 / 255.0;
		color.z = 64 / 255.0;
		break;
	case 10:  //pink 
		color.x = 255 / 255.0;
		color.y = 0 / 255.0;
		color.z = 255 / 255.0;
		break;
	case 11:  //red
		color.x = 255 / 255.0;
		color.y = 0 / 255.0;
		color.z = 0 / 255.0;
		break;
	case 12:  //white
		color.x = 255 / 255.0;
		color.y = 255 / 255.0;
		color.z = 255 / 255.0;
		break;
	case 13:  //yellow
		color.x = 255 / 255.0;
		color.y = 255 / 255.0;
		color.z = 0 / 255.0;
		break;
	case 14:  //dark green
		color.x = 0x00 / 255.0;
		color.y = 0x53 / 255.0;
		color.z = 0x03 / 255.0;
		break;
	case 15:  //pan blue
		color.x = 0xaa / 255.0;
		color.y = 0xd5 / 255.0;
		color.z = 0xee / 255.0;
		break;
	case 16:  //light green
		color.x = 173 / 255.0;
		color.y = 250 / 255.0;
		color.z = 175 / 255.0;
		break;
	case 17:  //"fade"
		color.x = -1;
		color.y = 0;
		color.z = 0;
		break;
	case 18:  //"thin black"
		color.x = 0;
		color.y = -1;
		color.z = 0;
		break;
	case 19:  //"anchor arrow"
		color.x = 0;
		color.y = 0;
		color.z = -1;
		break;

	case 20:  //tile boundary edge color
	{
		color.x = 51 / 255.0;
		color.y = 51 / 255.0;
		color.z = 51 / 255.0;
	}
	break;

	case 21:  //urban brown
	{
		color.x = 230 / 255.0;
		color.y = 219 / 255.0;
		color.z = 189 / 255.0;
	}
	break;

	case 22:  //urban green
	{
		color.x = 203 / 255.0;
		color.y = 223 / 255.0;
		color.z = 173 / 255.0;
	}
	break;

	case 23:  //parcel
	{
		color.x = 239 / 255.0;
		color.y = 235 / 255.0;
		color.z = 226 / 255.0;
	}
	break;

	case 24:  //tron white
	{
		color.x = 236 / 255.0;
		color.y = 255 / 255.0;
		color.z = 253 / 255.0;
	}
	break;

	case 25:  //tron green
	{
		color.x = 57 / 255.0;
		color.y = 99 / 255.0;
		color.z = 113 / 255.0;
	}
	break;

	case 26:  //light red
	{
		color.x = 255 / 255.0;
		color.y = 231 / 255.0;
		color.z = 214 / 255.0;
	}
	break;

	case 27:  //tokyo blue
	{
		color = g_color_tokyo_blue;		
	}
	break;

	case 28:  //FiveRing of Olympics 1
	{
		color = g_color_fiverings_1;
	}
	break;

	case 29:  //FiveRing of Olympics 2
	{
		color = g_color_fiverings_2;
	}
	break;

	case 30:  //FiveRing of Olympics 4
	{
		color = g_color_fiverings_4;
	}
	break;

	case 31:  //FiveRing of Olympics 5
	{
		color = g_color_fiverings_5;
	}
	break;

	}
	
	return color;
}

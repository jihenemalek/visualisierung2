#include "AdaptiveSubdivision.h"
#include <assert.h>

AdaptiveSubdivision::AdaptiveSubdivision(void)
{
}
AdaptiveSubdivision::~AdaptiveSubdivision(void)
{
}



std::vector<Mesh::Triangle> AdaptiveSubdivision:: Subdivide(std::vector<Mesh::Triangle> triangles, float treshold, int anzahl)
{
	std::vector<Mesh::Triangle> temp_triangles;
	
	float kappa, a,b,c,s;
	D3DXVECTOR3 AB, BC, CA, mittelpunkta, mittelpunktb,mittelpunktc,centerlinea,centerlineb,centerlinec, temp_vec;
	float radiusa,radiusb,radiusc;
	Mesh::Triangle temp_triangle;
	Subdivided temp_subdivided;
	Tagged temp_tagged;
	int counter = 0;
	std::ofstream out;
	out.open("bla2.txt", std::ios::app);
	

	for(int i = 0; i < triangles.size(); i++)
	{
		
		
			//Krümmung berechnen
			AB = triangles.at(i).vertex1 - triangles.at(i).vertex0;
			BC = triangles.at(i).vertex2 - triangles.at(i).vertex1;
			CA = triangles.at(i).vertex0 - triangles.at(i).vertex2;
			a = D3DXVec3Length(&AB);
			b = D3DXVec3Length(&BC);
			c = D3DXVec3Length(&CA);
			s = 0.5f * (a + b + c);

			kappa = 4.0f * sqrt(fabs(s * (s - a) * (s - b) * (s - c))) / (a * b * c);
			//Wenn Krümmung größer als ein Treshold ->teile das Dreieck in 4 neue Dreiecke
			if(kappa > treshold)
			{
				
				mittelpunkta = (triangles.at(i).vertex1 + triangles.at(i).vertex0) / 2;
				mittelpunktb = (triangles.at(i).vertex2 + triangles.at(i).vertex1) / 2;
				mittelpunktc = (triangles.at(i).vertex0 + triangles.at(i).vertex2) / 2;

				centerlinea = (triangles.at(i).vertex1_mittelpunkt + triangles.at(i).vertex0_mittelpunkt) / 2;
				centerlineb = (triangles.at(i).vertex2_mittelpunkt + triangles.at(i).vertex1_mittelpunkt)/2;
				centerlinec = (triangles.at(i).vertex0_mittelpunkt + triangles.at(i).vertex2_mittelpunkt)/2;

				radiusa = (triangles.at(i).vertex1_radius + triangles.at(i).vertex0_radius)/2;
				radiusb = (triangles.at(i).vertex2_radius + triangles.at(i).vertex1_radius) / 2;
				radiusc = (triangles.at(i).vertex0_radius + triangles.at(i).vertex2_radius) / 2;

				temp_vec = mittelpunkta - centerlinea;
				D3DXVec3Normalize(&temp_vec, &temp_vec);
				mittelpunkta = centerlinea + (temp_vec * radiusa);

				temp_vec = mittelpunktb - centerlineb;
				D3DXVec3Normalize(&temp_vec,&temp_vec);
				mittelpunktb = centerlineb + (temp_vec * radiusb);

				temp_vec = mittelpunktc - centerlinec;
				D3DXVec3Normalize(&temp_vec,&temp_vec);
				mittelpunktc = centerlinec + (temp_vec * radiusc);
				

				temp_triangle.vertex0 = triangles.at(i).vertex0;
				temp_triangle.vertex1 = mittelpunkta;
				temp_triangle.vertex2 = mittelpunktc;

				temp_triangle.vertex0_mittelpunkt = triangles.at(i).vertex0_mittelpunkt;
				temp_triangle.vertex1_mittelpunkt = centerlinea;
				temp_triangle.vertex2_mittelpunkt = centerlinec;

				temp_triangle.vertex0_radius = triangles.at(i).vertex0_radius;
				temp_triangle.vertex1_radius = radiusa;
				temp_triangle.vertex2_radius = radiusc;

				temp_triangles.push_back(temp_triangle);

				temp_triangle.vertex0 = mittelpunkta;
				temp_triangle.vertex1 = triangles.at(i).vertex1;
				temp_triangle.vertex2 = mittelpunktb;

				temp_triangle.vertex0_mittelpunkt = centerlinea;
				temp_triangle.vertex1_mittelpunkt = triangles.at(i).vertex1_mittelpunkt;
				temp_triangle.vertex2_mittelpunkt = centerlineb;

				temp_triangle.vertex0_radius = radiusa;
				temp_triangle.vertex1_radius = triangles.at(i).vertex1_radius;
				temp_triangle.vertex2_radius = radiusb;

				temp_triangles.push_back(temp_triangle);

				temp_triangle.vertex0 = mittelpunktc;
				temp_triangle.vertex1 = mittelpunktb;
				temp_triangle.vertex2 = triangles.at(i).vertex2;

				temp_triangle.vertex0_mittelpunkt = centerlinec; 
				temp_triangle.vertex1_mittelpunkt = centerlineb;
				temp_triangle.vertex2_mittelpunkt = triangles.at(i).vertex2_mittelpunkt;

				temp_triangle.vertex0_radius = radiusc;
				temp_triangle.vertex1_radius = radiusb;
				temp_triangle.vertex2_radius = triangles.at(i).vertex2_radius;

				temp_triangles.push_back(temp_triangle);

				temp_triangle.vertex0 = mittelpunktc;
				temp_triangle.vertex1 = mittelpunkta;
				temp_triangle.vertex2 = mittelpunktb;

				temp_triangle.vertex0_mittelpunkt = centerlinec;
				temp_triangle.vertex1_mittelpunkt = centerlinea;
				temp_triangle.vertex2_mittelpunkt = centerlineb;

				temp_triangle.vertex0_radius = radiusc;
				temp_triangle.vertex1_radius = radiusa;
				temp_triangle.vertex2_radius = radiusb;

				temp_triangles.push_back(temp_triangle);

				temp_subdivided.triangle = &triangles.at(i);
				subdivided.push_back(temp_subdivided);
				counter = counter + 4;
			

				
			}
			//Ansonsten ist es getaggt
			else
			{ 
				temp_tagged.triangle = &triangles.at(i);
				temp_tagged.geteilt = 0;
				tagged.push_back(temp_tagged);
				//counter = counter + 1;
				
			}

			
		}

		//Für alle getaggeden Triangles wird geschaut welche nichtgetaggden anliegen ->die Teilungsvetices werden gemerk
		out<< "ok" << tagged.size();
		out.close();
		
		D3DXVECTOR3 mittelpunkt;
		float radius;

		for(int i = 0; i < tagged.size(); i++)
		{
			//Hier wird geschaut wieviele Nachbarn ein triangle hat und der Vertex in die Liste geschrieben
			for(int j = 0; j < subdivided.size(); j++)
			{
				D3DXVECTOR3 t01, t02,t12,s01,s02,s12;
				t01 = tagged.at(i).triangle->vertex1 - tagged.at(i).triangle->vertex0;
				D3DXVec3Normalize(&t01, &t01);
				t02 = tagged.at(i).triangle->vertex2 - tagged.at(i).triangle->vertex0;
				D3DXVec3Normalize(&t02, &t02);
				t12 = tagged.at(i).triangle->vertex2 - tagged.at(i).triangle->vertex1;
				D3DXVec3Normalize(&t12, &t12);

				s01 = subdivided.at(j).triangle->vertex1 - subdivided.at(j).triangle->vertex0;
				D3DXVec3Normalize(&s01, &s01);
				s02 = subdivided.at(j).triangle->vertex2 - subdivided.at(j).triangle->vertex0;
				D3DXVec3Normalize(&s02, &s02);
				s12 = subdivided.at(j).triangle->vertex2 - subdivided.at(j).triangle->vertex1;
				D3DXVec3Normalize(&s12, &s12);

				if(t01 == s01)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex0 + subdivided.at(j).triangle->vertex1)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex1_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex1_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(0);
				}
				else if(t01 == s02)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex0 + subdivided.at(j).triangle->vertex2)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					temp_vec =vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(0);
				}
				else if(t01 == s12)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex1 + subdivided.at(j).triangle->vertex2)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex1_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex1_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(0);
				}
				else if(t02 == s01)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex0 + subdivided.at(j).triangle->vertex1)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex1_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex1_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(2);
				}
				else if(t02 == s02)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex0 + subdivided.at(j).triangle->vertex2)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(2);
				}
				else if(t02 == s12)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex1 + subdivided.at(j).triangle->vertex2)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex1_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex1_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(2);
				}
				else if(t12 == s01)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex0 + subdivided.at(j).triangle->vertex1)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex1_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex1_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.mittelpunkt = vertex.vertex + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(1);
				}
				else if(t12 == s02)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex0 + subdivided.at(j).triangle->vertex2)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(1);
				}
				else if(t12 == s12)
				{
					Vertex vertex;
					vertex.vertex = (subdivided.at(j).triangle->vertex1 + subdivided.at(j).triangle->vertex2)/2;
					vertex.mittelpunkt = (subdivided.at(j).triangle->vertex1_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt)/2;
					vertex.radius = (subdivided.at(j).triangle->vertex1_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					temp_vec = vertex.vertex - vertex.mittelpunkt;
					D3DXVec3Normalize(&temp_vec, &temp_vec);
					vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(1);
				}
				/*if((tagged.at(i).triangle->vertex0 == subdivided.at(j).triangle->vertex0) && (tagged.at(i).triangle->vertex1 == subdivided.at(j).triangle->vertex1))
				{
					D3DXVECTOR3 vertex;
					vertex = (tagged.at(i).triangle->vertex0 + tagged.at(i).triangle->vertex1) / 2;
					
					mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex1_mittelpunkt) / 2;
				
					radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex1_radius) / 2;
					
					temp_vec = mittelpunkt - vertex;
					D3DXVec3Normalize(&temp_vec,&temp_vec);
					vertex = vertex + (temp_vec * radius);

					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(0);
				}
				if((tagged.at(i).triangle->vertex0 == subdivided.at(j).triangle->vertex0) && (tagged.at(i).triangle->vertex2 == subdivided.at(j).triangle->vertex2))
				{
					D3DXVECTOR3 vertex;
					vertex = (tagged.at(i).triangle->vertex0 + tagged.at(i).triangle->vertex2) / 2;

					mittelpunkt = (subdivided.at(j).triangle->vertex0_mittelpunkt + subdivided.at(j).triangle->vertex2_mittelpunkt) / 2;
					radius = (subdivided.at(j).triangle->vertex0_radius + subdivided.at(j).triangle->vertex2_radius)/2;
					
					temp_vec = mittelpunkt - vertex;
					D3DXVec3Normalize(&temp_vec,&temp_vec);
					vertex = vertex + (temp_vec * radius);

					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(2);
				}
				if((tagged.at(i).triangle->vertex2 == subdivided.at(j).triangle->vertex2) && (tagged.at(i).triangle->vertex1 == subdivided.at(j).triangle->vertex1))
				{
					D3DXVECTOR3 vertex;
					vertex = (tagged.at(i).triangle->vertex2 + tagged.at(i).triangle->vertex1) / 2;

					mittelpunkt = (subdivided.at(j).triangle->vertex2_mittelpunkt + subdivided.at(j).triangle->vertex1_mittelpunkt) / 2;
					radius = (subdivided.at(j).triangle->vertex2_radius + subdivided.at(j).triangle->vertex1_radius) / 2;
					
					temp_vec = mittelpunkt - vertex;
					D3DXVec3Normalize(&temp_vec,&temp_vec);
					vertex = vertex + (temp_vec * radius);

					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(1);
				}*/
			}

			//Hier werden die getaggted Triangles geteilt wenn die Anzahl ihrer Nachbarn nicht 0 ist
			if(tagged.at(i).number.size() != 0)
			{
				//Wenn Anzahl der Nachbarn gleich 1 ist:
				if(tagged.at(i).number.size() == 1)
				{
					tagged.at(i).geteilt = 1;
					//counter = counter + 2;
					if(tagged.at(i).number.at(0) == 2)
					{
						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).vertices.at(0).vertex;

						temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
						temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;

						temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
						temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;

						temp_triangle.vertex2_mittelpunkt= tagged.at(i).vertices.at(0).mittelpunkt;
						temp_triangle.vertex2_radius = tagged.at(i).vertices.at(0).radius;
						
						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(0).vertex;
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(0).mittelpunkt;
						temp_triangle.vertex0_radius = tagged.at(i).vertices.at(0).radius;
						temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
						temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
						temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;
						temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

						temp_triangles.push_back(temp_triangle);
					}
					if(tagged.at(i).number.at(0) == 0)
					{
					
						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).vertices.at(0).vertex;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
						temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
						temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(0).mittelpunkt;
						temp_triangle.vertex1_radius = tagged.at(i).vertices.at(0).radius;
						temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;
						temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;
					

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(0).vertex;
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(0).mittelpunkt;
						temp_triangle.vertex0_radius = tagged.at(i).vertices.at(0).radius;
						temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
						temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
						temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;
						temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;
			

						temp_triangles.push_back(temp_triangle);
					}
					if(tagged.at(i).number.at(0) == 1)
					{
						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).vertices.at(0).vertex;

						temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
						temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
						temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
						temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
						temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(0).mittelpunkt;
						temp_triangle.vertex2_radius = tagged.at(i).vertices.at(0).radius;
						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).vertices.at(0).vertex;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
						temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
						temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(0).mittelpunkt;
						temp_triangle.vertex1_radius = tagged.at(i).vertices.at(0).radius;

						temp_triangles.push_back(temp_triangle);
					
					}
				}

				else
				{
					if(tagged.at(i).number.size() == 2)
					{
						//counter = counter + 3;
						if (((tagged.at(i).number.at(0) == 0) && (tagged.at(i).number.at(1) == 1)) || ((tagged.at(i).number.at(0) == 1) && (tagged.at(i).number.at(1) == 0)))
						{
							int temp_vertex0, temp_vertex1;
							if(tagged.at(i).number.at(0) == 0)
							{
								temp_vertex0 = 0;
								temp_vertex1 = 1;
							}
							else
							{
								temp_vertex0 = 1;
								temp_vertex1 = 0;
							}
						
					
							temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1).vertex;
							temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex1).radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex1).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex1).radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

							temp_triangles.push_back(temp_triangle);
					
						}
						if(((tagged.at(i).number.at(0) == 0) && (tagged.at(i).number.at(1) == 2)) || ((tagged.at(i).number.at(0) == 2) && (tagged.at(i).number.at(1) == 0)))
						{
							int temp_vertex0, temp_vertex2;
							if(tagged.at(i).number.at(0) == 0)
							{
								temp_vertex0 = 0;
								temp_vertex2 = 1;
							}
							else
							{
								temp_vertex0 = 1;
								temp_vertex2 = 0;
							}

							temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex2).radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex2).radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex2).vertex;
							temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
							temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex2).radius;
							temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;


							temp_triangles.push_back(temp_triangle);
						}

						if(((tagged.at(i).number.at(0) == 1) && (tagged.at(i).number.at(1) == 2)) || ((tagged.at(i).number.at(0) == 1) && (tagged.at(i).number.at(1) == 2)))
						{
							int temp_vertex1, temp_vertex2;
							if(tagged.at(i).number.at(0) == 1)
							{
								temp_vertex1 = 0;
								temp_vertex2 = 1;
							}
							else
							{
								temp_vertex1 = 1;
								temp_vertex2 = 0;
							}

							temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
							temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex1).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
							temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex1).radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1).vertex;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex1).radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex2).radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex2).vertex;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1).vertex;
							temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex2).radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex1).radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

							temp_triangles.push_back(temp_triangle);
						}
					}
					else
					{
						if(tagged.at(i).number.size() == 3)
						{
							//counter = counter + 4;
							int temp_vertex0 = 0, temp_vertex1 = 0, temp_vertex2 = 0;
							for(int k = 0; k < tagged.at(i).number.size(); k++)
							{
								if(tagged.at(i).number.at(k) == 0)
								{
									temp_vertex0 = k;
								}
								else
								{
									if(tagged.at(i).number.at(k) == 1)
									{
										temp_vertex1 = k;
									}
									else
									{
										temp_vertex2 = k;
									}
								}
							}

							temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex2).radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
							temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex1).vertex;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex1).radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1).vertex;
							temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex1).radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

							temp_triangles.push_back(temp_triangle);

							temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex2).vertex;
							temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0).vertex;
							temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

							temp_triangle.vertex0_mittelpunkt = tagged.at(i).vertices.at(temp_vertex2).mittelpunkt;
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).vertices.at(temp_vertex0).mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex2).radius;
							temp_triangle.vertex1_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

							temp_triangles.push_back(temp_triangle);
						}
					}
				}
			}

		}

		//jetzt müssen noch die richtigen Triangles in die original TriangleList geschrieben werden -> und dann nochmals aufgerufen werden, wenn in dem Durchgang mindestens ein Triangle geteilt wurde
		
		//anzahl = anzahl - 1;
		//anzahl = 1;
		if(anzahl != 0)
		{
			
			
			for(int k = 0; k < tagged.size(); k++)
			{
				if(tagged.at(k).geteilt != 1)
				{
					counter = counter + 1;
				}
			}
			out << "neu" << "\n";
			out << counter << "\n";
			
	
			
			
			std::vector<Mesh::Triangle> list;
			for(int i = 0; i < tagged.size(); i++)
			{
				if(tagged.at(i).geteilt == 0)
				{
					Mesh::Triangle tri;
					tri.vertex0 = tagged.at(i).triangle->vertex0;
					tri.vertex1 = tagged.at(i).triangle->vertex1;
					tri.vertex2 = tagged.at(i).triangle->vertex2;

					tri.vertex0_mittelpunkt = tagged.at(i).triangle->vertex0_mittelpunkt;
					tri.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
					tri.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;

					tri.vertex0_radius = tagged.at(i).triangle->vertex0_radius;
					tri.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
					tri.vertex2_radius = tagged.at(i).triangle->vertex2_radius;
					list.push_back(tri);
				}
			}
			for(int i = 0; i < temp_triangles.size(); i++)
			{
				list.push_back(temp_triangles.at(i));
			}
			
			
			tagged.clear();
			temp_triangles.clear();
			anzahl = anzahl - 1;
			subdivided.clear();

			tagged.clear();
			out << list.size() << "\n";
			out.close();
			//return list;
			return Subdivide(list,treshold,anzahl);
			
			
		}
		else
		{
			subdivided.clear();
			tagged.clear();
			return triangles;
		}
		

			

			
	
}
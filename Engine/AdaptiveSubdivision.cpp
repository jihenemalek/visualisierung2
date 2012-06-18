#include "AdaptiveSubdivision.h"
#include <assert.h>

AdaptiveSubdivision::AdaptiveSubdivision(void)
{
}
AdaptiveSubdivision::~AdaptiveSubdivision(void)
{
}

float AdaptiveSubdivision::calculateCurvature(Mesh::Triangle triangle)
{
	D3DXVECTOR3 AB = triangle.vertex1 - triangle.vertex0;
	D3DXVECTOR3 BC = triangle.vertex2 - triangle.vertex1;
	D3DXVECTOR3 CA = triangle.vertex0 - triangle.vertex2;
	float a = D3DXVec3Length(&AB);
	float b = D3DXVec3Length(&BC);
	float c = D3DXVec3Length(&CA);
	float s = 0.5f * (a + b + c);

	float kappa = 4.0f * sqrt(fabs(s * (s - a) * (s - b) * (s - c))) / (a * b * c);

	return kappa;
}

std::vector<Mesh::Triangle> AdaptiveSubdivision:: Subdivide(std::vector<Mesh::Triangle> triangles, float threshold, int anzahl)
{
	std::vector<Mesh::Triangle> temp_triangles;
	
	float kappa = 0.0f;

	D3DXVECTOR3 temp_vec;
	Mesh::Triangle temp_triangle;
	Subdivided temp_subdivided;
	Tagged temp_tagged;
	
	for(std::vector<Mesh::Triangle>::iterator it = triangles.begin(); it != triangles.end(); it++) 
	{
		Mesh::Triangle cTriangle = (*it);

		kappa = this->calculateCurvature(cTriangle);
		//Wenn Krümmung größer als ein threshold ->teile das Dreieck in 4 neue Dreiecke
		if(kappa > threshold) 
		{
			D3DXVECTOR3 mittelpunkt[3];
			D3DXVECTOR3 centerline[3];
			float		radius[3];

			for (int i = 0; i < 3; i++) 
			{
				mittelpunkt[i]	= (cTriangle.vertexAt(i + 1) + cTriangle.vertexAt(i)) / 2.0f;
				centerline[i]	= (cTriangle.centerlineAt(i + 1) + cTriangle.centerlineAt(i)) / 2.0f;
				radius[i]		= (cTriangle.radiusAt(i + 1) + cTriangle.radiusAt(i)) / 2.0f;

				D3DXVECTOR3 upVector = mittelpunkt[i] - centerline[i];
				D3DXVec3Normalize(&upVector, &upVector);
				mittelpunkt[i] = centerline[i] + (upVector * radius[i]);
			}


			for (int i = 0; i < 3; i++) 
			{
				Mesh::Triangle triangle;
				triangle.setVertexAt(0, cTriangle.vertexAt(i));
				triangle.setVertexAt(1, mittelpunkt[i]);
				triangle.setVertexAt(2, mittelpunkt[(i + 2) % 3]);
					
				triangle.setCenterlineAt(0, cTriangle.centerlineAt(i));
				triangle.setCenterlineAt(1, centerline[i]);
				triangle.setCenterlineAt(2, centerline[(i + 2) % 3]);

				triangle.setRadiusAt(0, cTriangle.radiusAt(i));
				triangle.setRadiusAt(1, radius[i]);
				triangle.setRadiusAt(2, radius[(i + 2) % 3]);

				temp_triangles.push_back(triangle);
			}

			Mesh::Triangle triangle;
			for (int i = 0; i < 3; i++) 
			{
				triangle.setVertexAt(i, mittelpunkt[i]);
				triangle.setCenterlineAt(i, centerline[i]);
				triangle.setRadiusAt(i, radius[i]);
			}
			temp_triangles.push_back(triangle);

			temp_subdivided.triangle = &cTriangle;
			subdivided.push_back(temp_subdivided);
		} 
		else 
		{ 
			//Ansonsten ist es getaggt
			temp_tagged.triangle = &cTriangle;
			temp_tagged.geteilt = 0;
			tagged.push_back(temp_tagged);
		}	
	}

	//Für alle getaggeden Triangles wird geschaut welche nichtgetaggden anliegen ->die Teilungsvetices werden gemerk
		
	D3DXVECTOR3 mittelpunkt;
	float radius;

	for(int i = 0; i < tagged.size(); i++)
	{
		//Hier wird geschaut wieviele Nachbarn ein triangle hat und der Vertex in die Liste geschrieben
		for(int j = 0; j < subdivided.size(); j++)
		{
			D3DXVECTOR3 t[3], s[3];

			t[0] = tagged.at(i).triangle->vertex1 - tagged.at(i).triangle->vertex0;
			D3DXVec3Normalize(&t[0], &t[0]);
			t[1] = tagged.at(i).triangle->vertex2 - tagged.at(i).triangle->vertex0;
			D3DXVec3Normalize(&t[1], &t[1]);
			t[2] = tagged.at(i).triangle->vertex2 - tagged.at(i).triangle->vertex1;
			D3DXVec3Normalize(&t[2], &t[2]);

			s[0] = subdivided.at(j).triangle->vertex1 - subdivided.at(j).triangle->vertex0;
			D3DXVec3Normalize(&s[0], &s[0]);
			s[1] = subdivided.at(j).triangle->vertex2 - subdivided.at(j).triangle->vertex0;
			D3DXVec3Normalize(&s[1], &s[1]);
			s[2] = subdivided.at(j).triangle->vertex2 - subdivided.at(j).triangle->vertex1;
			D3DXVec3Normalize(&s[2], &s[2]);
			
			for (int k = 0; i < 3; i++) 
			{
				for (int l = 0; j < 3; j++) 
				{
					if (t[k] == s[l]) 
					{
						int v1 = 0;
						int v2 = l;

						if (l == 2) 
						{ 
							v1 = 1;
							v2 = 2;
						}

						Vertex vertex;
						vertex.vertex		= (subdivided.at(j).triangle->vertexAt(v1)		+ subdivided.at(j).triangle->vertexAt(v2))		/ 2.0f;
						vertex.mittelpunkt	= (subdivided.at(j).triangle->centerlineAt(v1)	+ subdivided.at(j).triangle->centerlineAt(v2))	/ 2.0f;
						vertex.radius		= (subdivided.at(j).triangle->radiusAt(v1)		+ subdivided.at(j).triangle->radiusAt(v2))		/ 2.0f;

						temp_vec = vertex.vertex - vertex.mittelpunkt;
						D3DXVec3Normalize(&temp_vec, &temp_vec);
						
						vertex.vertex = vertex.mittelpunkt + (temp_vec * vertex.radius);

						tagged.at(i).vertices.push_back(vertex);
						tagged.at(i).number.push_back(0);
					}
				}
			}
		}

			//Hier werden die getaggted Triangles geteilt wenn die Anzahl ihrer Nachbarn nicht 0 ist
			if(tagged.at(i).number.size() != 0)
			{
				//Wenn Anzahl der Nachbarn gleich 1 ist:
				if(tagged.at(i).number.size() == 1)
				{
					tagged.at(i).geteilt = 1;

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
						temp_triangle.vertex2_mittelpunkt = tagged.at(i).triangle->vertex2_mittelpunkt;
						temp_triangle.vertex2_radius = tagged.at(i).triangle->vertex2_radius;

						temp_triangles.push_back(temp_triangle);

					}
				}

				else
				{
					if(tagged.at(i).number.size() == 2)
					{

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
							temp_triangle.vertex1_mittelpunkt = tagged.at(i).triangle->vertex1_mittelpunkt;
							temp_triangle.vertex2_mittelpunkt = tagged.at(i).vertices.at(temp_vertex1).mittelpunkt;

							temp_triangle.vertex0_radius = tagged.at(i).vertices.at(temp_vertex0).radius;
							temp_triangle.vertex1_radius = tagged.at(i).triangle->vertex1_radius;
							temp_triangle.vertex2_radius = tagged.at(i).vertices.at(temp_vertex1).radius;

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
		
		if(anzahl != 0)
		{
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
			//return list;
			return Subdivide(list,threshold,anzahl);
		}
		else
		{
			subdivided.clear();
			tagged.clear();
			return triangles;
		}
}
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
	D3DXVECTOR3 AB, BC, CA, mittelpunkta, mittelpunktb,mittelpunktc;
	Mesh::Triangle temp_triangle;
	Subdivided temp_subdivided;
	Tagged temp_tagged;
	int nochmal = 0;

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
				nochmal = nochmal + 1;
				mittelpunkta = (triangles.at(i).vertex1 + triangles.at(i).vertex0) / 2;
				mittelpunktb = (triangles.at(i).vertex2 + triangles.at(i).vertex1) / 2;
				mittelpunktc = (triangles.at(i).vertex0 + triangles.at(i).vertex2) / 2;

				temp_triangle.vertex0 = triangles.at(i).vertex0;
				temp_triangle.vertex1 = mittelpunkta;
				temp_triangle.vertex2 = mittelpunktc;

				temp_triangles.push_back(temp_triangle);

				temp_triangle.vertex0 = mittelpunkta;
				temp_triangle.vertex1 = triangles.at(i).vertex1;
				temp_triangle.vertex2 = mittelpunktb;

				temp_triangles.push_back(temp_triangle);

				temp_triangle.vertex0 = mittelpunktc;
				temp_triangle.vertex1 = mittelpunktb;
				temp_triangle.vertex2 = triangles.at(i).vertex2;

				temp_triangles.push_back(temp_triangle);

				temp_triangle.vertex0 = mittelpunktc;
				temp_triangle.vertex1 = mittelpunkta;
				temp_triangle.vertex2 = mittelpunktb;

				temp_triangles.push_back(temp_triangle);
			}
			//Ansonsten ist es getaggt
			else
			{ 
				temp_tagged.triangle = &triangles.at(i);
				tagged.push_back(temp_tagged);
			}

			
		}

		//Für alle getaggeden Triangles wird geschaut welche nichtgetaggden anliegen ->die Teilungsvetices werden gemerkt
		for(int i = 0; i < tagged.size(); i++)
		{
			for(int j = 0; j < subdivided.size(); j++)
			{
				if((tagged.at(i).triangle->vertex0 == subdivided.at(j).triangle->vertex0) && (tagged.at(i).triangle->vertex1 == subdivided.at(j).triangle->vertex1))
				{
					D3DXVECTOR3 vertex;
					vertex = (tagged.at(i).triangle->vertex0 + tagged.at(i).triangle->vertex1) / 2;
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(0);
				}
				if((tagged.at(i).triangle->vertex0 == subdivided.at(j).triangle->vertex0) && (tagged.at(i).triangle->vertex2 == subdivided.at(j).triangle->vertex2))
				{
					D3DXVECTOR3 vertex;
					vertex = (tagged.at(i).triangle->vertex0 + tagged.at(i).triangle->vertex2) / 2;
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(2);
				}
				if((tagged.at(i).triangle->vertex2 == subdivided.at(j).triangle->vertex2) && (tagged.at(i).triangle->vertex1 == subdivided.at(j).triangle->vertex1))
				{
					D3DXVECTOR3 vertex;
					vertex = (tagged.at(i).triangle->vertex2 + tagged.at(i).triangle->vertex1) / 2;
					tagged.at(i).vertices.push_back(vertex);
					tagged.at(i).number.push_back(1);
				}
			}

			//tHier werden die getaggted Triangles geteilt wenn die Anzahl ihrer Nachbarn nicht 0 ist
			if(tagged.at(i).number.size() != 0)
			{
				tagged.at(i).geteilt = 1;
				if(tagged.at(i).number.size() == 1)
				{
					if(tagged.at(i).number.at(0) == 0)
					{
						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).vertices.at(0);

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(0);
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);
					}
					if(tagged.at(i).number.at(0) == 1)
					{
					
						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).vertices.at(0);
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(0);
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);
					}
					if(tagged.at(i).number.at(0) == 2)
					{
						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).vertices.at(0);

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).vertices.at(0);
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);
					
					}
				}

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
						temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0);
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0);
						temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1);
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0);
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex1);

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
						temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0);
						temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2);

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0);
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2);

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex2);
						temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);
					}

					if(((tagged.at(i).number.at(0) == 1) && (tagged.at(i).number.at(1) == 2)) || ((tagged.at(i).number.at(0) == 1) && (tagged.at(i).number.at(1) == 0)))
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
						temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex1);

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).triangle->vertex0;
						temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1);
						temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2);

						temp_triangles.push_back(temp_triangle);

						temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex2);
						temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1);
						temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

						temp_triangles.push_back(temp_triangle);
					}
				}
				if(tagged.at(i).number.size() == 3)
				{
					int temp_vertex0, temp_vertex1, temp_vertex2;
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
					temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0);
					temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex2);

					temp_triangles.push_back(temp_triangle);

					temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0);
					temp_triangle.vertex1 = tagged.at(i).triangle->vertex1;
					temp_triangle.vertex2 = tagged.at(i).vertices.at(temp_vertex1);

					temp_triangles.push_back(temp_triangle);

					temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex0);
					temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex1);
					temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

					temp_triangles.push_back(temp_triangle);

					temp_triangle.vertex0 = tagged.at(i).vertices.at(temp_vertex2);
					temp_triangle.vertex1 = tagged.at(i).vertices.at(temp_vertex0);
					temp_triangle.vertex2 = tagged.at(i).triangle->vertex2;

					temp_triangles.push_back(temp_triangle);
				}
			}

		}

		//jetzt müssen noch die richtigen Triangles in die original TriangleList geschrieben werden -> und dann nochmals aufgerufen werden, wenn in dem Durchgang mindestens ein Triangle geteilt wurde
		
		
		if(anzahl != 0)
		{
			std::ofstream out;
			out.open("bla2.txt", std::ios::app);
			out << triangles.size() << "\n";
			out.close();
			

			std::vector<Mesh::Triangle> list;
			for(int i = 0; i < tagged.size(); i++)
			{
				if(tagged.at(i).geteilt != 1)
				{
					Mesh::Triangle tri;
					tri.vertex0 = tagged.at(i).triangle->vertex0;
					tri.vertex1 = tagged.at(i).triangle->vertex1;
					tri.vertex2 = tagged.at(i).triangle->vertex2;
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
			
			return Subdivide(list,treshold,anzahl);
			
			
		}
		else
		{
			return triangles;
		}
		

			

			
	
}
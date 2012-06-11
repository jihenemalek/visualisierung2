//
// Tree.hpp
// Visualisierung2 - Vesseltree
// 
// Created by Markus Mühlberger on 01.05.12
// Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

#ifndef Visualisierung2_Vesseltree_Tree_hpp
#define Visualisierung2_Vesseltree_Tree_hpp

#include <d3d11.h>
#include <d3dx10math.h>
#include <fstream>
#include <vector>

namespace Vesseltree
{
	//
	// A point is a simple representation of a vertex. Other values like normal
	// and extended data may be added in the future
	//
	typedef struct Point {
		float x;
		float y;
		float z;

		Point() { x = 0.0f; y = 0.0f; z = 0.0f; };
		Point(float _x, float _y, float _z) { x = _x; y = _y; z = _z; };
		Point operator+(Point p) { Point c; c.x = x + p.x; c.y = y + p.y; c.z = z + p.z; return c; };
		Point operator/(float c) { Point p; p.x = x / c; p.y = y / c; p.z = z / c; return p; };
		void operator+=(Point p) { x += p.x; y += p.y; z += p.z; };
		void operator/=(float c) { x /= c; y /= c; z /= c; };
	} Point;

	//
	// A node is a generic struct for storing control/segment points
	//
	typedef struct Node {
		Point position;		// Position of the control point center
		float radius;		// Radius of the control point
		float offset;		// An offset adde to the radius
		Node  *parent;		// The parent element of the node
		Node  *child;		// The child element of the node
		D3DXVECTOR3 direction; //saves direction of the cross-section
		D3DXVECTOR3 normal; // normal of the cross-section
		D3DXVECTOR3 upVector;
		std::vector<D3DXVECTOR3> vertices;


		Node() { position = Point(); };

		Node * combine(Node *n) {
			Node *node;
			node->position = (position + n->position) / 2.0f;
			node->radius = (radius + n->radius) / 2.0f;
			node->offset = (offset + n->offset) / 2.0f;
			node->parent = parent;
			node->child = n->child;
			node->direction = (direction + n->direction) / 2.0f;
			node->normal = (normal + n->normal) / 2.0f;
			node->upVector = (upVector + n->upVector) / 2.0f;

			return node;
		};
	} Node;
	
	//
	// Control points are the corner stones of a segment. They have an identifier
	// for parsing and attaching them to segments
	//
	typedef struct ControlPoint : Node {
		int identifier;
	} ControlPoint;

	//
	// Segment points are additional points between the control points of a segment
	//
	typedef Node SegmentPoint;

	//
	// Segment types
	//
	typedef enum {
		kSegmentTypeTracked,		// Has segment points
		kSegmentTypeCentered,		// Has segment points
		kSegmentTypeInterpolated	// No segment points. Interpolation between control points
	} SegmentType;

	//
	// A segment contains a list of points and is connected to other segments
	//
	typedef struct Segment {
		ControlPoint				*startNode;		// Starting control point
		ControlPoint				*endNode;		// End control point
		std::vector<SegmentPoint *> points;			// Points between start and end
		SegmentType					type;			// Type of the segment
		std::vector<Segment *>		parents;		// Parent nodes of the segment
		std::vector<Segment *>		children;		// Child nodes of the segment
		int classification; //0 for backward and 1 for forward
	} Segment;


	//
	// Root node of the vessel tree
	//
	typedef struct Root {
		Point   dataSize;		// Meta information, currently not used
		Point   dataSpacing;	// Meta information, currently not used
		Segment *child;			// First segment of the tree

		Root() { dataSize = Point(); dataSpacing = Point(); }
	} Root;
};

#endif
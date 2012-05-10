//
// Parser.hpp
// Visualisierung2 - Vesseltree
// 
// Created by Markus Mühlberger on 01.05.12
// Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

#ifndef Visualisierung2_Vesseltree_Parser_hpp
#define Visualisierung2_Vesseltree_Parser_hpp

#include <libxml/tree.h>
#include <Windows.h>

#include <vector>

#include "Tree.hpp"

namespace Vesseltree
{

	class Parser
	{
		
	private:
		static Root parseHeader(xmlNode *aNode);
		static std::vector<ControlPoint *> parseControlPoints(xmlNode *aNode);
		static std::vector<Segment *> parseSegments(xmlNode *aNode, std::vector<ControlPoint *> controlPoints);
		static std::vector<SegmentPoint *> parseSegmentPoints(xmlNode *aNode);

	public:

		static std::vector<ControlPoint *> controlPoints;

		static Root * parseDocument(const char *documentPath, HWND hwnd);

	};

};

#endif
//
// Parser.cpp
// Visualisierung2 - Vesseltree
// 
// Created by Markus Mühlberger on 01.05.12
// Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

#include "Parser.hpp"

#include <array>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>

static int strToInt(std::string value);
static float strToFlt(std::string value);

static std::map<std::string, std::string> parseAttributes(xmlNode *aNode);
static std::map<std::string, std::string> parseElements(xmlNode *aNode);

std::vector<Vesseltree::ControlPoint *> Vesseltree::Parser::controlPoints;

Vesseltree::Root * Vesseltree::Parser::parseDocument(const char *documentPath, HWND hwnd)
{
	xmlDoc *document = NULL;
	xmlNode *rootElement = NULL;

	document = xmlReadFile(documentPath, NULL, 0);

	if (document == NULL) {
		MessageBox(hwnd, L"Could not load XML file", L"Error", MB_OK);
		xmlCleanupParser();
		return NULL;
	}

	rootElement = xmlDocGetRootElement(document);

	Vesseltree::Root * rootNode = new Vesseltree::Root();

	xmlNode *element = NULL;

	std::vector<ControlPoint *> controlPoints;
	std::vector<Segment *> segments;

	for (element = rootElement->children; element; element = element->next) {
        if (element->type == XML_ELEMENT_NODE) {
			if (strcmp((char *)element->name, "cps") == 0) {
				// We have the control points section
				controlPoints = parseControlPoints(element);
			} else if (xmlStrEqual(element->name, xmlCharStrdup("segments"))) {
				// We have a segment
				segments = parseSegments(element, controlPoints);
			} else {
			}
		}
	}

	// Free and cleanup
	xmlFreeDoc(document);
	xmlCleanupParser();

	rootNode->child = segments[0];

	return rootNode;
}

Vesseltree::Root Vesseltree::Parser::parseHeader(xmlNode *aNode)
{
	return Root();
}

std::vector<Vesseltree::ControlPoint *> Vesseltree::Parser::parseControlPoints(xmlNode *aNode)
{
	std::map<std::string, std::string> attributes = parseAttributes(aNode);
	int count = strToInt(attributes["count"]);

	std::vector<ControlPoint *> controlPoints;
	controlPoints.resize(count);

	xmlNode *currentControlPoint = aNode->children;

	while(currentControlPoint) {
		if (currentControlPoint->type == XML_ELEMENT_NODE) {
			ControlPoint *cp = new ControlPoint();
			std::map<std::string, std::string> elements = parseElements(currentControlPoint);

			cp->identifier = strToInt(elements["id"]);
			cp->position.x = strToFlt(elements["x"]);
			cp->position.y = strToFlt(elements["y"]);
			cp->position.z = strToFlt(elements["z"]);
			cp->radius =	 strToFlt(elements["r1"]);
			cp->offset =	 strToFlt(elements["offset"]);

			controlPoints[cp->identifier - 1] = cp;
		}

		// Get the next element
		currentControlPoint = currentControlPoint->next;
	}

	return controlPoints;
}

std::vector<Vesseltree::Segment *> Vesseltree::Parser::parseSegments(xmlNode *aNode, std::vector<Vesseltree::ControlPoint *> controlPoints)
{
	std::vector<Segment *> segments;
	
	// Store for connecting the points afterwards
	std::vector<std::vector<Segment *> > startControlPoints;
	startControlPoints.resize(controlPoints.size());
	std::vector<std::vector<Segment *> > endControlPoints;
	endControlPoints.resize(controlPoints.size());

	xmlNode *currentSegment = aNode->children;
	while (currentSegment) {
		if (currentSegment->type == XML_ELEMENT_NODE) {
			Segment *seg = new Segment();
			std::string type = parseAttributes(currentSegment)["type"];
			std::map<std::string, std::string> elements = parseElements(currentSegment);
			
			int startNodeId = strToInt(elements["BeginCPID"]) - 1;
			int endNodeId = strToInt(elements["EndCPID"]) - 1;

			seg->startNode = controlPoints[startNodeId];
			seg->endNode = controlPoints[endNodeId];

			if (strcmp(type.c_str(), "centered")) {
				seg->type = kSegmentTypeCentered;
			} else if (strcmp(type.c_str(), "tracked")) {
				seg->type = kSegmentTypeTracked;
			} else if (strcmp(type.c_str(), "interpolated")) {
				seg->type = kSegmentTypeInterpolated;
			}

			// Parse the points if the type is correct
			if (seg->type == kSegmentTypeCentered || seg->type == kSegmentTypeTracked) {
				xmlNodePtr pointsPtr = aNode->children;
				while(pointsPtr) {
					if (pointsPtr->type == XML_ELEMENT_NODE) {
						if (strcmp((char *)pointsPtr->name, "points")) {
							seg->points = parseSegmentPoints(pointsPtr);
							break;
						}
					}
					pointsPtr = pointsPtr->next;
				}
			}
			
			// Add the segment as start/end points to the control point id
			startControlPoints[startNodeId].push_back(seg);
			endControlPoints[endNodeId].push_back(seg);

			segments.push_back(seg);
		}

		currentSegment = currentSegment->next;
	}

	// Connect the segments according to their control points
	for (unsigned int i = 0; i < endControlPoints.size(); i++) {
		std::vector<Segment *> parents = endControlPoints[i];

		for (std::vector<Segment *>::iterator pIt = parents.begin(); pIt != parents.end(); pIt++) {
			Segment *parent = *pIt;

			// Add all children to the parent and vice versa
			std::vector<Segment *> children = startControlPoints[i];
			for (std::vector<Segment *>::iterator cIt = children.begin(); cIt != children.end(); cIt++) {
				Segment *child = *cIt;
				parent->children.push_back(child);
				child->parents.push_back(parent);
			}
		}
	}

	return segments;
}

std::vector<Vesseltree::SegmentPoint *> Vesseltree::Parser::parseSegmentPoints(xmlNode *aNode)
{
	std::vector<SegmentPoint *> segmentPoints;

	xmlNode *currentSegmentPoint = aNode->children;
	while (currentSegmentPoint) {
		if (currentSegmentPoint->type == XML_ELEMENT_NODE) {
			std::map<std::string, std::string> elements = parseElements(currentSegmentPoint);

			SegmentPoint *sp = new SegmentPoint();
			sp->position.x = strToFlt(elements["x"]);
			sp->position.y = strToFlt(elements["y"]);
			sp->position.z = strToFlt(elements["z"]);
			sp->radius = strToFlt(elements["r1"]);
			sp->offset = strToFlt(elements["offset"]);

			segmentPoints.push_back(sp);
		}

		currentSegmentPoint = currentSegmentPoint->next;
	}

	return segmentPoints;
}

//
// Supporter functions
//

int strToInt(std::string value) 
{ 
    std::stringstream ss(value); 
    int ret; 
    ss >> ret; 

    return ret; 
}

float strToFlt(std::string value) 
{ 
    return (float)::atof(value.c_str());
} 

std::map<std::string, std::string> parseAttributes(xmlNode *aNode)
{
	std::map<std::string, std::string> attributes;

	xmlAttrPtr attribute = aNode->properties;
	while (attribute && attribute->name && attribute->children) {
		xmlChar *value = xmlNodeListGetString(aNode->doc, attribute->children, 1);
		attributes.insert(std::pair<std::string, std::string>((char *)attribute->name, (char *)value));
		//xmlFree(value);

		attribute = attribute->next;
	}

	return attributes;
}

std::map<std::string, std::string> parseElements(xmlNode *aNode)
{
	std::map<std::string, std::string> elements;

	xmlNodePtr element = aNode->children;
	while (element) {
		if (element->type == XML_ELEMENT_NODE) {
			if (element->children) {
				xmlChar *value = element->children->content;
				elements.insert(std::pair<std::string, std::string>((char *)element->name, (char *)value));
			} else {
				elements.insert(std::pair<std::string, std::string>((char *)element->name, ""));
			}
			//xmlFree(value);
		}

		element = element->next;
	}

	return elements;
}
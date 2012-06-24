//
//  Triangle.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "Triangle.h"

@implementation Triangle {
  GLKVector3 vertex[3];
  GLKVector3 center[3];
  
  float curvature[3];
  float radius[3];
  
  __weak id neighbor[3];
}

#pragma mark - Getter and Setter methods

- (GLKVector3)vertexAtIndex:(NSUInteger)idx
{
  return vertex[idx % 3];
}

- (GLKVector3)centerAtIndex:(NSUInteger)idx
{
  return center[idx % 3];
}

- (GLKVector3)normalAtIndex:(NSUInteger)idx
{
  return GLKVector3Normalize(GLKVector3Subtract(vertex[idx % 3], center[idx % 3]));
}

- (float)curvatureAtIndex:(NSUInteger)idx
{
  return curvature[idx % 3];
}

- (float)radiusAtIndex:(NSUInteger)idx
{
  return radius[idx % 3];
}

- (void)setVertex:(GLKVector3)aVertex atIndex:(NSUInteger)idx
{
  vertex[idx % 3] = aVertex;
}

- (void)setCenter:(GLKVector3)aCenter atIndex:(NSUInteger)idx
{
  center[idx % 3] = aCenter;
}

- (void)setCurvature:(float)aCurvature atIndex:(NSUInteger)idx
{
  curvature[idx % 3] = aCurvature;
}

- (void)setRadius:(float)aRadius atIndex:(NSUInteger)idx
{
  radius[idx % 3] = aRadius;
}

- (__weak Triangle *)neighborAtIndex:(NSUInteger)idx
{
  return neighbor[idx % 3];
}

- (void)setNeighbor:(__weak Triangle *)aNeighbor atIndex:(NSUInteger)idx
{
  neighbor[idx % 3] = aNeighbor;
}

@end

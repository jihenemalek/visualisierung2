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

@synthesize tagged = _tagged;

@synthesize subtriangles = _subtriangles;

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

#pragma mark - Subdivisioning

- (NSArray *)subdivide4
{
  // TODO: Move the vertex along the curvature
  GLKVector3 ABposition = GLKVector3DivideScalar(GLKVector3Add([self vertexAtIndex:0], [self vertexAtIndex:1]), 2.0f);
  GLKVector3 ACposition = GLKVector3DivideScalar(GLKVector3Add([self vertexAtIndex:0], [self vertexAtIndex:2]), 2.0f);
  GLKVector3 BCposition = GLKVector3DivideScalar(GLKVector3Add([self vertexAtIndex:1], [self vertexAtIndex:2]), 2.0f);
  
  GLKVector3 ABcenter = GLKVector3DivideScalar(GLKVector3Add([self centerAtIndex:0], [self centerAtIndex:1]), 2.0f);
  GLKVector3 ACcenter = GLKVector3DivideScalar(GLKVector3Add([self centerAtIndex:0], [self centerAtIndex:2]), 2.0f);
  GLKVector3 BCcenter = GLKVector3DivideScalar(GLKVector3Add([self centerAtIndex:1], [self centerAtIndex:2]), 2.0f);
  
  float ABradius = ([self radiusAtIndex:0] + [self radiusAtIndex:1]) / 2.0f;
  float ACradius = ([self radiusAtIndex:0] + [self radiusAtIndex:1]) / 2.0f;
  float BCradius = ([self radiusAtIndex:0] + [self radiusAtIndex:1]) / 2.0f;
  
  Triangle *t0 = [Triangle new];
  Triangle *t1 = [Triangle new];
  Triangle *t2 = [Triangle new];
  Triangle *t3 = [Triangle new];
  
  [t0 setVertex:ABposition atIndex:0];
  [t0 setVertex:ACposition atIndex:1];
  [t0 setVertex:[self vertexAtIndex:0] atIndex:2];
  [t0 setCenter:ABcenter atIndex:0];
  [t0 setCenter:ACcenter atIndex:1];
  [t0 setCenter:[self centerAtIndex:0] atIndex:2];
  [t0 setRadius:ABradius atIndex:0];
  [t0 setRadius:ACradius atIndex:1];
  [t0 setRadius:[self radiusAtIndex:0] atIndex:2];
  
  [t1 setVertex:BCposition atIndex:0];
  [t1 setVertex:ABposition atIndex:1];
  [t1 setVertex:[self vertexAtIndex:1] atIndex:2];
  [t1 setCenter:BCcenter atIndex:0];
  [t1 setCenter:ABcenter atIndex:1];
  [t1 setCenter:[self centerAtIndex:1] atIndex:2];
  [t1 setRadius:BCradius atIndex:0];
  [t1 setRadius:ABradius atIndex:1];
  [t1 setRadius:[self radiusAtIndex:1] atIndex:2];
  
  [t2 setVertex:ACposition atIndex:0];
  [t2 setVertex:BCposition atIndex:1];
  [t2 setVertex:[self vertexAtIndex:2] atIndex:2];
  [t2 setCenter:ACcenter atIndex:0];
  [t2 setCenter:BCcenter atIndex:1];
  [t2 setCenter:[self centerAtIndex:2] atIndex:2];
  [t2 setRadius:ACradius atIndex:0];
  [t2 setRadius:BCradius atIndex:1];
  [t2 setRadius:[self radiusAtIndex:2] atIndex:2];
  
  [t3 setVertex:ABposition atIndex:0];
  [t3 setVertex:BCposition atIndex:1];
  [t3 setVertex:ACposition atIndex:2];
  [t1 setCenter:ABcenter atIndex:0];
  [t1 setCenter:BCcenter atIndex:1];
  [t1 setCenter:ACcenter atIndex:2];
  [t1 setRadius:ABradius atIndex:0];
  [t1 setRadius:BCradius atIndex:1];
  [t1 setRadius:ACradius atIndex:2];
  
  // Set neighborship data
  [t0 setNeighbor:t3 atIndex:0];
  [t1 setNeighbor:t3 atIndex:0];
  [t2 setNeighbor:t3 atIndex:0];
  [t3 setNeighbor:t1 atIndex:0];
  [t3 setNeighbor:t2 atIndex:1];
  [t3 setNeighbor:t0 atIndex:2];
  
  NSArray *newTriangles = [NSArray arrayWithObjects:t0, t1, t2, t3, nil];
  
  for (NSUInteger n = 0; n < 3; n++) {
    if ([self neighborAtIndex:n] && [[[self neighborAtIndex:n] subtriangles] count] > 0) {
      NSUInteger i;
      
      for (i = 0; i < 4; i++) {
        if ([[self neighborAtIndex:n] neighborAtIndex:i] == self) break;
      }
      
      if ([[[self neighborAtIndex:n] subtriangles] count] == 4) {
        [[newTriangles objectAtIndex:n] setNeighbor:[[[self neighborAtIndex:n] subtriangles] objectAtIndex:(i + 1) % 3] atIndex:2];
        [[[[self neighborAtIndex:n] subtriangles] objectAtIndex:(i + 1) % 3] setNeighbor:[newTriangles objectAtIndex:n] atIndex:1];
        
        [[newTriangles objectAtIndex:(n + 1) % 3] setNeighbor:[[[self neighborAtIndex:n] subtriangles] objectAtIndex:(i + 1) % 3] atIndex:1];
        [[[[self neighborAtIndex:n] subtriangles] objectAtIndex:(i + 1) % 3] setNeighbor:[newTriangles objectAtIndex:(n + 1) % 3] atIndex:2];
      }
      
      if ([[[self neighborAtIndex:n] subtriangles] count] == 2) {
        [[newTriangles objectAtIndex:n] setNeighbor:[[[self neighborAtIndex:n] subtriangles] objectAtIndex:1] atIndex:2];
        [[[[self neighborAtIndex:n] subtriangles] objectAtIndex:1] setNeighbor:[newTriangles objectAtIndex:n] atIndex:1];
        
        [[newTriangles objectAtIndex:(n + 1) % 3] setNeighbor:[[[self neighborAtIndex:n] subtriangles] objectAtIndex:0] atIndex:1];
        [[[[self neighborAtIndex:n] subtriangles] objectAtIndex:0] setNeighbor:[newTriangles objectAtIndex:(n + 1) % 3] atIndex:2];
      }
    }
  }

  self.subtriangles = newTriangles;
  
  return newTriangles;
}

@end

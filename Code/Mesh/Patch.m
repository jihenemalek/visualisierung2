//
//  Patch.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "Patch.h"

#import "Triangle.h"

@implementation Patch {
  GLKVector3 vertex[4];
  GLKVector3 center[4];
  
  float curvature[4];
  float radius[4];
  
  __weak id neighbor[4];
}

@synthesize triangles = _triangles;
@synthesize subpatches = _subpatches;

- (GLKVector3)vertexAtIndex:(NSUInteger)idx
{
  return vertex[idx % 4];
}

- (void)setVertex:(GLKVector3)aVertex atIndex:(NSUInteger)idx
{
  vertex[idx % 4] = aVertex;
}

- (GLKVector3)centerAtIndex:(NSUInteger)idx
{
  return center[idx % 4];
}

- (void)setCenter:(GLKVector3)aCenter atIndex:(NSUInteger)idx
{
  center[idx % 4] = aCenter;
}

- (float)curvatureAtIndex:(NSUInteger)idx
{
  return curvature[idx % 4];
}

- (void)setCurvature:(float)aCurvature atIndex:(NSUInteger)idx
{
  curvature[idx % 4] = aCurvature;
}

- (float)radiusAtIndex:(NSUInteger)idx
{
  return radius[idx % 4];
}

- (void)setRadius:(float)aRadius atIndex:(NSUInteger)idx
{
  radius[idx % 4] = aRadius;
}

- (GLKVector3)normalAtIndex:(NSUInteger)idx
{
  return GLKVector3Normalize(GLKVector3Subtract(vertex[idx % 4], center[idx % 4]));
}

- (__weak Patch *)neighborAtIndex:(NSUInteger)idx
{
  return neighbor[idx % 4];
}

- (void)setNeighbor:(__weak Patch *)aNeighbor atIndex:(NSUInteger)idx
{
  neighbor[idx % 4] = aNeighbor;
}

#pragma mark - Subdivision methods

- (NSArray *)subdivide:(NSUInteger)times
{
  NSMutableArray *newPatches = [NSMutableArray array];
  [newPatches addObject:self];
  
  for (NSUInteger i = 0; i < times; i++) {
    NSMutableArray *patchBatch = [NSMutableArray arrayWithCapacity:[newPatches count]];
    
    for (Patch *patch in newPatches) {
      Patch * p0 = [Patch new];
      Patch * p1 = [Patch new];
      
      for (int j = 0; j < 4; j++) {
        [p0 setCenter:[patch centerAtIndex:j] atIndex:j];
        [p0 setRadius:[patch radiusAtIndex:j] atIndex:j];
        [p1 setCenter:[patch centerAtIndex:j] atIndex:j];
        [p1 setRadius:[patch radiusAtIndex:j] atIndex:j];
        [p0 setVertex:[patch vertexAtIndex:j] atIndex:j];
        [p1 setVertex:[patch vertexAtIndex:j] atIndex:j];
      }
      
      [p0 setRadius:(([patch radiusAtIndex:0] + [patch radiusAtIndex:1]) / 2.0f) atIndex:1];
      [p0 setRadius:(([patch radiusAtIndex:2] + [patch radiusAtIndex:3]) / 2.0f) atIndex:2];
      [p1 setRadius:[p0 radiusAtIndex:1] atIndex:0];
      [p1 setRadius:[p0 radiusAtIndex:2] atIndex:3];
      
      [p0 setCenter:GLKVector3DivideScalar(GLKVector3Add([patch centerAtIndex:0], [patch centerAtIndex:1]), 2.0f) atIndex:1];
      [p0 setCenter:GLKVector3DivideScalar(GLKVector3Add([patch centerAtIndex:2], [patch centerAtIndex:3]), 2.0f) atIndex:2];
      [p1 setCenter:[p0 centerAtIndex:1] atIndex:0];
      [p1 setCenter:[p0 centerAtIndex:2] atIndex:3];
      
      [p0 setVertex:GLKVector3Add([p0 centerAtIndex:1], GLKVector3MultiplyScalar(GLKVector3Normalize(GLKVector3Subtract(GLKVector3DivideScalar(GLKVector3Add([patch vertexAtIndex:0], [patch vertexAtIndex:1]), 2.0f), [p0 centerAtIndex:1])), [p0 radiusAtIndex:1])) atIndex:1];
      [p0 setVertex:GLKVector3Add([p0 centerAtIndex:2], GLKVector3MultiplyScalar(GLKVector3Normalize(GLKVector3Subtract(GLKVector3DivideScalar(GLKVector3Add([patch vertexAtIndex:2], [patch vertexAtIndex:3]), 2.0f), [p0 centerAtIndex:2])), [p0 radiusAtIndex:2])) atIndex:2];
      
      [p1 setVertex:[p0 vertexAtIndex:1] atIndex:0];
      [p1 setVertex:[p0 vertexAtIndex:2] atIndex:3];
      
      // Set neighbor info
      [p0 setNeighbor:[patch neighborAtIndex:0] atIndex:0];
      [p0 setNeighbor:p1 atIndex:2];
      
      [p1 setNeighbor:p0 atIndex:0];
      [p1 setNeighbor:[patch neighborAtIndex:2] atIndex:2];
    
      [patchBatch addObject:p0];
      [patchBatch addObject:p1];
    }
  
    newPatches = patchBatch;
  }

  self.subpatches = newPatches;

  if ([[self neighborAtIndex:1] subpatches] != nil) {
    [self.subpatches enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
      [obj setNeighbor:[[[self neighborAtIndex:1] subpatches] objectAtIndex:idx] atIndex:1];
      [[[[self neighborAtIndex:1] subpatches] objectAtIndex:idx] setNeighbor:obj atIndex:1];
    }];
  }
  
  if ([[self neighborAtIndex:1] subpatches] != nil) {
    [self.subpatches enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
      [obj setNeighbor:[[[self neighborAtIndex:3] subpatches] objectAtIndex:idx] atIndex:3];
      [[[[self neighborAtIndex:3] subpatches] objectAtIndex:idx] setNeighbor:obj atIndex:3];
    }];
  }

  return self.subpatches;
}

- (NSArray *)triangulate
{
  Triangle *t0 = [Triangle new];
  Triangle *t1 = [Triangle new];
  
  for (NSUInteger i = 0; i < 3; i++) {
    [t0 setVertex:[self vertexAtIndex:i] atIndex:i];
    [t0 setCenter:[self centerAtIndex:i] atIndex:i];
    [t0 setRadius:[self radiusAtIndex:i] atIndex:i];
  }
  
  for (NSUInteger i = 2; i < 5; i++) {
    [t1 setVertex:[self vertexAtIndex:i] atIndex:i];
    [t1 setCenter:[self centerAtIndex:i] atIndex:i];
    [t1 setRadius:[self radiusAtIndex:i] atIndex:i];
  }
  
  [t0 setNeighbor:t1 atIndex:2];
  [t1 setNeighbor:t1 atIndex:0];
  
  if ([[self neighborAtIndex:0] triangles] != nil) {
    [t1 setNeighbor:(Triangle *)[[[self neighborAtIndex:0] triangles] objectAtIndex:0] atIndex:2];
    [(Triangle *)[[[self neighborAtIndex:0] triangles] objectAtIndex:0] setNeighbor:t1 atIndex:1];
  }
  
  if ([[self neighborAtIndex:1] triangles] != nil) {
    [t0 setNeighbor:(Triangle *)[[[self neighborAtIndex:1] triangles] objectAtIndex:1] atIndex:0];
    [(Triangle *)[[[self neighborAtIndex:1] triangles] objectAtIndex:1] setNeighbor:t0 atIndex:1];
  }
  
  if ([[self neighborAtIndex:2] triangles] != nil) {
    [t0 setNeighbor:(Triangle *)[[[self neighborAtIndex:2] triangles] objectAtIndex:1] atIndex:1];
    [(Triangle *)[[[self neighborAtIndex:2] triangles] objectAtIndex:1] setNeighbor:t0 atIndex:2];
  }
  
  if ([[self neighborAtIndex:3] triangles] != nil) {
    [t1 setNeighbor:(Triangle *)[[[self neighborAtIndex:3] triangles] objectAtIndex:0] atIndex:1];
    [(Triangle *)[[[self neighborAtIndex:3] triangles] objectAtIndex:0] setNeighbor:t1 atIndex:0];
  }
  
  self.triangles = [NSArray arrayWithObjects:t0, t1, nil];
  return self.triangles;
}

@end

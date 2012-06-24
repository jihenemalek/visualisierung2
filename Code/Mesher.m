//
//  Mesher.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "Mesher.h"

@interface Mesher ()

@property (nonatomic, strong) NSMutableArray *generatedObjects;

- (NSArray *)generateQuadrilateralMesh:(Segment *)rootSegment;
- (NSArray *)subdivideQuadrilateralMesh:(NSArray *)mesh times:(NSUInteger)times;
- (NSArray *)triangulateQuadrilateralMesh:(NSArray *)mesh;

- (void)calculateDirections:(Segment *)segment;
- (void)calculateNormals:(Segment *)segment;
- (void)calculateUpVectors:(Segment *)segment;

- (GLKVector3)calculateUpVectorForPoint:(SegmentNode *)point;
- (GLKVector3)rotateUpVector:(GLKVector3)up aroundAxis:(GLKVector3)axis times:(NSUInteger)times;

- (void)tileTriviallyBetween:(SegmentNode *)prev and:(SegmentNode *)cur;
- (void)tileTree:(Segment *)segment;
- (void)tileJointForQuadrantVector:(GLKVector3)quadrantVector quadrantIndex:(NSUInteger)idx withSegments:(NSArray *)segments andCaller:(Segment *)caller andParentSegment:(Segment *)parent isBackward:(BOOL)backward wasEndNode:(BOOL)wasEndNode;

@end

@implementation Mesher

@synthesize generatedObjects = _generatedObjects;

+ (NSArray *)generateQuadrilateralMesh:(Segment *)rootSegment
{
  if (!rootSegment) return nil;
  
  Mesher *mesher = [Mesher new];
  
  return [mesher generateQuadrilateralMesh:rootSegment];
}

+ (NSArray *)subdivideQuadrilateralMesh:(NSArray *)mesh times:(NSUInteger)times
{
  if (!mesh || [mesh count] == 0) return nil;
  
  Mesher *mesher = [Mesher new];
  
  return [mesher subdivideQuadrilateralMesh:mesh times:times];
}

+ (NSArray *)triangulateQuadrilateralMesh:(NSArray *)mesh
{
  if (!mesh || [mesh count] == 0) return nil;
  
  Mesher *mesher = [Mesher new];
  
  return [mesher triangulateQuadrilateralMesh:mesh];
}

- (id)init
{
  self = [super init];
  if (self) {
    _generatedObjects = [NSMutableArray array];
  }
  return self;
}

- (NSArray *)generateQuadrilateralMesh:(Segment *)rootSegment
{
  [self calculateDirections:rootSegment];
  [self calculateNormals:rootSegment];
  [self calculateUpVectors:rootSegment];
  
  if ([rootSegment.segmentPoints count] > 0) {
    [self tileTriviallyBetween:rootSegment.startNode and:[rootSegment.segmentPoints objectAtIndex:0]];
  }
  
  [self tileTree:rootSegment];
  
  return self.generatedObjects;
}

- (NSArray *)subdivideQuadrilateralMesh:(NSArray *)mesh times:(NSUInteger)times
{
  for (Patch *p in mesh) {
    [self.generatedObjects addObjectsFromArray:[p subdivide:times]];
  }
  
  return self.generatedObjects;
}

- (NSArray *)triangulateQuadrilateralMesh:(NSArray *)mesh
{
  for (Patch *p in mesh) {
    [self.generatedObjects addObjectsFromArray:[p triangulate]];
  }
  
  return self.generatedObjects;
}

#pragma mark - Generation of directions, normals and up vectors

- (void)calculateDirections:(Segment *)segment
{
  if ([segment.segmentPoints count] > 0) {
    for (NSUInteger i = 0; GLKVector3Length(segment.startNode.direction) == 0; i++) {
      GLKVector3 v = GLKVector3Subtract(((SegmentNode *)[segment.segmentPoints objectAtIndex:i]).position, segment.startNode.position);
      if (GLKVector3Length(v) == 0.0f) continue;
      
      segment.startNode.direction = GLKVector3Normalize(v);
    }
    
    for(SegmentNode *s in segment.segmentPoints) {
      if (s == [segment.segmentPoints lastObject]) break;
      
      s.direction = GLKVector3Normalize(GLKVector3Subtract(s.next.position, s.position));
    }

    segment.endNode.prev.direction = GLKVector3Normalize(GLKVector3Subtract(segment.endNode.position, segment.endNode.prev.position));
    segment.endNode.direction = segment.endNode.prev.direction;
  } else {
    segment.startNode.direction = GLKVector3Normalize(GLKVector3Subtract(segment.endNode.position, segment.startNode.position));
    segment.endNode.direction = segment.startNode.direction;
  }
  
  segment.startNode.normal = segment.startNode.direction;
  segment.endNode.normal = segment.endNode.direction;

  for (Segment *s in segment.children) {
    [self calculateDirections:s];
  }
}

- (void)calculateNormals:(Segment *)segment
{
  // Calculate the normals for the segment
  if ([segment.segmentPoints count] > 0) {
    for (SegmentNode *s in segment.segmentPoints) {
      // TODO: Find out, why the normals are sometimes not correct
      s.normal = GLKVector3Normalize(GLKVector3Add(s.direction, s.prev.direction));
    }
  }
  
  // Average the normal of the end nod and the start node of all child segments
  if ([segment.children count] > 0) {
    GLKVector3 averagedNormal = segment.endNode.direction;
    int averageParticipants = 1;
    
    for (Segment *s in segment.children) {
      if (GLKVector3DotProduct(segment.endNode.direction, s.startNode.direction) > 0.0f) {
        averagedNormal = GLKVector3Add(averagedNormal, s.startNode.direction);
        averageParticipants++;
      }
    }
    
    averagedNormal = GLKVector3Normalize(GLKVector3DivideScalar(averagedNormal, averageParticipants));
    
    segment.endNode.normal = averagedNormal;
    
    for (Segment *s in segment.children) {
      if (GLKVector3DotProduct(segment.endNode.direction, s.startNode.direction) > 0.0f) {
        s.startNode.normal = averagedNormal;
      } else {
        s.startNode.normal = GLKVector3Negate(averagedNormal);
      }
    }
  }
  
  // Parse the rest of the tree
  for (Segment *s in segment.children) {
    [self calculateNormals:s];
  }
}

- (void)calculateUpVectors:(Segment *)segment
{
  if ([segment.parents count] == 0) {
    GLKVector3 startUp = GLKVector3Make(0, 0, 1);
    
    if (fabsf(segment.startNode.direction.x) < fabsf(segment.startNode.direction.y)) {
      startUp = GLKVector3Make(1, 0, 0);
    }
    
    if (fabsf(segment.startNode.direction.y) < fabsf(segment.startNode.direction.z)) {
      startUp = GLKVector3Make(0, 1, 0);
    }
    
    segment.startNode.up = GLKVector3Normalize(GLKVector3CrossProduct(startUp, segment.startNode.direction));
  } else {
    segment.startNode.up = [[[segment.parents lastObject] endNode] up];
  }
  
  NSLog(@"NEW SEGMENT");
  NSLog(@"Up Vector: %.2f, %.2f, %.2f, Normal: %.2f, %.2f, %.2f", segment.startNode.up.x, segment.startNode.up.y, segment.startNode.up.z, segment.startNode.normal.x, segment.startNode.normal.y, segment.startNode.normal.z);
  
  if ([segment.segmentPoints count] > 0) {
    for (SegmentNode *s in segment.segmentPoints) {
      NSLog(@"Up Vector: %.2f, %.2f, %.2f, Normal: %.2f, %.2f, %.2f", [self calculateUpVectorForPoint:s].x, [self calculateUpVectorForPoint:s].y, [self calculateUpVectorForPoint:s].z, s.normal.x, s.normal.y, s.normal.z);
      s.up = [self calculateUpVectorForPoint:s];
    }
  }
  
  segment.endNode.up = [self calculateUpVectorForPoint:segment.endNode];
  
  NSLog(@"Up Vector: %.2f, %.2f, %.2f, Normal: %.2f, %.2f, %.2f", segment.endNode.up.x, segment.endNode.up.y, segment.endNode.up.z, segment.endNode.normal.x, segment.endNode.normal.y, segment.endNode.normal.z);
  
  // Parse the rest of the tree
  for (Segment *s in segment.children) {
    [self calculateUpVectors:s];
  }
}

- (GLKVector3)calculateUpVectorForPoint:(SegmentNode *)point
{
  GLKVector3 pu = GLKVector3Add(point.prev.position, point.prev.up);
  float d = GLKVector3DotProduct(GLKVector3Subtract(point.position, pu), point.normal) /  GLKVector3DotProduct(point.prev.normal, point.normal);

  if (GLKVector3DotProduct(point.prev.normal, point.normal) == 0.0f) {
    return point.prev.up;
  }
  
  GLKVector3 pn = GLKVector3Add(pu, GLKVector3MultiplyScalar(point.prev.normal, d));
  
  return GLKVector3Normalize(GLKVector3Subtract(pn, point.position));
}


- (GLKVector3)rotateUpVector:(GLKVector3)up aroundAxis:(GLKVector3)axis times:(NSUInteger)times
{
  GLKVector3 rotatedUpVector = up;
  
  for (NSUInteger i = 0; i < times; i++) {
    float x = rotatedUpVector.x;
    float y = rotatedUpVector.y;
    float z = rotatedUpVector.z;
    float u = axis.x;
    float v = axis.y;
    float w = axis.z;
    
    rotatedUpVector = GLKVector3Normalize(GLKVector3Make(u * (u * x + v * y + w * z) + (-w * y + v * z),
                                                         v * (u * x + v * y + w * z) + ( w * x - u * z), 
                                                         w * (u * x + v * y + w * z) + (-v * x + u * y)));
  }
  
  return rotatedUpVector;
}

#pragma mark - Tiling algorithm methods

- (void)tileTree:(Segment *)segment
{
  if (segment.processed) return;
  
  for (SegmentNode *s in segment.segmentPoints) {
    if (s == [segment.segmentPoints lastObject]) break;
    
    [self tileTriviallyBetween:s and:s.next];
  }
  
  segment.processed = YES;
  
  NSMutableArray *forward = [NSMutableArray array];
  NSMutableArray *backward = [NSMutableArray array];
  
  for (Segment *b in segment.children) {
    if (GLKVector3DotProduct(segment.endNode.direction, segment.startNode.direction) < 0.0f) {
      [backward addObject:b];
    } else {
      [forward addObject:b];
    }
  }
  
  if ([backward count] == 0) {
    [self tileTriviallyBetween:segment.endNode.prev and:segment.endNode];
  } else {
    NSMutableArray *quadrants = [NSMutableArray arrayWithCapacity:4];
    for (NSUInteger i = 0; i < 4; i++) [quadrants addObject:[NSMutableArray array]];
    
    for (Segment *s in backward) {
      for (NSUInteger i = 0; i < 4; i++) {
        if (GLKVector3DotProduct([self rotateUpVector:segment.endNode.prev.up aroundAxis:segment.startNode.normal times:i], s.startNode.direction) > 0 &&
            GLKVector3DotProduct([self rotateUpVector:segment.endNode.prev.up aroundAxis:segment.startNode.normal times:(i + 1) % 4], s.startNode.direction) > 0) {
          [[quadrants objectAtIndex:i] addObject:s];
          break;
        }
      }
    }
    
    [quadrants enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
      [self tileJointForQuadrantVector:GLKVector3Normalize(GLKVector3DivideScalar(GLKVector3Add([self rotateUpVector:segment.endNode.prev.up aroundAxis:segment.endNode.prev.normal times:idx], 
                                                                                                [self rotateUpVector:segment.endNode.prev.up aroundAxis:segment.endNode.prev.normal times:(idx + 1) % 4]), 2.0f))
                         quadrantIndex:idx
                          withSegments:obj
                             andCaller:segment
                      andParentSegment:segment
                            isBackward:YES
                            wasEndNode:YES];
    }];
    
    for (Segment *s in backward) {
      [self tileTree:s];
    }
  }
  
  if ([forward count] > 0) {
    Segment *straightest;
    
    float minAngle = FLT_MAX;
    for (Segment *s in forward) {
      float angle = GLKVector3DotProduct(s.startNode.direction, segment.endNode.direction);
      if (fabsf(1 - angle) < minAngle) {
        minAngle = fabsf(1 - angle);
        straightest = s;
      }
    }
    
    [forward removeObject:straightest];
    
    NSMutableArray *quadrants = [NSMutableArray arrayWithCapacity:4];
    for (NSUInteger i = 0; i < 4; i++) [quadrants addObject:[NSMutableArray array]];
    
    for (Segment *s in forward) {
      for (NSUInteger i = 0; i < 4; i++) {
        if (GLKVector3DotProduct([self rotateUpVector:straightest.startNode.up aroundAxis:segment.startNode.normal times:i], s.startNode.direction) > 0.0f &&
            GLKVector3DotProduct([self rotateUpVector:straightest.startNode.up aroundAxis:segment.startNode.normal times:(i + 1) % 4], s.startNode.direction) > 0.0f) {
          [[quadrants objectAtIndex:i] addObject:s];
          break;
        }
      }
    }
    
    [quadrants enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
      [self tileJointForQuadrantVector:GLKVector3Normalize(GLKVector3DivideScalar(GLKVector3Add([self rotateUpVector:segment.endNode.prev.up aroundAxis:segment.endNode.prev.normal times:idx], 
                                                                                                [self rotateUpVector:segment.endNode.prev.up aroundAxis:segment.endNode.prev.normal times:(idx + 1) % 4]), 2.0f))
                         quadrantIndex:idx
                          withSegments:obj
                             andCaller:straightest
                      andParentSegment:segment
                            isBackward:NO
                            wasEndNode:YES];
    }];
    
    [self tileTree:straightest];
    for (Segment *s in forward) {
      //[self tileTree:s];
    }
  }
}

- (void)tileTriviallyBetween:(SegmentNode *)prev and:(SegmentNode *)cur
{
  NSMutableArray *patches = [NSMutableArray arrayWithCapacity:4];
  
  for (NSUInteger i = 0; i < 4; i++) {
    Patch *p = [Patch new];
    
    [p setVertex:GLKVector3Add(prev.position, GLKVector3MultiplyScalar([self rotateUpVector:prev.up aroundAxis:prev.normal times:i], prev.radius)) atIndex:0];
    [p setVertex:GLKVector3Add(prev.position, GLKVector3MultiplyScalar([self rotateUpVector:prev.up aroundAxis:prev.normal times:(i + 1) % 4], prev.radius)) atIndex:1];
    [p setVertex:GLKVector3Add(cur.position, GLKVector3MultiplyScalar([self rotateUpVector:cur.up aroundAxis:cur.normal times:(i + 1) % 4], cur.radius)) atIndex:2];
    [p setVertex:GLKVector3Add(cur.position, GLKVector3MultiplyScalar([self rotateUpVector:cur.up aroundAxis:cur.normal times:i], cur.radius)) atIndex:3];
    
    [p setRadius:prev.radius atIndex:0];
    [p setRadius:prev.radius atIndex:1];
    [p setRadius:cur.radius atIndex:2];
    [p setRadius:cur.radius atIndex:3];
    
    [p setCenter:prev.position atIndex:0];
    [p setCenter:prev.position atIndex:1];
    [p setCenter:cur.position atIndex:2];
    [p setCenter:cur.position atIndex:3];
    
    [patches addObject:p];
  }
  
  cur.patches = patches;
  
  for (NSUInteger i = 0; i < 4; i++) {
    if ([prev.patches count] > i) {
      [[cur.patches objectAtIndex:i] setNeighbor:[prev.patches objectAtIndex:i] atIndex:1];
      [[prev.patches objectAtIndex:i] setNeighbor:[cur.patches objectAtIndex:i] atIndex:3];
    }
    
    [[cur.patches objectAtIndex:i] setNeighbor:[cur.patches objectAtIndex:(i - 1) % 4] atIndex:0];
    [[cur.patches objectAtIndex:i] setNeighbor:[cur.patches objectAtIndex:(i + 1) % 4] atIndex:2];
  }
  
  [self.generatedObjects addObjectsFromArray:patches];
}

- (void)tileJointForQuadrantVector:(GLKVector3)quadrantVector quadrantIndex:(NSUInteger)idx withSegments:(NSArray *)segments andCaller:(Segment *)caller andParentSegment:(Segment *)parent isBackward:(BOOL)backward wasEndNode:(BOOL)wasEndNode
{
  if ([segments count] == 0) {
    Patch *p = [Patch new];
    
    SegmentNode *prev = parent.startNode;
    if (wasEndNode) prev = parent.endNode;
    
    SegmentNode *cur = caller.startNode.next;
    
    [p setVertex:GLKVector3Add(prev.position, GLKVector3MultiplyScalar([self rotateUpVector:prev.up aroundAxis:prev.normal times:idx], prev.radius)) atIndex:0];
    [p setVertex:GLKVector3Add(prev.position, GLKVector3MultiplyScalar([self rotateUpVector:prev.up aroundAxis:prev.normal times:(idx + 1) % 4], prev.radius)) atIndex:1];
    [p setVertex:GLKVector3Add(cur.position, GLKVector3MultiplyScalar([self rotateUpVector:cur.up aroundAxis:cur.normal times:(idx + 1) % 4], cur.radius)) atIndex:2];
    [p setVertex:GLKVector3Add(cur.position, GLKVector3MultiplyScalar([self rotateUpVector:cur.up aroundAxis:cur.normal times:idx], cur.radius)) atIndex:3];
    
    [p setRadius:prev.radius atIndex:0];
    [p setRadius:prev.radius atIndex:1];
    [p setRadius:cur.radius atIndex:2];
    [p setRadius:cur.radius atIndex:3];
    
    [p setCenter:prev.position atIndex:0];
    [p setCenter:prev.position atIndex:1];
    [p setCenter:cur.position atIndex:2];
    [p setCenter:cur.position atIndex:3];
    
    if ([prev.patches count] == 4) {
      [p setNeighbor:[prev.patches objectAtIndex:idx] atIndex:1];
    }
    
    if ([cur.patches count] > 0) {
      [p setNeighbor:[cur.patches objectAtIndex:[cur.patches count] - 1] atIndex:0];
      [[cur.patches objectAtIndex:[cur.patches count] - 1] setNeighbor:p atIndex:2];
    }
    
    [cur.patches addObject:p];
    
    if ([cur.patches count] == 4) {
      [p setNeighbor:[cur.patches objectAtIndex:0] atIndex:2];
      [[cur.patches objectAtIndex:0] setNeighbor:p atIndex:0];
    }
    
    [self.generatedObjects addObject:p];
  }
}

@end

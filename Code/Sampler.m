//
//  Sampler.m
//  SmoothVesselTree
//

#import "Sampler.h"

#import "Vesseltree.h"

@interface Sampler ()

@property (assign) float alpha;
@property (assign) float beta;

- (float)calculateCurvatureForPoint:(SegmentNode *)point;
- (float)calculateDistanceBetween:(SegmentNode *)firstPoint and:(SegmentNode *)secondPoint;
- (float)calculateWeightForPoint:(SegmentNode *)point;

- (void)sampleVesseltree:(Segment *)segment;
- (void)setAlpha:(float)alpha andBeta:(float)beta;

@end

@implementation Sampler

@synthesize alpha = _alpha;
@synthesize beta = _beta;

+ (void)sampleVesseltree:(Segment *)segment withAlpha:(float)alpha andBeta:(float)beta
{
  if (!segment) return;
  
  Sampler *sampler = [Sampler new];
  [sampler setAlpha:alpha andBeta:beta];
  
  [sampler sampleVesseltree:segment];
}

- (void)setAlpha:(float)alpha andBeta:(float)beta
{
  self.alpha = alpha;
  self.beta = beta;
}

- (void)sampleVesseltree:(Segment *)segment
{
  NSMutableArray *sampledPoints = [NSMutableArray array];
  NSMutableArray *sampledFront = [NSMutableArray array];
  NSMutableArray *sampledBack = [NSMutableArray array];
  
  if ([segment.segmentPoints count] > 0) {
    if (self.alpha > 0 && self.beta > 0) {
      NSUInteger i = 0;
      NSUInteger j = [segment.segmentPoints count] - 1;
      
      while (i < j) {
        float frontDistance, frontWeight;
        float backDistance, backWeight;
        
        SegmentNode *pAtI = [segment.segmentPoints objectAtIndex:i];
        frontDistance = [self calculateDistanceBetween:pAtI.prev and:pAtI];
        frontWeight = [self calculateWeightForPoint:pAtI.prev] + [self calculateWeightForPoint:pAtI];
        
        SegmentNode *pAtj = [segment.segmentPoints objectAtIndex:j];
        backDistance = [self calculateDistanceBetween:pAtj and:pAtj.next];
        backWeight = [self calculateWeightForPoint:pAtj];
        
        // Sample from front
        if (frontDistance >= frontWeight) {
          [sampledFront addObject:pAtI];
          i++;
        } else {
          if (frontWeight > 10000.0f) {
            i++;
          } else {
            int k = 1;
            
            while ((i + k) < j) {
              SegmentNode *pAtk = [segment.segmentPoints objectAtIndex:i + k];
              frontDistance = [self calculateDistanceBetween:pAtI.prev and:pAtk];
              frontWeight = [self calculateWeightForPoint:pAtI.prev] + [self calculateWeightForPoint:pAtk];
              
              if (frontDistance >= frontWeight) {
                [sampledFront addObject:[pAtk.prev combineWithNode:pAtk]];
                break;
              } else if (frontWeight > 10000.0f) {
                break;
              }
              
              k++;
            }
            
            i += k;
          }
        }
        
        // Sample from back
        if (backDistance >= backWeight) {
          [sampledBack addObject:pAtI];
          i++;
        } else {
          if (backWeight > 10000.0f) {
            i++;
          } else {
            int k = 1;
            
            while ((j - k) > i) {
              SegmentNode *pAtk = [segment.segmentPoints objectAtIndex:j - k];
              backDistance = [self calculateDistanceBetween:pAtk and:pAtj.next];
              backWeight = [self calculateWeightForPoint:pAtj.next] + [self calculateWeightForPoint:pAtk];
              
              if (backDistance >= backWeight) {
                [sampledBack addObject:[pAtk combineWithNode:pAtk.next]];
                break;
              } else if (backWeight > 10000.0f) {
                break;
              }
              
              k++;
            }
            
            j -= k;
          }
        }
      }
      
      // Merge the two arrays together
      [sampledPoints addObjectsFromArray:sampledFront];
      [sampledPoints addObjectsFromArray:[[sampledBack reverseObjectEnumerator] allObjects]];
      
      segment.segmentPoints = sampledPoints;
    }
    
    // Recreate linked list
    [segment.segmentPoints enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
      SegmentNode *p = obj;
      
      if (obj != [segment.segmentPoints lastObject]) {
        p.next = [segment.segmentPoints objectAtIndex:(idx + 1)];
      } else {
        p.next = segment.endNode;
      }
      
      if (obj != [segment.segmentPoints objectAtIndex:0]) {
        p.prev = [segment.segmentPoints objectAtIndex:(idx - 1)];
      } else {
        p.prev = segment.startNode;
      }
     
      p.curvature = [self calculateCurvatureForPoint:p];
    }];
    
    segment.startNode.next = [segment.segmentPoints objectAtIndex:0];
    segment.endNode.prev = [segment.segmentPoints lastObject];
  }
  
  // Parse the rest of the tree
  for (Segment *s in segment.children) {
    [self sampleVesseltree:s];
  }
}

- (float)calculateDistanceBetween:(SegmentNode *)firstPoint and:(SegmentNode *)secondPoint
{
  return GLKVector3Length(GLKVector3Subtract(firstPoint.position, secondPoint.position));
}

- (float)calculateWeightForPoint:(SegmentNode *)point
{
  float kappa = [self calculateCurvatureForPoint:point];
  
  return (self.alpha * point.radius) / (1.0f + self.beta * kappa);
}

- (float)calculateCurvatureForPoint:(SegmentNode *)point
{
  float kappa = 0.0f;
  
  if (point.prev && point.next) {
    float a = GLKVector3Length(GLKVector3Subtract(point.position, point.prev.position));
    float b = GLKVector3Length(GLKVector3Subtract(point.next.position, point.position));
    float c = GLKVector3Length(GLKVector3Subtract(point.next.position, point.prev.position));
    float s = 0.5f * (a + b + c);
    
    if ((a * b * c) == 0) return 10000.0f;
    
    float x = fabsf(s * (s - a) * (s - b) * (s - c));
    
    kappa = 4.0f * sqrtf(x) / (a * b * c);
  } else if (point.prev.curvature > 0) {
    kappa = point.prev.curvature;
  }
  
  return 10.0f * kappa;
}

@end

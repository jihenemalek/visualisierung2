//
//  SegmentNode.m
//  SmoothVesselTree
//

#import "SegmentNode.h"

@implementation SegmentNode

@synthesize direction = _direction;
@synthesize next = _next;
@synthesize normal = _normal;
@synthesize offset = _offset;
@synthesize prev = _prev;
@synthesize patches = _patches;
@synthesize position = _position;
@synthesize radius = _radius;
@synthesize up = _up;

- (id)copyWithZone:(NSZone *)zone
{
  SegmentNode *copy = [[[self class] allocWithZone:zone] init];
  
  if (copy) {
    copy.next = self.next;
    copy.direction = self.direction;
    copy.normal = self.normal;
    copy.offset = self.offset;
    copy.next = self.next;
    copy.patches = self.patches;
    copy.position = self.position;
    copy.radius = self.radius;
    copy.up = self.up;
  }
  
  return copy;
}

- (id)init
{
  self = [super init];
  if (self) {
    self.patches = [NSMutableArray arrayWithCapacity:4];
  }
  return self;
}

- (SegmentNode *)combineWithNode:(SegmentNode *)otherNode
{
  SegmentNode *newNode = [SegmentNode new];
  
  newNode.position = GLKVector3DivideScalar(GLKVector3Add(self.position, otherNode.position), 2.0f);
  newNode.radius   = (self.radius + otherNode.radius) / 2.0f;
  newNode.offset   = (self.offset + otherNode.offset) / 2.0f;
  
  return newNode;
}

@end

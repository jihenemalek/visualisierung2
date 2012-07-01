//
//  ControlPoint.m
//  SmoothVesselTree
//

#import "ControlPoint.h"

@implementation ControlPoint

@synthesize identifier = _identifier;

- (id)copyWithZone:(NSZone *)zone
{
  ControlPoint *copy = [super copyWithZone:zone];
  
  if (copy) {
    copy.identifier = self.identifier;
  }
  
  return copy;
}

@end

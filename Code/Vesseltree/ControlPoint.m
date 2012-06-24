//
//  ControlPoint.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
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

//
//  Segment.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "Segment.h"

#import "ControlPoint.h"

@implementation Segment

@synthesize children = _children;
@synthesize endNode = _endNode;
@synthesize parents = _parents;
@synthesize processed = _processed;
@synthesize segmentPoints = _segmentPoints;
@synthesize startNode = _startNode;
@synthesize type = _type;

- (id)init
{
  self = [super init];
  if (self) {
    self.children = [NSMutableArray array];
    self.parents = [NSMutableArray array];
    self.processed = NO;
  }
  return self;
}

@end

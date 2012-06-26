//
//  Segment.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@class ControlPoint;

typedef enum {
  kSegmentTypeTracked,
  kSegmentTypeCentered,
  kSegmentTypeInterpolated
} SegmentType;

@interface Segment : NSObject

@property (copy) ControlPoint   *startNode;
@property (copy) ControlPoint   *endNode;

@property (strong) NSArray        *segmentPoints;

@property (assign) SegmentType    type;

@property (strong) NSMutableArray *parents;
@property (strong) NSMutableArray *children;

@property (assign) bool           processed;

- (NSUInteger)countPoints;

@end

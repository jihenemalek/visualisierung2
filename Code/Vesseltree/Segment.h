//
//  Segment.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import <Foundation/Foundation.h>

@class ControlPoint;

typedef enum : NSUInteger
{
  kSegmentTypeTracked      = 0,
  kSegmentTypeCentered     = 1,
  kSegmentTypeInterpolated = 2
} SegmentType;

@interface Segment : NSObject

@property (copy) ControlPoint   *startNode;
@property (copy) ControlPoint   *endNode;

@property        NSArray        *segmentPoints;

@property        SegmentType    type;

@property        NSMutableArray *parents;
@property        NSMutableArray *children;

@property        bool           processed;

- (NSUInteger)countPoints;

@end

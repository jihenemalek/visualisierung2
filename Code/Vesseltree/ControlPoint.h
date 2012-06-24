//
//  ControlPoint.h
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "SegmentNode.h"

@interface ControlPoint : SegmentNode <NSCopying>

@property (copy) NSNumber *identifier;

@end

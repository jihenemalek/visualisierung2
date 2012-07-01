//
//  ControlPoint.h
//  SmoothVesselTree
//

#import "SegmentNode.h"

@interface ControlPoint : SegmentNode <NSCopying>

@property (copy) NSNumber *identifier;

@end

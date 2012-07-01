//
//  Parser.h
//  SmoothVesselTree
//

#import <Foundation/Foundation.h>

@class Segment;

@interface Parser : NSObject

+ (Segment *)parseDocument:(NSString *)documentName;

@end

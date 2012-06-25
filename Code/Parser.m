//
//  Parser.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "Parser.h"

#import "Vesseltree.h"
#import <libxml/parser.h>

@interface Parser () <NSXMLParserDelegate>

@property (nonatomic, strong) Segment             *currentSegment;
@property (nonatomic, strong) ControlPoint        *currentControlPoint;
@property (nonatomic, strong) SegmentNode         *currentSegmentNode;

@property (nonatomic, assign) BOOL                accumulatingText;
@property (nonatomic, assign) BOOL                processingControlPoint;

@property (nonatomic, strong) NSMutableArray      *accumulatedSegments;
@property (nonatomic, strong) NSMutableDictionary *accumulatedControlPoints;

@property (nonatomic, strong) NSMutableArray      *currentParsedBatch;
@property (nonatomic, strong) NSMutableString     *currentParsedText;

@property (nonatomic, assign) float               currentX;
@property (nonatomic, assign) float               currentY;
@property (nonatomic, assign) float               currentZ;

@end

@implementation Parser

@synthesize accumulatedControlPoints = _accumulatedControlPoints;
@synthesize accumulatedSegments = _accumulatedSegments;
@synthesize accumulatingText = _accumulatingText;
@synthesize currentControlPoint = _currentControlPoint;
@synthesize currentParsedBatch = _currentParsedBatch;
@synthesize currentParsedText = _currentParsedText;
@synthesize currentSegment = _currentSegment;
@synthesize currentSegmentNode = _currentSegmentNode;
@synthesize currentX = _currentX;
@synthesize currentY = _currentY;
@synthesize currentZ = _currentZ;
@synthesize processingControlPoint = _processingControlPoint;

+ (Segment *)parseDocument:(NSString *)documentName
{
  Parser *parser = [Parser new];
  
  return [parser parseDocument:documentName];
}

- (id)init
{
  self = [super init];
  if (self) {
    _accumulatedControlPoints = [NSMutableDictionary dictionary];
    _accumulatedSegments = [NSMutableArray array];
    _currentParsedBatch = [NSMutableArray array];
    _currentParsedText = [NSMutableString string];
    _processingControlPoint = NO;
  }
  return self;
}

- (Segment *)parseDocument:(NSString *)documentName
{
  NSURL *documentURL = [[NSBundle mainBundle] URLForResource:documentName withExtension:@"xml"];
  
  NSXMLParser *parser = [[NSXMLParser alloc] initWithContentsOfURL:documentURL];
  parser.delegate = self;
  [parser parse];
  
  // Create tree structure
  NSMutableArray *parents = [NSMutableArray arrayWithCapacity:[self.accumulatedControlPoints count]];
  NSMutableArray *children = [NSMutableArray arrayWithCapacity:[self.accumulatedControlPoints count]];
  
  for (int i = 0; i < [self.accumulatedControlPoints count]; i++) {
    [children addObject:[NSMutableArray array]];
    [parents addObject:[NSMutableArray array]];
  }
  
  for (Segment *segment in self.accumulatedSegments) {
    [[children objectAtIndex:([segment.startNode.identifier integerValue] - 1)] addObject:segment];
    [[parents objectAtIndex:([segment.endNode.identifier integerValue] - 1)] addObject:segment];
  }
  
  [parents enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
    for (Segment *parent in obj) {
      for (Segment *child in [children objectAtIndex:idx]) {
        [child.parents addObject:parent];
        [parent.children addObject:child];
      }
    }
  }];
  
  return [self.accumulatedSegments objectAtIndex:0];
}

#pragma mark - XML Attribute strings

static NSString * const kSegmentName = @"segment";
static NSString * const kSegmentStartNodeName = @"BeginCPID";
static NSString * const kSegmentEndNodeName = @"EndCPID";
static NSString * const kSegmentTypeName = @"type";

static NSString * const kSegmentNodeName = @"p";
static NSString * const kSegmentNodeOffsetName = @"offset";
static NSString * const kSegmentNodeRadiusName = @"r1";
static NSString * const kSegmentNodeXName = @"x";
static NSString * const kSegmentNodeYName = @"y";
static NSString * const kSegmentNodeZName = @"z";

static NSString * const kControlPointName = @"cp";
static NSString * const kControlPointIdName = @"id";

#pragma mark - NSXMLParserDelegate methods

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
  if ([elementName isEqualToString:kControlPointName]) 
  {
    self.currentControlPoint = [ControlPoint new];
    self.processingControlPoint = YES;
  } 
  else if ([elementName isEqualToString:kSegmentName]) 
  {
    self.currentSegment = [Segment new];
    
    if ([[attributeDict objectForKey:@"type"] isEqualToString:@"centered"]) {
      self.currentSegment.type = kSegmentTypeCentered;
    } else if ([[attributeDict objectForKey:@"type"] isEqualToString:@"interpolated"]) {
      self.currentSegment.type = kSegmentTypeInterpolated;
    } else if ([[attributeDict objectForKey:@"type"] isEqualToString:@"tracked"]) {
      self.currentSegment.type = kSegmentTypeTracked;
    }
    
    self.currentParsedBatch = [NSMutableArray array];
  } 
  else if ([elementName isEqualToString:kSegmentNodeName]) 
  {
    self.currentSegmentNode = [SegmentNode new];
  } 
  else if ([elementName isEqualToString:kSegmentNodeOffsetName] ||
             [elementName isEqualToString:kSegmentNodeRadiusName] ||
             [elementName isEqualToString:kSegmentNodeXName] ||
             [elementName isEqualToString:kSegmentNodeYName] ||
             [elementName isEqualToString:kSegmentNodeZName] ||
             [elementName isEqualToString:kSegmentStartNodeName] ||
             [elementName isEqualToString:kSegmentEndNodeName] ||
             [elementName isEqualToString:kControlPointIdName]) 
  {
    self.accumulatingText = YES;
    // Reset the text
    self.currentParsedText.string = @"";
  }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
  if ([elementName isEqualToString:kControlPointName])
  {
    self.currentControlPoint.position = GLKVector3Make(self.currentX, self.currentY, self.currentZ);
    
    [self.accumulatedControlPoints setObject:self.currentControlPoint forKey:self.currentControlPoint.identifier];
    
    self.processingControlPoint = NO;
  }
  else if ([elementName isEqualToString:kSegmentName])
  {
    self.currentSegment.segmentPoints = self.currentParsedBatch;
    
    // Create a linked list for the segment points
    if ([self.currentSegment.segmentPoints count] == 0) {
      self.currentSegment.startNode.next = self.currentSegment.endNode;
      self.currentSegment.endNode.prev = self.currentSegment.startNode;
    } else {
      self.currentSegment.startNode.next = [self.currentSegment.segmentPoints objectAtIndex:0];
      [[self.currentSegment.segmentPoints objectAtIndex:0] setPrev:self.currentSegment.startNode];
      for (NSUInteger i = 0; i < [self.currentSegment.segmentPoints count]; i++) {
        if (i > 0) {
          [[self.currentSegment.segmentPoints objectAtIndex:i] setPrev:[self.currentSegment.segmentPoints objectAtIndex:i - 1]];
        }
        
        if (i < [self.currentSegment.segmentPoints count] - 1) {
          [[self.currentSegment.segmentPoints objectAtIndex:i] setNext:[self.currentSegment.segmentPoints objectAtIndex:i + 1]];
        }
      }
      [[self.currentSegment.segmentPoints lastObject] setNext:self.currentSegment.endNode];
      self.currentSegment.endNode.prev = [self.currentSegment.segmentPoints lastObject];
    }
    
    [self.accumulatedSegments addObject:self.currentSegment];
  } 
  else if ([elementName isEqualToString:kSegmentNodeName])
  {
    self.currentSegmentNode.position = GLKVector3Make(self.currentX, self.currentY, self.currentZ);
    
    [self.currentParsedBatch addObject:self.currentSegmentNode];
  }
  
  // Text processing
  else if ([elementName isEqualToString:kControlPointIdName])
  {
    self.currentControlPoint.identifier = [NSNumber numberWithInteger:[self.currentParsedText integerValue]];
  }
  else if ([elementName isEqualToString:kSegmentNodeXName])
  {
    self.currentX = [self.currentParsedText floatValue];
  }
  else if ([elementName isEqualToString:kSegmentNodeYName])
  {
    self.currentY = [self.currentParsedText floatValue];
  }
  else if ([elementName isEqualToString:kSegmentNodeZName])
  {
    self.currentZ = [self.currentParsedText floatValue];
  }
  else if ([elementName isEqualToString:kSegmentNodeRadiusName])
  {
    if (self.processingControlPoint)
    {
      self.currentControlPoint.radius = [self.currentParsedText floatValue];
    } else {
      self.currentSegmentNode.radius = [self.currentParsedText floatValue];
    }
  }
  else if ([elementName isEqualToString:kSegmentNodeOffsetName])
  {
    if (self.processingControlPoint)
    {
      self.currentControlPoint.offset = [self.currentParsedText floatValue];
    } else {
      self.currentSegmentNode.offset = [self.currentParsedText floatValue];
    }
  }
  else if ([elementName isEqualToString:kSegmentStartNodeName]) {
    self.currentSegment.startNode = [self.accumulatedControlPoints objectForKey:[NSNumber numberWithInteger:[self.currentParsedText integerValue]]];
  }
  else if ([elementName isEqualToString:kSegmentEndNodeName]) {
    self.currentSegment.endNode = [self.accumulatedControlPoints objectForKey:[NSNumber numberWithInteger:[self.currentParsedText integerValue]]];
  }
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
  if (self.accumulatingText) {
    [self.currentParsedText appendString:string];
  }
}

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError {
  NSLog(@"Parser Error: %@, %@", parseError, [parseError localizedDescription]);
}

@end
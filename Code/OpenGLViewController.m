//
//  OpenGLViewController.m
//  SmoothVesselTree
//
//  Created by Markus Mühlberger on 23.06.12.
//  Copyright (c) 2012 OneBox Media Solutions GmbH. All rights reserved.
//

#import "OpenGLViewController.h"

#import "Mesh.h"
#import "Mesher.h"
#import "Parser.h"
#import "Sampler.h"
#import "Vesseltree.h"

enum {
  UNIFORM_MODELVIEWPROJECTION_MATRIX,
  UNIFORM_NORMAL_MATRIX,
  NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

enum {
  ATTRIB_VERTEX,
  ATTRIB_NORMAL,
  ATTRIB_COLOR,
  NUM_ATTRIBS
};

typedef enum : NSUInteger {
  kViewModeWireframe = 0,
  kViewModeSolid = 1,
  kViewModeBoth = 2,
  kViewModeTree = 3
} ViewMode;

typedef struct {
  float Position[3];
  float Normal[3];
  float Color[4];
} Vertex;

@interface OpenGLViewController ()

@property (weak, nonatomic) IBOutlet UILabel *fileLabel;
@property (weak, nonatomic) IBOutlet UILabel *loadingLabel;

@property (nonatomic, strong) EAGLContext *context;

@property (nonatomic, strong) NSArray *triangles;

- (void)setupGL;
- (void)tearDownGL;
- (BOOL)loadShadersWithName:(NSString *)name andProgram:(GLuint *)program;

- (void)loadVesseltree:(NSString *)name;

// UI Interaction
- (IBAction)subdivide0ButtonTapped:(id)sender;
- (IBAction)subdivide1ButtonTapped:(id)sender;
- (IBAction)subdivide2ButtonTapped:(id)sender;

- (IBAction)vesselTree0ButtonTapped:(id)sender;
- (IBAction)vesselTree1ButtonTapped:(id)sender;
- (IBAction)vesselTree2ButtonTapped:(id)sender;
- (IBAction)vesselTree3ButtonTapped:(id)sender;
- (IBAction)vesselTree4ButtonTapped:(id)sender;
- (IBAction)vesselTree5ButtonTapped:(id)sender;

- (IBAction)samplingButtonTapped:(id)sender;
- (IBAction)viewModeButtonTapped:(id)sender;

@property (weak, nonatomic) IBOutlet UIButton *samplingButton;
@property (weak, nonatomic) IBOutlet UILabel *samplingLabel;

@end

@implementation OpenGLViewController {
  GLuint _solidProgram;
  GLuint _wireframeProgram;
  
  GLKMatrix4 _mvpMatrix;
  GLKMatrix3 _normalMatrix;
  
  GLuint _solidIndexBuffer;
  GLuint _solidVertexArray;
  GLuint _vertexBuffer;
  GLuint _treeVertexBuffer;
  GLuint _wireframeIndexBuffer;
  GLuint _wireframeVertexArray;

  GLuint _nodeCount;
  GLuint _indexCount;
  
  GLKVector3 _cameraPosition;
  GLKVector3 _lookAtCenter;
  
  ViewMode _currentViewMode;
  
  BOOL _sampling;
  NSUInteger _quadSubdivisioning;
}
@synthesize samplingButton = _samplingButton;
@synthesize samplingLabel = _samplingLabel;

@synthesize fileLabel = _fileLabel;
@synthesize loadingLabel = _loadingLabel;
@synthesize context = _context;
@synthesize triangles = _triangles;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
  self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
  if (self) {
    // Custom initialization
  }
  return self;
}

- (void)viewDidLoad
{
  [super viewDidLoad];
  
  _currentViewMode = kViewModeBoth;
  _sampling = YES;
  
  self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  
  if (!self.context) {
    NSLog(@"Failed to create OpenGL ES 2.0 context");
  }
  
  GLKView *view = (GLKView *)self.view;
  view.context = self.context;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
  
  [self setupGL];
  
  [self loadVesseltree:@"vesselTree5"];
}

- (void)viewDidUnload
{
  [self setFileLabel:nil];
  [self setLoadingLabel:nil];
  [self setSamplingLabel:nil];
  [self setSamplingButton:nil];
  [super viewDidUnload];
  
  [self tearDownGL];
  
  if ([EAGLContext currentContext] == self.context) {
    [EAGLContext setCurrentContext:nil];
  }
  
  self.context = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma mark - Setup and tear down

- (void)setupGL
{
  [self handleTap:nil];
  
  [EAGLContext setCurrentContext:self.context];
  
  [self loadShadersWithName:@"solid" andProgram:&_solidProgram];
  [self loadShadersWithName:@"wireframe" andProgram:&_wireframeProgram];
  
  glEnable(GL_DEPTH_TEST);
}

- (void)tearDownGL
{
  [EAGLContext setCurrentContext:self.context];

  glDeleteBuffers(1, &_solidIndexBuffer);
  glDeleteBuffers(1, &_wireframeIndexBuffer);
  glDeleteBuffers(1, &_vertexBuffer);

  glDeleteVertexArraysOES(1, &_solidVertexArray);
  glDeleteVertexArraysOES(1, &_wireframeVertexArray);

  if (_solidProgram) {
    glDeleteProgram(_solidProgram);
    _solidProgram = 0;
  }

  if (_wireframeProgram) {
    glDeleteProgram(_wireframeProgram);
    _wireframeProgram = 0;
  }
}

- (BOOL)loadShadersWithName:(NSString *)shaderName andProgram:(GLuint *)program
{
  *program = glCreateProgram();

  NSString *vertShaderPathname = [[NSBundle mainBundle] pathForResource:shaderName ofType:@"vsh"];
  NSString *fragShaderPathname = [[NSBundle mainBundle] pathForResource:shaderName ofType:@"fsh"];

  GLuint vertShader;
  if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname]) {
    NSLog(@"Failed to compile vertex shader");
    return NO;
  }

  GLuint fragShader;
  if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname]) {
    NSLog(@"Failed to compile fragment shader");
    return NO;
  }

  glAttachShader(*program, vertShader);
  glAttachShader(*program, fragShader);

  glBindAttribLocation(*program, GLKVertexAttribPosition, "position");
  glBindAttribLocation(*program, GLKVertexAttribNormal, "normal");
  glBindAttribLocation(*program, GLKVertexAttribColor, "color");

  if (![self linkProgram:*program]) {
    NSLog(@"Failed to link program: %d", *program);
    
    if (vertShader) {
      glDeleteShader(vertShader);
      vertShader = 0;
    }
    
    if (fragShader) {
      glDeleteShader(fragShader);
      fragShader = 0;
    }
    
    if (*program) {
      glDeleteProgram(*program);
      *program = 0;
    }
    
    return NO;
  }

  uniforms[UNIFORM_MODELVIEWPROJECTION_MATRIX] = glGetUniformLocation(*program, "modelViewProjectionMatrix");
  uniforms[UNIFORM_NORMAL_MATRIX] = glGetUniformLocation(*program, "normalMatrix");

  if (vertShader) {
    glDetachShader(*program, vertShader);
    glDeleteShader(vertShader);
  }

  if (fragShader) {
    glDetachShader(*program, fragShader);
    glDeleteShader(fragShader);
  }

  return YES;
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
  const GLchar *source = [[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
  if (!source) {
    NSLog(@"Failed to load shader source (%@)", file);
    return NO;
  }

  *shader = glCreateShader(type);
  glShaderSource(*shader, 1, &source, NULL);
  glCompileShader(*shader);

  #if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      GLchar *log = (GLchar *)malloc(logLength);
      glGetShaderInfoLog(*shader, logLength, &logLength, log);
      NSLog(@"Shader compile log:\n%s", log);
      free(log);
    }
  #endif

  GLint status;
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
  if (status == 0) {
    glDeleteShader(*shader);
    return NO;
  }

  return YES;
}

- (BOOL)linkProgram:(GLuint)prog
{
  GLint status;
  glLinkProgram(prog);

  #if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      GLchar *log = (GLchar *)malloc(logLength);
      glGetProgramInfoLog(prog, logLength, &logLength, log);
      NSLog(@"Program link log:\n%s", log);
      free(log);
    }
  #endif

  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (status == 0) {
    return NO;
  }

  return YES;
  }

  - (BOOL)validateProgram:(GLuint)prog
  {
  GLint logLength, status;

  glValidateProgram(prog);
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    GLchar *log = (GLchar *)malloc(logLength);
    glGetProgramInfoLog(prog, logLength, &logLength, log);
    NSLog(@"Program validate log:\n%s", log);
    free(log);
  }

  glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
  if (status == 0) {
    return NO;
  }

  return YES;
}

#pragma mark - Render cycle

- (void)update
{
  float aspect = fabsf(self.view.bounds.size.width / self.view.bounds.size.height);
  GLKMatrix4 projectionMatrix = GLKMatrix4MakePerspective(GLKMathDegreesToRadians(65.0f), aspect, 0.1f, 10000.0f);

  GLKMatrix4 modelViewMatrix = GLKMatrix4MakeLookAt(_cameraPosition.x, _cameraPosition.y, _cameraPosition.z, _lookAtCenter.x, _lookAtCenter.y, _lookAtCenter.z, 0, 1, 0);
    
  _normalMatrix = GLKMatrix3InvertAndTranspose(GLKMatrix4GetMatrix3(modelViewMatrix), NULL);
  _mvpMatrix = GLKMatrix4Multiply(projectionMatrix, modelViewMatrix);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (_currentViewMode == kViewModeBoth || _currentViewMode == kViewModeSolid) {
    if (_currentViewMode == kViewModeBoth) {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0.75, 1.0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _solidIndexBuffer);
    
    glBindVertexArrayOES(_solidVertexArray);
    
    glUseProgram(_solidProgram);
    
    glUniformMatrix4fv(uniforms[UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, _mvpMatrix.m);
    glUniformMatrix3fv(uniforms[UNIFORM_NORMAL_MATRIX], 1, 0, _normalMatrix.m);
    
    glDrawArrays(GL_TRIANGLES, 0, _indexCount);
    
    if (_currentViewMode == kViewModeBoth) {
      glDisable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0.0, 0.0);
    }
  }
  
  if (_currentViewMode == kViewModeBoth || _currentViewMode == kViewModeWireframe) {
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _solidIndexBuffer);
    
    glBindVertexArrayOES(_solidVertexArray);
    
    glUseProgram(_wireframeProgram);
    
    glUniformMatrix4fv(uniforms[UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, _mvpMatrix.m);
    glUniformMatrix3fv(uniforms[UNIFORM_NORMAL_MATRIX], 1, 0, _normalMatrix.m);
    
    if (_currentViewMode != kViewModeBoth) {
      glDisable(GL_DEPTH_TEST);
    }
    
    for (NSUInteger i = 0; i < _indexCount / 6; i++) {
      glDrawArrays(GL_LINE_LOOP, 6 * i, 6);
    }
    
    glCullFace(GL_NONE);    
    glEnable(GL_DEPTH_TEST);
  }
  
  if (_currentViewMode == kViewModeTree) {
    glBindBuffer(GL_ARRAY_BUFFER, _treeVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _wireframeIndexBuffer);
    
    glBindVertexArrayOES(_wireframeVertexArray);
    
    glUseProgram(_wireframeProgram);
    
    glUniformMatrix4fv(uniforms[UNIFORM_MODELVIEWPROJECTION_MATRIX], 1, 0, _mvpMatrix.m);
    glUniformMatrix3fv(uniforms[UNIFORM_NORMAL_MATRIX], 1, 0, _normalMatrix.m);
    
    if (_currentViewMode != kViewModeBoth) {
      glDisable(GL_DEPTH_TEST);
    }
    
    glDrawArrays(GL_POINTS, 0, _nodeCount);
    
    glDrawArrays(GL_LINES, 0, _nodeCount);
    
    glCullFace(GL_NONE);    
    glEnable(GL_DEPTH_TEST);
  }
}

#pragma mark - Gesture recognizers

- (IBAction)handleRotation:(UIPanGestureRecognizer *)recognizer
{
  CGPoint t = [recognizer velocityInView:self.view];
  
  GLKQuaternion rotateX = GLKQuaternionMakeWithAngleAndAxis(0.0005 * t.x, 0, 1, 0);
  GLKQuaternion rotateZ = GLKQuaternionMakeWithAngleAndAxis(0.0005 * t.y, 0, 0, 1);
  
  _cameraPosition = GLKVector3Add(_lookAtCenter, GLKQuaternionRotateVector3(rotateX, GLKVector3Subtract(_cameraPosition, _lookAtCenter)));
  _cameraPosition = GLKVector3Add(_lookAtCenter, GLKQuaternionRotateVector3(rotateZ, GLKVector3Subtract(_cameraPosition, _lookAtCenter)));
  //  _cameraPosition = GLKVector3Add(_cameraPosition, GLKVector3MultiplyScalar(GLKVector3Make(1, 0, 0), 0.01 * (1024.0f/768.0f) * velocity.x));
}

- (IBAction)handlePan:(UIPanGestureRecognizer *)recognizer
{
  CGPoint velocity = [recognizer velocityInView:self.view];
  
  GLKVector3 rightVector = GLKVector3Normalize(GLKVector3CrossProduct(GLKVector3Make(0, 1, 0), GLKVector3Subtract(_cameraPosition, _lookAtCenter)));
  GLKVector3 upVector = GLKVector3Normalize(GLKVector3CrossProduct(GLKVector3Subtract(_cameraPosition, _lookAtCenter), rightVector));
  
  _cameraPosition = GLKVector3Add(_cameraPosition, GLKVector3MultiplyScalar(upVector, 0.001 * velocity.y));
  _cameraPosition = GLKVector3Add(_cameraPosition, GLKVector3MultiplyScalar(rightVector, -0.001 * (1024.0f/768.0f) * velocity.x));
  
  _lookAtCenter = GLKVector3Add(_lookAtCenter, GLKVector3MultiplyScalar(upVector, 0.001 * velocity.y));
  _lookAtCenter = GLKVector3Add(_lookAtCenter, GLKVector3MultiplyScalar(rightVector, -0.001 * (1024.0f/768.0f) * velocity.x));
}

- (IBAction)handlePinch:(UIPinchGestureRecognizer *)recognizer
{
  float scale = recognizer.scale;
  
  if (recognizer.scale < 1)
  {
    scale = - 1.0f / recognizer.scale;
  }
  
  GLKVector3 moveVector = GLKVector3MultiplyScalar(GLKVector3Subtract(_cameraPosition, _lookAtCenter), 0.1f * scale);
  
  _cameraPosition = GLKVector3Subtract(_cameraPosition, moveVector);
  _lookAtCenter = GLKVector3Subtract(_lookAtCenter, moveVector);
}

- (IBAction)handleTap:(UITapGestureRecognizer *)recognizer
{
  _cameraPosition = GLKVector3Make(0, 0, -100);
  _lookAtCenter = GLKVector3Make(0, 0, 0);
}

- (IBAction)handleZoomIn:(UITapGestureRecognizer *)recognizer
{
  if (GLKVector3Length(GLKVector3Subtract(_cameraPosition, _lookAtCenter)) < 60.0f) return;
  
  _cameraPosition = GLKVector3Subtract(_cameraPosition, GLKVector3MultiplyScalar(GLKVector3Normalize(GLKVector3Subtract(_cameraPosition, _lookAtCenter)), 50.0f));
}

- (IBAction)handleZoomOut:(UITapGestureRecognizer *)recognizer
{
  _cameraPosition = GLKVector3Add(_cameraPosition, GLKVector3MultiplyScalar(GLKVector3Normalize(GLKVector3Subtract(_cameraPosition, _lookAtCenter)), 50.0f));
}

# pragma mark - Vesseltree loading

- (void)loadVesseltree:(NSString *)name
{
  if ([self.fileLabel.text isEqualToString:name]) return;
  
  self.fileLabel.text = name;
  
  [self.view setUserInteractionEnabled:NO];
  [self.loadingLabel setHidden:NO];
  
  if ([name isEqualToString:@"vesselTree5"]) {
    [self.samplingButton setHidden:YES];
    [self.samplingLabel setHidden:YES];
  } else {
    [self.samplingButton setHidden:NO];
    [self.samplingLabel setHidden:NO];
  }
  
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    // Read the vessel tree from file 
    Segment *root = [Parser parseDocument:name];
    
    // Sample the vessel tree
    if (_sampling && ![name isEqualToString:@"vesselTree5"]) {
      [Sampler sampleVesseltree:root withAlpha:0.5f andBeta:2.0f];
    }
    
    // Generate base mesh
    NSArray *baseMesh = [Mesher generateQuadrilateralMesh:root];
    
    // Divide base mesh to round it
    NSArray *dividedMesh = [Mesher subdivideQuadrilateralMesh:baseMesh times:_quadSubdivisioning];
    
    // Do adaptive subdivision
    // TODO: Adaptive subdivision
    
    // Triangulate base mesh
    self.triangles = [Mesher triangulateQuadrilateralMesh:dividedMesh];
    _indexCount = 3 * [self.triangles count];
    
    dispatch_async(dispatch_get_main_queue(), ^{
      // Delete old buffers
      glDeleteBuffers(1, &_solidIndexBuffer);
      glDeleteBuffers(1, &_wireframeIndexBuffer);
      glDeleteBuffers(1, &_vertexBuffer);
      
      glDeleteVertexArraysOES(1, &_solidVertexArray);
      glDeleteVertexArraysOES(1, &_wireframeVertexArray);
      
      //
      // SOLID
      //
      glGenVertexArraysOES(1, &_solidVertexArray);
      glBindVertexArrayOES(_solidVertexArray);
      
      // Create vertex buffer
      glGenBuffers(1, &_vertexBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
      
      Vertex *vertices = (Vertex *)malloc(_indexCount * sizeof(Vertex));
      for (int i = 0; i < _indexCount; i++) {
        int idx = floorf((float)i / 3.0f);
        Triangle *t = [self.triangles objectAtIndex:idx];
        
        for (int j = 0; j < 3; j++) {
          vertices[i].Position[j] = [t vertexAtIndex:(i % 3)].v[j];
          vertices[i].Normal[j]   = [t normalAtIndex:(i % 3)].v[j];
          vertices[i].Color[j]    = [t curvatureAtIndex:(i % 3)];
        }
        
        UIColor *color = [UIColor colorWithHue:[t curvatureAtIndex:(i % 3)] saturation:1 brightness:1 alpha:1];
        
        [color getRed:&vertices[i].Color[0] green:&vertices[i].Color[1] blue:&vertices[i].Color[2] alpha:&vertices[i].Color[3]];
      }
      glBufferData(GL_ARRAY_BUFFER, _indexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
      
      // Create index buffer
      glGenBuffers(1, &_solidIndexBuffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _solidIndexBuffer);
      GLubyte *solidIndices = (GLubyte *)malloc(_indexCount * sizeof(GLubyte));
      for (int i = 0; i < _indexCount; i++) {
        solidIndices[i] = i;
      }
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexCount * sizeof(GLubyte), solidIndices, GL_STATIC_DRAW);
      
      // Set attribute pointers
      glEnableVertexAttribArray(GLKVertexAttribPosition);
      glVertexAttribPointer(GLKVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, Position));
      glEnableVertexAttribArray(GLKVertexAttribNormal);
      glVertexAttribPointer(GLKVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, Normal));
      glEnableVertexAttribArray(GLKVertexAttribColor);
      glVertexAttribPointer(GLKVertexAttribColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, Color));
      
      glBindVertexArrayOES(0);
      
      //
      // WIREFRAME
      //
      glGenVertexArraysOES(1, &_wireframeVertexArray);
      glBindVertexArrayOES(_wireframeVertexArray);
      
      // Bind general purpose vertex buffer
      glGenBuffers(1, &_treeVertexBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, _treeVertexBuffer);
      
      _nodeCount = [root countPoints];
      Vertex *treeVertices = (Vertex *)malloc(3 * _nodeCount * sizeof(Vertex));
      NSMutableArray *currentSegmentList = [NSMutableArray array];
      Segment *currentSegment = root;
      SegmentNode *node = root.startNode;
      NSUInteger i;
      for (i = 0; i < _nodeCount; i++) {
        if (i == 0 || !node.prev) {
          treeVertices[2 * i + 0].Position[0] = node.position.x;
          treeVertices[2 * i + 0].Position[1] = node.position.y;
          treeVertices[2 * i + 0].Position[2] = node.position.z;
        } else if (!node.next) {
          treeVertices[2 * i - 1].Position[0] = node.position.x;
          treeVertices[2 * i - 1].Position[1] = node.position.y;
          treeVertices[2 * i - 1].Position[2] = node.position.z;
          i--;
        } else {
          treeVertices[2 * i - 1].Position[0] = node.position.x;
          treeVertices[2 * i - 1].Position[1] = node.position.y;
          treeVertices[2 * i - 1].Position[2] = node.position.z;
          treeVertices[2 * i].Position[0] = node.position.x;
          treeVertices[2 * i].Position[1] = node.position.y;
          treeVertices[2 * i].Position[2] = node.position.z;
        }
        
        if (node.next) {
          node = node.next;
        } else {
          [currentSegmentList addObjectsFromArray:currentSegment.children];
          if ([currentSegmentList count] > 0) {
            currentSegment = [currentSegmentList objectAtIndex:0];
            [currentSegmentList removeObject:currentSegment];
            node = currentSegment.startNode;
          } else {
            break;
          }
        }
      }
      _nodeCount = 2 * i + 2;
      glBufferData(GL_ARRAY_BUFFER, _nodeCount * sizeof(Vertex), treeVertices, GL_STATIC_DRAW);
      
      // Create index buffer
      glGenBuffers(1, &_wireframeIndexBuffer);
      
      // Set attribute pointers
      glEnableVertexAttribArray(GLKVertexAttribPosition);
      glVertexAttribPointer(GLKVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)offsetof(Vertex, Position));
      
      glBindVertexArrayOES(0);
      
      // Free malloc'd arrays
      free(solidIndices);
      
      [self.loadingLabel setHidden:YES];
      [self.view setUserInteractionEnabled:YES];
    });
  });
}

- (IBAction)subdivide0ButtonTapped:(id)sender {
  _quadSubdivisioning = 0;
  
  [self loadVesseltree:self.fileLabel.text];
}

- (IBAction)subdivide1ButtonTapped:(id)sender {
  _quadSubdivisioning = 1;
  
  [self loadVesseltree:self.fileLabel.text];
}

- (IBAction)subdivide2ButtonTapped:(id)sender {
  _quadSubdivisioning = 2;
  
  [self loadVesseltree:self.fileLabel.text];
}

- (IBAction)vesselTree0ButtonTapped:(id)sender {
  [self loadVesseltree:@"vesselTree0"];
}

- (IBAction)vesselTree1ButtonTapped:(id)sender {
  [self loadVesseltree:@"vesselTree1"];
}

- (IBAction)vesselTree2ButtonTapped:(id)sender {
  [self loadVesseltree:@"vesselTree2"];
}

- (IBAction)vesselTree3ButtonTapped:(id)sender {
  [self loadVesseltree:@"vesselTree3"];
}

- (IBAction)vesselTree4ButtonTapped:(id)sender {
  [self loadVesseltree:@"vesselTree4"];  
}

- (IBAction)vesselTree5ButtonTapped:(id)sender {
  [self loadVesseltree:@"vesselTree5"];
}

- (IBAction)samplingButtonTapped:(id)sender {
  _sampling = !_sampling;
  
  if (_sampling) [sender setImage:[UIImage imageNamed:@"samplingOn.png"] forState:UIControlStateNormal];
  else [sender setImage:[UIImage imageNamed:@"samplingOff.png"] forState:UIControlStateNormal];
  
  [self loadVesseltree:self.fileLabel.text];
}

- (IBAction)viewModeButtonTapped:(id)sender {
  _currentViewMode = [sender tag];
}
@end
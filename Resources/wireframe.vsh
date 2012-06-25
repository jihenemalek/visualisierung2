//
//  Shader.vsh
//  Vesseltrees
//
//  Created by Markus Mühlberger on 21.06.12.
//  Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

attribute vec3 position;
attribute vec3 normal;
attribute vec4 color;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    vec3 eyeNormal = normalize(normalMatrix * normal);
    vec3 lightPosition = vec3(0.0, 0.0, 1000.0);
    
    gl_PointSize = 5.0;
    gl_Position = modelViewProjectionMatrix * vec4(position, 1);
}

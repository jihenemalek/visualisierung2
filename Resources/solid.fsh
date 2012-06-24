//
//  Shader.fsh
//  Vesseltrees
//
//  Created by Markus Mühlberger on 21.06.12.
//  Copyright (c) 2012 Markus Mühlberger. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}

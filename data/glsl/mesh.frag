/*
 *  mesh.frag
 *
 *  Generic shader for meshes.
 *
 *  Copyright (C) 2002-2011, Davorin Učakar <davorin.ucakar@gmail.com>
 *  This software is covered by GNU GPLv3. See COPYING file for details.
 */

varying vec2  exTexCoord;
varying vec4  exColour;
varying float exDistance;

void main()
{
  if( exDistance >= oz_Fog.end ) {
    discard;
  }

  gl_FragColor = exColour;
  gl_FragColor *= texture2D( oz_Textures[0], exTexCoord * oz_TextureScales[0] );
  gl_FragColor *= oz_Colour;
  gl_FragColor = applyFog( gl_FragColor, exDistance );
}
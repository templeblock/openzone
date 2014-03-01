/*
 * OpenZone - simple cross-platform FPS/RTS game engine.
 *
 * Copyright © 2002-2013 Davorin Učakar
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * dmesh.vert
 *
 * Mesh shader that reads and interpolates vertex positions from the given vertex texture.
 */

precision mediump float;

uniform mat4      oz_ProjCamera;
uniform mat4      oz_Model;
uniform vec3      oz_CameraPos;
#ifdef OZ_VERTEX_TEXTURE
uniform vec3      oz_MeshAnimation;
#endif
uniform sampler2D oz_VertexAnim;

attribute vec3 inPosition;
attribute vec2 inTexCoord;
attribute vec3 inNormal;
#ifdef OZ_BUMP_MAP
attribute vec3 inTangent;
attribute vec3 inBinormal;
#endif

varying vec2 exTexCoord;
varying vec3 exNormal;
#ifdef OZ_BUMP_MAP
varying vec3 exTangent;
varying vec3 exBinormal;
#endif
varying vec3 exLook;

void main()
{
#ifdef OZ_VERTEX_TEXTURE

  float iVertex       = inPosition.x;
  float iPosition0    = oz_MeshAnimation[0] * 0.5;
  float iPosition1    = oz_MeshAnimation[1] * 0.5;
  float iNormal0      = 0.5 + oz_MeshAnimation[0] * 0.5;
  float iNormal1      = 0.5 + oz_MeshAnimation[1] * 0.5;
  float interpolation = oz_MeshAnimation[2];

  vec4  position0     = texture2D( oz_VertexAnim, vec2( iVertex, iPosition0 ) );
  vec4  position1     = texture2D( oz_VertexAnim, vec2( iVertex, iPosition1 ) );
  vec4  normal0       = texture2D( oz_VertexAnim, vec2( iVertex, iNormal0 ) );
  vec4  normal1       = texture2D( oz_VertexAnim, vec2( iVertex, iNormal1 ) );
  vec4  position      = oz_Model * mix( position0, position1, interpolation );
  vec4  normal        = oz_Model * normalize( mix( normal0, normal1, interpolation ) );

#else

  vec4 position = oz_Model * vec4( inPosition, 1.0 );
  vec4 normal   = oz_Model * vec4( inNormal, 0.0 );

#endif
#ifdef OZ_BUMP_MAP
  vec4 tangent  = oz_Model * vec4( inTangent, 0.0 );
  vec4 binormal = oz_Model * vec4( inBinormal, 0.0 );
#endif

  exTexCoord  = inTexCoord;
  exNormal    = normal.xyz;
#ifdef OZ_BUMP_MAP
  exTangent   = tangent.xyz;
  exBinormal  = binormal.xyz;
#endif
  exLook      = position.xyz - oz_CameraPos;
  gl_Position = oz_ProjCamera * position;
}

//
// Bui Tuong Phong shading model (per-pixel) 

varying vec3 vNormal;
varying vec3 vPos;
varying vec4 vColor;

void main()
{	
	// vertex normal
	vNormal = normalize(gl_NormalMatrix * gl_Normal);
	
	// vertex position
	vPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	// Vertex color (used for the ambient and diffuse terms)
	vColor = gl_Color;

	// Texture coordinate
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// vertex position
	gl_Position = ftransform();
}
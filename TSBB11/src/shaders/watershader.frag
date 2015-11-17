#version 150

in vec3 out_Normal;
in vec2 out_TexCoord;
in vec3 out_ObjPos;

out vec4 out_Color;

uniform float t;
uniform vec3 camPos;	// Kamernapositionen.
uniform vec3 lightSourcePosAir;	// Ljuspositionen.
uniform vec3 lightSourcePosWater;	// Ljuspositionen.
uniform int isDirectional;
uniform float specularExponent;
uniform vec3 lightSourceColor;
uniform sampler2D texUnit;

uniform int texCoordScaleH;
uniform int texCoordScaleW;

vec3 r;
vec3 s;				// Infallande ljus.
vec3 eye;			// Vektor fr�n objektet till kameran.


// Phong-modellen:
float kamb;
float kdiff;
float kspec;
vec3 ambLight;		// Ambient.
vec3 diffLight;		// Diffuse.
vec3 specLight;		// Specular.
vec3 totalLight;	// Totalt ljus.

const float waterRefInd = 1.34451;
const float airRefInd = 1.0;
vec3 up = vec3(0.0, 1.0, 0.0);

void main(void)
{
	// Infallande och reflekterat ljus ber�knas f�r alla ljusk�llor.
	s = normalize(vec3(lightSourcePosAir.x, lightSourcePosAir.y, lightSourcePosAir.z) - (1 - isDirectional) * out_ObjPos);
	r = normalize(2 * out_Normal * dot(normalize(s), normalize(out_Normal)) - s);

	// eye-vektorn ber�knas.
	eye = normalize(camPos - out_ObjPos);

	// Ljus enligt Phong-modellen:
	kamb = 0.1;
	kdiff = 0.5;
	kspec = 0.5;
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse-ljus ber�knas.
	diffLight += kdiff * lightSourceColor * max(0.0, dot(s, normalize(out_Normal)));
	// Spekul�rt ljus.
	specLight += kspec * lightSourceColor * max(0.0, pow(dot(r, eye), specularExponent));

	totalLight = vec3(0.0, 0.0, 0.0);
	// De olika ljuskomponenterna adderas till det totala ljuset.
	totalLight += ambLight;
	totalLight += diffLight;
	totalLight += specLight;
	
	// At this point, totalLight corresponds to the surface reflection (no skybox yet)
	vec3 right = cross(eye, up);
	// asin is not cheap, maybe approximations later
	float theta1 = asin(length(right));
	float theta2 = asin(airRefInd * sin(theta1) / waterRefInd);
	
	vec4 terrainDataAtSurface = texture(texUnit, out_TexCoord);

	float depth = out_ObjPos.y - terrainDataAtSurface.a;
	float bottomDisplacement = tan(theta2) * depth;

	vec3 displacementDirection = normalize(cross(right, up));

	vec3 displacement = bottomDisplacement * displacementDirection;

	// Nedanstående korrekt?
	vec4 terrainDataAtBottom = texture(texUnit, out_TexCoord + vec2(displacement.x / texCoordScaleW, displacement.z / texCoordScaleH));
	
	vec3 bottomPos = out_ObjPos + displacement;
	vec3 bottomNormal = vec3(terrainDataAtBottom.x, terrainDataAtBottom.y, terrainDataAtBottom.z);

	// Phong lighting for the bottom
	s = normalize(vec3(lightSourcePosWater.x, lightSourcePosWater.y, lightSourcePosWater.z) - (1 - isDirectional) * bottomPos);
	r = normalize(2 * bottomNormal * dot(normalize(s), normalize(bottomNormal)) - s);

	// eye-vektorn ber�knas.
	eye = normalize(-(-up * depth + displacement));

	// Ljus enligt Phong-modellen:
	kamb = 0.1;
	kdiff = 0.5;
	kspec = 0.5;
	ambLight = kamb * vec3(1.0, 1.0, 1.0);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse-ljus ber�knas.
	diffLight += kdiff * lightSourceColor * max(0.0, dot(s, normalize(bottomNormal)));
	// Spekul�rt ljus.
	specLight += kspec * lightSourceColor * max(0.0, pow(dot(r, eye), specularExponent));

	vec3 bottomLight = vec3(0.0, 0.0, 0.0);
	// De olika ljuskomponenterna adderas till det totala ljuset.
	bottomLight += ambLight;
	bottomLight += diffLight;
	bottomLight += specLight;

	out_Color = vec4(0.5 * totalLight + 0.5 * bottomLight, 1.0);
}


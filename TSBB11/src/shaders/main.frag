#version 150

in vec3 out_Normal;
in vec2 out_TexCoord;
in vec3 out_ObjPos;

out vec4 out_Color;

uniform float t;
uniform vec3 camPos;	// Kamernapositionen.
uniform vec3 lightSourcePos;	// Ljuspositionen.
uniform int isDirectional;
uniform float specularExponent;
uniform vec3 lightSourceColor;
uniform sampler2D texUnit;

uniform int texWidth;
uniform int texHeight;

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

void main(void)
{
	// --- Phong stuff ---
	// --- FIXA ---
	//isDirectional = 1;
	//lightSourceColor = vec3(1.0, 1.0, 1.0);
	//specularExponent = 25.0;
	// ------------
	// Infallande och reflekterat ljus ber�knas f�r alla ljusk�llor.
	s = normalize(vec3(lightSourcePos.x, lightSourcePos.y, lightSourcePos.z) - (1 - isDirectional) * out_ObjPos);
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

	vec2 mod_TexCoord = out_TexCoord;
	mod_TexCoord.x = mod_TexCoord.x / texWidth;
	mod_TexCoord.y = mod_TexCoord.y / texHeight;

	out_Color = texture(texUnit, mod_TexCoord) * vec4(totalLight, 1);
}


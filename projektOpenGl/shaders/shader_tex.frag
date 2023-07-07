#version 430 core

float AMBIENT = 0.1;

uniform vec3 color;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform vec3 lightPos;


uniform vec3 spotPos;
uniform vec3 spotDir;
uniform float phi;
uniform float spotIntensity;


uniform sampler2D colorTexture;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 vecTex;

out vec4 outColor;
void main()
{

	vec3 lightDir = normalize(lightPos-worldPos);

	vec3 normal = normalize(vecNormal);
	float diffuse = max(0.0, dot(normal, lightDir));
	vec3 v = normalize(cameraPos - worldPos);
	vec3 r = reflect(-lightDir, normal);
	vec3 specularColor = lightColor * pow(max(dot(v, r), 0.0), 8.0);

	vec3 textureColor = texture2D(colorTexture, vecTex).xyz;

	vec4 light1 = vec4(textureColor*min(1,AMBIENT+diffuse) + specularColor, 7.0);


	vec3 spotToSurface = normalize(worldPos - spotPos);
    float spotFactor = dot(spotDir, spotToSurface);
    float cosPhi = cos(radians(phi / 2.0));
    float spotAttenuation = smoothstep(cosPhi, 1.0, spotFactor);
	float rangeAttenuation = smoothstep(0.0, 1.0, 1.0 - distance(spotPos, worldPos) / 6.0);

    spotAttenuation = rangeAttenuation * spotAttenuation;

    if (spotAttenuation <= 0.0) {
        outColor = light1;
        return;
    }

	outColor = light1 + vec4(textureColor * (min(1.0, AMBIENT + diffuse) + specularColor)  * spotIntensity * spotAttenuation , 1.0);







	
}
// 
// Bui Tuong Phong shading model (per-pixel) 


//
// Bui Tuong Phong shading model (per-pixel)


varying vec3 vNormal;
varying vec3 vPos;
varying vec4 vColor;

uniform float shiness; // default 200 in .gdb config

struct SLight
{
        vec3 pos;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float shiness;
};
vec3 computeColor(SLight light)
{
        vec3 n = normalize(vNormal);
        vec3 color = vColor.xyz;
        vec3 ambient = color * light.ambient;
        vec3 lightDir = normalize(light.pos - vPos);
        vec3 viewDir = (vec3(0, 0, 3) - vPos);

        float ndotl = dot(n, lightDir);
        vec3 diffuse = vec3(0.0);
        if (ndotl > 0.0)
                diffuse = color * light.diffuse * ndotl;

        vec3 halfDir = normalize(lightDir + viewDir);
        vec3 specular = light.specular * pow(max(dot(n, halfDir), 0.0), light.shiness) * color;

        vec3 objColor = specular + diffuse + ambient;
        return objColor;
}
void main()
{
        vec4 color = vColor;
        SLight light0;
        light0.ambient = vec3(0.1, 0.1, 0.08);
        light0.diffuse = vec3(0.9, 0.829, 0.829);
        light0.specular = vec3(0.2966, 0.2966, 0.2966);
        light0.shiness = 400.0;
        light0.pos = vec3(0, 0, 5);

        SLight light1;
        light1.ambient = vec3(0.3, 0.3, 0.24);
        light1.diffuse = vec3(0.9, 0.829, 0.829);
        light1.specular = vec3(0.5, 0.5, 0.5);
        light1.shiness = 200.0;
        light1.pos = vec3(5,0, 2);

        SLight light2;
        light2.ambient = vec3(0.3, 0.3, 0.24);
        light2.diffuse = vec3(0.9294, 0.9019, 0.8549);
        light2.specular = vec3(0.5, 0.5, 0.5);
        light2.shiness = 200.0;
        light2.pos = vec3(-5, 0, 2);

        vec3 colorLight0 = computeColor(light0);
        vec3 colorLight1 = computeColor(light1);
        vec3 colorLight2 = computeColor(light2);


        colorLight0 = pow(colorLight0, vec3(1.0 / 2.2));
        colorLight1 = pow(colorLight1, vec3(1.0 / 2.2));
        colorLight2 = pow(colorLight2, vec3(1.0 / 2.2));

        
        color.xyz = colorLight0/4 + colorLight1/3 + colorLight2*2/5 ;
        gl_FragColor = color;
}

#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

void main()
{
    vec4 color = texture(texture0, fragTexCoord);

    color += texture(texture0, fragTexCoord + 0.001);
    color += texture(texture0, fragTexCoord + 0.003);
    color += texture(texture0, fragTexCoord + 0.005);
    color += texture(texture0, fragTexCoord + 0.007);
    color += texture(texture0, fragTexCoord + 0.009);
    color += texture(texture0, fragTexCoord + 0.011);

    color += texture(texture0, fragTexCoord - 0.001);
    color += texture(texture0, fragTexCoord - 0.003);
    color += texture(texture0, fragTexCoord - 0.005);
    color += texture(texture0, fragTexCoord - 0.007);
    color += texture(texture0, fragTexCoord - 0.009);
    color += texture(texture0, fragTexCoord - 0.011);

    color.rgb = vec3((color.r + color.g + color.b)/3.0);
    color = color/9.5;

    fragColor = color;
}			
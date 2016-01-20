#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 sum = vec4(0);
    vec4 tc = vec4(0);

    for (int i = -4; i < 4; i++)
    {
        for (int j = -3; j < 3; j++)
        {
            sum += texture(texture0, fragTexCoord + vec2(j, i)*0.004) * 0.25;
        }
    }
    
    if (texture(texture0, fragTexCoord).r < 0.3)
    {
        tc = sum*sum*0.012 + texture(texture0, fragTexCoord);
    }
    else
    {
        if (texture(texture0, fragTexCoord).r < 0.5)
        {
            tc = sum*sum*0.009 + texture(texture0, fragTexCoord);
        }
        else
        {
            tc = sum*sum*0.0075 + texture(texture0, fragTexCoord);
        }
    }
    
    fragColor = tc;
}
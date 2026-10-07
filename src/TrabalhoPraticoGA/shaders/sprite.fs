#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D spriteTexture;
uniform vec4 spriteColor;

void main()
{
    vec4 texColor = texture(spriteTexture, TexCoord);
    
    // Descarte de pixels totalmente transparentes para alpha blending limpo
    if (texColor.a < 0.05)
        discard;
        
    FragColor = texColor * spriteColor;
}

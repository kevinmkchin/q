#version 330

uniform sampler2D screen_texture;

in vec2 texcoord;

out vec4 color;

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
    if(in_color.w < 0.001)
    {
    	discard;
    }
	color = in_color;
}
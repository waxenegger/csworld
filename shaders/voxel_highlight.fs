#version 460 core

uniform sampler2D depth;

out vec4 color;

void main() {
    ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));
    vec4 envDepth = texelFetch(depth, texcoord, 0);
    if (envDepth.r > gl_FragCoord.z)
        color = vec4(1.f,1.f,1.f,1.f);
    else
        color = vec4(1.f,1.f,1.f,0.f);
}
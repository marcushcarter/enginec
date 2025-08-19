// particles.vert
#version 430
layout(std430, binding = 0) buffer Particles {
    struct Particle {
        vec4 pos_life;
        vec4 vel_age;
        vec4 col_size;
    } p[];
};

uniform mat4 u_view;
uniform mat4 u_proj;

out vec4 vColor;
out float vLife;   // remaining life
out float vAge;    // age
out float vSize;   // pixel size suggestion

void main() {
    uint id = uint(gl_VertexID);
    vec3 pos = p[id].pos_life.xyz;
    float life = p[id].pos_life.w;
    float age  = p[id].vel_age.w;
    vec3 color = p[id].col_size.xyz;
    float size = p[id].col_size.w;

    // if dead, push offscreen + size 0 (cheap cull)
    if (life <= 0.0) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        gl_PointSize = 0.0;
        vColor = vec4(0);
        vLife = 0.0;
        vAge  = 0.0;
        vSize = 0.0;
        return;
    }

    vec4 viewPos = u_view * vec4(pos, 1.0);
    gl_Position = u_proj * viewPos;

    // perspective-correct point size: size is in pixels already; scale by distance if you want
    // (using constant size looks fine for many FX; you can multiply by 1/|viewPos.z| for world-size scaling)
    gl_PointSize = size;

    vColor = vec4(color, 1.0);
    vLife  = life;
    vAge   = age;
    vSize  = size;
}

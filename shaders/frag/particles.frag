// particles.frag
#version 430
in vec4  vColor;
in float vLife;
in float vAge;
in float vSize;

layout(location=0) out vec4 oColor;

uniform sampler2D u_sprite; // soft circle or flipbook atlas
uniform bool u_additive;    // toggle additive/alpha blending

void main() {
    // gl_PointCoord in [0,1]
    vec2 uv = gl_PointCoord;

    // For a simple soft circle sprite:
    vec4 tex = texture(u_sprite, uv);

    // Lifetime fade (soft): alpha = alpha * saturate(lifeRatio)
    float lifeRatio = clamp(vLife / (vLife + vAge + 1e-5), 0.0, 1.0);
    float alpha = tex.a * lifeRatio;

    // Premultiply if you prefer; here we’ll do straight alpha
    oColor = vec4(vColor.rgb * tex.rgb, alpha);

    if (oColor.a <= 0.01) discard; // reduce overdraw slightly
}


#version 330 core
layout (lines_adjacency) in;
layout (line_strip, max_vertices = 128) out;

in VS_OUT {
    vec4 color;
} gs_in[4];

out vec4 ourColor;  

uniform float start;
uniform float end;
uniform mat4 mvp;


int max_verts = 120;

vec3 drawBrezier4(float t, vec3 B0, vec3 B1, vec3 B2, vec3 B3) {
    float one_minus_t = 1.0 - t;
    if (t == 0.0f) return B0;
    if (t >= 1.0f) return B3;

    vec3 B0_ = B0 * one_minus_t + B1 * t;
    vec3 B1_ = B1 * one_minus_t + B2 * t;
    vec3 B2_ = B2 * one_minus_t + B3 * t;

    vec3 B0_d_ = B0_ * one_minus_t + B1_ * t;
    vec3 B1_d_ = B1_ * one_minus_t + B2_ * t;

    return B0_d_ * one_minus_t + B1_d_ * t;
}

vec3 drawBrezier3(float t, vec3 B0, vec3 B1, vec3 B2) {
    float one_minus_t = 1.0 - t;
    if (t == 0.0f) return B0;
    if (t >= 1.0f) return B2;

    vec3 B0_ = B0 * one_minus_t + B1 * t;
    vec3 B1_ = B1 * one_minus_t + B2 * t;

    return B0_ * one_minus_t + B1_ * t;
}

vec3 drawBrezier2(float t, vec3 B0, vec3 B1) {
    float one_minus_t = 1.0 - t;
    if (t == 0.0f) return B0;
    if (t >= 1.0f) return B1;

    return B0 * one_minus_t + B1 * t;
}



void main() {
    if (gs_in[0].color.x <0.0f || gs_in[1].color.x <0.0f) {
        EndPrimitive();
        return;
    }

    vec3 B0, B1, B2, B3;

    B0 = gl_in[0].gl_Position.xyz;
    B1 = gl_in[1].gl_Position.xyz;
    B2 = gl_in[2].gl_Position.xyz;
    B3 = gl_in[3].gl_Position.xyz;

    vec3 res;

    int number_of_proper_vetrex = 4;
    if(gs_in[3].color.x <0.0f) number_of_proper_vetrex = 3;
    if(gs_in[2].color.x <0.0f) number_of_proper_vetrex = 2;
    

    ourColor = gs_in[0].color;
    float t = 0.0f;
    float stride = end - start;
    stride /= max_verts;
    for (int i = 0; i<= max_verts; i++) {
        t = start + i * stride;
        if (number_of_proper_vetrex < 3) {
            res = drawBrezier2(t, B0, B1);
        }
        else if (number_of_proper_vetrex < 4) {
            res = drawBrezier3(t, B0, B1, B2);
        }
        else {
            res = drawBrezier4(t, B0, B1, B2, B3);
        }

        gl_Position = vec4(res,1.0f);
        gl_Position = mvp * gl_Position;
        EmitVertex();
    }

    EndPrimitive();
}  